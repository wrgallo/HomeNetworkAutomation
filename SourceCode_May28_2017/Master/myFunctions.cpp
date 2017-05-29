#include "Arduino.h"
#include "myFunctions.h"

/*
  -----------------------------------------------
 | PIN |  PURPOSE                                |
 |-----|-----------------------------------------|
 | A4  |  I2C SDA (LCD DISPLAY AND REAL TIME)    |
 | A5  |  I2C SCL (LCD DISPLAY AND REAL TIME)    |
 | 0   |  HARD UART RX (ESP8266 TX)              |
 | 1   |  HARD UART TX (ESP8266 RX)              |
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
 | +3v3          |  ESP8266                    |
 | +3v3          |  NRF24L01+                  |
  ---------------------------------------------
*/

//---------------------------------------------------------------
//				GPIO CONFIGURATION AND DEFINITIONS
//---------------------------------------------------------------
//PINOUT DEFINITIONS
#define SOFT_RX       3                                          //UART RX FOR BLUETOOTH MODULE
#define SOFT_TX       4                                          //UART TX FOR BLUETOOTH MODULE
#define buzzerGPIO    6                                          //BUZZER PIN OUT

//I2C ADDRESS DEFINITIONS
#define LCDADDR        0x3F                                      //LCD DISPLAY I2C ADDRESS
#define DS1307_ADDRESS 0x68                                      //REAL TIMER MODULE I2C ADDRESS

//CONSTANT OF PROJECT DEFINITIONS
#define alarmTimerMax 60                                         //MAX TIME FOR ALARM TO KEEP RUNNING
#define msg2TimerMax  10                                         //MAX TIME FOR A NEW MSG KEEP ON THE SCREEN OF DISPLAY

//COMMUNICATION BEETWEEN ESP8266 AND ARDUINO PROTOCOL
#define START_CHAR    2                                          //DEC NUMBER OF ASCII TABLE
#define END_CHAR      3                                          //DEC NUMBER OF ASCII TABLE

//DEBUGGING CONFIGURATION
#define DEBUG_PRINTING true                                      //[DEBUG ONLY] - Use UART for printing debug information
#define GPIO_LED1      12                                        //[DEBUG ONLY] - LED1 PIN OUT
#define GPIO_LED2      7                                         //[DEBUG ONLY] - LED2 PIN OUT

//---------------------------------------------------------------
//                 GLOBAL VARIABLES DEFINITIONS
//---------------------------------------------------------------
//                           ALARM
//---------------------------------------------------------------
bool     buzzerOn          = false;                              //BUZZER STATUS
bool     alarmSet          = false;                              //ALARM ON/OFF SCHEDULE
bool     alarmStop         = false;                              //SOMEONE REQUESTED TO STOP THE ALARM
bool     nightMode         = false;                              //NIGHT MODE ON/OFF SCHEDULE (BUZZER GOES ON WITH MOTION SENSOR)
bool     atNightNow        = false;                              //AT NIGHT MODE RIGHT NOW
uint8_t  nightTimeStart[2] = {22, 26};                           //NIGHT MODE TIME START SCHEDULE
uint8_t  nightTimeEnd[2]   = {5, 59};                            //NIGHT MODE TIME END SCHEDULE
uint8_t  alarmHour[2]      = {0, 33};                            //ALARM HOUR SCHEDULE
uint8_t  alarmDay          = 5;                                  //DAY OF WEEK FOR ALARM SCHEDULE (0 = SUN ... 6 = SAT)
uint16_t alarmTimer        = alarmTimerMax;                      //TIME COUNTER FOR ALARM AUTO GOES OFF
//---------------------------------------------------------------
//                    ESTADOS DOS ESCRAVOS
//---------------------------------------------------------------
bool     motion1_State = false;                                  //SLAVE1 - MOTION SENSOR
uint8_t  lamp1_State   = 0;                                      //SLAVE2 - LAMP
//---------------------------------------------------------------
//                             LCD
//---------------------------------------------------------------
LiquidCrystal_I2C lcd(LCDADDR, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);//OBJECT lcd FOR DISPLAY
uint8_t hora    = 23;                                            //DEFAULT HOUR VALUE
uint8_t minuto  = 59;                                            //DEFAULT MINUTE VALUE
uint8_t segundo = 59;                                            //DEFAULT SECOND VALUE
uint8_t dia     = 6;                                             //DEFAULT DAY OF WEEK VALUE (0 = SUNDAY, 6 = SATURDAY)
uint8_t dia_mes = 31;                                            //DEFAULT DAY VALUE
uint8_t mes     = 12;                                            //DEFAULT MONTH VALUE
uint8_t ano     = 17;                                            //DEFAULT YEAR VALUE (2 DIGITS)

