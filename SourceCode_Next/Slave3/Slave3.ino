/*
 Copyright (C) 2017 Wellington Rodrigo Gallo <w.r.gallo@grad.ufsc.br>
  This program is a free software; 
  You can:
    * redistribute it
    * modify it
  Under the terms of the GNU General Public License
  published by the Free Software Foundation.

  Slave 3
  IR Receiver
  IR Emitter (In Development)

  -----------------------------------------------
 | PIN |  PURPOSE                                |
 |-----|-----------------------------------------|
 | 3   |  IR  EMITTER                            |
 | 6   |  LED (ON WHEN VALID BUTTON IS PRESSED)  |
 | 7   |  IR  RECEIVER IN                        |
 | 9   |  CE   of NRF24L01+                      |
 | 10  |  CSN  of NRF24L01+                      |
 | 11  |  MOSI of NRF24L01+                      |
 | 12  |  MISO of NRF24L01+                      |
 | 13  |  SCK  of NRF24L01+                      |
  -----------------------------------------------

  ---------------------------------------------
 | Power Supply  | Peripheral                  |
 |---------------|-----------------------------|
 | +5V           |  IR EMITTER                 |
 | +5V           |  LED                        |
 | +5V           |  IR RECEIVER                |
 | +3v3          |  NRF24L01+                  |
  ---------------------------------------------

*/
//-----------------------------------------------
//                   IR LIBRARY
//-----------------------------------------------
/*math
  Purpose:      Needed for pow function only, when converting incoming bits to decimal value
  License:      GNU Lesser General Public License
                Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include <math.h>

/*IRremote
  Purpose:      Needed to Get and Send IR Data
  Source:       https://github.com/z3t0/Arduino-IRremote
  License:      GNU Lesser General Public License version 2.1
                Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include <IRremote.h>
//-----------------------------------------------


//-----------------------------------------------
//               NRF24L01+ LIBRARY
//-----------------------------------------------
/*SPI
  Purpose:      Needed for SPI communication with nRF24L01+
  License:      GNU Lesser General Public License version 2 and version 2.1
                Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include <SPI.h>

/*nRFL01
  Purpose:      Needed for SPI communication with nRF24L01+
  Source:       https://github.com/nRF24/RF24
  Author:       Copyright (c) 2007 Stefan Engelke <mbox@stefanengelke.de>
                Portions Copyright (C) 2011 Greg Copeland
                
  License:      Permission is hereby granted, free of charge, to any person
                obtaining a copy of this software and associated documentation
                files (the "Software"), to deal in the Software without
                restriction, including without limitation the rights to use, copy,
                modify, merge, publish, distribute, sublicense, and/or sell copies
                of the Software, and to permit persons to whom the Software is
                furnished to do so, subject to the following conditions:
                
                The above copyright notice and this permission notice shall be
                included in all copies or substantial portions of the Software.
*/
#include "nRF24L01.h"

/*RF24
  Purpose:      Needed for SPI communication with nRF24L01+
  Source:       https://github.com/nRF24/RF24
  License:      GNU Lesser General Public License version 2
                Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "RF24.h"
//-----------------------------------------------


//-----------------------------------------------
//                   IR CONFIG
//-----------------------------------------------
#define GPIO_IR_RECV 7
#define GPIO_IR_LED  6
IRrecv irrecv(GPIO_IR_RECV);
IRsend irsend;
decode_results resultsIR;
unsigned int rawCodes[RAWBUF]; // The durations if raw
int codeLen;                   // The length of the code

/**
 * Consul Power Button - Pre Configured
 * Mode:         Refrigerate
 * Temperature:  20ºC
 * Cooler Speed: Max
 */
const unsigned int CONSUL_POWER[] = { 8700 , 4750 , 250 , 1950 , 300 , 1900 , 300 , 850 , 300 , 900 , 250 , 900 , 250 , 850 , 300 , 850 , 300 , 1900 , 350 , 850 , 300 , 1850 , 350 , 1900 , 300 , 900 , 250 , 900 , 300 , 850 , 200 , 850 , 350 , 850 , 300 , 850 , 300 , 850 , 300 , 1950 , 250 , 850 , 300 , 900 , 250 , 900 , 250 , 850 , 300 , 900 , 250 , 900 , 250 , 1950 , 300 , 800 , 350 , 800 , 350 , 850 , 300 , 1900 , 300 , 850 , 300 , 850 , 300 , 900 , 250 , 850 , 300 , 850 , 300 , 850 , 300 , 800 , 350 , 800 , 350 , 850 , 300 , 750 , 250 , 900 , 300 , 800 , 350 , 850 , 300 , 850 , 300 , 850 , 300 , 850 , 300 , 850 , 300 , 850 , 300 };
const unsigned int SONY_POWER[]   = { 2200 ,  800 , 1000 ,  750 , 450 ,  750 , 1000 , 800 , 400 , 800 , 1000 , 800 ,  400  , 800 , 400 , 750 , 1050 ,  750 , 400 , 800 , 400 ,  800 , 400 ,  800 , 400 };
const unsigned int SONY_INPUT[]   = { 2250 ,  750 , 1050 ,  750 , 450 ,  700 , 1100 , 750 , 450 , 700 ,  450 , 750 , 1050  , 700 , 500 , 750 , 1050 ,  750 , 450 , 700 , 500 ,  700 , 450 ,  750 , 450 };
//-----------------------------------------------

