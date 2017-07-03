#include "myFunc.h"


//-----------------------------------------------
//                    PARAMETERS
//-----------------------------------------------
//DEBUG PRINTING
#define DEBUG_PRINTING true

//LAMP CONFIGURATION
#define GPIO_RELAY_OUT 6
bool motionSensorState = false;
uint8_t configState = 0;

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
const uint64_t pipes[2] = { 0xABCDA00 , 0xABCDA02 }; 

#define START_CHAR 2
#define END_CHAR   3
String messageRF24;
//-----------------------------------------------


void handleRelay(){
  static uint8_t lastConfigState = 255;

  if( lastConfigState != configState ){
    lastConfigState = configState;
    if(       configState == 0 ){
      digitalWrite( GPIO_RELAY_OUT, LOW  );
    }else if( configState == 1 ){
      digitalWrite( GPIO_RELAY_OUT, HIGH );
    }else if( configState == 2 ){
      if( motionSensorState ){
        digitalWrite( GPIO_RELAY_OUT, HIGH );
      }else{
        digitalWrite( GPIO_RELAY_OUT, LOW  );
      }
    }
  }
}

void configThisSlave(){
  
  //DEBUG PRINTING CONFIG
  if( DEBUG_PRINTING ){
    Serial.begin( 9600 );
  }

  //RELAY CONFIG
  pinMode( GPIO_RELAY_OUT, OUTPUT );
  
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

  configState = EEPROM.read(0);delay(50);
  motionSensorState = EEPROM.read(1);
}


void handleRF24(){
  //IS THERE RADIO SIGNAL INCOMING?
  if (radio.available())
  {
    char recebidos[50] = "";
    radio.read( recebidos , 49 );
    messageRF24 = recebidos;
    if( DEBUG_PRINTING ){ Serial.print("IN: \"");Serial.print(messageRF24);Serial.println("\""); }
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
    if( messageRF24.substring(1,4) == "MS2" ){
      
      if( messageRF24.substring(4,5) == "?" ){
        if( DEBUG_PRINTING ){ Serial.println("Sup Master"); }
        report2Master();
      }
  
      else if( messageRF24.substring(4,5) == "0" ){
        configState = 0;
      }

      else if( messageRF24.substring(4,5) == "1" ){
        configState = 1;
      }

      else if( messageRF24.substring(4,5) == "2" ){
        configState = 2;
      }

      else if( messageRF24.substring(4,5) == "3" ){
        motionSensorState = false;
      }

      else if( messageRF24.substring(4,5) == "4" ){
        motionSensorState = true;
      }

      EEPROM.write(0 , configState       );
      EEPROM.write(1 , motionSensorState );
      
    }
  }
}

void report2Master(){
  
  char message[7] = "#S2M0#";
  message[0] = START_CHAR;
  message[4] = 48 + configState;  
  message[5] = END_CHAR;
  message[6] = '\0';

  radio.stopListening();
  radio.openWritingPipe( pipes[0] );
  radio.write( message, sizeof(message) );
  radio.startListening();
  
  if( DEBUG_PRINTING ){ 
    Serial.println(" ");
    Serial.print("Text Sent: ");
    Serial.println( message );
  }
}