char     msg2[17]      = "                ";                     //MESSAGE FOR SECOND LINE OF DISPLAY
char     msg2_temp[17] = "2a Linha do LCD.";                     //TEMPORARY MESSAGE FOR SECOND LINE OF DISPLAY
uint16_t msg2Timer     = msg2TimerMax;                           //TIME COUNTER FOR TEMPORARY MESSAGE
//---------------------------------------------------------------
//                            BLUETOOTH
//---------------------------------------------------------------
SoftwareSerial softSerial(SOFT_RX, SOFT_TX);                     //OBJECT softSerial FOR UART COMMUNICATION WITH HC-05

//---------------------------------------------------------------
//                              WIFI
//---------------------------------------------------------------
bool     onlineClient = true;                                    //THERE IS ONE AUTHENTICATED USER ON SERVER
bool     stillBuffering = false;                                 //SERVER IS STILL COMMUNICATING WITH THIS UNIT
char     buf1[100];                                              //TO GET THE BYTES OUT OF UART BUFFER WITH ONLY VALID UART VALUES
String   buffer_read = "";                                       //ALL MSGS IN CURRENT BUFFER
uint16_t msgLen = 0;                                             //LENGHT OF BUFFER READ
uint8_t  i = 0, j = 0, bufLen = 0;                               //GENERIC COUNTERS AND THE LENGHT OF buf1
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
void inicializarArduino()
{
  //INPUT | OUTPUT CONFIGURATION
  Serial.begin(115200);
  softSerial.begin(115200);
  pinMode(GPIO_LED1, OUTPUT);digitalWrite(GPIO_LED1, LOW);
  pinMode(GPIO_LED2, OUTPUT);digitalWrite(GPIO_LED2, LOW);
  pinMode(buzzerGPIO, OUTPUT);digitalWrite(buzzerGPIO, LOW);
  lcd.begin(16, 2);
  Wire.begin();

  //SETTING THE DATE AND TIME ON REAL TIME MODULE FOR THE FIRST TIME
  //setTimeDS1307( 17 , 5 , 27, 6, 15, 0 ); //year, month, day, day_week, (0 = Sun, 6 = Sat), hour, minute

  //READING DATE AND TIME FROM REAL TIME MODULE
  updateTime();

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
  lcd.print("Inicializando...");
  delay(8000);

  lcd.clear();
  lcd.setCursor(0, 0);
  //          0123456789abcdef
  lcd.print("00:00:00        ");

  if (segundo < 10)
  {
    lcd.setCursor(7, 0);
    lcd.print(segundo);
  }
  else
  {
    lcd.setCursor(6, 0);
    lcd.print(segundo);
  }
  if (minuto < 10)
  {
    lcd.setCursor(4, 0);
    lcd.print(minuto);
  }
  else
  {
    lcd.setCursor(3, 0);
    lcd.print(minuto);
  }
  if (hora < 10)
  {
    lcd.setCursor(1, 0);
    lcd.print(hora);
  }
  else
  {
    lcd.setCursor(0, 0);
    lcd.print(hora);
  }
  lcd.setCursor(9, 0);
  switch (dia)
  {
    case 0:
      lcd.print("Domingo");
      break;
    case 1:
      lcd.print("Segunda");
      break;
    case 2:
      lcd.print("  Terca");
      break;
    case 3:
      lcd.print(" Quarta");
      break;
    case 4:
      lcd.print(" Quinta");
      break;
    case 5:
      lcd.print("  Sexta");
      break;
    default:
      lcd.print(" Sabado");
      break;
  }


  //--------------------------------------------
  //             RF24 CONFIGURATION
  //--------------------------------------------
  //Start Communication
  radio.begin();

  //Select Writting and Reading Pipe Addresses
  radio.openReadingPipe( 1 , pipes[0] ); //The Pipe where Master Answers This Slave
  radio.startListening();                // Start listening

  //radio.openWritingPipe( pipes[1] ); //To Talk to Slave 1
  //radio.openWritingPipe( pipes[2] ); //To Talk to Slave 2
  //radio.openWritingPipe( pipes[3] ); //To Talk to Slave 3
  //--------------------------------------------
  
}

