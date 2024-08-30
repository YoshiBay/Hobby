/*
  vehicle.ino
  Motor drive 
  Serial & BT(HC05) control
 */
// Define for L298N
#define _IN1 2
#define _IN2 3
#define _IN3 4
#define _IN4 5
#define _ENA 0
#define _ENB 0

#define SERIAL_RATE 9600 

#include <SoftwareSerial.h>
#define _BTRxPin 9
#define _BTTxPin 8
#define _BTEnPin 7
SoftwareSerial btSerial(_BTRxPin, _BTTxPin);
bool btComMode=false;
#define _SBAUD_COM 9600 // COMMUNICATION
#define _SBAUD_AT 38400 // for AT command

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
  // Motor driver setup
  pinMode(_IN1,OUTPUT);
  pinMode(_IN2,OUTPUT);
  pinMode(_IN3,OUTPUT);
  pinMode(_IN4,OUTPUT);
  if ( _ENA != 0 ) pinMode (_ENA,OUTPUT);
  if ( _ENB != 0 ) pinMode (_ENB,OUTPUT);
  driveFree();

  Serial.begin(SERIAL_RATE);

  // BT setup
  checkAT();
}
void driveFree() {
  digitalWrite(_IN1,LOW);
  digitalWrite(_IN2,LOW);
  digitalWrite(_IN3,LOW);
  digitalWrite(_IN4,LOW);  
}

void motorDrive(char command[]){
  char buf[100];
  int left  = (int)(command[1] - '0') - 1;
  int right = (int)(command[2] - '0') - 1;
  sprintf ( buf, "left=%d, right=%d", left, right);
  Serial.println(buf);

  driveFree();
  if ( left < 0 ) reverseLeft();
  else if ( 0 < left ) forwardLeft();
  else brakeLeft();
     
  if ( right < 0 ) reverseRight();
  else if ( 0 < right ) forwardRight();
  else brakeRight();
}
     
void forwardLeft() {
  digitalWrite(_IN1,HIGH);
  digitalWrite(_IN2,LOW);
}
void forwardRight() {
  digitalWrite(_IN3,HIGH);
  digitalWrite(_IN4,LOW);
}
void forward() {
  forwardLeft();
  forwardRight();
}
void reverseLeft() {
  digitalWrite(_IN1,LOW);
  digitalWrite(_IN2,HIGH);
}
void reverseRight() {
  digitalWrite(_IN3,LOW);
  digitalWrite(_IN4,HIGH);
}
void reverse() {
  reverseLeft();
  reverseRight();
}
void brakeLeft() {
  digitalWrite(_IN1,HIGH);
  digitalWrite(_IN2,HIGH);
}
void brakeRight() {
  digitalWrite(_IN3,HIGH);
  digitalWrite(_IN4,HIGH);
}
void brake() {
  brakeLeft();
  brakeRight();
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

bool demoMode = false;
void commandProcess(char command[]){
     if ( command[0] == '?' || command[0] == 'H' || command[0] == 'h' ) {
        Serial.println("Help");

     } else if ( command[0] == 'M' || command[0] == 'm' ) {
        motorDrive(command);

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

void demo (int state){
   switch (state) {
      // forward
      case 0:
        motorDrive("M22");
        break;
      case 1:
        motorDrive("M12");
        break;
      case 2:
        motorDrive("M21");
        break;
      case 3:
        motorDrive("M22");
        break;

      // back
      case 4:
        motorDrive("M00");
        break;

      case 5:
        motorDrive("M01");
        break;

      case 6:
        motorDrive("M10");
        break;

      case 7:
        motorDrive("M00");
        break;

      // rotate
      case 8:
        motorDrive("M11");
        break;

      case 9:
        motorDrive("M20");
        break;

      case 10:
        motorDrive("M02");
        break;

      case 11:
        motorDrive("M11");
        break;

      Defaults:
        motorDrive("M11");
   }
}

void loop() {
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
     }
  }
}
