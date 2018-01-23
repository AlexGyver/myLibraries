#include "GyverEncoder.h"
#include <Arduino.h>

Encoder::Encoder(){}

Encoder::init(uint8_t CLK, uint8_t DT, uint8_t SW)
{
  _CLK = CLK;
  _DT = DT;
  _SW = SW;
  pinMode (_CLK, INPUT);
  pinMode (_DT, INPUT);
  pinMode (_SW, INPUT_PULLUP);
  DT_last = digitalRead(_CLK);         // читаем начальное положение CLK
}
Encoder::setCounters(uint8_t norm, uint8_t hold, uint8_t norm_step, uint8_t hold_step) {
	_norm = norm;
	_hold = hold;
	_norm_step = norm_step;
	_hold_step = hold_step;
}
Encoder::setCounters(uint8_t norm, uint8_t norm_step) {
	_norm = norm;
	_norm_step = norm_step;
}
int Encoder::getNorm() {
	return _norm;
}
int Encoder::getHold() {
	return _hold;
}
boolean Encoder::isTurn() {
	if (isTurn_f) {
		isTurn_f = false;
		return true;
	} else return false;
}
boolean Encoder::isRight() {
	if (isRight_f) {
		isRight_f = false;
		return true;
	} else return false;
}
boolean Encoder::isLeft() {
	if (isLeft_f) {
		isLeft_f = false;
		return true;
	} else return false;
}
boolean Encoder::isRightH() {
	if (isRightH_f) {
		isRightH_f = false;
		return true;
	} else return false;
}
boolean Encoder::isLeftH() {
	if (isLeftH_f) {
		isLeftH_f = false;
		return true;
	} else return false;
}
boolean Encoder::isPress() {
	if (isPress_f) {
		isPress_f = false;
		return true;
	} else return false;
}
boolean Encoder::isRelease() {
	if (isRelease_f) {
		isRelease_f = false;
		return true;
	} else return false;
}
boolean Encoder::isHolded() {
	if (isHolded_f) {
		isHolded_f = false;
		return true;
	} else return false;
}
boolean Encoder::isHold() {
	if (isHold_f) {
		isHold_f = false;
		return true;
	} else return false;
}

Encoder::tick() {	
  DT_now = digitalRead(_CLK);          // читаем текущее положение CLK
  SW_state = !digitalRead(_SW);        // читаем положение кнопки SW
  if (SW_state) isHold_f = true;
  else isHold_f = false;

  // отработка нажатия кнопки энкодера
  if (SW_state && !butt_flag && millis() - debounce_timer > 200) {
    hold_flag = false;
    butt_flag = true;
    turn_flag = false;
    debounce_timer = millis();
    isPress_f = true;
  }
  if (!SW_state && butt_flag && millis() - debounce_timer > 200 && millis() - debounce_timer < 500) {
    butt_flag = false;
    if (!turn_flag && !hold_flag) {  // если кнопка отпущена и ручка не поворачивалась
      turn_flag = false;
      isRelease_f = true;
    }
    debounce_timer = millis();
  }

  if (SW_state && butt_flag && millis() - debounce_timer > 800 && !hold_flag) {
    hold_flag = true;
    if (!turn_flag) {  // если кнопка отпущена и ручка не поворачивалась
      turn_flag = false;
      isHolded_f = true;
    }
  }
  if (!SW_state && butt_flag && hold_flag) {
    butt_flag = false;
    debounce_timer = millis();
  }

  if (DT_now != DT_last) {            // если предыдущее и текущее положение CLK разные, значит был поворот
    if (digitalRead(_DT) != DT_now) {  // если состояние DT отличается от CLK, значит крутим по часовой стрелке
      if (SW_state) {           // если кнопка энкодера нажата
        _hold += _hold_step;
		isRightH_f = true;
		isLeftH_f = false;
      } else {                  // если кнопка энкодера не нажата
        _norm += _norm_step;
        isRight_f = true;
		isLeft_f = false;
      }
    } else {                          // если совпадают, значит против часовой
      if (SW_state) {           // если кнопка энкодера нажата
        _hold -= _hold_step;
		isLeftH_f = true;
		isRightH_f = false;
      } else {                  // если кнопка энкодера не нажата
        _norm -= _norm_step;
        isLeft_f = true;
		isRight_f = false;
      }
    }
    turn_flag = true;                    // флаг что был поворот ручки энкодера
	isTurn_f = true;
  }
  DT_last = DT_now;                   // обновить значение для энкодера
}