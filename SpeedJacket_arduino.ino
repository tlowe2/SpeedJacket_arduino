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
#include "TimerOne.h"

// How many boards do you have chained?
#define NUM_TLC5974 1

#define data      11
#define clk       10
#define latch     3
#define debug     1
#define debug1    0

#define OFFSET    200
#define NUMLEDS   23

Adafruit_TLC5947 driver = Adafruit_TLC5947(NUM_TLC5974, clk, data, latch);

void allOff(void);
void allOn(void);
void pulseOut(void);

// initialize holding arrays for groupings
int leftarrow[6];
int lmarrow[4];
int lsarrow[4];
int rightarrow[6];
int rmarrow[4];
int rsarrow[4];
int center[2];

int *frame;

// global vars
int i, j;
int skip = 0;

// cycling variables
volatile int head = 4095;
volatile int tail = head - OFFSET*(NUMLEDS-1);
volatile int dir_i = 1;
volatile int dir_j = 1;
int refresh_microsec = 500;

void setup() 
{
  if (debug){
    Serial.begin(115200);
    Serial.println("debug on @ 115200");
  }
  
  driver.begin();   // initialize the driver

  // initialize timer interrupt for frame refresh
  int buff = 16*refresh_microsec;

  // Populate the grouping arrays of LED numbers, adding sentinel at the end
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
  leftarrow[5] = 64;
  rightarrow[5] = 64;
  lmarrow[3] = 64;
  rmarrow[3] = 64;
  lsarrow[3] = 64;
  rsarrow[3] = 64;
  center[0] = 0;
  center[1] = 64;

  // flash 3 times after main setup
  allOff();
  allOn();
  delay(200);
  allOff();
  delay(200);
  allOn();
  delay(200);
  allOff();
  delay(200);
  allOn();
  delay(200);
  allOff();

}

void loop() 
{
  delay(200);
  allOff();
  delay(200);
  allOn();
  delay(200);

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


void allOff()
{
  for (i = 0; i < 24; i++)
  {
    driver.setLED(i, 0, 0, 0);
  }
  driver.write();
}

void allOn()
{
  for (i = 0; i < 24; i++)
  {
    driver.setLED(i, 4095, 0, 0);
  }
  driver.write();
}
