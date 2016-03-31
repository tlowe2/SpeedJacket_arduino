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

#define MOVAVG    2     //only 2 or 4 AND THAT'S IT!!!

#define SPREAD    3000

Adafruit_TLC5947 driver = Adafruit_TLC5947(NUM_TLC5974, clk, data, latch);

void frameHandler(void);
void findOffsets(int num_grps);
void allOff(void);
void allOn(void);

void pulse_in();
void pulse_out();
void LtoR();
void pulseIndividual();

// initialize holding arrays for groupings
volatile const int arrows[5][7] = {
    {11, 64, 64, 64, 64, 64, 12} ,
    {10, 6, 3, 64, 20, 17, 13} ,
    {9, 5, 2, 0, 21, 18, 14} ,
    {8, 4, 1, 64, 22, 19, 15} ,
    {7, 64, 64, 64, 64, 64, 16}
  };
volatile const int doublearrows[5][8] = {
    {11, 12, 64, 64, 64, 64, 64, 64} ,
    {10, 13, 6, 17, 20, 3, 64, 64} ,
    {9, 14, 5, 18, 21, 2, 0, 64} ,
    {8, 15, 4, 19, 22, 1, 64, 64} ,
    {7, 16, 64, 64, 64, 64, 64, 64}
  };
volatile const int outarrows[5][8] = {
    {64, 64, 64, 64, 64, 64, 12, 11} ,
    {64, 64, 3, 20, 17, 6, 13, 10} ,
    {64, 0, 2, 21, 18, 5, 14, 9} ,
    {64, 64, 1, 22, 19, 4, 15, 8} ,
    {64, 64, 64, 64, 64, 64, 16, 7}
  };
volatile int off[23];

//
SoftwareSerial GPS(9, 2); // RX, TX

short lockout = 0;
volatile int avgvel = 0;
volatile int ivel[MOVAVG];

int charToInt(char ten, char one);

// global vars
int i, j, k;
int skip = 0;

// cycling variables
volatile int head = 4000;
volatile int tail = head - SPREAD;
volatile int dir_head = 1;
volatile int dir_tail = 1;
volatile int pwm_steps = 250;
int refresh_microsec = 25000;

void setup() 
{
  if (debug){
    Serial.begin(115200);
  }

  GPS.begin(9600);  // start the GPS serial connection
  
  driver.begin();   // initialize the driver

  // initialize timer interrupt for frame refresh
  int buff = refresh_microsec;

  // initialize moving average
  for (k = 0; k < 4; k++)
    ivel[k] = 0;
    
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
  char holder[128];
  int sum = 0;

  // initialize character velocity and holder string
  for (k = 0; k < 64; k++){
    holder[64] = '0';
    if (k < 4)
      cvel[k] = '\n';
  }

  // GPS parsing loop
  k = 0;
  while (GPS.available()){
    lockout = 1;
    holder[k] = GPS.read();

    // If it is leftover data, ignore
    if (holder[0] != '$'){
      lockout = 0;
      break;
    }

    // If it is not the RMC NMEA string, ignore
    if (k == 3){
      if (holder[k] != 'R'){
        lockout = 0;
        break;
      }
    }
    
    Serial.write(holder[k]);
    //Serial.print(" ");
    //Serial.println();
    // if we have reached the end of the RMC string, begin parsing
    if (holder[k] == '\n'){
      //Serial.println("in parsing");
      
      lockout = 0;      // remove the lockout for this section

      if (holder[17] == 'V'){ // if the data is invalid, ignore
        avgvel = -1;
        break;
      }
      if (holder[17] != 'A') // if the data is invalid, ignore
        break;
        
      parse = holder;   // set the parser at the holder

      // send the parser to the velocity area (7 commas in)
      k = 0;
      while (k != 7){
        if (*parse == ',')
          k++;
        parse++;
        
      }

      // send the parser to the decimal point of the velocity
      do{
        parse++;
      }while (*parse != '.' && *parse != ',');

      // parse from LSB to MSB
      k = 0;
      while (*parse != ','){
        parse--;
        cvel[k] = *parse;
        //Serial.print("cvel[");
        //Serial.print(k);
        //Serial.print("] ");
        //Serial.write(cvel[k]);
        //Serial.println();
        k++;
      }

      //Serial.print("cvel[0] ");
      //Serial.println(cvel[0]);
      
      for (k = (MOVAVG-1); k > 0; k--){
        ivel[k] = ivel[k-1]; 
        
        //Serial.print("ivel[");
        //Serial.print(k);
        //Serial.print("] ");
        //Serial.println(ivel[k]);
      }

      // if speed is single digits, no ten
      if (cvel[1] == ',')
        cvel[1] = '0';
        
      ivel[0] = charToInt(cvel[1], cvel[0]);
      
      //Serial.print("ivel[0] ");
      //Serial.println(ivel[0]);

      for (k = 0; k < MOVAVG; k++)
        sum = sum + ivel[k];
      
      avgvel = sum >> MOVAVG/2;
      
      Serial.print("avgvel ");
      Serial.println(avgvel);
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
  if (ten == '\n')
    iten = '0';
  int ione = one;

  return 10*(iten - '0') + (ione - '0');
}

void frameHandler()
{
  if (lockout)
    return;
    
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
  if (avgvel == -1){
    pwm_steps = 100;
    pulse_in();
  }else if (avgvel < 5){
    pwm_steps = 200;
    pulseIndividual();
  }else if (avgvel >= 5 && avgvel < 10){
    pwm_steps = 400;
    pulseIndividual();
  }else if (avgvel >= 10 && avgvel < 15){
    pwm_steps = 300;
    pulse_out();
  }else if (avgvel >= 15 && avgvel < 20){
    pwm_steps = 500;
    pulseIndividual();
  }else if (avgvel >= 20 && avgvel < 30){
    pwm_steps = 800;
    pulseIndividual();
  }else if (avgvel >= 30 && avgvel < 40){
    pwm_steps = 1000;
    pulseIndividual();
  }else if (avgvel >= 40 && avgvel < 50){
    pwm_steps = 1300;
    pulseIndividual();
  }else if (avgvel >= 50){
    pwm_steps = 1500;
    pulseIndividual();
  }
  //LtoR(head, tail);
  //pulse_in(head, tail);
  //pulseIndividual();  
  
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

void LtoR()
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

void pulse_in()
{
  findOffsets(4);

  for (i = 7; i >= 0; i=i-2){
    for (j = 0; j < 5; j++){
      if (doublearrows[j][i] < 23){
        driver.setLED(doublearrows[j][i], off[i/2], 0, 0);
      }
      if (doublearrows[j][i-1] < 23){
        driver.setLED(doublearrows[j][i-1], off[i/2], 0, 0);
      }
    }
  }
}

void pulse_out()
{
  findOffsets(4);

  for (i = 7; i >= 0; i=i-2){
    for (j = 0; j < 5; j++){
      if (outarrows[j][i] < 23){
        driver.setLED(outarrows[j][i], off[i/2], 0, 0);
      }
      if (outarrows[j][i-1] < 23){
        driver.setLED(outarrows[j][i-1], off[i/2], 0, 0);
      }
    }
  }
}

void pulseIndividual()
{
  for (j = 0; j < 5; j++){
    if (outarrows[j][i] < 23){
      driver.setLED(outarrows[j][i], head, 0, 0);
    }
    if (outarrows[j][i+1] < 23){
      driver.setLED(outarrows[j][i+1], head, 0, 0);
    }
  }  
  if (head == 0)
    i = i+2;
  if (i > 6)
    i = 0;
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
