/*
 website.h - HTML and XML File Builder
 Created by:
              Wellington Rodrigo Gallo
               w.r.gallo@grad.ufsc.br
                    May, 2017
*/
#ifndef myWEBSITE_h
#define myWEBSITE_h

//Arduino
//Purpose:        Needed in all headers of Arduino Sketch
//License:        GNU Lesser General Public License
//                Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#include <Arduino.h>

String webSite,javaScript,XML;

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

/**
 * Create the .html Webpage
 */
void buildWebsite();

/**
 * Create the Javascript part of the HTML text
 */
void buildJavascript();

/**
 * Create the .xml text
 */
void buildXML();

#endif
