/*
  controller.ino
  Vehicle controller
  Serial & BT(HC06) control
 */
#define SERIAL_RATE 9600 
#define _RPin 2
#define _YPin 3
#define _GPin 6
#define _WPin 4
#define _RPush 0b0001
#define _YPush 0b0010
#define _GPush 0b0100
#define _WPush 0b1000
#define _RLed 5

#include <SoftwareSerial.h>
#define _BTRxPin A0
#define _BTTxPin A1
#define _BTEnPin 10 // dummy
SoftwareSerial btSerial(_BTRxPin, _BTTxPin);
#define _SBAUD_COM 9600 // COMMUNICATION
#define _SBAUD_AT 38400 // for AT command
bool btComMode=false;

int demoState=0;

// BT(HC05) functions //
void checkAT(){
  char buf[100];
  bool atMode = digitalRead(_BTEnPin);
  if ( atMode ) {
    btSerial.begin(_SBAUD_AT);
    sprintf(buf,"AT : %ld", (long)_SBAUD_AT);
    Serial.println(buf);
  } else {
    btSerial.begin(_SBAUD_COM);
    sprintf(buf,"com : %ld", (long)_SBAUD_COM);
    Serial.println(buf);
  }
}
void HC05_com (long baud){
  char buf[100];
  btSerial.end();
  pinMode(_BTEnPin,OUTPUT);
  digitalWrite(_BTEnPin,LOW); // communication mode
  btSerial.println("AT+RESET");
  btSerial.begin(baud);//115200);
  sprintf(buf,"com : %ld", baud);
  Serial.println(buf);
  pinMode(_BTEnPin,INPUT);
}
void HC05_AT (long baud){
  char buf[100];
  btSerial.end();
  pinMode(_BTEnPin,OUTPUT);
  digitalWrite(_BTEnPin,HIGH); // AT mode
  btSerial.begin(baud);
  sprintf(buf,"AT : %ld", baud);
  Serial.println(buf);
}

void btComTest(){
  char c;
  if (Serial.available()){
    c=Serial.read();
    if (c=='#') {
      for ( int i = 0; i < 10; i++){
         btSerial.println("1234567890");
      }
    } else if (c == '!') {
        Serial.println("->com");
        HC05_com(_SBAUD_COM);
    } else if (c == '$') {
        Serial.println("->AT");
        HC05_AT(_SBAUD_COM);
    } else if (c == '%') {
        Serial.println("->AT");
        HC05_AT(_SBAUD_AT);
    } else if (c == '-') {
        Serial.println("->exit");
        btComMode = false;
    } else {
        btSerial.write(c);
    }
  } else if ( btSerial.available()){
    c=btSerial.read();
    if (c=='#') {
      for ( int i = 0; i < 10; i++){
         Serial.println("1234567890");
      }
    } else {
       Serial.write(c);
    }
  }
}

void setup() {
  Serial.begin(SERIAL_RATE);

  // pin setting
  pinMode(_RPin, INPUT_PULLUP);
  pinMode(_YPin, INPUT_PULLUP);
  pinMode(_GPin, INPUT_PULLUP);
  pinMode(_WPin, INPUT_PULLUP);
  pinMode(_RLed, OUTPUT);
  digitalWrite(_RLed, HIGH);

  // Timer setup
  TCCR1A = 0; // 初期化
  TCCR1B = 0; // 初期化
  TCNT1 = 3036; 
  TCCR1B |= (1 << CS12);
       // ( 65536 * 256 ) / 8000000 = 2.09 sec
  TIMSK1 |= (1 << TOIE1); //TOIE -> 1
  TCNT1 = 0; // clear

  // BT setup
  checkAT();
}

bool intTimer1=false;
ISR(TIMER1_OVF_vect) {
  intTimer1=true;
  TCNT1 = 0;
}

byte commandPtr=0;
char commandStr[50];
int  commandLen=50;
unsigned long commandTimeOut;

