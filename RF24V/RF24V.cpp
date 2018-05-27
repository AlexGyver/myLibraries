#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif
#include <stddef.h>
#include "RF24V.h"
#include "RF24.h"
#include <settings.h>
volatile boolean buffEmpty[2] = {true,true}, whichBuff = false, a, lCntr=0, streaming = 0, transmitting = 0;
volatile byte buffCount = 0; volatile byte pauseCntr = 0; unsigned int intCount = 0; byte txCmd[2] = {'r','R'};
byte buffer[2][buffSize+1]; char volMod = -1; byte bitPos = 0, bytePos = 25; byte bytH; byte radioIdentifier;
#if defined (tenBit)
unsigned int sampl; byte bytL;
#endif
unsigned long volTime = 0; RF24 radi(0,0);
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || (__AVR_ATmega32U4__) || (__AVR_AT90USB646__) || defined(__AVR_AT90USB1286__) || (__AVR_ATmega128__) ||defined(__AVR_ATmega1281__)||defined(__AVR_ATmega2561__)
#define rampMega
#endif
const byte broadcastVal = 255; RF24V::RF24V(RF24& _radio, byte radioNum): radio(_radio){radi = radio;radioIdentifier = radioNum;}
void RF24V::begin(){radio.begin();
#if defined (ENABLE_LED)
#endif
pinMode(speakerPin,OUTPUT); 		pinMode(speakerPin2,OUTPUT);
pinMode(TX_PIN,INPUT_PULLUP);
if(SAMPLE_RATE < 16000){volMod = 3;}else{
#if !defined (tenBit)
volMod = 2;
#endif
}
radio.setAutoAck(0);radio.setCRCLength(RF24_CRC_8);
radio.openWritingPipe(pipes[0]);radio.openReadingPipe(1,pipes[1]);radio.openReadingPipe(2,pipes[radioIdentifier + 2]); 
radio.startListening();timerStart();RX();								   
#if !defined (MANUAL_BUTTON_HANDLING)
TIMSK0 |= _BV(OCIE0B);
#endif
}
void vol(bool upDn){
    if(upDn==1){ volMod++;}
    else{ volMod--; }}
void RF24V::volume(bool upDn){
	vol(upDn);}
void RF24V::setVolume(char vol) {
    volMod = vol - 4 ;}
void RF24V::timerStart(){ICR1 = 10 * (1600000/SAMPLE_RATE);TCCR1A = _BV(COM1A1) | _BV(COM1B0) | _BV(COM1B1);  
	TCCR1A |= _BV(WGM11);TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10);}
#if !defined (MANUAL_BUTTON_HANDLING)
void handleButtons(){boolean state = digitalRead(TX_PIN);
if(!state){
#if !defined (RX_ONLY)
if(!transmitting){transmitting = 1;TX();}
}else{
#endif
  if(transmitting){ RX(); transmitting = 0;}}}
