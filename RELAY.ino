int relaypin=11;//CONNECTED PIN 11 WITH RELAY INPUT
int button=7;//CONNECTED PIN 7 WITH BUTTON
void setup() {
  // put your setup code here, to run once:
 pinMode(relaypin,OUTPUT);
 pinMode(button,INPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
if (digitalRead(button)==HIGH){
  digitalWrite(relaypin,HIGH);
 }
 else{
digitalWrite(relaypin,LOW);

  }


}
