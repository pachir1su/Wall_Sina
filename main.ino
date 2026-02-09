#include <Stepper.h>
#include <TM1637Display.h>

const int stepsPerRevolution = 2048;  // 28BYJ-48 모터의 전체 스텝 수

// 핀 번호 설정
int sensor = A0;    // 물 감지 센서
int led1 = 11;      // 빨간색 LED 핀 번호
int led2 = 10;      // 파란색 LED 핀 번호
int buzzer = 12;    // 부저 핀 번호
int laserPin = 9;   // 레이저 핀 번호
int threshold = 500; // 물 감지 기준값

// 신호등 LED 핀
int greenLED = A3;   // 초록색 LED
int yellowLED = A4;  // 노란색 LED
int redLED = A5;     // 빨간색 LED

// TM1637 핀 설정
#define CLK 2
#define DIO 3
TM1637Display display(CLK, DIO);

// ULN2003 드라이버와 연결된 아두이노 핀 번호 설정 (4, 5, 6, 7 사용)
Stepper myStepper(stepsPerRevolution, 4, 6, 5, 7);

unsigned long signalStartTime = 0;  // 신호등 타이머 변수
bool signalActive = false;          // 신호등 활성화 상태
unsigned long ledToggleTime = 0;    // LED 교차 깜빡임 타이머
bool ledState = false;              // LED 상태 토글 변수

void setup() {
  pinMode(led1, OUTPUT);   // 빨간색 LED 핀 출력으로 설정
  pinMode(led2, OUTPUT);   // 파란색 LED 핀 출력으로 설정
  pinMode(buzzer, OUTPUT); // 부저 핀 출력으로 설정
  pinMode(laserPin, OUTPUT); // 레이저 핀 출력으로 설정
  pinMode(greenLED, OUTPUT); // 초록색 LED 핀 출력으로 설정
  pinMode(yellowLED, OUTPUT); // 노란색 LED 핀 출력으로 설정
  pinMode(redLED, OUTPUT); // 빨간색 LED 핀 출력으로 설정

  myStepper.setSpeed(15);  // 모터 속도 설정 (RPM)
  display.setBrightness(0x0f); // TM1637 최대 밝기 설정
  Serial.begin(9600);      // 시리얼 모니터 시작 (디버깅용)

  // 레이저 상시 켜기
  digitalWrite(laserPin, HIGH);
}

void loop() {
  int data = analogRead(sensor); // 센서 값 읽기
  unsigned long currentTime = millis();

  if (data > threshold) {
    // 물이 감지되었으므로 신호등과 모터 작동 시작
    if (!signalActive) {
      signalActive = true;  // 신호등 활성화 상태로 변경
      signalStartTime = currentTime; // 신호등 시작 시간 기록
    }

    unsigned long elapsedTime = currentTime - signalStartTime;

    if (elapsedTime < 27000) { // 초록불 상태 (0 ~ 27초)
      digitalWrite(greenLED, HIGH);
      digitalWrite(yellowLED, LOW);
      digitalWrite(redLED, LOW);
      tone(buzzer, 1000); // 부저 작동
      myStepper.step(stepsPerRevolution / 32); // 모터 정방향 회전

      if (currentTime - ledToggleTime >= 500) {
        ledState = !ledState;
        digitalWrite(led1, ledState);
        digitalWrite(led2, !ledState);
        ledToggleTime = currentTime;
      }

      display.showNumberDec(27 - elapsedTime / 1000, true); // 카운트다운 표시

    } else if (elapsedTime < 47000) { // 노란불 상태 (27 ~ 47초)
      digitalWrite(greenLED, LOW);
      digitalWrite(yellowLED, HIGH);
      digitalWrite(redLED, LOW);
      tone(buzzer, 1000); // 부저 작동

      if (currentTime - ledToggleTime >= 500) {
        ledState = !ledState;
        digitalWrite(led1, ledState);
        digitalWrite(led2, !ledState);
        ledToggleTime = currentTime;
      }

      display.showNumberDec(47 - elapsedTime / 1000, true); // 카운트다운 표시

    } else if (elapsedTime < 70000) { // 노란불 점멸 상태 (47 ~ 70초)
      digitalWrite(greenLED, LOW);
      digitalWrite(redLED, LOW);
      noTone(buzzer); // 부저 끄기

      if (currentTime - ledToggleTime >= 600) {
        ledState = !ledState;
        digitalWrite(yellowLED, ledState);
        digitalWrite(led1, ledState); // 빨간색 LED 천천히 깜빡임
        ledToggleTime = currentTime;
      }
      digitalWrite(led2, LOW); // 파란색 LED 꺼짐
      display.showNumberDec(70 - elapsedTime / 1000, true); // 카운트다운 표시

    } else if (elapsedTime < 90000) { // 빨간불과 노란불 점멸 상태 (70 ~ 90초)
      digitalWrite(greenLED, LOW);
      noTone(buzzer); // 부저 끄기

      if (currentTime - ledToggleTime >= 500) {
        ledState = !ledState;
        digitalWrite(redLED, ledState);
        digitalWrite(yellowLED, ledState);
        ledToggleTime = currentTime;
      }

      digitalWrite(led1, HIGH); // 빨간색 LED 빠르게 깜빡임
      digitalWrite(led2, LOW); // 파란색 LED 꺼짐
      display.showNumberDec(90 - elapsedTime / 1000, true); // 카운트다운 표시

    } else { // 빨간불 상태 (90초 이상)
      digitalWrite(greenLED, LOW);
      digitalWrite(yellowLED, LOW);
      digitalWrite(redLED, HIGH);
      noTone(buzzer); // 부저 끄기
      myStepper.step(-stepsPerRevolution / 32); // 모터 역방향 회전

      if (currentTime - ledToggleTime >= 500) {
        ledState = !ledState;
        digitalWrite(led2, ledState); // 파란색 LED 깜빡임
        ledToggleTime = currentTime;
      }

      digitalWrite(led1, LOW); // 빨간색 LED 꺼짐
      display.showNumberDec(elapsedTime / 1000, true); // 카운트업 표시
    }

  } else {
    // 물이 감지되지 않으면 신호등과 모터 작동 중지
    if (signalActive) {
      signalActive = false; // 신호등 비활성화 상태로 변경
      digitalWrite(greenLED, LOW);
      digitalWrite(yellowLED, LOW);
      digitalWrite(redLED, LOW);
      digitalWrite(led1, LOW); // 빨간색 LED 끄기
      digitalWrite(led2, LOW); // 파란색 LED 끄기
      noTone(buzzer);
      display.clear();
    }
  }

  Serial.println(data); // 센서 값 출력 (디버깅용)
}
