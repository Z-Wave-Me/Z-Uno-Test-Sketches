#include <ZUNO_Buttons.h>

// comment the next line if you don't need debug output. 
#define DBG_SERIAL Serial0 // Selects needed serial for debug output

#define PIN_BUTTON_1      6
#define PIN_BUTTON_2      10
#define PIN_BUTTON_3      11
#define PIN_BUTTON_4      12


PinButton button_1(PIN_BUTTON_1);
PinButton button_2(PIN_BUTTON_2);
PinButton button_3(PIN_BUTTON_3);
PinButton button_4(PIN_BUTTON_4);


byte flag = 0; 

ZUNO_SETUP_ASSOCIATIONS(ZUNO_ASSOCIATION_GROUP_SET_VALUE_AND_DIM,ZUNO_ASSOCIATION_GROUP_SET_VALUE_AND_DIM,ZUNO_ASSOCIATION_GROUP_SET_VALUE_AND_DIM,ZUNO_ASSOCIATION_GROUP_SET_VALUE_AND_DIM);


void setup() {




   #ifdef DBG_SERIAL
   DBG_SERIAL.begin(115200);
   #endif 

  pinMode(PIN_BUTTON_1, INPUT_PULLUP);
  pinMode(PIN_BUTTON_2, INPUT_PULLUP);  
  pinMode(PIN_BUTTON_3, INPUT_PULLUP);  
  pinMode(PIN_BUTTON_4, INPUT_PULLUP); 

}

