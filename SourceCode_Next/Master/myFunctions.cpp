#include "Arduino.h"
#include "myFunctions.h"
#include "myJoystick.h"
#include "myEEPROM.h"

/*
  -----------------------------------------------
 | PIN |  PURPOSE                                |
 |-----|-----------------------------------------|
 | A1  |  ADC JOYSTICK X AXIS                    |
 | A2  |  ADC JOYSTICK Y AXIS                    |
 | A4  |  I2C SDA (LCD DISPLAY AND REAL TIME)    |
 | A5  |  I2C SCL (LCD DISPLAY AND REAL TIME)    |
 | 0   |  UART RX (ESP8266 TX)                   |
 | 1   |  UART TX (ESP8266 RX)                   |
 | 2   |  JOYSTICK SWITCH BUTTON                 |
 | 3   |  SOFT UART RX (HC05 BT TX)              |
 | 4   |  SOFT UART TX (HC05 BT RX)              |
 | 6   |  GPIO OUT BUZZER                        |
 | 9   |  CE   of NRF24L01+                      |
 | 10  |  CSN  of NRF24L01+                      |
 | 11  |  MOSI of NRF24L01+                      |
 | 12  |  MISO of NRF24L01+                      |
 | 13  |  SCK  of NRF24L01+                      |
  -----------------------------------------------
  

  ---------------------------------------------
 | Power Supply  | Peripheral                  |
 |---------------|-----------------------------|
 | +5V           |  HC05                       |
 | +5V           |  LCD                        |
 | +5V           |  REAL TIME MODULE           |
 | +5V           |  BUZZER                     |
 | +5V           |  JOYSTICK                   |
 | +3v3          |  ESP8266                    |
 | +3v3          |  NRF24L01+                  |
  ---------------------------------------------
*/

//---------------------------------------------------------------
//                 GLOBAL VARIABLES DEFINITIONS
//---------------------------------------------------------------
//                           ALARM
//---------------------------------------------------------------
volatile bool firstTime     = false;                             //THE MCU STARTED ONE MINUTE AGO (true or false)
bool     buzzerOn           = false;                             //BUZZER STATUS
bool     alarmSet           = false;                             //ALARM ON/OFF SCHEDULE
bool     buzzerStop         = false;                             //SOMEONE REQUESTED TO STOP THE ALARM
bool     nightMode          = false;                             //NIGHT MODE ON/OFF SCHEDULE (BUZZER GOES ON WITH MOTION SENSOR)
bool     atNightNow         = false;                             //AT NIGHT MODE RIGHT NOW
uint8_t  nightTimeStart[2]  = {22, 26};                          //NIGHT MODE TIME START SCHEDULE
uint8_t  nightTimeEnd[2]    = {5, 59};                           //NIGHT MODE TIME END SCHEDULE
uint8_t  alarmHour[2]       = {0, 33};                           //ALARM HOUR SCHEDULE
uint8_t  alarmWeekday       = 5;                                 //DAY OF WEEK FOR ALARM SCHEDULE (0 = SUN ... 6 = SAT)
uint16_t alarmTimer         = ALARM_TIMER_MAX;                   //TIME COUNTER FOR ALARM CLOCK AUTO GOES OFF
uint16_t intruderAlertTimer = INTRUDER_TIMER_MAX;                //TIME COUNTER FOR INTRUDER ALERT ALARM AUTO GOES OFF

//---------------------------------------------------------------
//                     SLAVE STATUS
//---------------------------------------------------------------
bool     motion1_State = false;                                  //SLAVE1 - MOTION SENSOR
uint8_t  lamp1_State   = 0;                                      //SLAVE2 - LAMP
volatile uint8_t  slavesStatus  = 0b00000000;                    //Each bit correspond to one slave status (Offline/Online)
                                                                 //LSB correspond to Slave 1
uint8_t  slavesStatusCounter[3] = {0,0,0};                        //Seconds Counter without a answer from each slave
//---------------------------------------------------------------
//                             LCD
//---------------------------------------------------------------
LiquidCrystal_I2C lcd(LCDADDR, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);//OBJECT lcd FOR DISPLAY
uint8_t timeSecond;                                              //SECOND VALUE
uint8_t timeMinute;                                              //MINUTE VALUE
uint8_t timeHour;                                                //HOUR VALUE
uint8_t timeWeekday;                                             //DAY OF WEEK VALUE (0 = SUNDAY, 6 = SATURDAY)
uint8_t timeDay;                                                 //DAY VALUE
uint8_t timeMonth;                                               //MONTH VALUE
uint8_t timeYear;                                                //YEAR VALUE (2 DIGITS)
char     msg2[17]      = "                ";                     //MESSAGE FOR SECOND LINE OF DISPLAY
char     msg2_temp[17];                                          //TEMPORARY MESSAGE FOR SECOND LINE OF DISPLAY
uint16_t msg2Timer     = TEMP_MESSAGE_TIMER_MAX;                 //TIME COUNTER FOR TEMPORARY MESSAGE
//---------------------------------------------------------------
//                          BLUETOOTH
//---------------------------------------------------------------
SoftwareSerial btSerial(BT_RX, BT_TX);                           //OBJECT btSerial FOR UART COMMUNICATION WITH HC-05

