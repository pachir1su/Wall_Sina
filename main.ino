#include <Stepper.h>
#include <TM1637Display.h>  // TM1637 라이브러리 추가

const int stepsPerRevolution = 2048;  // 28BYJ-48 모터의 전체 스텝 수

// 핀 번호 설정
int sensor = A0;    // 물 감지 센서
int led1 = 11;      // 빨간색 LED 핀 번호
int led2 = 10;      // 파란색 LED 핀 번호
int buzzer = 12;    // 부저 핀 번호
int laser = 9;      // 레이저 핀 번호
int threshold = 500; // 물 감지 기준값

// 신호등 LED 핀
int greenLED = A3;   // 초록색 LED
int yellowLED = A4;  // 노란색 LED
int redLED = A5;     // 빨간색 LED

// ULN2003 드라이버와 연결된 아두이노 핀 번호 설정 (4, 5, 6, 7 사용)
Stepper myStepper(stepsPerRevolution, 4, 6, 5, 7);

// TM1637 디스플레이 핀 설정
const int CLK = 2;   // 클록 핀
const int DIO = 3;   // 데이터 핀
TM1637Display display(CLK, DIO); // 디스플레이 객체 생성

unsigned long signalStartTime = 0;  // 신호등 타이머 변수
bool signalActive = false;          // 신호등 활성화 상태
unsigned long blueLEDStartTime = 0; // 파란색 LED 타이머 변수
bool blueLEDState = false;          // 파란색 LED 상태

void setup() {
  pinMode(led1, OUTPUT);   // 빨간색 LED 핀 출력으로 설정
  pinMode(led2, OUTPUT);   // 파란색 LED 핀 출력으로 설정
  pinMode(buzzer, OUTPUT); // 부저 핀 출력으로 설정
  pinMode(laser, OUTPUT);  // 레이저 핀 출력으로 설정
  pinMode(greenLED, OUTPUT); // 초록색 LED 핀 출력으로 설정
  pinMode(yellowLED, OUTPUT); // 노란색 LED 핀 출력으로 설정
  pinMode(redLED, OUTPUT); // 빨간색 LED 핀 출력으로 설정

  digitalWrite(laser, HIGH); // 레이저 항상 켜기

  myStepper.setSpeed(15);  // 모터 속도 설정 (RPM)
  Serial.begin(9600);      // 시리얼 모니터 시작 (디버깅용)

  display.setBrightness(0x0f); // 최대 밝기 설정
  display.clear();            // 디스플레이 초기화
}

void loop() {
  int data = analogRead(sensor); // 센서 값 읽기

  if (data > threshold) {
    // 물이 감지되었으므로 신호등과 모터 작동 시작
    if (!signalActive) {
      signalActive = true;  // 신호등 활성화 상태로 변경
      signalStartTime = millis(); // 신호등 시작 시간 기록
      blueLEDStartTime = millis(); // 파란색 LED 시작 시간 초기화
    }

    // 신호등 상태 제어
    unsigned long currentTime = millis() - signalStartTime;
    int displayTime = 0; // 디스플레이에 표시할 시간 변수

    if (currentTime < 5000) { // 0 ~ 5초: 초록불
      digitalWrite(greenLED, HIGH);
      digitalWrite(yellowLED, LOW);
      digitalWrite(redLED, LOW);
      digitalWrite(led2, LOW); // 파란색 LED 끄기
      tone(buzzer, 1000); // 부저 작동
      displayTime = (5000 - currentTime) / 1000; // 남은 시간 초 단위
    } else if (currentTime < 10000) { // 5 ~ 10초: 노란불
      digitalWrite(greenLED, LOW);
      digitalWrite(yellowLED, HIGH);
      digitalWrite(redLED, LOW);
      digitalWrite(led2, LOW); // 파란색 LED 끄기
      tone(buzzer, 1000); // 부저 작동
      displayTime = (10000 - currentTime) / 1000; // 남은 시간 초 단위
    } else if (currentTime < 15000) { // 10 ~ 15초: 노란불 깜빡임
      digitalWrite(greenLED, LOW);
      digitalWrite(yellowLED, HIGH);
      digitalWrite(redLED, LOW);
      digitalWrite(led2, LOW); // 파란색 LED 끄기
      tone(buzzer, 1000); // 부저 작동
      delay(300);
      digitalWrite(yellowLED, LOW);
      delay(300);
      displayTime = (15000 - currentTime) / 1000; // 남은 시간 초 단위
    } else if (currentTime < 20000) { // 15 ~ 20초: 빨간불과 노란불 깜빡임
      digitalWrite(greenLED, LOW);
      digitalWrite(yellowLED, HIGH);
      digitalWrite(redLED, HIGH);
      digitalWrite(led2, LOW); // 파란색 LED 끄기
      tone(buzzer, 1000); // 부저 작동
      delay(300);
      digitalWrite(yellowLED, LOW);
      digitalWrite(redLED, LOW);
      delay(250);
      displayTime = (20000 - currentTime) / 1000; // 남은 시간 초 단위
    } else { // 20초 이상: 빨간불과 파란색 LED 깜빡임
      digitalWrite(greenLED, LOW);
      digitalWrite(yellowLED, LOW);
      digitalWrite(redLED, HIGH);

      // 파란색 LED 깜빡이기
      unsigned long blueLEDCurrentTime = millis() - blueLEDStartTime;
      if (blueLEDCurrentTime >= 500) {
        blueLEDState = !blueLEDState;
        digitalWrite(led2, blueLEDState ? HIGH : LOW);
        blueLEDStartTime = millis(); // 타이머 재설정
      }

      // 모터를 역방향으로 회전
      myStepper.step(-stepsPerRevolution / 32);
      noTone(buzzer); // 부저 끄기
      displayTime = 0; // 디스플레이에 0 표시
    }

    // 모터와 LED 작동 (부저가 작동하지 않을 때)
    if (currentTime < 20000) {
      // 신호등이 빨간불이 아닐 때만 LED를 깜빡임
      digitalWrite(led1, HIGH); // 빨간색 LED 켜기
      digitalWrite(led2, LOW);  // 파란색 LED 끄기
      myStepper.step(stepsPerRevolution / 32); // 모터 일정 스텝 회전
      delay(50);               // 50밀리초 대기

      digitalWrite(led1, LOW);  // 빨간색 LED 끄기
      digitalWrite(led2, HIGH); // 파란색 LED 켜기
      
      myStepper.step(stepsPerRevolution / 32); // 모터 일정 스텝 회전
      delay(50);               // 50밀리초 대기
    } else {
      // 빨간불 상태에서 모든 LED 끄기
      digitalWrite(led1, LOW);
      digitalWrite(led2, LOW);
    }

    // TM1637 디스플레이에 시간 표시
    display.showNumberDec(displayTime, false); // 남은 시간 초 단위 표시
  } else {
    // 물이 감지되지 않으면 신호등과 모터 작동 중지
    if (signalActive) {
      // 신호등 비활성화 상태로 변경
      signalActive = false;
      digitalWrite(greenLED, LOW);
      digitalWrite(yellowLED, LOW);
      digitalWrite(redLED, LOW);
      digitalWrite(led2, LOW); // 파란색 LED 끄기
    }

    digitalWrite(led1, LOW); // 빨간색 LED 끄기
    noTone(buzzer);
    // 모터를 멈추게 할 필요는 없습니다. 스텝을 명령하지 않으면 모터는 자동으로 멈춥니다.

    // TM1637 디스플레이에 0 표시
    display.showNumberDec(0, false); // 0 표시
  }

  Serial.println(data); // 센서 값 출력 (디버깅용)
}
