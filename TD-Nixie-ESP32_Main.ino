
const int ledPin = 26; 
const int enableHV = 18; 
const int Btn1 = 35; 
const int Btn2 = 34;
int R = 0; 
int G = 15; 
int B = 25; 
int cnt = 50;

//  -- Variables used for setting the time
int a = 0;
int b = 0;
int c = 0;
int d = 0;
int timeMODE = 24; 

// ----------------------------- Assign the ESP32 pins to shift register -------------------------------------

const int latchPin = 32;   // pin  12 on the 74hc595   ESP - 32      ST_CP
const int dataPin = 25;    // pin 14 on the 74hc595    ESP - 25      DS_pin
const int clockPin = 33;   // pin 11 on the 74hc595    ESP - 33      SH_CP

//------------------------------ NeoPixel setup --------------------------------------------------------------
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
#define PIN            12 
#define NUMPIXELS      4

//-------------------------------------------- RTC clock setup ------------------------------------------------
#include <Wire.h>  // for both RTC and Sensor
#include <RTClib.h>
//-------------------------------------------- SENSOR setup ---------------------------------------------------
#include <SparkFunBME280.h>
RTC_DS3231 rtc;
BME280 mySensorA; //Uses I2C address 0x76 (jumper closed)


// ---------------------------- Neopixel adressable LEDs ------------------------------------------------------
// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel strip(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// ------------------------------------------------------------------------------------------------------------

// --------------------- SETUP AND  INITIALIZATION ---------------------------------------------------------
void setup() { 
  Serial.begin(9600); 
  
  pinMode(Btn1, INPUT); 
  pinMode(Btn2, INPUT); 
  pinMode(enableHV, OUTPUT);
  digitalWrite(enableHV, LOW);  // pull the enable pin low to power on the high voltage supply

  pinMode(ledPin,   OUTPUT); 
  // Set the ShiftRegister Pinss as output
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin,  OUTPUT);

  Wire.begin(21,22,400000); // SDA,SCL,frequenzy RTC clock
  //rtc.adjust(DateTime(2020, 1, 21, 21, 21, 21));

  strip.begin(); // This initializes the NeoPixel LED library

  mySensorA.setI2CAddress(0x76); //The default for the SparkFun Environmental Combo board is 0x77 (jumper open).
  //If you close the jumper it is 0x76
  //The I2C address must be set before .begin() otherwise the cal values will fail to load.
  if(mySensorA.beginI2C() == false) Serial.println("Sensor A connect failed");
 
}

 /* For the TorDesign NIXLER display we have 20 bit shiftregisters 
    incorporated in the two HV drivers (HV5812 chip). These registers are controlled by the ESP32
    i  order to controll the 4 nixe-tubes. If the bit in the register is set to 1 the coresponding nixie-tube wil
    not light up, if its set to 0 it will. 

    In total we send 40 bit from the ESP32 everytime we update the display - posibility for partial update? 
    
    [00000 00000 00000 00000 00000 00000 00000 00000] 
         NIX4        NIX3        NIX2        NIX1 
  */ 
//----------------------- MAIN LOOP ----------------------------------


void loop(){
 bool Btn1_state = digitalRead(Btn1);
 bool Btn2_state = digitalRead(Btn2); 
 
 if(Btn1_state && Btn2_state  == HIGH){
      // Enter setting mode set the time, date and 12/24 hour mode.
      timeMODE = setupNixler(Btn1, Btn2);
 }
 
 strip.setPixelColor(0,R,G,B); // This is the left LED on the clock
 strip.setPixelColor(1,R,G,B); // 
 strip.show(); // This sends the updated pixel color to the hardware.
 strip.setPixelColor(2,R,G,B); //
 strip.setPixelColor(3,R,G,B); // This is the right LED on the clock
 strip.show(); // This sends the updated pixel color to the hardware.
 
 runNixlerTime(timeMODE); // Run the main program - displaying time - in either 24h or 12h format 

 cycleNixler(1); // Cycle through all numbers twise
 // If just one of the buttons are bushed enter "showoff mode"
}

//---------- END MAIN LOOP -----------------------------------------------------------------------------------------



//--------- START FUNCTIONS -----------------------------------------------------------------------------------------

// Name: dispNixie 
// Summmary: function for displaying all possible values on the four nixies:
void dispNixler(int Nixie4, int Nixie3, int Nixie2, int Nixie1){  

    
    int bitArray[] = { 
                       1,1,1,1,1,1,1,1,1,1, // NixieTube one      - A zero in the array activates the nixie number 1->9->0 from left to right. 
                       1,1,1,1,1,1,1,1,1,1, // NixieTube two
                       1,1,1,1,1,1,1,1,1,1, // NixieTube three
                       1,1,1,1,1,1,1,1,1,1  // NixieTube four    
                      }; 
                     
   // Manipulate the bitArray to light up a certain number in the first nixietube.        

   // NIXIE ONE: 
   
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
                    
   // Send the updated BitArray to the HV5812 shiftregisters by bit-banging 
           
   digitalWrite(latchPin, LOW); // Ground latchPin and hold low for as long as you are transmitting
   
   for (int i = 0; i < 40; i++){
        int bitVal = bitArray[i];
        digitalWrite(clockPin, LOW);
        digitalWrite(dataPin, bitVal);   
        digitalWrite(clockPin, HIGH);
        }
   digitalWrite(latchPin, HIGH); // Return the latch pin high to signal chip that sequence is done.  

} // END dispNixie function


