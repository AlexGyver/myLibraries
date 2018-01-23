uint8_t CLK = 4;
uint8_t DT = 3;
uint8_t SW = 2;

byte val1, val2;

#include "GyverEncoder.h"
Encoder encoder1;

void setup() {
  Serial.begin(9600);
  encoder1.init(CLK, DT, SW);
  
  // val1 - переменная изменения при повороте
  // val2 - переменная изменения при нажатии повороте
  // далее идут шаги изменения обеих переменных соответственно
  encoder1.setCounters(val1, val2, 1, 1);
}

void loop() {
	// обязательная функция отработки. Должна постоянно опрашиваться
  encoder1.tick();
  
  if (encoder1.isTurn()) {             // если был совершён поворот
    Serial.print(encoder1.getNorm());  // получить счётчик обычный
    Serial.print(" ");
    Serial.println(encoder1.getHold()); // получить счётчик при нажатой кнопке
  }
  if (encoder1.isRight()) Serial.println("Right");         // если был поворот
  if (encoder1.isLeft()) Serial.println("Left");
  if (encoder1.isRightH()) Serial.println("Right holded"); // если было удержание + поворот
  if (encoder1.isLeftH()) Serial.println("Left holded");
  if (encoder1.isPress()) Serial.println("Press");         // нажатие на кнопку (+ дебаунс)
  if (encoder1.isRelease()) Serial.println("Release");     // отпускание кнопки (+ дебаунс)
  if (encoder1.isHolded()) Serial.println("Holded");       // если была удержана и энк не поворачивался
  //if (encoder1.isHold()) Serial.println("Hold");         // возвращает состояние кнопки

}
