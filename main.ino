#include <Stepper.h>

const int stepsPerRevolution = 2048;  // 28BYJ-48 모터의 전체 스텝 수

// 핀 번호 설정
int sensor = A0;    // 물 감지 센서
int led1 = 11;      // 첫 번째 LED 핀 번호
int led2 = 10;      // 두 번째 LED 핀 번호
int buzzer = 12;    // 부저 핀 번호
int threshold = 500; // 물 감지 기준값

// 신호등 LED 핀
int greenLED = 3;   // 초록색 LED
int yellowLED = 2;  // 노란색 LED
int redLED = 1;     // 빨간색 LED

// ULN2003 드라이버와 연결된 아두이노 핀 번호 설정 (4, 5, 6, 7 사용)
Stepper myStepper(stepsPerRevolution, 4, 6, 5, 7);

unsigned long signalStartTime = 0;  // 신호등 타이머 변수
bool signalActive = false;          // 신호등 활성화 상태

void setup() {
  pinMode(led1, OUTPUT);   // 첫 번째 LED 핀 출력으로 설정
  pinMode(led2, OUTPUT);   // 두 번째 LED 핀 출력으로 설정
  pinMode(buzzer, OUTPUT); // 부저 핀 출력으로 설정
  pinMode(greenLED, OUTPUT); // 초록색 LED 핀 출력으로 설정
  pinMode(yellowLED, OUTPUT); // 노란색 LED 핀 출력으로 설정
  pinMode(redLED, OUTPUT); // 빨간색 LED 핀 출력으로 설정

  myStepper.setSpeed(15);  // 모터 속도 설정 (RPM)
  Serial.begin(9600);      // 시리얼 모니터 시작 (디버깅용)
}

void loop() {
  int data = analogRead(sensor); // 센서 값 읽기

  if (data > threshold) {
    // 물이 감지되었으므로 신호등과 모터 작동 시작
    if (!signalActive) {
      signalActive = true;  // 신호등 활성화 상태로 변경
      signalStartTime = millis(); // 신호등 시작 시간 기록
    }

    // 신호등 상태 제어
    unsigned long currentTime = millis() - signalStartTime;
    if (currentTime < 5000) { // 0 ~ 5초: 초록불
      digitalWrite(greenLED, HIGH);
      digitalWrite(yellowLED, LOW);
      digitalWrite(redLED, LOW);
    } else if (currentTime < 10000) { // 5 ~ 10초: 노란불
      digitalWrite(greenLED, LOW);
      digitalWrite(yellowLED, HIGH);
      digitalWrite(redLED, LOW);
    } else if (currentTime < 15000) { // 10 ~ 15초: 노란불 깜빡임
      digitalWrite(greenLED, LOW);
      digitalWrite(yellowLED, HIGH);
      digitalWrite(redLED, LOW);
      delay(300);
      digitalWrite(yellowLED, LOW);
      delay(300);
    } else if (currentTime < 20000) { // 15 ~ 20초: 빨간불과 노란불 깜빡임
      digitalWrite(greenLED, LOW);
      digitalWrite(yellowLED, HIGH);
      digitalWrite(redLED, HIGH);
      delay(300);
      digitalWrite(yellowLED, LOW);
      digitalWrite(redLED, LOW);
      delay(300);
    } else { // 20초 이상: 빨간불
      digitalWrite(greenLED, LOW);
      digitalWrite(yellowLED, LOW);
      digitalWrite(redLED, HIGH);
    }

    // 모터와 LED 작동
    for (int i = 0; i < 10; i++) { // 부저가 울리는 동안 LED를 10번 교차 깜빡이기
    
    // 부저 울리기
    tone(buzzer, 1000); // 부저에 1000Hz 소리 내기
      digitalWrite(led1, HIGH); // 첫 번째 LED 켜기
      digitalWrite(led2, LOW);  // 두 번째 LED 끄기
      myStepper.step(stepsPerRevolution / 32); // 모터 일정 스텝 회전
      delay(50);               // 50밀리초 대기

      digitalWrite(led1, LOW);  // 첫 번째 LED 끄기
      digitalWrite(led2, HIGH); // 두 번째 LED 켜기
      
      myStepper.step(stepsPerRevolution / 32); // 모터 일정 스텝 회전
      delay(50);               // 50밀리초 대기
    }

    // 모든 LED 끄기
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, LOW);
    delay(200);    // 0.2초 대기
  } else {
    // 물이 감지되지 않으면 신호등과 모터 작동 중지
    if (signalActive) {
      // 신호등 비활성화 상태로 변경
      signalActive = false;
      digitalWrite(greenLED, LOW);
      digitalWrite(yellowLED, LOW);
      digitalWrite(redLED, LOW);
    }

    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    noTone(buzzer);
    // 모터를 멈추게 할 필요는 없습니다. 스텝을 명령하지 않으면 모터는 자동으로 멈춥니다.
  }

  Serial.println(data); // 센서 값 출력 (디버깅용)
}
