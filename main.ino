#include <Stepper.h>
#include <TM1637Display.h>

const int stepsPerRevolution = 2048;  // 28BYJ-48 모터의 전체 스텝 수

// 핀 번호 설정
int sensor = A0;    // 물 감지 센서
int led1 = 11;      // 빨간색 LED 핀 번호
int led2 = 10;      // 파란색 LED 핀 번호
int buzzer = 12;    // 부저 핀 번호
int laserPin = 9;   // 레이저 핀 번호
int buttonPin = 1;  // 버튼 핀 번호
int threshold = 500; // 물 감지 기준값

// 신호등 LED 핀 (작동 LED)
int operationGreenLED = A1;   // 작동 LED 초록색 핀 (시스템 ON)
int operationYellowLED = A2;  // 작동 LED 노란색 핀 (예비 핀)
int operationRedLED = 8;      // 작동 LED 빨간색 핀 (시스템 OFF)

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
int ledTogglePhase = 0;             // 신호등 3색 교차 깜빡임 상태

bool isBuzzerOn = false;            // 부저가 켜져 있는지 여부를 추적하는 변수
bool waterDetected = false;         // 물 감지 여부를 저장하는 변수
bool systemOn = false;              // 시스템 ON/OFF 상태
unsigned long buttonDebounceTime = 0; // 버튼 디바운스 타이머

void setup() {
  pinMode(led1, OUTPUT);   // 빨간색 LED 핀 출력으로 설정
  pinMode(led2, OUTPUT);   // 파란색 LED 핀 출력으로 설정
  pinMode(buzzer, OUTPUT); // 부저 핀 출력으로 설정
  pinMode(laserPin, OUTPUT); // 레이저 핀 출력으로 설정
  pinMode(buttonPin, INPUT_PULLUP); // 버튼 핀을 풀업 저항 사용
  pinMode(greenLED, OUTPUT); // 초록색 LED 핀 출력으로 설정
  pinMode(yellowLED, OUTPUT); // 노란색 LED 핀 출력으로 설정
  pinMode(redLED, OUTPUT); // 빨간색 LED 핀 출력으로 설정

  pinMode(operationGreenLED, OUTPUT); // 작동 LED 초록색 출력
  pinMode(operationYellowLED, OUTPUT); // 작동 LED 노란색 출력
  pinMode(operationRedLED, OUTPUT); // 작동 LED 빨간색 출력

  myStepper.setSpeed(15);  // 모터 속도 설정 (RPM)
  display.setBrightness(0x0f); // TM1637 최대 밝기 설정
  Serial.begin(9600);      // 시리얼 모니터 시작 (디버깅용)

  // 레이저 상시 켜기
  digitalWrite(laserPin, HIGH);

  // 시스템 초기 상태 OFF로 설정 (작동 LED 빨간색)
  digitalWrite(operationGreenLED, LOW);
  digitalWrite(operationYellowLED, LOW);
  digitalWrite(operationRedLED, HIGH);
}

void resetSystem() {
  waterDetected = false;  // 물 감지 상태 초기화
  signalActive = false;   // 신호등 활성화 상태 초기화
  display.clear();        // 디스플레이 초기화
  noTone(buzzer);         // 부저 끄기
  digitalWrite(greenLED, LOW);
  digitalWrite(yellowLED, LOW);
  digitalWrite(redLED, LOW);
  digitalWrite(led1, LOW); // 빨간색 LED 끄기
  digitalWrite(led2, LOW); // 파란색 LED 끄기
  myStepper.step(0);       // 모터 정지
  Serial.println("System Reset");
}