//---------------------------------------------------------------
//                            WIFI
//---------------------------------------------------------------
bool     stillBuffering = false;                                 //SERVER IS STILL COMMUNICATING WITH THIS UNIT
char     buf1[100];                                              //TO GET THE BYTES OUT OF UART BUFFER WITH ONLY VALID UART VALUES
String   buffer_read = "";                                       //ALL MSGS IN CURRENT BUFFER
uint16_t msgLen = 0;                                             //LENGHT OF BUFFER READ
volatile uint8_t waitingForAnAnswer = WAIT_FOR_AN_ANSWER;        //Waiting Time Counter
volatile bool goAhead = false;                                   //STOP WAITING FOR A SLAVE MESSAGE, MAYBE IT IS OFFLINE
//---------------------------------------------------------------


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
const uint64_t pipes[4] = { 0xABCDA00 , 0xABCDA01, 0xABCDA02, 0xABCDA03 }; 
String messageRF24;
//-----------------------------------------------






//---------------------------------------------------------------
//             FUNCTIONS: ARDUINO CONFIGURATION
//---------------------------------------------------------------
void configThisUnit()
{
  //INPUT | OUTPUT CONFIGURATION
  Serial.begin(115200);
  btSerial.begin(115200);
  pinMode(buzzerGPIO, OUTPUT);digitalWrite(buzzerGPIO, LOW);
  lcd.begin(16, 2);
  Wire.begin();
  
  //SETTING THE DATE AND TIME ON REAL TIME MODULE FOR THE FIRST TIME
  //setTimeDS1307( 17 , 5 , 27, 6, 15, 0 ); //year, month, day, weekday (0 = Sun, 6 = Sat), hour, minute

  //READING DATE AND TIME FROM REAL TIME MODULE
  getTimeDS1307();

  //STARTING MESSAGE FOR THE DISPLAY
  for (int i = 0; i < 3; i++)
  {
    lcd.backlight();
    delay(250);
    lcd.noBacklight();
    delay(250);
  }
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing... ");
  delay(8000);

  lcd.clear();
  lcd.setCursor( 2 , 0);lcd.print(":");
  lcd.setCursor(12 , 0);lcd.print("/");
  setLCD('H');
  setLCD('m');
  setLCD('W');
  setLCD('D');
  setLCD('M');
  
  //--------------------------------------------
  //             RF24 CONFIGURATION
  //--------------------------------------------
  //Start Communication
  radio.begin();

  radio.setPALevel( RF24_PA_LOW   );
  radio.setDataRate( RF24_250KBPS );
  
  //Select Writting and Reading Pipe Addresses
  radio.openReadingPipe( 1 , pipes[0] ); //The Pipe where Slaves Answer Master
  radio.startListening();                // Start listening

  //radio.openWritingPipe( pipes[1] ); //To Talk to Slave 1
  //radio.openWritingPipe( pipes[2] ); //To Talk to Slave 2
  //radio.openWritingPipe( pipes[3] ); //To Talk to Slave 3
  //--------------------------------------------

  beginJoystick();

  //---------------------------------------------------------------
  //                        READING EEPROM DATA
  //---------------------------------------------------------------
  getNightTime( nightTimeStart, nightTimeEnd, &nightMode);
  getLampState(&lamp1_State);
  getMotionState(&motion1_State);
  getAlarm0(alarmHour, &alarmWeekday, &alarmSet);

  //Reporting last saved values to Slave2
  report2Slave2( lamp1_State );
  if( motion1_State ){ report2Slave2( 4 ); }
  else{                report2Slave2( 3 ); }
  //---------------------------------------------------------------
}

