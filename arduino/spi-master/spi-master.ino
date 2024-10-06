/*
 sample / test code for LCD27B4DH01
 This is spi-master code
   https://stupiddog.jp/note/archives/976
   https://tomosoft.jp/design/?p=42344
 */
#include <SPI.h>
// configuration
// D10 SS
// D11 MOSI
// D12 MISO
// D13 SCK

// 1回だけ実行させるためのフラグ
//boolean isFirst = true;

// 1. SPIの設定を保持するSPISettingsクラスのインスタンスを生成する
//SPISettings mySPISettings = SPISettings(8000000, MSBFIRST, SPI_MODE0);
//  最上位ビットから転送する場合は、MSBFIRST
//  最下位ビットから転送する場合は、LSBFIRST
//  SPI_MODE0: Clock NegEdge, DataChange at PosEdge of Clock
//  SPI_MODE1: Clock NegEdge, DataChange at NegEdge of Clock
//  SPI_MODE2: Clock PosEdge, DataChange at PosEdge of Clock
//  SPI_MODE3: Clock PosEdge, DataChange at NegEdge of Clock
//  Clock Freq
//    SPI2X SPR01 SPR00 freq
//    0     0     0     fosc/4  = 16M/4=4MHz
//    0     0     1     fosc/16 = 1MHz
//    0     1     0     fosc/64 = 1/4MHz
//    0     1     1     fosc/128= 1/8MHz
//    1     0     0     fosc/2  = 8MHz
//    1     0     1     fosc/8  = 2MHz
//    1     1     0     fosc/32 = 1/2MHz
//    1     1     1     fosc/64 = 1/4MHz

void setup() {
  Serial.begin(9600);
  Serial.println("/-----START Arduino nano-----/");
  Serial.println("/---------SPI Master--------/");
 
  SPI.begin();
  digitalWrite(SS, HIGH);
}

int knob = 54;
void loop() {
  // 3. 現在の設定を退避し、指定した設定をマイコンに反映させSPI通信を開始する
  //SPI.beginTransaction(mySPISettings);
  // 4. 制御するデバイスに通信の開始を通知する
  //digitalWrite(SS, LOW);
  // 5. 1バイトを送受信する
  //SPI.transfer(0xaf);
  // 6. 制御するデバイスに通信の終了を通知する
  //digitalWrite(SS, HIGH);
  // 7. SPI通信を終了し設定を以前の状態に戻す
  //SPI.endTransaction();

  byte rxdata;
  digitalWrite(SS, LOW);
  SPI.transfer(knob);
  digitalWrite(SS, HIGH);
  digitalWrite(SS, LOW);
  rxdata = SPI.transfer(0);
  digitalWrite(SS, HIGH);
 
  Serial.print("tx:");
  Serial.print(knob);
  Serial.print(" rx:");
  Serial.println(rxdata);
 
  knob++;
  delay(2000);
}
