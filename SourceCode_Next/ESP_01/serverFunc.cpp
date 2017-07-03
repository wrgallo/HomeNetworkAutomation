#include "./serverFunc.h"

void timerConfig();
void timerHandler(void *tCall);

//---------------------------------------------------------------------------------------
//                                      INTERNAL VARIABLES
//---------------------------------------------------------------------------------------
os_timer_t mTimer;
ESP8266WebServer myServer(80);        //Server object
volatile bool wifiConnected = false;  //indicates WiFi Connection Status
volatile uint8_t MCU_timeCounter = 90;//Counter of seconds without a MCU UART Answer
bool    stillBuffering = false;       //if there is a msg incomming
char    buf1[200];                    //to get the bytes out of UART buffer with only expected UART values
String  buffer_read = "";             //All msgs in current buffer
int     msgLen = 0;                   //Lenght of buffer_read 
uint8_t i = 0, j = 0, bufLen = 0;     //two counters and the Length of buf1
String  EEPROM_USER;                  //User Credential Username at EEPROM
String  EEPROM_PSWD;                  //User Credential Password at EEPROM
int8_t  EEPROM_TIMEZONE = 0;          //For Online Updating Time
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
//                            WEBSITE.H VARIABLES AND FUNCTIONS
//---------------------------------------------------------------------------------------
String websiteHTML,loginHtml,XML,Style,Footer;

//HOME AUTOMATION NETWORK INFORMATION - DEFAULT VALUES
String var01_lampState         = "Unknown";
String var02_motionSensorState = "Unknown";
String var03_buzzerState       = "Unknown";
String var04_time              = "HH:MM";
String var05_date              = "DD/MM/YY";
String var06_weekday           = "Unknown";
String var07_alarmTime         = "HH:MM";
String var08_alarmDay          = "Unknown";
String var09_alarmSet          = "Unknown";
String var10_nightTimeStart    = "HH:MM";
String var11_nightTimeEnd      = "HH:MM";
String var12_nightTimeSet      = "Unknown";
String var13_mailRecipient     = "Unknown";
String var15_masterUnit        = "OFFLINE";
String var16_slave1            = "OFFLINE";
String var17_slave2            = "OFFLINE";
String var18_slave3            = "OFFLINE";



//---------------------------------------------------------------------------------------
//                                     ESP Setup and Control
//---------------------------------------------------------------------------------------
void startESP(void){

  
  //-------------------------------------------------------------------------------------
  // WAIT 20 SECONDS BEFORE START, IF IT MAY RESET FOR ANY REASON, IT SHOULD HAPPEN NOW.
  delay(20000);
  //-------------------------------------------------------------------------------------


  //-------------------------------------------------------------------------------------
  //                               CONFIGURING UART CONNECTION
  Serial.begin(115200);  
  //-------------------------------------------------------------------------------------


  //TIMER TO AUTO RESET IF ESP8266 TOOK TOO LONG IN ACCESS POINT
  timerConfig();
  //-------------------------------------------------------------------------------------
  //                        CONNECTING TO WIFI, OR CREATING ACCESS POINT
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  
  //tries to connect to last known settings
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP" with password "password"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect( ESP_SSID , ESP_PSWD )) {
    //PROBLEM OCCURED DURING CONNECTION TO EXISTING WIFI
    Serial.write( START_CHAR );Serial.print("IP00");Serial.write( END_CHAR );
    delay(3000);
    ESP.reset();
    delay(5000);
  }
  //-------------------------------------------------------------------------------------


  //-------------------------------------------------------------------------------------
  //                            IF LOST CONNECTION, CONNECT AGAIN
  WiFi.setAutoReconnect(true);
  //-------------------------------------------------------------------------------------


  //-------------------------------------------------------------------------------------
  //                                    BUILDING STRINGS
  buildOnce();
  
  var13_mailRecipient = getEmailRecipient();
  EEPROM_USER         = getUserCredentialsLogin();
  EEPROM_PSWD         = getUserCredentialsPassword();
  EEPROM_TIMEZONE     = getTimezone();
  //-------------------------------------------------------------------------------------
  
  
  startServer();  

  //-------------------------------------------------------------------------------------
  //               WAIT 10 SECONDS BEFORE WE CONCLUDE IT STARTED PERFECTLY
  delay(10000);
  //-------------------------------------------------------------------------------------


  //-------------------------------------------------------------------------------------
  //                                    REPORT IP ADDRESS
  if(WiFi.status() == WL_CONNECTED){
    //SUCCESSFULLY CONNECTED TO EXISTING WIFI WITHOUT SELF-REBOOTING
    //RETURNING LOCAL IP ADDRESS
    Serial.write( START_CHAR );Serial.print("WEBOK");Serial.write( END_CHAR );
    Serial.write( START_CHAR );Serial.print("IP0");Serial.print( WiFi.localIP() );Serial.write( END_CHAR );
  }else{
    //PROBLEM OCCURED DURING CONNECTION TO EXISTING WIFI
    Serial.write( START_CHAR );Serial.print("IP00");Serial.write( END_CHAR );
    ESP.reset();
  }
  //-------------------------------------------------------------------------------------

  wifiConnected = true;

  sendOnlineTime();
  
}

void sendOnlineTime(){
  //--------------------------------------------------------------------------------------------------------------
  //                                    GET ONLINE TIME
  uint8_t i;
  for(i=0; i<10; i++){
    if( !requestUTC( EEPROM_TIMEZONE ) ){
      Serial.print("\nNTP Error");
    }else{ break; }
    i++;
    delay(1000);
  }
  if( i < 11 ){
    Serial.write(START_CHAR);Serial.print( "WTH" + (String)( getHour()    ) );Serial.write(END_CHAR);//TIME HOUR
    Serial.write(START_CHAR);Serial.print( "WTM" + (String)( getMinute()  ) );Serial.write(END_CHAR);//TIME MINUTE
    Serial.write(START_CHAR);Serial.print( "WDW" + (String)( getWeekday() ) );Serial.write(END_CHAR);//WEEKDAY
    Serial.write(START_CHAR);Serial.print( "WDD" + (String)( getDay()     ) );Serial.write(END_CHAR);//DAY
    Serial.write(START_CHAR);Serial.print( "WDM" + (String)( getMonth()   ) );Serial.write(END_CHAR);//MONTH
    Serial.write(START_CHAR);Serial.print( "WDY" + (String)( getYear()    ) );Serial.write(END_CHAR);//YEAR
  }
  //---------------------------------------------------------------------------------------------------------------
}

void startServer(){
  //-------------------------------------------------------------------------------------
  //                                  SERVER WEBPAGE HANDLERS
  myServer.onNotFound(handleNotFound);
  myServer.on("/xml"              , handleXML);
  myServer.on("/login"            , handleLogin);
  myServer.on("/"                 , handleRoot);
  myServer.on("/setTime"          , handleRoot);
  myServer.on("/setAlarmTime"     , handleRoot);
  myServer.on("/setNightTime"     , handleRoot);
  myServer.on("/setNewCredential" , handleRoot);
  myServer.on("/setMailRecipient" , handleRoot);
  myServer.on("/setTimezone"      , handleRoot);
  //-------------------------------------------------------------------------------------


  //-------------------------------------------------------------------------------------
  //                                    COOKIE CONFIGURATION
  //The list of headers to be recorded
  const char * headerkeys[] = {"User-Agent","Cookie"} ;
  size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
  //ask server to track these headers
  myServer.collectHeaders(headerkeys, headerkeyssize );
  myServer.begin();
  if(DEBUG_PRINTING){ Serial.println("[DEBUG] HTTP server started"); }
  //-------------------------------------------------------------------------------------
}

