#include <Arduino.h>

const int LED_PIN = 9;         // LED 제어 핀
const int HEATER_PIN = 10;      // PTC 히터 제어 핀
const int TEMP_SENSOR_PIN = A0;  // 온도 센서 핀
const float TARGET_TEMPERATURE = 60.0; // 목표 온도 60°C

bool heaterActive = false;  // 히터 동작 여부 저장

void setup() {
    Serial.begin(9600);       // 시리얼 통신 시작
    pinMode(LED_PIN, OUTPUT);
    pinMode(HEATER_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(HEATER_PIN, LOW);
}

void loop() {
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();  // 개행 문자 제거

        Serial.print("🛠️ Received command: ");
        Serial.println(command);  // 명령 입력 확인

        // 🔥 LED 제어
        if (command == "a") {
            digitalWrite(LED_PIN, HIGH);  // LED ON
            Serial.println("✅ LED ON");
        }
        else if (command == "b") {
            digitalWrite(LED_PIN, LOW);   // LED OFF
            Serial.println("✅ LED OFF");
        }

        // 🔥 히터 ON
        else if (command == "c") {
            heaterActive = true;  // 히터 활성화
            Serial.println("✅ Heater ON (Target: 60°C)");
        }

        // 🔥 히터 OFF
        else if (command == "d") {
            heaterActive = false;  // 히터 비활성화
            digitalWrite(HEATER_PIN, LOW);
            Serial.println("✅ Heater OFF");
        }
    }

    // 🌡️ 현재 온도 읽기 (변수 선언)
    float currentTemperature = readTemperature();

    // ✅ 특정 조건에서만 온도를 출력하도록 변경
    if (heaterActive) {  // 히터가 켜져 있을 때만 출력
        Serial.print("🌡️ Temperature: ");
        Serial.print(currentTemperature);
        Serial.println(" °C");
    }

    // 🌡️ 히터 자동 제어
    if (heaterActive) {
        if (currentTemperature < TARGET_TEMPERATURE) {
            digitalWrite(HEATER_PIN, HIGH);  // 목표 온도 미만이면 히터 계속 ON
        } else {
            digitalWrite(HEATER_PIN, HIGH);  // 목표 온도 도달 후에도 유지
        }
    }

    delay(2000); // 2초마다 실행
}

// 📡 온도 센서 값 읽기 함수
float readTemperature() {
    int tempValue = analogRead(TEMP_SENSOR_PIN);  // 아날로그 값 읽기
    float voltage = tempValue * 5.0 / 1023.0;  // 전압 변환 (0~5V)
    float temperature = (voltage - 0.5) * 100;  // 온도 변환 (센서 보정 필요)
    return temperature;
}
