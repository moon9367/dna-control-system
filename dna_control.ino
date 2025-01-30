// 핀 정의
const int tempSensorPin = A0;   // 온도 센서 핀 (NTC Thermistor)
const int heaterPin = 9;        // 히터 핀
const int ledPin = 10;          // LED 핀

// 온도 센서 계산용 상수
const int resistorValue = 100000; // 저항 값 (100kΩ)
const float beta = 3950;         // NTC 열 저항 베타 값
const int refTemp = 25;          // 기준 온도 (25°C)
const int refResistance = 100000; // 기준 저항 값 (100kΩ)

// 기본 목표 온도 (Flask에서 변경 가능)
float targetTemperature = 60.0;

void setup() {
    Serial.begin(9600);
    pinMode(heaterPin, OUTPUT);
    pinMode(ledPin, OUTPUT);
}

// 문자열을 대문자로 변환하는 함수
String toUpperCase(String str) {
    str.toUpperCase();
    return str;
}

void loop() {
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        command = toUpperCase(command);  // 🚀 받은 명령어를 자동으로 대문자로 변환

        if (command.startsWith("SET_TEMP:")) {
            targetTemperature = command.substring(9).toFloat();
            Serial.print("SET_TEMP_OK:");
            Serial.println(targetTemperature);
        } 
        else if (command == "HEATER_ON") {
            digitalWrite(heaterPin, HIGH);
            Serial.println("HEATER_ON");
        } 
        else if (command == "HEATER_OFF") {
            digitalWrite(heaterPin, LOW);
            Serial.println("HEATER_OFF");
        } 
        else if (command == "LED_ON") {
            digitalWrite(ledPin, HIGH);
            Serial.println("LED_ON");
        } 
        else if (command == "LED_OFF") {
            digitalWrite(ledPin, LOW);
            Serial.println("LED_OFF");
        } 
        else if (command == "GET_TEMP") {
            float temperature = readTemperature();
            Serial.print("TEMP:");
            Serial.println(temperature);
            Serial.print("LED:");
            Serial.println(digitalRead(ledPin) == HIGH ? "ON" : "OFF");
            Serial.print("HEATER:");
            Serial.println(digitalRead(heaterPin) == HIGH ? "ON" : "OFF");
        }
    }

    // 🌡️ 현재 온도가 목표 온도보다 낮으면 히터 ON, 높으면 OFF
    float currentTemperature = readTemperature();
    if (currentTemperature < targetTemperature) {
        digitalWrite(heaterPin, HIGH);
        Serial.println("AUTO_HEATER_ON");
    } else {
        digitalWrite(heaterPin, LOW);
        Serial.println("AUTO_HEATER_OFF");
    }

    delay(1000);
}

// 온도 센서 값 읽기 (NTC Thermistor 공식 적용)
float readTemperature() {
    int analogValue = analogRead(tempSensorPin);
    float resistance = (1023.0 / analogValue - 1) * resistorValue;
    float temperature = 1 / (log(resistance / refResistance) / beta + 1 / (refTemp + 273.15)) - 273.15;
    return temperature;
}
