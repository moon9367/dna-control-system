#include <Arduino.h>

// 핀 정의
const int tempSensorPin = A0; // 온도 센서 핀
const int heaterPin = 9;      // 히터 제어 핀
const int ledPin = 10;        // LED 제어 핀

// 온도 계산에 필요한 상수
const float SERIES_RESISTOR = 100000.0;  // 시리즈 저항 값 (10kΩ)
const float NOMINAL_RESISTANCE = 100000.0; // 서미스터의 공칭 저항 (25°C에서 10kΩ)
const float NOMINAL_TEMPERATURE = 25.0;  // 공칭 온도 (25°C)
const float BETA_COEFFICIENT = 3950.0;   // 서미스터의 베타 계수

// 목표 온도 설정
float targetTemperature = 40.0;  // "HEATER_ON" 명령 시 유지할 온도
const float TEMPERATURE_THRESHOLD = 2.0; // 허용 오차 (±2°C)

// 변수
float temperature = 0.0; // 계산된 온도 값
bool heaterOn = false;   // 히터 상태

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

  // 온도 센서 값 읽기
  int analogValue = analogRead(tempSensorPin);

  // 서미스터 저항 계산
  float resistance = SERIES_RESISTOR / ((1023.0 / analogValue) - 1.0);

  // 온도 계산 (Steinhart-Hart 방정식)
  temperature = 1.0 / (log(resistance / NOMINAL_RESISTANCE) / BETA_COEFFICIENT + (1.0 / (NOMINAL_TEMPERATURE + 273.15)));
  temperature -= 273.15; // 켈빈에서 섭씨로 변환

  // 히터 제어 (목표 온도 유지)
  if (heaterOn) {
    if (temperature < targetTemperature - TEMPERATURE_THRESHOLD) {
      digitalWrite(heaterPin, HIGH); // 히터 켜기
    } else if (temperature > targetTemperature + TEMPERATURE_THRESHOLD) {
      digitalWrite(heaterPin, LOW); // 히터 끄기
    }
  }

  // 온도 데이터 전송
  Serial.print("Temperature: ");
  Serial.println(temperature);

  // 1초 대기
  delay(1000);
}
