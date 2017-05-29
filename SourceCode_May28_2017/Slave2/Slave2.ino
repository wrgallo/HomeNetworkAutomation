/*

  Slave 2
  RELAY (LAMP)

  -----------------------------------------------
 | PIN |  PURPOSE                                |
 |-----|-----------------------------------------|
 | 6   |  RELAY OUTPUT                           |
 | 9   |  CE   of NRF24L01+                      |
 | 10  |  CSN  of NRF24L01+                      |
 | 11  |  MOSI of NRF24L01+                      |
 | 12  |  MISO of NRF24L01+                      |
 | 13  |  SCK  of NRF24L01+                      |
  -----------------------------------------------

  ---------------------------------------------
 | Power Supply  | Peripheral                  |
 |---------------|-----------------------------|
 | +5V           |  RELAY                      |
 | +3v3          |  NRF24L01+                  |
  ---------------------------------------------

*/

#include "myFunc.h"

void setup() {
  
  configThisSlave();
  
}

void loop() {
  
  handleRF24();
  handleRelay();
  delay( 1000 );
  
}
