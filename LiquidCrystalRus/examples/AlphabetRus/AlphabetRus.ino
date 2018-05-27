#include <LiquidCrystalRus.h>

LiquidCrystalRus lcd(12, 11, 10, 5, 4, 3, 2);

void setup() {
}

void loop() {
  lcd.print("АБВГДЕЁЖЗИЙКЛМНО");
  delay(3000);
  lcd.clear();
  lcd.print("ПРСТУФХЦЧШЩЪЫЬЭЮ");
  delay(3000);
  lcd.clear();
  lcd.print("Я");
  delay(3000);
  lcd.clear();
  lcd.print("абвгдеёжзийклмно");
  delay(3000);
  lcd.clear();
  lcd.print("прстуфхцчшщъыьэю");
  delay(3000);
  lcd.clear();
  lcd.print("я");
  delay(3000);
  lcd.clear();  
}