bool checkCredentials(String user, String pswd){
  if( ((user == ADMIN_NAME ) and (pswd == ADMIN_PSWD )) or
      ((user == EEPROM_USER) and (pswd == EEPROM_PSWD)) )
  {
      return true; 
  }
  return false;
}

void timerConfig(){
    os_timer_setfn(&mTimer, timerHandler, NULL);
    os_timer_arm(  &mTimer, 1000        , true);
}

void timerHandler(void *tCall){
  static uint16_t timeCounter = 0;
  
  if( !wifiConnected ){
    timeCounter++;
    
    if( timeCounter >= 600 ){
      timeCounter = 0;
      Serial.println("Timer Reset");
      ESP.restart();
    }
  }else{
    timeCounter = 0;
  }

  if( MCU_timeCounter < 70 ){
    MCU_timeCounter++;
    if( MCU_timeCounter == 70 ){
      var15_masterUnit = "OFFLINE";
    }
  }

}

bool is_authentified(){
  //Check if header is present and correct
  if(DEBUG_PRINTING){ Serial.println("[DEBUG] Enter is_authentified"); }
  if (myServer.hasHeader("Cookie")){
    String cookie = myServer.header("Cookie");
    if(DEBUG_PRINTING){ Serial.print("[DEBUG] Found cookie: "); }
    if(DEBUG_PRINTING){ Serial.println(cookie); }
    if (cookie.indexOf("ESPSESSIONID=1") != -1) {
      if(DEBUG_PRINTING){ Serial.println("[DEBUG] Authentification Successful"); }
      
      return true;
    }
  }
  if(DEBUG_PRINTING){ Serial.println("[DEBUG] Authentification Failed"); }
  return false;
}

void handleServer(void){
  if(WiFi.status() != WL_CONNECTED){ wifiConnected = false; }
  else{                              wifiConnected = true; }
  
  myServer.handleClient();
}