void loop() {
  int data = analogRead(sensor); // 센서 값 읽기
  unsigned long currentTime = millis();
  bool currentButtonState = digitalRead(buttonPin);

  // 버튼이 눌렸는지 체크 (디바운스 처리)
  if (currentButtonState == LOW && (millis() - buttonDebounceTime) > 200) {
    buttonDebounceTime = millis();
    systemOn = !systemOn;  // 시스템 ON/OFF 상태 토글

    if (systemOn) {
      // 시스템 ON: 작동 LED 초록색 켜기, 빨간색 끄기
      digitalWrite(operationGreenLED, HIGH);
      digitalWrite(operationRedLED, LOW);
      Serial.println("System ON");
    } else {
      // 시스템 OFF: 작동 LED 빨간색 켜기, 초록색 끄기
      digitalWrite(operationGreenLED, LOW);
      digitalWrite(operationRedLED, HIGH);
      resetSystem();  // 시스템 정지 및 초기화
      Serial.println("System OFF");
    }
  }

  // 시스템이 ON 상태일 때만 코드가 실행되도록
  if (systemOn) {
    if (!waterDetected) {  // 물이 한 번도 감지되지 않은 경우
      if (data > threshold) {
        waterDetected = true;  // 물이 감지되면 상태를 변경
        signalStartTime = millis();  // 물이 감지된 순간 타이머 시작
        signalActive = true;
      }
    }

    if (waterDetected) {  // 물이 감지된 후 계속 작동
      unsigned long elapsedTime = currentTime - signalStartTime;

      if (elapsedTime < 27000) { // 초록불 상태 (0 ~ 27초)
        digitalWrite(greenLED, HIGH);
        digitalWrite(yellowLED, LOW);
        digitalWrite(redLED, LOW);

        // 부저가 꺼져 있으면 켜기
        if (!isBuzzerOn) {
          tone(buzzer, 1000); // 부저 작동
          isBuzzerOn = true;  // 부저가 켜졌음을 기록
        }
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

        // 부저가 꺼져 있으면 켜기
        if (!isBuzzerOn) {
          tone(buzzer, 1000); // 부저 작동
          isBuzzerOn = true;
        }

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

        // 부저가 켜져 있으면 끄기
        if (isBuzzerOn) {
          noTone(buzzer);  // 부저 끄기
          isBuzzerOn = false;
        }

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

        // 부저가 켜져 있으면 끄기
        if (isBuzzerOn) {
          noTone(buzzer); // 부저 끄기
          isBuzzerOn = false;
        }

        if (currentTime - ledToggleTime >= 500) {
          ledState = !ledState;
          digitalWrite(redLED, ledState);
          digitalWrite(yellowLED, ledState);
          ledToggleTime = currentTime;
        }

        digitalWrite(led1, HIGH); // 빨간색 LED 빠르게 깜빡임
        digitalWrite(led2, LOW); // 파란색 LED 꺼짐
        display.showNumberDec(90 - elapsedTime / 1000, true); // 카운트다운 표시

      } else if (elapsedTime < 110000) { // 빨간불 상태 (90 ~ 110초)
        digitalWrite(greenLED, LOW);
        digitalWrite(yellowLED, LOW);
        digitalWrite(redLED, HIGH);

        // 부저가 켜져 있으면 끄기
        if (isBuzzerOn) {
          noTone(buzzer); // 부저 끄기
          isBuzzerOn = false;
        }

        myStepper.step(-stepsPerRevolution / 32); // 모터 역방향 회전

        if (currentTime - ledToggleTime >= 500) {
          ledState = !ledState;
          digitalWrite(led2, ledState); // 파란색 LED 깜빡임
          ledToggleTime = currentTime;
        }

        digitalWrite(led1, LOW); // 빨간색 LED 꺼짐
        display.showNumberDec(elapsedTime / 1000, true); // 카운트업 표시

      } else { // 110초 이상 - 신호등 3개의 LED 교차 깜빡임, 모터 정지
        // 부저가 켜져 있으면 끄기
        if (isBuzzerOn) {
          noTone(buzzer); // 부저 끄기
          isBuzzerOn = false;
        }

        if (currentTime - ledToggleTime >= 500) {
          ledTogglePhase = (ledTogglePhase + 1) % 3;
          if (ledTogglePhase == 0) {
            digitalWrite(redLED, HIGH);
            digitalWrite(yellowLED, LOW);
            digitalWrite(greenLED, LOW);
          } else if (ledTogglePhase == 1) {
            digitalWrite(redLED, LOW);
            digitalWrite(yellowLED, HIGH);
            digitalWrite(greenLED, LOW);
          } else {
            digitalWrite(redLED, LOW);
            digitalWrite(yellowLED, LOW);
            digitalWrite(greenLED, HIGH);
          }
          ledToggleTime = currentTime;
        }

        digitalWrite(led1, LOW); // 빨간색 LED 꺼짐
        digitalWrite(led2, LOW); // 파란색 LED 꺼짐
        display.showNumberDec(elapsedTime / 1000, true); // 카운트업 표시
        // 모터 멈춤
      }
    }
  }

  Serial.println(data); // 센서 값 출력 (디버깅용)
}
