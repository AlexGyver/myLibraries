/*
FFT for arduino
guest openmusiclabs.com 8.10.12
this is a speed optimized program
for calculating an N point FFT on a block of data
please read the read_me file for more info

10.26.12 - guest - fixed clobbers so registers are stored
7.7.14 - guest - fixed progmem to work with new avr-gcc (thanks to forum user kirill9617)
11.15.16 - guest - fixed progmem once again to work with another f**king update to avrgcc (added __attribute__((used))).  also fixed clobber lists to eliminate non-clobbered registers.  also fixed STRINGIFY error by adding some spaces.  
*/

#ifndef _fft_h // include guard
#define _fft_h

#define STRINGIFY_(a) #a
#define STRINGIFY(a) STRINGIFY_(a)

#ifndef FFT_N // number of samples
  #define FFT_N 256
#endif

#ifndef SCALE // scaling factor for lin8 output function
  #define SCALE 1
#endif

#ifndef WINDOW // wether using the window function or not
  #define WINDOW 1
#endif

#ifndef OCT_NORM // wether using the octave normilization
  #define OCT_NORM 1
#endif

#ifndef REORDER // wether using the reorder function or not
  #define REORDER 1
#endif

#ifndef LOG_OUT // wether using the log output function or not
  #define LOG_OUT 0
#endif

#ifndef LIN_OUT // wether using the linear output function or not
  #define LIN_OUT 0
#endif

#ifndef LIN_OUT8 // wether using the linear output function or not
  #define LIN_OUT8 0
#endif

#ifndef OCTAVE // wether using the octave output function or not
  #define OCTAVE 0
#endif

#if FFT_N == 256
  #define LOG_N 8
  #define _R_V 8 // reorder value - used for reorder list
#elif  FFT_N == 128
  #define LOG_N 7
  #define _R_V 8
#elif FFT_N == 64
  #define LOG_N 6
  #define _R_V 4
#elif FFT_N == 32
  #define LOG_N 5
  #define _R_V 4
#elif FFT_N == 16
  #define LOG_N 4
  #define _R_V 2
#else
  #error FFT_N value not defined
#endif

#include <avr/pgmspace.h>

extern const int16_t __attribute__((used)) _wk_constants[] PROGMEM = {
#if (FFT_N ==  256)
  #include <wklookup_256.inc>
#elif (FFT_N ==  128)
  #include <wklookup_128.inc>
#elif (FFT_N ==  64)
  #include <wklookup_64.inc>
#elif (FFT_N ==  32)
  #include <wklookup_32.inc>
#elif (FFT_N ==  16)
  #include <wklookup_16.inc>
#endif
};

#if (REORDER == 1)
  extern const uint8_t __attribute__((used)) _reorder_table[] PROGMEM = {
  #if (FFT_N == 256)
    #include <256_reorder.inc>
  #elif (FFT_N == 128)
    #include <128_reorder.inc>
  #elif (FFT_N == 64)
    #include <64_reorder.inc>
  #elif (FFT_N == 32)
    #include <32_reorder.inc>
  #elif (FFT_N == 16)
    #include <16_reorder.inc>
  #endif
  };
#endif

#if ((LOG_OUT == 1)||(OCTAVE == 1))
  extern const uint8_t __attribute__((used)) _log_table[] PROGMEM = {
    #include <decibel.inc>
  };
#endif

#if (LOG_OUT == 1)
  uint8_t __attribute__((used)) fft_log_out[(FFT_N/2)]; // fft log output magintude buffer
#endif

#if (LIN_OUT == 1)
  extern const uint8_t __attribute__((used)) _lin_table[] PROGMEM = {
    #include <sqrtlookup16.inc>
  };
  uint16_t __attribute__((used)) fft_lin_out[(FFT_N/2)]; // fft linear output magintude buffer
#endif

#if (LIN_OUT8 == 1)
  extern const uint8_t __attribute__((used)) _lin_table8[] PROGMEM = {
    #include <sqrtlookup8.inc>
  };
  uint8_t __attribute__((used)) fft_lin_out8[(FFT_N/2)]; // fft linear output magintude buffer
#endif

#if (OCTAVE == 1)
  uint8_t __attribute__((used)) fft_oct_out[(LOG_N)]; // fft octave output magintude buffer
#endif

#if (WINDOW == 1) // window functions are in 16b signed format
  extern const int16_t __attribute__((used)) _window_func[] PROGMEM = {
  #if (FFT_N ==  256)
    #include <hann_256.inc>
  #elif (FFT_N ==  128)
    #include <hann_128.inc>
  #elif (FFT_N ==  64)
    #include <hann_64.inc>
  #elif (FFT_N ==  32)
    #include <hann_32.inc>
  #elif (FFT_N ==  16)
    #include <hann_16.inc>
  #endif
  };
#endif


int __attribute__((used)) fft_input[(FFT_N*2)]; // fft input data buffer