bool serialCommandLine() {
   /*
    * Serial.available() > 0 でcommand にstore開始
    * \nまでの入力結果がcommandにstoreされるを返す
    * 2sec間入力が無ければcommand入力終了
    */

   char c;
   unsigned long previous, now;

   bool commandDet = false; 
   if (Serial.available() > 0){
      do {
        c=Serial.read();
        commandTimeOut = millis() + 10000; // set timeout value
        commandStr[commandPtr++] = c;

        if ( (commandPtr >= commandLen) || ( c=='\n' || c=='\r' ) ){
           commandDet = true; 
        }
      } while (Serial.available()>0);

   } else if ( commandPtr > 0 ) {
      if ( millis() > commandTimeOut ){
         commandStr[commandPtr++] = '\n';
         commandDet = true; 
      }
   }

   if (commandDet) {
      if ( commandStr[0] == '\n' || commandStr[0]== '\r') { // ignore \n\r
         commandDet=false;
         commandPtr=0;
      } else {
         commandStr[commandPtr]=0; // end 
         commandPtr = 0; // clear
         Serial.print("sCommand:");
         Serial.println(commandStr);
      }
   }
   return (commandDet);
}
/* timerがロールオーバーしたときの処理サンプル
           if (  4294967295 - previous + now > TIMEOUT ) {
*/

bool btCommandLine() {
   /*
    * Serial.available() > 0 でcommand にstore開始
    * \nまでの入力結果がcommandにstoreされるを返す
    * 2sec間入力が無ければcommand入力終了
    */
   bool commandDet=false;
   char c;
   unsigned long previous, now;

   if (btSerial.available() > 0){
      do {
        c=btSerial.read();

        commandTimeOut = millis() + 10000; // set timeout value
        commandStr[commandPtr++] = c;
        // echo back for debug
        if (false) { // debug
           btSerial.print(c);
        }

        if ( (commandPtr >= commandLen) || ( c=='\n' || c=='\r' ) ){
           commandDet = true; 
        }
      } while (btSerial.available()>0);

   } else if ( commandPtr > 0 ) {
      // timeout process
      if ( millis() > commandTimeOut ){
         commandStr[commandPtr++] = '\n';
         commandDet = true; 
      }
   }

   if (commandDet) {
      commandStr[commandPtr]=0; // end 
      commandPtr = 0; // clear
      Serial.print("btCommand:");
      Serial.println(commandStr);
   }
  return (commandDet) ;
}

long command2dec(char str[], int st_ptr) {
   long val = 0;
   int ptr = st_ptr;
   bool minus = false;
   if (str[ptr]== '-' ){
      minus = true;
      ptr++;
   }
   while ( '0' <= str[ptr]
           && str[ptr] <= '9'){
      val = val * 10 + (str[ptr] - '0');
      ptr++;
   }
   if (minus) val = (-1)*val;
   return val;
}
float command2float(char str[], int st_ptr) {
   char cTemp[100];
   int tempPtr=0;
   int ptr = st_ptr;
   int pol = 1;
   if (str[ptr]== '-' ){
      pol = -1;
      ptr++;
   }
   while ( ( '0' <= str[ptr]
             && str[ptr] <= '9')
           || str[ptr] == '.' ) {
      cTemp[tempPtr++] = str[ptr++];
   }
   cTemp[tempPtr] = 0;
   return (atof(cTemp) * (float)pol);
}

long command2hex(char str[], int st_ptr) {
   long val = 0;
   int ptr = st_ptr;
   int base;
   while ( ( '0' <= str[ptr]
             && str[ptr] <= '9')
           || ( 'a' <= str[ptr]
                && str[ptr] <= 'f' )
           || ( 'A' <= str[ptr]
                && str[ptr] <= 'F' )){

      if( '0' <= str[ptr] && str[ptr] <= '9') {
         base = '0';
      } else if ('a' <= str[ptr] && str[ptr] <= 'f') {
         base = 'a' - 10;
      } else {
         base = 'A' - 10;
      }
      val = val * 16 + (long)(str[ptr] - base);
      ptr++;
   }
   //if (false) Serial.println(val,HEX);
   return val;
}

void driveFree(){

}

bool demoMode = false;
void commandProcess(char command[]){
     if ( command[0] == '?' || command[0] == 'H' || command[0] == 'h' ) {
        Serial.println("Help");

     } else if ( command[0] == 'M' || command[0] == 'm' ) {
        motorDrive(command, true);

     } else if ( command[0] == 'D' || command[0] == 'd' ) {
        demoState = 0;
        if ( command[1] == '1' ) {
           Serial.println("demo start");
           demoMode = true;
        } else {
           Serial.println("demo end");
           demoMode=false;
           driveFree();
        }
     } else if ( command[0] == 'B' || command[0] == 'b' ) {
        if ( command[1] == '1' ) {
           btComMode=true;
           Serial.println("BT bridge mode start");
        } else if ( command[1] == '0' ) { // fail safe
           btComMode=false;
           Serial.println("BT bridge mode end");
        } else {
           //NOP
        }
     }
}

