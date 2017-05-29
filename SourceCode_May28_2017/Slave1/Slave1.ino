/*

  Slave 1
  Motion Sensor

  ------------------------------------------------------------------
 | PIN |  PURPOSE                                                   |
 |-----|------------------------------------------------------------|
 | 5   |  LED2 OUTPUT (ON WHEN PRESENCE DETECTED) Without Debouncer |
 | 6   |  LED1 OUTPUT (ON WHEN PRESENCE DETECTED) With Debouncer    |
 | 7   |  PIR  SENSOR IN                                            |
 | 9   |  CE   of NRF24L01+                                         |
 | 10  |  CSN  of NRF24L01+                                         |
 | 11  |  MOSI of NRF24L01+                                         |
 | 12  |  MISO of NRF24L01+                                         |
 | 13  |  SCK  of NRF24L01+                                         |
  ------------------------------------------------------------------

  ---------------------------------------------
 | Power Supply  | Peripheral                  |
 |---------------|-----------------------------|
 | +5V           |  LED                        |
 |  +5V          |  PIR SENSOR                 |
 |  +3v3         |  NRF24L01+                  |
  ---------------------------------------------

*/

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

//DEBUG PRINTING
#define DEGUB_PRINTING true
#define GPIO_LED1 6
#define GPIO_LED2 5

//MOTION SENSOR CONFIG
#define SECONDS_2_KEEP_VALUE 240
#define GPIO_PIR_SENSOR 7
bool oldPresenceDetected = false; //Changes directly with PIR sensor
bool newPresenceDetected = false; //Has a debouncer of SECONDS_2_KEEP_VALUE seconds to turn false
uint8_t presenceValue  = 0;       //PIR Sensor ADC
uint8_t secondsCounter = 0;       //Debouncer counter
uint8_t internalCounter = 0;      

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
const uint64_t pipes[2] = { 0xABCDA00 , 0xABCDA01 }; 

#define START_CHAR 2
#define END_CHAR   3
String messageRF24;
//-----------------------------------------------

void setup()
{
  //MOTION SENSOR PIN CONFIG
  pinMode(GPIO_PIR_SENSOR, INPUT); //Define pino sensor como entrada
  pinMode(GPIO_LED1, OUTPUT);      //ON When Presence Detected (with Debouncer)
  pinMode(GPIO_LED2, OUTPUT);      //ON When Presence Detected (without Debouncer)
  
  //SERIAL FOR DEBUG PRINTING CONFIG
  if( DEGUB_PRINTING ){ Serial.begin(9600); }

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
  delay(250);
  
  //Motion Sensor Doesn't Work Properly When Just Powered, Counting 10 Seconds to Start program
  //Blink LEDs 2 Times (Start 10 Seconds Counting)
  digitalWrite(GPIO_LED1,HIGH);digitalWrite(GPIO_LED2,HIGH);delay(250);
  digitalWrite(GPIO_LED1,LOW );digitalWrite(GPIO_LED2,LOW );delay(250);
  digitalWrite(GPIO_LED1,HIGH);digitalWrite(GPIO_LED2,HIGH);delay(250);
  digitalWrite(GPIO_LED1,LOW );digitalWrite(GPIO_LED2,LOW );delay(250);

  delay( 10000 );
  
  //Blink Alternately LEDs 3 Times (Program Starting)
  digitalWrite(GPIO_LED1,HIGH);digitalWrite(GPIO_LED2,LOW  );delay(250);
  digitalWrite(GPIO_LED1,LOW );digitalWrite(GPIO_LED2,HIGH );delay(250);
  digitalWrite(GPIO_LED1,HIGH);digitalWrite(GPIO_LED2,LOW  );delay(250);
  digitalWrite(GPIO_LED1,LOW );digitalWrite(GPIO_LED2,HIGH );delay(250);
  digitalWrite(GPIO_LED1,HIGH);digitalWrite(GPIO_LED2,LOW );delay(250);
  digitalWrite(GPIO_LED1,LOW );delay(250);
  
}

void loop()
{
  
  //READING THE MOTION SENSOR PIN
  presenceValue = digitalRead(GPIO_PIR_SENSOR); 

  //NO MOVEMENT
  if (presenceValue == LOW) 
  {
    if( oldPresenceDetected ){
      oldPresenceDetected = false;
      digitalWrite(GPIO_LED2,LOW);
      if( DEGUB_PRINTING ){ Serial.println("NO MOVEMENT DETECTED!"); }
    }
    
    if( (newPresenceDetected == true) and (secondsCounter >= SECONDS_2_KEEP_VALUE) ){
      newPresenceDetected = false;
      digitalWrite(GPIO_LED1,LOW);
      //report2Master();
      
    }
  }
  //MOVEMENT
  else
  {
    if( oldPresenceDetected == false ){
      oldPresenceDetected = true;
      secondsCounter = 0;
      digitalWrite(GPIO_LED2,HIGH);
      if( DEGUB_PRINTING ){ Serial.println("MOVEMENT DETECTED!"); }
    }
    
    if( newPresenceDetected == false ){
      newPresenceDetected = true;
      digitalWrite(GPIO_LED1,HIGH);
      //report2Master();
      
    }
  }

  handleRF24();
  
  internalCounter++;
  if( internalCounter > 9 ){
    internalCounter = 0;
    secondsCounter++;
    if( secondsCounter < SECONDS_2_KEEP_VALUE ){
      secondsCounter++;
    }
  }
  
  delay( 100 );
  
}

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
  if( (messageRF24[0] == START_CHAR) and (messageRF24[5] == END_CHAR) ){
    if( messageRF24.substring(1,5) == "MS1?" ){
      if( DEGUB_PRINTING ){ Serial.println("Sup Master"); }
      report2Master();
    }
  }
  
}

void report2Master(){
  
  char message[7] = "#S1M0#";
  message[0] = START_CHAR;
  
  if( newPresenceDetected ){
    message[4] = '1';
  }else{
    message[4] = '0';
  }
  
  message[5] = END_CHAR;
  message[6] = '\0';

  radio.stopListening();
  radio.write( message, sizeof(message) );
  radio.startListening();
  
  if( DEGUB_PRINTING ){ 
    Serial.println(" ");
    Serial.print("Text Sent: ");
    Serial.println( message );
  }
}



