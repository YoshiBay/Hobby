#define IN1 9　　　　　//9番ピンを出力ピンIN1と定義してます。
#define IN2 8　　　　　//8番ピンを出力ピンIN1と定義してます。

void setup() {
  pinMode(IN1,OUTPUT);
  pinMode(IN2,OUTPUT);
}

void digital(){
  digitalWrite(IN1,HIGH);     //どちらかがHIGHでモータが周ります。
  digitalWrite(IN2,LOW);      //正転
  delay(2000);                //delayは適当に入れてください。

  digitalWrite(IN1,HIGH);　　　　　//両方ともHIGHにするとブレーキ
  digitalWrite(IN2,HIGH);     //停止（ブレーキ）
  delay(2000);

  digitalWrite(IN1,LOW);      //上記のものと逆回転になります。
  digitalWrite(IN2,HIGH);     //逆転
  delay(2000);

  digitalWrite(IN1,LOW);      //停止（惰性回転）
  digitalWrite(IN2,LOW);
  delay(2000);
}

void loop() {
  digital();
}
