fft_read_me.txt
explanation of how to use the arduino fft libarary
guest openmusiclabs.com 8.17.12
guest openmusiclabs.com 10.26.12 -> fixed clobbers - registers werent being
stored during function calls.

this fft runs on 16b real inputs, and returns either 8b linear,
16b linear, or 8b logarithmic outputs.  it can handle an fft with anywhere
from 16 -> 256 samples, and gives back N/2 magnitudes. it is optmized
for speed, but still has a pretty good noise floor of around 12b, and an
SNR of around 10b.  it only operates on real data, and only returns the first
N/2 bins. the fft size is limited by the 2kB SRAM in the arduino.  the code
is written with this is mind, and would need a few variables to be transferred
from bytes to ints to make it work with larger ffts.

REFERENCES:

1. ELM-ChaN fft library:

http://elm-chan.org/docs/avrlib/avrfft.zip

a very good implementation that is more portable and can handle imaginary inputs,
larger fft sizes, and more accurate output. it is slower, and not quite as
arduino friendly.  be sure to check out the rest of the stuff on the site as
well, tons of great info.

http://elm-chan.org/cc_e.html

2. FFT tutorial from alwayslearn.com:

http://www.alwayslearn.com/dft%20and%20fft%20tutorial/DFTandFFT_BasicIdea.html

this site is great for breaking down how the fft works, into an easily understood
format.  highly reccomended if you want to understand the butterfly operations.

3. Katja's homepage on sinusoids:

http://www.katjaas.nl/home/home.html

this site is amazing. a must see!  take the tour, buy the t-shirt!  it goes through
and explains all sorts of crazy math things in a very fun, and excrutiatingly in-
depth fashion.  i laughed, i cried, i learned a lot.  thanks.

4. wikipedia article on window functions

http://en.wikipedia.org/wiki/Window_function

relatively good explanation of what window functions do, and why you need them.
i mostly used it for the nice graph of the relative attenuation of various
window functions. good for picking which one to use.


INDEX:
-----
0. resource usage
1. included files
2. function calls
3. example
4. #define explanations


0. resource usage 

A. SPEED CHARACTERISTICS:

--------------------------------------------------
Func: run  : reorder : window : lin : lin8 : log :
--------------------------------------------------
 N  | (ms) :   (us)  :  (us)  : (us): (us)*: (us):
--------------------------------------------------
256 : 6.32 :   412   :  608   : 588 : 470  : 608 :
128 : 2.59 :   193   :  304   : 286 : 234  : 290 :
64  : 1.02 :   97    :  152   : 145 : 114  : 144 :
32  : 0.37 :   41    :  76    : 80  : 59   : 74  :
16  : 0.12 :   21    :  37    : 46  : 30   : 39  :
--------------------------------------------------

* Note: the lin8 values are approximate, as they vary a small amount due
to SCALE factor.  see #define section for more detials.

B. MEMORY CHARACTERISTICS

these numbers assume you are going to be using fft_run(), so the values
listed for the other functions are the memory usage in addition to that
already consumed by fft_run.  they are given for SRAM (S) and FLASH (F)
in bytes (B).

----------------------------------------------------------------
Func:   run   : reorder : window :   lin   :  lin8   :   log   :
----------------------------------------------------------------
 N  |  S/F(B) :   F(B)  :  F(B)  :  S/F(B) :  S/F(B) :  S/F(B) :
----------------------------------------------------------------
256 :  1k/952 :   120   :  512   : 256/768 : 128/640 : 128/256 :
128 : 512/448 :   56    :  256   : 128/768 :  64/640 :  64/256 :
64  : 256/200 :   28    :  128   :  64/768 :  32/640 :  32/256 :
32  : 128/80  :   12    :  64    :  32/768 :  16/640 :  16/256 :
16  :  64/24  :    6    :  32    :  16/768 :   8/640 :   8/256 :
----------------------------------------------------------------


1. the following files should be included with the fft library:

FFT.h - header file with all the code
keywords.txt - color coding for keywords in sketch
fft_codec.pde - example sketch using codecshield
fft_adc.pde - example sketch using adc
arduinofft_display.pd - puredata fft data display patch for example sketches
reorder_table_creator.pde - arduion sketch for generating the reorder lookup table