static inline void fft_run(void) {
  // store registers so they dont get clobbered
  // avr-gcc requires r2:r17,r28:r29, and r1 cleared
  asm volatile (
  "push r2 \n"
  "push r3 \n"
  "push r4 \n"
  "push r5 \n"
  "push r6 \n"
  "push r7 \n"
  "push r8 \n"
  "push r9 \n"
  "push r10 \n"
  "push r11 \n"
  "push r12 \n"
  "push r13 \n"
  "push r14 \n"
  "push r15 \n"
  "push r16 \n"
  "push r17 \n"
  "push r28 \n"
  "push r29 \n"
  );


  // do first set of butterflies - all real, no multiplies
  // initialize
  asm volatile (
  "clr r15 \n" // clear the null register
  "ldi r16, " STRINGIFY(FFT_N/2) " \n" // prep loop counter
  "ldi r28, lo8(fft_input) \n" //set to beginning of data space
  "ldi r29, hi8(fft_input) \n"

  // run butterfly Wk = (1,0)
  "1: \n"
  "ld r2,y \n" // fetch top real
  "ldd r3,y+1 \n"
  "ldd r4,y+4 \n" // fetch bottom real
  "ldd r5,y+5 \n"
  "asr r3 \n" // divide by 2 to keep from overflowing
  "ror r2 \n"
  "movw r6,r2 \n" // make backup
  "asr r5 \n" // divide by 2 to keep from overflowing
  "ror r4 \n"
  "add r6,r4 \n" // add for top real
  "adc r7,r5 \n"
  "sub r2,r4 \n" // subtract for bottom real
  "sbc r3,r5 \n"
  "st y,r6 \n" // store top real
  "std y+1,r7 \n"
  "std y+4,r2 \n" // store bottom real
  "std y+5,r3 \n"
  "adiw r28,0x08 \n" // go to next butterfly
  "dec r16 \n" // check if at end of data space
  "brne 1b \n"
  );

  // do second set of butterflies - all real, no multiplies
  // initialize
  asm volatile (
  "ldi r16, " STRINGIFY(FFT_N/4) " \n" // prep loop counter
  "ldi r28, lo8(fft_input) \n" //set to beginning of data space
  "ldi r29, hi8(fft_input) \n"

  // first pass Wk = (1,0)
  "2: \n"
  "ld r2,y \n" // fetch top real
  "ldd r3,y+1 \n"
  "ldd r4,y+8 \n" // fetch bottom real
  "ldd r5,y+9 \n"
  "asr r3 \n" // divide by 2 to keep from overflowing
  "ror r2 \n"
  "movw r6,r2 \n" // make backup
  "asr r5 \n" // divide by 2 to keep from overflowing
  "ror r4 \n"
  "add r6,r4 \n" // add for top real
  "adc r7,r5 \n"
  "sub r2,r4 \n" // subtract for bottom real
  "sbc r3,r5 \n"
  "st y,r6 \n" // store top real
  "std y+1,r7 \n"
  "std y+8,r2 \n" // store bottom real
  "std y+9,r3 \n"

  // second pass Wk = (0,1)
  "ldd r2,y+4 \n" // fetch top real
  "ldd r3,y+5 \n"
  "ldd r4,y+12 \n" // fetch bottom real
  "ldd r5,y+13 \n"
  "asr r3 \n" // divide by 2 to keep from overflowing
  "ror r2 \n"
  "asr r5 \n" // divide by 2 to keep from overflowing
  "ror r4 \n"
  "std y+4,r2 \n" // store top real
  "std y+5,r3 \n"
  "std y+6,r4 \n" // store top img
  "std y+7,r5 \n"
  "std y+12,r2 \n" // store bottom real
  "std y+13,r3 \n"
  "neg r5 \n" // negate bottom img
  "neg r4 \n"
  "sbc r5,r15 \n"
  "std y+14,r4 \n" // store bottom img
  "std y+15,r5 \n"
  "adiw r28,0x10 \n" // go to next butterfly
  "dec r16 \n" // check if at end of data space
  "brne 2b \n"
  );

  //do third set of butterflies - half are all real
  // initialize
  asm volatile (
  "ldi r24, " STRINGIFY(FFT_N/8) " \n" // prep loop counter
  "ldi r28, lo8(fft_input) \n" //set to beginning of data space
  "ldi r29, hi8(fft_input) \n"
  "ldi r20,0x82 \n" // load multiply register with 0.707
  "ldi r21,0x5a \n"
  "ldi r22,0x7e \n" // load multiply register with -0.707
  "ldi r23,0xa5 \n"


  //first pass Wk = (1,0)
  "3: \n"
  "ld r2,y \n" // fetch top real
  "ldd r3,y+1 \n"
  "ldd r4,y+16 \n" // fetch bottom real
  "ldd r5,y+17 \n"
  "asr r3 \n" // divide by 2 to keep from overflowing
  "ror r2 \n"
  "movw r6,r2 \n" // make backup
  "asr r5 \n" // divide by 2 to keep from overflowing
  "ror r4 \n"
  "add r6,r4 \n" // add for top real
  "adc r7,r5 \n"
  "sub r2,r4 \n" // subtract for bottom real
  "sbc r3,r5 \n"
  "st y,r6 \n" // store top real
  "std y+1,r7 \n"
  "std y+16,r2 \n" // store bottom real
  "std y+17,r3 \n"

  // second pass is Wk = (0.7,0.7)
  // add before multiply to save a multiply
  "ldd r8,y+20 \n" // fetch real
  "ldd r9,y+21 \n"
  "ldd r18,y+22 \n" // fetch imaginary
  "ldd r19,y+23 \n"
  "asr r9 \n" // divide by 2 to prevent overflow
  "ror r8 \n"
  "asr r19 \n" // divide by 2 to prevent overflow
  "ror r18 \n"
  "movw r16,r8 \n" // move to temp register
  "sub r16,r18 \n" // create new real
  "sbc r17,r19 \n"
  "add r18,r8 \n" // create new img
  "adc r19,r9 \n"

  // process real*cos - use fmuls to keep scaling
  "fmuls r17,r21 \n"
  "movw r4,r0 \n"
  "fmul r16,r20 \n"
  "adc r4,r15 \n"
  "movw r2,r0 \n"
  "fmulsu r17,r20 \n"
  "sbc r5,r15 \n"
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"
  "fmulsu r21,r16 \n"
  "sbc r5,r15 \n"
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"

  // process img*sin - use fmuls to keep scaling
  "fmuls r19,r21 \n"
  "movw r6,r0 \n"
  "fmul r18,r20 \n"
  "adc r6,r15 \n"
  "movw r2,r0 \n"
  "fmulsu r19,r20 \n"
  "sbc r7,r15 \n"
  "add r3,r0 \n"
  "adc r6,r1 \n"
  "adc r7,r15 \n"
  "fmulsu r21,r18 \n"
  "sbc r7,r15 \n"
  "add r3,r0 \n"
  "adc r6,r1 \n"
  "adc r7,r15 \n"

  // fetch top half of butterfly
  "ldd r8,y+4 \n" // fetch real
  "ldd r9,y+5 \n"
  "ldd r16,y+6 \n" // fetch img
  "ldd r17,y+7 \n"
  "asr r9 \n" // divide by 2 to prevent overflow
  "ror r8 \n"
  "asr r17 \n" // divide by 2 to prevent overflow
  "ror r16 \n"
  "movw r2,r8 \n"

  // create new data
  "add r2,r4 \n" // sum reals for top half
  "adc r3,r5 \n"
  "sub r8,r4 \n" // subtract reals for bottom half
  "sbc r9,r5 \n"
  "movw r4,r16 \n" // backup img
  "add r4,r6 \n" // sum imgs for top half
  "adc r5,r7 \n"
  "sub r16,r6 \n"  // subtract imgs for bottom half
  "sbc r17,r7 \n"

  // restore data
  "std y+4,r2 \n" // top half real
  "std y+5,r3 \n"
  "std y+6,r4 \n" // top half imgs
  "std y+7,r5 \n"
  "std y+20,r8 \n" // bottom half real
  "std y+21,r9 \n"
  "std y+22,r16 \n" // bottom half imgs
  "std y+23,r17 \n"

  // third pass is Wk = (0,1)
  "ldd r2,y+8 \n" // fetch top real
  "ldd r3,y+9 \n"
  "ldd r4,y+24 \n" // fetch bottom real
  "ldd r5,y+25 \n"
  "asr r3 \n" // divide by 2 to keep from overflowing
  "ror r2 \n"
  "asr r5 \n" // divide by 2 to keep from overflowing
  "ror r4 \n"
  "std y+8,r2 \n" // store top real 
  "std y+9,r3 \n"
  "std y+10,r4 \n" // store top img
  "std y+11,r5 \n"
  "std y+24,r2 \n" // store bottom real
  "std y+25,r3 \n"
  "neg r5 \n" // negate bottom img
  "neg r4 \n"
  "sbc r5,r15 \n"
  "std y+26,r4 \n" // store bottom img
  "std y+27,r5 \n"

  // fourth pass is Wk = (-0.7,0.7)
  // add first to reduce the number of multiplies
  "ldd r18,y+28 \n" // fetch real
  "ldd r19,y+29 \n"
  "ldd r8,y+30 \n" // fetch imaginary
  "ldd r9,y+31 \n"
  "asr r9 \n" // divide by 2 to prevent overflow
  "ror r8 \n"
  "asr r19 \n" // divide by 2 to prevent overflow
  "ror r18 \n"
  "movw r16,r18 \n" // move to temp register
  "add r16,r8 \n" // create new real
  "adc r17,r9 \n"
  "sub r18,r8 \n" // create new img
  "sbc r19,r9 \n"

  // process real*cos - use fmuls to keep scaling
  "fmuls r17,r23 \n"
  "movw r4,r0 \n"
  "fmul r16,r22 \n"
  "adc r4,r15 \n"
  "movw r2,r0 \n"
  "fmulsu r17,r22 \n"
  "sbc r5,r15 \n"
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"
  "fmulsu r23,r16 \n"
  "sbc r5,r15 \n"
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"

  // process img*sin - use fmuls to keep scaling
  "fmuls r19,r21 \n"
  "movw r6,r0 \n"
  "fmul r18,r20 \n"
  "adc r6,r15 \n"
  "movw r2,r0 \n"
  "fmulsu r19,r20 \n"
  "sbc r7,r15 \n"
  "add r3,r0 \n"
  "adc r6,r1 \n"
  "adc r7,r15 \n"
  "fmulsu r21,r18 \n"
  "sbc r7,r15 \n"
  "add r3,r0 \n"
  "adc r6,r1 \n"
  "adc r7,r15 \n"

  // fetch top half of butterfly
  "ldd r8,y+12 \n" // fetch real
  "ldd r9,y+13 \n"
  "ldd r16,y+14 \n" // fetch img
  "ldd r17,y+15 \n"
  "asr r9 \n" // divide by 2 to prevent overflow
  "ror r8 \n"
  "asr r17 \n" // divide by 2 to prevent overflow
  "ror r16 \n"
  "movw r2,r8 \n"

  // create new data
  "add r2,r4 \n" // sum reals for top half
  "adc r3,r5 \n"
  "sub r8,r4 \n" // subtract reals for bottom half
  "sbc r9,r5 \n"
  "movw r4,r16 \n" // backup img
  "add r4,r6 \n" // sum imgs for top half
  "adc r5,r7 \n"
  "sub r16,r6 \n"  // subtract imgs for bottom half
  "sbc r17,r7 \n"

  // restore data
  "std y+12,r2 \n" // top half real
  "std y+13,r3 \n"
  "std y+14,r4 \n" // top half imgs
  "std y+15,r5 \n"
  "std y+28,r8 \n" // bottom half real
  "std y+29,r9 \n"
  "std y+30,r16 \n" // bottom half imgs
  "std y+31,r17 \n"

  "adiw r28,0x20 \n" // go to next butterfly
  "dec r24 \n" // check if at end of data space
  "breq 4f \n"
  "rjmp 3b \n"
  );

  // remainder of the butterflies (fourth and higher)
  // initialize
  asm volatile (
  "4: \n"
  "ldi r16, 0x20 \n" // prep outer loop counter
  "mov r12,r16 \n"
  "clr r13 \n"
  "ldi r16, hi8((fft_input + " STRINGIFY(FFT_N*4) ")) \n" // prep end of dataspace register
  "mov r10, r16 \n"
  "ldi r30, lo8(_wk_constants) \n" // initialize lookup table address
  "ldi r31, hi8(_wk_constants) \n"
  "ldi r16, 0x04 \n" // prep inner loop midpoint
  "mov r11,r16 \n"
  "lsl r16 \n" // load inner loop counter to twice midpoint
  "mov r14,r16 \n"

  // outer_loop - reset variables for next pass through the butterflies
  "5: \n"
  "ldi r26, lo8(fft_input) \n" //set top pointer to beginning of data space
  "ldi r27, hi8(fft_input) \n"
  "movw r28,r26 \n" // set bottom pointer to top
  "add r28,r12 \n" // add outer loop counter to the bottom pointer
  "adc r29,r13 \n"

  // inner_loop - increment through individual butterflies
  // first butterfly is Wk = (1,0)
  "6: \n"
  "ld r2,x+ \n" // fetch top real
  "ld r3,x \n"
  "ld r4,y \n" // fetch bottom real
  "ldd r5,y+1 \n"
  "asr r3 \n" // divide by 2 to keep from overflowing
  "ror r2 \n"
  "movw r6,r2 \n" // make backup
  "asr r5 \n" // divide by 2 to keep from overflowing
  "ror r4 \n"
  "add r6,r4 \n" // add for top real
  "adc r7,r5 \n"
  "sub r2,r4 \n" // subtract for bottom real
  "sbc r3,r5 \n"
  "st y,r2 \n" // store bottom real
  "std y+1,r3 \n"
  "st x,r7 \n" // store top real
  "st -x,r6 \n"
  "adiw r26,0x04 \n" // increment to next butterfly
  "adiw r28,0x04 \n"
  "dec r14 \n" // weve done the first one already
  );

  // actual butterfly core
  asm volatile (
  "7: \n"
  "ld r16,y \n" // fetch real
  "ldd r17,y+1 \n"
  "ldd r18,y+2 \n" // fetch imaginary
  "ldd r19,y+3 \n"
  "lpm r20,z+ \n" // fetch cosine
  "lpm r21,z+ \n"
  "lpm r22,z+ \n" // fetch sine
  "lpm r23,z+ \n"

   // process real*cos
  "muls r17,r21 \n"
  "movw r4,r0 \n"
  "mul r16,r20 \n"
  "movw r2,r0 \n"
  "mulsu r17,r20 \n"
  "sbc r5,r15 \n"
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"
  "mulsu r21,r16 \n"
  "sbc r5,r15 \n"
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"

  // process img*sin and accumulate (subtract)
  "muls r19,r23 \n"
  "movw r6,r0 \n"
  "mul r18,r22 \n"
  "sub r2,r0 \n"
  "sbc r3,r1 \n"
  "sbc r4,r6 \n"
  "sbc r5,r7 \n"
  "mulsu r19,r22 \n"
  "adc r5,r15 \n"
  "sub r3,r0 \n"
  "sbc r4,r1 \n"
  "sbc r5,r15 \n"
  "mulsu r23,r18 \n"
  "adc r5,r15 \n"
  "sub r3,r0 \n"
  "sbc r4,r1 \n"
  "sbc r5,r15 \n"

  // process reals
  "ld r8,x+ \n" // fetch top half of butterfly - real
  "ld r9,x+ \n"
  "asr r9 \n" // divide by 2 to prevent overflow
  "ror r8 \n"
  "movw r2,r8 \n"
  "add r8,r4 \n" // sum reals for top half
  "adc r9,r5 \n"
  "sub r2,r4 \n" // subtract reals for bottom half
  "sbc r3,r5 \n"
  "st y+,r2 \n" // restore bottom half real
  "st y+,r3 \n"

  // process img*cos
  "muls r19,r21 \n"
  "movw r4,r0 \n"
  "mul r18,r20 \n"
  "movw r2,r0 \n"
  "mulsu r19,r20 \n"
  "sbc r5,r15 \n"
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"
  "mulsu r21,r18 \n"
  "sbc r5,r15 \n"
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"

  // process real*sin and accumulate
  "muls r17,r23 \n"
  "movw r6,r0 \n"
  "mul r16,r22 \n"
  "add r2,r0 \n"
  "adc r3,r1 \n"
  "adc r4,r6 \n"
  "adc r5,r7 \n"
  "mulsu r17,r22 \n"
  "sbc r5,r15 \n"
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"
  "mulsu r23,r16 \n"
  "sbc r5,r15 \n"
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"

  // process imgs
  "ld r6,x+ \n" // fetch top half img
  "ld r7,x \n"
  "asr r7 \n" //  divide by 2 to prevent overflow
  "ror r6 \n"
  "movw r2,r6 \n"
  "add r2,r4 \n" // sum imgs for top half
  "adc r3,r5 \n"
  "sub r6,r4 \n" // subtract imgs for bottom half
  "sbc r7,r5 \n"
  "st x,r3 \n" // restore top half img
  "st -x,r2 \n"
  "st -x,r9 \n" // restore top half real
  "st -x,r8 \n"
  "st y+,r6 \n" // restore bottom half img
  "st y+,r7 \n"

  // check where we are in the process
  "adiw r26,0x04 \n" // increment to next butterfly (y,z already done above)
  "dec r14 \n" // check which butterfly were doing
  "breq 9f \n" // finish off if last one
  "cp r14,r11 \n" // check if middle one
  "breq 8f \n"
  "rjmp 7b \n" // go back and do it again if not done yet
  );

  // middle buttefly is wk = (0,1)
  asm volatile (
  "8: \n"
  "ld r2,x+ \n" // fetch top real
  "ld r3,x \n"
  "ld r4,y \n" // fetch bottom real
  "ldd r5,y+1 \n"
  "asr r3 \n" // divide by 2 to keep from overflowing
  "ror r2 \n"
  "asr r5 \n" // divide by 2 to keep from overflowing
  "ror r4 \n"
  "movw r6,r4 \n" // make backup
  "neg r7 \n" // negate bottom img
  "neg r6 \n"
  "sbc r7,r15 \n"
  "st y+,r2 \n" // store bottom real
  "st y+,r3 \n"
  "st y+,r6 \n" // store bottom img
  "st y+,r7 \n"
  "st x,r3 \n" // store top real
  "st -x,r2 \n"
  "adiw r26,0x02 \n"
  "st x+,r4 \n" // store top img
  "st x+,r5 \n"
  "dec r14 \n" // increment to next butterfly
  "rjmp 7b \n" // keep going
  );

  // reset for next pass
  asm volatile (
  "9: \n"
  "cpi r28, lo8(fft_input + " STRINGIFY(FFT_N*4) ") \n" // check if at end of dataspace
  "cpc r29, r10 \n"
  "brsh 10f \n"
  "movw r26,r28 \n" // bottom is now top
  "add r28,r12 \n" // bottom is incremented by outer loop count
  "adc r29,r13 \n"
  "mov r14,r11 \n" // reset inner loop counter
  "lsl r14 \n"
  "sub r30,r12 \n" // reset Wk lookup table pointer
  "sbc r31,r13 \n"
  "adiw r30,0x08 \n" // number of butterflies minus 2 for the ones not done
  "rjmp 6b \n" // keep going

  // inner_done - reset for next set of butteflies
  "10: \n"
  "sbrc r11, " STRINGIFY(LOG_N - 2) " \n" // check if finished with all butteflies
  "rjmp 11f \n"
  "lsl r11 \n" // multiply inner loop midpoint by 2
  "mov r14,r11 \n" // reset inner loop counter
  "lsl r14 \n"
  "lsl r12 \n" // multiply outer loop counter by 2
  "rol r13 \n"
  "rjmp 5b \n" // keep going
  "11: \n" // rest of code here
  : :
  : "r0", "r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25", "r26", "r27", "r30", "r31" // clobber list for whole thing
  );

  // restore registers
  asm volatile (
  "pop r29 \n"
  "pop r28 \n"
  "pop r17 \n"
  "pop r16 \n"
  "pop r15 \n"
  "pop r14 \n"
  "pop r13 \n"
  "pop r12 \n"
  "pop r11 \n"
  "pop r10 \n"
  "pop r9 \n"
  "pop r8 \n"
  "pop r7 \n"
  "pop r6 \n"
  "pop r5 \n"
  "pop r4 \n"
  "pop r3 \n"
  "pop r2 \n"
  "clr r1 \n" // reset the c compiler null register
  );
}

