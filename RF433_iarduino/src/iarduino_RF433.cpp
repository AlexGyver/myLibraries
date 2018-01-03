#include "iarduino_RF433.h"
#include "iarduino_RF433_Receiver.h"
#include "iarduino_RF433_Transmitter.h"

			iarduino_RF433							i433CRC;
volatile	iarduino_RF433_Receiver_volatile_class	i433VC;

//		Обработчик прерываний
void	i433_func_IRQ(){
//			Определяем длительность и уровень полубита, после которого вызвано прерывание
			i433VC.i433_time_ThisIRQ		= micros();																											//	время текущего вызова по прерыванию
			i433VC.i433_time_Duration		= i433VC.i433_time_ThisIRQ - i433VC.i433_time_PrevIRQ;																//	длительность полубита до прерывания
			i433VC.i433_time_PrevIRQ		= i433VC.i433_time_ThisIRQ;																							//	время предыдущего прерывания
			i433VC.i433_bits_Value			= !digitalRead(i433VC.i433_pins_INPUT);																				//	уровень полубита до прерывания

//			Определяем кратность длительности импульса к длительности полубита
			if( i433VC.i433_time_Duration < i433VC.i433_time_RATE[1]-i433VC.i433_time_RATE[0])	{return						;}else								//	импульс меньше обычного полубита (игнорируем его)
			if( i433VC.i433_time_Duration < i433VC.i433_time_RATE[1]+i433VC.i433_time_RATE[0])	{i433VC.i433_time_Aliquot=1	;}else								//	импульс соответствует обычному полубиту
			if( i433VC.i433_time_Duration < i433VC.i433_time_RATE[2]+i433VC.i433_time_RATE[0])	{i433VC.i433_time_Aliquot=2	;}else								//	импульс соответствует удвоенному полубиту
			if( i433VC.i433_time_Duration < i433VC.i433_time_RATE[3]+i433VC.i433_time_RATE[0])	{i433VC.i433_time_Aliquot=3	;}else								//	импульс соответствует стартовому полубиту
			if( i433VC.i433_time_Duration < i433VC.i433_time_RATE[4]+i433VC.i433_time_RATE[0])	{i433VC.i433_time_Aliquot=4	;}else								//	импульс соответствует стартовому полубиту + первый полубит данных
																								{i433VC.i433_time_Aliquot=5	;}									//	импульс выходит за максимально допустимые рамки
//			Если получен первый стартовый полубит
			if(!i433VC.i433_flas_Start && i433VC.i433_bits_Value && i433VC.i433_time_Aliquot==3){																//	если сброшен флаг фиксации первого стартового полубита, уровень импульса равен 1, а его длительность кратна стартовому полубиту
				i433VC.i433_flas_Start=1;																														//	устанавливаем флаг фиксации первого стартового полубита
				i433VC.i433_flas_Data=0;																														//	сбрасываем флаг указывающий на необходимость записывать данные
				i433VC.i433_flas_Prev=0;																														//	сбрасываем флаг указывающий что ранее был принят полубит
				i433VC.i433_summ_Byte=0;																														//	сбрасываем количество полных принятых байт
				i433VC.i433_summ_Bits=0;																														//	сбрасываем количество принятых бит (без учёта приятых байтов)
				i433VC.i433_summ_ByteStated=0;																													//	сбрасываем количество заявленных байт в пакете
				return;
			}
//			Если получен второй стартовый полубит
			if( i433VC.i433_flas_Start && !i433VC.i433_bits_Value && !i433VC.i433_flas_Data && (i433VC.i433_time_Aliquot==3 || i433VC.i433_time_Aliquot==4)){	//	если установлен флаг фиксации первого стартового полубита, уровень импульса равен 0, его длительность кратна стартовому полубиту (с или без первого полубита данных) и сброшен флаг указывающий на необходимость записывать данные
				i433VC.i433_flas_Start=0;																														//	сбрасываем флаг фиксации первого стартового полубита
				i433VC.i433_flas_Data=1;																														//	устанавливаем флаг указывающий на необходимость записывать данные
				i433VC.i433_flas_Prev=0;																														//	сбрасываем флаг указывающий что ранее был принят полубит
				i433VC.i433_summ_Byte=0;																														//	сбрасываем количество полных принятых байт
				i433VC.i433_summ_Bits=0;																														//	сбрасываем количество принятых бит (без учёта приятых байтов)
				i433VC.i433_summ_ByteStated=0;																													//	сбрасываем количество заявленных байт в пакете
				if(i433VC.i433_time_Aliquot==3){return;}else{i433VC.i433_time_Aliquot=1;}																		//	если второй импульс стартового полибита содержит информационный полубит, то разрешаем записать его данные
			}
//			Если необходимо записывать данные
			if( i433VC.i433_flas_Data && (i433VC.i433_time_Aliquot==1 || i433VC.i433_time_Aliquot==2)){
				if(i433VC.i433_flas_Prev){																														//	если имеется ранее принятый полубит
					if(i433VC.i433_time_Aliquot==1){i433VC.i433_flas_Prev=0;}																					//	если принят один полубит, то сбрасываем флаг указывающий что ранее был принят полубит
//						Запись бита в массив i433_data_Byte
						if(i433VC.i433_summ_Byte<35){																											//	если не достигнут предел массива
							i433VC.i433_data_Byte[i433VC.i433_summ_Byte]&=~(1<<(7-i433VC.i433_summ_Bits));														//	сбрасываем бит в байте перед записью
							i433VC.i433_data_Byte[i433VC.i433_summ_Byte]|=i433VC.i433_bits_Prev<<(7-i433VC.i433_summ_Bits);										//	устанавливаем значение бита в байте
							i433VC.i433_summ_Bits++; if(i433VC.i433_summ_Bits>=8){i433VC.i433_summ_Bits=0; i433VC.i433_summ_Byte++;}							//	увеличиваем позицию для следующего бита в массиве
						}
					if(i433VC.i433_time_Aliquot==2){i433VC.i433_bits_Prev=i433VC.i433_bits_Value;}																//	если принят удвоенный полубит, то сохраняем его уровень как ранее принятый, при этом флаг ранее принятого полубита остаётся установленным
				}else{																																			//	если нет ранее принятого полубита
					i433VC.i433_flas_Prev=1;																													//	устанавливаем флаг наличия ранее принятого полубита
					i433VC.i433_bits_Prev=i433VC.i433_bits_Value;																								//	сохраняем текущее значение импульса, как значение ранее принятого полубита
					if(i433VC.i433_time_Aliquot==2){i433VC.i433_time_Aliquot=5;}																				//	если принят удвоенный полубит, то это ошибка
				}
			}
//			Определение количества заявленных байт в пакете и его соответствие открытой трубе
			if(i433VC.i433_summ_Byte==1 && i433VC.i433_summ_Bits==0){																							//	если полностью принят первый байт пакета
				if(!((1<<(i433VC.i433_data_Byte[0]>>5)) & i433VC.i433_flag_ADDR)){i433VC.i433_time_Aliquot=5;}													//	не совпал номер трубы, завершаем приём пакета
				i433VC.i433_summ_ByteStated=(i433VC.i433_data_Byte[0]&0x1F)+1+1+2;																				//	получаем количество заявленных байт в пакете (0 => 1байт + сам первый байт и два байта CRC)
			}
//			Сохранение данных массива i433_data_Byte в массив i433_data_ByteRead
				if(i433VC.i433_summ_Byte>0 && i433VC.i433_summ_Byte==i433VC.i433_summ_ByteStated){																//	если количество принятых байт соответствует количеству заявленных
				for(uint8_t i=0; i<i433VC.i433_summ_Byte; i++){i433VC.i433_data_ByteRead[i]=i433VC.i433_data_Byte[i];}											//	копируем данные из массива i433_data_Byte в массив i433_data_ByteRead
				i433VC.i433_data_Read=i433VC.i433_summ_Byte;																									//	устанавливаем количество данных доступных для чтения
				i433VC.i433_time_Aliquot=5;																														//	завершаем приём пакета
			}
//			Если импульс выходит за рамки длительности
			if( i433VC.i433_time_Aliquot==5){
				i433VC.i433_flas_Start=0;																														//	сбрасываем флаг фиксации первого стартового полубита
				i433VC.i433_flas_Data=0;																														//	сбрасываем флаг указывающий на необходимость записывать данные
				i433VC.i433_flas_Prev=0;																														//	сбрасываем флаг указывающий что ранее был принят полубит
				i433VC.i433_summ_Byte=0;																														//	сбрасываем количество полных принятых байт
				i433VC.i433_summ_Bits=0;																														//	сбрасываем количество принятых бит (без учёта приятых байтов)
				i433VC.i433_summ_ByteStated=0;																													//	сбрасываем количество заявленных байт в пакете
				return;
			}
}

