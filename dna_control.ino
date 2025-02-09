#include <Arduino.h>

// 핀 정의
const int tempSensorPin = A0;  // 서미스터 핀
const int heaterPin = 9;       // 히터 제어 핀
const int ledPin = 10;         // LED 제어 핀

// 목표 온도와 히스테리시스
const float targetTemperature = 40.0; // 목표 온도
const float hysteresis = 2.0;         // 히스테리시스 값 (온도 차)

// 상태 플래그
bool isHeaterOn = false;

// 온도 읽기 함수
float readTemperature() {
  int analogValue = analogRead(tempSensorPin);
  float resistance = (1023.0 / analogValue - 1) * 100000; // 저항값 계산 (100K 기준)
  float temperature = 1 / (log(resistance / 100000) / 3950 + 1 / (25 + 273.15)) - 273.15;
  return temperature;
}

void setup() {
  Serial.begin(9600);
  pinMode(heaterPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  digitalWrite(heaterPin, LOW); // 초기 상태: 히터 OFF
  digitalWrite(ledPin, LOW);    // 초기 상태: LED OFF

  Serial.println("Arduino 초기화 완료");
}

void loop() {
  // 명령 처리
  handleSerialCommands();

  // 온도 전송 (히터가 꺼져 있을 때만)
  if (!isHeaterOn) {
    float temperature = readTemperature();
    Serial.print("Temperature:");
    Serial.println(temperature);
    delay(2000); // 2초마다 데이터 전송
  }
}

void handleSerialCommands() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "HEATER_ON") {
      Serial.println("HEATER_ON 명령 실행");
      isHeaterOn = true; // 히터 상태 플래그 활성화
      digitalWrite(heaterPin, HIGH);

      // 목표 온도 유지
      while (true) {
        float temperature = readTemperature();
        Serial.print("Temperature:");
        Serial.println(temperature);

        if (temperature >= targetTemperature) {
          digitalWrite(heaterPin, LOW);
          Serial.println("히터 OFF: 목표 온도 도달");
          break;
        }

        delay(1000); // 1초마다 온도 확인
      }

      isHeaterOn = false; // 히터 상태 플래그 비활성화
    } else if (command == "HEATER_OFF") {
      digitalWrite(heaterPin, LOW);
      Serial.println("HEATER_OFF 명령 실행");
      isHeaterOn = false;
    } else if (command == "LED_ON") {
      digitalWrite(ledPin, HIGH);
      Serial.println("LED_ON 명령 실행");
    } else if (command == "LED_OFF") {
      digitalWrite(ledPin, LOW);
      Serial.println("LED_OFF 명령 실행");
    } else {
      Serial.println("알 수 없는 명령입니다.");
    }
  }
}