static inline void fft_reorder(void) {
  // store registers so they dont get clobbered
  // avr-gcc requires r2:r17,r28:r29, and r1 cleared
  asm volatile (
  "push r2 \n"
  "push r3 \n"
  "push r4 \n"
  "push r5 \n"
  "push r6 \n"
  "push r7 \n"
  "push r8 \n"
  "push r9 \n"
  "push r28 \n"
  "push r29 \n"
  );

  // move values to bit reversed locations
  asm volatile (
  "ldi r30, lo8(_reorder_table) \n" // initialize lookup table address
  "ldi r31, hi8(_reorder_table) \n"
  "ldi r20, " STRINGIFY((FFT_N/2) - _R_V) " \n" // set to first sample

  // get source sample
  "1: \n"
  "lpm r26,z+ \n" // fetch source address
  "clr r27 \n"
  "lsl r26 \n" // multiply offset by 4
  "rol r27 \n"
  "lsl r26 \n"
  "rol r27 \n"
  "subi r26, lo8(-(fft_input)) \n" // pointer to offset
  "sbci r27, hi8(-(fft_input)) \n"
  "ld r2,x+ \n" // fetch real
  "ld r3,x+ \n"
  "ld r4,x+ \n" // fetch img
  "ld r5,x \n"

  // find destination
  "lpm r28,z+ \n"
  "clr r29 \n"
  "lsl r28 \n" // multiply offset by 4
  "rol r29 \n"
  "lsl r28 \n"
  "rol r29 \n"
  "subi r28, lo8(-(fft_input)) \n" // add pointer to offset
  "sbci r29, hi8(-(fft_input)) \n"
  "ld r6,y+ \n" // fetch real
  "ld r7,y+ \n"
  "ld r8,y+ \n" // fetch img
  "ld r9,y \n"

  // swap source and destination samples
  "st x,r9 \n"
  "st -x,r8 \n"
  "st -x,r7 \n"
  "st -x,r6 \n"
  "st y,r5 \n"
  "st -y,r4 \n"
  "st -y,r3 \n"
  "st -y,r2 \n"

  // check if done
  "dec r20 \n" // go to next sample
  "brne 1b \n" // finish off if last sample
  : :
  : "r20", "r26", "r27", "r30", "r31" // clobber list
  );

  // restore registers
  asm volatile (
  "pop r29 \n"
  "pop r28 \n"
  "pop r9 \n"
  "pop r8 \n"
  "pop r7 \n"
  "pop r6 \n"
  "pop r5 \n"
  "pop r4 \n"
  "pop r3 \n"
  "pop r2 \n"
  );
}