//		Объявление экземпляра класса							([номер вывода])
		iarduino_RF433_Receiver	::iarduino_RF433_Receiver		(uint8_t i)			{if(digitalPinToInterrupt(i)!=NOT_AN_INTERRUPT){i433VC.i433_pins_INPUT=i; i433VC.i433_ints_IRQ=digitalPinToInterrupt(i);}}
		iarduino_RF433_Transmitter::iarduino_RF433_Transmitter	(uint8_t i)			{i433_pins_OUTPUT=i;}

//		Инициация работы приёмника/передатчика					(без параметров)
void	iarduino_RF433_Receiver	::begin							(uint16_t i)		{i433_time_RATE=(uint32_t) 500000/i; pinMode(i433VC.i433_pins_INPUT, INPUT); i433VC.i433_time_RATE[0]=i433_time_RATE/2; i433VC.i433_time_RATE[1]=i433_time_RATE*1; i433VC.i433_time_RATE[2]=i433_time_RATE*2; i433VC.i433_time_RATE[3]=i433_time_RATE*3; i433VC.i433_time_RATE[4]=i433_time_RATE*4;}
void	iarduino_RF433_Transmitter::begin						(uint16_t i)		{i433_time_RATE=(uint32_t) 500000/i; pinMode(i433_pins_OUTPUT, OUTPUT); digitalWrite(i433_pins_OUTPUT,LOW);}

