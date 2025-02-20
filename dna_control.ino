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
const float targetTemperature = 60.0;  // "HEATER_ON" 명령 시 유지할 온도
const float temperatureThreshold = 2.0; // 허용 오차 (±2°C)

// 이동 평균 필터 설정
#define SAMPLE_SIZE 10 // 이동 평균에 사용할 샘플 크기
float temperatureSamples[SAMPLE_SIZE];
int sampleIndex = 0;

unsigned long heaterStartTime = 0;        // 히터가 켜진 시간을 저장할 변수
const unsigned long heaterMaxDuration = 40*60000; // 히터 최대 동작 시간 (1분)

// 전역 변수
float currentTemperature = 0.0; // 실시간 온도 저장
bool heaterOn = false;          // 히터 상태

// 온도 계산 함수
float readTemperature() {
  int analogValue = analogRead(tempSensorPin);
  if (analogValue == 0) return -273.15; // 아날로그 값이 0이면 에러 방지

  float resistance = (1023.0 / analogValue - 1) * resistorValue; // 저항 계산
  float temperature = 1 / (log(resistance / refResistance) / beta + 1 / (refTemp + 273.15)) - 273.15; // 온도 계산

  // 이동 평균 필터 적용
  temperatureSamples[sampleIndex] = temperature; // 현재 측정값 저장
  sampleIndex = (sampleIndex + 1) % SAMPLE_SIZE; // 인덱스 순환

  // 평균값 계산
  float averageTemperature = 0.0;
  for (int i = 0; i < SAMPLE_SIZE; i++) {
    averageTemperature += temperatureSamples[i];
  }
  return averageTemperature / SAMPLE_SIZE; // 평균값 반환
}

void setup() {
  // 핀 모드 설정
  pinMode(tempSensorPin, INPUT);
  pinMode(heaterPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  // 시리얼 통신 시작
  Serial.begin(9600);

  // 이동 평균 초기화
  for (int i = 0; i < SAMPLE_SIZE; i++) {
    temperatureSamples[i] = 0.0;
  }
}

void loop() {
  // 명령어 수신 확인
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n'); // 명령어 읽기 (줄바꿈 기준)
    command.trim(); // 공백 제거

    // 명령어 처리
    if (command == "HEATER_ON") {
      heaterOn = true;
      heaterStartTime = millis(); // 히터 시작 시간 기록
      Serial.println("히터가 켜졌습니다.");
    } else if (command == "HEATER_OFF") {
      heaterOn = false;
      digitalWrite(heaterPin, LOW); // 히터 끄기
      Serial.println("히터가 꺼졌습니다.");
    } else if (command == "LED_ON") {
      digitalWrite(ledPin, HIGH); // LED 켜기
      Serial.println("LED가 켜졌습니다.");
      delay(15*60000); // 15분 후 자동 종료 안전 기능
      digitalWrite(ledPin, LOW);
    } else if (command == "LED_OFF") {
      digitalWrite(ledPin, LOW);  // LED 끄기
      Serial.println("LED가 꺼졌습니다.");
    }
  }

  // 온도 읽기
  currentTemperature = readTemperature();

  // 히터 제어 (목표 온도 유지)
  if (heaterOn) {
    if (millis() - heaterStartTime > heaterMaxDuration) {
      // 히터가 최대 동작 시간을 초과한 경우
      heaterOn = false;
      digitalWrite(heaterPin, LOW);
      Serial.println("히터가 최대 동작 시간을 초과하여 OFF되었습니다.");
    } else {
      // 온도에 따라 히터 제어
      if (currentTemperature < targetTemperature - temperatureThreshold) {
        digitalWrite(heaterPin, HIGH); // 히터 켜기
      } else if (currentTemperature > targetTemperature + temperatureThreshold) {
        digitalWrite(heaterPin, LOW); // 히터 끄기
      }
    }
  }

  // 온도 데이터 전송
  Serial.print("Temperature: ");
  Serial.println(currentTemperature);

  // 1초 대기
  delay(1000);
}