void handleMaster(void){

  if(Serial.available() > 0){
    stillBuffering = false;
    //------------------------------------------------------------
    //     READING UART VALUES AS VALID ASCII CHARACTERS ONLY
    //------------------------------------------------------------
    Serial.setTimeout(10);
    bufLen = Serial.readBytes(buf1 , 200);buf1[bufLen] = '\0';
    j=0;
    for(i=0; i<bufLen; i++){ 
      if( ( buf1[i] ==  START_CHAR ) or ( buf1[i] == END_CHAR ) or (( buf1[i]  > 32 ) && ( buf1[i]  < 127 )) ){ 
        buf1[j++] = buf1[i];
      } 
    }
    buf1[j] = '\0';

    buffer_read += buf1;
    msgLen = buffer_read.length();
    //------------------------------------------------------------
  }


  if( (msgLen > 0) and (stillBuffering == false) ){
    
    //FLUSHING CHARS UNTILL A VALID START CHAR IS FOUND
    while( (buffer_read.indexOf( START_CHAR ) != 0) and (msgLen > 0) ){
      buffer_read = buffer_read.substring(1);
      msgLen--;
    }
    if(DEBUG_PRINTING){ Serial.println( buffer_read ); }
    
    //WORD RECEIVED STARTS WITH A VALID START CHAR
    if( buffer_read.indexOf( START_CHAR ) == 0 ){
      stillBuffering = true;
      
      //WORD RECEIVED HAS THE VALID END CHAR
      int iterationCounter = 0;
      while( ( buffer_read.indexOf( END_CHAR ) >= 4 ) and (iterationCounter < 10) ) {
        iterationCounter++;
        stillBuffering = false;

        var15_masterUnit = "ONLINE";MCU_timeCounter = 0;
        
        //-----------------------------------------------------------------------------------------------
        //                                     CMDS THAT DO NOT NEED ANSWERS
        //-----------------------------------------------------------------------------------------------
        //LAMP STATE
        if(      ( buffer_read.substring(1,4) == "MLS" ) && ( buffer_read.indexOf( END_CHAR ) == 5 ) ){
          if(      buffer_read.substring(4,5) == "0" ){ var01_lampState = "OFF";   }
          else if( buffer_read.substring(4,5) == "1" ){ var01_lampState = "ON";  }
          else if( buffer_read.substring(4,5) == "2" ){ var01_lampState = "AUTO"; }
        }

        //MOTION SENSOR STATE
        else if( ( buffer_read.substring(1,4) == "MMS" ) && ( buffer_read.indexOf( END_CHAR ) == 5 ) ){
          if(      buffer_read.substring(4,5) == "0" ){ var02_motionSensorState = "PRESENCE NOT DETECTED";     }
          else if( buffer_read.substring(4,5) == "1" ){ var02_motionSensorState = "PRESENCE DETECTED"; }
        }

        //BUZZER STATE
        else if( ( buffer_read.substring(1,4) == "MBS" ) && ( buffer_read.indexOf( END_CHAR ) == 5 ) ){
          if(      buffer_read.substring(4,5) == "0" ){ var03_buzzerState = "OFF";  }
          else if( buffer_read.substring(4,5) == "1" ){ var03_buzzerState = "ON"; }
        }

        //TIME
        else if( ( buffer_read.substring(1,4) == "MT1" ) && ( buffer_read.indexOf( END_CHAR ) >= 7 ) && ( buffer_read.indexOf( END_CHAR ) <= 9 ) ){
          if(      buffer_read.indexOf(":") == 5 ){ 
            var04_time  = "0" + buffer_read.substring( 4 , 6 );
            if(    buffer_read.indexOf( END_CHAR ) == 7 ){ var04_time  += "0" + buffer_read.substring( 6 , 7 ); }
            else{                                          var04_time  +=       buffer_read.substring( 6 , 8 ); }
          }
          else if( buffer_read.indexOf(":") == 6 ){
            var04_time  =       buffer_read.substring( 4 , 7 );
            if(    buffer_read.indexOf( END_CHAR ) == 8 ){ var04_time  += "0" + buffer_read.substring( 7 , 8 ); }
            else{                                          var04_time  +=       buffer_read.substring( 7 , 9 ); }
          }
          
        }

        //DATE
        else if( ( buffer_read.substring(1,4) == "MT2" ) && ( buffer_read.indexOf( END_CHAR ) >= 9 ) && ( buffer_read.indexOf( END_CHAR ) <= 12 ) ){
          var05_date = buffer_read.substring( 4 , buffer_read.indexOf( END_CHAR ) );
        }

        //DAY OF THE WEEK
        else if( ( buffer_read.substring(1,4) == "MT3" ) && ( buffer_read.indexOf( END_CHAR ) == 5 ) ){
          if(      buffer_read.substring(4,5) == "0" ){ var06_weekday = "Sunday";   }
          else if( buffer_read.substring(4,5) == "1" ){ var06_weekday = "Monday";   }
          else if( buffer_read.substring(4,5) == "2" ){ var06_weekday = "Tuesday";  }
          else if( buffer_read.substring(4,5) == "3" ){ var06_weekday = "Wednesday";}
          else if( buffer_read.substring(4,5) == "4" ){ var06_weekday = "Thursday"; }
          else if( buffer_read.substring(4,5) == "5" ){ var06_weekday = "Friday";   }
          else if( buffer_read.substring(4,5) == "6" ){ var06_weekday = "Saturday"; }
        }

        //ALARM TIME
        else if( ( buffer_read.substring(1,4) == "MA1" ) && ( buffer_read.indexOf( END_CHAR ) >= 7 ) && ( buffer_read.indexOf( END_CHAR ) <= 9 ) ){
          if(      buffer_read.indexOf(":") == 5 ){ 
            var07_alarmTime  = "0" + buffer_read.substring( 4 , 6 );
            if(    buffer_read.indexOf( END_CHAR ) == 7 ){ var07_alarmTime  += "0" + buffer_read.substring( 6 , 7 ); }
            else{                                          var07_alarmTime  +=       buffer_read.substring( 6 , 8 ); }
          }
          else if( buffer_read.indexOf(":") == 6 ){
            var07_alarmTime  =       buffer_read.substring( 4 , 7 );
            if(    buffer_read.indexOf( END_CHAR ) == 8 ){ var07_alarmTime  += "0" + buffer_read.substring( 7 , 8 ); }
            else{                                          var07_alarmTime  +=       buffer_read.substring( 7 , 9 ); }
          }
        }

        //ALARM DAY OF THE WEEK
        else if( ( buffer_read.substring(1,4) == "MA2" ) && ( buffer_read.indexOf( END_CHAR ) == 5 ) ){
          if(      buffer_read.substring(4,5) == "0" ){ var08_alarmDay = "Sunday";   }
          else if( buffer_read.substring(4,5) == "1" ){ var08_alarmDay = "Monday";   }
          else if( buffer_read.substring(4,5) == "2" ){ var08_alarmDay = "Tuesday";  }
          else if( buffer_read.substring(4,5) == "3" ){ var08_alarmDay = "Wednesday";}
          else if( buffer_read.substring(4,5) == "4" ){ var08_alarmDay = "Thursday"; }
          else if( buffer_read.substring(4,5) == "5" ){ var08_alarmDay = "Friday";   }
          else if( buffer_read.substring(4,5) == "6" ){ var08_alarmDay = "Saturday"; }
        }

        //ALARM SET
        else if( ( buffer_read.substring(1,4) == "MA3" ) && ( buffer_read.indexOf( END_CHAR ) == 5 ) ){
          if(      buffer_read.substring(4,5) == "0" ){ var09_alarmSet = "OFF SCHEDULE";  }
          else if( buffer_read.substring(4,5) == "1" ){ var09_alarmSet = "ON SCHEDULE"; }
        }

        //NIGHT TIME START
        else if( ( buffer_read.substring(1,4) == "MN1" ) && ( buffer_read.indexOf( END_CHAR ) >= 7 ) && ( buffer_read.indexOf( END_CHAR ) <= 9 ) ){
          if(      buffer_read.indexOf(":") == 5 ){ 
            var10_nightTimeStart  = "0" + buffer_read.substring( 4 , 6 );
            if(    buffer_read.indexOf( END_CHAR ) == 7 ){ var10_nightTimeStart  += "0" + buffer_read.substring( 6 , 7 ); }
            else{                                          var10_nightTimeStart  +=       buffer_read.substring( 6 , 8 ); }
          }
          else if( buffer_read.indexOf(":") == 6 ){
            var10_nightTimeStart  =       buffer_read.substring( 4 , 7 );
            if(    buffer_read.indexOf( END_CHAR ) == 8 ){ var10_nightTimeStart  += "0" + buffer_read.substring( 7 , 8 ); }
            else{                                          var10_nightTimeStart  +=       buffer_read.substring( 7 , 9 ); }
          }
        }

        //NIGHT TIME END
        else if( ( buffer_read.substring(1,4) == "MN2" ) && ( buffer_read.indexOf( END_CHAR ) >= 7 ) && ( buffer_read.indexOf( END_CHAR ) <= 9 ) ){
          if(      buffer_read.indexOf(":") == 5 ){ 
            var11_nightTimeEnd  = "0" + buffer_read.substring( 4 , 6 );
            if(    buffer_read.indexOf( END_CHAR ) == 7 ){ var11_nightTimeEnd  += "0" + buffer_read.substring( 6 , 7 ); }
            else{                                          var11_nightTimeEnd  +=       buffer_read.substring( 6 , 8 ); }
          }
          else if( buffer_read.indexOf(":") == 6 ){
            var11_nightTimeEnd  =       buffer_read.substring( 4 , 7 );
            if(    buffer_read.indexOf( END_CHAR ) == 8 ){ var11_nightTimeEnd  += "0" + buffer_read.substring( 7 , 8 ); }
            else{                                          var11_nightTimeEnd  +=       buffer_read.substring( 7 , 9 ); }
          }
        }

        //NIGHT TIME SET
        else if( ( buffer_read.substring(1,4) == "MN3" ) && ( buffer_read.indexOf( END_CHAR ) == 5 ) ){
          if(      buffer_read.substring(4,5) == "0" ){ var12_nightTimeSet = "OFF SCHEDULE";  }
          else if( buffer_read.substring(4,5) == "1" ){ var12_nightTimeSet = "ON SCHEDULE"; }
        }


        //REQUEST SOFTWARE RESTART
        else if( ( buffer_read.substring(1,5) == "RST1" ) && ( buffer_read.indexOf( END_CHAR ) == 5 ) ){ 
          ESP.restart();
        }

        //REQUEST SOFTWARE RESET
        else if( ( buffer_read.substring(1,5) == "RST2" ) && ( buffer_read.indexOf( END_CHAR ) == 5 ) ){ 
          ESP.reset();
        }

        //REQUEST ONLINE TIME
        else if( ( buffer_read.substring(1,4) == "RTO" ) && ( buffer_read.indexOf( END_CHAR ) == 4 ) ){ 
          sendOnlineTime();
        }
        
        //REPORT SLAVE STATUS
        else if( (   buffer_read.substring(1,4) == "MSS" ) && ( buffer_read.indexOf( END_CHAR ) == 6 ) ){ 
          String temp = "";
          if(      buffer_read.substring(5,6) == "0" ){ temp = "OFFLINE"; }
          else if( buffer_read.substring(5,6) == "1" ){ temp = "ONLINE";  }
          
          if(      buffer_read.substring(4,5) == "1" ){ var16_slave1 = temp; }
          else if( buffer_read.substring(4,5) == "2" ){ var17_slave2 = temp; }
          else if( buffer_read.substring(4,5) == "3" ){ var18_slave3 = temp; }
        }

        //REPORT ALERT
        else if( ( buffer_read.substring(1,4) == "MRP" ) && ( buffer_read.indexOf( END_CHAR ) == 5 ) ){ 
          if(      buffer_read.substring(4,5) == "0" ){
            String MailContent = "<html>\n<body>\n<b>Intrude Alert Detected</b> at <i>" + var04_time + " of " + var05_date + "</i>\n</body>\n</html>";
            sendEmail( getEmailLogin(), getEmailPassword(), getEmailRecipient(), "!Intrude Alert!" , MailContent );
          }
          else if( buffer_read.substring(4,5) == "1" ){
            String MailContent = "<html>\n<body>\n<b>MCU Rebooted</b> at <i>" + var04_time + " of " + var05_date + "</i>\n</body>\n</html>";
            sendEmail( getEmailLogin(), getEmailPassword(), getEmailRecipient(), "!MCU Reset!" , MailContent );
          }
        }
        
        //-----------------------------------------------------------------------------------------------
        //                                          CMDS THAT NEED ANSWERS
        //-----------------------------------------------------------------------------------------------
        //REQUEST IP ADDRESS
        else if( ( buffer_read.substring(1,4) == "RIP" ) && ( buffer_read.indexOf( END_CHAR ) == 5 ) ){
          
          //REQUEST WIFI IP ADDRESS
          if(      buffer_read.substring(4,5) == "0" ){
            if(WiFi.status() == WL_CONNECTED){
              //RETURNING WF IP ADDRESS
              Serial.write( START_CHAR );Serial.print("IP0");Serial.print( WiFi.localIP() );Serial.write( END_CHAR );
            }else{
              //PROBLEM OCCURED DURING CREATION OF AN ACCESS POINT
              Serial.write( START_CHAR );Serial.print("IP00");Serial.write( END_CHAR );
            }
          }
          
        }
        
      //DELETING THE PART OF THE MSG THAT HAS ALREADY BEEN READ
      if( buffer_read.indexOf( END_CHAR ) > 0 ){ buffer_read = buffer_read.substring( buffer_read.indexOf( END_CHAR ) + 1 );msgLen = buffer_read.length(); }
      
      }
    }
  }
}
//---------------------------------------------------------------------------------------






