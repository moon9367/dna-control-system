#include <Arduino.h>

// 핀 정의
const int tempSensorPin = A0;   // 온도 센서 핀
const int heaterPin = 9;        // PTC 히터 제어 핀
const int ledPin = 10;          // LED 제어 핀

void setup() {
    Serial.begin(9600);         // 시리얼 통신 시작
    pinMode(tempSensorPin, INPUT);
    pinMode(heaterPin, OUTPUT);
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);  // 초기 LED 상태 OFF
    digitalWrite(heaterPin, LOW);
}

void loop() {
    // 🌡️ 온도 측정
    float temperature = readTemperature();
    Serial.print("Temperature:");
    Serial.println(temperature);

    // 📡 명령어 수신 및 처리
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');  // 명령어 끝까지 읽기
        command.trim();  // 불필요한 공백 제거

        if (command == "LED_ON") {
            digitalWrite(ledPin, HIGH);
            Serial.println("LED turned ON");
        } 
        else if (command == "LED_OFF") {
            digitalWrite(ledPin, LOW);
            Serial.println("LED turned OFF");
        }
        else if (command == "HEATER_ON") {
            digitalWrite(heaterPin, HIGH);
            Serial.println("Heater turned ON");
        } 
        else if (command == "HEATER_OFF") {
            digitalWrite(heaterPin, LOW);
            Serial.println("Heater turned OFF");
        }
        else {
            Serial.println("Unknown command");
        }
    }

    delay(2000);  // 2초마다 온도 측정
}

// 📡 온도 센서 값 읽기 함수
float readTemperature() {
    int analogValue = analogRead(tempSensorPin);
    float voltage = analogValue * (5.0 / 1023.0);
    float temperatureC = (voltage - 0.5) * 100.0;  // TMP36 센서 보정 공식
    return temperatureC;
}
