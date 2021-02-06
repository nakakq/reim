#ifndef __REIM_VOCODER_H__
#define __REIM_VOCODER_H__
#include "reim/defines.h"
REIM_BEGIN_EXTERN_C
#include "reim/fft.h"
#include <stddef.h>

typedef struct {
    double period;   // frame period
    double fs;       // sampling frequency
    double fo_floor; // lower bounds of fo
    double fo_ceil;  // upper bounds of fo
    size_t fftsize;  // FFT size
    size_t numbins;  // number of bins from DC to Nyquist frequency
    fft_t* fft;      // FFT
    ifft_t* ifft;    // IFFT
} vocoder_context_t;

vocoder_context_t* create_vocoder_context(double period, size_t fftsize, double fo_floor, double fo_ceil, double fs);
void destroy_vocoder_context(vocoder_context_t** vocoder);

REIM_END_EXTERN_C
#endif
