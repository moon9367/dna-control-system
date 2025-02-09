// Arduino 코드 - 온도 제어 및 PID 포함

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

// PID 제어 상수
const float Kp = 2.0;  // 비례 상수
const float Ki = 0.5;  // 적분 상수
const float Kd = 1.0;  // 미분 상수

// PID 변수
float integral = 0.0;
float lastError = 0.0;

// 히터 동작 상태
bool heaterOn = false;
unsigned long heaterStartTime = 0;

// 온도 읽기 함수
float readTemperature() {
  int analogValue = analogRead(tempSensorPin); // 아날로그 값 읽기
  float resistance = (1023.0 / analogValue - 1) * resistorValue; // 서미스터 저항 계산
  float temperature = 1 / (log(resistance / refResistance) / beta + 1 / (refTemp + 273.15)) - 273.15; // 온도 계산
  return temperature;
}

// PID 제어 함수
float calculatePID(float target, float current) {
  float error = target - current;
  integral += error;
  float derivative = error - lastError;
  lastError = error;

  // PID 출력 계산
  float output = Kp * error + Ki * integral + Kd * derivative;

  // 출력 값 제한 (0 ~ 255)
  if (output > 255) output = 255;
  if (output < 0) output = 0;

  return output;
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
  Serial.print("Current Temperature: ");
  Serial.println(temperature);

  // 명령 처리
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "HEATER_ON") {
      heaterOn = true;
      heaterStartTime = millis();
      Serial.println("Heater ON");
    } else if (command == "HEATER_OFF") {
      digitalWrite(heaterPin, LOW);
      heaterOn = false;
      Serial.println("Heater OFF");
    } else if (command == "LED_ON") {
      digitalWrite(ledPin, HIGH);
      Serial.println("LED_ON_OK");
    } else if (command == "LED_OFF") {
      digitalWrite(ledPin, LOW);
      Serial.println("LED_OFF_OK");
    } else {
      Serial.println("UNKNOWN_COMMAND");
    }
  }

  // PID 제어 동작
  if (heaterOn) {
    if (temperature >= targetTemperature + 10.0) {
      // 온도 초과 시 히터 강제 종료
      digitalWrite(heaterPin, LOW);
      heaterOn = false;
      Serial.println("Heater OFF: 온도 초과");
    } else if (millis() - heaterStartTime >= 10L * 60L * 1000L) {
      // 히터 10분 이상 동작 시 자동 종료
      digitalWrite(heaterPin, LOW);
      heaterOn = false;
      Serial.println("Heater OFF: 10분 동작 제한");
    } else {
      // PID 제어
      float pidOutput = calculatePID(targetTemperature, temperature);
      analogWrite(heaterPin, pidOutput);
    }
  }

  delay(1000); // 1초 간격
}