void oneSecondTimerHandler()
{ 
  if (msg2Timer < msg2TimerMax)
  {
    msg2Timer++;
  }

  if (alarmTimer == alarmTimerMax - 1)
  {
    buzzerOn = false;
    digitalWrite(buzzerGPIO, LOW);
    alarmStop = false;
  }

  if (alarmTimer < alarmTimerMax)
  {
    alarmTimer++;
  }

  if (alarmSet)
  {
    if ((alarmDay == dia) and (alarmHour[0] == hora) and (alarmHour[1] == minuto) and (segundo < 50))
    {
      if ((buzzerOn == false) and (alarmStop == false))
      {
        buzzerOn = true;
        digitalWrite(buzzerGPIO, HIGH);
        alarmTimer = 0;
      }
    }
  }

  if (nightMode)
  {
    atNightMode();
  }

  serverHandler();
}

//---------------------------------------------------------------
//             FUNCTIONS: COMMUNICATION PROTOCOLS
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
  Serial.print(hora);
  Serial.print(":");
  Serial.print(minuto);
  Serial.write(END_CHAR);

  //DATE / 10 TO 13 BYTES
  while( Serial.availableForWrite() < 13 ){}
  Serial.write(START_CHAR);
  Serial.print("MT2");
  Serial.print(dia_mes);
  Serial.print("/");
  Serial.print(mes);
  Serial.print("/");
  Serial.print(ano);
  Serial.write(END_CHAR);

  //WEEKDAY / 6 BYTES
  while( Serial.availableForWrite() < 6 ){}
  Serial.write(START_CHAR);
  Serial.print("MT3");
  Serial.print(dia);
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
  Serial.print(alarmDay);
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

  //IS THERE ANYONE STILL ONLINE
  while( Serial.availableForWrite() < 6 ){}
  Serial.write(START_CHAR);
  Serial.print("RCL");
  Serial.write(END_CHAR);
  
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
    Serial.setTimeout(10);
    bufLen = Serial.readBytes(buf1, 100);
    buf1[bufLen] = '\0';
    j = 0;
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

    //FLUSHING CHARS UNTILL A VALID START CHAR IS FOUND
    while ((buffer_read.indexOf(START_CHAR) != 0) and (msgLen > 0))
    {
      buffer_read = buffer_read.substring(1);
      msgLen--;
    }
    if (DEBUG_PRINTING)
    {
      softSerial.println(buffer_read);
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
            
            char ipMSG[17];
            ipMSG[0] = 'W';

            i = 1;
            for(j=0; j<(15 - ipLen); j++){
              ipMSG[i++] = ' ';
            }
            for(j=0; j<ipLen; j++){
              ipMSG[i++] = ipNumber[j];
            }
            ipMSG[i] = '\0';
            strncpy(msg2 , ipMSG, 17);
          }
        }

        //GETTING ACCESS POINT ESP8266 SERVER IP
        if ((buffer_read.substring(1, 4) == "IP1") && (buffer_read.indexOf(END_CHAR) <= 19))
        {
          if ( (buffer_read.indexOf(END_CHAR) == 5) ) {
            strncpy(msg2_temp, "AP   DIDNT WORK!", 17);
            msg2Timer = 0;
          } else
          {
            if ( msg2Timer > msg2TimerMax-5 ) {
              strncpy(msg2_temp, "ACCESS POINT ON ", 17); msg2Timer = 0;
            }
            if ( msg2[0] != 'W' ) {
              char ipNumber[16];
              ( buffer_read.substring(4, buffer_read.indexOf(END_CHAR)) ).toCharArray( ipNumber, 16 );
              uint8_t ipLen = (buffer_read.indexOf(END_CHAR) - 4);
              
              char ipMSG[17];
              ipMSG[0] = 'A';
    
              i = 1;
              for(j=0; j<(15 - ipLen); j++){
                ipMSG[i++] = ' ';
              }
              for(j=0; j<ipLen; j++){
                ipMSG[i++] = ipNumber[j];
              }
              ipMSG[i] = '\0';
              strncpy(msg2 , ipMSG, 17);
            }
          }
        }

        //INFORM ESP8266 SUCCESSFULLY CONNECTED TO WIFI OR NOT
        if ((buffer_read.substring(1, 4) == "IWF") && (buffer_read.indexOf(END_CHAR) == 5))
        {
          if (buffer_read.substring(4, 5) == "0")
          {
            strncpy(msg2_temp, "WIFI DIDNT WORK!", 17);
            msg2Timer = 0;
          }
          else
          {
            //
          }
        }

        //INFORM ESP8266 SUCCESSFULLY CREATED ACCESS POINT OR NOT
        if ((buffer_read.substring(1, 4) == "IAP") && (buffer_read.indexOf(END_CHAR) == 5))
        {
          if (buffer_read.substring(4, 5) == "0")
          {
            strncpy(msg2_temp, "AP   DIDNT WORK!", 17);
            msg2Timer = 0;
          }
          else
          {
            //
          }
        }

        //INFORM IF THERE IS CLIENTS ONLINE
        if ((buffer_read.substring(1, 4) == "ICL") && (buffer_read.indexOf(END_CHAR) == 5))
        {
          if (buffer_read.substring(4, 5) == "0")
          {
            strncpy(msg2_temp, "CLIENTS OFFLINE ", 17);
            msg2Timer = 0;
            onlineClient = false;
          }
          else
          {
            onlineClient = true;
          }
        }

        //WEB SERVER BUTTON
        if ((buffer_read.substring(1, 4) == "WB1") && (buffer_read.indexOf(END_CHAR) == 5))
        {
          if (buffer_read.substring(4, 5) == "1")
          {
            //CHANGE LAMP STATE
            strncpy(msg2_temp, "  WEB BUTTON 1  ", 17);
            lamp1_State++;
            if ( lamp1_State > 2 ) {
              lamp1_State = 0;
            }
            report2Slave2( lamp1_State );
            
          } else if (buffer_read.substring(4, 5) == "2")
          {
            //CHANGE BUZZER STATE
            if (buzzerOn)
            {
              buzzerOn = false;
              digitalWrite(buzzerGPIO, LOW);
            }
            else
            {
              buzzerOn = true;
              digitalWrite(buzzerGPIO, HIGH);
            }
            strncpy(msg2_temp, "  WEB BUTTON 2  ", 17);
            msg2Timer = 0;
          }
          else if (buffer_read.substring(4, 5) == "3")
          {
            //CHANGE ALARM SET
            alarmSet = !alarmSet;
            strncpy(msg2_temp, "  WEB BUTTON 3  ", 17);
            msg2Timer = 0;
          }
          else if (buffer_read.substring(4, 5) == "4")
          {
            //CHANGE NIGHT TIME MODE SET
            nightMode = !nightMode;
            strncpy(msg2_temp, "  WEB BUTTON 4  ", 17);
            msg2Timer = 0;
          }
          refreshServer();
        }

        //SET TIME HOUR
        if ((buffer_read.substring(1, 4) == "WTH") && (buffer_read.indexOf(END_CHAR) <= 6))
        {
          strncpy(msg2_temp, "WEB TIME UPDATED", 17); msg2Timer = 0;
          hora = (buffer_read.substring(4, buffer_read.indexOf(END_CHAR))).toInt();
          setTimeDS1307(ano, mes, dia_mes, dia, hora, minuto);
        }

        //SET TIME MINUTE
        if ((buffer_read.substring(1, 4) == "WTM") && (buffer_read.indexOf(END_CHAR) <= 6))
        {
          minuto = (buffer_read.substring(4, buffer_read.indexOf(END_CHAR))).toInt();
          setTimeDS1307(ano, mes, dia_mes, dia, hora, minuto);
          refreshServer();
        }

        //SET DATE DAY
        if ((buffer_read.substring(1, 4) == "WDD") && (buffer_read.indexOf(END_CHAR) <= 6))
        {
          strncpy(msg2_temp, "WEB DATE UPDATED", 17); msg2Timer = 0;
          dia_mes = (buffer_read.substring(4, buffer_read.indexOf(END_CHAR))).toInt();
          setTimeDS1307(ano, mes, dia_mes, dia, hora, minuto);
        }

        //SET DATE MONTH
        if ((buffer_read.substring(1, 4) == "WDM") && (buffer_read.indexOf(END_CHAR) <= 6))
        {
          mes = (buffer_read.substring(4, buffer_read.indexOf(END_CHAR))).toInt();
          setTimeDS1307(ano, mes, dia_mes, dia, hora, minuto);
        }

        //SET DATE YEAR
        if ((buffer_read.substring(1, 4) == "WDY") && (buffer_read.indexOf(END_CHAR) <= 6))
        {
          ano = (buffer_read.substring(4, buffer_read.indexOf(END_CHAR))).toInt();
          setTimeDS1307(ano, mes, dia_mes, dia, hora, minuto);
          refreshServer();
        }

        //SET DATE WEEKDAY
        if ((buffer_read.substring(1, 4) == "WDW") && (buffer_read.indexOf(END_CHAR) == 5))
        {
          strncpy(msg2_temp, "WEB DAY  UPDATED", 17); msg2Timer = 0;
          dia = (buffer_read.substring(4, 5)).toInt();
          setTimeDS1307(ano, mes, dia_mes, dia, hora, minuto); //ano, mes, dia, dia_semana (0 = Dom, 6 = Sab), hora, minuto
          refreshServer();
        }

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
          refreshServer();
        }

        //SET ALARM SCHEDULE DAY OF WEEK
        if ((buffer_read.substring(1, 4) == "WAD") && (buffer_read.indexOf(END_CHAR) == 5))
        {
          strncpy(msg2_temp, "ALARM TIME UPDTD", 17); msg2Timer = 0;
          alarmDay = (buffer_read.substring(4, 5)).toInt();
          refreshServer();
        }

        //SET NIGHT TIME START HOUR
        if ((buffer_read.substring(1, 4) == "WN1") && (buffer_read.indexOf(END_CHAR) <= 6))
        {
          strncpy(msg2_temp, "NIGHT TIME UPDTD", 17); msg2Timer = 0;
          nightTimeStart[0] = (buffer_read.substring(4, buffer_read.indexOf(END_CHAR))).toInt();
        }

        //SET NIGHT TIME START MINUTE
        if ((buffer_read.substring(1, 4) == "WN2") && (buffer_read.indexOf(END_CHAR) <= 6))
        {
          nightTimeStart[1] = (buffer_read.substring(4, buffer_read.indexOf(END_CHAR))).toInt();
          refreshServer();
        }

        //SET NIGHT TIME END HOUR
        if ((buffer_read.substring(1, 4) == "WN3") && (buffer_read.indexOf(END_CHAR) <= 6))
        {
          strncpy(msg2_temp, "NIGHT TIME UPDTD", 17); msg2Timer = 0;
          nightTimeEnd[0] = (buffer_read.substring(4, buffer_read.indexOf(END_CHAR))).toInt();
        }

        //SET NIGHT TIME END MINUTE
        if ((buffer_read.substring(1, 4) == "WN4") && (buffer_read.indexOf(END_CHAR) <= 6))
        {
          nightTimeEnd[1] = (buffer_read.substring(4, buffer_read.indexOf(END_CHAR))).toInt();
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
//             FUNCTIONS: MASTER UNIT ACTIVITY
//---------------------------------------------------------------
bool atNightMode()
{
  bool resposta = false;
  if (nightMode)
  {
    if ((nightTimeStart[0] == nightTimeEnd[0]) and (nightTimeStart[1] == nightTimeEnd[1]))
    {
      //Full Time nightMode
      resposta = true;
    }
    else
    {
      unsigned long timeNow   = 60 * hora + minuto;
      unsigned long timeStart = 60 * nightTimeStart[0] + nightTimeStart[1];
      unsigned long timeEnd   = 60 * nightTimeEnd[0] + nightTimeEnd[1];
      if ((nightTimeStart[0] <= nightTimeEnd[0]))
      {
        if ((timeStart <= timeNow) and (timeNow <= timeEnd))
        {
          resposta = true;
        }
      }
      else
      {
        //( nightTimeStart[0] > nightTimeEnd[0] )
        if ((timeStart <= timeNow) or (timeNow <= timeEnd))
        {
          resposta = true;
        }
      }
    }
  }

  if (resposta)
  {
    if (atNightNow == false)
    {
      atNightNow = true;
      strncpy(msg2, "Em Modo Noturno!", 17);
    }
  }
  else
  {
    if (atNightNow == true)
    {
      atNightNow = false;
      strncpy(msg2, "                ", 17);
    }
  }

  return resposta;
}

void updateLCD()
{
  static int LCD_seg = segundo;
  static int LCD_min = minuto;
  static int LCD_hor = hora;
  static int LCD_dia = dia;
  bool oneMinuteTimer = false;
  
  /*
    Modelo do Display LCD 16x02:
        Linha 1 (16x01): [0123456789abcdef]
        Linha 2 (16x01): [0123456789abcdef]

    Exemplo:
    Hora e Dia da Semana: [23:59:59 Domingo]
    Mensagem Informativa: [Alerta, Intruso!]
  */

  if (LCD_seg != segundo)
  {
    oneSecondTimerHandler();
    
    if (segundo < 10)
    {
      lcd.setCursor(7, 0);
      lcd.print(segundo);
    }
    else
    {
      lcd.setCursor(6, 0);
      lcd.print(segundo);
    }
    if (segundo < LCD_seg)
    {
      lcd.setCursor(6, 0);
      lcd.print(0);
    }
    LCD_seg = segundo;

    lcd.setCursor(0, 1);
    if (msg2Timer < msg2TimerMax)
    {
      lcd.print(msg2_temp);
    }
    else
    {
      if ((alarmStop == false) and
          (alarmTimer < alarmTimerMax))
      {
        lcd.print("  DESPERTADOR!  ");
      }
      else
      {
        lcd.print(msg2);
      }
    }
  }

  if (LCD_min != minuto)
  {
    oneMinuteTimer = true;
    if (minuto < 10)
    {
      lcd.setCursor(4, 0);
      lcd.print(minuto);
    }
    else
    {
      lcd.setCursor(3, 0);
      lcd.print(minuto);
    }
    if (minuto < LCD_min)
    {
      lcd.setCursor(3, 0);
      lcd.print(0);
    }
    LCD_min = minuto;
  }

  if (LCD_hor != hora)
  {
    if (hora < 10)
    {
      lcd.setCursor(1, 0);
      lcd.print(hora);
    }
    else
    {
      lcd.setCursor(0, 0);
      lcd.print(hora);
    }
    if (hora < LCD_hor)
    {
      lcd.setCursor(0, 0);
      lcd.print(0);
    }
    LCD_hor = hora;
  }

  if (LCD_dia != dia)
  {
    LCD_dia = dia;
    lcd.setCursor(9, 0);
    switch (dia)
    {
      case 0:
        lcd.print("Domingo");
        break;
      case 1:
        lcd.print("Segunda");
        break;
      case 2:
        lcd.print("  Terca");
        break;
      case 3:
        lcd.print(" Quarta");
        break;
      case 4:
        lcd.print(" Quinta");
        break;
      case 5:
        lcd.print("  Sexta");
        break;
      default:
        lcd.print(" Sabado");
        break;
    }
  }
  if( oneMinuteTimer ){
    if( onlineClient ){
      refreshServer();
    }
    
    //Request Information from Slaves via RF24
    requestUpdateFromSlaves();
    
  }

  //Something is REALLY Wrong with DS1307
  if( ( segundo > 60 ) or ( minuto > 60 ) or (hora > 24) or (dia > 7) ){
    //Set Any Value form DS1307 Time
    setTimeDS1307( 17 , 5 , 29, 1, 14, 0 ); //year, month, day, day_week, (0 = Sun, 6 = Sat), hour, minute
  }
  
}




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
    
    if( DEBUG_PRINTING ){
      softSerial.print("Got: ");
      softSerial.println( messageRF24 );
    }
    
    if (messageRF24[5] == END_CHAR){
      
      if( messageRF24.substring(1,4) == "S1M" ){
        if( messageRF24.substring(4,5) == "0" ){
          if( DEBUG_PRINTING ){ softSerial.println("Presence Not Detected"); }
          motion1_State = false;
          report2Slave2(3);
        }

        else if( messageRF24.substring(4,5) == "1" ){
          if( DEBUG_PRINTING ){ softSerial.println("Presence Detected"); }
          motion1_State = true;
          report2Slave2(4);
        }
      }

      else if( messageRF24.substring(1,4) == "S2M" ){
        if( messageRF24.substring(4,5) == "0" ){
          if( DEBUG_PRINTING ){ softSerial.println("Lamp is OFF (MANUAL)"); }
          lamp1_State = 0;
        }

        else if( messageRF24.substring(4,5) == "1" ){
          if( DEBUG_PRINTING ){ softSerial.println("Lamp is ON (MANUAL)"); }
          lamp1_State = 1;
        }

        else if( messageRF24.substring(4,5) == "2" ){
          if( DEBUG_PRINTING ){ softSerial.println("Lamp is AUTO"); }
          lamp1_State = 2;
        }
      }
    }
    else if (messageRF24[6] == END_CHAR){
      
      if( messageRF24.substring(1,4) == "S3M" ){
        uint8_t buttonPressed = 99;

        buttonPressed = ( ( messageRF24.substring(4,6) ).toInt() );
        if( DEBUG_PRINTING ){
          softSerial.println("");
          softSerial.print("IR Remote Button: ");
          softSerial.println( buttonPressed, DEC);
        }

        char messageDisplay[17] = "IR REMOTE:      ";
        
        if( buttonPressed > 9 ){ messageDisplay[11] = '1'; }
        
        if(      buttonPressed == 0  ){ lamp1_State = 0; messageDisplay[12] = '0';report2Slave2(0); }
        else if( buttonPressed == 1  ){ lamp1_State = 1; messageDisplay[12] = '1';report2Slave2(1); }
        else if( buttonPressed == 2  ){ lamp1_State = 2; messageDisplay[12] = '2';report2Slave2(2); }
        else if( buttonPressed == 3  ){ messageDisplay[12] = '3'; }
        else if( buttonPressed == 4  ){ messageDisplay[12] = '4'; }
        else if( buttonPressed == 5  ){ messageDisplay[12] = '5'; }
        else if( buttonPressed == 6  ){ messageDisplay[12] = '6'; }
        else if( buttonPressed == 7  ){ messageDisplay[12] = '7'; }
        else if( buttonPressed == 8  ){ messageDisplay[12] = '8'; }
        else if( buttonPressed == 9  ){ messageDisplay[12] = '9'; }
        else if( buttonPressed == 10 ){ messageDisplay[12] = '0';nightMode = !nightMode; } //Button: *
        else if( buttonPressed == 11 ){ messageDisplay[12] = '1';alarmSet  = !alarmSet;  } //Button: #
        else if( buttonPressed == 12 ){ messageDisplay[12] = '2'; }
        else if( buttonPressed == 13 ){ messageDisplay[12] = '3'; }
        else if( buttonPressed == 14 ){ messageDisplay[12] = '4'; }
        else if( buttonPressed == 15 ){ messageDisplay[12] = '5'; }
        else if( buttonPressed == 16 ){ messageDisplay[12] = '6'; buzzerOn = false; digitalWrite(buzzerGPIO, LOW); } //Button: OK

        strncpy(msg2_temp, messageDisplay, 17); msg2Timer = 0;
        
      }
    }
  }
}