void oneSecondTimerHandler()
{
  static uint8_t joySeqLenOld = getSequenceLen();
   
  if( joySeqLenOld != getSequenceLen() ){
    joySeqLenOld = getSequenceLen();
    char joystickSequenceMsg[17] = "Joystick:       ";

    uint8_t i;
    for(i=0; i<joySeqLenOld; i++){
      if( SHOW_PSWD_CHARS ){ joystickSequenceMsg[10+i] = getJoyValue( i ) + 48; }
      else{                  joystickSequenceMsg[10+i] = '*'; }
    }
    strncpy(msg2_temp, joystickSequenceMsg, 17);
    msg2Timer = 0;
  }

  if( joyCheckPassword() ){
    if( buzzerOn ){
      buzzerStop = true; buzzerOn = false; digitalWrite(buzzerGPIO, LOW);
    }
  }
    
  if (msg2Timer < TEMP_MESSAGE_TIMER_MAX)
  {
    msg2Timer++;
  }
  
  if (alarmTimer < ALARM_TIMER_MAX)
  {
    alarmTimer++;
    if (alarmTimer == ALARM_TIMER_MAX )
    {
      if( buzzerOn ){
        buzzerOn = false;
        digitalWrite(buzzerGPIO, LOW);
      }
      if( buzzerStop ){
        buzzerStop = false;
      }
    }
  }

  if (intruderAlertTimer < INTRUDER_TIMER_MAX)
  {
    intruderAlertTimer++;
    if (intruderAlertTimer == INTRUDER_TIMER_MAX )
    {
      if( buzzerOn ){
        buzzerOn = false;
        digitalWrite(buzzerGPIO, LOW);
      }
      if( buzzerStop ){
        buzzerStop = false;
      }
    }
  }

  //Update Slaves Status Information
  uint8_t i;
  for( i=0; i<3; i++ ){
    if (slavesStatusCounter[i] < SLAVE_IS_OFFLINE_TIMER){ slavesStatusCounter[i] +=1; }
    if (slavesStatusCounter[i] == (SLAVE_IS_OFFLINE_TIMER - 1)){ slavesStatus &= ~(1 << i); }
  }


  if (alarmSet)
  {
    if ((alarmWeekday == timeWeekday) and (alarmHour[0] == timeHour) and (alarmHour[1] == timeMinute) and (timeSecond < 50))
    {
      if ((buzzerOn == false) and (buzzerStop == false))
      {
        buzzerOn = true;
        digitalWrite(buzzerGPIO, HIGH);
        alarmTimer = 0;

        //BUZZER STATE /6 BYTES
        while( Serial.availableForWrite() < 6 ){}
        Serial.write(START_CHAR);
        Serial.print("MBS1");
        Serial.write(END_CHAR);
      }
    }
  }

  atNightMode();
  
  if (nightMode)
  {
    
    if( ( atNightNow ) and ( motion1_State ) and (buzzerStop == false) and (intruderAlertTimer >= INTRUDER_TIMER_MAX) ){
      intruderAlertTimer = 0;
      buzzerOn = true;
      digitalWrite( buzzerGPIO, HIGH );

      //BUZZER STATE /6 BYTES
      while( Serial.availableForWrite() < 6 ){}
      Serial.write(START_CHAR);
      Serial.print("MBS1");
      Serial.write(END_CHAR);

      //REPORT INTRUDE ALERT
      while( Serial.availableForWrite() < 6 ){}
      Serial.write(START_CHAR);
      Serial.print("MRP0");
      Serial.write(END_CHAR);

      while( Serial.availableForWrite() < 63 ){}
    }
    
  }

  serverHandler();
}

bool atNightMode()
{
  bool answer = false;
  if (nightMode)
  {
    if ((nightTimeStart[0] == nightTimeEnd[0]) and (nightTimeStart[1] == nightTimeEnd[1]))
    {
      //Full Time nightMode
      answer = true;
    }
    else
    {
      unsigned long timeNow   = 60 * timeHour + timeMinute;
      unsigned long timeStart = 60 * nightTimeStart[0] + nightTimeStart[1];
      unsigned long timeEnd   = 60 * nightTimeEnd[0] + nightTimeEnd[1];
      if ((nightTimeStart[0] <= nightTimeEnd[0]))
      {
        if ((timeStart <= timeNow) and (timeNow <= timeEnd))
        {
          answer = true;
        }
      }
      else
      {
        //( nightTimeStart[0] > nightTimeEnd[0] )
        if ((timeStart <= timeNow) or (timeNow <= timeEnd))
        {
          answer = true;
        }
      }
    }
  }

  atNightNow = answer;

  return answer;
}

void setLCD(char option /*'H' Hour, 'm' Minute, 'W' Weekday, 'D' Day, 'M' Month */){
  if( option == 'H' ){
    lcd.setCursor(0, 0);
    if (timeHour < 10)
    {
      lcd.print(0);
      lcd.print(timeHour);
    }
    else
    {
      lcd.print(timeHour);
    }
  }

  else if( option == 'm' ){
    lcd.setCursor(3, 0);
    if (timeMinute < 10)
    {
      lcd.print(0);
      lcd.print(timeMinute);
    }
    else
    {
      lcd.print(timeMinute);
    }
  }

  else if( option == 'W' ){
    lcd.setCursor(6, 0);
    switch (timeWeekday)
    {
      case 0:
        lcd.print("Sun");
        break;
      case 1:
        lcd.print("Mon");
        break;
      case 2:
        lcd.print("Tue");
        break;
      case 3:
        lcd.print("Wed");
        break;
      case 4:
        lcd.print("Thu");
        break;
      case 5:
        lcd.print("Fri");
        break;
      default:
        lcd.print("Sat");
        break;
    }
  }

  else if( option == 'D' ){
    lcd.setCursor(10, 0);
    if (timeDay < 10)
    {
      lcd.print(0);
      lcd.print(timeDay);
    }
    else
    {
      lcd.print(timeDay);
    }
  }
  
  else if( option == 'M' ){
    lcd.setCursor(13, 0);
    switch (timeMonth)
    {
      case 1:
        lcd.print("Jan");
        break;
      case 2:
        lcd.print("Feb");
        break;
      case 3:
        lcd.print("Mar");
        break;
      case 4:
        lcd.print("Apr");
        break;
      case 5:
        lcd.print("May");
        break;
      case 6:
        lcd.print("Jun");
        break;
      case 7:
        lcd.print("Jul");
        break;
      case 8:
        lcd.print("Aug");
        break;
      case 9:
        lcd.print("Sep");
        break;
      case 10:
        lcd.print("Oct");
        break;
      case 11:
        lcd.print("Nov");
        break;
      default:
        lcd.print("Dec");
        break;
    } 
  }
}

