// 테스트 코드: 온도 데이터 출력
const int tempSensorPin = A0;
const int resistorValue = 100000; // 100K 저항
const float beta = 3950;
const int refTemp = 25;
const int refResistance = 100000;

float readTemperature() {
  int analogValue = analogRead(tempSensorPin);
  float resistance = (1023.0 / analogValue - 1) * resistorValue;
  float temperature = 1 / (log(resistance / refResistance) / beta + 1 / (refTemp + 273.15)) - 273.15;
  return temperature;
}

void setup() {
  Serial.begin(9600);
}

void loop() {
  float temperature = readTemperature();
  Serial.print("Temperature: ");
  Serial.println(temperature);
  delay(1000); // 1초 간격
}
