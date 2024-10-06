/*
 * sample code spi-slave
 *  https://tomosoft.jp/design/?p=42344
 */
#include <SPI.h>
 
  byte rxdata;
 
void setup() {
  Serial.begin(9600);
  Serial.println("/-----START Arduino uno-----/");
  Serial.println("/----------SPI Slave---------/");
 
  SPCR |= bit(SPE);
  pinMode(MISO, OUTPUT);
  SPI.attachInterrupt();
}
 
bool rxFlag=false;
void loop() {
  if (rxFlag) {
     rxFlag=false;
     Serial.println(rxdata);
  }   
}
 
ISR(SPI_STC_vect) {
  rxFlag=true;
  rxdata = SPDR;
  SPDR = rxdata+1;
}
