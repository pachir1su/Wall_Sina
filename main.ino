int sensor = A0; // water sensor에 연결된 A0(아날로그)을 sensor 변수에 저장
int led = 11;

void setup () {
pinMode(11,OUTPUT);
}

void loop() {
  int data = analogRead(sensor);

int light = map(data, 0, 1023, 0, 255);
analogWrite(led, light);
}
