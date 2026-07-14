// Blinks the ESP32's onboard LED to confirm the board, drivers, and upload path all work.
// Most dev boards use GPIO2 for the onboard LED; change LED_PIN if yours differs.

const int LED_PIN = 2;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  Serial.println("LED on");
  delay(500);

  digitalWrite(LED_PIN, LOW);
  Serial.println("LED off");
  delay(500);
}
