#include <DHT.h>

#define DHTPIN    8
#define DHTTYPE   DHT11
#define LED_PIN   9

// Adjust these to match your environment.
// At TEMP_MIN the LED is off. At TEMP_MAX the LED is at full brightness.
// Put your finger on the DHT11 to push the temperature up and watch the LED respond.
#define TEMP_MIN  20.0
#define TEMP_MAX  35.0

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  dht.begin();
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  delay(2000);

  float temperature = dht.readTemperature();

  if (isnan(temperature)) {
    analogWrite(LED_PIN, 0);
    Serial.println("Sensor error — check DHT11 wiring");
    return;
  }

  int brightness = (int)((temperature - TEMP_MIN) / (TEMP_MAX - TEMP_MIN) * 255.0);
  brightness = constrain(brightness, 0, 255);

  analogWrite(LED_PIN, brightness);

  Serial.print("Temp: ");
  Serial.print(temperature, 1);
  Serial.print(" C  |  Brightness: ");
  Serial.print(brightness);
  Serial.println(" / 255");
}
