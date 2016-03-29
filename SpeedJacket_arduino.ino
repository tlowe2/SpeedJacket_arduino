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
#include <SoftwareSerial.h>

// How many boards do you have chained?
#define NUM_TLC5974 1

#define data      11
#define clk       10
#define latch     3
#define debug     1
#define debug1    0

#define SPREAD    3000

Adafruit_TLC5947 driver = Adafruit_TLC5947(NUM_TLC5974, clk, data, latch);

void frameHandler(void);
void findOffsets(int num_grps);
void allOff(void);
void allOn(void);

void pulseOut(int front, int back);
void LtoR(int front, int back);

// initialize holding arrays for groupings
int arrows[5][7] = {
    {11, 64, 64, 64, 64, 64, 12} ,
    {10, 6, 3, 64, 20, 17, 13} ,
    {9, 5, 2, 0, 21, 18, 14} ,
    {8, 4, 1, 64, 22, 19, 15} ,
    {7, 64, 64, 64, 64, 64, 16}
  };
int off[23];

//
SoftwareSerial GPS(9, 2); // RX, TX

short lockout = 0;
int avgvel = 0;

int charToInt(char ten, char one);

// global vars
int i, j, k;
int skip = 0;

// cycling variables
volatile int head = 4000;
volatile int tail = head - SPREAD;
volatile int dir_head = 1;
volatile int dir_tail = 1;
int pwm_steps = 250;
int refresh_microsec = 25000;

void setup() 
{
  if (debug){
    Serial.begin(115200);
    Serial.println("debug on @ 115200");
  }

  GPS.begin(9600);  // start the GPS serial connection
  
  driver.begin();   // initialize the driver

  // initialize timer interrupt for frame refresh
  int buff = refresh_microsec;

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

  Timer1.initialize(buff);
  Timer1.attachInterrupt(frameHandler);
}

void loop() 
{
  char cvel[4];
  char *parse;
  char holder[64];
  int ivel[4];

  for (k = 0; k < 4; k++){
    ivel[k] = 0;
    cvel[k] = '\n';
  }
  k = 0;
  while (GPS.available()){
    lockout = 1;
    holder[k] = GPS.read();

    if (holder[0] != '$'){
      lockout = 0;
      break;
    }
    if (k == 3){
      if (holder[k] != 'R'){
        lockout = 0;
        break;
      }
    }
    
    Serial.write(holder[k]);
    if (holder[k] == '\n'){
      lockout = 0;
      parse = holder;

      k = 0;
      while (k != 7){
        if (*parse == ',')
          k++;
        parse++;
      }
      k = 0;
      while (*parse != '.' && *parse != ','){
        cvel[k] = *parse;
        k++;
        parse++;
      }

      for (k = 3; k > 0; k--)
        ivel[k] = ivel[k-1]; 
      
      ivel[0] = charToInt(cvel[0], cvel[1]);

      avgvel = (ivel [0] + ivel[1] + ivel[2] + ivel[3]) >> 2;
      break;
    }
    k++;
  }
   
  //Serial.println("out");
  
}

int charToInt( char ten, char one)
{
  //int ihun = hundred;
  int iten = ten;
  int ione = one;

  return 10*iten + ione;
}

void frameHandler()
{
  if (lockout)
    return;
    
  if (!skip)
  {
    if (head <= 0){
      dir_head = 1;
      if (head < 0){
        head = 0;
      }
    }else if (head >= 4095){
      dir_head = 0;
      if (head > 4095){
        head = 4095;
      }
    }
    /*
    if (tail == 0){
      dir_tail = 1;
    }else if (tail == 4095){
      dir_tail = 0;
    }else if (tail > 4095 || tail < 0){
      tail = 0;
    }
    */
    
    LtoR(head, tail);

    //Serial.println("In frame handler");
    //Serial.println(head);

    driver.write();
    
    if (dir_head){
      head = head + pwm_steps;
    }else{
      head = head - pwm_steps;
    }
    return;
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

void LtoR(int front, int back)
{
  findOffsets(7);

  for (i = 0; i < 7; i++){
    for (j = 0; j < 5; j++){
      if (arrows[j][i] < 23){
        driver.setLED(arrows[j][i], off[i], 0, 0);
      }
    }
  }
  //Serial.println("In LtoR");
}

void pulseOut(int front, int back)
{
  
}

void findOffsets(int num_grps)
{
  int offset = SPREAD/num_grps;
  int prev_dir = dir_head;
  
  off[0] = head;

  for (i = 1; i < num_grps; i++){
    if (prev_dir){
      off[i] = off[i-1] - offset;
      if (off[i] < 0){
        off[i] = offset - off[i-1];
        prev_dir = !prev_dir;
      }
    }else if (!prev_dir){
      off[i] = off[i-1] + offset;
      if (off[i] > 4095){
        off[i] = 4095 - (offset - (4095 - off[i-1]));
        prev_dir = !prev_dir;
      }
    }
    //Serial.print(i);
    //Serial.print(" ");
    //Serial.println(off[i]);
  }
}
