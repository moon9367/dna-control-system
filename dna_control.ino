// 핀 정의
const int tempSensorPin = A0;  // 서미스터 핀
const int heaterPin = 9;       // 히터 제어 핀
const int ledPin = 10;         // LED 제어 핀

// 서미스터 관련 상수
const int resistorValue = 100000; // 직렬 저항 값 (100K)
const float beta = 3950;          // 서미스터 베타 계수
const int refTemp = 25;           // 기준 온도 (섭씨)
const int refResistance = 100000; // 기준 저항 값 (100K)

// 제어 상수
const float targetTemperature = 40.0; // 목표 온도
const float hysteresis = 2.0;         // 히스테리시스 값 (온도 차)
const unsigned long maxHeaterTime = 10L * 60L * 1000L; // 10분 (밀리초)

// 히터 상태
bool isHeaterOn = false;
unsigned long heaterStartTime = 0;

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

  digitalWrite(heaterPin, LOW);
  digitalWrite(ledPin, LOW);

  Serial.println("Arduino 초기화 완료");
}

void loop() {
  // 현재 온도 읽기
  float temperature = readTemperature();
  Serial.print("Temperature: ");
  Serial.println(temperature);

  // 시리얼 명령 처리
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "HEATER_ON") {
      isHeaterOn = true;
      heaterStartTime = millis();
      Serial.println("Heater ON: 제어 시작");
    } else if (command == "HEATER_OFF") {
      digitalWrite(heaterPin, LOW); // 히터 끄기
      isHeaterOn = false;
      Serial.println("Heater OFF: 제어 중지");
    } else if (command == "LED_ON") {
      digitalWrite(ledPin, HIGH); // LED 켜기
      Serial.println("LED ON");
    } else if (command == "LED_OFF") {
      digitalWrite(ledPin, LOW); // LED 끄기
      Serial.println("LED OFF");
    } else if (command == "GET_TEMP") {
      Serial.print("Temperature: ");
      Serial.println(temperature);
    } else {
      Serial.println("Unknown command");
    }
  }

  // 히터 제어 로직
  if (isHeaterOn) {
    unsigned long elapsedTime = millis() - heaterStartTime;

    if (elapsedTime >= maxHeaterTime) {
      // 히터 동작 시간이 10분을 초과하면 종료
      digitalWrite(heaterPin, LOW);
      isHeaterOn = false;
      Serial.println("Heater OFF: 10분 유지 후 종료");
    } else {
      // 히스테리시스 기반 온도 제어
      if (temperature >= targetTemperature) {
        digitalWrite(heaterPin, LOW); // 히터 끄기
        Serial.println("Heater OFF: 온도가 목표치 이상");
      } else if (temperature <= targetTemperature - hysteresis) {
        digitalWrite(heaterPin, HIGH); // 히터 켜기
        Serial.println("Heater ON: 온도가 목표치 이하");
      }
    }
  }

  delay(1000); // 1초 간격
}
