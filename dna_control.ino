// Arduino 코드 - 단순 LED 제어 (5초 후 자동 OFF)

// 핀 정의
const int ledPin = 10; // LED 제어 핀

void setup() {
  Serial.begin(9600);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // LED 초기 상태: 꺼짐

  Serial.println("Arduino 초기화 완료");
}

void loop() {
  // 명령 수신 확인
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "LED_ON") {
      digitalWrite(ledPin, HIGH);       // LED 켜기
      Serial.println("LED_ON_OK");

      delay(5000);                      // 5초 대기

      digitalWrite(ledPin, LOW);        // 5초 후 LED 끄기
      Serial.println("LED_OFF: 5초 후 자동 종료");
    } else {
      Serial.println("UNKNOWN_COMMAND");
    }
  }
}