
/*
The load monitor continuously measures the current in phases L1, L2 and L3.
For fast load variations and at a power outage over 14 A, the load monitor sends reports to the controller.
At current below 14A, the LEDs turn green. At load between 14A and 16A, the diodes are red. At load over 16A, 
the diodes flash intense red. In addition to LEDs, find the option to display the values ​​in real-time on an OLED display
As an additional function, the load guard is equipped with an energy meter that measures pulses from LED on the 
electricity meter and sends them to the controller. 
The load guard is also equipped with a temperature sensor designed to measure the outdoor temperature.
 */
// Indatatavariabler belastningsvakt
// Reworked by Z-Wave.ME team, 2018 
// v 2.0



// Additional libraries
#include <EEPROM.h> 
#include <ZUNO_DS18B20.h>   // library ds18b20
#include <ZUNO_OLED_I2C.h>  // OLED display 
// OLED fonts
#include <ZUNO_OLED_FONT_NUMB16.h>
#include <ZUNO_OLED_FONT_NUMB24.h>
#include <ZUNO_OLED_FONT_NUMB40.h>

// Variabler, konstanter och specifikation av in- och utgångar
// C-style of constants. uCxx doesn't like C++ const style till now. 
// In some case they could be non-constans. To avoid a risk of it we use macroses
#define NUM_READINGS          6 //Size of Array for rolling average current
#define NUM_LS                3
#define DS18B20_ADDR_SIZE     8 // (!) not 6
#define NUM_SAMPLES           100 // how many samples required
#define LED_RUN               50
#define LED_PULSE             255
#define TEMPERATURE_PERIOD    150000 //check temperature once in chosen time
// in 0.01 of AMPS
#define I_HYSTERESIS  25 // 0.25 Amps
//#define H_FUSE        1400
//#define H_FUSE_DELTA  200
#define CALIBR_V      3085  // (in mV) 3.085 Calibrated with external voltage source, (if USB - 3.05 V)

#define KWH1000_HYSTERESIS 100   // 0.1Kwh
#define TEMP100_HYSTERESIS 50 // 0.5 ^C

//#define LED_HI_CURRENT        H_FUSE
//#define LED_OVERLOAD_CURRENT  (H_FUSE + H_FUSE_DELTA)

// comment the next line if you don't need debug output. 
#define DBG_SERIAL Serial // Selects needed serial for debug output
// Here We define the PINs
#define PIN_L1      A1
#define PIN_L2      A2
#define PIN_L3      A3
#define PIN_PULS    0
#define PIN_DS18B20     16  // pin connection ds18b20
#define PIN_SWITCH1     13
#define PIN_SWITCH2     17
#define PIN_SENSOR_BIN  15
#define PIN_Z_LED       14


#define AVG_WATTS   30000UL//72000UL // in ms. Avarage power calculation period
//Pulscounter

// EEPROM
#define EEPROM_ADDR             0x800
#define EEPROM_UPDATE_INTERVAL  120000UL
#define PULSES_PER_KWH1000      WhPerTick
#define PULSES2KWH1000()        current_KWh1000 = my_meter_data.ticks_puls; current_KWh1000 *=1000UL; current_KWh1000 /= PULSES_PER_KWH1000;//количество импульсов
#define DEBOUNCE_COUNT  3
#define METER_PIN_TRIGGERED() (!digitalRead(PIN_PULS)) // If you need a backward logic please use (!digitalRead(PIN_PULS))

enum {
  PULSE_PIN_IDLE,
  PULSE_PIN_DEBOUNCE,
  PULSE_PIN_TRIGGERED
};
enum {
  DISP_UPD_KWH = 0x01,
  DISP_UPD_WATTS = 0x02,
  DISP_UPD_TEMP = 0x04

};
// structure to store all the data about current in compact way
typedef struct sCurrentLData{
  int     records_avg[NUM_READINGS];
  dword   total_avg;
  byte    records_i;
  word     I100; // fixed point value 0.01A //  the current in L1
  word     last_I100; 
}CurrentLData;
// structure to store meter data in the eeprom
struct meter_data
{
    dword ticks_puls;
    byte relays_state_map;
    byte sen_binary_map;
    byte crc8;
};

meter_data my_meter_data;
CCBYTE current_pins[NUM_LS] = {PIN_L1,PIN_L2,PIN_L3};

// Calibration coefficients for Current sensors
// I = V*coef_a/coef_b + coef_c
// For example if you use k = 1800:1, R=62 Ohms
word   coef_a[] = {1800, 1800, 1800};    // K
word   coef_b[] = {62*10, 62*10, 62*10}; // R*10mV  
word   coef_c[] = {0, 0, 0}; // linear matching


