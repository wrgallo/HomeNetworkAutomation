#include "myEEPROM.h"

#define START_CHAR 2 //DEC NUMBER OF ASCII TABLE
#define END_CHAR   3 //DEC NUMBER OF ASCII TABLE

#define WAIT_MS 30

enum byteMeaning{
  byteNightTime = 0,
  byteLampState,
  byteMotionState,
  byteAlarm0
};

//------------------------------------------------------------------------------------------
//                                     private functions
//------------------------------------------------------------------------------------------
String lookFor( byteMeaning byteMeaningValue );
void writeThis( byteMeaning byteMeaningValue, String stringToWrite   );
uint16_t getGoodPosition( byteMeaning byteMeaningValue  );

String lookFor( byteMeaning byteMeaningValue ){
  String output = "";
  uint16_t address = 0;
  byte value = 0;
  bool founded = false;
  
  //FINDING STRING
  while( (address < EEPROM_LENGTH) and (!founded) ){
    value = EEPROM.read(address++);delay(WAIT_MS);
    
    if( address == EEPROM_LENGTH ){
      output = "EEPROM Data Not Found";
      Serial.println();
      Serial.println(output);
      return output;
    }

    //FINDING START_CHAR
    if( value == START_CHAR ){
      value = EEPROM.read(address++);delay(WAIT_MS);
      
      //THIS IS THE CORRECT START_CHAR
      if( value == byteMeaningValue ){
        
        //FINDING THE REST OF THE MSG
        while( (address < EEPROM_LENGTH) ){
          value = EEPROM.read(address++);delay(WAIT_MS);
          if( value == END_CHAR ){
            founded = true;
            break;
          }
          else if( (value > 31) and (value < 127) ){
            output += (char)(value);
          }
        }
      }
    }
  }
  return output;
}

void writeThis( byteMeaning byteMeaningValue, String stringToWrite   ){
  uint16_t address;
  address = getGoodPosition( byteMeaningValue  );
  
  if( address < EEPROM_LENGTH ){
    int maxLength = 0;
    
    if(       byteMeaningValue == byteNightTime      ){ maxLength = NIGHTTIME_LEN; }
    else if(  byteMeaningValue == byteLampState      ){ maxLength = LAMPSTATE_LEN; }
    else if(  byteMeaningValue == byteMotionState    ){ maxLength = MOTIONSTATE_LEN; }
    else if(  byteMeaningValue == byteAlarm0         ){ maxLength = ALARM_LEN; }
    
    if( stringToWrite.length() == maxLength ){
      
      EEPROM.write(address++, START_CHAR       );delay(WAIT_MS);
      EEPROM.write(address++, byteMeaningValue );delay(WAIT_MS);
      int i;
      for( i=0; i < stringToWrite.length(); i++ ){
        EEPROM.write( address++, stringToWrite.charAt(i) );delay(WAIT_MS);
      }
      EEPROM.write(address++, END_CHAR );delay(WAIT_MS);
    }
    
  }
}

uint16_t getGoodPosition( byteMeaning byteMeaningValue  ){
  /*
   * byteNightTime = 0,
   * byteLampState,
   * byteMotionState,
   * byteAlarm0
   */
  
  if(       byteMeaningValue == byteNightTime      ){ return 0; }
  else if(  byteMeaningValue == byteLampState      ){ return (3 + NIGHTTIME_LEN); }
  else if(  byteMeaningValue == byteMotionState    ){ return (3*2 + NIGHTTIME_LEN + LAMPSTATE_LEN); }
  else if(  byteMeaningValue == byteAlarm0         ){ return (3*3 + NIGHTTIME_LEN + LAMPSTATE_LEN + MOTIONSTATE_LEN); }
  
  return EEPROM_LENGTH;
}
//------------------------------------------------------------------------------------------





//------------------------------------------------------------------------------------------
//                                     public functions
//------------------------------------------------------------------------------------------
void setThisByteToAll( byte value ){
  uint16_t address=0;
  
  while( (address < EEPROM_LENGTH) ){
    EEPROM.write(address++, value );delay(WAIT_MS);
  }
}

