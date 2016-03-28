// LED Jacket code - Teddy Lowe, March 2016 
// Based on code from Adafruit Industries

/*************************************************** 
  This is an example for our Adafruit 24-channel PWM/LED driver

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/1429

  These drivers uses SPI to communicate, 3 pins are required to  
  interface: Data, Clock and Latch. The boards are chainable

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include "Adafruit_TLC5947.h"

// How many boards do you have chained?
#define NUM_TLC5974 1

#define data    11
#define clk     10
#define latch   3
#define debug   1
#define debug1  0

Adafruit_TLC5947 driver = Adafruit_TLC5947(NUM_TLC5974, clk, data, latch);

void pulseOut(int spd);

// initialize holding arrays for groupings
int leftarrow[5];
int lmarrow[3];
int lsarrow[3];
int rightarrow[5];
int rmarrow[3];
int rsarrow[3];
int center = 0;

// global vars
int i, ;

void setup() {
  if (debug){
    Serial.begin(9600);
    Serial.println("debug on @ 115200");
  }
  
  driver.begin();   // initialize the driver

  // Populate the grouping arrays of LED numbers
  for (i = 0; i < 5; i++)
  {
    leftarrow[i] = 11 - i;
    rightarrow[i] = 12 + i;

    if (i < 3)
    {
      lmarrow[i] = 6 - i;
      rmarrow[i] = 17 + i;
      lsarrow[i] = 3 - i;
      rsarrow[i] = 20 + i;  
    }
  }
  for (i = 0; i < 24; i++)
  {
    driver.setLED(i, 0, 0, 0);
  }
  driver.write();
}

void loop() {
  pulseOut(10); 

  if (debug1)
  {
    for (i = 0; i < 5; i++)
    {
      Serial.print("leftarrow");
      Serial.print(i);
      Serial.print(" ");
      Serial.println(leftarrow[i]);
      Serial.print("rightarrow");
      Serial.print(i);
      Serial.print(" ");
      Serial.println(rightarrow[i]);
      if (i < 3)
      {
        Serial.print("lmarrow");
        Serial.print(i);
        Serial.print(" ");
        Serial.println(lmarrow[i]);
        Serial.print("rmarrow");
        Serial.print(i);
        Serial.print(" ");
        Serial.println(rmarrow[i]);
        Serial.print("lsarrow");
        Serial.print(i);
        Serial.print(" ");
        Serial.println(lsarrow[i]);
        Serial.print("rsarrow");
        Serial.print(i);
        Serial.print(" ");
        Serial.println(rsarrow[i]);
      }
    }
  }

}

void pulseOut(int wait)
{
  for (i = 0; i < 4095; i++)
  {
    
  }
}
