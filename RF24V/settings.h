#define SAMPLE_RATE 24000//16000, 24000
#define ANALOG_PIN A0
#define ENABLE_LED
#define TX_PIN A1
#define buffSize 32
#if defined(__AVR_ATmega1280__)||defined(__AVR_ATmega2560__)||(__AVR_ATmega32U4__)||(__AVR_AT90USB646__)||defined(__AVR_AT90USB1286__)||(__AVR_ATmega128__)||defined(__AVR_ATmega1281__)||defined(__AVR_ATmega2561__)
  #define rampMega
  #if !defined (speakerPin)
    #define speakerPin 11 
  #endif
  #if !defined (speakerPin2)
    #define speakerPin2 12
  #endif
  #if defined (ENABLE_LED)
    #define ledPin 13
  #endif
#else
  #if !defined (speakerPin)
  	#define speakerPin 9 
  #endif
  #if !defined (speakerPin2)
	#define speakerPin2 10
  #endif
  #if defined (ENABLE_LED)
  #endif
#endif
const uint64_t pipes[14] = { 0xABCDABCD71LL, 0x544d52687CLL, 0x544d526832LL, 0x544d52683CLL,0x544d526846LL, 0x544d526850LL,0x544d52685ALL, 0x544d526820LL, 0x544d52686ELL, 0x544d52684BLL, 0x544d526841LL, 0x544d526855LL,0x544d52685FLL,0x544d526869LL};
