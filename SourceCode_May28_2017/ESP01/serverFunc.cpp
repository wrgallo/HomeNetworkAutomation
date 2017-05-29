#include "./serverFunc.h"

ESP8266WebServer myServer(80);   //Server object

//---------------------------------------------------------------------------------------
//                      DEFINES, PARAMETERS AND INTERNAL VARIABLES
//---------------------------------------------------------------------------------------
//DEBUG PARAMETERS
#define DEBUG_PRINTING          false

//ESP8266 ACESS POINT PARAMETERS
#define ESP_SSID                "HomeAutomationNetwork"
#define ESP_PSWD                "12345678"
  
//EXISTING WIFI PARAMETERS
#define WIFI_SSID               "PontoAcesso"
#define WIFI_PASSWORD           "12348765"
#define MAX_CONNECTION_ATTEMPTS 50

//HOME NETWORK LOGIN CREDENTIALS
#define USER_NICK               "wrgallo"
#define USER_PSWD               "1234"

//COMMUNICATION BEETWEEN UNITS PROTOCOL
#define START_CHAR              2 //DEC NUMBER OF ASCII TABLE
#define END_CHAR                3 //DEC NUMBER OF ASCII TABLE


//INTERNAL VARIABLES
bool    myAPConnected = false;        //if espAP was successfully created
bool    stillBuffering = false;       //if there is a msg incomming
char    buf1[200];                    //to get the bytes out of UART buffer with only valid UART values
String  buffer_read = "";             //All msgs in current buffer
int     msgLen = 0;                   //Lenght of buffer_read 
uint8_t i = 0, j = 0, bufLen = 0;     //two counters and the Length of buf1
//---------------------------------------------------------------------------------------


//Check if header is present and correct
bool is_authentified(){
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

//login page, also called for disconnect
void handleLogin(){
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
    if (myServer.arg("USERNAME") == USER_NICK && myServer.arg("PASSWORD") == USER_PSWD ){
      myServer.sendHeader("Location","/");
      myServer.sendHeader("Cache-Control","no-cache");
      myServer.sendHeader("Set-Cookie","ESPSESSIONID=1");
      myServer.send(301);
      if(DEBUG_PRINTING){ Serial.println("[DEBUG] Log in Successful"); }
      return;
    }
  msg = "Invalid Credentials!";
  }
  
  String content = "<!DOCTYPE html>\n";
  content +="<title>Enter Your Credentials</title>\n";
  content += "<html lang='en' >\n";
  content += "    <style>\n";
  content += "        body {\n";
  content += "            height: 420px;\n";
  content += "            margin: 0 auto;\n";
  content += "            font-family: Courier New, Courier, monospace;\n";
  content += "            font-size: 18px;\n";
  content += "            text-align: center;\n";
  content += "            color: lightcyan;\n";
  content += "            background-color: darkslategray;\n";
  content += "            background-color: background-image: -webkit-gradient(linear, left top, left bottom, from(darkgray), to(darkslategray));\n";
  content += "            background-image: -webkit-linear-gradient(top, darkgray, darkslategray);\n";
  content += "            background-image: -moz-linear-gradient(top, darkgray, darkslategray);\n";
  content += "            background-image: linear-gradient(to bottom, darkgray, darkslategray);\n";
  content += "            background-repeat: no-repeat;\n";
  content += "        }\n";
  content += "            p {\n";
  content += "                text-align: center;\n";
  content += "                font-size: 10px;\n";
  content += "                font-style: italic;\n";
  content += "                width: 100%;\n";
  content += "            }\n";
  content += "            h1{\n";
  content += "                text-align: center;\n";
  content += "                font-size: 24px;\n";
  content += "                width: 100%;\n";
  content += "            }\n";
  content += "            h2{\n";
  content += "                text-align: center;\n";
  content += "                font-size: 16px;\n";
  content += "                width: 100%;\n";
  content += "            }\n";
  content += "           sup{\n";
  content += "                text-align: center;\n";
  content += "                font-size: 16px;\n";
  content += "                color: lightpink;\n";
  content += "                font-style: italic;\n";
  content += "                width: 100%;\n";
  content += "            }\n";
  content += "    </style>\n";
  content += "    <body>\n";
  content += "    <h1> Home Automation Network </h1>\n";
  content += "    <h2> Enter your Credentials </h2>\n";
  content += "    <table height='100px'></table>\n";
  content += "    <table align='center'>\n";
  content += "    <form action='/login' method='POST'>\n";
  content += "            <tr>\n";
  content += "                <td align='right'>\n";
  content += "                <label class='center' for='USERNAME' style='margin: 0px 0px 0px 0px;color: lightcyan;'>USERNAME:</label>\n";
  content += "                </td>\n";
  content += "                <td align='left'>\n";
  content += "                <input name='USERNAME' id='USERNAME' type='text' class='loginForm' style='width: 227px' /><br />\n";
  content += "                </td>\n";
  content += "            </tr>\n";
  content += "            <tr>\n";
  content += "                <td align='right'>\n";
  content += "                <label class='left' for='PASSWORD' style='margin: 0px 0px 0px 0px;color: lightcyan ;'>PASSWORD:</label>\n";
  content += "                </td>\n";
  content += "                <td align='left'>\n";
  content += "                <input name='PASSWORD' id='PASSWORD' type='PASSWORD' class='loginForm' style='width:159px;' />\n";
  content += "                <input name='submit' type='submit' class='button_2' value='Sign In' />\n";
  content += "                </td>\n";
  content += "            </tr>\n";
  content += "        </form>\n";
  content += "        </table>\n";
  content += "        <sup> " + msg + " </sup>\n";
  content += "        <table height='100px'></table>\n";
  content += "            <p>\n";
  content += "                <br>Wellington Rodrigo Gallo\n";
  content += "                <br><a href='mailto:w.r.gallo@grad.ufsc.br' style='color: lightcyan;'>w.r.gallo@grad.ufsc.br</a>\n";
  content += "                <br>2017\n";
  content += "            </p>\n";
  content += "       </body>";
  content += "</html>\n";
  myServer.send(200, "text/html", content);
}