void updateLCD()
{
  static int LCD_Second  = timeSecond;
  static int LCD_Minute  = timeMinute;
  static int LCD_Hour    = timeHour;
  static int LCD_Weekday = timeWeekday;
  static int LCD_Day     = timeDay;
  static int LCD_Month   = timeMonth;
  
  bool oneMinuteTimer = false;
  
  /*
    LCD Display 16x02 Pattern:
        Line 1 (16x01): [0123456789abcdef]
        Line 2 (16x01): [0123456789abcdef]

    Example:
      Line 1:           [23:59 Sun 31/Dec]
      Line 2:           [Intrude Alert!!!]
  */

  if (LCD_Second != timeSecond)
  {
    oneSecondTimerHandler();
    LCD_Second = timeSecond;
    
    lcd.setCursor(0, 1);
    if (msg2Timer < TEMP_MESSAGE_TIMER_MAX)
    {
      lcd.print(msg2_temp);
    }
    else
    {
      if ((buzzerStop == false) and
          (intruderAlertTimer < INTRUDER_TIMER_MAX))
      {
        lcd.print("INTRUDER ALERT! ");
      }
      else if ((buzzerStop == false) and
          (alarmTimer < ALARM_TIMER_MAX))
      {
        lcd.print("  ALARM CLOCK!  ");
      }
      else if( atNightNow ){
        lcd.print(" At Night Mode! ");
      }
      else
      {
        lcd.print(msg2);
      }
    }
  }

  if (LCD_Minute != timeMinute)
  {
    oneMinuteTimer = true;
    setLCD('m');
    LCD_Minute = timeMinute;
  }

  if (LCD_Hour != timeHour)
  {
    setLCD('H');
    LCD_Hour = timeHour;
  }

  if (LCD_Weekday != timeWeekday)
  {
    LCD_Weekday = timeWeekday;
    setLCD('W');
  }

  if (LCD_Day != timeDay){
    setLCD('D');
    LCD_Day = timeDay;
    Serial.write( START_CHAR );Serial.print("RTO");Serial.write( END_CHAR );
  }

  if (LCD_Month != timeMonth){
    LCD_Month = timeMonth;
    setLCD('M');
  }

  if( oneMinuteTimer ){
    
    //Request Information from Slaves via RF24
    requestUpdateFromSlaves();

    refreshServer();
  }

  //Something is REALLY Wrong with DS1307
  if( ( timeSecond > 59 ) or ( timeMinute > 59 ) or (timeHour > 23) or (timeWeekday > 6) ){
    //Set Any Value for DS1307 Time
    setTimeDS1307( 17 , 7 , 3, 1, 14, 0 ); //year, month, day, day_week, (0 = Sun, 6 = Sat), hour, minute
    //Request Time Online
    Serial.write( START_CHAR );Serial.print("RTO");Serial.write( END_CHAR );
  }
  
}

void joyTimerHandler(){
  static uint16_t firstMinuteCounter = 0;
  if( firstMinuteCounter < 3000 ){
    firstMinuteCounter++;
    if( firstMinuteCounter == 3000 ){
      firstTime = true;
    }
  }

  if( !goAhead ){
    if( waitingForAnAnswer < WAIT_FOR_AN_ANSWER ){
      waitingForAnAnswer++;
      if( waitingForAnAnswer == WAIT_FOR_AN_ANSWER ){
        goAhead = true;
      }
    }
  }
  
  beepAlarm();
}

void beepAlarm(){
  if( (!buzzerStop) and (buzzerOn) and (alarmTimer < ALARM_TIMER_MAX) ){
    static uint8_t sixteenthNoteCounter = 0;
    const uint8_t sixteenthNoteStep = 10;
    
    sixteenthNoteCounter++;
    if( ( sixteenthNoteCounter == sixteenthNoteStep   ) or 
        ( sixteenthNoteCounter == sixteenthNoteStep*2 ) or
        ( sixteenthNoteCounter == sixteenthNoteStep*3 ) or
        ( sixteenthNoteCounter == sixteenthNoteStep*4 )  ){
      digitalWrite( buzzerGPIO, HIGH );
    }
    
    else if( ( sixteenthNoteCounter == sixteenthNoteStep*1.5  ) or 
             ( sixteenthNoteCounter == sixteenthNoteStep*2.5 ) or
             ( sixteenthNoteCounter == sixteenthNoteStep*3.5 ) or
             ( sixteenthNoteCounter == sixteenthNoteStep*4.5 )  ){
      digitalWrite( buzzerGPIO, LOW );
    }
    
    else if( sixteenthNoteCounter == sixteenthNoteStep*8 ){
      sixteenthNoteCounter = 0;
    }
  }
  
}

//---------------------------------------------------------------