char prevCommand[3]="M11"; // stop
void motorDrive(char command[], bool heartBeat){
  if ( !heartBeat ) {
     if ( strcmp(prevCommand, command) != 0 ){
        strcpy(prevCommand, command);
        btSerial.println(command);
        TCNT1 = 0; // HB timer clear
        if (true) { // debug
          Serial.println(command);
        }
     }
  } else { // heartBeat
     btSerial.println(command);
     if (true) { // debug
        Serial.println(command);
     }
  }
}

void demo (int state){
   switch (state) {
      // forward
      case 0:
        motorDrive("M22", false);
        break;
      case 1:
        motorDrive("M12", false);
        break;
      case 2:
        motorDrive("M21", false);
        break;
      case 3:
        motorDrive("M22", false);
        break;

      // back
      case 4:
        motorDrive("M00", false);
        break;

      case 5:
        motorDrive("M01", false);
        break;

      case 6:
        motorDrive("M10", false);
        break;

      case 7:
        motorDrive("M00", false);
        break;

      // rotate
      case 8:
        motorDrive("M11", false);
        break;

      case 9:
        motorDrive("M20", false);
        break;

      case 10:
        motorDrive("M02", false);
        break;

      case 11:
        motorDrive("M11", false);
        break;

      Defaults:
        motorDrive("M11", false);
   }
}

byte readButton(){
  byte rc = 0;
  if ( !digitalRead(_RPin) ) rc |= _RPush;
  if ( !digitalRead(_GPin) ) rc |= _GPush;
  if ( !digitalRead(_YPin) ) rc |= _YPush;
  if ( !digitalRead(_WPin) ) rc |= _WPush;
  return rc;
}
byte buttonCheck(){
  if (readButton() == 0 ) {
     digitalWrite(_RLed, HIGH);
     return 0;
  }
  digitalWrite(_RLed, LOW);
  delay(100);
  int rc = readButton();
  if ( true ) { // debug
     if ( rc & _RPush ) {
        Serial.print("R ");
     }
     if ( rc & _GPush ) {
        Serial.print("G ");
     }
     if ( rc & _YPush ) {
        Serial.print("Y ");
     }
     if ( rc & _WPush ) {
        Serial.print("W ");
     }
     Serial.println();
  }
  return rc;
}

void  buttonProcess(byte flag){
  char command[2];
  switch (flag) {
    case _RPush | _GPush :
       motorDrive("M22", false); // forward
       break;
    case _RPush :
       motorDrive("M12", false); // forward turn left
       break;
    case _GPush :
       motorDrive("M21", false); // forward turn right
       break;
    case _RPush | _WPush :
       motorDrive("M02", false); // rotate left
       break;
    case _GPush | _YPush:
       motorDrive("M20", false);  // rotate right
       break;
    case _WPush | _YPush:
       motorDrive("M00", false); // backword
       break;
    case _WPush:
       motorDrive("M01", false); // backword turn right
       break;
    case _YPush:
       motorDrive("M10", false); // backword turn left
       break;
    default:
       motorDrive("M11", false);  // stop
       break;
  }
}

byte buttonFlag =0;
void loop() {
  buttonFlag |= buttonCheck();

  if ( btComMode ){  // for debug & BT control
     btSerial.listen();
     btComTest();
  } else {
     if ( serialCommandLine() ) {
        // command from serial
        commandProcess(commandStr);
     
     } else if ( btCommandLine() ) {
        commandProcess(commandStr);
     }

     if ( demoMode )  {
        Serial.println(demoState);
        demo(demoState++);
        if ( 11 < demoState ) demoState = 0;
        delay(5000);
     } else {
       if ( intTimer1 ) {
           intTimer1=false;
           TCNT1=0; // clear
           motorDrive(prevCommand, true);  // HeartBeat
       } else {
           buttonProcess(buttonFlag);
           buttonFlag = 0;
       }
     }
  }
}
