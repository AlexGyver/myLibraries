/*
fft_codec.pde
guest openmusiclabs.com 8.18.12
example sketch for running an fft on data collected
with the codecshield.  this will send out 128 bins of
data over the serial port at 115.2kb.  there is a
pure data patch for visualizing the data.

note: be sure to download the latest AudioCodec library
if yours is older than 8.16.12.  there were modifications
made that allow for code outside of the interrupt.
*/

#define SAMPLE_RATE 44 // 44.1Khz
#define ADCS 0 // no ADCs are being used
#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft

// include necessary libraries
#include "FFT.h"
#include <Wire.h>
#include <SPI.h>
#include <AudioCodec.h>

// create data variables for audio transfer
int left_in = 0x0000;
int left_out = 0x0000;
int right_in = 0x0000;
int right_out = 0x0000;
unsigned int count = 0;
volatile byte flag = 1;

void setup() {
  Serial.begin(115200); // use serial port
  AudioCodec_init();
}

void loop() {
  while(1) { // reduces clock jitter
    while(flag); // wait for samples to be collected
    fft_window(); // window the data
    fft_reorder(); // reorder for fft input
    fft_run(); // process fft
    fft_mag_log(); // take output of fft
    Serial.write(255); // send out a start byte
    Serial.write(fft_log_out, FFT_N/2); // send out data bytes
    flag = 1; // tell the codec that processing is done
  }
}

// timer1 interrupt routine - data collected here
ISR(TIMER1_COMPA_vect) { // store registers (NAKED removed)

  // &'s are necessary on data_in variables
  AudioCodec_data(&left_in, &right_in, left_out, right_out);
  left_out = left_in; // pass audio through
  right_out = right_in;
  if (flag) { // check if the fft is ready for more data
    fft_input[count] = left_in; // put real data into even bins
    fft_input[count + 1] = 0; // put zeros in odd bins
    count += 2; // increment to next bin
    if (count >= FFT_N*2) { // check if all bins are full
      flag = 0; // tell the fft to start running
      count = 0; // reset the bin counter
    }
  }
}