void loop(){
button_1.update();
button_2.update();
button_3.update();
button_4.update();

//---------------------------first button--------------------------------------------------------------------------
if (button_1.isSingleClick()){
  zunoSendToGroupSetValueCommand(CTRL_GROUP_1, 255);
    #ifdef DBG_SERIAL
   DBG_SERIAL.println("кнопку 1 нажали");
   #endif 
  }

if (button_1.isLongClick()){
  zunoSendToGroupSetValueCommand(CTRL_GROUP_1, 0);
  flag =1;
    #ifdef DBG_SERIAL
   DBG_SERIAL.println("кнопку 1 держат");
   #endif 
}


//---------------------------second button-------------------------------------------------------------------------
  if (button_2.isSingleClick()){
  zunoSendToGroupSetValueCommand(CTRL_GROUP_2, 255);
    #ifdef DBG_SERIAL
   DBG_SERIAL.println("кнопку 2 нажали");
   #endif 
  }

  if (button_2.isLongClick()){
  zunoSendToGroupSetValueCommand(CTRL_GROUP_2, 0);
  flag =1;
    #ifdef DBG_SERIAL
   DBG_SERIAL.println("кнопку 2 держат");
   #endif 
}
//----------------------------third button-------------------------------------------------------------------------
  if (button_3.isSingleClick()){
  zunoSendToGroupSetValueCommand(CTRL_GROUP_3, 255);
    #ifdef DBG_SERIAL
   DBG_SERIAL.println("кнопку 3 нажали");
   #endif 
  }

  if (button_3.isLongClick()){
  zunoSendToGroupSetValueCommand(CTRL_GROUP_3, 0);
  flag =1;
    #ifdef DBG_SERIAL
   DBG_SERIAL.println("кнопку 3 держат");
   #endif 
}
//----------------------------fourth button------------------------------------------------------------------------
  if (button_4.isSingleClick()){
  zunoSendToGroupSetValueCommand(CTRL_GROUP_4, 255);
    #ifdef DBG_SERIAL
   DBG_SERIAL.println("кнопку 4 нажали");
   #endif 
  }

  if (button_4.isLongClick()){
  zunoSendToGroupSetValueCommand(CTRL_GROUP_4, 0);
  flag =1;
    #ifdef DBG_SERIAL
   DBG_SERIAL.println("кнопку 4 держат");
   #endif 
}





/*
//---------------------------------first button---------------------------------------------------------------

if (button_1.isSingleClick()){
zunoSendToGroupDimmingCommand(CTRL_GROUP_1, FALSE, TRUE);
    #ifdef DBG_SERIAL
   DBG_SERIAL.println("кнопку 1 нажали");
   #endif 
  }
//---------------------------------second button--------------------------------------------------------------
if (button_2.isSingleClick()){
  zunoSendToGroupDimmingCommand(CTRL_GROUP_1, FALSE, FALSE);
    #ifdef DBG_SERIAL
   DBG_SERIAL.println("кнопку 2 нажали");
   #endif 
}
//---------------------------------third button---------------------------------------------------------------
if (button_3.isSingleClick()){
  zunoSendToGroupDimmingCommand(CTRL_GROUP_1, TRUE, TRUE);
    #ifdef DBG_SERIAL
   DBG_SERIAL.println("кнопку 3 нажали");
   #endif 
}
//---------------------------------fourth button--------------------------------------------------------------
if (button_4.isSingleClick()){
  zunoSendToGroupDimmingCommand(CTRL_GROUP_1, TRUE, FALSE);
    #ifdef DBG_SERIAL
   DBG_SERIAL.println("кнопку 4 нажали");
   #endif 
}

*/




/*

//------------------------------------first button-------------------------------------------------------------
if (button_1.isSingleClick()){
  zunoSendToGroupSetValueCommand(CTRL_GROUP_1, 255);
    #ifdef DBG_SERIAL
   DBG_SERIAL.println("кнопку 1 нажали");
   #endif 
}

if (button_1.isLongClick()){
  zunoSendToGroupDimmingCommand(CTRL_GROUP_1, FALSE, TRUE);
  flag =1;
    #ifdef DBG_SERIAL
   DBG_SERIAL.println("кнопку 1 держат");
   #endif 
}
if ((button_1.isReleased())&&(flag==1)){
  zunoSendToGroupDimmingCommand(CTRL_GROUP_1, FALSE, FALSE);
  flag=0;
   #ifdef DBG_SERIAL
   DBG_SERIAL.println("кнопку 1 отпустили");
   #endif 
}

//-------------------------------------second button------------------------------------------------------------
if (button_2.isSingleClick()){
  zunoSendToGroupSetValueCommand(CTRL_GROUP_1, 0);
    #ifdef DBG_SERIAL
   DBG_SERIAL.println("кнопку 2 нажали");
   #endif
}
if (button_2.isLongClick()){
  zunoSendToGroupDimmingCommand(CTRL_GROUP_1, TRUE, TRUE);
  flag=1;
   #ifdef DBG_SERIAL
   DBG_SERIAL.println("кнопку 2 держат");
   #endif
}
if ((button_2.isReleased())&&(flag==1)){
  zunoSendToGroupDimmingCommand(CTRL_GROUP_1, TRUE, FALSE);
  flag=0;
   #ifdef DBG_SERIAL
   DBG_SERIAL.println("кнопку 2 отпустили");
   #endif
}

//------------------------------------------third button--------------------------------------------------------
if (button_3.isSingleClick()){
  zunoSendToGroupSetValueCommand(CTRL_GROUP_2, 100);
   #ifdef DBG_SERIAL
   DBG_SERIAL.println("кнопку 3 нажали");
   #endif
}

if (button_3.isLongClick()){
  zunoSendToGroupDimmingCommand(CTRL_GROUP_2, FALSE, TRUE);
  flag=1;
   #ifdef DBG_SERIAL
   DBG_SERIAL.println("кнопку 3 держат");
   #endif
}
if ((button_3.isReleased())&&(flag==1)){
  zunoSendToGroupDimmingCommand(CTRL_GROUP_2, FALSE, FALSE);
   flag=0;
   #ifdef DBG_SERIAL
   DBG_SERIAL.println("кнопку 3 отпустили");
   #endif
}
//------------------------------------------fourth button-------------------------------------------------------
if (button_4.isSingleClick()){
  zunoSendToGroupSetValueCommand(CTRL_GROUP_2, 0);
   #ifdef DBG_SERIAL
   DBG_SERIAL.println("кнопку 4 нажали");
   #endif
}

if (button_4.isLongClick()){
  zunoSendToGroupDimmingCommand(CTRL_GROUP_2, TRUE, TRUE);
  flag=1;
   #ifdef DBG_SERIAL
   DBG_SERIAL.println("кнопку 4 держат");
   #endif
}
if ((button_4.isReleased())&&(flag==1)){
  zunoSendToGroupDimmingCommand(CTRL_GROUP_2, TRUE, FALSE);
  flag=0; 
   #ifdef DBG_SERIAL
   DBG_SERIAL.println("кнопку 4 отпустили");
   #endif
}
*/

delay(40);
}