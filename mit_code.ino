#include <SoftwareSerial.h>


SoftwareSerial BT(10, 11); //TX, RX respetively
String state;// string to store incoming message from bluetooth


void setup() {
 BT.begin(9600);// bluetooth serial communication will happen on pin 10 and 11
 Serial.begin(9600); // serial communication to check the data on serial monitor
  pinMode(13, OUTPUT); // LED connected to 13th pin




}
//-----------------------------------------------------------------------//  
void loop() {
  while (BT.available()){  //Check if there is an available byte to read
  delay(10); //Delay added to make thing stable 
  char c = BT.read(); //Conduct a serial read
  state += c; //build the string- either "On" or "off"
  }  
  if (state.length() > 0) {
    Serial.println(state); 


  if(state == "turn on") 
  {
    digitalWrite(13, HIGH);
    
      } 
  
  else if(state == "turn off") 
  {
    digitalWrite(13, LOW);
     }
  
  
state ="";}} //Reset the variable
