#include <Wire.h>

//LiquidTWI2.h:
#include <inttypes.h>
#include "Print.h"

// for memory-constrained projects, comment out the MCP230xx that doesn't apply
#define MCP23008 // Adafruit I2C Backpack

// if DETECT_DEVICE is enabled, then when constructor's detectDevice != 0
// device will be detected in the begin() function...
// if the device isn't detected in begin() then we won't try to talk to the
// device in any of the other functions... this allows you to compile the
// code w/o an LCD installed, and not get hung in the write functions
#define DETECT_DEVICE // enable device detection code

#define MCP23008_ADDRESS 0x20

// registers
#define MCP23008_IODIR 0x00
#define MCP23008_IPOL 0x01
#define MCP23008_GPINTEN 0x02
#define MCP23008_DEFVAL 0x03
#define MCP23008_INTCON 0x04
#define MCP23008_IOCON 0x05
#define MCP23008_GPPU 0x06
#define MCP23008_INTF 0x07
#define MCP23008_INTCAP 0x08
#define MCP23008_GPIO 0x09
#define MCP23008_OLAT 0x0A

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_BACKLIGHT 0x08 // used to pick out the backlight flag since _displaycontrol will never set itself

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
//we only support 4-bit mode #define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

class LiquidTWI2 : public Print {
public:
LiquidTWI2(uint8_t i2cAddr,uint8_t detectDevice=0,uint8_t backlightInverted=0);

void begin(uint8_t cols, uint8_t rows,uint8_t charsize = LCD_5x8DOTS);

#ifdef DETECT_DEVICE
uint8_t LcdDetected() { return _deviceDetected; }
#endif // DETECT_DEVICE
void clear();
void home();

void noDisplay();
void display();
void noBlink();
void blink();
void noCursor();
void cursor();
void scrollDisplayLeft();
void scrollDisplayRight();
void leftToRight();
void rightToLeft();
void autoscroll();
void noAutoscroll();

void setBacklight(uint8_t status);

void createChar(uint8_t, uint8_t[]);
void setCursor(uint8_t, uint8_t);
#if defined(ARDUINO) && (ARDUINO >= 100) // scl
virtual size_t write(uint8_t);
#else
virtual void write(uint8_t);
#endif
void command(uint8_t);

private:
void send(uint8_t, uint8_t);
void burstBits8(uint8_t);

uint8_t _displayfunction;
uint8_t _displaycontrol;
uint8_t _displaymode;
uint8_t _numlines,_currline;
uint8_t _i2cAddr;
uint8_t _backlightInverted;
#ifdef DETECT_DEVICE
uint8_t _deviceDetected;
#endif // DETECT_DEVICE

};



//====================================================
//LiquidTWI2.cpp: 


/*
  LiquidTWI2 High Performance i2c LCD driver for MCP23008 & MCP23017
  hacked by Sam C. Lin / http://www.lincomatic.com
  from 
   LiquidTWI by Matt Falcon (FalconFour) / http://falconfour.com
   logic gleaned from Adafruit RGB LCD Shield library
   Panelolu2 support by Tony Lock / http://blog.think3dprint3d.com
   enhancements by Nick Sayer / https://github.com/nsayer

  Compatible with Adafruit I2C LCD backpack (MCP23008) and
  Adafruit RGB LCD Shield
*/

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <Wire.h>
#if defined(ARDUINO) && (ARDUINO >= 100) //scl
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

static inline void wiresend(uint8_t x) {
#if ARDUINO >= 100
  Wire.write((uint8_t)x);
#else
  Wire.send(x);
#endif
}

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set: 
//    DL = 0; 4-bit interface data 
//    N = 0; 1-line display 
//    F = 0; 5x8 dot character font 
// 3. Display on/off control: 
//    D = 0; Display off 
//    C = 0; Cursor off 
//    B = 0; Blinking off 
// 4. Entry mode set: 
//    I/D = 1; Increment by 1 
//    S = 0; No shift 
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// LiquidTWI2 constructor is called). This is why we save the init commands
// for when the sketch calls begin(), except configuring the expander, which
// is required by any setup.

