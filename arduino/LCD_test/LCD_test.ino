#include <LiquidCrystal.h>

// LCD connected to Mega pins: RS=22, E=23, D4=24, D5=25, D6=26, D7=27
LiquidCrystal lcd(22, 23, 24, 25, 26, 27);

void setup() {
  delay(500);           // give LCD time to power up
  lcd.begin(16, 2);     // 16 columns, 2 rows
  lcd.clear();

  lcd.setCursor(0, 0);  // row 1, first character
  lcd.print("Hello World!");

  lcd.setCursor(0, 1);  // row 2, first character
  lcd.print("LCD is working!");
}

void loop() {
  // nothing needed — text stays on screen
}