void report2Slave2(uint8_t cmd){
  char message[8] = "#MS20#";
  message[0]      = START_CHAR;
  message[5]      = END_CHAR;
  message[6]      = '\0';
  message[7]      = '\0';

  if(     cmd == 0 ){ message[4] = '0'; }
  else if(cmd == 1 ){ message[4] = '1'; }
  else if(cmd == 2 ){ message[4] = '2'; }
  else if(cmd == 3 ){ message[4] = '3'; }
  else if(cmd == 4 ){ message[4] = '4'; }
  else if(cmd == 5 ){ message[4] = '5'; }
  else if(cmd == 6 ){ message[4] = '6'; }
  else if(cmd == 7 ){ message[4] = '7'; }
  else if(cmd == 8 ){ message[4] = '8'; }
  else if(cmd == 9 ){ message[4] = '9'; }

  //Send the message to the slave
  radio.stopListening();
  radio.openWritingPipe( pipes[2] );
  radio.write( message, sizeof(message) );
  radio.startListening();
  if( DEBUG_PRINTING ){ 
    softSerial.println(" ");
    softSerial.print("Text Sent: ");
    softSerial.println( message );
  }
}

void requestUpdateFromSlaves(){
  char message[8] = "#MS1?#";
  message[0]      = START_CHAR;
  message[5]      = END_CHAR;
  message[6]      = '\0';
  message[7]      = '\0';

  //Request Slave 1 Status
  radio.stopListening();
  radio.openWritingPipe( pipes[1] );
  radio.write( message, sizeof(message) );
  radio.startListening();
  if( DEBUG_PRINTING ){ 
    softSerial.println(" ");
    softSerial.print("Text Sent: ");
    softSerial.println( message );
  }

  delay(100);
  
  //Request Slave 2 Status
  message[3] = '2';
  radio.stopListening();
  radio.openWritingPipe( pipes[2] );
  radio.write( message, sizeof(message) );
  radio.startListening();
  if( DEBUG_PRINTING ){ 
    softSerial.println(" ");
    softSerial.print("Text Sent: ");
    softSerial.println( message );
  }
}
//-----------------------------------------------