static inline void fft_mag_log(void) {
  // store registers so they dont get clobbered
  // avr-gcc requires r2:r17,r28:r29, and r1 cleared
  asm volatile (
  "push r2 \n"
  "push r3 \n"
  "push r4 \n"
  "push r5 \n"
  "push r6 \n"
  "push r7 \n"
  "push r15 \n"
  "push r16 \n"
  "push r17 \n"
  "push r28 \n"
  "push r29 \n"
  );

  // this returns an 8b unsigned value which is 16*log2((img^2 + real^2)^0.5)
  asm volatile (
  "ldi r26, lo8(fft_input) \n" // set to beginning of data space
  "ldi r27, hi8(fft_input) \n"
  "ldi r28, lo8(fft_log_out) \n" // set to beginning of result space
  "ldi r29, hi8(fft_log_out) \n"
  "clr r15 \n" // clear null register
  "ldi r20, " STRINGIFY(FFT_N/2) " \n" // set loop counter

  "1: \n"
  "ld r16,x+ \n" // fetch real
  "ld r17,x+ \n"
  "ld r18,x+ \n" // fetch imaginary
  "ld r19,x+ \n"

  // process real^2
  "muls r17,r17 \n"
  "movw r4,r0 \n"
  "mul r16,r16 \n"
  "movw r2,r0 \n"
  "fmulsu r17,r16 \n" // automatically does x2
  "sbc r5,r15 \n"
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"

  // process img^2 and accumulate
  "muls r19,r19 \n"
  "movw r6,r0 \n"
  "mul r18,r18 \n"
  "add r2,r0 \n"
  "adc r3,r1 \n"
  "adc r4,r6 \n"
  "adc r5,r7 \n"
  "fmulsu r19,r18 \n" // automatically does x2
  "sbc r5,r15 \n"
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"

  // decibel of the square root via lookup table
  // scales the magnitude to an 8b value times an 8b exponent
  "clr r17 \n" // clear exponent register
  "tst r5 \n"
  "breq 3f \n"
  "ldi r17,0x0c \n"
  "mov r30,r5 \n"

  "2: \n"
  "cpi r30,0x40 \n"
  "brsh 8f \n"
  "lsl r4 \n"
  "rol r30 \n"
  "lsl r4 \n"
  "rol r30 \n"
  "dec r17 \n"
  "rjmp 2b \n"

  "3: \n"
  "tst r4 \n"
  "breq 5f \n"
  "ldi r17,0x08 \n"
  "mov r30,r4 \n"

  "4: \n"
  "cpi r30,0x40 \n"
  "brsh 8f \n"
  "lsl r3 \n"
  "rol r30 \n"
  "lsl r3 \n"
  "rol r30 \n"
  "dec r17 \n"
  "rjmp 4b \n"

  "5: \n"
  "tst r3 \n"
  "breq 7f \n"
  "ldi r17,0x04 \n"
  "mov r30,r3 \n"

  "6: \n"
  "cpi r30,0x40 \n"
  "brsh 8f \n"
  "lsl r2 \n"
  "rol r30 \n"
  "lsl r2 \n"
  "rol r30 \n"
  "dec r17 \n"
  "rjmp 6b \n"

  "7: \n"
  "mov r30,r2 \n"

  "8: \n"
  "clr r31 \n"
  "subi r30, lo8(-(_log_table)) \n" // add offset to lookup table pointer
  "sbci r31, hi8(-(_log_table)) \n"
  "lpm r16,z \n" // fetch log compressed square root
  "swap r17 \n"  // multiply exponent by 16
  "add r16,r17 \n" // add for final value
  "st y+,r16 \n" // store value
  "dec r20 \n" // check if all data processed
  "breq 9f \n"
  "rjmp 1b \n"
  "9: \n" // all done
  : :
  : "r0", "r26", "r27", "r30", "r31", "r18", "r19", "r20" // clobber list
  );

  // restore registers
  asm volatile (
  "pop r29 \n"
  "pop r28 \n"
  "pop r17 \n"
  "pop r16 \n"
  "pop r15 \n"
  "pop r7 \n"
  "pop r6 \n"
  "pop r5 \n"
  "pop r4 \n"
  "pop r3 \n"
  "pop r2 \n"
  "clr r1 \n" // reset the c compiler null register
  );
}

