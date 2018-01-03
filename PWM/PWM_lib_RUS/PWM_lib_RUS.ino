
/*
Пример для мега и Уно.
позволяет менять диапазона частот:
 1 Гц - 2 мГц (2000000 Гц) на 16-битных таймеров 
31 Гц - 2 мГц (2000000 Гц) на  8-битных таймеров 
Стандартная частота ШИМ ардуины 500Гц.
pwmWriteHR() может использоваться пины: на Uno (9 и 10) на Мега (2,3, 5, 6, 7, 8, 11, 12, 44, 45, и 46)
*/
#include <PWM.h>
int led = 5; // пин, светодиод 
unsigned long frequency = 31; // частота  1 - 2000000 (Гц)
int brightness = 125;         // частота ШИМ (0-255)  
//или
//word brightnessHR = 32768;  // частота ШИМ (0-65536) is 32768.
void setup(){
InitTimersSafe();                         //инициализируем все таймеры, кроме 0,
//SetPinFrequencySafe(led, frequency);    //устанавливает частоту для указанного pin
SetPinFrequency(led, frequency);          //устанавливает частоту для указанного pin
}

void loop(){
  pwmWrite(led, brightness);     //0-255   используйте эту функцию вместо analogWrite 
//или
//pwmWriteHR(led, brightnessHR); //0-65536 используйте эту функцию вместо analogWrite     

}

