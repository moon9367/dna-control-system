// 핀 정의
const int tempSensorPin = A0;  // 서미스터 핀
const int heaterPin = 9;       // 히터 제어 핀
const int ledPin = 10;         // LED 제어 핀

// 서미스터 관련 상수
const int resistorValue = 100000; // 직렬 저항 값 (100K)
const float beta = 3950;          // 서미스터 베타 계수
const int refTemp = 25;           // 기준 온도 (섭씨)
const int refResistance = 100000; // 기준 저항 값 (100K)

// PID 제어 상수
const float targetTemperature = 40.0; // 목표 온도
const float hysteresis = 2.0;         // 히스테리시스 값
const float Kp = 2.0, Ki = 0.5, Kd = 1.0; // PID 상수

// PID 변수
float integral = 0.0;
float lastError = 0.0;

// 히터 활성화 상태
bool isHeaterOn = false;

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
      Serial.println("Heater turned ON");
    } else if (command == "HEATER_OFF") {
      digitalWrite(heaterPin, LOW); // 히터 끄기
      isHeaterOn = false;
      Serial.println("Heater turned OFF");
    } else if (command == "GET_TEMP") {
      Serial.print("Temperature: ");
      Serial.println(temperature);
    } else {
      Serial.println("Unknown command");
    }
  }

  // 히터 제어 로직
  if (isHeaterOn) {
    if (temperature >= targetTemperature + 10.0) {
      // 온도가 목표값 +10도 이상이면 강제 종료
      digitalWrite(heaterPin, LOW); // 히터 끄기
      isHeaterOn = false;
      Serial.println("Heater OFF: 온도 초과");
    } else if (temperature <= targetTemperature - hysteresis) {
      // PID 계산
      float error = targetTemperature - temperature;
      integral += error;
      float derivative = error - lastError;
      lastError = error;

      float pidOutput = Kp * error + Ki * integral + Kd * derivative;
      pidOutput = constrain(pidOutput, 0, 255); // PID 출력 제한

      analogWrite(heaterPin, pidOutput);
    } else {
      // 목표 온도에 도달했을 때 히터 끄기
      digitalWrite(heaterPin, LOW);
    }
  }

  delay(1000); // 1초 간격
}