LiquidTWI2::LiquidTWI2(uint8_t i2cAddr,uint8_t detectDevice, uint8_t backlightInverted) {
  // if detectDevice != 0, set _deviceDetected to 2 to flag that we should
  // scan for it in begin()
#ifdef DETECT_DEVICE
  _deviceDetected = detectDevice ? 2 : 1;
#endif

  _backlightInverted = backlightInverted;

  //  if (i2cAddr > 7) i2cAddr = 7;
  _i2cAddr = i2cAddr; // transfer this function call's number into our internal class state
  _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS; // in case they forget to call begin() at least we have something
}

void LiquidTWI2::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {
  // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
  // according to datasheet, we need at least 40ms after power rises above 2.7V
  // before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
  delay(50);

  Wire.begin();

  uint8_t result;

    
    // now we set the GPIO expander's I/O direction to output since it's soldered to an LCD output.
    Wire.beginTransmission(MCP23008_ADDRESS | _i2cAddr);
    wiresend(MCP23008_IODIR);
    wiresend(0x00); // all output: 00000000 for pins 1...8
    result = Wire.endTransmission();
#ifdef DETECT_DEVICE
    if (result) {
        if (_deviceDetected == 2) {
          _deviceDetected = 0;
          return;
        }
    }
#endif 

#ifdef DETECT_DEVICE
  // If we haven't failed by now, then we pass
  if (_deviceDetected == 2) _deviceDetected = 1;
#endif

  if (lines > 1) {
    _displayfunction |= LCD_2LINE;
  }
  _numlines = lines;
  _currline = 0;

  //put the LCD into 4 bit mode
  // start with a non-standard command to make it realize we're speaking 4-bit here
  // per LCD datasheet, first command is a single 4-bit burst, 0011.
  //-----
  //  we cannot assume that the LCD panel is powered at the same time as
  //  the arduino, so we have to perform a software reset as per page 45
  //  of the HD44780 datasheet - (kch)
  //-----
    // bit pattern for the burstBits function is
    //
    //  7   6   5   4   3   2   1   0
    // LT  D7  D6  D5  D4  EN  RS  n/c
    //-----
    burstBits8(B10011100); // send LITE D4 D5 high with enable
    burstBits8(B10011000); // send LITE D4 D5 high with !enable
    burstBits8(B10011100); //
    burstBits8(B10011000); //
    burstBits8(B10011100); // repeat twice more
    burstBits8(B10011000); //
    burstBits8(B10010100); // send D4 low and LITE D5 high with enable
    burstBits8(B10010000); // send D4 low and LITE D5 high with !enable

  delay(5); // this shouldn't be necessary, but sometimes 16MHz is stupid-fast.

  command(LCD_FUNCTIONSET | _displayfunction); // then send 0010NF00 (N=lines, F=font)
  delay(5); // for safe keeping...
  command(LCD_FUNCTIONSET | _displayfunction); // ... twice.
  delay(5); // done!

  // turn on the LCD with our defaults. since these libs seem to use personal preference, I like a cursor.
  _displaycontrol = (LCD_DISPLAYON|LCD_BACKLIGHT);
  display();
  // clear it off
  clear();

  _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
  // set the entry mode
  command(LCD_ENTRYMODESET | _displaymode);
}

/********** high level commands, for the user! */
void LiquidTWI2::clear()
{
#ifdef DETECT_DEVICE
  if (!_deviceDetected) return;
#endif
  command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
  delayMicroseconds(2000);  // this command takes a long time!
}

void LiquidTWI2::home()
{
#ifdef DETECT_DEVICE
  if (!_deviceDetected) return;
#endif
  command(LCD_RETURNHOME);  // set cursor position to zero
  delayMicroseconds(2000);  // this command takes a long time!
}

