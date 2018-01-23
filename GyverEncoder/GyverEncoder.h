#ifndef GyverRC_h
#define GyverRC_h
#include <Arduino.h>

class Encoder
{
  public:
    Encoder();
    init(uint8_t CLK, uint8_t DT, uint8_t SW);
	setCounters(uint8_t norm, uint8_t hold, uint8_t norm_step, uint8_t hold_step);
	tick();
	int getNorm();
	int getHold();
	
	boolean isTurn();	
	
	boolean isRight();
	boolean isLeft();	
	boolean isRightH();
	boolean isLeftH();
	
	boolean isPress();
	boolean isRelease();
    boolean isHolded();
	boolean isHold();
	
  private:
    byte _CLK, _DT, _SW;
	
    boolean DT_now, DT_last, SW_state, hold_flag, butt_flag, turn_flag;
	boolean isRight_f, isLeft_f, isRightH_f, isLeftH_f, isTurn_f;
	boolean isPress_f, isRelease_f, isHolded_f, isHold_f;
	
	int _norm, _hold;
	uint8_t _norm_step, _hold_step;
	
	unsigned long debounce_timer;
};
 
#endif