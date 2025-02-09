#include <Arduino.h>

// 핀 정의
const int tempSensorPin = A0;  // 서미스터 핀
const int heaterPin = 9;       // 히터 제어 핀
const int ledPin = 10;         // LED 제어 핀

// 목표 온도와 히스테리시스
const float targetTemperature = 40.0; // 목표 온도
const float hysteresis = 2.0;         // 히스테리시스 값 (온도 차)

// 플래그
bool enableTemperatureReading = true; // 온도 읽기 활성화 여부

// 온도 읽기 함수
float readTemperature() {
  int analogValue = analogRead(tempSensorPin); // 서미스터로부터 아날로그 값 읽기
  float resistance = (1023.0 / analogValue - 1) * 100000; // 저항값 계산 (100K 기준)
  float temperature = 1 / (log(resistance / 100000) / 3950 + 1 / (25 + 273.15)) - 273.15; // 온도 계산
  return temperature;
}

void setup() {
  Serial.begin(9600); // 시리얼 통신 시작
  pinMode(heaterPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  // 초기 상태: 히터와 LED OFF
  digitalWrite(heaterPin, LOW);
  digitalWrite(ledPin, LOW);

  Serial.println("Arduino 초기화 완료");
}

void loop() {
  float temperature = readTemperature();
  Serial.print("Temperature:");
  Serial.println(temperature);

  delay(2000);  // 2초마다 데이터 전송
}


  // 명령 처리
  handleSerialCommands();
}

void handleSerialCommands() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n'); // 시리얼에서 명령 읽기
    command.trim(); // 명령 문자열의 공백 제거

    // 온도 읽기 일시 중단
    enableTemperatureReading = false;

    if (command == "HEATER_ON") {
      Serial.println("HEATER_ON 명령 실행");
      // 히터 제어 로직: 40도 유지
      float temperature = readTemperature();
      if (temperature >= targetTemperature) {
        digitalWrite(heaterPin, LOW); // 목표 온도 도달 시 히터 끄기
        Serial.println("히터 OFF: 목표 온도 도달");
      } else if (temperature <= targetTemperature - hysteresis) {
        digitalWrite(heaterPin, HIGH); // 온도가 낮아지면 히터 켜기
        Serial.println("히터 ON: 온도가 목표치 이하");
      }
    } else if (command == "HEATER_OFF") {
      digitalWrite(heaterPin, LOW); // 히터 끄기
      Serial.println("HEATER_OFF 명령 실행");
    } else if (command == "LED_ON") {
      digitalWrite(ledPin, HIGH); // LED 켜기
      Serial.println("LED_ON 명령 실행");
    } else if (command == "LED_OFF") {
      digitalWrite(ledPin, LOW); // LED 끄기
      Serial.println("LED_OFF 명령 실행");
    } else {
      Serial.println("알 수 없는 명령입니다.");
    }

    // 명령 처리 완료 후 온도 읽기 재개
    enableTemperatureReading = true;
  }
}