//---------------------------------------------------------------------------------------
//                                       PAGES HANDLERS
//---------------------------------------------------------------------------------------

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += myServer.uri();
  message += "\nMethod: ";
  message += (myServer.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += myServer.args();
  message += "\n";
  for (uint8_t i=0; i<myServer.args(); i++){
    message += " " + myServer.argName(i) + ": " + myServer.arg(i) + "\n";
  }
  myServer.send(404, "text/plain", message);
}

void handleXML(){
  XML="<?xml version='1.0'?>";
  XML+="<response>\n";
  XML+="  <rvar01>" + var01_lampState         + "</rvar01>\n";
  XML+="  <rvar02>" + var02_motionSensorState + "</rvar02>\n";
  XML+="  <rvar03>" + var03_buzzerState       + "</rvar03>\n";
  XML+="  <rvar04>" + var04_time              + "</rvar04>\n";
  XML+="  <rvar05>" + var05_date              + "</rvar05>\n";
  XML+="  <rvar06>" + var06_weekday           + "</rvar06>\n";
  XML+="  <rvar07>" + var07_alarmTime         + "</rvar07>\n";
  XML+="  <rvar08>" + var08_alarmDay          + "</rvar08>\n";
  XML+="  <rvar09>" + var09_alarmSet          + "</rvar09>\n";
  XML+="  <rvar10>" + var10_nightTimeStart    + "</rvar10>\n";
  XML+="  <rvar11>" + var11_nightTimeEnd      + "</rvar11>\n";
  XML+="  <rvar12>" + var12_nightTimeSet      + "</rvar12>\n";
  XML+="  <rvar13>" + var13_mailRecipient     + "</rvar13>\n";
  if( EEPROM_TIMEZONE > 0 ){ XML+="  <rvar14>UTC +" + (String)(EEPROM_TIMEZONE) + "</rvar14>\n"; }
  else{                      XML+="  <rvar14>UTC "  + (String)(EEPROM_TIMEZONE) + "</rvar14>\n"; }
  XML+="  <rvar15>" + var15_masterUnit        + "</rvar15>\n";
  XML+="  <rvar16>" + var16_slave1            + "</rvar16>\n";
  XML+="  <rvar17>" + var17_slave2            + "</rvar17>\n";
  XML+="  <rvar18>" + var18_slave3            + "</rvar18>\n";
  XML+="</response>\n";
  
  myServer.send(200,"text/xml",XML);
}

void handleLogin(){
  //login page, also called for disconnect
  String msg = "";
  if (myServer.hasHeader("Cookie")){
    String cookie = myServer.header("Cookie");
    if(DEBUG_PRINTING){ Serial.print("[DEBUG] Found cookie: ");Serial.println(cookie); }
  }
  if (myServer.hasArg("DISCONNECT")){
    if(DEBUG_PRINTING){ Serial.println("Disconnection"); }
    myServer.sendHeader("Location","/login");
    myServer.sendHeader("Cache-Control","no-cache");
    myServer.sendHeader("Set-Cookie","ESPSESSIONID=0");
    myServer.send(301);
    return;
  }
  
  if (myServer.hasArg("USERNAME") && myServer.hasArg("PASSWORD")){
    if( checkCredentials( myServer.arg("USERNAME"), myServer.arg("PASSWORD") ) ){
      myServer.sendHeader("Location","/");
      myServer.sendHeader("Cache-Control","no-cache");
      myServer.sendHeader("Set-Cookie","ESPSESSIONID=1");
      myServer.send(301);
      if(DEBUG_PRINTING){ Serial.println("[DEBUG] Log in Successful"); }
      return;
    }
  msg = "Invalid Credentials!";
  }
  
  buildLoginPage( msg );
  myServer.send(200, "text/html", loginHtml);
}

