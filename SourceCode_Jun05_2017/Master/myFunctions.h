#ifndef _myFUNCTIONS_H_
#define _myFUNCTIONS_H_

//---------------------------------------------------------------
//                     I2C, LCD LIBRARIES
//--------------------------------------------------------------- 
/*Arduino
  Purpose:      Needed in all Header Files
  License:      GNU Lesser General Public License
                Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "Arduino.h"

/*LiquidCrystal_I2C
  Purpose:      Needed in all Header Files
  Author:       Created by Francisco Malpartida on 20/08/11.
  License:      Copyright 2011 - Under creative commons license 3.0:
                Attribution-ShareAlike CC BY-SA
*/
#include <LiquidCrystal_I2C.h>

/*SoftwareSerial
  Purpose:      Needed to use more than one UART communication
  License:      GNU Lesser General Public License
                Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include <SoftwareSerial.h>

/*Wire
  Purpose:      Needed for I2C Communication
  License:      GNU Lesser General Public License
                Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "Wire.h"
//--------------------------------------------------------------- 






//--------------------------------------------------------------- 
//                      NRF24L01+ LIBRARY
//--------------------------------------------------------------- 
/*SPI
  Purpose:      Needed for SPI communication with nRF24L01+
  License:      GNU Lesser General Public License version 2 and version 2.1
                Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include <SPI.h>

/*nRFL01
  Purpose:      Needed for SPI communication with nRF24L01+
  Source:       https://github.com/nRF24/RF24
  Author:       Copyright (c) 2007 Stefan Engelke <mbox@stefanengelke.de>
                Portions Copyright (C) 2011 Greg Copeland
                
  License:      Permission is hereby granted, free of charge, to any person
                obtaining a copy of this software and associated documentation
                files (the "Software"), to deal in the Software without
                restriction, including without limitation the rights to use, copy,
                modify, merge, publish, distribute, sublicense, and/or sell copies
                of the Software, and to permit persons to whom the Software is
                furnished to do so, subject to the following conditions:
                
                The above copyright notice and this permission notice shall be
                included in all copies or substantial portions of the Software.
*/
#include "nRF24L01.h"

/*RF24
  Purpose:      Needed for SPI communication with nRF24L01+
  Source:       https://github.com/nRF24/RF24
  License:      GNU Lesser General Public License version 2
                Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "RF24.h"
//--------------------------------------------------------------- 






//---------------------------------------------------------------
//                           FUNCTIONS
//---------------------------------------------------------------

//---------------------------------------
// FUNCTIONS: MASTER UNIT CONFIGURATION
//---------------------------------------
/**
 * Setup GPIO, UART, SPI, I2C, Start Display...
 */
void configThisUnit();

/**
 * 1 Second Timer Handler
 * This function is called when DS1307 reports
 * a new second value.
 * For internal timers such as alarmTimer;
 * serverHandler() is also called in this function.
 */
void oneSecondTimerHandler();

/**
 * Check if it is at Night Mode Right Now
 */
bool atNightMode();

/**
 * Update Display Information
 */
void updateLCD();

//---------------------------------------


//---------------------------------------
// FUNCTIONS: WEB SERVER COMMUNICATION
//---------------------------------------
/**
 * Send updated information to ESP8266
 */
void refreshServer();

/**
 * Check if there is an incoming message from ESP8266
 */
void serverHandler();
//---------------------------------------

//---------------------------------------
// FUNCTIONS: RF24 COMMUNICATION
//---------------------------------------
/**
 * Check for incoming messages from nRF24L01+
 */
void handleRF24();

/**
 * Handle valid incoming messages from nRF24L01+
 */
void handleMessage();

/**
 * Send a command to the slave 2
 */
void report2Slave2(int cmd);

/**
 * Send the request updated information to the slaves
 */
void requestUpdateFromSlaves();
//---------------------------------------

//---------------------------------------
// FUNCTIONS: REAL TIME MODULE
//---------------------------------------
/**
 * Convert Decimal to BCD
 * Used by DS1307
 */
byte    dec2bcd(byte    number);

/**
 * Convert BCD to Decimal
 * Used by DS1307
 */
uint8_t bcd2dec(uint8_t number);

/**
 * Update DS1307 time
 * timeWeekday = 0, means Sunday
 * timeWeekday = 6, means Saturday
 */
void setTimeDS1307(byte timeYear /*00 to 99*/, byte timeMonth /*01 to 12*/, byte timeDay /*01 to 31*/, byte timeWeekday /*0 to 6*/, byte timeHour /*00 to 23*/, byte timeMinute /*00 to 59*/);

/**
 * Get DS1307 time
 */
void getTimeDS1307();
//---------------------------------------

//---------------------------------------------------------------
#endif // _myFUNCTIONS_H_
