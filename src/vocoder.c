#include "reim/vocoder.h"

#include "reim/mathematics.h"
#include "reim/memory.h"
#include <assert.h>

vocoder_context_t* create_vocoder_context(double period, size_t fftsize, double fo_floor, double fo_ceil, double fs)
{
    assert(ISPOW2(fftsize));
    assert(fo_floor > 0);
    assert(fo_floor < fo_ceil);
    assert(fo_ceil < fs / 2);
    assert(fs > 0);

    vocoder_context_t* vocoder = REIM_ALLOC_SINGLE(vocoder_context_t);
    vocoder->period = period;
    vocoder->fs = fs;
    vocoder->fo_floor = fo_floor;
    vocoder->fo_ceil = fo_ceil;
    vocoder->fftsize = fftsize;
    vocoder->numbins = fftsize / 2 + 1;
    vocoder->fft = create_fft(fftsize);
    vocoder->ifft = create_ifft(fftsize);

    return vocoder;
}

void destroy_vocoder_context(vocoder_context_t** vocoder)
{
    destroy_fft(&(*vocoder)->fft);
    destroy_ifft(&(*vocoder)->ifft);
    REIM_FREE(*vocoder);
    *vocoder = NULL;
}
