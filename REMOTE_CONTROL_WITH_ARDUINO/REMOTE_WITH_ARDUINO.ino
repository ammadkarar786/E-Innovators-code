#include <IRremote.h>
#include <SoftwareSerial.h>
IRsend irsend;
uint32_t  channelcode[]={0x10EF9B64,0x10EFA956,0x10EF9966,0x10EFB946,0x10EF6B94,0x10EF5BA4,0x10EF7B84,0x10EF6996,0x10EF59A6,0x10EF7986};
                         // 0             1         2         3            4         5          6          7          8          9   
SoftwareSerial BT(10, 11); //TX, RX respetively
String state;// string to store incoming message from bluetooth

void setup() {
  Serial.begin(9600);
  BT.begin(9600);// bluetooth serial communication will happen on pin 10 and 11

}

void loop() {
 while (BT.available()){  //Check if there is an available byte to read
  delay(10); //Delay added to make thing stable 
  char c = BT.read(); //Conduct a serial read
  state += c; 
  }  
  if (state.length() > 0) {
    Serial.println(state);
    if(state!="Volume up"&&state!="Volume down"&&state!="power"&&state!="mute"){
  int intstate =state.toInt(); //CHANGE TTHE STRING INTO NUMBER 
  int ten=intstate/10;///    62    SO 62/10     6   02    0
  int one=intstate%10;///    62    SO 62%10     2   2     2 
  Serial.println(ten);
  Serial.println(one);
  if(intstate>9  && intstate<100){ //if two digit number so enter in the loop 
  irsend.sendNEC(channelcode[ten], 32); 
  Serial.println(channelcode[ten]);
  delay(1000);
  irsend.sendNEC(channelcode[one], 32);
  Serial.println(channelcode[one]); 
  }
  else if(intstate>=0 && intstate<=9 ){//if one digit number enter in this for 
  irsend.sendNEC(channelcode[one], 32); 
   }
   else{
    Serial.print("NOT FOUND");
    }
   
   }
   else{
    if(state=="Volume up"){
    irsend.sendNEC(0x10EFC13E, 32); 
    }
    else if(state=="Volume down"){
    irsend.sendNEC(0x10EF619E, 32); 
    }
else if(state=="power"){
    irsend.sendNEC(0x10EFEB14, 32); 
    }
else if(state=="mute"){
    irsend.sendNEC(0x10EFFB04, 32); 
    }
else{
  Serial.print("NOT FOUND");
  }
    
    }    

   
  
state ="";}} //Reset the variable





 