// Device cacjed values
CurrentLData Ldata[NUM_LS];
dword current_KWh1000 = 0;
int   current_temp100 = 0;
dword last_KWh1000 = 0;
int   last_temp100 = 0;
BYTE  meter_was_resetted = TRUE;
unsigned long last_update_millis = 0;
word WhPerTick = 1200;
dword current_W = 0;
unsigned long last_update_delta_w = 0;
unsigned long last_update_millis_w = 0;
word last_update_ticks = 0;
dword math_tmp1;
dword math_tmp2;
word  delta;
byte  upd_display = 0;
// Peripherial devices
// DS18B20
OneWire       ow(PIN_DS18B20);
DS18B20Sensor ds1820(&ow); // onewire connection temperature sensors
byte          addr1[DS18B20_ADDR_SIZE];
// Display
OLED oled;
// Interrupt data
byte meter_pin_state = PULSE_PIN_IDLE;
byte meter_pin_debcnt = 0;
byte meter_pin_tmpcount = 0; // we use local small counter to make it fast
bool upd_ee =  FALSE;
bool force_upd_ee = FALSE;
// the last 3 channels
byte sen_bin_reported;
byte switches_update = 0x03;

byte g_loop_counter = 0;
long int time_count = 0;
// Set and definition of Z-Wave cfg parameter
ZUNO_SETUP_CFGPARAMETER_HANDLER(config_parameter_changed);


// Set and definition of Z wave channels
ZUNO_SETUP_CHANNELS(
  ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_CURRENT, SENSOR_MULTILEVEL_SCALE_AMPERE, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_TWO_DECIMALS, getterL1),
  ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_CURRENT, SENSOR_MULTILEVEL_SCALE_AMPERE, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_TWO_DECIMALS, getterL2),
  ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_CURRENT, SENSOR_MULTILEVEL_SCALE_AMPERE, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_TWO_DECIMALS, getterL3),
  ZUNO_METER(ZUNO_METER_TYPE_ELECTRIC, METER_RESET_ENABLE, ZUNO_METER_ELECTRIC_SCALE_KWH, METER_SIZE_FOUR_BYTES, METER_PRECISION_THREE_DECIMALS, getterPuls, resetterPuls),
  ZUNO_METER(ZUNO_METER_TYPE_ELECTRIC, 0, ZUNO_METER_ELECTRIC_SCALE_WATTS, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_ZERO_DECIMALS, getterW, 0),
  ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_TEMPERATURE,SENSOR_MULTILEVEL_SCALE_CELSIUS,SENSOR_MULTILEVEL_SIZE_TWO_BYTES,SENSOR_MULTILEVEL_PRECISION_TWO_DECIMALS,getterTemp),
  ZUNO_SENSOR_BINARY(ZUNO_SENSOR_BINARY_TYPE_GENERAL_PURPOSE,getterBinary),
  ZUNO_SWITCH_BINARY(getterSwitch1, setterSwitch1),
  ZUNO_SWITCH_BINARY(getterSwitch2, setterSwitch2)
  );
  
// Timer 
// It makes possible to process data from pulses independent from loop
ZUNO_SETUP_ISR_1MSTIMER(my_1ms_timer);
// Timer interrupt handler
// Here is a simple "StateMachine"
void my_1ms_timer() {
  switch(meter_pin_state){
      case PULSE_PIN_IDLE:
          analogWrite(PIN_Z_LED, LED_RUN);
          if(METER_PIN_TRIGGERED()){
              meter_pin_debcnt = DEBOUNCE_COUNT;
              meter_pin_state = PULSE_PIN_DEBOUNCE;
          }
          break;
      case PULSE_PIN_DEBOUNCE:
          if(!METER_PIN_TRIGGERED()){
              meter_pin_state = PULSE_PIN_IDLE;
          }
          meter_pin_debcnt--;
          if(!meter_pin_debcnt){
            meter_pin_state = PULSE_PIN_TRIGGERED;
            meter_pin_tmpcount++;
            analogWrite(PIN_Z_LED, LED_PULSE);
            meter_pin_state = PULSE_PIN_TRIGGERED;
          }
          break;
      case PULSE_PIN_TRIGGERED:
          if(!METER_PIN_TRIGGERED())
            meter_pin_state = PULSE_PIN_IDLE;
          break;

  }
}

