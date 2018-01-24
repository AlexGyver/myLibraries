uint8_t CLK = 4;
uint8_t DT = 3;
uint8_t SW = 2;

byte val1, val2;

#include "GyverEncoder.h"
Encoder enc1;

void setup() {
  Serial.begin(9600);
  enc1.init(CLK, DT, SW);
  
  // val1 - переменная изменения при повороте
  // val2 - переменная изменения при нажатии повороте
  enc1.setCounterNorm(val1);
  enc1.setCounterHold(val2);
  
  // установка шага при повороте и при нажатом повороте
  enc1.setStepNorm(1);
  enc1.setStepHold(1);

  // пределы изменения в каждом режиме
  enc1.setLimitsNorm(0, 100);
  enc1.setLimitsHold(0, 100);
}

void loop() {
	// обязательная функция отработки. Должна постоянно опрашиваться
  enc1.tick();
  
  if (enc1.isTurn()) {             // если был совершён поворот
    Serial.print(enc1.getNorm());  // получить счётчик обычный
    Serial.print(" ");
    Serial.println(enc1.getHold()); // получить счётчик при нажатой кнопке
  }
  if (enc1.isRight()) Serial.println("Right");         // если был поворот
  if (enc1.isLeft()) Serial.println("Left");
  if (enc1.isRightH()) Serial.println("Right holded"); // если было удержание + поворот
  if (enc1.isLeftH()) Serial.println("Left holded");
  if (enc1.isPress()) Serial.println("Press");         // нажатие на кнопку (+ дебаунс)
  if (enc1.isRelease()) Serial.println("Release");     // отпускание кнопки (+ дебаунс)
  if (enc1.isHolded()) Serial.println("Holded");       // если была удержана и энк не поворачивался
  //if (enc1.isHold()) Serial.println("Hold");         // возвращает состояние кнопки

}
