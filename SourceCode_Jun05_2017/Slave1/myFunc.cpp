#include "myFunc.h"




//-----------------------------------------------
//                GENERAL DEFINITIONS
//-----------------------------------------------
//DEBUG PRINTING
#define DEGUB_PRINTING false      //UART Information
#define GPIO_LED1 6               //ON When Presence Detected (with Debouncer to go OFF)
#define GPIO_LED2 5               //ON When Presence Detected (without Debouncer)

//MOTION SENSOR CONFIG
#define SECONDS_2_KEEP_VALUE 240  //SECONDS FOR 'DEBOUNCER TO GO OFF' CONFIGURATION
#define GPIO_PIR_SENSOR 7         //PIR MOTION SENSOR INPUT PIN
bool oldPresenceDetected = false; //Changes directly with PIR sensor
bool newPresenceDetected = false; //Has a debouncer of SECONDS_2_KEEP_VALUE seconds to turn false
uint8_t presenceValue   = 0;      //PIR Sensor ADC
uint8_t secondsCounter  = 0;      //Debouncer Counter
//-----------------------------------------------





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





//-----------------------------------------------
//                   FUNCTIONS
//-----------------------------------------------
void configThisSlave()
{
  //--------------------------------------------
  //             GPIO AND UART SETUP
  //--------------------------------------------
  //MOTION SENSOR PIN CONFIG
  pinMode(GPIO_PIR_SENSOR, INPUT); //Define pino sensor como entrada
  pinMode(GPIO_LED1, OUTPUT);      //ON When Presence Detected (with Debouncer to go OFF)
  pinMode(GPIO_LED2, OUTPUT);      //ON When Presence Detected (without Debouncer)
  
  //SERIAL FOR DEBUG PRINTING CONFIG
  if( DEGUB_PRINTING ){ Serial.begin(9600); }
  //--------------------------------------------
  

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


  //--------------------------------------------
  //             TIMER CONFIGURATION
  //--------------------------------------------
  noInterrupts();
  TCCR1A = B00000000;//Register A all 0's since we're not toggling any pins
    // TCCR1B clock prescalers
    // 1 x x x CTC mode
    // x 0 0 1 clkI/O /1 (No prescaling)
    // x 0 1 0 clkI/O /8 (From prescaler)
    // x 0 1 1 clkI/O /64 (From prescaler)
    // x 1 0 0 clkI/O /256 (From prescaler)
    // x 1 0 1 clkI/O /1024 (From prescaler)
  TCCR1B = B00001101;//bit 3 set for CTC mode, bits 2,1,0 set to 1024 prescaler
  TIMSK1 = B00000010;//bit 1 set to call the interrupt on an OCR1A match
  OCR1A  = (unsigned long)(15625UL); //1 Second = 15625 cicles = 16M / 1024
  interrupts();
  //--------------------------------------------
  
}

void loopThisSlave(){
  
  //READING THE MOTION SENSOR PIN
  presenceValue = digitalRead(GPIO_PIR_SENSOR); 

  //NO MOVEMENT
  if (presenceValue == LOW) 
  {
    if( oldPresenceDetected ){
      oldPresenceDetected = false;
      digitalWrite(GPIO_LED2,LOW);
      if( DEGUB_PRINTING ){ Serial.println("[NO DEBOUNCER] NO MOVEMENT DETECTED!"); }
    }
    
    if( (newPresenceDetected == true) and (secondsCounter >= SECONDS_2_KEEP_VALUE) ){
      if( DEGUB_PRINTING ){ Serial.println("[WITH DEBOUNCER] NO MOVEMENT DETECTED!"); }
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
      if( DEGUB_PRINTING ){ Serial.println("[NO DEBOUNCER] MOVEMENT DETECTED!"); }
    }
    
    if( newPresenceDetected == false ){
      if( DEGUB_PRINTING ){ Serial.println("[WITH DEBOUNCER] MOVEMENT DETECTED!"); }
      newPresenceDetected = true;
      digitalWrite(GPIO_LED1,HIGH);
      //report2Master();
    }
  }

  //ANSWER MASTER
  handleRF24();

  //JUST TO SAVE ENERGY
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

  if( DEGUB_PRINTING ){ Serial.print( "Got: " ); Serial.println( messageRF24 ); }
  
  //HANDLING VALID MESSAGES
  if( (messageRF24[0] == START_CHAR) and (messageRF24[5] == END_CHAR) ){
    if( messageRF24.substring(1,5) == "MS1?" ){
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
  if( DEGUB_PRINTING ){ Serial.println("  Writing Message"); }
  radio.write( message, sizeof(message) );
  radio.startListening();
  
  if( DEGUB_PRINTING ){ 
    Serial.println(" ");
    Serial.print("Text Sent: ");
    Serial.println( message );
  }
  
}

ISR(TIMER1_COMPA_vect){
  if( secondsCounter < SECONDS_2_KEEP_VALUE ){
    secondsCounter++;
  }
}
//-----------------------------------------------
