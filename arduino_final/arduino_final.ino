

#include <Keypad.h>
#include <Servo.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>

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

LiquidCrystal_I2C lcd(0x27,16,2);
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
