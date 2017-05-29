/*
 serverFunc.h - HTML and XML Handler with UART Handler
 Created by:
              Wellington Rodrigo Gallo
               w.r.gallo@grad.ufsc.br
                    May, 2017
*/
#ifndef serverFunc_h
#define serverFunc_h

//Arduino
//  Purpose:      Needed in all headers of Arduino Sketch
//  License:      GNU Lesser General Public License
//                Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#include <Arduino.h>

//ESP8266WiFi
//  Purpose:      ESSENTIAL FOR ESP8266
//  Version Used: Last Modified December 2014
//  License:      GNU Lesser General Public License
//                Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#include <ESP8266WiFi.h>

//ESP8266WebServer
//  Purpose:      FOR HTML SERVER HANDLER FUNCTIONS
//  Version Used: Last Modified 8 May 2015
//  License:      GNU Lesser General Public License
//                Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#include <ESP8266WebServer.h>

//---------------------------------------------------------------------------------------
//                            WEBSITE.H VARIABLES AND FUNCTIONS
//---------------------------------------------------------------------------------------
extern String webSite,javaScript,XML;

//HOME AUTOMATION NETWORK INFORMATION - DEFAULT VALUES
extern String var01_lampState;
extern String var02_motionSensorState;
extern String var03_buzzerState;
extern String var04_time;
extern String var05_date;
extern String var06_weekday;
extern String var07_alarmTime;
extern String var08_alarmDay;
extern String var09_alarmSet;
extern String var10_nightTimeStart;
extern String var11_nightTimeEnd;
extern String var12_nightTimeSet;

extern void buildWebsite();
extern void buildJavascript();
extern void buildXML();
//---------------------------------------------------------------------------------------

bool is_authentified();
void handleLogin();
void handleRoot();
void handleNotFound();
void handleXML();
void handleWebsite();
void startESP(void);
void handleServerAndMaster(void);

#endif 
