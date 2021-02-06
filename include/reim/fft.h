#ifndef __REIM_FFT_H__
#define __REIM_FFT_H__
#include "reim/defines.h"
#include <stddef.h>
REIM_BEGIN_EXTERN_C

typedef void fft_t;
typedef void ifft_t;

fft_t* create_fft(size_t fftsize);
ifft_t* create_ifft(size_t fftsize);
void destroy_fft(fft_t** fft);
void destroy_ifft(ifft_t** ifft);
void execute_fft(fft_t* fft, double* real, double* imag);
void execute_ifft(ifft_t* ifft, double* real, double* imag);
const char* get_fft_library_name();

REIM_END_EXTERN_C
#endif
