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
    // 📌 시리얼 데이터 수신 및 명령 처리
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');  // 개행 문자까지 읽기
        command.trim();  // 개행 문자 및 공백 제거

        Serial.print("Received: ");
        Serial.println(command);

        if (command == "a") {
            digitalWrite(ledPin, HIGH);
            Serial.println("LED ON");
        } 
        else if (command == "b") {
            digitalWrite(ledPin, LOW);
            Serial.println("LED OFF");
        } 
        else if (command == "c") {
            heaterActive = true;
            Serial.println("Heater ON");
        } 
        else if (command == "d") {
            heaterActive = false;
            digitalWrite(heaterPin, LOW);
            Serial.println("Heater OFF");
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
