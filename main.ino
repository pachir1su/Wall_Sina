int sensor = A0; // water sensor에 연결된 A0(아날로그)을 sensor 변수에 저장
int led = 11;    // LED 핀 번호
int buzzer = 12; // 부저 핀 번호
int threshold = 500; // 물 감지의 기준값 설정 (적절한 값으로 설정)

void setup() {
  pinMode(led, OUTPUT);    // LED 핀을 출력으로 설정
  pinMode(buzzer, OUTPUT); // 부저 핀을 출력으로 설정
  Serial.begin(9600);      // 시리얼 모니터 시작 (디버깅용)
}

void loop() {
  int data = analogRead(sensor); // 센서 값 읽기

  // 물이 감지되었는지 확인
  if (data > threshold) {
    // LED 깜빡이기
    digitalWrite(led, HIGH); // LED 켜기
    delay(250);              // 250밀리초 대기
    digitalWrite(led, LOW);  // LED 끄기
    delay(250);              // 250밀리초 대기

    // 부저 울리기
    tone(buzzer, 1000); // 부저에 1000Hz 소리 내기
    delay(1250);         // 1250밀리초 대기
    noTone(buzzer);     // 부저 소리 끄기
    delay(500);         // 500밀리초 대기
  } else {
    // 물이 감지되지 않으면 LED와 부저 끄기
    digitalWrite(led, LOW);
    noTone(buzzer);
  }

  Serial.println(data); // 센서 값 출력 (디버깅용)
}