//		Указываем скорость приёма/передачи данных				(скорость числом или константами: i433_5KBPS, i433_4KBPS, i433_3KBPS, i433_2KBPS, i433_1KBPS, i433_500BPS, i433_100BPS)
void	iarduino_RF433_Receiver	::setDataRate					(uint16_t i)		{begin(i);}																	//	Устанавливаем скорость приёма данных
void	iarduino_RF433_Transmitter::setDataRate					(uint16_t i)		{begin(i);}																	//	Устанавливаем скорость передачи данных

//		Открываем трубу для прослушивания/передачи				(номер трубы)
void	iarduino_RF433_Receiver	::openReadingPipe				(uint8_t i)			{if(i>7){i433VC.i433_flag_ADDR=0xFF;}else{i433VC.i433_flag_ADDR|=1<<i;}}	//	Устанавливаем флаги труб  для приёма данных
void	iarduino_RF433_Transmitter::openWritingPipe				(uint8_t i)			{if(i<8){i433_pipe_VALUE=i;}else{i433_pipe_VALUE=0;}}						//	Устанавливаем номер трубы для передачи данных

//		Закрываем трубу от прослушивания						(номер трубы)
void	iarduino_RF433_Receiver	::closeReadingPipe				(uint8_t i)			{if(i>7){i433VC.i433_flag_ADDR=0x00;}else{i433VC.i433_flag_ADDR&=~(1<<i);}}	//	Сбрасываем флаги труб

//		Включаем приемник, начинаем прослушивание				(без параметров)
void	iarduino_RF433_Receiver	::startListening				(void)				{
			#ifdef __SAM3X8E__ 
				attachInterrupt(i433VC.i433_pins_INPUT, i433_func_IRQ, CHANGE);																					//	Направляем обработку прерывания i433_pins_INPUT (при условии CHANGE) на функцию i433_func_IRQ
			#else              
				attachInterrupt(i433VC.i433_ints_IRQ,   i433_func_IRQ, CHANGE);																					//	Направляем обработку прерывания i433_ints_IRQ   (при условии CHANGE) на функцию i433_func_IRQ
			#endif
}

//		Выключаем приёмник, завершаем прослушивание				(без параметров)
void	iarduino_RF433_Receiver	::stopListening					(void)				{
			#ifdef __SAM3X8E__ 
				detachInterrupt(i433VC.i433_pins_INPUT);																										//	Завершаем обработку прерываний для i433_pins_INPUT
			#else              
				detachInterrupt(i433VC.i433_ints_IRQ  );																										//	Завершаем обработку прерываний для i433_ints_IRQ
			#endif
}

//		Проверка наличия принятых данных						(без параметров)
bool	iarduino_RF433_Receiver	::available						(void)				{return available(NULL);}

