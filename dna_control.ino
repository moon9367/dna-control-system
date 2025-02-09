void setup() {
    pinMode(10, OUTPUT);
}

void loop() {
    digitalWrite(10, HIGH);  // LED 켜기
    delay(1000);             // 1초 대기
    digitalWrite(10, LOW);   // LED 끄기
    delay(1000);
}
