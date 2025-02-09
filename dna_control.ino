// 핀 정의
const int tempSensorPin = A0;  // 서미스터 핀
const int heaterPin = 9;       // 히터 제어 핀
const int ledPin = 10;         // LED 제어 핀

// 목표 온도와 히스테리시스
const float targetTemperature = 40.0; // 목표 온도
const float hysteresis = 2.0;         // 히스테리시스 값 (온도 차)

// 온도 읽기 함수
float readTemperature() {
  int analogValue = analogRead(tempSensorPin); // 서미스터로부터 아날로그 값 읽기
  float resistance = (1023.0 / analogValue - 1) * 100000; // 저항값 계산 (100K 기준)
  float temperature = 1 / (log(resistance / 100000) / 3950 + 1 / (25 + 273.15)) - 273.15; // 온도 계산
  return temperature;
}

void setup() {
  Serial.begin(9600); // 시리얼 통신 시작
  pinMode(heaterPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  // 초기 상태: 히터와 LED OFF
  digitalWrite(heaterPin, LOW);
  digitalWrite(ledPin, LOW);

  Serial.println("Arduino 초기화 완료");
}

void loop() {
  // 현재 온도 읽기
  float temperature = readTemperature();

  // 주기적으로 온도 데이터를 시리얼로 전송
  Serial.print("Temperature:");
  Serial.println(temperature);

  // 시리얼 명령 처리
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n'); // 시리얼에서 명령 읽기
    command.trim(); // 명령 문자열의 공백 제거

    if (command == "HEATER_ON") {
      // 히터 ON: 10분 동안 유지
      digitalWrite(heaterPin, HIGH); // 히터 켜기
      delay(10L * 60L * 1000L); // 10분 대기
      digitalWrite(heaterPin, LOW); // 히터 끄기
    } else if (command == "HEATER_OFF") {
      // 히터 OFF: 강제로 끄기
      digitalWrite(heaterPin, LOW);
    } else if (command == "LED_ON") {
      // LED ON: LED 켜기
      digitalWrite(ledPin, HIGH);
    } else if (command == "LED_OFF") {
      // LED OFF: LED 끄기
      digitalWrite(ledPin, LOW);
    }
  }

  delay(1000); // 1초 대기 (다음 루프 실행 전)
}
