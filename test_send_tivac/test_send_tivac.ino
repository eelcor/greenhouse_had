/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

extern "C"{
  int _write(int file, char *ptr, int len)
  {
    unsigned int i;
    for(i = 0; i < len; i++){
      Serial.write((uint8_t)ptr[i]);
    }
    return len;
  }
}


/**
 * Example RF Radio Ping Pair
 *
 * This is an example of how to use the RF24 class.  Write this sketch to two different nodes,
 * connect the role_pin to ground on one.  The ping node sends the current time to the pong node,
 * which responds by sending the value back.  The ping node can then see how long the whole cycle
 * took.
 */

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "Wire.h"
#include "DHT22_430.h"
// #include "printf.h"

//
// Hardware configuration
//

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10

RF24 radio(3,4);

// Set up a Soft I2C channel
#include "BMP085_t.h"
// Set up a Barometric Pressure Sensor
BMP085<0> PSensor;

// Set up Humidity
#define DHTPIN 33
DHT22 mySensor(DHTPIN);

#define DS1307_I2C_ADDRESS 0x68
#define BH1750_Device 0x23

//Delays
#define dly 100
#define interval 30000

//
// Topology
//

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;

//
// Role management
//
// Set up role.  This sketch uses the same software for all the nodes
// in this system.  Doing so greatly simplifies testing.  The hardware itself specifies
// which node it is.
//
// This is done through the role_pin
//


void setup(void) {
  Serial.begin(9600); //Debug 
  pinMode(32,OUTPUT); 
  mySensor.begin();

  //nRF24 stuff
  SPI.setModule(0);
  radio.begin();
  //Wire.setModule(0);
  //pinMode(24,INPUT_PULLUP); // I2C3SDA 
  //pinMode(23,INPUT_PULLUP); // I2C3SCL

  Wire.begin();

  radio.setRetries(15,15);
  radio.setDataRate(RF24_250KBPS);
  radio.setPayloadSize(16);
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);
  radio.printDetails();
  radio.startListening();
  PSensor.begin();
  Serial.print("Init done...");
  //myServo.attach(7);
  //myServo.write(90);
}

void loop() {
  int result;
  // Turn sensors on
  //digitalWrite(13, HIGH); 
  delay(dly);

  // Start packet
  radio.stopListening();
  sendval("START");
  radio.startListening();
  
  delay(dly);
  // Send the Time
  radio.stopListening();
  // Reset the register pointer
  digitalWrite(32,HIGH);
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0x00);
  Wire.endTransmission(); 
  Wire.requestFrom(DS1307_I2C_ADDRESS, 7);
 
  // A few of these need masks because certain bits are control bits
  second     = Wire.read() & 0x7f;
  minute     = Wire.read();
  hour       =  Wire.read() & 0x3f;  // Need to change this if 12 hour am/pm
  dayOfWeek  = Wire.read();
  dayOfMonth = Wire.read();
  month      = Wire.read();
  year       = Wire.read();
  digitalWrite(32,LOW);
  result = sendval("TIME:"+String(hour/16)+String(hour%16)+":"+String(minute/16)+String(minute%16)+":"+String(second/16)+String(second%16));
  radio.startListening();

  delay(dly);
  radio.stopListening();
  result = sendval("DATE:"+String(dayOfMonth/16)+String(dayOfMonth%16)+"/"+String(month/16)+String(month%16)+"/20"+String(year/16)+String(year%16));
  radio.startListening();
  
  delay(dly);
  // Send Pressure Value
   radio.stopListening();
  // Get value from Pressure sensor
  PSensor.refresh();                    // read current sensor data
  PSensor.calculate();                  // run calculations for temperature and pressure
  Serial.print(PSensor.temperature);
  Serial.print(PSensor.pressure);
  result = sendval("TEMP:"+String(PSensor.temperature/10)+"."+String(PSensor.temperature%10));
  radio.startListening();  
  
  delay(dly);
  radio.stopListening();
  result = sendval("PRES:"+String((PSensor.pressure+50)/100));
  radio.startListening(); 
  
  delay(dly);
  // Read Light Sensor
  radio.stopListening();
  Configure_BH1750();
  sendval("LUX:"+String(BH1750_Read()));
  radio.startListening();
  
    // Read from Humidity sensor
  mySensor.get();
  int32_t h = mySensor.humidityX10();
  radio.stopListening();
  sendval("HUMI:"+String(h/10)+"."+String(h%10));
  radio.startListening();
  delay(dly);
  // Send End packet
  radio.stopListening();
  sendval("END"); 
  radio.startListening();

  //digitalWrite(13,LOW);
  delay(interval);
}

int sendval(String msg)
{
  int result;
  char buf[msg.length()+2];
  msg.toCharArray(buf,msg.length()+1);
  result = radio.write(&buf, msg.length()+1);
  Serial.println(msg);
  return result;
}

void Configure_BH1750() 
{
  Wire.beginTransmission(BH1750_Device);
  Wire.write(0x10);      // Set resolution to 1 Lux
  Wire.endTransmission();
}

unsigned int BH1750_Read() //
{
  unsigned int i=0;
  Wire.beginTransmission(BH1750_Device);
  Wire.requestFrom(BH1750_Device, 2);
  while(Wire.available()) //
  {
    i <<=8;
    i|= Wire.read();  
  }
  Wire.endTransmission();  
  return i/1.2;  // Convert to Lux
}