void handleRoot(){
  //root page can be accessed only if authentification is ok
  String header;
  
  if (!is_authentified()){
    myServer.sendHeader("Location","/login");
    myServer.sendHeader("Cache-Control","no-cache");
    myServer.send(301);
    return;
  }

  bool sendTheHeader = false;
  
  if (myServer.hasArg("buttonChangeLampState")){
    if (myServer.arg("buttonChangeLampState") == "YES"){
      sendTheHeader = true;

      Serial.write(START_CHAR);Serial.print("WB11");Serial.write(END_CHAR);;
    }
  }

  if (myServer.hasArg("buttonChangeBuzzerState")){
    if (myServer.arg("buttonChangeBuzzerState") == "YES"){
      sendTheHeader = true;

      Serial.write(START_CHAR);Serial.print("WB12");Serial.write(END_CHAR);;
    }
  }

  if (myServer.hasArg("buttonChangeAlarmSet")){
    if (myServer.arg("buttonChangeAlarmSet") == "YES"){
      sendTheHeader = true;

      Serial.write(START_CHAR);Serial.print("WB13");Serial.write(END_CHAR);;
    }
  }

  if (myServer.hasArg("buttonChangeNightTimeSet")){
    if (myServer.arg("buttonChangeNightTimeSet") == "YES"){
      sendTheHeader = true;

      Serial.write(START_CHAR);Serial.print("WB14");Serial.write(END_CHAR);;
    }
  }

  if (myServer.hasArg("buttonEmailTest")){
    if (myServer.arg("buttonEmailTest") == "YES"){
      sendTheHeader = true;

      sendEmail( getEmailLogin(), getEmailPassword(), getEmailRecipient(), "Test" ,"<html>\n<body>\n<i>Email Test <b>Button</b> Pressed</i>\n</body>\n</html>" );
    }
  }

  if (myServer.hasArg("buttonGetTime")){
    if (myServer.arg("buttonGetTime") == "YES"){
      sendTheHeader = true;

      sendOnlineTime();
    }
  }
  

  if (myServer.hasArg("timeHour") && myServer.hasArg("timeMin")){
    sendTheHeader = true;

    Serial.write(START_CHAR);Serial.print( "WTH" + (String)((myServer.arg("timeHour")).toInt()) );Serial.write(END_CHAR);;
    Serial.write(START_CHAR);Serial.print( "WTM" + (String)((myServer.arg("timeMin" )).toInt()) );Serial.write(END_CHAR);;
  }

  if (myServer.hasArg("dateDay") && myServer.hasArg("dateMon") && myServer.hasArg("dateYea")){
    sendTheHeader = true;

    Serial.write(START_CHAR);Serial.print( "WDD" + (String)((myServer.arg("dateDay")).toInt()) );Serial.write(END_CHAR);;
    Serial.write(START_CHAR);Serial.print( "WDM" + (String)((myServer.arg("dateMon")).toInt()) );Serial.write(END_CHAR);;
    Serial.write(START_CHAR);Serial.print( "WDY" + (String)((myServer.arg("dateYea")).toInt()) );Serial.write(END_CHAR);;
  }

  if (myServer.hasArg("weekday")){
    sendTheHeader = true;

    Serial.write(START_CHAR);Serial.print( "WDW" + (String)((myServer.arg("weekday")).toInt()) );Serial.write(END_CHAR);;
  }

  if (myServer.hasArg("alarmHour") && myServer.hasArg("alarmMin")){
    sendTheHeader = true;

    Serial.write(START_CHAR);Serial.print( "WAH" + (String)((myServer.arg("alarmHour")).toInt()) );Serial.write(END_CHAR);;
    Serial.write(START_CHAR);Serial.print( "WAM" + (String)((myServer.arg("alarmMin" )).toInt()) );Serial.write(END_CHAR);;
  }

  if (myServer.hasArg("alarmDay")){
    sendTheHeader = true;

    Serial.write(START_CHAR);Serial.print( "WAD" + (String)((myServer.arg("alarmDay")).toInt()) );Serial.write(END_CHAR);
  }

  if (myServer.hasArg("nightTimeHourStart") && myServer.hasArg("nightTimeMinStart")){
    sendTheHeader = true;

    Serial.write(START_CHAR);Serial.print( "WN1" + (String)((myServer.arg("nightTimeHourStart")).toInt()) );Serial.write(END_CHAR);
    Serial.write(START_CHAR);Serial.print( "WN2" + (String)((myServer.arg("nightTimeMinStart" )).toInt()) );Serial.write(END_CHAR);
  }

  if (myServer.hasArg("nightTimeHourEnd") && myServer.hasArg("nightTimeMinEnd")){
    sendTheHeader = true;

    Serial.write(START_CHAR);Serial.print( "WN3" + (String)((myServer.arg("nightTimeHourEnd")).toInt()) );Serial.write(END_CHAR);
    Serial.write(START_CHAR);Serial.print( "WN4" + (String)((myServer.arg("nightTimeMinEnd" )).toInt()) );Serial.write(END_CHAR);
  }

  if (myServer.hasArg("mailRecipient")){
    sendTheHeader = true;

    var13_mailRecipient = (String)(myServer.arg("mailRecipient")); 
    setEmailRecipient( var13_mailRecipient );
  }

  if (myServer.hasArg("userCredentialName") && myServer.hasArg("userCredentialPswd1") && myServer.hasArg("userCredentialPswd2")){
    sendTheHeader = true;
    
    EEPROM_USER = (String)(myServer.arg("userCredentialName"));
    setUserCredentialsLogin( EEPROM_USER );

    EEPROM_PSWD = (String)(myServer.arg("userCredentialPswd1"));
    setUserCredentialsPassword( EEPROM_PSWD );
    
  }

  
  if (myServer.hasArg("timezoneR")){
    sendTheHeader = true;
    
    EEPROM_TIMEZONE = (myServer.arg("timezoneR")).toInt();    
    setTimezone( EEPROM_TIMEZONE );
  }
  
  
  if( sendTheHeader ){
    myServer.sendHeader("Location","/");
    myServer.sendHeader("Cache-Control","no-cache");
    myServer.send(301);
    return;
  }
  
  //CREATING AND SENDING THE .HTML FILE
  handleWebsite();
  
}

void handleWebsite(){
  /*
  myServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  myServer.sendHeader("Pragma", "no-cache");
  myServer.sendHeader("Expires", "-1");
  
  myServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  myServer.send(200, "text/html", "");
  //...
  myServer.client().stop();
  */
  
  buildWebsite();
  myServer.send(200, "text/html", websiteHTML);
  
}

//---------------------------------------------------------------------------------------

void buildOnce(){
  Style   = "<style>\n";
  Style  += "  body {\n";
  Style  += "      height: 420px;\n";
  Style  += "      margin: 0 auto;\n";
  Style  += "      font-family: Courier New, Courier, monospace;\n";
  Style  += "      font-size: 18px;\n";
  Style  += "      text-align: center;\n";
  Style  += "      color: lightcyan;\n";
  Style  += "      background-color: darkslategray;\n";
  Style  += "      background-color: background-image: -webkit-gradient(linear, left top, left bottom, from(darkgray), to(darkslategray));\n";
  Style  += "      background-image: -webkit-linear-gradient(top, darkgray, darkslategray);\n";
  Style  += "      background-image: -moz-linear-gradient(top, darkgray, darkslategray);\n";
  Style  += "      background-image: linear-gradient(to bottom, darkgray, darkslategray);\n";
  Style  += "      background-repeat: no-repeat;\n";
  Style  += "   }\n";
  Style  += "   p {\n";
  Style  += "      text-align: center;\n";
  Style  += "      font-size: 10px;\n";
  Style  += "      font-style: italic;\n";
  Style  += "      width: 100%;\n";
  Style  += "   }\n";
  Style  += "   h1{\n";
  Style  += "      text-align: center;\n";
  Style  += "      font-size: 24px;\n";
  Style  += "      width: 100%;\n";
  Style  += "   }\n";
  Style  += "   h2{\n";
  Style  += "      text-align: center;\n";
  Style  += "      font-size: 16px;\n";
  Style  += "      width: 100%;\n";
  Style  += "   }\n";
  Style  += "   sup{\n";
  Style  += "      text-align: center;\n";
  Style  += "      font-size: 16px;\n";
  Style  += "      color: lightpink;\n";
  Style  += "      font-style: italic;\n";
  Style  += "      width: 100%;\n";
  Style  += "   }\n";
  Style  += "</style>\n";
  
  Footer  = "  <table height='100px'></table>\n";
  Footer += "  <p><br>Wellington Rodrigo Gallo\n";
  Footer += "  <br><a href='mailto:w.r.gallo@grad.ufsc.br' style='color: lightcyan;'>w.r.gallo@grad.ufsc.br</a>\n";
  Footer += "  <br>2017</p>\n";
  Footer += "</BODY>\n";
  Footer += "</HTML>\n";
}

void buildLoginPage(String msg){
  loginHtml = "<!DOCTYPE html>\n";
  loginHtml +="<title>Enter Your Credentials</title>\n";
  loginHtml += "<html lang='en' >\n";

  loginHtml += Style;
  
  loginHtml += "    <body>\n";
  loginHtml += "    <h1> Home Automation Network </h1>\n";
  loginHtml += "    <h2> Enter your Credentials </h2>\n";
  loginHtml += "    <table height='100px'></table>\n";
  loginHtml += "    <table align='center'>\n";
  loginHtml += "    <form action='/login' method='POST'>\n";
  loginHtml += "            <tr>\n";
  loginHtml += "                <td align='right'>\n";
  loginHtml += "                <label class='center' for='USERNAME' style='margin: 0px 0px 0px 0px;color: lightcyan;'>USERNAME:</label>\n";
  loginHtml += "                </td>\n";
  loginHtml += "                <td align='left'>\n";
  loginHtml += "                <input name='USERNAME' id='USERNAME' type='text' class='loginForm' style='width: 227px' /><br />\n";
  loginHtml += "                </td>\n";
  loginHtml += "            </tr>\n";
  loginHtml += "            <tr>\n";
  loginHtml += "                <td align='right'>\n";
  loginHtml += "                <label class='left' for='PASSWORD' style='margin: 0px 0px 0px 0px;color: lightcyan ;'>PASSWORD:</label>\n";
  loginHtml += "                </td>\n";
  loginHtml += "                <td align='left'>\n";
  loginHtml += "                <input name='PASSWORD' id='PASSWORD' type='PASSWORD' class='loginForm' style='width:159px;' />\n";
  loginHtml += "                <input name='submit' type='submit' class='button_2' value='Sign In' />\n";
  loginHtml += "                </td>\n";
  loginHtml += "            </tr>\n";
  loginHtml += "        </form>\n";
  loginHtml += "        </table>\n";
  loginHtml += "        <sup> " + msg + " </sup>\n";

  loginHtml += Footer;
  
  loginHtml += "       </body>";
  loginHtml += "</html>\n";
}