//---------------------------------------------------------------
//             FUNCTIONS: WEB SERVER COMMUNICATION
//---------------------------------------------------------------
void refreshServer()
{
  
  //LAMP1_STATE /6 BYTES
  while( Serial.availableForWrite() < 6 ){}
  Serial.write(START_CHAR);
  Serial.print("MLS"); Serial.print( lamp1_State );
  Serial.write(END_CHAR);

  //MOTION1 SENSOR STATE /6 BYTES
  while( Serial.availableForWrite() < 6 ){}
  Serial.write(START_CHAR);
  Serial.print("MMS");
  if (motion1_State)
  {
    Serial.print(1);
  }
  else
  {
    Serial.print(0);
  }
  Serial.write(END_CHAR);

  //BUZZER STATE /6 BYTES
  while( Serial.availableForWrite() < 6 ){}
  Serial.write(START_CHAR);
  Serial.print("MBS");
  if (buzzerOn)
  {
    Serial.print(1);
  }
  else
  {
    Serial.print(0);
  }
  Serial.write(END_CHAR);

  //HOUR TIME / 8 TO 10 BYTES
  while( Serial.availableForWrite() < 10 ){}
  Serial.write(START_CHAR);
  Serial.print("MT1");
  Serial.print(timeHour);
  Serial.print(":");
  Serial.print(timeMinute);
  Serial.write(END_CHAR);

  //DATE / 10 TO 13 BYTES
  while( Serial.availableForWrite() < 13 ){}
  Serial.write(START_CHAR);
  Serial.print("MT2");
  Serial.print(timeDay);
  Serial.print("/");
  Serial.print(timeMonth);
  Serial.print("/");
  Serial.print(timeYear);
  Serial.write(END_CHAR);

  //WEEKDAY / 6 BYTES
  while( Serial.availableForWrite() < 6 ){}
  Serial.write(START_CHAR);
  Serial.print("MT3");
  Serial.print(timeWeekday);
  Serial.write(END_CHAR);

  //ALARM TIME //8 TO 10 BYTES
  while( Serial.availableForWrite() < 10 ){}
  Serial.write(START_CHAR);
  Serial.print("MA1");
  Serial.print(alarmHour[0]);
  Serial.print(":");
  Serial.print(alarmHour[1]);
  Serial.write(END_CHAR);

  //ALARM DAY //6 BYTES
  while( Serial.availableForWrite() < 6 ){}
  Serial.write(START_CHAR);
  Serial.print("MA2");
  Serial.print(alarmWeekday);
  Serial.write(END_CHAR);

  //ALARM SET //6 BYTES
  while( Serial.availableForWrite() < 6 ){}
  Serial.write(START_CHAR);
  Serial.print("MA3");
  if (alarmSet)
  {
    Serial.print(1);
  }
  else
  {
    Serial.print(0);
  }
  Serial.write(END_CHAR);

  //NIGHT TIME START //8 TO 10 BYTES
  while( Serial.availableForWrite() < 10 ){}
  Serial.write(START_CHAR);
  Serial.print("MN1");
  Serial.print(nightTimeStart[0]);
  Serial.print(":");
  Serial.print(nightTimeStart[1]);
  Serial.write(END_CHAR);

  //NIGHT TIME END //8 TO 10 BYTES
  while( Serial.availableForWrite() < 10 ){}
  Serial.write(START_CHAR);
  Serial.print("MN2");
  Serial.print(nightTimeEnd[0]);
  Serial.print(":");
  Serial.print(nightTimeEnd[1]);
  Serial.write(END_CHAR);

  //NIGHT TIME SET //6 BYTES
  while( Serial.availableForWrite() < 6 ){}
  Serial.write(START_CHAR);
  Serial.print("MN3");
  if (nightMode)
  {
    Serial.print(1);
  }
  else
  {
    Serial.print(0);
  }
  Serial.write(END_CHAR);

  //REPORT SLAVE STATUS
  uint8_t i;
  for(i=0; i<3; i++){
    while( Serial.availableForWrite() < 7 ){}
    Serial.write(START_CHAR);
    Serial.print("MSS");
    Serial.print( i+1 );
    Serial.print( (slavesStatus >> i) & 1 );
    Serial.write(END_CHAR);
  }

  if( firstTime ){
    firstTime = false;
    //REPORT MCU STARTING
    while( Serial.availableForWrite() < 6 ){}
    Serial.write(START_CHAR);
    Serial.print("MRP1");
    Serial.write(END_CHAR);
  }
  

  /*
  ATMega328p UART Buffer = 127 Bytes
  
  The BUFFER Length is divided into:
    1 Hardware UART (63 UART Buffer)
    1 Software UART (64 UART Buffer)
  */
  //WAIT FOR ALL DATA TO BE SENT
  while( Serial.availableForWrite() < 63 ){}
}