lookup tables for reordering the input data
---------------
16_reorder.inc
32_reorder.inc
64_reorder.inc
128_reorder.inc
256_reorder.inc

window functions for windowing the data
------------
hann_16.inc
hann_32.inc
hann_64.inc
hann_128.inc
hann_256.inc

cos and sin tables for fft multiplication
----------------
wklookup_16.inc
wklookup_32.inc
wklookup_64.inc
wklookup_128.inc
wklookup_256.inc

log and sqrt tables for calculating output magnitude
----------------
sqrtlookup8.inc
sqrtlookup16.inc
decibel.inc

2. there are multiple functions you can call to operate the fft.  the
reason they are broken up, is so you can tailor the fft to your needs.
if you dont neet particular parts, you can not run them and save time.

A. fft_run() - this is the main fft function call.  takes no variables and
returns no variables.  it assumes there is a block of data already in sram,
and that it is already reordered.  the data is stored in array called:

fft_input[]

which contains 2 16b values per fft data point.  one value for real, the
other for imaginary.  if you are filling the array yourself, place the real
parameter in the even bins, and the imaginary values in the odd bins.  for
example:

fft_input[0] = real1, fft_input[1] = imaginary1
fft_input[2] = real2, fft_input[3] = imaginary2

therefore, there are 2 times as many points in the array as there are fft bins.
if you are using only real data (i.e. values sampled from the ADC), then only
put those values into the even numbered bins, and be sure to write 0 to the
odd valued bins.

the final output is kept in fft_input[], with the even bins being the real
magnitude, and the odd bins being the imaginary magnitude. the bins are in
sequence of increasing frequncy. for example:

fft_input[0] & fft_input[1] = bin0 magnitudes (0hz -> Fs/N)
fft_input[2] & fft_input[3] = bin1 magnitudes (Fs/N -> 2Fs/N)

you will have to run one of the magnitude functions to get useful data from
these bins.

B. fft_reorder() - this reorders the fft inputs to get them ready for the
special way in which the fft algorithm processes data.  unless you do this
yourself with another piece of code, you have to call this before running
fft_run().  it takes no variables and returns no variables.  this runs on
the array fft_input[], so the data must be first filled into that array
before this function is called.

C. fft_window() - this function multiplies the input data by a window function
to help increase the frequency resolution of the fft data.  this takes no
variables, and returns no variables.  this processes the data in fft_input[],
so that data must first be placed in that array before it is called.  it must
be called before fft_reorder() or fft_run().

D. fft_mag_lin8() - this gives the magnitude of each bin in from the fft.  it
sums the squares of the imaginary and real, and then takes the square root,
rounding the answer to 8b precision (it uses a lookup table, and scales the 
values to fit the full 8b range.  scale factor = 255/(181*256)).  this takes
no variables, and returns no variables. it operates on fft_input, and returns
the data in an array called fft_lin_out8[]. you can then use the data in
fft_lin_out8[]. the magnitude is only calculated for the first N/2 bins, as
the second half of an fft is identical to the first half for all real inputs.
so fft_lin_out8[] has N/2 8b values, with each index equaliing the bin order.
for example:

fft_lin_out8[0] = first bin magnitude (0hz -> Fs/N)
fft_lin_out8[1] = second bin magnitude (Fs/N -> 2Fs/N)

E. fft_mag_lin() - this gives the magnitude of each bin in the fft.  it sums
the squares of the imaginary and real, and then takes the square root.  it uses
a lookup table to calculate the square root, so it has limited precision.  you
can think of it as 8b of value times 4b of exponent.  so it covers the full 16b
range, but only has 8b of precision at any point in that range.  the data is
taken in on fft_input[] and returned on fft_lin_out[].  the values are in
sequential order, and there are only N/2 values total, as the fft of a real
signal is symetric about the center frequency.