//---------------------------------------------------------------------------------------------------
// Name: setupNixie 
// Summmary: function for setting the time, date and 12/24 hour format 
int setupNixler(int Btn1, int Btn2){
      bool Btn1_state = digitalRead(Btn1);
      bool Btn2_state = digitalRead(Btn2); 

      int timeMode;
      int hourMode1 = 2;
      int hourMode2 = 4;
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
      // -----------------------------------------------------------------------
      while(digitalRead(Btn2) == HIGH){
          // ... Wait for release of button 2 and move to next
          delay(50); // Debounce delay
      }

      // ----- SET 12/24 HOUR ------------------------------------------------------
      while(digitalRead(Btn2) == LOW){  
        
          dispNixler(10,hourMode1,hourMode2,10);
          bool btn_1_state_1 = digitalRead(Btn1);
          
          if(btn_1_state_1 == HIGH){
            if(hourMode1 == 2){
              hourMode1 = 1;
              hourMode2 = 2;
              timeMode = 12; // 12h mode saved in variable
            }
            else{
              hourMode1 = 2;
              hourMode2 = 4;
              timeMode = 24; // 24h mode saved in variable
            }
            delay(200);
          }
      }
      // -----------------------------------------------------------------------
      while(digitalRead(Btn2) == HIGH){
          // ... Wait for release of button 2 and move to next
          delay(50); // Debounce delay
      }
      
      // -------------- SET LED color and brightness ------------------------------------------------------
      int LED_counter = 0;
      int push = 0;
      while(digitalRead(Btn2) == LOW){  
          dispNixler(0,0,0,0);
          
          bool state1 = digitalRead(Btn1);
          delay(50); // wait for debounce
          bool state2 = digitalRead(Btn1);

          if(state1 != state2){
            // push registered
            push++;
          }
          while(digitalRead(Btn1) == HIGH){
            //.... WAIT
            Serial.print(push); 
          }  


          if(push == 1){ // RED
              R = 200; G = 0; B = 0;   
          }
          if(push == 2){ // GREEN
              R = 0; G = 200; B = 0;
          }
          if(push == 3){ // BLUE
              R = 0; G = 0; B = 200;
          }
          if(push == 4){ // WHIE YELLOW
              R = 240; G = 240; B = 50;
          }
          if(push == 5){ // YELLOW
              R = 240; G = 220; B = 0;
          }
          if(push == 6){ // PINK
              R = 250; G = 0; B = 220;
          }  
          if(push == 7){ // WHITE
              R = 250; G = 250; B = 250;
          } 
          if(push == 8){ // TURQOISE
              R = 0; G = 250; B = 250;
          }    
          if(push == 9){ // NO COLOR
              R = 0; G = 0; B = 0;
          }  
                               
          strip.setPixelColor(0,R,G,B); // This is the left LED on the clock
          strip.setPixelColor(1,R,G,B); // 
          strip.show(); // This sends the updated pixel color to the hardware.
          strip.setPixelColor(2,R,G,B); //
          strip.setPixelColor(3,R,G,B); // This is the right LED on the clock
          strip.show(); // This sends the updated pixel color to the hardware.
          
          if(push > 9){
            push = 1;
          }
      }
      
      // -----------------------------------------------------------------------
      while(digitalRead(Btn2) == HIGH){
          // ... Wait for release of button 2 and move to next
          delay(50); // Debounce delay
      }
      
      // ------------- UPDATE RTC CHIP set time date and format -----------------------------------------
      
      int Nix1 = a; // The leftmost Nixie tube on the NIXLER clock  
      int Nix2 = b; 
      int Nix3 = c; 
      int Nix4 = d; 
      
      // Convert 4 independent digits H:H:M:M to two digits H:M
      int HOUR;
      int MINUTE;
      int SECOND = 0;

      HOUR = (10*Nix1) + Nix2;
      MINUTE = (10*Nix3) + Nix4;
 
      // Pass the hour and minutes to the RTC chip
      rtc.adjust(DateTime(2020, 1, 21, HOUR, MINUTE, SECOND));
      
      // Flash Leds to signal setup complete
      
      int i = 0;
      int R;
      int G;
      int B;
      
      while( i < 2){
          if(i == 0){
            R = 0;
            G = R;
            B = R;
          }
          if(i == 1){
            R = 150;
            G = R;
            B = R;
          }
          if(i == 2){
            R = 0;
            G = R;
            B = R;
          }         
          
          strip.setPixelColor(0,R,G,B); // This is the left LED on the NIXLER clock
          strip.setPixelColor(1,R,G,B); 
          strip.show(); // Send the color information to the LED
          strip.setPixelColor(2,R,G,B);
          strip.setPixelColor(3,R,G,B); // This is the right LED on the clock
          strip.show(); // Send the color information to the LED
          delay(100);
          i++;
      }  
  return timeMode;
} // END setupNixie  function



//---------------------------------------------------------------------------------------------------
// Name: runNixlerTime 
// Summmary: Display time in either 24h or 12h format 

void runNixlerTime(int timeMode)
{
   int S1;
   int S2;
   DateTime now = rtc.now();
   int SECOND = now.second();
   int MINUTE = now.minute();
   int HOUR = now.hour();
   Serial.print(timeMode); //debug
   if(timeMode == 12){
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
 
   Serial.print("Temp:");
   Serial.println(mySensorA.readTempC(), 2);
   delay(3000);
   
   int i = 0;
   while(i < 5){
      DateTime now = rtc.now();
      SECOND = now.second();
      S1 = SECOND / 10;
      S2 = SECOND % 10;

      dispNixler(0,0,S1,S2);
      i++;
      delay(1000);
   }  
} // END runNixlerTime function



// Name: cycleNixler
// Sumamry: Cycle throug all the numbers of the display from back to front.
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
      while(i >= 0){
        int NX = values[i];
        dispNixler(NX,NX,NX,NX);
        delay(80);
        i = i - 1;
      } 
  }
}
   
