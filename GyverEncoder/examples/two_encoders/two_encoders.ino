#include "GyverEncoder.h"
Encoder encoder1, encoder2;

byte val1, val2;

void setup() {
  Serial.begin(9600);
  // (CLK, DT, SW);
  encoder1.init(4, 3, 2);
  encoder2.init(7, 6, 5);
  
  // в таком виде энкодер будет работать только с поворотом, без нажатия
  encoder1.setCounters(val1, 1);
  encoder2.setCounters(val2, 1);
}

void loop() {
	// обязательная функция отработки. Должна постоянно опрашиваться
  encoder1.tick();
  encoder2.tick();
  
  if (encoder1.isTurn()) Serial.println(encoder1.getNorm());  // получить счётчик обычный
  if (encoder2.isTurn()) Serial.println(encoder2.getNorm());  // получить счётчик обычный  
}
