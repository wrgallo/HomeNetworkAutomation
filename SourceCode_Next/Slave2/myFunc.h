#ifndef _myFunch_
#define _myFunch_

//-----------------------------------------------
//                   LIBRARIES
//-----------------------------------------------
/*EEPROM
  Purpose:      Save and Read last saved values
  License:      GNU Lesser General Public License version 2 and version 2.1
                Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include <EEPROM.h>

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
//-----------------------------------------------





//-----------------------------------------------
//                   FUNCTIONS
//-----------------------------------------------
/**
 * Setup GPIO, UART and nRF24L01+
 */
void configThisSlave();

/**
 * Handle Relay
 */
void handleRelay();

/**
 * Report this slave status to Master Unit
 * through master unit pipe
 */
void report2Master();

/**
 * Check for valid commands in incoming message
 */
void handleMessage();

/**
 * Check for Incoming Messages on nRF24L01+
 * If there is a message, it is handled by
 * handleMessage();
 */
void handleRF24();
//-----------------------------------------------

#endif
