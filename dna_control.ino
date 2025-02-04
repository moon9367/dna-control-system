#include <Arduino.h>

// 핀 정의
const int tempSensorPin = A0;  // 서미스터 핀
const int heaterPin = 9;       // PTC 히터 제어 핀 (MOSFET)
const int ledPin = 10;         // 파워 LED 제어 핀 (MOSFET)

bool heaterActive = false;              // 히터 자동 제어 활성화 여부
const float targetTemperature = 40.0;   // 목표 온도
const float hysteresis = 0.5;           // ✅ 히스테리시스 범위 축소
bool commandReceived = false;           // ✅ 명령어 수신 상태 플래그

void setup() {
    Serial.begin(9600);                 // 시리얼 통신 시작
    pinMode(ledPin, OUTPUT);
    pinMode(heaterPin, OUTPUT);
    digitalWrite(ledPin, LOW);
    digitalWrite(heaterPin, LOW);
}

void loop() {
    // ✅ 1️⃣ 명령어 수신 처리
    if (Serial.available()) {
        commandReceived = true;         // 🔒 온도 읽기 일시 중지
        String command = Serial.readStringUntil('\n');
        command.trim();
        
        Serial.println("⏸️ 온도 읽기 중지"); // 디버깅 메시지

        if (command == "LED_ON") {
            digitalWrite(ledPin, HIGH);
            Serial.println("LED_ON_OK");
        } 
        else if (command == "LED_OFF") {
            digitalWrite(ledPin, LOW);
            Serial.println("LED_OFF_OK");
        }
        else if (command == "HEATER_ON") {
            heaterActive = true;
            Serial.println("HEATER_ON_OK");
        } 
        else if (command == "HEATER_OFF") {
            heaterActive = false;
            digitalWrite(heaterPin, LOW);
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

        commandReceived = false;        // 🔓 명령어 처리 후 온도 읽기 재개
        Serial.println("▶️ 온도 읽기 재개"); // 디버깅 메시지
    }

    // ✅ 2️⃣ 자동 온도 제어 루프
    if (heaterActive && !commandReceived) {  // ❗ 명령어 수신 중에는 동작 중지
        float currentTemperature = readTemperature();
        Serial.print("Current Temperature: ");
        Serial.println(currentTemperature);

        if (currentTemperature < targetTemperature - hysteresis) {
            digitalWrite(heaterPin, HIGH);   // 온도가 목표보다 충분히 낮으면 히터 ON
        } 
        else if (currentTemperature >= targetTemperature) {
            digitalWrite(heaterPin, LOW);    // 목표 온도 도달 시 즉시 히터 OFF
        }
    }

    delay(500);  // 0.5초마다 온도 확인
}

// 📡 온도 센서 값 읽기 함수
float readTemperature() {
    int tempValue = analogRead(tempSensorPin);
    float voltage = tempValue * 5.0 / 1023.0;
    float temperature = (voltage - 0.5) * 100;
    return temperature;
}
