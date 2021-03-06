/*

  Slave 3
  IR Receiver
  IR Emitter (In Development)

  -----------------------------------------------
 | PIN |  PURPOSE                                |
 |-----|-----------------------------------------|
 | 5   |  IR  EMITTER                            |
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
#include <IRremote.h>
#include <math.h>
//-----------------------------------------------


//-----------------------------------------------
//               NRF24L01+ LIBRARY
//-----------------------------------------------
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
//-----------------------------------------------


//-----------------------------------------------
//                   IR CONFIG
//-----------------------------------------------
#define GPIO_IR_RECV 7
#define GPIO_IR_EMIT 5
#define GPIO_IR_LED  6
IRrecv irrecv(GPIO_IR_RECV);
decode_results resultsIR;
unsigned int rawCodes[RAWBUF]; // The durations if raw
int codeLen;                   // The length of the code
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
  pinMode(GPIO_IR_LED, OUTPUT);
  digitalWrite( GPIO_IR_LED, LOW );
  //-----------------------------------------------


  //--------------------------------------------
  //             RF24 CONFIGURATION
  //--------------------------------------------
  //Start Communication
  radio.begin();

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
  if( (messageRF24[0] == START_CHAR) and (messageRF24[6] == END_CHAR) ){
    if( messageRF24.substring(1,6) == "MS301" ){
      if( DEGUB_PRINTING ){ Serial.println("Send IR CMD 01"); }
    }
  }
  
}

void report2Master(uint8_t buttonPressed){
  
  char message[8] = "#S3M00#";
  message[0]      = START_CHAR;
  message[6]      = END_CHAR;
  message[7]      = '\0';

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
