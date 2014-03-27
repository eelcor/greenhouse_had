
/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

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
// Set up nRF24L01 radio on SPI bus plus pins 9 & 10

RF24 radio(9,10);

// sets the role of this unit in hardware.  Connect to GND to be the 'pong' receiver
// Leave open to be the 'ping' transmitter

//
// Topology
//

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

//
// Role management
//
// Set up role.  This sketch uses the same software for all the nodes
// in this system.  Doing so greatly simplifies testing.  The hardware itself specifies
// which node it is.
//
// This is done through the role_pin
//


// Additional Routines
// Putchar instantiation

uint32_t i = 0;
// The various roles supported by this sketch

// The debug-friendly names of those roles

// The role of the current running sketch

void setup(void)
{
  Serial.begin(9600);
  //
  // Print preamble
  //
  //
  // Setup and configure rf radio
  //
  pinMode(RED_LED,OUTPUT);
  pinMode(PUSH2,INPUT);
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.setRetries(15,15);
  radio.setPayloadSize(16);

  //
  // Open pipes to other nodes for communication
  //
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);

  //
  // Start listening
  radio.startListening();
}

void loop(void)
{
  Serial.print("Start\r\n");
  // Setup variables
  char buf[11];
  
  uint32_t rmdr;
  uint32_t integ;
  
  // Wait for trigger
  while(!digitalRead(PUSH2));
  
  //increase and convert i
  i++;
  integ = i;
  Serial.print(integ);
  Serial.print(integ%10);
  Serial.print(integ/10);
  int iter;
  for (iter=0; iter < 11; iter++)
  {
    rmdr = integ%10;
    buf[10-iter] = char(rmdr) + 48;
    integ = integ/10;
  }
  
  
  Serial.print(buf);
  // First, stop listening so we can talk.
  radio.stopListening();
  digitalWrite(RED_LED,HIGH);
  // Take the time, and send it.  This will block until complete
  bool ok = radio.write( &buf, 12);
  // Now, continue listening
  radio.startListening();
  digitalWrite(RED_LED,LOW);
  delay(200);
}