void buildWebsite(){

  buildJavascript();

  /* HTML PAGE, WITH THE HOME NETWORK AUTOMATION INFORMATION */
  websiteHTML += Style;
  
  websiteHTML +="<BODY onload='process()'>\n";
  websiteHTML +="<h1> Home Automation Network </h1><br>\n";
  
  websiteHTML +="<br><table width=600px align='center' border='1' edgecolor='green' bgcolor=darkslategray>\n";
  websiteHTML +="      <tr><td align=left width='220px'>Lamp State</td>\n";
  websiteHTML +="          <td align=center > <A id='var01'> </A></td></tr>\n";
  websiteHTML +="      <tr><td align=left>Motion Sensor State</td>\n";
  websiteHTML +="          <td align=center> <A id='var02'></A> </td></tr>\n";
  websiteHTML +="      <tr><td align=left>Buzzer State</td>\n";
  websiteHTML +="          <td align=center> <A id='var03'></A> </td></tr>\n";
  websiteHTML +="      <tr><td align=left> Time </td>\n";
  websiteHTML +="          <td align=center> <A id='var04'></A> </td></tr>\n";
  websiteHTML +="      <tr><td align=left> Date (DD/MM/YY) </td>\n";
  websiteHTML +="          <td align=center> <A id='var05'></A> </td></tr>\n";
  websiteHTML +="      <tr><td align=left> Weekday </td>\n";
  websiteHTML +="          <td align=center> <A id='var06'></A> </td></tr>\n";
  websiteHTML +="      <tr><td align=left> Alarm Time </td>\n";
  websiteHTML +="          <td align=center> <A id='var07'></A> </td></tr>\n";
  websiteHTML +="      <tr><td align=left> Alarm Day </td>\n";
  websiteHTML +="          <td align=center> <A id='var08'></A> </td></tr>\n";
  websiteHTML +="      <tr><td align=left> Alarm Set </td>\n";
  websiteHTML +="          <td align=center> <A id='var09'></A> </td></tr>\n";
  websiteHTML +="      <tr><td align=left> Night Time Start </td>\n";
  websiteHTML +="          <td align=center> <A id='var10'></A> </td></tr>\n";
  websiteHTML +="      <tr><td align=left> Night Time End </td>\n";
  websiteHTML +="          <td align=center> <A id='var11'></A> </td></tr>\n";
  websiteHTML +="      <tr><td align=left> Night Time Set </td>\n";
  websiteHTML +="          <td align=center> <A id='var12'></A> </td></tr>\n";
  websiteHTML +="      <tr><td align=left> Mail Recipient </td>\n";
  websiteHTML +="          <td align=center> <A id='var13'></A> </td></tr>\n";
  websiteHTML +="     <tr><td align=left> Time Zone </td>\n";
  websiteHTML +="          <td align=center> <A id='var14'></A> </td></tr>\n";
  websiteHTML +="</table>\n";
  


  websiteHTML +="<br><table width=600px align='center'>\n";
  websiteHTML +="<a href='/?buttonChangeLampState=YES'><input name='buttonChangeLampState'   type='submit' class='button_2' value='Change Lamp State'   ></a>\n";
  websiteHTML +="&nbsp<a href='/?buttonChangeBuzzerState=YES'><input name='buttonChangeBuzzerState' type='submit' class='button_2' value='Change Buzzer State' ></a>\n";
  websiteHTML +="&nbsp<a href='/?buttonEmailTest=YES'><input name='buttonEmailTest' type='submit' class='button_2' value='Test Email' ></a>\n";
  websiteHTML +="&nbsp<a href='/?buttonGetTime=YES'><input name='buttonGetTime' type='submit' class='button_2' value='Get Time Online' ></a>\n";
  websiteHTML +="</table>\n";

  websiteHTML +="<br><table width=600px align='center' border='1' edgecolor='green' bgcolor=darkslategray>\n";
  websiteHTML +="<form action='/setTimezone' method='POST'>\n";
  websiteHTML +="<td><table width=420px align='left'>\n";
  websiteHTML +="<tr><td align='left' width=210px>Time Zone: </td>\n";
  websiteHTML +="<td align='left' width=210px >\n";
  websiteHTML +="    <input name='timezoneR' id='timezoneR' type='range' min='-12' max='12' value='0' step='1' onchange='showValue(this.value)' style='width:200px'/>\n";
  websiteHTML +="</td></tr>\n";
  websiteHTML +="<tr><td align='left' width=210px></td><td align='center' width=210px><span id='timezone'></span></td></tr>\n";
  websiteHTML +="</table></td>\n";
  websiteHTML +="<td align='center' width=160px><input name='buttonSetTimezone' type='submit' class='button_2' value='Update Time Zone' /></td>\n";
  websiteHTML +="</form></table>\n";

  websiteHTML +="<br><table width=600px align='center' border='1' edgecolor='green' bgcolor=darkslategray>\n";
  websiteHTML +="  <form action='/setTime' method='POST'>\n";
  websiteHTML +="  <td><table width=420px align='left'>\n";
  websiteHTML +="  <tr><td align='left' width=210px>Time: </td>\n";
  websiteHTML +="  <td align='left' width=210px >\n";
  websiteHTML +="      <input name='timeHour' id='timeHour' type='number' min='0' max='23' class='setTimeForm' placeholder='Hour'   style='width:90px; text-align: center' required/>\n";
  websiteHTML +="      <input name='timeMin'  id='timeMin'  type='number' min='0' max='59' class='setTimeForm' placeholder='Minute' style='width:90px; text-align: center' required/>\n";
  websiteHTML +="  </td></tr>\n";
  websiteHTML +="  <tr><td align='left' width=210px>Date: </td>\n";
  websiteHTML +="  <td align='left' width=210px>\n";
  websiteHTML +="      <input name='dateDay'  id='dateDay' type='number' min='1'  max='31' class='setTimeForm' placeholder='Day'   style='width:55px; text-align: center' required/>\n";
  websiteHTML +="      <input name='dateMon'  id='dateMon' type='number' min='1'  max='12' class='setTimeForm' placeholder='Month' style='width:55px; text-align: center' required/>\n";
  websiteHTML +="      <input name='dateYea'  id='dateYea' type='number' min='17' max='50' class='setTimeForm' placeholder='Year'  style='width:55px; text-align: center' required/>\n";
  websiteHTML +="  </td></tr>\n";
  websiteHTML +="  <tr><td align='left' width=210px>Weekday: </td>\n";
  websiteHTML +="  <td align='left'><select style='width: 200px;' name='weekday'>\n";
  websiteHTML +="      <option value=0>Sunday</option>\n";
  websiteHTML +="      <option value=1>Monday</option>\n";
  websiteHTML +="      <option value=2>Tuesday</option>\n";
  websiteHTML +="      <option value=3>Wednesday</option>\n";
  websiteHTML +="      <option value=4>Thursday</option>\n";
  websiteHTML +="      <option value=5>Friday</option>\n";
  websiteHTML +="      <option value=6>Saturday</option>\n";
  websiteHTML +="  </select></td></tr></table></td>\n";
  websiteHTML +="  <td align='center' width=160px><input name='buttonSetTime' type='submit' class='button_2' value='Update Time' /></td>\n";
  websiteHTML +="</form></table>\n";
    


  websiteHTML +="<br><table width=600px align='center' border='1' edgecolor='green' bgcolor=darkslategray>\n";
  websiteHTML +="  <form action='/setAlarmTime' method='POST'>\n";
  websiteHTML +="  <td><table width=420px align='left'>\n";
  websiteHTML +="  <tr><td align='left' width=210px>Alarm Time: </td>\n";
  websiteHTML +="  <td align='left' width=210px>\n";
  websiteHTML +="      <input name='alarmHour' id='alarmHour' type='number' min='0' max='23' class='setAlarmForm' placeholder='Hour'   style='width:90px; text-align: center' required/>\n";
  websiteHTML +="      <input name='alarmMin'  id='alarmMin'  type='number' min='0' max='59' class='setAlarmForm' placeholder='Minute' style='width:90px; text-align: center' required/>\n";
  websiteHTML +="  </td></tr>\n";
  websiteHTML +="  <tr><td align='left' width=210px>Alarm Day: </td>\n";
  websiteHTML +="      <td align='left'><select style='width: 200px;' name='alarmDay' >\n";
  websiteHTML +="      <option value=0>Sunday</option>\n";
  websiteHTML +="      <option value=1>Monday</option>\n";
  websiteHTML +="      <option value=2>Tuesday</option>\n";
  websiteHTML +="      <option value=3>Wednesday</option>\n";
  websiteHTML +="      <option value=4>Thursday</option>\n";
  websiteHTML +="      <option value=5>Friday</option>\n";
  websiteHTML +="      <option value=6>Saturday</option>\n";
  websiteHTML +="  </select></td></tr></table></td>\n";
  websiteHTML +="  <td align='center' width=160px><input name='buttonSetAlarmTime' type='submit' class='button_2' value='Update Alarm'/>\n";
  websiteHTML +="  </form>\n";
  websiteHTML +="  <a href='/?buttonChangeAlarmSet=YES'><input name='buttonChangeAlarmSet' type='submit' class='button_2' value=' Change Alarm Set ' ></a></td>\n";
  websiteHTML +="</table>\n";



  websiteHTML +="<br><table width=600px align='center' border='1' edgecolor='green' bgcolor=darkslategray>\n";
  websiteHTML +="  <form action='/setNightTime' method='POST'>\n";
  websiteHTML +="  <td><table width=420px align='left'>\n";
  websiteHTML +="  <tr><td align='left' width=210px>Night Time Start: </td>\n";
  websiteHTML +="  <td align='left' width=210px>\n";
  websiteHTML +="      <input name='nightTimeHourStart' id='nightTimeHourStart' type='number' min='0' max='23' class='setNightTimeForm' placeholder='Hour'   style='width:90px; text-align: center' required/>\n";
  websiteHTML +="      <input name='nightTimeMinStart'  id='nightTimeMinStart'  type='number' min='0' max='59' class='setNightTimeForm' placeholder='Minute' style='width:90px; text-align: center' required/>\n";
  websiteHTML +="  </td></tr>\n";
  websiteHTML +="  <tr><td align='left' width=210px>Night Time End: </td>\n";
  websiteHTML +="  <td align='left' width=210px>\n";
  websiteHTML +="      <input name='nightTimeHourEnd' id='nightTimeHourEnd' type='number' min='0' max='23' class='setNightTimeForm' placeholder='Hour'   style='width:90px; text-align: center' required/>\n";
  websiteHTML +="      <input name='nightTimeMinEnd'  id='nightTimeMinEnd'  type='number' min='0' max='59' class='setNightTimeForm' placeholder='Minute' style='width:90px; text-align: center' required/>\n";
  websiteHTML +="  </td></tr></table></td>\n";
  websiteHTML +="  <td align='center' width=160px><input name='buttonsetNightTimeStart' type='submit' class='button_2' value='Update Night Time' />\n";
  websiteHTML +="  </form>\n";
  websiteHTML +="  <a href='/?buttonChangeNightTimeSet=YES'><input name='buttonChangeNightTimeSet' type='submit' class='button_2' value='Change Night Time Set' ></a></td>\n";
  websiteHTML +="</table>\n";



  websiteHTML +="<br><table width=600px align='center' border='1' edgecolor='green' bgcolor=darkslategray>\n";
  websiteHTML +="  <form action='/setMailRecipient' method='POST'>\n";
  websiteHTML +="  <td><table width=420px align='left'>\n";
  websiteHTML +="  <tr><td align='left' width=210px>Email Recipient: </td>\n";
  websiteHTML +="  <td align='left' width=205px >\n";
  websiteHTML +="      <input name='mailRecipient' id='mailRecipient' type='email' maxlength='50' class='setMailRecipientForm' placeholder='example@example.com'  style='width:195px; text-align: center' required/>\n";
  websiteHTML +="  </td></tr></table></td>\n";
  websiteHTML +="  <td align='center' width=160px><input name='buttonSetMailRecipient' type='submit' class='button_2' value='Change Mail Recipient' /></td>\n";
  websiteHTML +="</form></table>\n";



  websiteHTML +="<br><table width=600px align='center' border='1' edgecolor='green' bgcolor=darkslategray>\n";
  websiteHTML +="  <form action='/setNewCredential' method='POST' onsubmit='return checkPassword(this);'>\n";
  websiteHTML +="  <td><table width=420px align='left'>\n";
  websiteHTML +="  <tr><td align='left' width=210px>User Credential: </td>\n";
  websiteHTML +="  <td align='left' width=205px>\n";
  websiteHTML +="      <input name='userCredentialName' id='userCredentialName' type='text'     minlength='5' maxlength='20' class='setCredentialFrom' placeholder='Username'  style='width:195px; text-align: center' required/>\n";
  websiteHTML +="  </td></tr>\n";

  websiteHTML +="  <tr><td align='left' width=210px>User Password: </td>\n";
  websiteHTML +="  <td align='left' width=205px>\n";
  websiteHTML +="      <input name='userCredentialPswd1' id='userCredentialPswd1' type='password' minlength='8' maxlength='20' class='setCredentialFrom' placeholder='Password'  style='width:195px; text-align: center' required/>\n";
  websiteHTML +="  </td></tr>\n";
  websiteHTML +="  <tr><td align='left' width=210px>Confirm Password: </td>\n";
  websiteHTML +="  <td align='left' width=205px>\n";
  websiteHTML +="      <input name='userCredentialPswd2' id='userCredentialPswd2' type='password' minlength='8' maxlength='20' class='setCredentialFrom' placeholder='Password'  style='width:195px; text-align: center' required/>\n";
  websiteHTML +="  </td></tr>\n";
  websiteHTML +="  </table></td>\n";
  websiteHTML +="  <td align='center' width=160px><input name='buttonSetCredentials' type='submit' class='button_2' value='Change Credentials' /></td>\n";
  websiteHTML +="</form></table>\n";


  websiteHTML +="<br><table align=center width='600px' border='1' edgecolor='gray' bgcolor='darkslategray'>\n";
  websiteHTML +="      <tr><td align=left width='220px'>Master Unit</td>\n";
  websiteHTML +="          <td align=center > <A id='var15'> </A></td></tr>\n";
  websiteHTML +="      <tr><td align=left>Slave Unit 1</td>\n";
  websiteHTML +="          <td align=center> <A id='var16'></A> </td></tr>\n";
  websiteHTML +="      <tr><td align=left>Slave Unit 2</td>\n";
  websiteHTML +="          <td align=center> <A id='var17'></A> </td></tr>\n";
  websiteHTML +="      <tr><td align=left>Slave Unit 3</td>\n";
  websiteHTML +="          <td align=center> <A id='var18'></A> </td></tr>\n";
  websiteHTML +="</table>\n";


  websiteHTML +="<br><br>You can access this page until you <a style='color: lightcyan;' href='/login?DISCONNECT=YES'>disconnect</a>";


  websiteHTML += Footer ;

}