//		Проверка наличия принятых данных						(указатель на переменную для получения номера трубы по которой приняты данные)
bool	iarduino_RF433_Receiver	::available						(uint8_t* i)		{
			if(i433VC.i433_data_Read){																															//	Если есть доступные для чтения байты
				uint16_t CRC=i433CRC.i433_func_CRC16((uint8_t*)i433VC.i433_data_ByteRead,i433VC.i433_data_Read-2,1);											//	Рассчитываем значение CRC16 для информационных байт данных из принятого пакета (первый и два последних байта пакета, не являются информационными)
				if(uint8_t(CRC>>8)!=i433VC.i433_data_ByteRead[i433VC.i433_data_Read-2]){i433VC.i433_data_Read=0; return 0;}										//	Если выявлено несовпадение старшего байта CRC16, то сбрасываем количество байт доступных для чтения и возвращаем false
				if(uint8_t(CRC   )!=i433VC.i433_data_ByteRead[i433VC.i433_data_Read-1]){i433VC.i433_data_Read=0; return 0;}										//	Если выявлено несовпадение младшего байта CRC16, то сбрасываем количество байт доступных для чтения и возвращаем false
				*i=i433VC.i433_data_ByteRead[0]>>5;																												//	Передаём номер трубы по указателю
				return 1;																																		//	Возвращаем true
			}else{return 0;}																																	//	Возвращаем false
}

//		Выводим принятые данные по переданному указателю		(указатель, количество байт)
void	iarduino_RF433_Receiver	::read							(void* i, uint8_t j){
			i433VC.i433_data_Read-=3;
			if(j>i433VC.i433_data_Read){j=i433VC.i433_data_Read;}																								//	Если количество запрашиваемых байт больше полученных, то вернем исключительно полученные
			uint8_t* answer = reinterpret_cast<uint8_t*>(i);																									//	Преобразуем полученный указатель к типу uint8_t
			uint8_t k=0; while(k<j){*answer++ = i433VC.i433_data_ByteRead[k+1]; k++;}																			//	Передаем данные в цикле, разыменовывая указатель и увеличивая адрес на который он ссылкается
			i433VC.i433_data_Read=0;																															//	Сбрасываем количество байт доступных для чтения
}

//		Получаем по указателю данные для передачи				()
void	iarduino_RF433_Transmitter::write						(const void* i, uint8_t j){
			const uint8_t* transfer = reinterpret_cast<const uint8_t*>(i);																						//	Преобразуем полученный указатель к типу const uint8_t
			uint16_t CRC=i433CRC.i433_func_CRC16(transfer,j);																									//	Рассчитываем значение CRC16 для данных из передаваемого пакета
			i433_func_SendBit(1); i433_func_SendBit(1); i433_func_SendBit(1);																					//	Передаём три предстартовых бита
			i433_func_SendBit(1,3);																																//	Передаём один стартовый бит (период передачи увеличен в 3 раза)
			i433_func_SendByte( (i433_pipe_VALUE<<5) | (j-1) );																									//	Передаём первый байт (номер трубы и количество информационных байт в пакете)
			uint8_t k=0; while(k<j){i433_func_SendByte(*transfer++); k++;}																						//	Передаём информационные данные (младшим битом и байтом вперёд)
			i433_func_SendByte(CRC>>8); i433_func_SendByte(CRC);																								//	Передаём два байта CRC
			i433_func_SendBit(1); i433_func_SendBit(1);																											//	Передаём два стоповых бита
			delayMicroseconds(i433_time_RATE*6);																												//	Устанавливаем задержку длительностью в три бита (6 полубит)
}

//		Передача одного бита									(значение бита, [кратность периода])
void	iarduino_RF433_Transmitter::i433_func_SendBit			(bool i, uint8_t j){
			digitalWrite(i433_pins_OUTPUT, i); delayMicroseconds(i433_time_RATE*j);																				//	Устанавливаем уровень первого полубита на заданное время
			digitalWrite(i433_pins_OUTPUT,!i); delayMicroseconds(i433_time_RATE*j);																				//	Устанавливаем уровень второго полубита на заданное время
}

//		Передача одного байта									(значение байта)
void	iarduino_RF433_Transmitter::i433_func_SendByte			(uint8_t i)			{for(uint8_t j=0; j<8; j++){i433_func_SendBit((i>>(7-j))&1);}}

//		Создаём циклически избыточный код						(указатель, количество байт, [первый байт])
uint16_t iarduino_RF433::i433_func_CRC16						(const uint8_t *i, uint8_t j, uint8_t k){
		uint16_t c=0xFFFF;																																		//	предустанавливаем CRC в значение 0xFFFF
		for(uint8_t a=k; a<j; a++){ c^=i[a];																													//	проходим по элементам массива и выполняем операцию XOR между CRC и очередным байтом массива i
		for(uint8_t b=0; b<8; b++){ if(c & 0x01){c>>=1; c^=0xA001;}else{c>>=1;}																					//	проходим по битам байта       и сдвигаем значение CRC на 1 байт вправо, если младший (сдвинутый) байт был равен 1, то выполняем операцию XOR между CRC и полиномом 0xA001
		}} return c;
}