//root page can be accessed only if authentification is ok
void handleRoot(){
  String header;
  if (!is_authentified()){
    myServer.sendHeader("Location","/login");
    myServer.sendHeader("Cache-Control","no-cache");
    myServer.send(301);
    return;
  }

  //REPORT TO MASTER UNIT THAT THERE IS SOMEONE AUTHENTIFIED
  Serial.write( START_CHAR );Serial.print("ICL1");Serial.write( END_CHAR );

  if (myServer.hasArg("buttonChangeLampState")){
    if (myServer.arg("buttonChangeLampState") == "YES"){
      myServer.sendHeader("Location","/");
      myServer.sendHeader("Cache-Control","no-cache");
      myServer.send(301);

      Serial.write(START_CHAR);Serial.print("WB11");Serial.write(END_CHAR);;
    }
  }

  if (myServer.hasArg("buttonChangeBuzzerState")){
    if (myServer.arg("buttonChangeBuzzerState") == "YES"){
      myServer.sendHeader("Location","/");
      myServer.sendHeader("Cache-Control","no-cache");
      myServer.send(301);

      Serial.write(START_CHAR);Serial.print("WB12");Serial.write(END_CHAR);;
    }
  }

  if (myServer.hasArg("buttonChangeAlarmSet")){
    if (myServer.arg("buttonChangeAlarmSet") == "YES"){
      myServer.sendHeader("Location","/");
      myServer.sendHeader("Cache-Control","no-cache");
      myServer.send(301);

      Serial.write(START_CHAR);Serial.print("WB13");Serial.write(END_CHAR);;
    }
  }

  if (myServer.hasArg("buttonChangeNightTimeSet")){
    if (myServer.arg("buttonChangeNightTimeSet") == "YES"){
      myServer.sendHeader("Location","/");
      myServer.sendHeader("Cache-Control","no-cache");
      myServer.send(301);

      Serial.write(START_CHAR);Serial.print("WB14");Serial.write(END_CHAR);;
    }
  }

  if (myServer.hasArg("timeHour") && myServer.hasArg("timeMin")){
    myServer.sendHeader("Location","/");
    myServer.sendHeader("Cache-Control","no-cache");
    myServer.send(301);

    Serial.write(START_CHAR);Serial.print( "WTH" + (String)((myServer.arg("timeHour")).toInt()) );Serial.write(END_CHAR);;
    Serial.write(START_CHAR);Serial.print( "WTM" + (String)((myServer.arg("timeMin" )).toInt()) );Serial.write(END_CHAR);;
  }

  if (myServer.hasArg("dateDay") && myServer.hasArg("dateMon") && myServer.hasArg("dateYea")){
    myServer.sendHeader("Location","/");
    myServer.sendHeader("Cache-Control","no-cache");
    myServer.send(301);

    Serial.write(START_CHAR);Serial.print( "WDD" + (String)((myServer.arg("dateDay")).toInt()) );Serial.write(END_CHAR);;
    Serial.write(START_CHAR);Serial.print( "WDM" + (String)((myServer.arg("dateMon")).toInt()) );Serial.write(END_CHAR);;
    Serial.write(START_CHAR);Serial.print( "WDY" + (String)((myServer.arg("dateYea")).toInt()) );Serial.write(END_CHAR);;
  }

  if (myServer.hasArg("weekday")){
    myServer.sendHeader("Location","/");
    myServer.sendHeader("Cache-Control","no-cache");
    myServer.send(301);

    Serial.write(START_CHAR);Serial.print( "WDW" + (String)((myServer.arg("weekday")).toInt()) );Serial.write(END_CHAR);;
  }

  if (myServer.hasArg("alarmHour") && myServer.hasArg("alarmMin")){
    myServer.sendHeader("Location","/");
    myServer.sendHeader("Cache-Control","no-cache");
    myServer.send(301);

    Serial.write(START_CHAR);Serial.print( "WAH" + (String)((myServer.arg("alarmHour")).toInt()) );Serial.write(END_CHAR);;
    Serial.write(START_CHAR);Serial.print( "WAM" + (String)((myServer.arg("alarmMin" )).toInt()) );Serial.write(END_CHAR);;
  }

  if (myServer.hasArg("alarmDay")){
    myServer.sendHeader("Location","/");
    myServer.sendHeader("Cache-Control","no-cache");
    myServer.send(301);

    Serial.write(START_CHAR);Serial.print( "WAD" + (String)((myServer.arg("alarmDay")).toInt()) );Serial.write(END_CHAR);
  }

  if (myServer.hasArg("nightTimeHourStart") && myServer.hasArg("nightTimeMinStart")){
    myServer.sendHeader("Location","/");
    myServer.sendHeader("Cache-Control","no-cache");
    myServer.send(301);

    Serial.write(START_CHAR);Serial.print( "WN1" + (String)((myServer.arg("nightTimeHourStart")).toInt()) );Serial.write(END_CHAR);
    Serial.write(START_CHAR);Serial.print( "WN2" + (String)((myServer.arg("nightTimeMinStart" )).toInt()) );Serial.write(END_CHAR);
  }

  if (myServer.hasArg("nightTimeHourEnd") && myServer.hasArg("nightTimeMinEnd")){
    myServer.sendHeader("Location","/");
    myServer.sendHeader("Cache-Control","no-cache");
    myServer.send(301);

    Serial.write(START_CHAR);Serial.print( "WN3" + (String)((myServer.arg("nightTimeHourEnd")).toInt()) );Serial.write(END_CHAR);
    Serial.write(START_CHAR);Serial.print( "WN4" + (String)((myServer.arg("nightTimeMinEnd" )).toInt()) );Serial.write(END_CHAR);
  }
  
  //ENVIANDO O HTML COM INFORMAÇÕES DAS VARIAVEIS
  handleWebsite();
  
}