//---------------------------------------------------------------
//        FUNCTIONS: REAL TIME MODULE CONTROL
//---------------------------------------------------------------
byte dec2bcd(byte number) {
  return ((number / 10 * 16) + (number % 10));
}
uint8_t bcd2dec(uint8_t number) {
  return ((number >> 4) * 10 + (number & 0x0F));
}

void setTimeDS1307(byte ano, byte mes, byte dia, byte dia_semana, byte hora, byte minuto) //Seta a data e a hora do DS1307
{
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(0x00);

  Wire.write(dec2bcd(0));
  Wire.write(dec2bcd(minuto));
  Wire.write(dec2bcd(hora));
  Wire.write(dec2bcd(dia_semana));
  Wire.write(dec2bcd(dia));
  Wire.write(dec2bcd(mes));
  Wire.write(dec2bcd(ano));

  Wire.write(0x00);
  Wire.endTransmission();
}

void updateTime()
{
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(0x00);
  Wire.endTransmission();

  Wire.requestFrom(DS1307_ADDRESS, 7);
  segundo = bcd2dec(Wire.read());
  minuto = bcd2dec(Wire.read());
  hora = bcd2dec(Wire.read() & 0b111111);
  dia = bcd2dec(Wire.read());
  dia_mes = bcd2dec(Wire.read());
  mes = bcd2dec(Wire.read());
  ano = bcd2dec(Wire.read());
}