//Pulscounter
byte my_crc8(byte * data, byte count)
{
    byte result = 0xDF;

    while(count--)
    {
        result ^= *data;
        data++;
    }
    return result;
}
void update_meter_data()
{ 
    my_meter_data.crc8 = my_crc8((byte*)&my_meter_data, sizeof(meter_data) - 1);
    EEPROM.put(EEPROM_ADDR, &my_meter_data, sizeof(meter_data));
}

void setup() {
   #ifdef DBG_SERIAL
   DBG_SERIAL.begin(115200);
   DBG_SERIAL.print("Meter Init");
   #endif 
   // PINS
   pinMode(PIN_PULS, INPUT);   //Optical pulse counters run from VCC to GND at pulse. The code originally written for pullup. Will this be correct?
   pinMode(PIN_L1, INPUT);
   pinMode(PIN_L2, INPUT);
   pinMode(PIN_L3, INPUT);
   pinMode(PIN_Z_LED, INPUT);
   pinMode(PIN_DS18B20, INPUT);
   pinMode(PIN_SENSOR_BIN, INPUT_PULLUP);
   pinMode(PIN_SWITCH1, OUTPUT);
   pinMode(PIN_SWITCH2, OUTPUT);
   
   //Pulscounter
  // Get last meter values from EEPROM
    EEPROM.get(EEPROM_ADDR,  &my_meter_data, sizeof(meter_data));
    // Check data  
 if(my_crc8((byte*)&my_meter_data, sizeof(meter_data) - 1) != my_meter_data.crc8){
      // Invalid data - reset all
      #ifdef DBG_SERIAL
      DBG_SERIAL.println("Bad eeprom crc8 - init meter data");
      #endif
      my_meter_data.ticks_puls = 0;
      my_meter_data.relays_state_map = 0; // all relays off by default
      my_meter_data.sen_binary_map = 0; // all sensors idle by default
      update_meter_data();
  }

  // flush all the current data to zero
  memset((byte*)Ldata, 0, NUM_LS*sizeof(CurrentLData));
  // Update counter value
  PULSES2KWH1000();
  zunoSendReport(4); // Report it to controller
  // Update binary sensor
  zunoSendReport(7);
  // Update relays
  zunoSendReport(8);
  zunoSendReport(9);
 
  // initialize OLED
  oled.begin();
  oled.clrscr();
  // initialize DS18B20
  ds1820.scanAloneSensor(addr1);

  WhPerTick = zunoLoadCFGParam(64);//---------------------------------------------------------------------------------
  
  //zunoLoadCFGParam(64, &WhPerTick);
  // to stash "empty" memory value
  if(WhPerTick > 10000){
    WhPerTick = 1200;
  }
  //WhPerTick = 1200;
  meter_pin_tmpcount = 0;
}
// Here we use fixed point math (instead of float) 
// It reduces stack/code & RAM usage
int x_val,A0_val,i;
#ifdef DBG_SERIAL
int x_val_dump[NUM_SAMPLES];
#endif
byte x_count = 0;
byte x_start = 0;
byte s;
byte mask;
dword alt_cur_val[3]; // array of values for diods and oled


