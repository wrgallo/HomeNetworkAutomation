#ifndef _myFUNCTIONS_H_
#define _myFUNCTIONS_H_

//---------------------------------------------------------------
//                     BIBLIOTECAS PARA O PROJETO
//--------------------------------------------------------------- 
#include "Arduino.h"                                             //GNU Lesser General Public License
                                                                 //Necessária em todo arquivo header para Arduino

#include <LiquidCrystal_I2C.h>                                   // Created by Francisco Malpartida on 20/08/11.
                                                                 // Copyright 2011 - Under creative commons license 3.0:
                                                                 //        Attribution-ShareAlike CC BY-SA

#include <SoftwareSerial.h>

#include "Wire.h"                                                //GNU Lesser General Public License

//-----------------------------------------------
//               NRF24L01+ LIBRARY
//-----------------------------------------------
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
//-----------------------------------------------

//---------------------------------------------------------------
//                           FUNÇÕES
//---------------------------------------------------------------
bool atNightMode();                                              //Verifica se entrou/saiu do Modo Noturno
void refreshServer();											                       //Send updated information to ESP8266
void serverHandler();                                            //Verifica se há uma mensagem válida no Buffer UART (ESP8266)
void updateLCD();                                                //Atualiza informações no Display LCD
void inicializarArduino();                                       //Configurações iniciais
void oneSecondTimerHandler();                                    //1 Second Timer Handler

void handleRF24();
void handleMessage();
void report2Slave2(uint8_t cmd);
void requestUpdateFromSlaves();

byte    dec2bcd(byte    number);
uint8_t bcd2dec(uint8_t number);
void setTimeDS1307(byte ano, byte mes, byte dia, byte dia_semana, byte hora, byte minuto);   //Seta a data e a hora do DS1307
void updateTime();

#endif // _myFUNCTIONS_H_
