/*
   Пример работы с дисплеем на микросхеме TM74HC595
   Динамическая индикация осуществляется "вручную" таймером на micros()
   AlexGyver Technologies http://alexgyver.ru/
*/

/*
  Основные методы библиотеки:
  disp.set(X, port);             // вывести 16 ричный код X в порт port (либо элемент массива SYM!!!)

  // вывести двухзначное число value на порт port (0 - первый и второй порт, 2 - 2 и 3, 3 - 3 и 4 порты))
  disp.digit2(value, port);

  disp.digit4(value);            // вывести число value
  disp.digit4showZero(value);    // вывести число value с нулями СЛЕВА

  disp.float_dot(value, pos);    // вывести float число value с точкой СПРАВА по счёту pos
  disp.int_dot(value, pos);      // вывести int число value с точкой СПРАВА по счёту pos

  disp.clear();                  // очистить дисплей
  *порты идут справа налево! (3 2 1 0)
*/

//---- ПИНЫ ----
byte SCLK = 4;
byte RCLK = 3;
byte DIO = 2;
//---- ПИНЫ ----

#include <TM74HC595Display.h>
#include <TimerOne.h>
TM74HC595Display disp(SCLK, RCLK, DIO);
unsigned char SYM[39];
unsigned long dispIsrTimer, countTimer;
float value = 0.0;

void setup() {
  symbols();               // объявление пачки символов для работы дисплея
}

void loop() {
  if (millis() - countTimer > 50) {   // каждые 50 миллисекунд ("прозрачный" аналог delay)
    value += 0.01;                    // прибавить 0.01
    disp.float_dot(value, 2);         // вывести на дисплей
    countTimer = millis();            // сбросить таймер
  }

  disp_isr();                         // динамическая индикация
}

void disp_isr() {
  if (micros() - dispIsrTimer > 300) {       // таймер динамической индикации (по-русски: КОСТЫЛЬ!)
    disp.timerIsr();                         // "пнуть" дисплей
    dispIsrTimer = micros();                 // сбросить таймер
  }
}

// символы для дисплея
void symbols() {
  // обычные
  SYM[0] = 0xC0; //0
  SYM[1] = 0xF9; //1
  SYM[2] = 0xA4; //2
  SYM[3] = 0xB0; //3
  SYM[4] = 0x99; //4
  SYM[5] = 0x92; //5
  SYM[6] = 0x82; //6
  SYM[7] = 0xF8; //7
  SYM[8] = 0x80; //8
  SYM[9] = 0x90; //9

  // с точкой
  SYM[10] = 0b01000000; //0.
  SYM[11] = 0b01111001; //1.
  SYM[12] = 0b00100100; //2.
  SYM[13] = 0b00110000; //3.
  SYM[14] = 0b00011001; //4.
  SYM[15] = 0b00010010; //5.
  SYM[16] = 0b00000010; //6.
  SYM[17] = 0b01111000; //7.
  SYM[18] = 0b00000000; //8.
  SYM[19] = 0b00010000; //9.

  // буквы
  SYM[20] = 0x88; //A
  SYM[21] = 0x83; //b
  SYM[22] = 0xC6; //C
  SYM[23] = 0xA1; //d
  SYM[24] = 0x86; //E
  SYM[25] = 0x8E; //F
  SYM[26] = 0xC2; //G
  SYM[27] = 0x89; //H
  SYM[28] = 0xF9; //I
  SYM[29] = 0xF1; //J
  SYM[30] = 0xC3; //L
  SYM[31] = 0xA9; //n
  SYM[32] = 0xC0; //O
  SYM[33] = 0x8C; //P
  SYM[34] = 0x98; //q
  SYM[35] = 0x92; //S
  SYM[36] = 0xC1; //U
  SYM[37] = 0x91; //Y
  SYM[38] = 0b11110111; // _
}
