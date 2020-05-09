
const int trigPin = 9;
const int echoPin = 10;
const int relay=11;
// defines variables
long duration;
int distance;
void setup() {
pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
pinMode(echoPin, INPUT); // Sets the echoPin as an Input
pinMode(relay,OUTPUT);  

}
void loop() {
// Clears the trigPin
digitalWrite(trigPin, LOW);
delayMicroseconds(4);
// Sets the trigPin on HIGH state for 10 micro seconds
digitalWrite(trigPin, HIGH);
delayMicroseconds(20);
digitalWrite(trigPin, LOW);
// Reads the echoPin, returns the sound wave travel time in microseconds
duration = pulseIn(echoPin, HIGH);
// Calculating the distance
distance= duration*0.034/2;
if (distance>=100){
  digitalWrite(relay,HIGH);//PUMP ON 
  }
 else{
 digitalWrite(relay,LOW);//PUMP OFF
    }
}
