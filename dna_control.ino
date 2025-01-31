#include <Arduino.h>

// 핀 정의
const int tempSensorPin = A0;  // 서미스터 핀
const int heaterPin = 9;       // PTC 히터 제어 핀 (MOSFET)
const int ledPin = 10;         // 파워 LED 제어 핀 (MOSFET)

bool heaterActive = false;  // 히터 동작 여부 저장

void setup() {
    Serial.begin(9600);       // 시리얼 통신 시작
    pinMode(ledPin, OUTPUT);
    pinMode(heaterPin, OUTPUT);
    digitalWrite(ledPin, LOW);
    digitalWrite(heaterPin, LOW);
}

void loop() {
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();

        if (command == "a") {
            digitalWrite(ledPin, HIGH);  
        }
        else if (command == "b") {
            digitalWrite(ledPin, LOW);   
        }
        else if (command == "c") {
            heaterActive = true;  
        }
        else if (command == "d") {
            heaterActive = false;  
            digitalWrite(heaterPin, LOW);
        }
    }

    // 현재 온도 출력 (온도 값만 표시)
    Serial.println(readTemperature());

    // 히터 자동 제어
    if (heaterActive) {
        if (readTemperature() < 60.0) {
            digitalWrite(heaterPin, HIGH);  
        } else {
            digitalWrite(heaterPin, HIGH);  
        }
    }

    delay(2000); // 2초마다 실행
}

// 온도 센서 값 읽기 함수
float readTemperature() {
    int tempValue = analogRead(tempSensorPin);  
    float voltage = tempValue * 5.0 / 1023.0;  
    float temperature = (voltage - 0.5) * 100;  
    return temperature;
}