void serverHandler()
{

  if (Serial.available() > 0)
  {
    stillBuffering = false;
    //------------------------------------------------------------
    //     READING UART VALUES AS VALID ASCII CHARACTERS ONLY
    //------------------------------------------------------------
    //Serial.setTimeout(10);
    uint8_t bufLen = Serial.readBytes(buf1, 100);
    buf1[bufLen] = '\0';
    uint8_t i=0, j = 0;
    for (i = 0; i < bufLen; i++)
    {
      if ((buf1[i] == START_CHAR) or (buf1[i] == END_CHAR) or ((buf1[i] > 32) && (buf1[i] < 127)))
      {
        buf1[j++] = buf1[i];
      }
    }
    buf1[j] = '\0';

    buffer_read += buf1;
    msgLen = buffer_read.length();
    //------------------------------------------------------------
  }

  if ((msgLen > 0) and (stillBuffering == false))
  {
    
    if (DEBUG_PRINTING)
    {
      btSerial.println(buffer_read);
    }
    
    //FLUSHING CHARS UNTILL A VALID START CHAR IS FOUND
    while ((buffer_read.indexOf(START_CHAR) != 0) and (msgLen > 0))
    {
      buffer_read = buffer_read.substring(1);
      msgLen--;
    }

    //WORD RECEIVED STARTS WITH A VALID START CHAR
    if (buffer_read.indexOf(START_CHAR) == 0)
    {
      stillBuffering = true;

      //WORD RECEIVED HAS THE VALID END CHAR
      if (buffer_read.indexOf(END_CHAR) >= 4)
      {
        stillBuffering = false;

        //ESP8266 CONNECTED
        if ((buffer_read.substring(1, 4) == "WEB") && (buffer_read.indexOf(END_CHAR) == 6))
        {
          strncpy(msg2_temp, "WEBSERVER ONLINE", 17);
          msg2Timer = 0;
          refreshServer();
        }

        //GETTING EXISTING WIFI ESP8266 SERVER IP
        if ((buffer_read.substring(1, 4) == "IP0") && (buffer_read.indexOf(END_CHAR) <= 19))
        {
          if ( (buffer_read.indexOf(END_CHAR) == 5) ) {
            strncpy(msg2_temp, "WIFI DIDNT WORK!", 17);
            msg2Timer = 0;
          } else
          {
            strncpy(msg2_temp, "WIFI CONNECTED  ", 17); msg2Timer = 0;
            char ipNumber[16];
            ( buffer_read.substring(4, buffer_read.indexOf(END_CHAR)) ).toCharArray( ipNumber, 16 );
            uint8_t ipLen = (buffer_read.indexOf(END_CHAR) - 4);
            
            msg2[0] = 'W';
            uint8_t i = 1, j = 0;
            for(j=0; j<(15 - ipLen); j++){
              msg2[i++] = ' ';
            }
            for(j=0; j<ipLen; j++){
              msg2[i++] = ipNumber[j];
            }
            msg2[i] = '\0';
            
          }
        }

        //WEB SERVER BUTTON
        if ((buffer_read.substring(1, 4) == "WB1") && (buffer_read.indexOf(END_CHAR) == 5))
        {
          //                  0123456789012345
          strncpy(msg2_temp, "  WEB BUTTON #  ", 17);
          msg2_temp[13] = buffer_read.charAt(4);
          msg2Timer = 0;
          
          if (buffer_read.substring(4, 5) == "1")
          {
            //CHANGE LAMP STATE
            lamp1_State++;
            if ( lamp1_State > 2 ) {
              lamp1_State = 0;
            }
            report2Slave2( lamp1_State );
            setLampState( lamp1_State );
            
          }
          else if (buffer_read.substring(4, 5) == "2")
          {
            //CHANGE BUZZER STATE
            if (buzzerOn)
            {
              if( (alarmTimer < ALARM_TIMER_MAX ) or
                  (intruderAlertTimer < INTRUDER_TIMER_MAX) ){
                buzzerStop = true;
              }
              
              buzzerOn = false;
              digitalWrite(buzzerGPIO, LOW);
            }
            else
            {
              buzzerOn = true;
              digitalWrite(buzzerGPIO, HIGH);
            }
          }
          else if (buffer_read.substring(4, 5) == "3")
          {
            //CHANGE ALARM SET
            alarmSet = !alarmSet;
            setAlarm0(alarmHour, alarmWeekday, alarmSet);
          }
          else if (buffer_read.substring(4, 5) == "4")
          {
            //CHANGE NIGHT TIME MODE SET
            nightMode = !nightMode;
            setNightTime(nightTimeStart, nightTimeEnd, nightMode);
          }
          refreshServer();
        }

        //SET TIME AND DATE VALUES
        if ( (buffer_read.substring(1, 3) == "WT") or (buffer_read.substring(1, 3) == "WD") ){
          //  SET TIME HOUR
          if ((buffer_read.substring(1, 4) == "WTH") && (buffer_read.indexOf(END_CHAR) <= 6))
          {
            strncpy(msg2_temp, "WEB TIME UPDATED", 17); msg2Timer = 0;
            timeHour = (buffer_read.substring(4, buffer_read.indexOf(END_CHAR))).toInt();
          }
  
          //  SET TIME MINUTE
          if ((buffer_read.substring(1, 4) == "WTM") && (buffer_read.indexOf(END_CHAR) <= 6))
          {
            timeMinute = (buffer_read.substring(4, buffer_read.indexOf(END_CHAR))).toInt();
          }
  
          //  SET DATE DAY
          if ((buffer_read.substring(1, 4) == "WDD") && (buffer_read.indexOf(END_CHAR) <= 6))
          {
            timeDay = (buffer_read.substring(4, buffer_read.indexOf(END_CHAR))).toInt();
          }
  
          //  SET DATE MONTH
          if ((buffer_read.substring(1, 4) == "WDM") && (buffer_read.indexOf(END_CHAR) <= 6))
          {
            timeMonth = (buffer_read.substring(4, buffer_read.indexOf(END_CHAR))).toInt();
          }
  
          //  SET DATE YEAR
          if ((buffer_read.substring(1, 4) == "WDY") && (buffer_read.indexOf(END_CHAR) <= 6))
          {
            timeYear = (buffer_read.substring(4, buffer_read.indexOf(END_CHAR))).toInt();
          }
  
          //  SET DATE WEEKDAY
          if ((buffer_read.substring(1, 4) == "WDW") && (buffer_read.indexOf(END_CHAR) == 5))
          {
            timeWeekday = (buffer_read.substring(4, 5)).toInt();
          }

          setTimeDS1307(timeYear, timeMonth, timeDay, timeWeekday, timeHour, timeMinute);
          refreshServer();
        }

        //UPDATE ALARM VALUES
        if ((buffer_read.substring(1, 3) == "WA") && (buffer_read.indexOf(END_CHAR) <= 6)){
          
          //SET ALARM SCHEDULE HOUR
          if ((buffer_read.substring(1, 4) == "WAH") && (buffer_read.indexOf(END_CHAR) <= 6))
          {
            strncpy(msg2_temp, "ALARM TIME UPDTD", 17); msg2Timer = 0;
            alarmHour[0] = (buffer_read.substring(4, buffer_read.indexOf(END_CHAR))).toInt();
          }
  
          //SET ALARM SCHEDULE MINUTE
          if ((buffer_read.substring(1, 4) == "WAM") && (buffer_read.indexOf(END_CHAR) <= 6))
          {
            alarmHour[1] = (buffer_read.substring(4, buffer_read.indexOf(END_CHAR))).toInt();
          }
  
          //SET ALARM SCHEDULE DAY OF WEEK
          if ((buffer_read.substring(1, 4) == "WAD") && (buffer_read.indexOf(END_CHAR) == 5))
          {
            strncpy(msg2_temp, "ALARM TIME UPDTD", 17); msg2Timer = 0;
            alarmWeekday = (buffer_read.substring(4, 5)).toInt();
          }

          setAlarm0(alarmHour, alarmWeekday, alarmSet);
          refreshServer();
        }

        //UPDATE NIGHT TIME VALUES
        if ((buffer_read.substring(1, 3) == "WN") && (buffer_read.indexOf(END_CHAR) <= 6)){
        
          //SET NIGHT TIME START HOUR
          if ((buffer_read.substring(1, 4) == "WN1"))
          {
            strncpy(msg2_temp, "NIGHT TIME UPDTD", 17); msg2Timer = 0;
            nightTimeStart[0] = (buffer_read.substring(4, buffer_read.indexOf(END_CHAR))).toInt();
          }
  
          //SET NIGHT TIME START MINUTE
          if ((buffer_read.substring(1, 4) == "WN2"))
          {
            nightTimeStart[1] = (buffer_read.substring(4, buffer_read.indexOf(END_CHAR))).toInt();
          }
  
          //SET NIGHT TIME END HOUR
          if ((buffer_read.substring(1, 4) == "WN3"))
          {
            strncpy(msg2_temp, "NIGHT TIME UPDTD", 17); msg2Timer = 0;
            nightTimeEnd[0] = (buffer_read.substring(4, buffer_read.indexOf(END_CHAR))).toInt();
          }
  
          //SET NIGHT TIME END MINUTE
          if ((buffer_read.substring(1, 4) == "WN4"))
          {
            nightTimeEnd[1] = (buffer_read.substring(4, buffer_read.indexOf(END_CHAR))).toInt();
          }

          setNightTime(nightTimeStart, nightTimeEnd, nightMode);
          refreshServer();
        }
      }
      //DELETING THE PART OF THE MSG THAT HAS ALREADY BEEN READ
      if (buffer_read.indexOf(END_CHAR) > 0)
      {
        buffer_read = buffer_read.substring(buffer_read.indexOf(END_CHAR) + 1);
        msgLen = buffer_read.length();
      }
    }
  }
}
//---------------------------------------------------------------






