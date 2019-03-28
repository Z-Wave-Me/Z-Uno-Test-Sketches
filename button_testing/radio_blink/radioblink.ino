#define PIN_LED1     10 
#define PIN_LED2     9

byte dimmerValue=100;
byte LedValue1=0;
byte LedValue2=0;

//Next line sets up Z-Uno channels. In this case it adds the Switch Multilevel channel
ZUNO_SETUP_CHANNELS(ZUNO_SWITCH_MULTILEVEL(getSwitchMultilevelValue,setSwitchMultilevelValue),ZUNO_SWITCH_MULTILEVEL(get_led_1, set_led_1),ZUNO_SWITCH_MULTILEVEL(get_led_2, set_led_2));
// the setup function runs once, when you press reset or power the board
void setup() {
  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_LED2, OUTPUT); 
  pinMode(LED_BUILTIN, OUTPUT);
  dimmerValue = 100;
}

void loop() {
  /*
  digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
  delay(dimmerValue*10);           // wait for timeout
  digitalWrite(LED_BUILTIN, LOW);  // turn the LED off by making the voltage LOW
  delay(dimmerValue*10);           // wait for timeout
  */
  digitalWrite(PIN_LED1, LedValue1);//первый диод
  digitalWrite(PIN_LED2, LedValue2);//второй диод
  delay(20);
}

void setSwitchMultilevelValue(byte newValue) {
  dimmerValue = newValue;
}

byte getSwitchMultilevelValue(void) {
  return dimmerValue;
}

void set_led_1(byte newValue) {
  LedValue1 = newValue;
}

byte get_led_1(void) {
  return LedValue1;
}

void set_led_2(byte newValue) {
  LedValue2 = newValue;
}

byte get_led_2(void) {
  return LedValue2;
}