#endif 
void rampDown(){
 int current = OCR1A;
 if(current > 0){
for(int i=0; i < ICR1; i++){
 #if defined(rampMega)
OCR1B = constrain((current + i),0,ICR1);OCR1A = constrain((current - i),0,ICR1);
#else
OCR1B = constrain((current - i),0,ICR1); OCR1A = constrain((current - i),0,ICR1);
#endif
delayMicroseconds(100);}}}
void rampUp(byte nextVal){
#if defined(rampMega)
unsigned int resolution = ICR1;OCR1A = 0; OCR1B = resolution;
for(int i=0; i < resolution; ++i){OCR1B = constrain(resolution-i,0,resolution);}
	#endif
	byte tmp = 200;
	unsigned int mod;
if(volMod > 0){ mod = OCR1A >> volMod; }else{ mod = OCR1A << (volMod*-1); }
if(tmp > mod){
for(unsigned int i=0; i<buffSize; i++){ mod = constrain(mod+1,mod, tmp); buffer[0][i] = mod; }
for(unsigned int i=0; i<buffSize; i++){ mod = constrain(mod+1,mod, tmp); buffer[1][i] = mod; }
}else{
for(unsigned int i=0; i<buffSize; i++){ mod = constrain(mod-1,tmp ,mod); buffer[0][i] = mod; }
for(unsigned int i=0; i<buffSize; i++){ mod = constrain(mod-1,tmp, mod); buffer[1][i] = mod; }}
whichBuff = 0; buffEmpty[0] = 0; buffEmpty[1] = 0; buffCount = 0;}
void RF24V::transfer(){TX();}
void RF24V::reader(){RX();}
#if !defined MANUAL_BUTTON_HANDLING					
ISR(TIMER0_COMPB_vect){handleButtons();}
#endif
uint64_t RF24V::getAddress(byte addressNo){
return pipes[addressNo];}
void handleRadio(){
 if(buffEmpty[!whichBuff] && streaming){           
if(radi.available() ){boolean n=!whichBuff; TIMSK1 &= ~_BV(ICIE1);sei();radi.read(&buffer[n],32); buffEmpty[n] = 0;pauseCntr = 0;                            
TIMSK1 |= _BV(ICIE1);						
}else{ pauseCntr++; }                       
 if(pauseCntr > 50){pauseCntr = 0;rampDown();streaming = 0; TIMSK1 &= ~(_BV(TOIE1) );			
#if defined (ENABLE_LED)
 TCCR0A &= ~_BV(COM0A1);			
#endif
TCCR1A &= ~(_BV(COM1A1) | _BV(COM1B1) | _BV(COM1B0)); }
}else
if(!streaming){                  					
 if(radi.available() ){TIMSK1 &= ~_BV(ICIE1);sei(); radi.read(&buffer[0],32);		
 switch(buffer[0][0]){          
 #if !defined (RX_ONLY)
case 'r': if(buffer[0][1] == 'R' && radioIdentifier < 2){ TX();}
 break;
 #endif
 default: streaming= 1; TCCR1A |= _BV(COM1A1) | _BV(COM1B1) | _BV(COM1B0);  
rampUp(buffer[0][31]);TIMSK1 |= _BV(TOIE1);			
#if defined (ENABLE_LED)
 TCCR0A |= _BV(COM0A1);			
#endif
break;}
TIMSK1 |= _BV(ICIE1);}}}
void RX(){TIMSK1 &= ~_BV(OCIE1B) | _BV(OCIE1A);ADCSRA = 0; ADCSRB = 0; buffEmpty[0] = 1; buffEmpty[1] = 1;    	
#if defined (oversampling)					
ICR1 = 10 * (800000/SAMPLE_RATE);    	
 #else
ICR1 = 10 * (1600000/SAMPLE_RATE);	
#endif
radi.openWritingPipe(pipes[0]);radi.openReadingPipe(1,pipes[1]); radi.startListening(); TIMSK1 = _BV(ICIE1);}
boolean nn = 0;volatile byte bufCtr = 0;volatile unsigned int visCtr = 0;
ISR(TIMER1_CAPT_vect){bufCtr++; visCtr++;
if(bufCtr >= 16){handleRadio();bufCtr = 0;
if(visCtr >= 32 && streaming){OCR0A = buffer[whichBuff][0] << 2;visCtr = 0;}}}
ISR(TIMER1_OVF_vect){
if(buffEmpty[whichBuff] ){ whichBuff=!whichBuff; }else{
#if defined (oversampling)
 if(lCntr){lCntr = !lCntr;return;}   lCntr=!lCntr;
#endif
#if !defined (tenBit)
if(volMod < 0 ){
OCR1A = OCR1B = (buffer[whichBuff][intCount] >> volMod*-1);
}else{
 OCR1A = OCR1B = buffer[whichBuff][intCount] << volMod;}
intCount++;
 if(intCount >= buffSize){intCount = 0; buffEmpty[whichBuff] = true;whichBuff = !whichBuff;}
	#else
sampl = buffer[whichBuff][intCount];bitWrite(  sampl, 8,  bitRead( buffer[whichBuff][bytePos],bitPos));bitPos++;
bitWrite(sampl, 9, bitRead(buffer[whichBuff][bytePos],bitPos));bitPos++;
if(volMod < 0 ){OCR1A = OCR1B = sampl >> (volMod*-1);
}else{
 OCR1A = OCR1B = sampl << volMod;}
if(bitPos >=8){bitPos = 0; bytePos = bytePos+1;}
intCount++;
 if(intCount >= 25){bytePos = 25; bitPos = 0; intCount = 0; buffEmpty[whichBuff] = true; whichBuff = !whichBuff;}
#endif
}}
#if !defined (RX_ONLY)
void RF24V::broadcast(byte radioID){
if(radioID == radioIdentifier){ return; }
noInterrupts();
if(radioID == broadcastVal){radio.openWritingPipe(pipes[1]);
}else{
radio.openWritingPipe(pipes[radioID + 2]);}
interrupts();}
ISR(TIMER1_COMPA_vect){
if(buffEmpty[!whichBuff] == 0){a = !whichBuff;
TIMSK1 &= ~(_BV(OCIE1A));sei();radi.writeFast(&buffer[a],32);buffEmpty[a] = 1;TIMSK1 |= _BV(OCIE1A);}}
ISR(TIMER1_COMPB_vect){
#if !defined (tenBit)
buffer[whichBuff][buffCount] = bytH = ADCH;
#if defined (speakerTX)
if(volMod < 0 ){  OCR1A = bytH >> (volMod*-1);
}else{ OCR1A = bytH << volMod;}
#endif
#else
 buffer[whichBuff][buffCount] = bytL = ADCL;bytH = ADCH;
bitWrite(buffer[whichBuff][bytePos],bitPos, bitRead(bytH,0));bitWrite(buffer[whichBuff][bytePos],bitPos+1, bitRead(bytH,1));bitPos+=2;
if(bitPos >= 8){ bitPos = 0; bytePos = bytePos+1; }
#if defined (speakerTX)
sampl = bytL;sampl |= bytH << 8;
if(volMod < 0 ){  OCR1A = sampl >> (volMod*-1);
}else{  	  OCR1A = sampl << volMod;}
 #endif
#endif
 buffCount++;
#if !defined (tenBit)
	if(buffCount >= 32){
 #else
    if(buffCount >= 25){bytePos = 25;bitPos = 0;
#endif
buffCount = 0;buffEmpty[!whichBuff] = 0;
whichBuff = !whichBuff;}}
void TX(){TIMSK1 &= ~(_BV(ICIE1) | _BV(TOIE1));
#if defined (ENABLE_LED)
TCCR0A &= ~_BV(COM0A1);
#endif
 radi.openWritingPipe(pipes[1]);radi.openReadingPipe(1,pipes[0]);radi.stopListening();
streaming = 0;buffCount = 0; buffEmpty[0] = 1; buffEmpty[1] = 1;byte pin = ANALOG_PIN;
	#if defined(analogPinToChannel)
	#if defined(__AVR_ATmega32U4__)
		if (pin >= 18) pin -= 18;
	#endif
pin = analogPinToChannel(pin);
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
if (pin >= 54) pin -= 54;
	#elif defined(__AVR_ATmega32U4__)
		if (pin >= 18) pin -= 18; 
	#elif defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega644__) || defined(__AVR_ATmega644A__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644PA__)
		if (pin >= 24) pin -= 24; 
	#else
		if (pin >= 14) pin -= 14;
	#endif
	#if defined(ADCSRB) && defined(MUX5)
ADCSRB = (ADCSRB & ~(1 << MUX5)) | (((pin >> 3) & 0x01) << MUX5);
	#endif
	#if defined(ADMUX)
ADMUX = (pin & 0x07) | _BV(REFS0);
	#endif
ICR1 = 10 * (1600000/SAMPLE_RATE);
 #if !defined (speakerTX)
rampDown();TCCR1A &= ~(_BV(COM1A1));
 #endif
TCCR1A &= ~(_BV(COM1B1) |_BV(COM1B0) );
 #if !defined (tenBit)
ADMUX |= _BV(ADLAR);
#else
ADMUX &= ~_BV(ADLAR);
#endif
ADCSRB |= _BV(ADTS0) | _BV(ADTS0) | _BV(ADTS2);byte prescaleByte = 0;
if( SAMPLE_RATE < 8900){  prescaleByte = B00000111;} 
else if( SAMPLE_RATE < 18000){ prescaleByte = B00000110;} 
else if( SAMPLE_RATE < 27000){ prescaleByte = B00000101;} 
else if( SAMPLE_RATE < 65000){ prescaleByte = B00000100;} 
else { prescaleByte = B00000011;} 
 ADCSRA = prescaleByte;ADCSRA |= _BV(ADEN) | _BV(ADATE);TIMSK1 = _BV(OCIE1B) | _BV(OCIE1A);}
#endif