F. fft_mag_log() - this gives the magnitude of each bin in the fft.  it sums
the squares of the imaginary and real, and then takes the square root, and then
takes the log base 2 of that value.  so the output is compressed in a logrithmic
fashion, and is essentially in decibels (times a scaling factor).  it takes no
variables, and returns no variables.  it uses a lookup table to calculate the 
log of the square root, and scales the output over the full 8b range {the
equation is 16*(log2((img^2 + real^2)^0.5))}.  it is only an 8b value, and the
values are taken from fft_input[], and returned at fft_log_out[].  the output
values are in sequential order of fft frequency bins, and there are only N/2
total bins, as the second half of the fft result is redundant for real inputs.

G. fft_mag_octave() - this outputs the RMS value of the bins in an octave
(doubling of frequencies) format. this is more useful in some ways, as it is
closer to how humans percieve sound.  it doesnt take any variables, and doesnt
return any variables.  the input is taken from fft_output[] and returned on
fft_oct_out[].  the data is represented in and 8b value of 16*log2(sqrt(mag)).
there are LOG_N bins. and they are given as follows:

FFT_N = 256 :: bins = [0, 1, 2:4, 5:8, 9:16, 17:32, 3:64, 65:128]
FFT_N = 128 :: bins = [0, 1, 2:4, 5:8, 9:16, 17:32, 3:64]

where (5:8) is a summation of all bins, 5 through 8.  the data for each bin is
squared, imaginary and real parts, and then added with all the squared magnitudes
for the range.  it is then divided down by the numbe of bins (which can be turned
off - see #defines below), and then the square root is taken, and then the log is
taken.

3. EXAMPLE: 256 point FFT

1. fill up fft_input[] with a sample at the even indices, and 0 at the odd
indices.  do this until youve filled up to fft_input[511].

2. call fft_window() to window the data

3. call fft_reorder() to reorder the data

4. call fft_run() to process the fft

5. call fft_mag_log() to get the magnitudes of the bins

6. take data out of fft_log_out[] and display them in sequence to make a
spectrum analyzer.  do this for fft_log_out[0] -> fft_log_out[127]. the
output data will be for 0Hz -> Fs/(2*N), with Fs/N spacing.

4. #DEFINES

these values allow you to modify the fft code to fit your needs.  for the most
part, they just turn off stuff you arent using. by default everything is on,
so its best to use them to turn off the extra resource hogs.

A. FFT_N - sets the fft size.  possible options are 16, 32, 64, 128, 256.
256 is the defualt.

B. SCALE - sets the scaling factor for fft_mag_lin8().  since 8b resolution
is pretty poor, you will want to scale the values to max out the full range.
setting SCALE multiplies the output by a constant beefore doing the square
root, so you have maximum resolution.  it does consume slightly more
resources, but is pretty minimal.  SCALE can be any number from 1 -> 255. by
default it is 1.  1, 2, 4, 128, and 256 consume the least resources.

C. WINDOW - turns on or off the window function resources.  if you are not using
fft_window(), then you should set WINDOW 0 (off). by default its 1 (on).

D. REORDER - turns on or off the reorder function resources.  if you are not using
fft_reorder(), then you should set REORDER 0 (off). by default its 1 (on).

E. LOG_OUT - turns on or off the log function resources.  if you are using
fft_mag_log(), then you should set LOG_OUT 1 (on). by default its 0 (off).

F. LIN_OUT - turns on or off the lin output function resources.  if you are using
fft_mag_lin(), then you should set LIN_OUT 1 (on). by default its 0 (off).

G. LIN_OUT8 - turns on or off the lin8 output function resources.  if you are using
fft_mag_lin8(), then you should set LIN_OUT8 1 (on). by default its 0 (off).

H. OCTAVE - this turns on or off the octave output function resources. if you are
using fft_mag_octave(), then you should set OCTAVE 1 (on).  by default it is 0 (off).

I. OCT_NORM - this turns on or off the octave normilisation feature.  this is the
part of fft_mat_octave() that divides each bin grouping by the number of bins.
since a lot of sound sources are pink noise (they drop off in amplitude as the
frequency increases), the scale tends to slope off rather quickly.  this artificially
boosts the higher frequencies when off (OCT_NORM 0).  by default, the normilisation
is on (OCT_NORM 1).

 