#include <Arduino.h>

// 핀 정의
const int heaterPin = 9;       // PTC 히터 제어 핀 (MOSFET)
const int ledPin = 10;         // 파워 LED 제어 핀 (MOSFET)

bool heaterActive = false;  // 히터 동작 여부 저장

void setup() {
    Serial.begin(9600);  // 시리얼 통신 시작
    pinMode(ledPin, OUTPUT);
    pinMode(heaterPin, OUTPUT);
    digitalWrite(ledPin, HIGH); // 테스트 HIGH 상태 추후 변경
    digitalWrite(heaterPin, LOW);
}

void loop() {
    if (Serial.available()) {
        char command = Serial.read();
        
        if (command == 'a') {
            digitalWrite(ledPin, HIGH);
            Serial.println("LED ON OK");  // 명령어 수신 후 응답
        }
        else if (command == 'b') {
            digitalWrite(ledPin, LOW);
            Serial.println("LED OFF OK");  // 명령어 수신 후 응답
        }
        else if (command == 'c') {
            digitalWrite(heaterPin, HIGH);
            Serial.println("HEATER ON OK");  // 명령어 수신 후 응답
        }
        else if (command == 'd') {
            digitalWrite(heaterPin, LOW);
            Serial.println("HEATER OFF OK");  // 명령어 수신 후 응답
        }
    }

    // 🌡️ 온도 제어 비활성화
    // float currentTemperature = readTemperature();
    // Serial.print("Temperature: ");
    // Serial.println(currentTemperature);

    // 🔥 히터 자동 제어 비활성화
    // if (heaterActive) {
    //     if (readTemperature() < 60.0) {
    //         digitalWrite(heaterPin, HIGH);  // 목표 온도 미만이면 히터 ON
    //     } else {
    //         digitalWrite(heaterPin, HIGH);  // 목표 온도 도달 후에도 유지
    //     }
    // }

    delay(2000); // 2초마다 실행
}

// 📡 온도 센서 값 읽기 함수 비활성화
// float readTemperature() {
//     int tempValue = analogRead(A0);
//     float voltage = tempValue * 5.0 / 1023.0;
//     float temperature = (voltage - 0.5) * 100;
//     return temperature;
// }
