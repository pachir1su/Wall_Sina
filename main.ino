int sensor = A0; // water sensor에 연결된 A0(아날로그)을 sensor 변수에 저장
int led = 11;
int threshold = 500; // 물 감지의 기준값 설정 (적절한 값으로 설정)

void setup() {
  pinMode(led, OUTPUT); // LED 핀을 출력으로 설정
  Serial.begin(9600); // 시리얼 모니터 시작 (디버깅용)
}

void loop() {
  int data = analogRead(sensor); // 센서 값 읽기

  // 물이 감지되었는지 확인
  if (data > threshold) {
    // LED 깜빡임
    digitalWrite(led, HIGH); // LED 켜기
    delay(500); // 500밀리초 대기
    digitalWrite(led, LOW); // LED 끄기
    delay(500); // 500밀리초 대기
  } else {
    digitalWrite(led, LOW); // LED 끄기
  }

  Serial.println(data); // 센서 값 출력 (디버깅용)
}