void buildJavascript(){
  websiteHTML ="<!DOCTYPE HTML>\n";
  websiteHTML +="<title>Home Automation Network</title>\n";
  
  websiteHTML +="<SCRIPT>\n";
  websiteHTML +="var xmlHttp=createXmlHttpObject();\n";

  websiteHTML +="function createXmlHttpObject(){\n";
  websiteHTML +=" if(window.XMLHttpRequest){\n";
  websiteHTML +="    xmlHttp=new XMLHttpRequest();\n";
  websiteHTML +=" }else{\n";
  websiteHTML +="    xmlHttp=new ActiveXObject('Microsoft.XMLHTTP');\n";
  websiteHTML +=" }\n";
  websiteHTML +=" return xmlHttp;\n";
  websiteHTML +="}\n";

  websiteHTML +="function process(){\n";
  websiteHTML +=" if(xmlHttp.readyState==0 || xmlHttp.readyState==4){\n";
  websiteHTML +="   xmlHttp.open('PUT','xml',true);\n";
  websiteHTML +="   xmlHttp.onreadystatechange=handleServerResponse;\n"; // no brackets?????
  websiteHTML +="   xmlHttp.send(null);\n";
  websiteHTML +=" }\n";
  websiteHTML +=" setTimeout('process()',1000);\n";
  websiteHTML +="}\n";
  
  websiteHTML +="function handleServerResponse(){\n";
  websiteHTML +=" if(xmlHttp.readyState==4 && xmlHttp.status==200){\n";
    
  websiteHTML +="   xmlResponse=xmlHttp.responseXML;\n";
  websiteHTML +="   xmldoc = xmlResponse.getElementsByTagName('response');\n";
  //UPDATING VARIABLES ON PAGE
  websiteHTML +="   message = xmldoc[0];\n";
  websiteHTML +="   document.getElementById('var01').innerHTML = message.children[0].firstChild.data;\n";
  websiteHTML +="   document.getElementById('var02').innerHTML = message.children[1].firstChild.data;\n";
  websiteHTML +="   document.getElementById('var03').innerHTML = message.children[2].firstChild.data;\n";
  websiteHTML +="   document.getElementById('var04').innerHTML = message.children[3].firstChild.data;\n";
  websiteHTML +="   document.getElementById('var05').innerHTML = message.children[4].firstChild.data;\n";
  websiteHTML +="   document.getElementById('var06').innerHTML = message.children[5].firstChild.data;\n";
  websiteHTML +="   document.getElementById('var07').innerHTML = message.children[6].firstChild.data;\n";
  websiteHTML +="   document.getElementById('var08').innerHTML = message.children[7].firstChild.data;\n";
  websiteHTML +="   document.getElementById('var09').innerHTML = message.children[8].firstChild.data;\n";
  websiteHTML +="   document.getElementById('var10').innerHTML = message.children[9].firstChild.data;\n";
  websiteHTML +="   document.getElementById('var11').innerHTML = message.children[10].firstChild.data;\n";
  websiteHTML +="   document.getElementById('var12').innerHTML = message.children[11].firstChild.data;\n";
  websiteHTML +="   document.getElementById('var13').innerHTML = message.children[12].firstChild.data;\n";
  websiteHTML +="   document.getElementById('var14').innerHTML = message.children[13].firstChild.data;\n";
  websiteHTML +="   document.getElementById('var15').innerHTML = message.children[14].firstChild.data;\n";
  websiteHTML +="   document.getElementById('var16').innerHTML = message.children[15].firstChild.data;\n";
  websiteHTML +="   document.getElementById('var17').innerHTML = message.children[16].firstChild.data;\n";
  websiteHTML +="   document.getElementById('var18').innerHTML = message.children[17].firstChild.data;\n";
  websiteHTML +=" }\n";
  websiteHTML +="}\n";

  websiteHTML +="function checkPassword(form){\n";
  websiteHTML +="  re = /^\\w+$/;\n";
  websiteHTML +="  if(!re.test(form.userCredentialName.value)) {\n";
  websiteHTML +="    alert('Error: Username must contain only letters, numbers and underscores!');\n";
  websiteHTML +="    form.userCredentialName.focus();\n";
  websiteHTML +="    return false;\n";
  websiteHTML +="  }\n";
  websiteHTML +="  if(form.userCredentialPswd1.value == form.userCredentialPswd2.value) {\n";
  websiteHTML +="    if(form.userCredentialPswd1.value == form.userCredentialName.value) {\n";
  websiteHTML +="      alert('Error: Password must be different from Username!');\n";
  websiteHTML +="      form.userCredentialPswd1.focus();\n";
  websiteHTML +="      return false;\n";
  websiteHTML +="    }\n";
  websiteHTML +="    re = /[0-9]/;\n";
  websiteHTML +="    if(!re.test(form.userCredentialPswd1.value)) {\n";
  websiteHTML +="      alert('Error: password must contain at least one number (0-9)!');\n";
  websiteHTML +="      form.userCredentialPswd1.focus();\n";
  websiteHTML +="      return false;\n";
  websiteHTML +="    }\n";
  websiteHTML +="    re = /[a-z]/;\n";
  websiteHTML +="    if(!re.test(form.userCredentialPswd1.value)) {\n";
  websiteHTML +="      alert('Error: password must contain at least one lowercase letter (a-z)!');\n";
  websiteHTML +="      form.userCredentialPswd1.focus();\n";
  websiteHTML +="      return false;\n";
  websiteHTML +="    }\n";
  websiteHTML +="    re = /[A-Z]/;\n";
  websiteHTML +="    if(!re.test(form.userCredentialPswd1.value)) {\n";
  websiteHTML +="      alert('Error: password must contain at least one uppercase letter (A-Z)!');\n";
  websiteHTML +="      form.userCredentialPswd1.focus();\n";
  websiteHTML +="      return false;\n";
  websiteHTML +="    }\n";
  websiteHTML +="  }\n";
  websiteHTML +="  return true;\n";
  websiteHTML +="}\n";

  websiteHTML +="function showValue(newValue)\n";
  websiteHTML +="{\n";
  websiteHTML +="  if( newValue > 0 ){\n";
  websiteHTML +="      document.getElementById('timezone').innerHTML='UTC +' + newValue;\n";
  websiteHTML +="  }else{\n";
  websiteHTML +="      document.getElementById('timezone').innerHTML='UTC ' + newValue;\n";
  websiteHTML +="  }\n";
  websiteHTML +="}\n";
  
  websiteHTML +="</SCRIPT>\n";
}

//---------------------------------------------------------------------------------------