//no need authentification
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

void handleWebsite(){
  buildWebsite();
  myServer.send(200,"text/html",webSite);
}

void handleXML(){
  buildXML();
  myServer.send(200,"text/xml",XML);
}

void startESP(void){
  delay(20000);
  
  Serial.begin(115200);  
  Serial.println("");
  
  //--------------------------------------------
  //CONNECTING TO AN EXISTING WIFI
  //--------------------------------------------
  WiFi.softAPdisconnect();
  WiFi.disconnect();
  WiFi.mode(WIFI_AP_STA);
  delay(1000);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  //WAITING FOR A SUCCESSFULL CONNECTION
  i = 0;
  if(DEBUG_PRINTING){ Serial.println("[DEBUG] Connecting to WiFi"); }
  while ( (WiFi.status() != WL_CONNECTED) & (i < MAX_CONNECTION_ATTEMPTS) )  {
    delay(1000);
    if(DEBUG_PRINTING){ Serial.print("."); }
    i++;
  }
  if( i >= MAX_CONNECTION_ATTEMPTS ){ 
    if(DEBUG_PRINTING){ Serial.println("");Serial.println("[DEBUG] Problem connecting to Existing WiFi."); }
  }
  else{
    if(DEBUG_PRINTING){ Serial.print(  "[DEBUG] Connected to ");Serial.print(WIFI_SSID);Serial.print(" with IP Address: ");Serial.println( WiFi.localIP() ); }
  }
  i = 0;
  //--------------------------------------------
  

  //--------------------------------------------
  //CREATING AN ACCESS POINT
  //--------------------------------------------  
  myAPConnected = WiFi.softAP( ESP_SSID, ESP_PSWD );
  delay(1000);
  if(DEBUG_PRINTING){ Serial.print(  "[DEBUG] Connected to ");Serial.print(ESP_SSID);Serial.print(" with IP Address: ");Serial.println( WiFi.softAPIP() ); }
  //--------------------------------------------

  myServer.onNotFound(handleNotFound);
  myServer.on("/xml"              , handleXML);
  myServer.on("/login"            , handleLogin);
  myServer.on("/"                 , handleRoot);  
  myServer.on("/setTime"          , handleRoot);
  myServer.on("/setDate"          , handleRoot);
  myServer.on("/setWeekday"       , handleRoot);
  myServer.on("/setAlarmTime"     , handleRoot);
  myServer.on("/setAlarmDay"      , handleRoot);
  myServer.on("/setNightTimeStart", handleRoot);
  myServer.on("/setNightTimeEnd"  , handleRoot);
  
  
  //here the list of headers to be recorded
  const char * headerkeys[] = {"User-Agent","Cookie"} ;
  size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
  //ask server to track these headers
  myServer.collectHeaders(headerkeys, headerkeyssize );
  myServer.begin();
  if(DEBUG_PRINTING){ Serial.println("[DEBUG] HTTP server started"); }

  delay(10000);
  
  if(WiFi.status() == WL_CONNECTED){
    //SUCCESSFULLY CONNECTED TO EXISTING WIFI WITHOUT SELF-REBOOTING
    //RETURNING LOCAL IP ADDRESS
    Serial.write( START_CHAR );Serial.print("WEBOK");Serial.write( END_CHAR );
    Serial.write( START_CHAR );Serial.print("IP0");Serial.print( WiFi.localIP() );Serial.write( END_CHAR );
  }else{
    //PROBLEM OCCURED DURING CONNECTION TO EXISTING WIFI
    Serial.write( START_CHAR );Serial.print("IP00");Serial.write( END_CHAR );
  }

  if( myAPConnected ){
    //RETURNING AP IP ADDRESS
    Serial.write( START_CHAR );Serial.print("IP1");Serial.print( WiFi.softAPIP() );Serial.write( END_CHAR );
  }else{
    //PROBLEM OCCURED DURING CREATION OF AN ACCESS POINT
    Serial.write( START_CHAR );Serial.print("IP10");Serial.write( END_CHAR );
  }
  
}

