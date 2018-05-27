#include "GyverRC.h"
#include <Arduino.h>
#include <EEPROM.h> 

analogTX::analogTX(){}

int analogTX::start(int pin, int minTX, int maxTX, int trimL_pin, int trimR_pin, int expon, int deadzone)
{
	_pin = pin;
	_minTX=minTX;
	_maxTX=maxTX;
	_expon=expon;
	_deadzone=deadzone;	
	_center=EEPROM.read(_pin*3 - 1)*5;
	_min_pos=EEPROM.read(_pin*3 - 2)*5;
	_max_pos=EEPROM.read(_pin*3)*5;	
	_trimL_pin=trimL_pin;
	_trimR_pin=trimR_pin;
	pinMode(_trimL_pin, INPUT_PULLUP);
	pinMode(_trimR_pin, INPUT_PULLUP);
	_stick_center=analogRead(_pin);
}


int analogTX::TXread()
{
	if (digitalRead(_trimL_pin) == 0 && _trimL_flag == 0) 
	{
		_center-=5;
		_trimL_flag=1;
	}
	if (digitalRead(_trimL_pin) == 1 && _trimL_flag == 1) 
	{
		_trimL_flag=0;
		_EEPROM_flag = 1;
		_last_press = millis();
	}
	if (digitalRead(_trimR_pin) == 0 && _trimR_flag == 0) 
	{
		_center+=5;
		_trimR_flag=1;
	}
	if (digitalRead(_trimR_pin) == 1 && _trimR_flag == 1) 
	{
		_trimR_flag=0;
		_EEPROM_flag = 1;
		_last_press = millis();
	}
	
	if (millis() - _last_press > 7000 && _EEPROM_flag == 1) {
		EEPROM.write(_pin*3 - 1, _center/5);
		_EEPROM_flag = 0;		
	}
	
	if (_flag_cal == 1) 
	{
		EEPROM.write(_pin*3 - 2, _min_pos/5);
		EEPROM.write(_pin*3, _max_pos/5);
		_flag_cal = 0;
	}	

	_data = analogRead(_pin);
	if (_data < (_stick_center - _deadzone)){
		_data = _data + _deadzone;
	} else if (_data > (_stick_center + _deadzone)) {
		_data = _data - _deadzone;
	} else {
		_data = _stick_center;
	}
	_data = map(_data, _min_pos, _max_pos, _minTX, _maxTX) + _center;
	_data = constrain(_data, _minTX, _maxTX);
	return _data;
	
}

int analogTX::calibrate()
{
	if (_flag_cal == 0) 
	{
		_min_pos = 1023;
		_max_pos = 0;
		_flag_cal = 1;
	}
	int val = analogRead(_pin);
	if (val < _min_pos) _min_pos = val;
	if (val > _max_pos) _max_pos = val;	
}

digitalTX::digitalTX(int pin)
{	
	_pin = pin;
	pinMode(_pin, INPUT_PULLUP);
}

int digitalTX::TXstate()
{
	data = !digitalRead(_pin);
	return data;
}