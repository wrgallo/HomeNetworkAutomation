#include "myFunctions.h"

void setup() { inicializarArduino(); }

void loop() {
  
  updateLCD();
  updateTime();
  handleRF24();
  
}
