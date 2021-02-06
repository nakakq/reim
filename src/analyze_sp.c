#include "reim/analyze_sp.h"

#include "reim/mathematics.h"
#include "reim/memory.h"

static double hanning_window(double index, double fftsize, double length)
{
    double wt = REIM_PI * (2 * index - (fftsize - 1)) / length;
    if (wt < -REIM_PI || REIM_PI < wt)
        return 0;
    return 0.5 + 0.5 * cos(wt);
}

static void apply_replica(double* pspec, size_t numbins, double fo, double fs)
{
    size_t fftsize = 2 * (numbins - 1);

    // replicate the spectrum for DC component
    size_t fobin = 1 + round(fo / (fs / 2) * (numbins - 1));
    for (size_t k = 0; k < fobin; k++) {
        pspec[k] += pspec[fftsize - fobin - k];
    }

    // copy to the last half
    for (size_t k = 0; k < numbins - 2; k++) {
        pspec[numbins + k] = pspec[numbins - 2 - k];
    }
}

static void smooth_spectrum(double* pspec, double* spec_cumsum, size_t numbins, double freq_range, double fs)
{
    size_t fftsize = 2 * (numbins - 1);

    // cumulative summation of spectrum
    const size_t offset = numbins - 2;
    spec_cumsum[0] = pspec[numbins];
    for (size_t k = 1; k < offset; k++) {
        spec_cumsum[k] = pspec[numbins + k] + spec_cumsum[k - 1];
    }
    for (size_t k = 0; k < fftsize; k++) {
        spec_cumsum[offset + k] = pspec[k] + spec_cumsum[offset + k - 1];
    }

    // moving average of the spectrum
    const double half_range = freq_range / fs * (numbins - 1);
    const size_t half_range_int = (size_t)floor(half_range);
    const double half_range_frc = half_range - half_range_int;
    for (size_t k = 0; k < numbins; k++) {
        const size_t index_upper = offset + k + half_range_int;
        const size_t index_lower = offset + k - half_range_int;
        const double upper = (1.0 - half_range_frc) * spec_cumsum[index_upper - 1] + half_range_frc * spec_cumsum[index_upper];
        const double lower = (1.0 - half_range_frc) * spec_cumsum[index_lower] + half_range_frc * spec_cumsum[index_lower - 1];
        pspec[k] = MAX(upper - lower, 1e-12) / (2.0 * half_range);
    }

    // copy to the last half
    for (size_t k = 0; k < numbins - 2; k++) {
        pspec[numbins + k] = pspec[numbins - 2 - k];
    }
}

static void lifter_spectrum(double* pspec, double* imag, size_t numbins, double fo, double fs, fft_t* fft, ifft_t* ifft)
{
    size_t fftsize = 2 * (numbins - 1);

    // cepstrum
    for (size_t k = 0; k < fftsize; k++) {
        pspec[k] = log(pspec[k] + 1e-12);
        imag[k] = 0.0;
    }
    execute_ifft(ifft, pspec, imag);

    // sinc liftering
    const double q = -0.15;
    for (size_t k = 0; k < numbins; k++) {
        const double t = k * fo / fs;
        const double sinct = sin(REIM_PI * t + 1e-12) / (REIM_PI * t + 1e-12);
        pspec[k] *= sinct * ((1.0 - 2.0 * q) + 2.0 * q * cos(2.0 * REIM_PI * t));
        imag[k] = 0.0;
    }
    for (size_t k = 0; k < numbins - 2; k++) {
        pspec[numbins + k] = pspec[numbins - 2 - k];
        imag[numbins + k] = 0.0;
    }

    // power spectrum
    execute_fft(fft, pspec, imag);
    for (size_t k = 0; k < numbins; k++) {
        pspec[k] = exp(pspec[k]);
    }
    for (size_t k = 0; k < numbins - 2; k++) {
        pspec[numbins + k] = pspec[numbins - 2 - k];
    }
}

sp_context_t* create_sp_context(vocoder_context_t* vocoder)
{
    const size_t fftsize = vocoder->fftsize;
    const size_t numbins = vocoder->numbins;

    sp_context_t* context = REIM_ALLOC_SINGLE(sp_context_t);
    context->window = allocate_vector(fftsize);
    context->x_real = allocate_vector(fftsize);
    context->x_imag = allocate_vector(fftsize);
    context->pspec = allocate_vector(fftsize);
    context->spec_cumsum = allocate_vector(numbins + fftsize);
    return context;
}

void destroy_sp_context(sp_context_t** context)
{
    free_vector((*context)->window);
    free_vector((*context)->x_real);
    free_vector((*context)->x_imag);
    free_vector((*context)->pspec);
    free_vector((*context)->spec_cumsum);
    REIM_FREE(*context);
    *context = NULL;
}

void analyze_sp(vocoder_context_t* vocoder, sp_context_t* context, const double* input, double fo, bool isvoiced, bool issilence, double* sp)
{
    const double fs = vocoder->fs;
    const size_t fftsize = vocoder->fftsize;
    const size_t numbins = vocoder->numbins;

    const double window_fo = (isvoiced ? fo : 1.0 / (vocoder->period / 1000.0));
    const double smooth_fo = (isvoiced ? fo : 300.0);

    // don't estimate when silence
    if (issilence) {
        for (size_t k = 0; k < numbins; k++) {
            sp[k] = 1e-12;
        }
        return;
    }

    // analysis windowing
    const double analysis_interval = fs / window_fo;
    const double window_length = MIN(3.0 * analysis_interval, fftsize);
    const double window_scale = 1.0 / sqrt(analysis_interval);
    for (size_t i = 0; i < fftsize; i++) {
        context->window[i] = hanning_window(i, fftsize, window_length) * window_scale;
        context->x_real[i] = input[i] * context->window[i];
        context->x_imag[i] = 0.0;
    }

    // remove DC component
    double sum_x = 0.0, sum_window = 0.0;
    for (size_t i = 0; i < fftsize; i++) {
        sum_x += context->x_real[i];
        sum_window += context->window[i];
    }
    const double gain_dc = sum_x / sum_window;
    for (size_t i = 0; i < fftsize; i++) {
        context->x_real[i] -= gain_dc * context->window[i];
    }

    // power spectrum
    execute_fft(vocoder->fft, context->x_real, context->x_imag);
    for (size_t k = 0; k < numbins; k++) {
        context->pspec[k] = COMPLEX_ABS2(context->x_real[k], context->x_imag[k]);
    }
    for (size_t k = 0; k < numbins - 2; k++) {
        context->pspec[numbins + k] = context->pspec[numbins - 2 - k];
    }

    // DC replication
    apply_replica(context->pspec, numbins, window_fo, fs);

    // smoothing
    smooth_spectrum(context->pspec, context->spec_cumsum, numbins, smooth_fo / 2, fs);

    // liftering
    if (isvoiced) {
        lifter_spectrum(context->pspec, context->x_imag, numbins, smooth_fo, fs, vocoder->fft, vocoder->ifft);
    }

    // copy
    for (size_t k = 0; k < numbins; k++) {
        sp[k] = context->pspec[k];
    }
}
