// 핀 정의
const int tempSensorPin = A0;   
const int heaterPin = 9;        
const int ledPin = 10;          
const int resistorValue = 100000; 
const float beta = 3950;        
const int refTemp = 25;         
const int refResistance = 100000; 

float targetTemperature = 60.0;

void setup() {
  Serial.begin(9600);
  pinMode(heaterPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readString();
    command.trim();
    
    if (command.startsWith("SET_TEMP:")) {
      targetTemperature = command.substring(9).toFloat();
      Serial.print("New target temperature: ");
      Serial.println(targetTemperature);
    } else if (command == "HEATER_ON") {
      digitalWrite(heaterPin, HIGH);
      Serial.println("HEATER_ON");
    } else if (command == "HEATER_OFF") {
      digitalWrite(heaterPin, LOW);
      Serial.println("HEATER_OFF");
    } else if (command == "LED_ON") {
      digitalWrite(ledPin, HIGH);
      Serial.println("LED_ON");
    } else if (command == "LED_OFF") {
      digitalWrite(ledPin, LOW);
      Serial.println("LED_OFF");
    } else if (command == "GET_TEMP") {
      float temperature = readTemperature();
      Serial.println(temperature);
      Serial.println(digitalRead(ledPin) == HIGH ? "LED_ON" : "LED_OFF");
      Serial.println(digitalRead(heaterPin) == HIGH ? "HEATER_ON" : "HEATER_OFF");
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
