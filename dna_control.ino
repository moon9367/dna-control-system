// Arduino 코드 - 온도 제어 및 명령 처리

// 핀 정의
const int tempSensorPin = A0;  // 서미스터 핀
const int heaterPin = 9;       // 히터 제어 핀
const int ledPin = 10;         // LED 제어 핀

// 서미스터 관련 상수
const int resistorValue = 100000; // 직렬 저항 값 (100K)
const float beta = 3950;          // 서미스터 베타 계수
const int refTemp = 25;           // 기준 온도 (섭씨)
const int refResistance = 100000; // 기준 저항 값 (100K)

// 목표 온도 설정
const float targetTemperature = 40.0; // 목표 온도 (°C)
const float hysteresis = 2.0;         // 히스테리시스 값 (°C)

// 온도 읽기 함수
float readTemperature() {
  int analogValue = analogRead(tempSensorPin);
  float resistance = (1023.0 / analogValue - 1) * resistorValue;
  float temperature = 1 / (log(resistance / refResistance) / beta + 1 / (refTemp + 273.15)) - 273.15;
  return temperature;
}

void setup() {
  Serial.begin(9600);

  pinMode(heaterPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  digitalWrite(heaterPin, LOW); // 히터 초기 상태: 꺼짐
  digitalWrite(ledPin, LOW);   // LED 초기 상태: 꺼짐

  Serial.println("Arduino 초기화 완료");
}

void loop() {
  // 온도 읽기
  float temperature = readTemperature();

  // 명령 수신 확인
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "HEATER_ON") {
      // 온도 제어 시작
      if (temperature >= 40.0) {
        digitalWrite(heaterPin, LOW); // 히터 끄기
        Serial.println("Heater OFF: 목표 온도 초과");
      } else if (temperature <= 40.0 - hysteresis) {
        digitalWrite(heaterPin, HIGH); // 히터 켜기
        Serial.println("Heater ON: 온도를 올리는 중...");
      }
    } else if (command == "HEATER_OFF") {
      digitalWrite(heaterPin, LOW);
      Serial.println("HEATER_OFF_OK");
    } else if (command == "LED_ON") {
      digitalWrite(ledPin, HIGH);
      delay(5000);
      digitalWrite(ledPin, LOW);
      Serial.println("LED_ON_OK");
    } else if (command == "LED_OFF") {
      digitalWrite(ledPin, LOW);
      Serial.println("LED_OFF_OK");
    } else {
      Serial.println("UNKNOWN_COMMAND");
    }
  }

  // 온도 출력
  Serial.print("Current Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  delay(3000); // 1초 간격으로 실행
}
