#include "reim/analyze_ap.h"

#include "reim/mathematics.h"
#include "reim/memory.h"
#include <assert.h>

static double blackman_window(double index, double fftsize, double length)
{
    double wt = 2 * REIM_PI * (index - (fftsize - 1) / 2) / length;
    if (wt < -REIM_PI || REIM_PI < wt)
        return 0;
    return 0.42 + 0.5 * cos(wt) + 0.08 * cos(2 * wt);
}

static bool estimate_is_voiced(const double* input, double* re, double* im, size_t fftsize, double fo, double fs, fft_t* fft)
{
    if (fs < 16000) {
        return true;
    }

    // power spectrum
    const double window_length = MIN(1.5 * fs / fo, fftsize);
    for (size_t i = 0; i < fftsize; i++) {
        re[i] = input[i] * blackman_window(i, fftsize, window_length);
        im[i] = 0.0;
    }
    execute_fft(fft, re, im);
    for (size_t k = 0; k <= fftsize / 2; k++) {
        re[k] = COMPLEX_ABS2(re[k], im[k]);
    }

    // D4C LoveTrain
    const size_t indexLower = (size_t)floor(100 / fs * fftsize);
    const size_t indexUpper1 = (size_t)floor(4000 / fs * fftsize);
    const size_t indexUpper2 = (size_t)floor(7900 / fs * fftsize);

    double weight1 = 1e-6;
    for (size_t k = indexLower + 1; k <= indexUpper1; k++) {
        weight1 += re[k];
    }

    double weight2 = weight1;
    for (size_t k = indexUpper1 + 1; k <= indexUpper2; k++) {
        weight2 += re[k];
    }

    return weight1 / weight2 > 0.7;
}

ap_context_t* create_ap_context(vocoder_context_t* vocoder)
{
    const size_t fftsize = vocoder->fftsize;

    ap_context_t* context = REIM_ALLOC_SINGLE(ap_context_t);
    context->x_real = allocate_vector(fftsize);
    context->x_imag = allocate_vector(fftsize);

    return context;
}

void destroy_ap_context(ap_context_t** context)
{
    free_vector((*context)->x_real);
    free_vector((*context)->x_imag);

    REIM_FREE(*context);
    *context = NULL;
}

bool analyze_ap(vocoder_context_t* vocoder, ap_context_t* context, const double* input, double fo, bool issilence, double* ap)
{
    const double fs = vocoder->fs;
    const double fo_floor = vocoder->fo_floor;
    const double fo_ceil = vocoder->fo_ceil;
    const size_t fftsize = vocoder->fftsize;
    const size_t numbins = vocoder->numbins;

    // unvoiced when silence or fo is out-of-range (including 0 Hz)
    if (issilence || fo < fo_floor || fo > fo_ceil) {
        goto when_unvoiced;
    }

    // estimate voiced/unvoiced
    if (!estimate_is_voiced(input, context->x_real, context->x_imag, fftsize, fo, fs, vocoder->fft)) {
        goto when_unvoiced;
    }

    // placeholder
    for (size_t k = 0; k < numbins; k++) {
        ap[k] = 1e-3;
    }

    return true;

when_unvoiced:
    // fully aperiodic
    for (size_t k = 0; k < numbins; k++) {
        ap[k] = 1.0;
    }
    return false;
}
