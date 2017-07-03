/*
  Copyright (C) 2017 Wellington Rodrigo Gallo <w.r.gallo@grad.ufsc.br>
  This program is a free software; 
  You can:
    * redistribute it
    * modify it
  Under the terms of the GNU General Public License
  published by the Free Software Foundation.
 
  Slave 1
  Motion Sensor

  ------------------------------------------------------------------
 | PIN |  PURPOSE                                                   |
 |-----|------------------------------------------------------------|
 | 5   |  LED2 OUTPUT (ON WHEN PRESENCE DETECTED) Without Debouncer |
 | 6   |  LED1 OUTPUT (ON WHEN PRESENCE DETECTED) With Debouncer    |
 | 7   |  PIR  SENSOR IN                                            |
 | 9   |  CE   of NRF24L01+                                         |
 | 10  |  CSN  of NRF24L01+                                         |
 | 11  |  MOSI of NRF24L01+                                         |
 | 12  |  MISO of NRF24L01+                                         |
 | 13  |  SCK  of NRF24L01+                                         |
  ------------------------------------------------------------------

  ---------------------------------------------
 | Power Supply  | Peripheral                  |
 |---------------|-----------------------------|
 | +5V           |  LED                        |
 | +5V           |  PIR SENSOR                 |
 | +3v3          |  NRF24L01+                  |
  ---------------------------------------------

*/
#include "myFunc.h"

void setup()
{
  configThisSlave();
}

void loop()
{
  loopThisSlave();
}






