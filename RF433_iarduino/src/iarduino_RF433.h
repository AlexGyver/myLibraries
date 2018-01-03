//	Библиотека для работы с радиопередатчиком FS1000A и(или) радиоприёмником MX-RM-5V, работающими на частоте 433,920 МГц: http://iarduino.ru/shop/Expansion-payments/besprovodnoy-peredatchik-priemnik-315-433mhz-dc5v.html
//  Версия: 1.0.0
//  Последнюю версию библиотеки Вы можете скачать по ссылке: http://iarduino.ru/file/280.html
//  Подробное описание функции бибилиотеки доступно по ссылке: https://lesson.iarduino.ru/page/urok-26-6-soedinyaem-dve-arduino-po-radiokanalu-433-mgc/
//  Библиотека является собственностью интернет магазина iarduino.ru и может свободно использоваться и распространяться!
//  При публикации устройств или скетчей с использованием данной библиотеки, как целиком, так и её частей,
//  в том числе и в некоммерческих целях, просим Вас опубликовать ссылку: http://iarduino.ru
//  Автор библиотеки: Панькин Павел sajaem@narod.ru
//  Если у Вас возникли технические вопросы, напишите нам: shop@iarduino.ru

#ifndef iarduino_RF433_h
#define iarduino_RF433_h

#if defined(ARDUINO) && (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

class iarduino_RF433{
	public:
	uint16_t i433_func_CRC16(const uint8_t*, uint8_t, uint8_t k=0); // Создаём циклически избыточный код	(указатель на массив, количество байт, [первый байт])
};

#endif