static inline void fft_mag_lin(void) {
  // store registers so they dont get clobbered
  // avr-gcc requires r2:r17,r28:r29, and r1 cleared
  asm volatile (
  "push r2 \n"
  "push r3 \n"
  "push r4 \n"
  "push r5 \n"
  "push r6 \n"
  "push r7 \n"
  "push r15 \n"
  "push r16 \n"
  "push r17 \n"
  "push r28 \n"
  "push r29 \n"
  );

  // this returns an 16b unsigned value which is 16*((img^2 + real^2)^0.5)
  asm volatile (
  "ldi r26, lo8(fft_input) \n" // set to beginning of data space
  "ldi r27, hi8(fft_input) \n"
  "ldi r28, lo8(fft_lin_out) \n" // set to beginning of result space
  "ldi r29, hi8(fft_lin_out) \n"
  "clr r15 \n" // clear null register
  "ldi r20, " STRINGIFY(FFT_N/2) " \n" // set loop counter

  "1: \n"
  "ld r16,x+ \n" // fetch real
  "ld r17,x+ \n"
  "ld r18,x+ \n" // fetch imaginary
  "ld r19,x+ \n"

  // process real^2
  "muls r17,r17 \n"
  "movw r4,r0 \n"
  "mul r16,r16 \n"
  "movw r2,r0 \n"
  "fmulsu r17,r16 \n" // automatically does x2
  "sbc r5,r15 \n"
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"

  // process img^2 and accumulate
  "muls r19,r19 \n"
  "movw r6,r0 \n"
  "mul r18,r18 \n"
  "add r2,r0 \n"
  "adc r3,r1 \n"
  "adc r4,r6 \n"
  "adc r5,r7 \n"
  "fmulsu r19,r18 \n" // automatically does x2
  "sbc r5,r15 \n"
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"

  // square root via lookup table
  // first scales the magnitude to a 16b value times an 8b exponent
  "clr r17 \n" // clear exponent register
  "tst r5 \n"
  "breq 3f \n"
  "ldi r17,0x08 \n"
  "movw r30,r4 \n"

  "2: \n"
  "cpi r31,0x40 \n"
  "brsh 6f \n" // all values already known to be > 0x40
  "lsl r3 \n"
  "rol r30 \n"
  "rol r31 \n"
  "lsl r3 \n"
  "rol r30 \n"
  "rol r31 \n"
  "dec r17 \n"
  "rjmp 6f \n"

  "3: \n"
  "tst r4 \n"
  "breq 5f \n"
  "ldi r17,0x04 \n"
  "mov r31,r4 \n"
  "mov r30,r3 \n"

  "4: \n"
  "cpi r31,0x40 \n"
  "brsh 6f \n" // all values already known to be > 0x40
  "lsl r2 \n"
  "rol r30 \n"
  "rol r31 \n"
  "lsl r2 \n"
  "rol r30 \n"
  "rol r31 \n"
  "dec r17 \n"
  "rjmp 4b \n"

  // find sqrt via lookup table
  "5: \n"
  "movw r30,r2 \n"
  "cpi r31,0x40 \n"
  "brsh 6f \n"
  "cpi r31,0x10 \n"
  "brsh 12f \n"
  "cpi r31,0x01 \n"
  "brlo 10f \n"
  "swap r31 \n"
  "swap r30 \n"
  "andi r30,0x0f \n"
  "or r30,r31 \n"
  "lsr r30 \n"
  "ldi r31,0x01 \n"
  "rjmp 10f \n"
 
  "6: \n"
  "mov r30,r31 \n"
  "ldi r31,0x02 \n"
  "rjmp 10f \n"

  "12: \n"
  "lsl r30 \n"
  "rol r31 \n"
  "mov r30,r31 \n"
  "ori r30,0x80 \n"
  "ldi r31,0x01 \n"

  "10: \n"
  "subi r30, lo8(-(_lin_table)) \n" // add offset to lookup table pointer
  "sbci r31, hi8(-(_lin_table)) \n"
  "lpm r16,z \n" // fetch square root
  "clr r18 \n"

  "7: \n" // multiply by exponent
  "tst r17 \n"
  "breq 8f \n" // skip if no exponent
  "13: \n"
  "lsl r16 \n"
  "rol r18 \n"
  "dec r17 \n"
  "brne 13b \n"

  "8: \n"
  "st y+,r16 \n" // store value
  "st y+,r18 \n"
  "dec r20 \n" // check if all data processed
  "breq 9f \n"
  "rjmp 1b \n"
  "9: \n" // all done
  : :
  : "r0", "r26", "r27", "r30", "r31", "r18", "r19", "r20" // clobber list
  );

  // restore registers
  asm volatile (
  "pop r29 \n"
  "pop r28 \n"
  "pop r17 \n"
  "pop r16 \n"
  "pop r15 \n"
  "pop r7 \n"
  "pop r6 \n"
  "pop r5 \n"
  "pop r4 \n"
  "pop r3 \n"
  "pop r2 \n"
  "clr r1 \n" // reset the c compiler null register
  );
}

