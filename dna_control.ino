// Arduino 코드 - HEATER_ON 동작 시 온도 유지

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
  float temperature = readTemperature();

  // HEATER_ON 동작 시 온도 유지
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "HEATER_ON") {
      if (temperature >= targetTemperature) {
        digitalWrite(heaterPin, LOW); // 히터 끄기
        Serial.println("Heater OFF: 목표 온도 도달");
      } else if (temperature <= targetTemperature - hysteresis) {
        digitalWrite(heaterPin, HIGH); // 히터 켜기
        Serial.println("Heater ON: 온도를 올리는 중...");
      }
    } else if (command == "HEATER_OFF") {
      digitalWrite(heaterPin, LOW); // 히터 강제 끄기
      Serial.println("HEATER_OFF_OK");
    } else {
      Serial.println("UNKNOWN_COMMAND");
    }
  }

  // 현재 상태 출력
  Serial.print("Current Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  delay(1000); // 1초마다 실행
}


void handleCommand(String cmd) {
  if (cmd == "LED_ON") {
    digitalWrite(ledPin, HIGH);
    Serial.println("LED_ON_OK");
  } else if (cmd == "LED_OFF") {
    digitalWrite(ledPin, LOW);
    Serial.println("LED_OFF_OK");
  } else if (cmd == "HEATER_ON") {
    digitalWrite(heaterPin, HIGH);
    heaterOn = true;
    Serial.println("HEATER_ON_OK");
  } else if (cmd == "HEATER_OFF") {
    digitalWrite(heaterPin, LOW);
    heaterOn = false;
    Serial.println("HEATER_OFF_OK");
  } else {
    Serial.println("UNKNOWN_COMMAND");
  }

  // 명령 처리 후 상태 초기화
  commandReceived = false;
}
