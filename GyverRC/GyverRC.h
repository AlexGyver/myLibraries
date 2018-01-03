#ifndef GyverRC_h
#define GyverRC_h
#include <Arduino.h>
#include <EEPROM.h> 

class analogTX
{
  public:
  analogTX();
  int start(int pin, int minTX, int maxTX, int trimL_pin, int trimR_pin, int expon, int deadzone);
  int TXread();
  int calibrate();
  
  private:
    long _last_press;
	bool _EEPROM_flag;
	bool _trimL_flag;
	bool _trimR_flag;
	int _expon;
    int _center;
	int _trimL_pin;
	int _trimR_pin;
  	int _pin;
	int _min_pos;
	int _max_pos;
	int _minTX;
	int _maxTX;
	int _data;
	int _flag_cal = 0;
	int _deadzone=0;
	int _stick_center;
};

class digitalTX
{
  public:
  digitalTX(int pin);    
  int TXstate();
  int data;
  
  private:
  int _pin;
};

 
#endif