/* Copyrigth Tor Design 2020 - 2021  
 *  
 *  GNU General Public License v3.0
 *  Permissions of this strong copyleft license are conditioned on making available complete 
 *  source code of licensed works and modifications, which include larger works using a licensed 
 *  work, under the same license. Copyright and license notices must be preserved. Contributors 
 *  provide an express grant of patent rights.
 *  
 *   ... See full version as GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 *  
 *  Thank you to all the backers on kickstarter that supported the project on kickstarter
 *  during the spring of 2020. 
 */ 
//------------------------------ INCLUDES ---------------------------------------------------------
#include <Wire.h>
#include <RTClib.h>
#include <SparkFunBME280.h>
#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
#include <avr/power.h>
#endif

#define PIN            12 
#define NUMPIXELS      4

// --------------------- GLOBAL SETTINGS ---------------------------------------------------------------
int a = 0;
int b = 0;
int c = 0;
int d = 0;
int timeMODE = 24;  // default mode = 24h, set to 12 in settings for 12h mode.
int tubeSaving = 0; // default mode 0 = off,  1 = on
int savingTime = 0; 

int temperatureMode = 1;  // 1 = For Celsius 
                          // 2 = For Farenheit                       
const int ledPin = 26; 
const int enableHV = 18; 
const int Btn1 = 35; 
const int Btn2 = 34;
int R = 0; 
int G = 20; 
int B = 25; 
int cnt = 50;  
                       
//----------------------------- ESP32 pins to shift register --------------------------------
const int latchPin = 32;   // pin 12 on the 74hc595   ESP - 32      ST_CP
const int dataPin = 25;    // pin 14 on the 74hc595   ESP - 25      DS_pin
const int clockPin = 33;   // pin 11 on the 74hc595   ESP - 33      SH_CP

//------------------------------ For RTC and BMP280 sensor ---------------------------------------------
RTC_DS3231 rtc;
BME280 mySensor;

//---------------------------- Neopixel adressable LEDs ------------------------------------------------
// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel strip(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

//--------------------- SETUP AND  INITIALIZATION ------------------------------------------------------
void setup() { 
  Serial.begin(9600); 
  
  pinMode(Btn1, INPUT); 
  pinMode(Btn2, INPUT); 
  pinMode(enableHV, OUTPUT);
  digitalWrite(enableHV, LOW); // Pull the enable pin low in order to power up the high voltage power supply

  pinMode(ledPin,   OUTPUT);
  digitalWrite(ledPin, HIGH);    
  // Set the ShiftRegister Pins as output
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin,  OUTPUT);

  Wire.begin(21,22,400000); // SDA,SCL,frequenzy RTC clock

  strip.begin(); // This initializes the NeoPixel LED library

  mySensor.setI2CAddress(0x76); 
  if(mySensor.beginI2C() == false) Serial.println("Bosh sensor connect failed");
  
  Serial.println(" ");
  Serial.println("NIXLER by Tor Design NORWAY");
  Serial.println("Thank you to all the backers that supported the project on Kickstarter during the spring of 2020.");
  Serial.println("Head over to www.tordesign.net for more information and guides.");
  Serial.println("Software version 1.2");
}

//----------------------- MAIN LOOP ---------------------------------------------------------------------------------
void loop(){
 DateTime now = rtc.now();
 int HOU1 = now.hour();
 int MIN1 = now.minute();
 int SEC1 = now.second();
 int MON1 = now.month();
 int DAY1 = now.day();
 int YEA1 = now.year();
 Serial.println("-------------------------------------------------");
 Serial.println("NIXLER by Tor Design - Software version: 1.2");
 Serial.print("Time: ");
 Serial.print(HOU1);
 Serial.print(":");
 Serial.print(MIN1);
 Serial.print(":");
 Serial.println(SEC1);
 Serial.print("Date: ");
 Serial.print(DAY1);
 Serial.print("/");
 Serial.print(MON1);
 Serial.print("/");
 Serial.println(YEA1);
 Serial.print("Temperature celsius: ");
 Serial.println(mySensor.readTempC(),2);
 Serial.print("Temperature fahrenheit: ");
 Serial.println(mySensor.readTempF(),2);
 Serial.print("Pressure: ");
 Serial.println(mySensor.readFloatPressure());
  
 savingTime = 0;
 if (HOU1 > 0 && HOU1 < 7){ // Check to see if its time for tube saving (night). 
      savingTime = 1;       // If the "tubeSaving" setting is set to on  
 }                          // between 0100 and 0700 the display will shut down.

 
 if(savingTime == 1 && tubeSaving == 1){ 
          // Disable HV power supply and LEDs 
          digitalWrite(enableHV, HIGH); 
          strip.setPixelColor(0,0,0,0); // This is the left most LED on the clock
          strip.setPixelColor(1,0,0,0); 
          strip.show(); // Send the updated pixel color to hardware.
          strip.setPixelColor(2,0,0,0);
          strip.setPixelColor(3,0,0,0); // This is the right most LED on the clock
          strip.show(); // Send the updated pixel color to hardware
      } 
 else{    // Enable HV power supply and LEDs
          digitalWrite(enableHV, LOW); // Enable HV power supply 
          strip.setPixelColor(0,R,G,B); // This is the left most LED on the clock
          strip.setPixelColor(1,R,G,B); 
          strip.show(); // Send the updated pixel color to hardware.
          strip.setPixelColor(2,R,G,B);
          strip.setPixelColor(3,R,G,B); // This is the right most LED on the clock
          strip.show(); // Send the updated pixel color to hardware
     }
     
  digitalWrite(ledPin, HIGH);   
  int i = 0; 
  while(i < 50){
      dispSecond(); // Display seconds 
      checkButtons();
      delay(100);
      i++;
  }
  
  digitalWrite(ledPin, LOW);     
  int n = 0;
  while(n < 50){
      dispHourMinute(timeMODE); // Displaying hour and minute (hh:mm) in either 24h or 12h format = timeMODE
      checkButtons();
      delay(100);
      n++;
   }
   
   cycleNixler(1); // Cycle through all numbers 2x, once   
    
}//---------- END MAIN LOOP -----------------------------------------------------------------------------------------

