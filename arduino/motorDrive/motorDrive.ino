#define _IN1 2
#define _IN2 3
#define _IN3 4
#define _IN4 5
#define _ENA 0
#define _ENB 0

void setup() {
  // put your setup code here, to run once:
  pinMode(_IN1,OUTPUT);
  pinMode(_IN2,OUTPUT);
  pinMode(_IN3,OUTPUT);
  pinMode(_IN4,OUTPUT);

  driveFree();
}
void driveFree()
{
  digitalWrite(_IN1,LOW);
  digitalWrite(_IN2,LOW);
  digitalWrite(_IN3,LOW);
  digitalWrite(_IN4,LOW);  
}
void forward()
{
  digitalWrite(_IN1,HIGH);
  digitalWrite(_IN2,LOW);
  digitalWrite(_IN3,HIGH);
  digitalWrite(_IN4,LOW);  
}
void reverse()
{
  digitalWrite(_IN1,LOW);
  digitalWrite(_IN2,HIGH);
  digitalWrite(_IN3,LOW);
  digitalWrite(_IN4,HIGH);  
}
void brake()
{
  digitalWrite(_IN1,HIGH);
  digitalWrite(_IN2,HIGH);
  digitalWrite(_IN3,HIGH);
  digitalWrite(_IN4,HIGH);  
}
void loop() {
  // put your main code here, to run repeatedly:
  //openDrive();
  forward();
  delay(1000);
  brake();
  delay(20);
  reverse();
  delay(1000);
  brake();
  delay(20);  
}