void readEEPROM(bool detailed){
  uint16_t address = 0;
  byte value;

  if( detailed ){
    Serial.println();
    Serial.println("------------------------------------");
    Serial.println("ADDR\tDEC\tCHAR");
  }
  while( address < EEPROM_LENGTH ){
    // read a byte from the current address of the EEPROM
    value = EEPROM.read(address);

    if( detailed ){
      delay( 100 );
      Serial.print(address);
      Serial.print("\t");
      Serial.print(value, DEC);
      Serial.print("\t");
    }else{
      delay( WAIT_MS );
    }
    
    Serial.write(value);
    
    if( detailed ){Serial.println();}
  
    // advance to the next address of the EEPROM
    address = address + 1;
  }
}


//-------------------------------------------
//               get functions
//-------------------------------------------
void getNightTime(uint8_t* nightTimeStart, uint8_t* nightTimeEnd, bool* set){
  String text = lookFor( byteNightTime );

  nightTimeStart[0] = (text.substring(0,2)).toInt();
  nightTimeStart[1] = (text.substring(2,4)).toInt();
  nightTimeEnd[0]   = (text.substring(4,6)).toInt();
  nightTimeEnd[1]   = (text.substring(6,8)).toInt();
  *set              = (text.substring(8,9)).toInt();
  
}

void getLampState(uint8_t* state){
  String text = lookFor( byteLampState );
  *state = text.toInt();
}

void getMotionState(bool* state){
  String text = lookFor( byteMotionState );
  *state = text.toInt();
}


void getAlarm0(uint8_t* alarmTime, uint8_t* alarmWeekday, bool* set){
  String text = lookFor( byteAlarm0 );

  alarmTime[0]   = (text.substring(0,2)).toInt();
  alarmTime[1]   = (text.substring(2,4)).toInt();
  alarmWeekday   = (text.substring(4,5)).toInt();
  *set           = (text.substring(5,6)).toInt();
}

//-------------------------------------------


//-------------------------------------------
//               set functions
//-------------------------------------------
void setNightTime(uint8_t* nightTimeStart, uint8_t* nightTimeEnd, bool set){
  String values = "ABCDEFGHI";
  
  if( nightTimeStart[0] < 10 ){
    values.replace("A","0");
    values.replace("B",(String)(nightTimeStart[0]));
  }
  else{
    values.replace("AB",(String)(nightTimeStart[0]));
  }
  
  if( nightTimeStart[1] < 10 ){
    values.replace("C","0");
    values.replace("D",(String)(nightTimeStart[1]));
  }
  else{
    values.replace("CD",(String)(nightTimeStart[1]));
  }
  

  if( nightTimeEnd[0] < 10 ){
    values.replace("E","0");
    values.replace("F",(String)(nightTimeEnd[0]));
  }
  else{
    values.replace("EF",(String)(nightTimeEnd[0]));
  }
  
  if( nightTimeEnd[1] < 10 ){
    values.replace("G","0");
    values.replace("H",(String)(nightTimeEnd[1]));
  }
  else{
    values.replace("GH",(String)(nightTimeEnd[1]));
  }

  if( set ){
    values.replace("I","1");
  }
  else{
    values.replace("I","0");
  }
  
  writeThis( byteNightTime , values   );
}

void setLampState(uint8_t value){
  writeThis( byteLampState , (String)(value)  );
}

void setMotionState(bool value){
  if( value ){
    writeThis( byteMotionState , "1"   );
  }
  else{
    writeThis( byteMotionState , "0"   );
  }
}

void setAlarm0(uint8_t* alarmTime, uint8_t alarmWeekday, bool set){
  String values = "ABCDEF";
  
  if( alarmTime[0] < 10 ){
    values.replace("A","0");
    values.replace("B",(String)(alarmTime[0]));
  }
  else{
    values.replace("AB",(String)(alarmTime[0]));
  }
  
  if( alarmTime[1] < 10 ){
    values.replace("C","0");
    values.replace("D",(String)(alarmTime[1]));
  }
  else{
    values.replace("CD",(String)(alarmTime[1]));
  }
  

  if( alarmWeekday <= 6 ){
    values.replace("E",(String)(alarmWeekday));
  }
  else{
    Serial.println("Invalid alarmWeekday");
    return;
  }
  
  if( set ){
    values.replace("F","1");
  }
  else{
    values.replace("F","0");
  }
  
  writeThis( byteAlarm0 , values   );
}
//-------------------------------------------

//------------------------------------------------------------------------------------------