void LiquidTWI2::setCursor(uint8_t col, uint8_t row)
{
#ifdef DETECT_DEVICE
  if (!_deviceDetected) return;
#endif
  int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
  if ( row > _numlines ) row = _numlines - 1;    // we count rows starting w/0
  command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

// Turn the display on/off (quickly)
void LiquidTWI2::noDisplay() {
#ifdef DETECT_DEVICE
  if (!_deviceDetected) return;
#endif
  _displaycontrol &= ~LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LiquidTWI2::display() {
#ifdef DETECT_DEVICE
  if (!_deviceDetected) return;
#endif
  _displaycontrol |= LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void LiquidTWI2::noCursor() {
#ifdef DETECT_DEVICE
  if (!_deviceDetected) return;
#endif
  _displaycontrol &= ~LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LiquidTWI2::cursor() {
#ifdef DETECT_DEVICE
  if (!_deviceDetected) return;
#endif
  _displaycontrol |= LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void LiquidTWI2::noBlink() {
#ifdef DETECT_DEVICE
  if (!_deviceDetected) return;
#endif
  _displaycontrol &= ~LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LiquidTWI2::blink() {
#ifdef DETECT_DEVICE
  if (!_deviceDetected) return;
#endif
  _displaycontrol |= LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void LiquidTWI2::scrollDisplayLeft(void) {
#ifdef DETECT_DEVICE
  if (!_deviceDetected) return;
#endif
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void LiquidTWI2::scrollDisplayRight(void) {
#ifdef DETECT_DEVICE
  if (!_deviceDetected) return;
#endif
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void LiquidTWI2::leftToRight(void) {
#ifdef DETECT_DEVICE
  if (!_deviceDetected) return;
#endif
  _displaymode |= LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void LiquidTWI2::rightToLeft(void) {
#ifdef DETECT_DEVICE
  if (!_deviceDetected) return;
#endif
  _displaymode &= ~LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void LiquidTWI2::autoscroll(void) {
#ifdef DETECT_DEVICE
  if (!_deviceDetected) return;
#endif
  _displaymode |= LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void LiquidTWI2::noAutoscroll(void) {
#ifdef DETECT_DEVICE
  if (!_deviceDetected) return;
#endif
  _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void LiquidTWI2::createChar(uint8_t location, uint8_t charmap[]) {
#ifdef DETECT_DEVICE
  if (!_deviceDetected) return;
#endif
  location &= 0x7; // we only have 8 locations 0-7
  command(LCD_SETCGRAMADDR | (location << 3));
  for (int i=0; i<8; i++) {
    write(charmap[i]);
  }
}

/*********** mid level commands, for sending data/cmds */
inline void LiquidTWI2::command(uint8_t value) {
  send(value, LOW);
}
#if defined(ARDUINO) && (ARDUINO >= 100) //scl
inline size_t LiquidTWI2::write(uint8_t value) {
#ifdef DETECT_DEVICE
  if (!_deviceDetected) return 1;
#endif
  send(value, HIGH);
  return 1;
}
#else
inline void LiquidTWI2::write(uint8_t value) {
#ifdef DETECT_DEVICE
  if (!_deviceDetected) return;
#endif
  send(value, HIGH);
}
#endif

/************ low level data pushing commands **********/

// Allows to set the backlight, if the LCD backpack is used
void LiquidTWI2::setBacklight(uint8_t status) {
#ifdef DETECT_DEVICE
  if (!_deviceDetected) return;
#endif
  if (_backlightInverted) status ^= 0x7;
    bitWrite(_displaycontrol,3,status); // flag that the backlight is enabled, for burst commands
    burstBits8((_displaycontrol & LCD_BACKLIGHT)?0x80:0x00);
}

// write either command or data, burst it to the expander over I2C.
void LiquidTWI2::send(uint8_t value, uint8_t mode) {
    // BURST SPEED, OH MY GOD
    // the (now High Speed!) I/O expander pinout
    // RS pin = 1
    // Enable pin = 2
    // Data pin 4 = 3
    // Data pin 5 = 4
    // Data pin 6 = 5
    // Data pin 7 = 6
    byte buf;
    // crunch the high 4 bits
    buf = (value & B11110000) >> 1; // isolate high 4 bits, shift over to data pins (bits 6-3: x1111xxx)
    if (mode) buf |= 3 << 1; // here we can just enable enable, since the value is immediately written to the pins
    else buf |= 2 << 1; // if RS (mode), turn RS and enable on. otherwise, just enable. (bits 2-1: xxxxx11x)
    buf |= (_displaycontrol & LCD_BACKLIGHT)?0x80:0x00; // using DISPLAYCONTROL command to mask backlight bit in _displaycontrol
    burstBits8(buf); // bits are now present at LCD with enable active in the same write
    // no need to delay since these things take WAY, WAY longer than the time required for enable to settle (1us in LCD implementation?)
    buf &= ~(1<<2); // toggle enable low
    burstBits8(buf); // send out the same bits but with enable low now; LCD crunches these 4 bits.
    // crunch the low 4 bits
    buf = (value & B1111) << 3; // isolate low 4 bits, shift over to data pins (bits 6-3: x1111xxx)
    if (mode) buf |= 3 << 1; // here we can just enable enable, since the value is immediately written to the pins
    else buf |= 2 << 1; // if RS (mode), turn RS and enable on. otherwise, just enable. (bits 2-1: xxxxx11x)
    buf |= (_displaycontrol & LCD_BACKLIGHT)?0x80:0x00; // using DISPLAYCONTROL command to mask backlight bit in _displaycontrol
    burstBits8(buf);
    buf &= ~( 1 << 2 ); // toggle enable low (1<<2 = 00000100; NOT = 11111011; with "and", this turns off only that one bit)
    burstBits8(buf);
}

void LiquidTWI2::burstBits8(uint8_t value) {
  // we use this to burst bits to the GPIO chip whenever we need to. avoids repetitive code.
  Wire.beginTransmission(MCP23008_ADDRESS | _i2cAddr);
  wiresend(MCP23008_GPIO);
  wiresend(value); // last bits are crunched, we're done.
  while(Wire.endTransmission());
}



//user code: ========================================================


#include <Keypad.h>
#include <Servo.h>
#include <EEPROM.h>


int Button=10;             //Push Button

const byte numRows= 4;          //number of rows on the keypad
const byte numCols= 4;          //number of columns on the keypad

char keymap[numRows][numCols]= 
{
{'1', '2', '3', 'A'}, 
{'4', '5', '6', 'B'}, 
{'7', '8', '9', 'C'},
{'*', '0', '#', 'D'}
};

char keypressed;                 //Where the keys are stored it changes very often
char code[]= {'1','2','3','4'};  //The default code, you can change it or make it a 'n' digits one

char check1[sizeof(code)];  //Where the new key is stored
char check2[sizeof(code)];  //Where the new key is stored again so it's compared to the previous one

short a=0,i=0,s=0,j=0;          //Variables used later

byte rowPins[numRows] = {9,8,7,6}; //Rows 0 to 3 //if you modify your pins you should modify this too
byte colPins[numCols]= {5,4,3,2}; //Columns 0 to 3

LiquidTWI2 lcd(0x20,16,2);
Keypad myKeypad= Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);
Servo myservo;
void setup()
         {
          lcd.begin (16,2);
          lcd.print("Standby");      //What's written on the LCD you can change
          pinMode(Button,INPUT);
          myservo.attach(11);
          myservo.write(0); 
         //  for(i=0 ; i<sizeof(code);i++){        //When you upload the code the first time keep it commented
//            EEPROM.get(i, code[i]);             //Upload the code and change it to store it in the EEPROM
//           }                                  //Then uncomment this for loop and reupload the code (It's done only once) 
                      

         }


void loop()
{

  keypressed = myKeypad.getKey();               //Constantly waiting for a key to be pressed
    if(keypressed == '*'){                      // * to open the lock
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Enter code");            //Message to show
            ReadCode();                          //Getting code function
                  if(a==sizeof(code))           //The ReadCode function assign a value to a (it's correct when it has the size of the code array)
                  OpenDoor();                   //Open lock function if code is correct
                  else{
                  lcd.clear();               
                  lcd.print("Wrong");          //Message to print when the code is wrong
                  }
            delay(2000);
            lcd.clear();
            lcd.print("Standby");             //Return to standby mode it's the message do display when waiting
        }

     if(keypressed == '#'){                  //To change the code it calls the changecode function
      ChangeCode();
      lcd.clear();
      lcd.print("Standby");                 //When done it returns to standby mode
     }

     if(digitalRead(Button)==HIGH){      //Opening by the push button
      myservo.write(0);     
     }
         
}

void ReadCode(){                  //Getting code sequence
       i=0;                      //All variables set to 0
       a=0;
       j=0; 
                                     
     while(keypressed != 'A'){                                     //The user press A to confirm the code otherwise he can keep typing
           keypressed = myKeypad.getKey();                         
             if(keypressed != NO_KEY && keypressed != 'A' ){       //If the char typed isn't A and neither "nothing"
              lcd.setCursor(j,1);                                  //This to write "*" on the LCD whenever a key is pressed it's position is controlled by j
              lcd.print("*");
              j++;
            if(keypressed == code[i]&& i<sizeof(code)){            //if the char typed is correct a and i increments to verify the next caracter
                 a++;                                              
                 i++;
                 }
            else
                a--;                                               //if the character typed is wrong a decrements and cannot equal the size of code []
            }
            }
    keypressed = NO_KEY;

}

void ChangeCode(){                      //Change code sequence
      lcd.clear();
      lcd.print("Changing code");
      delay(1000);
      lcd.clear();
      lcd.print("Enter old code");
      ReadCode();                      //verify the old code first so you can change it
      
            if(a==sizeof(code)){      //again verifying the a value
            lcd.clear();
            lcd.print("Changing code");
            GetNewCode1();            //Get the new code
            GetNewCode2();            //Get the new code again to confirm it
            s=0;
              for(i=0 ; i<sizeof(code) ; i++){     //Compare codes in array 1 and array 2 from two previous functions
              if(check1[i]==check2[i])
              s++;                                //again this how we verifiy, increment s whenever codes are matching
              }
                  if(s==sizeof(code)){            //Correct is always the size of the array
                  
                   for(i=0 ; i<sizeof(code) ; i++){
                  code[i]=check2[i];         //the code array now receives the new code
                  EEPROM.put(i, code[i]);        //And stores it in the EEPROM
                  
                  }
                  lcd.clear();
                  lcd.print("Code Changed");
                  delay(2000);
                  }
                  else{                         //In case the new codes aren't matching
                  lcd.clear();
                  lcd.print("Codes are not");
                  lcd.setCursor(0,1);
                  lcd.print("matching !!");
                  delay(2000);
                  }
            
          }
          
          else{                     //In case the old code is wrong you can't change it
          lcd.clear();
          lcd.print("Wrong");
          delay(2000);
          }
}

void GetNewCode1(){                      
  i=0;
  j=0;
  lcd.clear();
  lcd.print("Enter new code");   //tell the user to enter the new code and press A
  lcd.setCursor(0,1);
  lcd.print("and press A");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("and press A");     //Press A keep showing while the top row print ***
             
         while(keypressed != 'A'){            //A to confirm and quits the loop
             keypressed = myKeypad.getKey();
               if(keypressed != NO_KEY && keypressed != 'A' ){
                lcd.setCursor(j,0);
                lcd.print("*");               //On the new code you can show * as I did or change it to keypressed to show the keys
                check1[i]=keypressed;     //Store caracters in the array
                i++;
                j++;                    
                }
                }
keypressed = NO_KEY;
}

void GetNewCode2(){                         //This is exactly like the GetNewCode1 function but this time the code is stored in another array
  i=0;
  j=0;
  
  lcd.clear();
  lcd.print("Confirm code");
  lcd.setCursor(0,1);
  lcd.print("and press A");
  delay(3000);
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("and press A");

         while(keypressed != 'A'){
             keypressed = myKeypad.getKey();
               if(keypressed != NO_KEY && keypressed != 'A' ){
                lcd.setCursor(j,0);
                lcd.print("*");
                check2[i]=keypressed;
                i++;
                j++;                    
                }
                }
keypressed = NO_KEY;
}

void OpenDoor(){             //Lock opening function open for 3s
  lcd.clear();
  lcd.print("Welcome");
  myservo.write(90);
  
  
  }