void handleServerAndMaster(void){
  myServer.handleClient();

  if(Serial.available() > 0){
    stillBuffering = false;
    //------------------------------------------------------------
    //     READING UART VALUES AS VALID ASCII CHARACTERS ONLY
    //------------------------------------------------------------
    Serial.setTimeout(10);bufLen = Serial.readBytes(buf1 , 200);buf1[bufLen] = '\0';
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
      if( buffer_read.indexOf( END_CHAR ) >= 4 ) {
        stillBuffering = false;

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

        //-----------------------------------------------------------------------------------------------
        //                                          CMDS THAT NEED ANSWERS
        //-----------------------------------------------------------------------------------------------
        //REQUEST IP ADDRESS
        else if( ( buffer_read.substring(1,4) == "RIP" ) && ( buffer_read.indexOf( END_CHAR ) == 5 ) ){
          
          //REQUEST WIFI IP ADDRESS
          if(      buffer_read.substring(4,5) == "0" ){
            if(WiFi.status() == WL_CONNECTED){
              //RETURNING AP IP ADDRESS
              Serial.write( START_CHAR );Serial.print("IP0");Serial.print( WiFi.localIP() );Serial.write( END_CHAR );
            }else{
              //PROBLEM OCCURED DURING CREATION OF AN ACCESS POINT
              Serial.write( START_CHAR );Serial.print("IP00");Serial.write( END_CHAR );
            }
          }
          
          //REQUEST AP   IP ADDRESS
          else if( buffer_read.substring(4,5) == "1" ){
            if( myAPConnected ){
              //RETURNING AP IP ADDRESS
              Serial.write( START_CHAR );Serial.print("IP1");Serial.print( WiFi.softAPIP() );Serial.write( END_CHAR );
            }else{
              //PROBLEM OCCURED DURING CREATION OF AN ACCESS POINT
              Serial.write( START_CHAR );Serial.print("IP10");Serial.write( END_CHAR );
            }
          }
          
        }

        //REQUEST WIFI CONNECTION STATUS
        else if( ( buffer_read.substring(1,4) == "RWF" ) && ( buffer_read.indexOf( END_CHAR ) == 5 ) ){ 
          if(   WiFi.status() == WL_CONNECTED){ Serial.write( START_CHAR );Serial.print("IWF1");Serial.write( END_CHAR );
          }else{                                Serial.write( START_CHAR );Serial.print("IWF0");Serial.write( END_CHAR );}
        }

        //REQUEST AP CONNECTION STATUS
        else if( ( buffer_read.substring(1,4) == "RAP" ) && ( buffer_read.indexOf( END_CHAR ) == 5 ) ){ 
          if(   myAPConnected  ){ Serial.write( START_CHAR );Serial.print("IAP1");Serial.write( END_CHAR );
          }else{                  Serial.write( START_CHAR );Serial.print("IAP0");Serial.write( END_CHAR );}
        }

        //REQUEST AUTHENTIFIED USER STATUS
        else if( ( buffer_read.substring(1,4) == "RCL" ) && ( buffer_read.indexOf( END_CHAR ) == 4 ) ){ 
          
          //REPORT TO MASTER UNIT THAT THERE IS NOT SOMEONE AUTHENTIFIED
          if(!is_authentified()){ Serial.write( START_CHAR );Serial.print("ICL0");Serial.write( END_CHAR ); }
          
          //REPORT TO MASTER UNIT THAT THERE IS SOMEONE AUTHENTIFIED
          else{ Serial.write( START_CHAR );Serial.print("ICL1");Serial.write( END_CHAR ); }
        }
        
      }
      //DELETING THE PART OF THE MSG THAT HAS ALREADY BEEN READ
      if( buffer_read.indexOf( END_CHAR ) > 0 ){ buffer_read = buffer_read.substring( buffer_read.indexOf( END_CHAR ) + 1 );msgLen = buffer_read.length(); }
    }
  }
}
