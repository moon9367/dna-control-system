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

// 목표 온도
const float targetTemperature = 40.0;  // "HEATER_ON" 명령 시 유지할 온도
const float temperatureThreshold = 2.0; // 허용 오차 (±2°C)

// 전역 변수
float currentTemperature = 0.0; // 실시간 온도 저장
bool heaterOn = false;          // 히터 상태

// 온도 계산 함수
float readTemperature() {
  int analogValue = analogRead(tempSensorPin);
  if (analogValue == 0) return -273.15; // 아날로그 값이 0이면 에러 방지

  float resistance = (1023.0 / analogValue - 1) * resistorValue; // 저항 계산
  float temperature = 1 / (log(resistance / refResistance) / beta + 1 / (refTemp + 273.15)) - 273.15; // 온도 계산
  return temperature;
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

    // 명령어 처리
    if (command == "HEATER_ON") {
      heaterOn = true;
    } else if (command == "HEATER_OFF") {
      heaterOn = false;
      digitalWrite(heaterPin, LOW); // 히터 끄기
    } else if (command == "LED_ON") {
      digitalWrite(ledPin, HIGH); // LED 켜기
    } else if (command == "LED_OFF") {
      digitalWrite(ledPin, LOW);  // LED 끄기
    }
  }

  // 온도 읽기
  currentTemperature = readTemperature();

  // 히터 제어 (목표 온도 유지)
  if (heaterOn) {
    if (currentTemperature < targetTemperature - temperatureThreshold) {
      digitalWrite(heaterPin, HIGH); // 히터 켜기
    } else if (currentTemperature > targetTemperature + temperatureThreshold) {
      digitalWrite(heaterPin, LOW); // 히터 끄기
    }
  }

  // 온도 데이터 전송
  Serial.print("Temperature: ");
  Serial.println(currentTemperature);

  // 1초 대기
  delay(1000);
}