void updateCurrentData(byte ch) {
    //cycle for calculating mean-square values
    math_tmp1 = 0;
    byte j = 0;
    byte state = 0;
    x_count = 0;
    s = 0;
    for(j=0; j<NUM_SAMPLES; j++) {
      if (state == 4) 
        break;
      // Read ADC Values
      x_val = analogRead(A1+ch);
      A0_val = analogRead(A0);
      x_val -= A0_val;
      #ifdef DBG_SERIAL
      x_val_dump[j] = x_val;
      #endif
      // Store last n signs 
      s <<= 1; 
      if (x_val&0x8000) 
        s |= 0x01;
      s &= 0X03;
      // Sum of squares
      math_tmp1 += sq(dword(x_val));
      // State machine to find the halfperiods      
      switch (state){
          case 0 : 
            // find the sign of the start sequence and save it to mask
            // We have to use at least 2 samples to prevent the noise from ADC
            if ((j>=2)&&((s == 3)||(s == 0))){  
              state++;
              mask = s; 
            }                
            break;
          // Searching for the first halfperiod to start   
          case 1 :
            if ((s^mask) == 0x03){
              x_start = j-1;
              x_count = 0;  // The number of samples
              state++;
              mask = s; 
              math_tmp1 = 0;         
            }
            break;
        // The first and the second halfperiods
        case 2 :
        case 3 :
            x_count++;
            if ((s^mask) == 0x03){
                state++;
                mask = s;
            }
            break;
      }   
   }
         
    if(x_count){
      math_tmp1 /= x_count;
      math_tmp1 = dword(sqrt(math_tmp1));
      // 3.05 = 3050 mV
      // 3.05 * 1000
      math_tmp1 *= CALIBR_V;
      math_tmp1 /= 1023;
    } else {
      math_tmp1 = 0;
    }
    // Calculate the current using coeffitients:
    alt_cur_val[ch] = math_tmp1;
    alt_cur_val[ch] *= coef_a[ch];
    alt_cur_val[ch] /= coef_b[ch];
    alt_cur_val[ch] += coef_c[ch];
    #ifdef DBG_SERIAL
    DBG_SERIAL.print("X:");
    for(j=0;j<NUM_SAMPLES;j++){
      if (j == x_start) {DBG_SERIAL.print(" |START| "); }
      DBG_SERIAL.print(" ");
      if (j == x_count+x_start) DBG_SERIAL.print(" |END| ");
      DBG_SERIAL.print(x_val_dump[j]);
      x_val_dump[j] = 0;
    }
    DBG_SERIAL.println(" ");
    DBG_SERIAL.print("x_count=");
    DBG_SERIAL.println(x_count);
    DBG_SERIAL.print("Vrms=");
    DBG_SERIAL.fixPrint(int(math_tmp1),3);
    DBG_SERIAL.print("Irms=");
    DBG_SERIAL.fixPrint(int(alt_cur_val[ch]),2);
    #endif
    Ldata[ch].I100 = alt_cur_val[ch];
    // Do we need to send the report to controller?
    delta = MAX(Ldata[ch].I100,Ldata[ch].last_I100);
    delta -= MIN(Ldata[ch].I100,Ldata[ch].last_I100);
    delay(10);
}
// todo
void updateEnergyMeter(){
  if(meter_pin_tmpcount || meter_was_resetted){
    last_update_ticks += meter_pin_tmpcount;
    upd_ee = TRUE;  
    my_meter_data.ticks_puls += meter_pin_tmpcount; 
    PULSES2KWH1000();
    math_tmp1 = MAX(current_KWh1000,last_KWh1000);
    math_tmp1 -= MIN(current_KWh1000,last_KWh1000);
    if(math_tmp1 > KWH1000_HYSTERESIS)
      zunoSendReport(4);
    meter_was_resetted = FALSE;
    meter_pin_tmpcount = 0;
    upd_display |= DISP_UPD_KWH;

  } else {
    // analogWrite(PIN_Z_LED, LED_RUN);
  }
  if((upd_ee && ((millis() - last_update_millis ) > EEPROM_UPDATE_INTERVAL)) || force_upd_ee){
        update_meter_data();
        last_update_millis = millis();
        upd_ee = FALSE;
        force_upd_ee = FALSE;
  } 
  // Calculate the Power, AVG_WATTS is the time to accumulate pulses for power calculation
  last_update_delta_w = millis() - last_update_millis_w;
  if(last_update_delta_w >= AVG_WATTS) {
    
    math_tmp1 = 3600000UL;    // 1hour = 3600000ms
    math_tmp1 *= 1000;
    math_tmp1 /= last_update_delta_w;   
    math_tmp1 *= 100;
    math_tmp1 *= last_update_ticks; // ticks that we have during "last_update_delta_w"
    math_tmp1 /= 119;
    WhPerTick = 1200;
    math_tmp1 /= WhPerTick; 
    current_W = math_tmp1;
    
    last_update_millis_w = millis();
    last_update_ticks = 0;
    upd_display |= DISP_UPD_WATTS;
    zunoSendReport(5);
    
  }

}

void updateTemperature(){
    current_temp100 = ds1820.getTempC100(addr1); // this needs about one second on high precision BE CAREFUL!
    delta = MAX(current_temp100,last_temp100);
    delta -= MIN(current_temp100,last_temp100);
    if(delta > TEMP100_HYSTERESIS) {
      zunoSendReport(6);
      upd_display |= DISP_UPD_TEMP;
      last_temp100 = current_temp100;
    }
}

