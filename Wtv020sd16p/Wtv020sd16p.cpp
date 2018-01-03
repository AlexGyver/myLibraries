/*
 Wtv020sd16p.cpp - Library to control a WTV020-SD-16P module to play voices from an Arduino board.
 Created by Diego J. Arevalo, August 6th, 2012.
 Released into the public domain.
 */

#include "Arduino.h"
#include "Wtv020sd16p.h"

const unsigned int PLAY_PAUSE = 0xFFFE;
const unsigned int STOP = 0xFFFF;
const unsigned int VOLUME_MIN = 0xFFF0;
const unsigned int VOLUME_MAX = 0xFFF7;

Wtv020sd16p::Wtv020sd16p(int resetPin,int clockPin,int dataPin,int busyPin)
{
  _resetPin=resetPin;
  _clockPin=clockPin;
  _dataPin=dataPin;
  _busyPin=busyPin;
  _busyPinState=HIGH;
  pinMode(_resetPin, OUTPUT);
  pinMode(_clockPin, OUTPUT);
  pinMode(_dataPin, OUTPUT);
  pinMode(_busyPin, INPUT);
}

void Wtv020sd16p::reset(){
  digitalWrite(_clockPin, LOW);
  digitalWrite(_resetPin, HIGH);
  //Reset pulse.
  digitalWrite(_resetPin, LOW);
  delay(5);
  digitalWrite(_resetPin, HIGH);
  //Reset idle to start bit. 
  digitalWrite(_clockPin, HIGH);
  delay(300);
}

void Wtv020sd16p::playVoice(int voiceNumber){  
  sendCommand(voiceNumber);
  _busyPinState=digitalRead(_busyPin);
  while(_busyPinState==HIGH){
    _busyPinState=digitalRead(_busyPin);
  }
}

void Wtv020sd16p::asyncPlayVoice(int voiceNumber){
  sendCommand(voiceNumber);
}

void Wtv020sd16p::stopVoice(){
  sendCommand(STOP);
}

void Wtv020sd16p::pauseVoice(){
  sendCommand(PLAY_PAUSE);
}

void Wtv020sd16p::mute(){
  sendCommand(VOLUME_MIN);
}

void Wtv020sd16p::unmute(){
  sendCommand(VOLUME_MAX);
}

void Wtv020sd16p::sendCommand(unsigned int command) {
  //Start bit Low level pulse.
  digitalWrite(_clockPin, LOW);
  delay(2);
  for (unsigned int mask = 0x8000; mask > 0; mask >>= 1) {
    //Clock low level pulse.
    digitalWrite(_clockPin, LOW);
    delayMicroseconds(50);
    //Write data setup.
    if (command & mask) {
      digitalWrite(_dataPin, HIGH);
    }
    else {
      digitalWrite(_dataPin, LOW);
    }
    //Write data hold.
    delayMicroseconds(50);
    //Clock high level pulse.
    digitalWrite(_clockPin, HIGH);
    delayMicroseconds(100);
    if (mask>0x0001){
      //Stop bit high level pulse.
      delay(2);      
    }
  }
  //Busy active high from last data bit latch.
  delay(20);
}
