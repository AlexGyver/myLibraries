#include "GyverEncoder.h"
Encoder enc1, enc2;

byte val1, val2;

void setup() {
  Serial.begin(9600);
  // (CLK, DT, SW);
  enc1.init(4, 3, 2);
  enc2.init(7, 6, 5);
  
  // установка начальной точки для поворота
  enc1.setCounterNorm(val1);
  enc2.setCounterNorm(val2);

  // установка шага при повороте
  enc1.setStepNorm(1);
  enc2.setStepNorm(5);

  // устанвока пределов
  enc1.setLimitsNorm(0, 10);
  enc2.setLimitsNorm(0, 100);
}

void loop() {
	// обязательная функция отработки. Должна постоянно опрашиваться
  enc1.tick();
  enc2.tick();
  
  if (enc1.isTurn()) Serial.println(enc1.getNorm());  // получить счётчик обычный
  if (enc2.isTurn()) Serial.println(enc2.getNorm());  // получить счётчик обычный  
}
