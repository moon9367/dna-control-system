// Arduino 코드 - PID 제어 및 강제 OFF 조건 강화

// 핀 정의
const int tempSensorPin = A0;  // 서미스터 핀
const int heaterPin = 9;       // 히터 제어 핀
const int ledPin = 10;         // LED 제어 핀

// 서미스터 관련 상수
const int resistorValue = 100000; // 직렬 저항 값 (100K)
const float beta = 3950;          // 서미스터 베타 계수
const int refTemp = 25;           // 기준 온도 (섭씨)
const int refResistance = 100000; // 기준 저항 값 (100K)

// PID 제어 관련 상수
const float targetTemperature = 40.0; // 목표 온도 (°C)
const float Kp = 1.5;                 // 비례 상수 (조정)
const float Ki = 0.3;                 // 적분 상수 (조정)
const float Kd = 0.5;                 // 미분 상수 (조정)

// 안전 제어 상수
const float maxTemperature = targetTemperature + 10.0; // 최대 허용 온도 (목표 온도 + 10°C)

// PID 제어 변수
float integral = 0;
float lastError = 0;
unsigned long lastTime = 0;

// 명령 처리 변수
String command = "";
bool commandReceived = false;

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

  Serial.println("Arduino PID 제어 초기화 완료");
  lastTime = millis();
}

void loop() {
  // 명령 수신 처리
  if (Serial.available() > 0) {
    command = Serial.readStringUntil('\n');
    command.trim();
    commandReceived = true;
    handleCommand(command);
  }

  // 명령이 없으면 PID 제어 수행
  if (!commandReceived) {
    float temperature = readTemperature();

    // 강제 히터 OFF 조건
    if (temperature > maxTemperature) {
      digitalWrite(heaterPin, LOW);
      Serial.println("Heater FORCED OFF: 초과 온도 감지");
      delay(1000); // 강제 OFF 후 1초 대기
      return;
    }

    unsigned long currentTime = millis();
    float elapsedTime = (currentTime - lastTime) / 1000.0; // 초 단위 시간 계산
    lastTime = currentTime;

    // PID 제어 계산
    float error = targetTemperature - temperature;
    integral += error * elapsedTime;
    float derivative = (error - lastError) / elapsedTime;
    float output = Kp * error + Ki * integral + Kd * derivative;

    // 히터 제어
    if (output > 0) {
      analogWrite(heaterPin, constrain(output, 0, 255)); // 히터 출력 제한
      Serial.println("Heater ON: PID 제어");
    } else {
      digitalWrite(heaterPin, LOW);
      Serial.println("Heater OFF: 목표 온도 도달");
    }

    lastError = error;

    // 현재 상태 출력
    Serial.print("Current Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
    Serial.print("PID Output: ");
    Serial.println(output);
  }

  delay(1000); // 1초 주기로 제어 수행
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
    Serial.println("HEATER_ON_OK");
  } else if (cmd == "HEATER_OFF") {
    digitalWrite(heaterPin, LOW);
    Serial.println("HEATER_OFF_OK");
  } else {
    Serial.println("UNKNOWN_COMMAND");
  }

  // 명령 처리 후 상태 초기화
  commandReceived = false;
}