static inline void fft_mag_lin8(void) {
  // store registers so they dont get clobbered
  // avr-gcc requires r2:r17,r28:r29, and r1 cleared
  asm volatile (
  "push r2 \n"
  "push r3 \n"
  "push r4 \n"
  "push r5 \n"
  "push r6 \n"
  "push r7 \n"
  "push r15 \n"
  "push r16 \n"
  "push r17 \n"
  "push r28 \n"
  "push r29 \n"
  );

  // this returns an 8b unsigned value which is (225/(181*256*256))*((img^2 + real^2)^0.5)
  asm volatile (
  "ldi r26, lo8(fft_input) \n" // set to beginning of data space
  "ldi r27, hi8(fft_input) \n"
  "ldi r28, lo8(fft_lin_out8) \n" // set to beginning of result space
  "ldi r29, hi8(fft_lin_out8) \n"
  "clr r15 \n" // clear null register
  "ldi r20, " STRINGIFY(FFT_N/2) " \n" // set loop counter

  "1: \n"
  "ld r16,x+ \n" // fetch real
  "ld r17,x+ \n"
  "ld r18,x+ \n" // fetch imaginary
  "ld r19,x+ \n"

  // process real^2
  "muls r17,r17 \n"
  "movw r4,r0 \n"
  "mul r16,r16 \n"
  "movw r2,r0 \n"
  "fmulsu r17,r16 \n" // automatically does x2
  "sbc r5,r15 \n"
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"

  // process img^2 and accumulate
  "muls r19,r19 \n"
  "movw r6,r0 \n"
  "mul r18,r18 \n"
  "add r2,r0 \n"
  "adc r3,r1 \n"
  "adc r4,r6 \n"
  "adc r5,r7 \n"
  "fmulsu r19,r18 \n" // automatically does x2
  "sbc r5,r15 \n"
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"

#if (SCALE == 1)
  "movw r30,r4 \n"
#elif (SCALE == 2)
  "lsl r3 \n"
  "rol r4 \n"
  "rol r5 \n"
  "movw r30,r4 \n"
#elif (SCALE == 4)
  "lsl r3 \n"
  "rol r4 \n"
  "rol r5 \n"
  "lsl r3 \n"
  "rol r4 \n"
  "rol r5 \n"
  "movw r30,r4 \n"
#elif (SCALE == 128)
  "lsr r5 \n"
  "ror r4 \n"
  "ror r3 \n"
  "mov r31,r4 \n"
  "mov r30,r3 \n"
#elif (SCALE == 256)
  "mov r30,r3 \n"
  "mov r31,r4 \n"
#else
  "ldi r18, " STRINGIFY(SCALE) " \n"
  "mul r5,r18 \n"
  "mov r31,r0 \n"
  "mul r3,r18 \n"
  "mov r30,r1 \n"
  "mul r4,r18 \n"
  "add r30,r0 \n"
  "adc r31,r1 \n"
#endif

  // square root via lookup table
  // scales the magnitude to an 8b value
  "cpi r31,0x10 \n"
  "brsh 2f \n"
  "cpi r31,0x01 \n"
  "brsh 3f \n"
  "rjmp 6f \n"

  "2: \n"
  "lsl r30 \n"
  "rol r31 \n"
  "mov r30,r31 \n"
  "ldi r31,0x01 \n"
  "subi r30,0x80 \n"
  "sbci r31,0xff \n"
  "rjmp 6f \n"

  "3: \n"
  "swap r30 \n"
  "swap r31 \n"
  "andi r30,0x0f \n"
  "or r30,r31 \n"
  "lsr r30 \n"
  "ldi r31,0x01 \n"

  "6: \n"
  "subi r30, lo8(-(_lin_table8)) \n" // add offset to lookup table pointer
  "sbci r31, hi8(-(_lin_table8)) \n"
  "lpm r16,z \n" // fetch log compressed square root
  "st y+,r16 \n" // store value
  "dec r20 \n" // check if all data processed
  "breq 9f \n"
  "rjmp 1b \n"
  "9: \n" // all done
  "clr r1 \n" // reset c compiler null register
  : :
  : "r0", "r26", "r27", "r30", "r31", "r18", "r19", "r20" // clobber list
  );

  // restore registers
  asm volatile (
  "pop r29 \n"
  "pop r28 \n"
  "pop r17 \n"
  "pop r16 \n"
  "pop r15 \n"
  "pop r7 \n"
  "pop r6 \n"
  "pop r5 \n"
  "pop r4 \n"
  "pop r3 \n"
  "pop r2 \n"
  "clr r1 \n" // reset the c compiler null register
  );
}