#define DEGUB_PRINTING true

//-----------------------------------------------
//                   RF24 CONFIG
//-----------------------------------------------
//Start NRF24L01+ with CE and CS on pins 9 and 10
RF24 radio(9,10);

// Radio pipe addresses for the nodes to communicate.
/*
 * MASTER UNIT PIPE  0xABCDA00
 * SLAVE 01 PIPE     0xABCDA01
 * SLAVE 02 PIPE     0xABCDA02
 * ...
 * SLAVE 99 PIPE     0xABCDA99
 */
const uint64_t pipes[2] = { 0xABCDA00 , 0xABCDA03 }; 

#define START_CHAR 2
#define END_CHAR   3
String messageRF24;
//-----------------------------------------------


void setup()
{
  
  //-----------------------------------------------
  //                   IR SETUP
  //-----------------------------------------------
  if( DEGUB_PRINTING ){ Serial.begin(9600); }
  irrecv.enableIRIn(); // Start the receiver
  pinMode(GPIO_IR_LED , OUTPUT);digitalWrite( GPIO_IR_LED , LOW  );
  //-----------------------------------------------


  //--------------------------------------------
  //             RF24 CONFIGURATION
  //--------------------------------------------
  //Start Communication
  radio.begin();
  radio.setPALevel( RF24_PA_LOW );
  radio.setDataRate( RF24_250KBPS );

  //Select Writting and Reading Pipe Addresses
  radio.openWritingPipe(     pipes[0] ); //When Answering the Master
  radio.openReadingPipe( 1 , pipes[1] ); //The Pipe where Master Answers This Slave
  radio.startListening();                // Start listening
  //--------------------------------------------
  
}

void loop() {

  //-----------------------------------------------
  //                   IR HANDLER
  //-----------------------------------------------
  if ( irrecv.decode(&resultsIR) ) {
    storeCode(&resultsIR);
    irrecv.resume(); // resume receiver
  }
  //-----------------------------------------------

  handleRF24();
  
}


//-----------------------------------------------
//                   IR FUNCTIONS
//-----------------------------------------------
void storeCode(decode_results *resultsIR) {
  int count = resultsIR->rawlen;
  codeLen = resultsIR->rawlen - 1;
  
  if( (codeLen == 67) and (count == 68) ){
    uint8_t myValueNEC[8] = {255, 255, 255, 255, 255, 255, 255, 255};
    uint8_t thisValueNEC  = 0;
    uint8_t i = 0, j = 0, k = 0;
    for (i = 1; i <= codeLen; i++) {
      if (i % 2) {
        rawCodes[i - 1] = resultsIR->rawbuf[i]*USECPERTICK - MARK_EXCESS;
      } 
      else {
        rawCodes[i - 1] = resultsIR->rawbuf[i]*USECPERTICK + MARK_EXCESS;
      }
      if( (i > 2) and (i < 67) ){
        j++;
        if( rawCodes[i - 1] > 1500 ){
          thisValueNEC += pow( 2, (j-1) );
        }
        if( j == 8 ){
          j = 0;
          myValueNEC[k++] = thisValueNEC;
        }
      }
    }

    uint8_t buttonPressed = checkButton( myValueNEC );

    if( buttonPressed != 255 ){
      if( DEGUB_PRINTING ){
        Serial.println("");
        Serial.print("Button Pressed: ");
        Serial.println( buttonPressed, DEC );
      }
      digitalWrite( GPIO_IR_LED, HIGH );
      delay(250);
      digitalWrite( GPIO_IR_LED, LOW );
      report2Master( buttonPressed );
    }
    
  }
}