void display() {
  byte i;
  oled.setFont(0);
  if(upd_display & DISP_UPD_KWH) {
    oled.gotoXY(0, 0);
    oled.print("                        ");
    oled.gotoXY(0, 1);
    oled.print("                        ");
    oled.gotoXY(0, 0);
    oled.setFont(zuno_font_numbers16);
    oled.print((long)current_KWh1000/100);
    oled.setFont(0);
    oled.gotoXY(90, 1);
    oled.print("KWh");
  }
  // clear field for current
  oled.gotoXY(0, 3);
  oled.print("                    ");
  oled.gotoXY(0, 4);
  oled.print("                    "); 
  // print a value
  oled.gotoXY(0, 3);
  for(i=0;i<3;i++){
    oled.setFont(zuno_font_numbers16);
   if((int)alt_cur_val[i]>=10){
    oled.fixPrint((int)alt_cur_val[i],0);
    }
    else{
      oled.fixPrint((int)alt_cur_val[i],1);
      }  
    oled.setFont(0);
    if (i!=2){
    oled.print(" ");
    }
  }
  oled.setFont(0);
 
  if(upd_display & DISP_UPD_TEMP) { // update only if it needed
    oled.gotoXY(0, 5);
    oled.print("         ");
    oled.gotoXY(0, 6);
    oled.print("         ");
    oled.gotoXY(0, 6);
    oled.setFont(zuno_font_numbers16);
    if((int)current_temp100<=0){
      oled.fixPrint(((int)current_temp100),0);
      }
      else{
         oled.fixPrint(((int)current_temp100)/10,1);
        }
    oled.setFont(0);
    oled.gotoXY(52, 7);
    oled.print("*C");
  }
  if(upd_display & DISP_UPD_WATTS) { // update only if it needed
    oled.gotoXY(72, 5);
    oled.print("         ");
    oled.gotoXY(72, 6);
    oled.print("         ");
    oled.gotoXY(72,6);
    oled.setFont(zuno_font_numbers16);
    if (current_W<1000){
      oled.print(current_W);
      oled.setFont(0);
      oled.gotoXY(122,7);
      oled.print("W");
      }
      else if((current_W>1000)&&(current_W<10000)){
          oled.fixPrint((long)current_W/100,1);
          oled.setFont(0);
          oled.gotoXY(110,7);
          oled.print("kW");
        }
        else
        {
          oled.fixPrint((long)current_W/1000,0);
          oled.setFont(0);
          oled.gotoXY(110,7);
          oled.print("kW");
          }
  }
  upd_display = 0;
 

}


void config_parameter_changed(byte param, word value) {
  if(param == 64) {
    WhPerTick = value;
  }
}







void loop() {
  byte i;
  // Sensor binary 
  i = !digitalRead(PIN_SENSOR_BIN);
  if(i != (my_meter_data.sen_binary_map & 0x01)){
    if(i)
      my_meter_data.sen_binary_map |= 0x01;
    else
       my_meter_data.sen_binary_map &= ~(0x01);
    zunoSendReport(7);
    force_upd_ee = TRUE;
  }
  // Relays
  digitalWrite(PIN_SWITCH1, (my_meter_data.relays_state_map & 0x01) != 0);
  digitalWrite(PIN_SWITCH2, (my_meter_data.relays_state_map & 0x02) != 0);
  // Update L1..L3 data
  for(i=0;i<NUM_LS;i++){
      updateCurrentData(i);  
  }
  // Update Energy meter
  updateEnergyMeter();
  // Temperature once in 2min and 30sec , and for first loop
  if ((millis() - time_count > TEMPERATURE_PERIOD)||(time_count == 0)){ 
  time_count = millis(); 
  updateTemperature();
 }
  
  // once in 8 loops
  if((g_loop_counter & 0x07) == 0)
    display();
  // some delay to make OS alive
  delay(50);
  g_loop_counter++;
}  

// Values ​​sent to the controller upon request

word getterL1() {
    return Ldata[0].I100; 
}
word getterL2() {
    return Ldata[1].I100;  
}
word getterL3() {
    return Ldata[2].I100;  
}

void resetterPuls(byte v) {
  my_meter_data.ticks_puls = 0;
  meter_was_resetted = TRUE;
  
}
unsigned long getterPuls(void) {
  return current_KWh1000;
}
 
word getterTemp() {
    return current_temp100;  
}

word getterW() {
  return current_W;
}

byte getterBinary(){
   return my_meter_data.sen_binary_map & 0x01 ? 0xFF : 0x00;
}
void setterSwitch1(byte val){
   if(val)
      my_meter_data.relays_state_map |= 0x01;
   else
      my_meter_data.relays_state_map &= ~(0x01);
   force_upd_ee = TRUE;
   
}
byte getterSwitch1(){
   return my_meter_data.relays_state_map & 0x01 ? 0xFF : 0x00;
}
void setterSwitch2(byte val){
   if(val)
      my_meter_data.relays_state_map |= 0x02;
   else
      my_meter_data.relays_state_map &= ~(0x02);
   force_upd_ee = TRUE;
}
byte getterSwitch2(){
  return my_meter_data.relays_state_map & 0x02 ? 0xFF : 0x00;
}

