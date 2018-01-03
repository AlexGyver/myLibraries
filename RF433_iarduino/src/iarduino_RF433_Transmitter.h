#ifndef iarduino_RF433_Transmitter_h
#define iarduino_RF433_Transmitter_h

#if defined(ARDUINO) && (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#define i433_5KBPS 5000
#define i433_4KBPS 4000
#define i433_3KBPS 3000
#define i433_2KBPS 2000
#define i433_1KBPS 1000
#define i433_500BPS 500
#define i433_100BPS 100

class iarduino_RF433_Transmitter{
	public:			iarduino_RF433_Transmitter	(uint8_t);						//	Объявление	экземпляра класса					(№ вывода, к которому подключён передатчик)
	/**	пользовательские функции **/
		void		begin						(uint16_t=i433_2KBPS);			//	Инициируем	работу с передатчиком				([скорость передачи данных])
		void		setDataRate					(uint16_t);						//	Указываем	скорость передачи данных			(i433_5KBPS, i433_4KBPS, i433_3KBPS, i433_2KBPS, i433_1KBPS, i433_500BPS, i433_100BPS)
		void		openWritingPipe				(uint8_t);						//	Открываем	трубу								(номер трубы)
		void		write						(const void*, uint8_t);			//	Передаём	данные из массива					(указатель на массив для передачи данных, длинна передаваемых данных)
	private:
	/**	внутренние переменные **/
		uint8_t		i433_pins_OUTPUT;											//	Вывод		к которому подключен передатчик
		uint16_t	i433_time_RATE;												//	Скорость	передачи данных						(количество микросекунд в 1 полубите)
		uint8_t		i433_pipe_VALUE;											//	Номер		трубы для передачи данных			(возможны значения от 0 до 7)
	/**	внутренние функции **/
		void		i433_func_SendBit			(bool, uint8_t=1);				//	Передача	одного бита							(значение передаваемого бита, [кратность периода передачи данных])
		void		i433_func_SendByte			(uint8_t);						//	Передача	одного байта						(значение передаваемого байта)
};

#endif