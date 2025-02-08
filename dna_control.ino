void setup() {
  pinMode(10, OUTPUT); // LED 핀 설정
}

void loop() {
  digitalWrite(10, HIGH); // LED 켜기
  delay(1000);            // 1초 대기
  digitalWrite(10, LOW);  // LED 끄기
  delay(1000);            // 1초 대기
}
