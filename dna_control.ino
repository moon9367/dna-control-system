#include <Arduino.h>

// 핀 정의
const int tempSensorPin = A0;  // 서미스터 핀
const int heaterPin = 9;       // PTC 히터 제어 핀 (MOSFET)
const int ledPin = 10;         // 파워 LED 제어 핀 (MOSFET)

bool heaterActive = false;              // 히터 자동 제어 활성화 여부
const float targetTemperature = 40.0;   // 목표 온도
const float hysteresis = 2.0;           // 온도 유지 범위 (히스테리시스)

void setup() {
    Serial.begin(9600);                 // 시리얼 통신 시작
    pinMode(ledPin, OUTPUT);
    pinMode(heaterPin, OUTPUT);
    digitalWrite(ledPin, LOW);
    digitalWrite(heaterPin, LOW);
}

void loop() {
    if (Serial.available()) {
        delay(100);
        String command = Serial.readStringUntil('\n');
        command.trim();

        if (command == "LED_ON") {
            digitalWrite(ledPin, HIGH);
            Serial.println("LED_ON_OK");
        } 
        else if (command == "LED_OFF") {
            digitalWrite(ledPin, LOW);
            Serial.println("LED_OFF_OK");
        }
        else if (command == "HEATER_ON") {
            heaterActive = true;                     // ✅ 자동 제어 활성화
            Serial.println("HEATER_ON_OK");
        } 
        else if (command == "HEATER_OFF") {
            heaterActive = false;                    // ✅ 자동 제어 비활성화
            digitalWrite(heaterPin, LOW);            // 히터 강제 OFF
            Serial.println("HEATER_OFF_OK");
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

    // 🔥 자동 제어 루프
    if (heaterActive) {
        float currentTemperature = readTemperature();
        Serial.print("Current Temperature: ");
        Serial.println(currentTemperature);

        if (currentTemperature < targetTemperature - hysteresis) {
            digitalWrite(heaterPin, HIGH);   // ✅ 온도가 목표보다 충분히 낮으면 히터 ON
        } 
        else if (currentTemperature > targetTemperature + hysteresis) {
            digitalWrite(heaterPin, LOW);    // ✅ 온도가 목표보다 충분히 높으면 히터 OFF
        }
        // 히스테리시스 범위 내에서는 현재 상태 유지
    }

    delay(2000); // 2초마다 온도 확인
}

// 📡 온도 센서 값 읽기 함수
float readTemperature() {
    int tempValue = analogRead(tempSensorPin);
    float voltage = tempValue * 5.0 / 1023.0;
    float temperature = (voltage - 0.5) * 100;
    return temperature;
}