//---------------------------------------------------------------
//               FUNCTIONS: RF24 COMMUNICATION
//---------------------------------------------------------------
void handleRF24(){
  //IS THERE RADIO SIGNAL INCOMING?
  if (radio.available())
  {
    char recebidos[50] = "";
    radio.read( recebidos , 49 );
    messageRF24 = recebidos;
    if( DEBUG_PRINTING ){
      btSerial.print("RF IN: ");
      btSerial.println( messageRF24 );
    }
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
    
    if (messageRF24[5] == END_CHAR){
      
      if( messageRF24.substring(1,4) == "S1M" ){
        slavesStatus |= (1 << 0);slavesStatusCounter[0] = 0;
        
        if( messageRF24.substring(4,5) == "0" ){
          motion1_State = false;
          report2Slave2(3);
        }

        else if( messageRF24.substring(4,5) == "1" ){
          motion1_State = true;
          report2Slave2(4);
        }
        setMotionState(motion1_State);
      }

      else if( messageRF24.substring(1,4) == "S2M" ){
        slavesStatus |= (1 << 1);slavesStatusCounter[1] = 0;
        
        if( messageRF24.substring(4,5) == "0" ){
          lamp1_State = 0;
        }

        else if( messageRF24.substring(4,5) == "1" ){
          lamp1_State = 1;
        }

        else if( messageRF24.substring(4,5) == "2" ){
          lamp1_State = 2;
        }
      }
    }
    else if (messageRF24[6] == END_CHAR){
      
      if( messageRF24.substring(1,4) == "S3M" ){
        slavesStatus |= (1 << 2);slavesStatusCounter[2] = 0;
        
        uint8_t buttonPressed = 99;

        buttonPressed = ( ( messageRF24.substring(4,6) ).toInt() );

        if( buttonPressed <= 16 ){
          strncpy(msg2_temp, "IR REMOTE:      ", 17); msg2Timer = 0;
          if( buttonPressed > 9 ){ msg2_temp[11] = '1'; }
          msg2_temp[12] = 48 + (buttonPressed % 10);
          
          if(      buttonPressed == 0  ){ lamp1_State = 0; report2Slave2(0); setLampState( lamp1_State ); }
          else if( buttonPressed == 1  ){ lamp1_State = 1; report2Slave2(1); setLampState( lamp1_State ); }
          else if( buttonPressed == 2  ){ lamp1_State = 2; report2Slave2(2); setLampState( lamp1_State ); }
          else if( buttonPressed == 10 ){ nightMode = !nightMode; setNightTime(nightTimeStart, nightTimeEnd, nightMode); } //Button: *
          else if( buttonPressed == 11 ){ alarmSet  = !alarmSet; setAlarm0(alarmHour, alarmWeekday, alarmSet);} //Button: #
          else if( buttonPressed == 16 ){ buzzerStop = true; buzzerOn = false; digitalWrite(buzzerGPIO, LOW); } //Button: OK

          refreshServer();
        }
      }
    }
  }
}

