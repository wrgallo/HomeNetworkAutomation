/*
 Copyright (C) 2017 Wellington Rodrigo Gallo <w.r.gallo@grad.ufsc.br>
  This program is a free software; 
  You can:
    * redistribute it
    * modify it
  Under the terms of the GNU General Public License
  published by the Free Software Foundation.
*/

#include "myFunctions.h"

void setup() { configThisUnit();  }

void loop() {
  
  updateLCD();
  getTimeDS1307();
  handleRF24();
  
}
