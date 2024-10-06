/*
 sample / test code for LCD27B4DH01
 */
#include <SPI.h>
// configuration
// D10 SS
// D11 MOSI -> 2:SI
// D12 MISO
// D13 SCK  -> 1:SCLK
#define _SCS          9 // D9 SCS -> 3:SCS
#define _EXTCOM 8// D8 EXTCOM -> 4:EXTCOM
#define _DISP   7 // D7 DISP   -> 5:DISP
#define _MODE   6 // D6 MODE   -> 8:MODE
// 5V        -> 6:VDDA, 7:VDD
// GND       -> 9:VSS, 10:VSSA

bool intTimer=false;
//SPISettings mySPISettings = SPISettings(8000000, MSBFIRST, SPI_MODE3);
SPISettings mySPISettings = SPISettings(1000000, MSBFIRST, SPI_MODE0);
  //  最上位ビットから転送する場合は、MSBFIRST
  //  最下位ビットから転送する場合は、LSBFIRST
  //  SPI_MODE0: Clock NegEdge, DataChange at PosEdge of Clock
  //  SPI_MODE1: Clock NegEdge, DataChange at NegEdge of Clock
  //  SPI_MODE2: Clock PosEdge, DataChange at PosEdge of Clock
  //  SPI_MODE3: Clock PosEdge, DataChange at NegEdge of Clock
  // SPI clock 1/2:8MHz, 1/4:4MHz, 1/8:2MHz, 1/16:1MHz, 1/32:500KHz, 1/64:250KHz, 1/128:125KHz, 

void setup() {
  Serial.begin(9600);

   // SPI setting
  SPI.begin();
  SPI.beginTransaction(mySPISettings);
    //  最上位ビットから転送する場合は、MSBFIRST
    //  最下位ビットから転送する場合は、LSBFIRST
    //  SPI_MODE0: Clock NegEdge, DataChange at PosEdge of Clock
    //  SPI_MODE1: Clock NegEdge, DataChange at NegEdge of Clock
    //  SPI_MODE2: Clock PosEdge, DataChange at PosEdge of Clock
    //  SPI_MODE3: Clock PosEdge, DataChange at NegEdge of Clock
  digitalWrite(SS, HIGH);
  //digitalWrite(SS, HIGH); //normal固定
  
  // LCD setting
  pinMode(_SCS, OUTPUT);
  pinMode(_EXTCOM, OUTPUT);
  pinMode(_DISP, OUTPUT);
  pinMode(_MODE, OUTPUT);

  digitalWrite(_SCS, LOW);
  digitalWrite(_MODE, HIGH); // モード選択信号をEXTCOMで設定する
  digitalWrite(_EXTCOM, LOW);  // 
  digitalWrite(_DISP, LOW); // 表示OFF

  /* EXTCOM timer interupt */
  //CTC_timerSetup();
  Normal_timerSetup();

}
unsigned int intCount=256; //65536-256 = 1sec 
void Normal_timerSetup(){
  // 500msec - 2sec で割り込みを上げる
  /* Timer setup */
  TCCR1A = 0; // 初期化
  TCCR1B = 0; // 初期化
  // TCNT1 = 65536-intCount; // intCount*256/16M
  TCNT1 = intCount; // (65536-intCount)*256/16M
  TCCR1B |= (1 << CS12);
       // CS12 -> 1 (prescaler -> 256)
       // Normal mode on 
       // 256/16M=16usec単位での設定
  TIMSK1 |= (1 << TOIE1);
       // counter reset at TCNT1 < 65535 
}
ISR(TIMER1_OVF_vect) { 
   intTimer = true;
   TCNT1 = intCount;
}

void CTC_timerSetup(){
  /* Timer setup */
  TCCR1A = 0; // 初期化
  TCCR1B = 0; // 初期化
  //OCR1A = 31250; // 31250*1024/16M=2sec (maxはunsigned int(65535))
  //OCR1B = 7812; // 7812*1024/8M=1sec (maxはunsigned int(65535))
  OCR1A = 640; // 31250*1024/16M=2sec (maxはunsigned int(65535))
  OCR1B = 320; // 7812*1024/8M=1sec (maxはunsigned int(65535))
  //TCCR1B |= (1 << CS12) | (1 << CS10) | (1 << WGM12);
  TCCR1B |= (1 << CS12) | (1 << WGM12);
       // CS12 -> 1, CS10 -> 0 (prescaler -> 256)
       // CTC mode on // 1024/16M=64usec単位での設定
  TIMSK1 |= (1 << OCIE1A) | (0 << OCIE1B);
       // OCIEA -> 1 (enable OCR1A Interrupt)
       // OCIEB -> 0 (disable OCR1B Interript)
       // counter reset at TOP(OCRA1)
}
/* CTC timer interrupt */
ISR(TIMER1_COMPA_vect) {
   intTimer = true;
}

#define _CLEAR            0x20 // 0b0020_0000
#define _MODE_SET  0x80 // 0b1000_0000
#define _DUMMY         0x00 

char buf[100];
void intProcess(){
       if(false){
       sprintf(buf, "extcom %ld", millis());
       Serial.println(buf);
     }
      bool extcom=digitalRead(_EXTCOM);
     digitalWrite(_EXTCOM, !extcom);
     if ( false){ // debug
        extcom=digitalRead(_EXTCOM);
        Serial.println(extcom);
     }
}
void clearDisplay(){
    digitalWrite(SS,LOW);
    digitalWrite(_SCS, HIGH);
     digitalWrite(SS,LOW); // dummy
    SPI.transfer(_CLEAR);
    SPI.transfer(_DUMMY);
    digitalWrite(_SCS, LOW);
    digitalWrite(SS,HIGH);
}
void dataTransfer(){
    //sprintf(buf,"Start transfer %ld", millis());
   //Serial.println(buf);
   for (int lineNo=0; lineNo < 240; lineNo++){  
    //int lineNo=0x0f;[
        digitalWrite(SS,LOW);
       digitalWrite(_SCS,HIGH);
      digitalWrite(_SCS,HIGH); // dummy
     SPI.transfer(_MODE_SET);
     SPI.transfer(lineNo);
     for ( int x=0; x< 50; x++){
        //SPI.transfer(0x0f);
         //SPI.transfer(0xcc);
         SPI.transfer(0xcc);
         //if ( (x%2) == 0 )   SPI.transfer(0x0f);
         //else SPI.transfer(0xf0);
     }
     SPI.transfer(_DUMMY);
      SPI.transfer(_DUMMY);
       digitalWrite(SS,LOW);
       digitalWrite(SS,HIGH); // dummy
        digitalWrite(_SCS,LOW);

     //delay(10);
  }
}
int frameCount=0;

bool transferFlag=true;
void loop() { 
   if (intTimer){
      intTimer=false;
      intProcess();
   }
   //sprintf(buf,"%d frame",frameCount++);
   // Serial.println(buf);  
   if ( transferFlag){
      //transferFlag = false;
      digitalWrite(_DISP, LOW); // 表示OFF
      clearDisplay();
       digitalWrite(_DISP, HIGH); // 表示ON
      delay(10);
      //digitalWrite(_DISP, HIGH); // 表示ON
     dataTransfer();
     //digitalWrite(_DISP, HIGH); // 表示ON
     delay(1);
   }
}
