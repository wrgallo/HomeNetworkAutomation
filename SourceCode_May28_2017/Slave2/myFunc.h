#ifndef _myFunch_
#define _myFunch_

#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

void configThisSlave();
void handleRelay();
void report2Master();
void handleMessage();
void handleRF24();


#endif