//----------- START FUNCTIONS -----------------------------------------------------------------------------------------

// Name: dispNixie 
// Summmary: function for displaying all possible values on the four nixies:
 /* For the TorDesign NIXLER display we have 2x 20 bit shiftregisters 
    incorporated in the two HV drivers (HV5812). These registers are controlled by the ESP32
    in  order to controll digits in the 4 nixe-tubes. If the bit in the register is set to 1 the coresponding nixie-tube wil
    not light up, if its set to 0 it will. 

    In total we send 40 bit from the ESP32 to the shift registers everytime we update the display
    
    [00000   00000   00000  00000   00000   00000   00000   00000] 
         NIX4            NIX3           NIX2            NIX1 
         Register 2      Register 2     Register 1      Register 1
         HV driver 2     HV driver 2    HV driver 1     HV driver 1
  */ 
void dispNixler(int Nixie4, int Nixie3, int Nixie2, int Nixie1){  

    
    int bitArray[] = { 
                       1,1,1,1,1,1,1,1,1,1, // NixieTube one      A zero in the array activates the nixie number 1->9->0 from left to right. 
                       1,1,1,1,1,1,1,1,1,1, // NixieTube two      first row is the leftmost nixie tube.
                       1,1,1,1,1,1,1,1,1,1, // NixieTube three
                       1,1,1,1,1,1,1,1,1,1  // NixieTube four    
                      }; 
                     
   // Manipulate the bitArray to light up a certain number in the first nixietube (NIXIE ONE)      
        if ( Nixie1 == 1){
          bitArray[0] = 0; 
        }
         if ( Nixie1 == 2){
          bitArray[1] = 0; 
        }
         if ( Nixie1 == 3){
          bitArray[2] = 0; 
        }
         if ( Nixie1 == 4){
          bitArray[3] = 0; 
        }
         if ( Nixie1 == 5){
          bitArray[4] = 0; 
        }
         if ( Nixie1 == 6){
          bitArray[5] = 0; 
        }
         if ( Nixie1 == 7){
          bitArray[6] = 0; 
        }
         if ( Nixie1 == 8){
          bitArray[7] = 0; 
        }
         if ( Nixie1 == 9){
          bitArray[8] = 0; 
        }
         if ( Nixie1 == 0){
          bitArray[9] = 0; 
        }
                                                      
    //NIXIE TWO: 
       if ( Nixie2 == 1){
          bitArray[10] = 0; 
        }
         if ( Nixie2 == 2){
          bitArray[11] = 0; 
        }
         if ( Nixie2 == 3){
          bitArray[12] = 0; 
        }
         if ( Nixie2 == 4){
          bitArray[13] = 0; 
        }
         if ( Nixie2 == 5){
          bitArray[14] = 0; 
        }
         if ( Nixie2 == 6){
          bitArray[15] = 0; 
        }
         if ( Nixie2 == 7){
          bitArray[16] = 0; 
        }
         if ( Nixie2 == 8){
          bitArray[17] = 0; 
        }
         if ( Nixie2 == 9){
          bitArray[18] = 0; 
        }
         if ( Nixie2 == 0){
          bitArray[19] = 0; 
        }

    // NIXIE THREE:  
       if ( Nixie3 == 1){
          bitArray[20] = 0; 
        }
         if ( Nixie3 == 2){
          bitArray[21] = 0; 
        }
         if ( Nixie3 == 3){
          bitArray[22] = 0; 
        }
         if ( Nixie3 == 4){
          bitArray[23] = 0; 
        }
         if ( Nixie3 == 5){
          bitArray[24] = 0; 
        }
         if ( Nixie3 == 6){
          bitArray[25] = 0; 
        }
         if ( Nixie3 == 7){
          bitArray[26] = 0; 
        }
         if ( Nixie3 == 8){
          bitArray[27] = 0; 
        }
         if ( Nixie3 == 9){
          bitArray[28] = 0; 
        }
         if ( Nixie3 == 0){
          bitArray[29] = 0; 
        }

   // NIXIE FOUR:  
       if ( Nixie4 == 1){
          bitArray[30] = 0; 
        }
         if ( Nixie4 == 2){
          bitArray[31] = 0; 
        }
         if ( Nixie4 == 3){
          bitArray[32] = 0; 
        }
         if ( Nixie4 == 4){
          bitArray[33] = 0; 
        }
         if ( Nixie4 == 5){
          bitArray[34] = 0; 
        }
         if ( Nixie4 == 6){
          bitArray[35] = 0; 
        }
         if ( Nixie4 == 7){
          bitArray[36] = 0; 
        }
         if ( Nixie4 == 8){
          bitArray[37] = 0; 
        }
         if ( Nixie4 == 9){
          bitArray[38] = 0; 
        }
         if ( Nixie4 == 0){
          bitArray[39] = 0; 
        }
                    
   // Send the updated BitArray to the HV5812 shiftregisters by bitbanging 
           
   digitalWrite(latchPin, LOW); // Ground latchPin and hold low for as long as we are transmitting
   
   for (int i = 0; i < 40; i++){
        int bitVal = bitArray[i];
        digitalWrite(clockPin, LOW);
        digitalWrite(dataPin, bitVal);   
        digitalWrite(clockPin, HIGH);
        }
   digitalWrite(latchPin, HIGH); // Return latch pin high signaling to chip that sequence is done.  

} // END dispNixie

