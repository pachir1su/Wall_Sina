#include <Stepper.h>

const int stepsPerRevolution = 2048;  // 28BYJ-48 모터의 전체 스텝 수

// 핀 번호 설정
int sensor = A0;    // 물 감지 센서
int led1 = 11;      // 첫 번째 LED 핀 번호
int led2 = 10;      // 두 번째 LED 핀 번호
int buzzer = 12;    // 부저 핀 번호
int threshold = 500; // 물 감지 기준값

// ULN2003 드라이버와 연결된 아두이노 핀 번호 설정 (4, 5, 6, 7 사용)
Stepper myStepper(stepsPerRevolution, 4, 6, 5, 7);

void setup() {
  pinMode(led1, OUTPUT);   // 첫 번째 LED 핀 출력으로 설정
  pinMode(led2, OUTPUT);   // 두 번째 LED 핀 출력으로 설정
  pinMode(buzzer, OUTPUT); // 부저 핀 출력으로 설정
  myStepper.setSpeed(15);  // 모터 속도 설정 (RPM)
  Serial.begin(9600);      // 시리얼 모니터 시작 (디버깅용)
}

void loop() {
  int data = analogRead(sensor); // 센서 값 읽기

  // 물이 감지되었는지 확인
  if (data > threshold) {
    // 부저 울리기
    tone(buzzer, 1000); // 부저에 1000Hz 소리 내기

    // LED와 모터 동시에 작동
    for (int i = 0; i < 10; i++) { // 부저가 울리는 동안 LED를 10번 교차 깜빡이기
      digitalWrite(led1, HIGH); // 첫 번째 LED 켜기
      digitalWrite(led2, LOW);  // 두 번째 LED 끄기
      myStepper.step(stepsPerRevolution / 32); // 모터 일정 스텝 회전
      delay(50);               // 50밀리초 대기

      digitalWrite(led1, LOW);  // 첫 번째 LED 끄기
      digitalWrite(led2, HIGH); // 두 번째 LED 켜기
      
      myStepper.step(stepsPerRevolution / 32); // 모터 일정 스텝 회전
      delay(50);               // 50밀리초 대기
    }

    // 부저 끄기
    noTone(buzzer); // 부저 소리 끄기
    // 모든 LED 끄기
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    delay(200);    // 0.2초 대기
  } else {
    // 물이 감지되지 않으면 LED, 부저, 모터 끄기
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    noTone(buzzer);
    // 모터를 멈추게 할 필요는 없습니다. 스텝을 명령하지 않으면 모터는 자동으로 멈춥니다.
  }

  Serial.println(data); // 센서 값 출력 (디버깅용)
}
