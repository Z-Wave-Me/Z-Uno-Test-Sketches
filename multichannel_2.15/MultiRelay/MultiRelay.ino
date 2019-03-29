/*
  This sketch provide a method to control up to 21 
  switch. You have to configure the number or channels you
  want via parameter #64. Default value is 4. 
  You can use this to control open drain controlling device. 
  For this purpose you have to setup parameter #65 to 0. 
  It switches normal/inverse logic. 
    0 = inverse (open drain) logic, 
    1 = normal logic
  This sketch uses dynamic approach of channel definition
  to decrease code space and demonstrate how to use it.
  See setup() & zunoCallback() functions for details.
  (c) Z-Wave>ME 2017
 */
#define NUM_CHANNELS   32 
#define DEFAULT_NUMBER_OF_CHANNELS 4
#define DEFAULT_CHANNEL_LOGIC      1

ZUNO_DYNAMIC_CHANNELS(NUM_CHANNELS);
ZUNO_SETUP_CFGPARAMETER_HANDLER(config_parameter_changed);  
ZUNO_ENABLE(WITH_CC_SWITCH_BINARY);

byte  number_of_channels = DEFAULT_NUMBER_OF_CHANNELS; // Default
byte  channel_logic      = DEFAULT_CHANNEL_LOGIC;
word  tmp;



// pin mapping
byte  pin_mapping[]={0, // ZUNO GPIO #0 
                      1, // ZUNO GPIO #1  
                      2, // ZUNO GPIO #2 
                      // -- WE SKIP UART0, LEAVE IT FOR DEBUG -- 
                      // UNCOMMENT IF NEEDED
                      // 24, // TX0
                      // 25, // RX0
                      // --------------------
                      3, // ZUNO A0
                      4, // ZUNO A1
                      5, // ZUNO A2
                      6, // ZUNO A3
                      7, // ZUNO GPIO #7
                      8, // ZUNO GPIO #8
                      9, // ZUNO GPIO #9
                      10,// ZUNO GPIO #10 
                      11,// ZUNO GPIO #11
                      12,// ZUNO GPIO #12
                      13,// ZUNO PW1
                      14,// ZUNO PW2
                      15,// ZUNO PW3
                      16,// ZUNO PW4
                      17,// ZUNO GPIO 17
                      // -- WE SKIP INT1, IT ALWAY PULLED UP --
                      // IF YOU NEED NC PIN - YOU CAN UNCOMMENT IT
                      // 18, // INT1
                      // --------------------
                      19,
                      20,
                      21,
                      22,
                      // -- WE SKIP BTN
                      // IF YOU NEED NC PIN - YOU CAN UNCOMMENT IT
                      // 23, // BTN
                      // --------------------
                      
                      };
byte pin_states[sizeof(pin_mapping)];


// the setup function runs once, when you press reset or power the board
void setup() {
  byte i;
  // Loading configuration data...
  //zunoLoadCFGParam(64,&tmp); // Here we can use dword, but byte is cheaper
  number_of_channels = zunoLoadCFGParam(64);
  //zunoLoadCFGParam(65,&tmp); // Here we can use dword, but byte is cheaper
  channel_logic = zunoLoadCFGParam(65);
 
  // Check if value of param is valid
  Serial0.begin(115200);
  Serial0.print("number_of_channels=");
  Serial0.println(number_of_channels);
  Serial0.print("size pin_maping=");
  Serial0.println(sizeof(pin_mapping));
  if((number_of_channels > sizeof(pin_mapping)) || (number_of_channels == 0))
  {
    tmp = number_of_channels = DEFAULT_NUMBER_OF_CHANNELS;
    //zunoSaveCFGParam(64,&tmp); 
    zunoSaveCFGParam(64,tmp);//--------------------------------------------------------------------------------------------------------???? 
  }
  if((channel_logic != 0) && (channel_logic != 1))
  {
    channel_logic = DEFAULT_CHANNEL_LOGIC;
  }
  // we use "off" by default
  // you can use EEPROM library to load/save default states
  memset(pin_states, 0, number_of_channels);
  // Dynamic config...
  ZUNO_START_CONFIG();
  for(i=0;i<number_of_channels;i++)
  {
    // add Z-Wave channel
    if(i==0){
      ZUNO_SET_ZWCHANNEL(0x81);
    } else {
      ZUNO_SET_ZWCHANNEL(i+1);
    }
    ZUNO_ADD_CHANNEL(ZUNO_SWITCH_BINARY_CHANNEL_NUMBER, 0, 0)
    // set pin to output
    pinMode(pin_mapping[i], OUTPUT);
    digitalWrite(pin_mapping[i],!channel_logic); 

  }
  ZUNO_COMMIT_CONFIG();
}
// the loop function runs over and over again forever
void loop() {
  // We don't need loop here...
  // All logic is located in zunoCallback
  byte i;
  for(i=0;i<number_of_channels;i++){
    digitalWrite(pin_mapping[i], g_channels_data[i].bParam ? channel_logic : 1-channel_logic); 
  }
  delay(20);
}
void config_parameter_changed(byte param, word value) {
  if(param == 64) {
    number_of_channels = value;
  } else if(param == 65){
channel_logic = value;
  }
}