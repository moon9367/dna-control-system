// 핀 정의
const int tempSensorPin = A0;   // 온도 센서 핀 (NTC Thermistor)
const int heaterPin = 9;        // 히터 핀
const int ledPin = 10;          // LED 핀

// 온도 센서 계산용 상수
const int resistorValue = 100000; // 저항 값 (100kΩ)
const float beta = 3950;         // NTC 열 저항 베타 값
const int refTemp = 25;          // 기준 온도 (25°C)
const int refResistance = 100000; // 기준 저항 값 (100kΩ)

// 기본 목표 온도
float targetTemperature = 60.0;

void setup() {
    Serial.begin(9600);
    pinMode(heaterPin, OUTPUT);
    pinMode(ledPin, OUTPUT);
}

// 문자열을 소문자로 변환하는 함수
String toLowerCase(String str) {
    str.toLowerCase();
    return str;
}

void loop() {
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        command = toLowerCase(command);

        if (command.startsWith("set_temp:")) {
            targetTemperature = command.substring(9).toFloat();
            Serial.print("set_temp_ok:");
            Serial.println(targetTemperature);
        } else if (command == "heater_on") {
            digitalWrite(heaterPin, HIGH);
            Serial.println("heater_on");
        } else if (command == "heater_off") {
            digitalWrite(heaterPin, LOW);
            Serial.println("heater_off");
        } else if (command == "led_on") {
            digitalWrite(ledPin, HIGH);
            Serial.println("led_on");
        } else if (command == "led_off") {
            digitalWrite(ledPin, LOW);
            Serial.println("led_off");
        } else if (command == "get_temp") {
            float temperature = readTemperature();
            Serial.print("temp:");
            Serial.println(temperature, 1);  // 소수점 1자리까지만 출력
            Serial.print("led:");
            Serial.println(digitalRead(ledPin) == HIGH ? "on" : "off");
            Serial.print("heater:");
            Serial.println(digitalRead(heaterPin) == HIGH ? "on" : "off");
        }
    }

    float currentTemperature = readTemperature();
    if (currentTemperature < targetTemperature) {
        digitalWrite(heaterPin, HIGH);
        Serial.println("auto_heater_on");
    } else {
        digitalWrite(heaterPin, LOW);
        Serial.println("auto_heater_off");
    }

    delay(1000);
}


// test
// 온도 센서 값 읽기 (NTC Thermistor 공식 적용)
float readTemperature() {
    int analogValue = analogRead(tempSensorPin);
    float resistance = (1023.0 / analogValue - 1) * resistorValue;
    float temperature = 1 / (log(resistance / refResistance) / beta + 1 / (refTemp + 273.15)) - 273.15;
    return temperature;
}
