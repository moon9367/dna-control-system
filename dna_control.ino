#include <Arduino.h>

// 핀 정의
const int tempSensorPin = A0;  // 서미스터 핀
const int heaterPin = 9;       // PTC 히터 제어 핀 (MOSFET)
const int ledPin = 10;         // 파워 LED 제어 핀 (MOSFET)

bool heaterActive = false;  // 히터 동작 여부 저장

void setup() {
    Serial.begin(9600);  // 시리얼 통신 시작
    pinMode(ledPin, OUTPUT);
    pinMode(heaterPin, OUTPUT);
    digitalWrite(ledPin, HIGH); // 테스트 HIGH 상태 추후 변경경
    digitalWrite(heaterPin, LOW);
}

void loop() {
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();

        if (command == "LED_ON") {
            digitalWrite(ledPin, HIGH);
            Serial.println("LED_ON_OK");  // 응답 추가
        } 
        else if (command == "LED_OFF") {
            digitalWrite(ledPin, LOW);
            Serial.println("LED_OFF_OK");  // 응답 추가
        }
        else if (command == "HEATER_ON") {
            digitalWrite(heaterPin, HIGH);
            Serial.println("HEATER_ON_OK");  // 응답 추가
        } 
        else if (command == "HEATER_OFF") {
            digitalWrite(heaterPin, LOW);
            Serial.println("HEATER_OFF_OK");  // 응답 추가
        }
        else if (command == "GET_TEMP") {
            float temperature = readTemperature();
            Serial.print("Temperature:");
            Serial.println(temperature);
        }
        else {
            Serial.println("UNKNOWN_COMMAND");
        }
    }
}




  // 🌡️ 현재 온도 읽기
    float currentTemperature = readTemperature();

    // 📡 온도 출력 (디버깅용)
    Serial.print("Temperature: ");
    Serial.println(currentTemperature);

    // 🔥 히터 자동 제어
    if (heaterActive) {
        if (readTemperature() < 60.0) {
            digitalWrite(heaterPin, HIGH);  // 목표 온도 미만이면 히터 ON
        } else {
            digitalWrite(heaterPin, HIGH);  // 목표 온도 도달 후에도 유지
        }
    }

    delay(2000); // 2초마다 실행
}

// 📡 온도 센서 값 읽기 함수
float readTemperature() {
    int tempValue = analogRead(tempSensorPin);
    float voltage = tempValue * 5.0 / 1023.0;
    float temperature = (voltage - 0.5) * 100;
    return temperature;
}