//---------------------------------------------------------------------------------------------------
// Name: setupNixie 
// Summmary: function for setting the time, 12/24 hour format, day/date/month, year, fahrenheit/celsius, 
//           tube saving mode and LED color.   

int setupNixler(int Btn1, int Btn2){
      digitalWrite(enableHV, LOW); // enable HV power supply
      bool Btn1_state = digitalRead(Btn1);
      bool Btn2_state = digitalRead(Btn2); 
      int timeMode;
      int hourMode1 = 2;
      int hourMode2 = 4;
      int day1 = 0;
      int day2 = 1;
      int month1 = 0;
      int month2 = 1;
      int R_setting = 120;
      int G_setting = 120;
      int B_setting = 120;
      
      Serial.println("Entered setting mode...");
      a = 0;
      b = 0;
      c = 0;
      d = 0;    
      
      bool Btn1_state_new = digitalRead(Btn1);
      bool Btn2_state_new = digitalRead(Btn2);

      // Wait while the buttons are pressed 
      while(Btn1_state_new == Btn1_state || Btn2_state_new == Btn2_state){
          Btn1_state_new = digitalRead(Btn1);
          Btn2_state_new = digitalRead(Btn2);
          dispNixler(0,0,0,0);
          delay(300);
          dispNixler(3,3,3,3);
          delay(300);
      }
      
      // ---- SET HOUR ------------------------------------------------------
      strip.setPixelColor(0,R_setting,G_setting,B_setting); // This is the left LED on the clock
      strip.setPixelColor(1,R_setting,G_setting,B_setting); // 
      strip.show(); // This sends the updated pixel color to the hardware.
      strip.setPixelColor(2,0,0,0); //
      strip.setPixelColor(3,0,0,0); // This is the right LED on the clock
      strip.show(); // This sends the updated pixel color to the hardware. 
      while(digitalRead(Btn2) == LOW){
          dispNixler(a,b,10,10);
          bool btn_1_state_1 = digitalRead(Btn1);
          
          if(btn_1_state_1 == HIGH){
            b++; 
            if(b > 9){
              a++;
              b = 0;
            }
            if(a == 2 && b > 3){
              a = 0;
              b = 0;
            }
            delay(200);
            bool btn_1_state_1 = digitalRead(Btn1);
          }    
      }
      
      // ----- DEBOUNCE ---------------------------------------------------------
      while(digitalRead(Btn2) == HIGH){ 
        // ... Wait for release of button 2 and move to next
        delay(50); // Debounce delay
      }
      
      // ----- SET MINUTES ------------------------------------------------------
      strip.setPixelColor(0,0,0,0); // This is the left LED on the clock
      strip.setPixelColor(1,0,0,0); // 
      strip.show(); // This sends the updated pixel color to the hardware.
      strip.setPixelColor(2,R_setting,G_setting,B_setting); //
      strip.setPixelColor(3,R_setting,G_setting,B_setting); // This is the right LED on the clock
      strip.show(); // This sends the updated pixel color to the hardware.
       
      while(digitalRead(Btn2) == LOW){
          dispNixler(a,b,c,d);
          bool btn_1_state_1 = digitalRead(Btn1);
          
          if(btn_1_state_1 == HIGH){
            d++; 
            if(d > 9){
              c++;
              d = 0;
            }
            if(c > 5){
              c = 0;
              d = 0;
            }
            delay(200);
          } 
          
      }
      // ----- DEBOUNCE ------------------------------------------------------------
      while(digitalRead(Btn2) == HIGH){
          // ... Wait for release of button 2 and move to next
          delay(50); // Debounce delay
      }

      // ----- SET 12/24 HOUR ------------------------------------------------------
      strip.setPixelColor(0,0,0,0); // This is the left LED on the clock
      strip.setPixelColor(1,R_setting,G_setting,B_setting); // 
      strip.show(); // This sends the updated pixel color to the hardware.
      strip.setPixelColor(2,R_setting,G_setting,B_setting); //
      strip.setPixelColor(3,0,0,0); // This is the right LED on the clock
      strip.show(); // This sends the updated pixel color to the hardware.
            
      while(digitalRead(Btn2) == LOW){  
        
          dispNixler(10,hourMode1,hourMode2,10);
          bool btn_1_state_1 = digitalRead(Btn1);
          
          if(btn_1_state_1 == HIGH){
            if(hourMode1 == 2){
              hourMode1 = 1;
              hourMode2 = 2;
              timeMODE = 12; // 12h mode saved in variable
            }
            else{
              hourMode1 = 2;
              hourMode2 = 4;
              timeMODE = 24; // 24h mode saved in variable
            }
            delay(200);
          }
      }
      // -------------- DEBOUNCE -----------------------------------------------------
      while(digitalRead(Btn2) == HIGH){
          // ... Wait for release of button 2 and move to next
          delay(50); // Debounce delay
      }

      // -------------- SET DAY ------------------------------------------------------
      strip.setPixelColor(0,R_setting,G_setting,B_setting); // This is the left LED on the clock
      strip.setPixelColor(1,R_setting,G_setting,B_setting); // 
      strip.show(); // This sends the updated pixel color to the hardware.
      strip.setPixelColor(2,0,0,0); //
      strip.setPixelColor(3,0,0,0); // This is the right LED on the clock
      strip.show(); // This sends the updated pixel color to the hardware.
            
      while(digitalRead(Btn2) == LOW){
          dispNixler(day1,day2,10,10);
          bool btn_1_state_1 = digitalRead(Btn1);
          
          if(btn_1_state_1 == HIGH){
            day2++; 
            if(day2 > 9){
              day1++;
              day2 = 0;
            }
            if(day1 == 3 && day2 > 1){
              day1 = 0;
              day2 = 1;
            }
            delay(200);
          }    
      }
    
      // ----- DEBOUNCE ------------------------------------------------------------
      while(digitalRead(Btn2) == HIGH){
          // ... Wait for release of button 2 and move to next
          delay(50); // Debounce delay
      }

      // -------------- SET MONTH ------------------------------------------------------
      strip.setPixelColor(0,R_setting,G_setting,B_setting); // This is the left LED on the clock
      strip.setPixelColor(1,R_setting,G_setting,B_setting); // 
      strip.show(); // This sends the updated pixel color to the hardware.
      strip.setPixelColor(2,R_setting,G_setting,B_setting); //
      strip.setPixelColor(3,R_setting,G_setting,B_setting); // This is the right LED on the clock
      strip.show(); // This sends the updated pixel color to the hardware.
      
      while(digitalRead(Btn2) == LOW){
          dispNixler(day1,day2,month1,month2);
          bool btn_1_state_1 = digitalRead(Btn1);
          
          if(btn_1_state_1 == HIGH){
            month2++; 
            if(month2 > 9){
              month1++;
              month2 = 0;
            }
            if(month1 == 1 && month2 > 2){
              month1 = 0;
              month2 = 1;
            }
            delay(200);
            }    
      }
      
      // ----- DEBOUNCE ------------------------------------------------------------
      while(digitalRead(Btn2) == HIGH){
          // ... Wait for release of button 2 and move to next
          delay(50); // Debounce delay
      }
      
      // -------------- SET YEAR ------------------------------------------------------
      bool yearState = true;
      int year1 = 0;
      int year2 = 0;
      int year3 = 0;
      int year4 = 0;
      
      while(yearState == true){
          // Set the Most Significant Digit (MSD) in the year
              strip.setPixelColor(0,R_setting,G_setting,B_setting); // This is the left LED on the clock
              strip.setPixelColor(1,0,0,0); // 
              strip.show(); // This sends the updated pixel color to the hardware.
              strip.setPixelColor(2,0,0,0); //
              strip.setPixelColor(3,0,0,0); // This is the right LED on the clock
              strip.show(); // This sends the updated pixel color to the hardware.           
          while(digitalRead(Btn2) == LOW){
              dispNixler(year1,year2,year3,year4);                       
              if(digitalRead(Btn1) == HIGH){
                  year1++; 
                  if(year1 > 9){
                      year1 = 0;
                  }
                  delay(200);
              }
          }
          while(digitalRead(Btn2) == HIGH){ // Debounce 
              //..wait
              delay(20);
          }
          
          // Set digit 2 in the year
          strip.setPixelColor(0,R_setting,G_setting,B_setting); // This is the left LED on the clock
          strip.setPixelColor(1,R_setting,G_setting,B_setting); // 
          strip.show(); // This sends the updated pixel color to the hardware.
          strip.setPixelColor(2,0,0,0); // 
          strip.setPixelColor(3,0,0,0); //     
          strip.show();  
                
          while(digitalRead(Btn2) == LOW){
              dispNixler(year1,year2,year3,year4);                    
              if(digitalRead(Btn1) == HIGH){
                  year2++; 
                  if(year2 > 9){
                      year2 = 0;
                  }
                  delay(200);
              }            
          } 
          while(digitalRead(Btn2) == HIGH){ // Debounce 
              //..wait
              delay(20);
          }
           // Set digit 3 in the year
          strip.setPixelColor(0,R_setting,G_setting,B_setting); // This is the left LED on the clock
          strip.setPixelColor(1,R_setting,G_setting,B_setting); // 
          strip.show(); // This sends the updated pixel color to the hardware.
          strip.setPixelColor(2,R_setting,G_setting,B_setting); //
          strip.setPixelColor(3,0,0,0); //
          strip.show(); // This sends the updated pixel color to the hardware.
                       
          while(digitalRead(Btn2) == LOW){
              dispNixler(year1,year2,year3,year4);            
              if(digitalRead(Btn1) == HIGH){
                  year3++; 
                  if(year3 > 9){
                      year3 = 0;
                  }
                  delay(200);
              }
          } 
          while(digitalRead(Btn2) == HIGH){ // Debounce 
              //..wait
              delay(20);
          }
           // Set digit LSD in the year
          strip.setPixelColor(0,R_setting,G_setting,B_setting); // This is the left LED on the clock
          strip.setPixelColor(1,R_setting,G_setting,B_setting); // 
          strip.show(); // This sends the updated pixel color to the hardware.
          strip.setPixelColor(2,R_setting,G_setting,B_setting); //          
          strip.setPixelColor(3,R_setting,G_setting,B_setting); //
          strip.show(); // This sends the updated pixel color to the hardware.
           
          while(digitalRead(Btn2) == LOW){
              dispNixler(year1,year2,year3,year4);
              if(digitalRead(Btn1) == HIGH){
                  year4++; 
                  if(year4 > 9){
                      year4 = 0;
                  }
                  delay(200);
              }
          }
          yearState = false;    
      }
      
      // ----- DEBOUNCE ------------------------------------------------------------
      while(digitalRead(Btn2) == HIGH){
          // ... Wait for release of button 2 and move to next
          delay(50); // Debounce delay
      }
      
      // -------------- SET TEMPERATURE MODE --------------------------------------------------------------
      strip.setPixelColor(0,0,0,0); // This is the left LED on the clock
      strip.setPixelColor(1,R_setting,G_setting,B_setting); // 
      strip.show(); // This sends the updated pixel color to the hardware.
      strip.setPixelColor(2,R_setting,G_setting,B_setting); //          
      strip.setPixelColor(3,0,0,0); //
      strip.show(); // This sends the updated pixel color to the hardware.
      
      temperatureMode = 1; 
      while(digitalRead(Btn2) == LOW){  
          dispNixler(10,temperatureMode,temperatureMode,10);
          bool btn_1_state_1 = digitalRead(Btn1);
          
          if(btn_1_state_1 == HIGH){
            if(temperatureMode == 2){
                  temperatureMode = 1;
            }
            else{
                  temperatureMode = 2;
            }
            delay(200);
          }
      }
      // -------------- DEBOUNCE --------------------------------------------------------------------------
      while(digitalRead(Btn2) == HIGH){
          // ... Wait for release of button 2 and move to next
          delay(50); // Debounce delay
      }

      // -------------- ENABLE/DISABLE NIXIE TUBE SAVING MODE --------------------------------------------------------------
      strip.setPixelColor(0,0,0,0); // This is the left LED on the clock
      strip.setPixelColor(1,R_setting,G_setting,B_setting); // 
      strip.show(); // This sends the updated pixel color to the hardware.
      strip.setPixelColor(2,R_setting,G_setting,B_setting); //          
      strip.setPixelColor(3,0,0,0); //
      strip.show(); // This sends the updated pixel color to the hardware.

      tubeSaving = 0;
      while(digitalRead(Btn2) == LOW){  
          dispNixler(10,tubeSaving,tubeSaving,10);
          bool btn_1_state_1 = digitalRead(Btn1);
          
          if(btn_1_state_1 == HIGH){
            if(tubeSaving == 1){
                  tubeSaving = 0;
            }
            else{
                  tubeSaving = 1;
            }
            delay(200);
          }
      }

      // -------------- DEBOUNCE --------------------------------------------------------------------------
      while(digitalRead(Btn2) == HIGH){
          // ... Wait for release of button 2 and move to next
          delay(50); // Debounce delay
      }
         
      // -------------- SET LED COLOR ------------------------------------------------------
      int LED_counter = 0;
      int push = 0;
      while(digitalRead(Btn2) == LOW){  
          dispNixler(9,9,9,9);
          
          bool state1 = digitalRead(Btn1);
          delay(50); // wait for debounce
          bool state2 = digitalRead(Btn1);

          if(state1 != state2){
            // Push registered
            push++;
          }
          while(digitalRead(Btn1) == HIGH){
            //.... WAIT
          }  

          if(push == 1){ // RED
              R = 100; G = 0; B = 0;   
          }
          if(push == 2){ // GREEN
              R = 0; G = 100; B = 0;
          }
          if(push == 3){ // BLUE
              R = 0; G = 0; B = 100;
          }
          if(push == 4){ // WHITE YELLOW
              R = 100; G = 100; B = 20;
          }
          if(push == 5){ // YELLOW
              R = 100; G = 80; B = 0;
          }
          if(push == 6){ // PINK
              R = 130; G = 0; B = 110;
          }  
          if(push == 7){ // WHITE
              R = 100; G = 100; B = 100;
          } 
          if(push == 8){ // TURQOISE
              R = 0; G = 100; B = 100;
          }
          if(push == 9){ // ORANGE
              R = 100; G = 30; B = 0;
          }      
          if(push == 10){ // NO COLOR
              R = 0; G = 0; B = 0;
          }  
                               
          strip.setPixelColor(0,R,G,B); // This is the left LED on the clock
          strip.setPixelColor(1,R,G,B); // 
          strip.show(); // This sends the updated pixel color to the hardware.
          strip.setPixelColor(2,R,G,B); //
          strip.setPixelColor(3,R,G,B); // This is the right LED on the clock
          strip.show(); // This sends the updated pixel color to the hardware.
          
          if(push > 10){
            push = 1;
          }
      }
      
      // -------------- DEBOUNCE ------------------------------------------------------------------------
      while(digitalRead(Btn2) == HIGH){
          // ... Wait for release of button 2 and move to next
          delay(50); // Debounce delay
      }

      // -----------------------------------------------------------------------
      while(digitalRead(Btn2) == HIGH){
          // ... Wait for release of button 2 and move to next
          delay(50); // Debounce delay
      }
      
      // ------------- UPDATE RTC CHIP: set time, date and year -----------------------------------------
      
      int Nix1 = a; // The leftmost Nixie tube on the NIXLER clock  
      int Nix2 = b; 
      int Nix3 = c; 
      int Nix4 = d; 
      int SECOND = 0;
      int MINUTE;
      int HOUR;
      int DAY;
      int MONTH;
      int YEAR;
      
      // Convert  independent digits x:x:x:x to one int digit x
      MINUTE = (10*Nix3) + Nix4;
      HOUR = (10*Nix1) + Nix2;

      DAY = (10*day1) + day2;
      MONTH = (10*month1) + month2;
      YEAR = (year1*1000) + (year2*100) + (year3*10) + year4;
      
      // Pass the hour and minutes to the DS3231 RTC chip
      // Year, Month, Date
      rtc.adjust(DateTime(YEAR, MONTH, DAY, HOUR, MINUTE, SECOND));
      
      // Flash Leds to signal setup complete
      int i = 0;
      int R_end;
      int G_end;
      int B_end;
      
      while( i < 5){
          if(i == 0){
            R_end = 0;
            G_end = 0;
            B_end = 0;
          }
          if(i == 1){
            R_end = 200;
            G_end = 200;
            B_end = 200;
          }
          if(i == 2){
            R_end = 0;
            G_end = 0;
            B_end = 0;
          }
          if(i == 3){
            R_end = 200;
            G_end = 200;
            B_end = 200;
          }          
          if(i == 4){
            R_end = 0;
            G_end = 0;
            B_end = 0;
          }     
          strip.setPixelColor(0,R_end,G_end,B_end); // This is the left LED on the NIXLER clock
          strip.setPixelColor(1,R_end,G_end,B_end); 
          strip.show(); // Send the color information to the LED
          strip.setPixelColor(2,R_end,G_end,B_end);
          strip.setPixelColor(3,R_end,G_end,B_end); // This is the right LED on the clock
          strip.show(); // Send the color information to the LED
          delay(200);
          i++;
      }
      strip.setPixelColor(0,R,G,B); // This is the left LED on the NIXLER clock
      strip.setPixelColor(1,R,G,B); 
      strip.show(); // Send the color information to the LED
      strip.setPixelColor(2,R,G,B);
      strip.setPixelColor(3,R,G,B); // This is the right LED on the clock
      strip.show(); // Send the color information to the LED  
      return timeMode;
} // END setupNixie function