static inline void fft_window(void) {
  // store registers so they dont get clobbered
  // avr-gcc requires r2:r17,r28:r29, and r1 cleared
  asm volatile (
  "push r2 \n"
  "push r3 \n"
  "push r4 \n"
  "push r5 \n"
  "push r15 \n"
  "push r16 \n"
  "push r17 \n"
  "push r28 \n"
  "push r29 \n"
  );

  // this applies a window to the data for better frequency resolution
  asm volatile (
  "ldi r28, lo8(fft_input) \n" // set to beginning of data space
  "ldi r29, hi8(fft_input) \n"
  "ldi r30, lo8(_window_func) \n" // set to beginning of lookup table
  "ldi r31, hi8(_window_func) \n"
  "clr r15 \n" // prep null register
  "ldi r20, " STRINGIFY(((FFT_N)&(0xff))) " \n"

  "1: \n"
  "lpm r22,z+ \n" // fetch window value
  "lpm r23,z+ \n"
  "ld r16,y \n" // fetch data
  "ldd r17,y+1 \n"

  // multiply by window
  "fmuls r17,r23 \n"
  "movw r4,r0 \n"
  "fmul r16,r22 \n"
  "adc r4,r15 \n"
  "movw r2,r0 \n"
  "fmulsu r17,r22 \n"
  "sbc r5,r15 \n"
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"
  "fmulsu r23,r16 \n"
  "sbc r5,r15 \n"
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"

  "st y+,r4 \n" // restore data
  "st y+,r5 \n"
  "adiw r28,0x02 \n" // skip imgs
  "dec r20 \n" // check if done
  "brne 1b \n"
  : :
  : "r0", "r20", "r30", "r31", "r22", "r23" // clobber list
  );

  // restore registers
  asm volatile (
  "pop r29 \n"
  "pop r28 \n"
  "pop r17 \n"
  "pop r16 \n"
  "pop r15 \n"
  "pop r5 \n"
  "pop r4 \n"
  "pop r3 \n"
  "pop r2 \n"
  "clr r1 \n" // reset the c compiler null register
  );
}


