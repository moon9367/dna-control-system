#include <Arduino.h>

// 핀 정의
const int tempSensorPin = A0; // 온도 센서 핀
const int heaterPin = 9;      // 히터 제어 핀
const int ledPin = 10;        // LED 제어 핀

// 서미스터 관련 상수
const int resistorValue = 100000; // 직렬 저항 값 (100K)
const float beta = 3950;          // 서미스터 베타 계수
const int refTemp = 25;           // 기준 온도 (섭씨)
const int refResistance = 100000; // 기준 저항 값 (100K)

// PID 상수
float kp = 5.0;   // 비례 이득
float ki = 0.1;   // 적분 이득
float kd = 1.0;   // 미분 이득

// PID 변수
float targetTemperature = 40.0; // 목표 온도
float integral = 0.0;           // 적분 항
float previousError = 0.0;      // 이전 오차
int currentTemperature = 0;     // 현재 온도

// 온도 계산 함수
int readTemperature() {
  int analogValue = analogRead(tempSensorPin);
  if (analogValue == 0) return -273; // 아날로그 값이 0이면 에러 방지

  float resistance = (1023.0 / analogValue - 1) * resistorValue; // 저항 계산
  float temperature = 1 / (log(resistance / refResistance) / beta + 1 / (refTemp + 273.15)) - 273.15; // 온도 계산
  return round(temperature); // 소숫점 제거
}

// PID 제어 함수 (히터 전용)
void controlHeater() {
  float error = targetTemperature - currentTemperature; // 현재 오차
  integral += error;                                    // 적분 계산
  float derivative = error - previousError;             // 미분 계산
  float output = kp * error + ki * integral + kd * derivative; // PID 출력 계산

  // 히터 출력 조정
  analogWrite(heaterPin, constrain(output, 0, 255)); // 0~255 범위로 제한

  // 이전 오차 업데이트
  previousError = error;
}

void setup() {
  // 핀 모드 설정
  pinMode(tempSensorPin, INPUT);
  pinMode(heaterPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  // 시리얼 통신 시작
  Serial.begin(9600);
}

void loop() {
  // 명령어 수신 확인
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n'); // 명령어 읽기 (줄바꿈 기준)
    command.trim(); // 공백 제거

    // LED 제어 명령어
    if (command == "LED_ON") {
      digitalWrite(ledPin, HIGH); // LED 켜기
    } else if (command == "LED_OFF") {
      digitalWrite(ledPin, LOW); // LED 끄기
    }
  }

  // 현재 온도 읽기
  currentTemperature = readTemperature();

  // PID 제어 함수 호출
  controlHeater();

  // 온도 데이터 전송
  Serial.print("Temperature: ");
  Serial.println(currentTemperature);

  // 1초 대기
  delay(1000);
}
