#include "myJoystick.h"

joyMeaning joyValueNew=0, joyValueOld=0;
volatile uint16_t joyRxValue, joyRyValue;
volatile uint8_t joySequence[J_SEQUENCE_LENGTH];
volatile uint8_t joyPosition = 0;
volatile bool joyPasswordCorrect = false;

void beginJoystick() {
  pinMode( J_RX, INPUT );
  pinMode( J_RY, INPUT );
  pinMode( J_SW, INPUT_PULLUP );

  joyPosition = 0;
  for( joyPosition = 0; joyPosition < J_SEQUENCE_LENGTH; joyPosition++ ){joySequence[ joyPosition ] = 0;}
  joyPosition = 0;
  attachInterrupt( digitalPinToInterrupt(J_SW), joySWHandler, CHANGE  );

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
  TCCR1B = B00001100;
  TIMSK1 = B00000010;//bit 1 set to call the interrupt on an OCR1A match
  OCR1A  = (unsigned long)(1250UL); // cicles = 16e6 / (frequency * prescaler)
  interrupts();
  //--------------------------------------------
}

void printAnalogValues(){
  Serial.print("Rx: ");Serial.print(joyRxValue);
  Serial.write( 9 );
  Serial.print("Ry: ");Serial.println(joyRyValue);
}

void printSequence() {
  
  Serial.print("Sequence: {");
  int i = 0;
  for( i=0; i<J_SEQUENCE_LENGTH-1; i++ ){ Serial.print( joySequence[i] ); Serial.print(" ; "); }
  Serial.print( joySequence[J_SEQUENCE_LENGTH-1] ); Serial.print(" } ");
  Serial.println("");
  
}

uint8_t getSequenceLen(){
  uint8_t zeroCounter = 0;
  int i;
  for(i=0 ; i<J_SEQUENCE_LENGTH; i++){
    if( joySequence[i] == 0 ){
      zeroCounter++;
    }
  }
  return (J_SEQUENCE_LENGTH - zeroCounter );
}

uint8_t getJoyValue( uint8_t pos ){
  if( pos < J_SEQUENCE_LENGTH ){
    return joySequence[ pos ];
  }
  return 10;
}

void getJoyValues(){
  noInterrupts(); 
  joyRxValue = analogRead( J_RX );
  joyRyValue = analogRead( J_RY );
  interrupts();
  
  /*
   *  ----------------------------------
   * | joyXValue | joyYValue | joyValue |
   *  ----------------------------------
   * | IDLE      | IDLE      | 0        |
   * | IDLE      | UP        | 1        |
   * | IDLE      | DOWN      | 2        |
   * | LEFT      | IDLE      | 3        |
   * | LEFT      | UP        | 4        |
   * | LEFT      | DOWN      | 5        |
   * | RIGHT     | IDLE      | 6        |
   * | RIGHT     | UP        | 7        |
   * | RIGHT     | DOWN      | 8        |
   * -----------------------------------
   */
  
  
  uint8_t joyXValue = 0, joyYValue = 0;
  //LEFT UP
  if( ((joyRxValue <= J_ACCEPTABLE_MIN) and (joyRyValue <= J_DEADZONE_Y_MIN)) or 
      ((joyRxValue <= J_DEADZONE_X_MIN) and (joyRyValue <= J_ACCEPTABLE_MIN)) ){
    joyXValue = 1;
    joyYValue = 1;
  }

  //LEFT DOWN
  else if( ((joyRxValue <= J_ACCEPTABLE_MIN) and (joyRyValue >= J_DEADZONE_Y_MAX)) or 
           ((joyRxValue <= J_DEADZONE_X_MIN) and (joyRyValue >= J_ACCEPTABLE_MAX)) ){
    joyXValue = 1;
    joyYValue = 2;
  }
  
  //RIGHT UP
  else if( ((joyRxValue >= J_ACCEPTABLE_MAX) and (joyRyValue <= J_DEADZONE_Y_MIN)) or 
           ((joyRxValue >= J_DEADZONE_X_MAX) and (joyRyValue <= J_ACCEPTABLE_MIN)) ){
    joyXValue = 2;
    joyYValue = 1;
  }

  //RIGHT DOWN
  else if( ((joyRxValue >= J_ACCEPTABLE_MAX) and (joyRyValue >= J_DEADZONE_Y_MAX)) or 
           ((joyRxValue >= J_DEADZONE_X_MAX) and (joyRyValue >= J_ACCEPTABLE_MAX)) ){
    joyXValue = 2;
    joyYValue = 2;
  }

  //UP or DOWN
  else if( (joyRxValue > J_DEADZONE_X_MIN) and (joyRxValue < J_DEADZONE_X_MAX) ){
    joyXValue = 0;
    if (      joyRyValue <= J_ACCEPTABLE_MIN ){ joyYValue = 1; }
    else if ( joyRyValue >= J_ACCEPTABLE_MAX ){ joyYValue = 2; }    
  }
  
  //LEFT or RIGHT
  else if( (joyRyValue > J_DEADZONE_Y_MIN) and (joyRyValue < J_DEADZONE_Y_MAX) ){
    joyYValue = 0;
    if (      joyRxValue <= J_ACCEPTABLE_MIN ){ joyXValue = 1; }
    else if ( joyRxValue >= J_ACCEPTABLE_MAX ){ joyXValue = 2; }    
  }
  
  joyValueOld = joyValueNew;
  joyValueNew = 3*joyXValue + joyYValue;
  if( joyValueNew > 0 ){
    
    if( J_SMOOTH_MOVEMENT ){
      if( joyValueOld != joyValueNew ){
        joySequence[ joyPosition++ ] = joyValueNew;
        if(joyPosition == J_SEQUENCE_LENGTH){ joyPosition = 0; }
        
        //printSequence();printAnalogValues();
      }
    }else{
      if( joyValueOld == 0 ){
        joySequence[ joyPosition++ ] = joyValueNew;
        if(joyPosition == J_SEQUENCE_LENGTH){ joyPosition = 0; }

        //printSequence();printAnalogValues();
      }
    }
  }
  
}

bool joyCheckPassword(){
  if( joyPasswordCorrect ){
    joyPasswordCorrect = false;
    return true;
  }
  return false;
}

void joySWHandler(){
  bool correctPassword = true;
  int i = 0;
  for( i=0; i<J_SEQUENCE_LENGTH; i++ ){
    if( joySequence[i] != J_PASWWORD[i] ){ correctPassword = false; }
  }
  if( correctPassword ){ joyPasswordCorrect = true; }
  for( i=0; i<J_SEQUENCE_LENGTH; i++ ){ joySequence[i] = 0; }
  joyPosition = 0;
}

ISR(TIMER1_COMPA_vect){
  if( !joyPasswordCorrect ){ getJoyValues(); }
  joyTimerHandler();
}