static inline void fft_mag_octave(void) {
  // store registers so they dont get clobbered
  // avr-gcc requires r2:r17,r28:r29, and r1 cleared
  asm volatile (
  "push r2 \n"
  "push r3 \n"
  "push r4 \n"
  "push r5 \n"
  "push r6 \n"
  "push r7 \n"
  "push r8 \n"
  "push r9 \n"
  "push r15 \n"
  "push r16 \n"
  "push r17 \n"
  "push r28 \n"
  "push r29 \n"
  );

  // this returns the energy in the sum of bins within an octave (doubling of frequencies)
  asm volatile (
  "ldi r26, lo8(fft_input) \n" // set to beginning of data space
  "ldi r27, hi8(fft_input) \n"
  "ldi r28, lo8(fft_oct_out) \n" // set to beginning of result space
  "ldi r29, hi8(fft_oct_out) \n"
  "clr r15 \n" // clear null register
  "ldi r20, 0x02 \n" // set first bin check (needed to make sequence 1-1-2-4-etc)
  "ldi r21, 0x01 \n" // set loop counter

  "13: \n"
  "lsr r20 \n"

  "10: \n"
  "mov r22,r21 \n" // make backup of counter for usage
  "clr r2 \n" // clear the accumulator
  "clr r3 \n"
  "movw r4,r2 \n"
  "clr r6 \n"

  "1: \n"
  "ld r16,x+ \n" // fetch real
  "ld r17,x+ \n"
  "ld r18,x+ \n" // fetch imaginary
  "ld r19,x+ \n"

  // process real^2
  "muls r17,r17 \n"
  "movw r8,r0 \n" // dont need an sbc as the result is always positive
  "mul r16,r16 \n"
  "add r2,r0 \n"
  "adc r3,r1 \n"
  "adc r4,r8 \n"
  "adc r5,r9 \n"
  "adc r6,r15 \n"
  "fmulsu r17,r16 \n" // automatically does x2
  "sbc r5,r15 \n"
  "sbc r6,r15 \n" // need to carry, might overflow if r5 = 0
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"
  "adc r6,r15 \n"

  // process img^2 and accumulate
  "muls r19,r19 \n"
  "movw r8,r0 \n" // dont need an sbc as the result is always positive
  "mul r18,r18 \n"
  "add r2,r0 \n"
  "adc r3,r1 \n"
  "adc r4,r8 \n"
  "adc r5,r9 \n"
  "adc r6,r15 \n"
  "fmulsu r19,r18 \n" // automatically does x2
  "sbc r5,r15 \n"
  "sbc r6,r15 \n" // need to carry, might overflow if r5 = 0
  "add r3,r0 \n"
  "adc r4,r1 \n"
  "adc r5,r15 \n"
  "adc r6,r15 \n"

  // check if summation done
  "dec r22 \n"
  "brne 1b \n"

#if (OCT_NORM == 1) // put normilisation code in if needed
  "mov r22,r21 \n"
  "lsr r22 \n" // check if done
  "brcs 12f \n"

  "11: \n"
  "lsr r6 \n"
  "ror r5 \n"
  "ror r4 \n"
  "ror r3 \n"
  "ror r2 \n"
  "lsr r22 \n" // check if done
  "brcc 11b \n"
#endif

  // decibel of the square root via lookup table
  // scales the magnitude to an 8b value times an 8b exponent
  "12: \n"
  "clr r17 \n" // clear exponent register
  "tst r5 \n"
  "breq 3f \n"
  "ldi r17,0x0c \n"
  "mov r30,r5 \n"

  "2: \n"
  "cpi r30,0x40 \n"
  "brsh 8f \n"
  "lsl r4 \n"
  "rol r30 \n"
  "lsl r4 \n"
  "rol r30 \n"
  "dec r17 \n"
  "rjmp 2b \n"

  "3: \n"
  "tst r4 \n"
  "breq 5f \n"
  "ldi r17,0x08 \n"
  "mov r30,r4 \n"

  "4: \n"
  "cpi r30,0x40 \n"
  "brsh 8f \n"
  "lsl r3 \n"
  "rol r30 \n"
  "lsl r3 \n"
  "rol r30 \n"
  "dec r17 \n"
  "rjmp 4b \n"

  "5: \n"
  "tst r3 \n"
  "breq 7f \n"
  "ldi r17,0x04 \n"
  "mov r30,r3 \n"

  "6: \n"
  "cpi r30,0x40 \n"
  "brsh 8f \n"
  "lsl r2 \n"
  "rol r30 \n"
  "lsl r2 \n"
  "rol r30 \n"
  "dec r17 \n"
  "rjmp 6b \n"

  "7: \n"
  "mov r30,r2 \n"

  "8: \n"
  "clr r31 \n"
  "subi r30, lo8(-(_log_table)) \n" // add offset to lookup table pointer
  "sbci r31, hi8(-(_log_table)) \n"
  "lpm r16,z \n" // fetch log compressed square root
  "swap r17 \n"  // multiply exponent by 16
  "add r16,r17 \n" // add for final value
  "st y+,r16 \n" // store value
  "sbrc r20, 0x00 \n" // check if first 2 bins done
  "rjmp 13b \n"
  "lsl r21 \n"
  "sbrs r21, " STRINGIFY((LOG_N) - 1) " \n" // check if done
  "rjmp 10b \n"
  : :
  : "r0", "r26", "r27", "r30", "r31", "r18", "r19", "r20", "r21", "r22" // clobber list
  );

  // restore registers
  asm volatile (
  "pop r29 \n"
  "pop r28 \n"
  "pop r17 \n"
  "pop r16 \n"
  "pop r15 \n"
  "pop r9 \n"
  "pop r8 \n"
  "pop r7 \n"
  "pop r6 \n"
  "pop r5 \n"
  "pop r4 \n"
  "pop r3 \n"
  "pop r2 \n"
  "clr r1 \n" // reset the c compiler null register
  );
}

#endif // end include guard