void report2Slave2(uint8_t cmd){
  if( cmd > 9 ){ return; }
  char message[7] = "#MS20#";
  message[0]      = START_CHAR;
  message[5]      = END_CHAR;
  message[6]      = '\0';

  message[4] = 48 + cmd;
  
  //Send the message to the slave
  radio.stopListening();
  radio.openWritingPipe( pipes[2] );
  radio.write( message, sizeof(message) );
  radio.startListening();
  if( DEBUG_PRINTING ){ 
    btSerial.print("RF OUT: ");
    btSerial.println( message );
  }
}

void requestUpdateFromSlaves(){
  char message[7] = "#MS1?#";
  message[0]      = START_CHAR;
  message[5]      = END_CHAR;
  message[6]      = '\0';

  int i;
  for(i=2; i>-1; i--){
    message[3] = 49+i;
    //Request Slave (i+1) Status
    radio.stopListening();
    radio.openWritingPipe( pipes[i+1] );
    radio.write( message, sizeof(message) );
    radio.startListening();
    if( DEBUG_PRINTING ){ 
      btSerial.print("RF OUT: ");
      btSerial.println( message );
    }
    
    waitingForAnAnswer = 0;
    while( (!(radio.available())) and (!goAhead) ){}
    handleRF24();
    waitingForAnAnswer = WAIT_FOR_AN_ANSWER;
    goAhead = false;
  }
  
}
//---------------------------------------------------------------





//---------------------------------------------------------------
//        FUNCTIONS: REAL TIME MODULE CONTROL
//---------------------------------------------------------------
byte dec2bcd(byte number) {
  return ((number / 10 * 16) + (number % 10));
}

uint8_t bcd2dec(uint8_t number) {
  return ((number >> 4) * 10 + (number & 0x0F));
}

void setTimeDS1307(byte timeYear, byte timeMonth, byte timeDay, byte timeWeekday, byte timeHour, byte timeMinute)
{
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(0x00);

  Wire.write(dec2bcd(0));
  Wire.write(dec2bcd(timeMinute));
  Wire.write(dec2bcd(timeHour));
  Wire.write(dec2bcd(timeWeekday));
  Wire.write(dec2bcd(timeDay));
  Wire.write(dec2bcd(timeMonth));
  Wire.write(dec2bcd(timeYear));

  Wire.write(0x00);
  Wire.endTransmission();
}

void getTimeDS1307()
{
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(0x00);
  Wire.endTransmission();

  Wire.requestFrom(DS1307_ADDRESS, 7);
  timeSecond  = bcd2dec(Wire.read());
  timeMinute  = bcd2dec(Wire.read());
  timeHour    = bcd2dec(Wire.read() & 0b111111);
  timeWeekday = bcd2dec(Wire.read());
  timeDay     = bcd2dec(Wire.read());
  timeMonth   = bcd2dec(Wire.read());
  timeYear    = bcd2dec(Wire.read());
}
//---------------------------------------------------------------





