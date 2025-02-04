// 핀 정의
const int tempSensorPin = A0;   // 서미스터 핀
const int heaterPin = 9;        // PTC 히터 제어 핀
const int ledPin = 10;          // LED 제어 핀
const int resistorValue = 100000; // 서미스터 직렬 저항
const float beta = 3950;        // 서미스터 베타 계수
const int refTemp = 25;         // 기준 온도
const int refResistance = 100000; // 서미스터 기준 저항

float targetTemperature = 60.0;

void setup() {
  Serial.begin(9600);
  pinMode(heaterPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  float temperature = readTemperature();

  // 현재 온도를 시리얼로 전송
  Serial.print("Temperature:");
  Serial.println(temperature);

  // 라즈베리파이로부터 명령 수신
  if (Serial.available()) {
    String command = Serial.readString();
    command.trim();
    
    if (command.startsWith("SET_TEMP:")) {
      targetTemperature = command.substring(9).toFloat();
      Serial.print("New target temperature: ");
      Serial.println(targetTemperature);
    } else if (command == "HEATER_ON") {
      digitalWrite(heaterPin, HIGH);
      Serial.println("Heater turned ON");
    } else if (command == "HEATER_OFF") {
      digitalWrite(heaterPin, LOW);
      Serial.println("Heater turned OFF");
    } else if (command == "LED_ON") {
      digitalWrite(ledPin, HIGH);
      Serial.println("LED turned ON");
    } else if (command == "LED_OFF") {
      digitalWrite(ledPin, LOW);
      Serial.println("LED turned OFF");
    }
  }

  delay(1000);
}

float readTemperature() {
  int analogValue = analogRead(tempSensorPin);
  float resistance = (1023.0 / analogValue - 1) * resistorValue;
  float temperature = 1 / (log(resistance / refResistance) / beta + 1 / (refTemp + 273.15)) - 273.15;
  return temperature;
}