uint8_t checkButton(uint8_t myValueNEC[]){
  uint8_t button = 255; //INVALID BUTTON DETECTED
  uint64_t n = *((uint64_t*) myValueNEC);
  
  if(      n == 0x81D8303030A70000 ){ button = 1; } //BUTTON 1
  else if( n == 0x81D8323230A70000 ){ button = 2; }
  else if( n == 0x81D8383830A70000 ){ button = 3; }
  else if( n == 0x81D84F4F30A70000 ){ button = 4; }
  else if( n == 0x81D8515130A70000 ){ button = 5; }
  else if( n == 0x81D8575730A70000 ){ button = 6; }
  else if( n == 0x81D8AFAF30A70000 ){ button = 7; }
  else if( n == 0x81D8B1B130A70000 ){ button = 8; }
  else if( n == 0x81D8B7B730A70000 ){ button = 9; }
  else if( n == 0x81D8D0D030A70000 ){ button = 0; }
  else if( n == 0x81D8CECE30A70000 ){ button = 10; } //BUTTON *
  else if( n == 0x81D8D6D630A70000 ){ button = 11; } //BUTTON #
  else if( n == 0x81DA343230A70000 ){ button = 12; } //BUTTON UP
  else if( n == 0x81DA514F30A70000 ){ button = 13; } //BUTTON LEFT
  else if( n == 0x81DA595730A70000 ){ button = 14; } //BUTTON RIGHT
  else if( n == 0x81DAB3B130A70000 ){ button = 15; } //BUTTON DOWN
  else if( n == 0x81DA535130A70000 ){ button = 16; } //BUTTON OK
  
  return button;
}

void sendIRCmd(uint8_t cmd){
  
  if( cmd == 1 ){
    const int khz = 38;
    irsend.sendRaw(CONSUL_POWER, 99, khz);
  }
  else if( cmd == 2 ){
    const int khz = 38;
    irsend.sendRaw(SONY_POWER, 25, khz);
  }
  else if( cmd == 3 ){
    const int khz = 38;
    irsend.sendRaw(SONY_INPUT, 25, khz);
  }
  
}
//-----------------------------------------------





//-----------------------------------------------
//                 RF24 FUNCTIONS
//-----------------------------------------------
void handleRF24(){
  //IS THERE RADIO SIGNAL INCOMING?
  if (radio.available())
  {
    char recebidos[50] = "";
    radio.read( recebidos , 49 );
    messageRF24 = recebidos;
    handleMessage();
  }
}

void handleMessage(){
  
  //DUMPING INVALID CHARS
  while( (messageRF24[0] != START_CHAR) and (messageRF24.length() > 1) ){
    messageRF24 = messageRF24.substring(1);
  }

  //HANDLING VALID MESSAGES
  if( (messageRF24[0] == START_CHAR) ){
    if( (messageRF24[6] == END_CHAR) ){
      if( messageRF24.substring(1,6) == "MS301" ){
        if( DEGUB_PRINTING ){ Serial.println("Send IR CMD 01"); }
        sendIRCmd( 1 );
      }
      else if( messageRF24.substring(1,6) == "MS302" ){
        if( DEGUB_PRINTING ){ Serial.println("Send IR CMD 02"); }
        sendIRCmd( 2 );
      }
      else if( messageRF24.substring(1,6) == "MS303" ){
        if( DEGUB_PRINTING ){ Serial.println("Send IR CMD 03"); }
        sendIRCmd( 3 );
      }
    }
    else if( (messageRF24[5] == END_CHAR) ){
      if( messageRF24.substring(1,5) == "MS3?" ){
        report2Master( 99 );
      }
    }
  }
}

void report2Master(uint8_t buttonPressed){
  if( buttonPressed > 99 ){return;}
  
  char message[8] = "#S3M00#";
  message[0]      = START_CHAR;
  message[6]      = END_CHAR;
  message[7]      = '\0';
  
  message[4] = 48 + (buttonPressed / 10);
  message[5] = 48 + (buttonPressed % 10);
  
  /*
  if(      buttonPressed < 10 ){ message[4] = '0'; }
  else{                          message[4] = '1'; }
  uint8_t secondDigit = buttonPressed % 10;
  if(      secondDigit == 1 ){ message[5] = '1'; }
  else if( secondDigit == 2 ){ message[5] = '2'; }
  else if( secondDigit == 3 ){ message[5] = '3'; }
  else if( secondDigit == 4 ){ message[5] = '4'; }
  else if( secondDigit == 5 ){ message[5] = '5'; }
  else if( secondDigit == 6 ){ message[5] = '6'; }
  else if( secondDigit == 7 ){ message[5] = '7'; }
  else if( secondDigit == 8 ){ message[5] = '8'; }
  else if( secondDigit == 9 ){ message[5] = '9'; }
  else{                        message[5] = '0'; }
  */
  
  radio.stopListening();
  radio.write( message, sizeof(message) );
  radio.startListening();
  
  if( DEGUB_PRINTING ){ 
    Serial.println(" ");
    Serial.print("Text Sent: ");
    Serial.println( message );
  }
}
//-----------------------------------------------