//---------------------------------------------------------------------------------------------------
// Name: runNixlerTime 
// Summmary: Display time in either 24h or 12h format 

void dispHourMinute(int timeMode)
{
   DateTime now = rtc.now();
   int MINUTE = now.minute();
   int HOUR = now.hour();

   if(timeMODE == 12){
      if(HOUR == 13){
           HOUR = 1;
      }
      if(HOUR == 14){
           HOUR = 2;
      }
      if(HOUR == 15){
           HOUR = 3;
      }
      if(HOUR == 16){
           HOUR = 4;
      }
      if(HOUR == 17){
          HOUR = 5;
      }
      if(HOUR == 18){
          HOUR = 6;
      }
      if(HOUR == 19){
          HOUR = 7;
      }
      if(HOUR == 20){
          HOUR = 8;
      }
      if(HOUR == 21){
          HOUR = 9;
      }
      if(HOUR == 22){
          HOUR = 10;
      }
      if(HOUR == 23){
          HOUR = 11;
      }
   }

   int H1 = HOUR / 10;  // dividing by 10 gives us the first number. example 12/10 = 1.2 = 1
   int H2 = HOUR % 10;  // modulus opperation: 12 mod 10 equals 1 and a remainder of 2. 
                        // H2 takes the value of the remainder. 
                        // Same procedure for Minute and seconds. 
   int M1 = MINUTE / 10;   
   int M2 = MINUTE % 10;

   dispNixler(H1,H2,M1,M2); // send the values of H1,H2,M1 and M2 to the nixiedisplays
} // END runNixlerTime 

