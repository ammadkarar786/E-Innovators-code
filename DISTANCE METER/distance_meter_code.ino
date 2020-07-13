#include <HCSR04.h>
#include <SoftwareSerial.h>
#include <Wire.h> // Library for I2C communication
#include <LiquidCrystal_I2C.h> // Library for LCD


LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27,16,2);
SoftwareSerial BT(10, 11); //TX, RX respetively

UltraSonicDistanceSensor distanceSensor(9, 8);  // Initialize sensor that uses digital pins 9 and 10.
const int relay=12;
void setup () {
    Serial.begin(9600);  // We initialize serial connection so that we could print values from sensor.
    BT.begin(9600);// We initialize bluetooth module.
    lcd.init();// We initialize LCD 16*2.
    lcd.backlight();
}
//E-innovators
void loop () {
 
    // Every 1 SECOND, do a measurement using the sensor and print the distance in centimeters.
    int distance=distanceSensor.measureDistanceCm();
    if(distance<0){
      distance=0;
      }
    Serial.println(distance);

   BT.println(distance);
   lcd.setCursor(0,0);
   lcd.print("Distance :");
   lcd.setCursor(0,1);
   lcd.print(distance);
    delay(1000);
    lcd.setCursor(0,1);
    
   lcd.print("                      ");
    
    
   
    }