//---------------------------------------------------------------------------------------------------
// Name: dispSecond
// Summmary: Display current second on nixie tubes  
void dispSecond(){
   DateTime now = rtc.now();
   int SECOND = now.second();
   
   int S1 = SECOND / 10;
   int S2 = SECOND % 10;
   
   dispNixler(0,0,S1,S2);
  
} // END dispSecond function
//---------------------------------------------------------------------------------------------------
// Name: cycleNixler
// Summary: Cycle throug all the numbers of the display from back to front.
// the number stack for IN12 tubes back to fron: 1 6 2 7 5 0 4 9 8 3
void cycleNixler(int iterations){
  int values [] = {1, 6, 2, 7, 5, 0, 4, 9, 8, 3};

  for(int x = 0; x < iterations; x++){
      int i = 0;
      while(i < 10){
        int NX = values[i];
        dispNixler(NX,NX,NX,NX);
        delay(80);
        i++;
      } 
      i = 8;
      while(i > 0){
        int NX = values[i];
        dispNixler(NX,NX,NX,NX);
        delay(80);
        i = i - 1;
      } 
  }
} // END cycleNixler
//---------------------------------------------------------------------------------------------------
// Name: checkButtons
// Summary: check the state of the two buttons and decide what function to run based on the state
int checkButtons(){
     delay(50);
     bool Btn1_state = digitalRead(Btn1);
     bool Btn2_state = digitalRead(Btn2); 
     if(Btn1_state && Btn2_state  == HIGH){
          // Enter the NIXLER setting mode set the time, date and 12/24 hour mode.
          int returned_data = setupNixler(Btn1, Btn2);
          return returned_data; //returns TimeMODE as int (24 or 12)
     }
     
     if(Btn1_state == LOW && Btn2_state == HIGH){
          digitalWrite(enableHV, LOW); // Enable HV power supply again
          strip.setPixelColor(0,R,G,B); // This is the left LED on the clock
          strip.setPixelColor(1,R,G,B); // 
          strip.show(); // This sends the updated pixel color to the hardware.
          strip.setPixelColor(2,R,G,B); //
          strip.setPixelColor(3,R,G,B); // Last digit LED uses temperature color to mark decimal 
          strip.show(); // This sends the updated pixel color to the hardware.
          cycleNixler(2); // Put on a show... new function goes here blinky, blinky, party, party.
     }

     if(Btn1_state == HIGH && Btn2_state == LOW){
           digitalWrite(enableHV, LOW); // Enable HV power supply again
           // Get into a loop where every next push of the button cycles through to the next display function: 
           // Cycle through: temperature, pressure and day/month/year
           int R_s = 0; // variable for temperature color
           int G_s = 0; // variable for temperature color
           int B_s = 0; // variable for temperature color
           int R_p = 0; // variable for pressure color
           int G_p = 0; // variable for pressure color
           int B_p = 0; // variable for pressure color
           bool state = true;
           int push = 0;
           int setColor = 1;
           int wasPush = 0; // variable for holding if buttons was actually pushed 
           while(state == true){
 
             // Test to see wheter the button was toggeld
             bool Btn1_state1 = digitalRead(Btn1);
             delay(10); // Try without delay too
             bool Btn1_state2 = digitalRead(Btn1);

             if(Btn1_state1!= Btn1_state2){
                   // Register a push of the button (button 1)
                   push++;
                   while(digitalRead(Btn1) == HIGH){
                    //.. wait for the release of the button
                   }
             }
             // 1st toggle: Read and display temperature
             if(push == 1){
                    wasPush = 1;
                    int temperature = 0;
                    // Read and display temperature 
                    if(temperatureMode == 1){ // Celsius
                        temperature = mySensor.readTempC();
                    }
                    if(temperatureMode == 2){ // Fahrenheit
                        temperature = mySensor.readTempF();
                    }
                    
                    int temp1; // Declare variables
                    int temp2;
                    int temp3;

                    if(temperatureMode == 1){ // Celsius mode 
                      temp1 = 0; 
                      temp2 = temperature / 10; // Base ten
                      temp3 = temperature % 10; // modulus 
                    }

                    if(temperatureMode == 2){ // Fahrenheit mode
                      temp1 = temperature / 100;
                      temp2 = (temperature % 100) / 10;
                      temp3 = (temperature % 100) % 10;
                    }
                    
                    if(temperature > 0){ // Set backlight to red if positive temperature, blue if negative
                    R_s = 80;
                    G_s = 0;
                    B_s = 0;
                    }
                    else{ 
                    R_s = 0;
                    G_s = 0;
                    B_s = 80;
                    }
                    
                    if(setColor == 1){
                        strip.setPixelColor(0,R_s,G_s,B_s); // This is the left LED on the clock
                        strip.setPixelColor(1,R_s,G_s,B_s); // 
                        strip.show(); // This sends the updated pixel color to the hardware.
                        strip.setPixelColor(2,R_s,G_s,B_s); //
                        strip.setPixelColor(3,R_s,G_s,B_s); // This is the right LED on the clock
                        strip.show(); // This sends the updated pixel color to the hardware.
                        setColor = 2;
                       
                    }
                    dispNixler(0,temp1,temp2,temp3);  // display temperature 
             }

             // 2nd toggle: Read and display pressure in KPa
             if(push == 2){
                  // Read and display pressure
                  int pressure = mySensor.readFloatPressure();
                  pressure = pressure / 100; // Get the pressure in KPa
                  int P1 = pressure / 1000;
                  int P2 = (pressure % 1000) / 100;
                  int P3 = (pressure % 100) / 10;
                  int P4 = pressure % 10; 
            
                  dispNixler(P1,P2,P3,P4);  // Display pressure values on nixies
                  
                  if(setColor == 2){ // Set the LED color
                        R_p = 60;
                        G_p = 60;
                        B_p = 60;
                        strip.setPixelColor(0,R_p,G_p,B_p); // This is the left LED on the clock
                        strip.setPixelColor(1,R_p,G_p,B_p); // 
                        strip.show(); // This sends the updated pixel color to the hardware.
                        strip.setPixelColor(2,R_p,G_p,B_p); //
                        strip.setPixelColor(3,0,0,B_p); // Last digit LED uses temperature color to mark decimal 
                        strip.show(); // This sends the updated pixel color to the hardware.
                        delay(10);
                        setColor = 3;
                  } 
             }         
             // 3rd toggle: Get and display day and month   
             if(push == 3){ 
                  DateTime now = rtc.now();
                  int DAY = now.day();
                  int MONTH = now.month();
                  int D_D1 = 0;
                  int D_D2 = DAY;
                  int M_D1 = 0;
                  int M_D2 = MONTH;

                  if(DAY > 9){
                      D_D1 = DAY / 10; 
                      D_D2 = DAY % 10;
                  }
                  if(MONTH > 9){
                      M_D1 = MONTH / 10;
                      M_D2 = MONTH % 10;
                  }
                  dispNixler(D_D1,D_D2,M_D1,M_D2); // Display current day and month
                  
                  if(setColor == 3){ // set the LED color 
                        strip.setPixelColor(0,R_p,G_p,B_p); // This is the left LED on the clock
                        strip.setPixelColor(1,R_p,G_p,B_p); // 
                        strip.show(); // This sends the updated pixel color to the hardware.
                        strip.setPixelColor(2,R_p,G_p,B_p); //
                        strip.setPixelColor(3,R_p,G_p,B_p); // Last digit LED uses temperature color to mark decimal 
                        strip.show(); // This sends the updated pixel color to the hardware.
                        delay(10);
                        setColor = 4;
                  }                   
             }
             // 4th toggle: Get and display year
             if(push == 4){  // 2nd toggle: Display pressure in KPa
                  DateTime now = rtc.now();
                  int YEAR = now.year();
                  int Y_D1 = YEAR / 1000;
                  int Y_D2 = (YEAR % 1000) / 100;
                  int Y_D3 = (YEAR % 100) / 10;
                  int Y_D4 = YEAR % 10; 
                  dispNixler(Y_D1,Y_D2,Y_D3,Y_D4);
             }
             // 5th toggle, out of function
             if(push > 4  && wasPush == 1){
                  strip.setPixelColor(0,R,G,B); // This is the left LED on the clock
                  strip.setPixelColor(1,R,G,B); // 
                  strip.show(); // This sends the updated pixel color to the hardware.
                  strip.setPixelColor(2,R,G,B); //
                  strip.setPixelColor(3,R,G,B); // Last digit LED uses temperature color to mark decimal 
                  strip.show(); // This sends the updated pixel color to the hardware.
                  
                  state = false; // return out of the "display temperature, pressure, day/monsth/year - loop"
                  break;
             }  
         }
    }

}// END checkButtons
//----------------------- END FUNCTIONS ----------------------------------------------------------------------------
