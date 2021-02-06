#include "reim/analyze_fo.h"
#include "reim/mathematics.h"
#include "reim/memory.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

static double get_interpolated_spectrum(double freq, double fs, double* spec, size_t numbins)
{
    double position = CLAMP_INDEX(freq / (fs / 2) * (numbins - 1), numbins - 1); // [0, numbins-2]
    size_t index = floor(position);
    double delta = position - index;
    return (1.0 - delta) * spec[index] + delta * spec[index + 1];
}

static double refine_fo(double fo, double fo_floor, double fo_ceil, double fs, double* ifreqf, double* pspec, size_t numbins)
{
    const size_t harmonics = 3;

    // refine the frequency using the instantaneous frequency and the power spectrum
    double sum_freq = 0;
    double denominator = 0;
    for (size_t h = 1; h <= harmonics; h++) {
        double freq = get_interpolated_spectrum(fo * h, fs, ifreqf, numbins);
        double weight = get_interpolated_spectrum(fo * h, fs, pspec, numbins);
        sum_freq += freq * weight;
        denominator += h * weight;
    }
    double refined_fo = sum_freq / denominator;

    // revert to the original fo if the refined_fo exceed the limits or deviates from it too much
    if (refined_fo < fo_floor || refined_fo > fo_ceil || fabs(refined_fo - fo) > fo)
        return fo;

    return refined_fo;
}

static double get_harmonic_score(double fo, double fs, double* pspec, size_t numbins)
{
    const size_t harmonics = 3;
    double score = 1.0;
    for (size_t h = 1; h <= harmonics; h++) {
        // summation of residual harmonics (log-spectrum)
        score *= get_interpolated_spectrum(fo * h, fs, pspec, numbins);
        score /= get_interpolated_spectrum(fo * (h - 0.5), fs, pspec, numbins);
    }
    return score;
}

static bool analyze_fo_with_zerocross(double* x, size_t length, double fs, double* result_fo, double* result_rsd)
{
    int32_t last_positive = -1;
    int32_t last_negative = -1;
    int32_t last_peak = -1;
    int32_t last_dip = -1;

    double denominator = 0.0;
    double sum_freq = 0.0;
    double sum_square_freq = 0.0;

    double xprev = x[0];
    double xdiffprev = x[1] - x[0];
    for (size_t i = 1; i < length - 1; i++) {
        double xcurr = x[i];
        double xdiffcurr = x[i + 1] - x[i];
        if (xprev < 0 && xcurr >= 0) {
            if (last_positive >= 0) {
                double interval = (double)i - last_positive;
                double freq = fs / interval;
                denominator += interval;
                sum_freq += freq * interval;
                sum_square_freq += freq * freq * interval;
            }
            last_positive = i;
        } else if (xprev > 0 && xcurr <= 0) {
            if (last_negative >= 0) {
                double interval = (double)i - last_negative;
                double freq = fs / interval;
                denominator += interval;
                sum_freq += freq * interval;
                sum_square_freq += freq * freq * interval;
            }
            last_negative = i;
        }
        if (xdiffprev < 0 && xdiffcurr >= 0) {
            if (last_peak >= 0) {
                double interval = (double)i - last_peak;
                double freq = fs / interval;
                denominator += interval;
                sum_freq += freq * interval;
                sum_square_freq += freq * freq * interval;
            }
            last_peak = i;
        } else if (xdiffprev > 0 && xdiffcurr <= 0) {
            if (last_dip > 1) {
                double interval = (double)i - last_dip;
                double freq = fs / interval;
                denominator += interval;
                sum_freq += freq * interval;
                sum_square_freq += freq * freq * interval;
            }
            last_dip = i;
        }
        xprev = xcurr;
        xdiffprev = xdiffcurr;
    }

    if (denominator <= 0.0) {
        return false;
    }

    double mean_freq = sum_freq / denominator;
    if (mean_freq <= 0.0 || mean_freq > fs / 2) {
        return false;
    }

    double std_freq = sqrt(sum_square_freq / denominator - mean_freq);

    *result_fo = mean_freq;
    *result_rsd = std_freq / mean_freq; // relative standard deviation, smaller is better
    return true;
}

static double nuttall_window(double index, double fftsize, double length)
{
    double wt = 2 * REIM_PI * (index - (fftsize - 1) / 2) / length;
    if (wt < -REIM_PI || REIM_PI < wt)
        return 0;
    return 0.355768 + 0.487396 * cos(wt) + 0.144232 * cos(2 * wt) + 0.012604 * cos(3 * wt);
}

fo_context_t* create_fo_context(vocoder_context_t *vocoder)
{
    fo_context_t* context = REIM_ALLOC_SINGLE(fo_context_t);
    const double fs = vocoder->fs;
    const double fo_floor = vocoder->fo_floor;
    const double fo_ceil = vocoder->fo_ceil;
    const size_t fftsize = vocoder->fftsize;
    const size_t numbins = vocoder->numbins;

    // DIO settings
    const double channels_per_octave = 2;
    const size_t num_candidates = (size_t)ceil(log2(fo_ceil / fo_floor) * channels_per_octave);
    context->num_candidates = num_candidates;

    // LPF for DIO
    context->channel_filters = allocate_matrix(num_candidates, fftsize);
    context->channel_offsets = REIM_ALLOC(num_candidates, size_t);
    double* xr = allocate_vector(fftsize);
    double* xi = allocate_vector(fftsize);
    for (size_t ch = 0; ch < num_candidates; ch++) {
        const double frequency = fo_floor * pow(2.0, (1.0 + ch) / channels_per_octave);

        // window length
        const double lpf_window_length = ceil(fs / frequency);

        // create Nuttall window LPF
        for (size_t k = 0; k < fftsize; k++) {
            xr[k] = nuttall_window(k, fftsize, lpf_window_length);
            xi[k] = 0.0;
        }
        execute_fft(vocoder->fft, xr, xi);
        for (size_t k = 0; k < fftsize; k++) {
            context->channel_filters[ch][k] = COMPLEX_ABS(xr[k], xi[k]);
        }

        // offset caused by the LPF
        context->channel_offsets[ch] = (size_t)lpf_window_length;
    }
    free_vector(xr);
    free_vector(xi);

    // analysis window
    const double window_length = MIN(4.0 * fs / fo_floor, fftsize);
    context->window = allocate_vector(fftsize);
    for (size_t k = 0; k < fftsize; k++) {
        context->window[k] = nuttall_window(k, fftsize, window_length);
    }

    // allocate buffers
    context->spec_r = allocate_vector(fftsize);
    context->spec_i = allocate_vector(fftsize);
    context->specd_r = allocate_vector(fftsize);
    context->specd_i = allocate_vector(fftsize);
    context->pspec = allocate_vector(numbins);
    context->ifreqf = allocate_vector(numbins);
    context->spec_filt_r = allocate_vector(fftsize);
    context->spec_filt_i = allocate_vector(fftsize);
    context->filtered_r = allocate_vector(fftsize);
    context->filtered_i = allocate_vector(fftsize);

    // previous fo
    context->fo_previous = 0;

    return context;
}

void destroy_fo_context(fo_context_t** context)
{
    free_matrix((*context)->channel_filters, (*context)->num_candidates);
    REIM_FREE((*context)->channel_offsets);
    free_vector((*context)->window);

    free_vector((*context)->spec_r);
    free_vector((*context)->spec_i);
    free_vector((*context)->specd_r);
    free_vector((*context)->specd_i);
    free_vector((*context)->pspec);
    free_vector((*context)->ifreqf);
    free_vector((*context)->spec_filt_r);
    free_vector((*context)->spec_filt_i);
    free_vector((*context)->filtered_r);
    free_vector((*context)->filtered_i);

    REIM_FREE(*context);
    *context = NULL;
}

double analyze_fo(vocoder_context_t* vocoder, fo_context_t* context, const double* input, const double* input_delayed)
{
    const double fs = vocoder->fs;
    const double fo_floor = vocoder->fo_floor;
    const double fo_ceil = vocoder->fo_ceil;
    const size_t fftsize = vocoder->fftsize;
    const size_t numbins = vocoder->numbins;

    // spectrum
    for (size_t k = 0; k < fftsize; k++) {
        context->spec_r[k] = input[k] * context->window[k];
        context->spec_i[k] = 0.0;
        context->specd_r[k] = input_delayed[k] * context->window[k];
        context->specd_i[k] = 0.0;
    }
    execute_fft(vocoder->fft, context->spec_r, context->spec_i);
    execute_fft(vocoder->fft, context->specd_r, context->specd_i);

    for (size_t k = 0; k < numbins; k++) {
        // power spectrum
        context->pspec[k] = COMPLEX_ABS2(context->spec_r[k], context->spec_i[k]) + 1e-15;
        // instantaneous frequency
        context->ifreqf[k] = INSTFREQ(context->spec_r[k], context->spec_i[k], context->specd_r[k], context->specd_i[k], fs);
    }

    // spectrum for filtering
    double mean_input = 0;
    for (size_t k = 0; k < fftsize; k++) {
        mean_input += input[k];
    }
    mean_input /= fftsize;
    for (size_t k = 0; k < fftsize; k++) {
        context->spec_filt_r[k] = input[k] - mean_input;
        context->spec_filt_i[k] = 0.0;
    }
    execute_fft(vocoder->fft, context->spec_filt_r, context->spec_filt_i);

    // initial estimate: previous fo
    double best_fo = -1;
    double best_score = -1;
    if (context->fo_previous > fo_floor) {
        best_fo = refine_fo(context->fo_previous, fo_floor, fo_ceil, fs, context->ifreqf, context->pspec, numbins);
        best_score = get_harmonic_score(best_fo, fs, context->pspec, numbins);
    }

    // DIO (Distributed Inline Operation)
    for (size_t ch = 0; ch < context->num_candidates; ch++) {
        // apply LPF in frequency domain
        for (size_t k = 0; k < fftsize; k++) {
            const double filter = context->channel_filters[ch][k];
            context->filtered_r[k] = context->spec_filt_r[k] * filter;
            context->filtered_i[k] = context->spec_filt_i[k] * filter;
        }
        execute_ifft(vocoder->ifft, context->filtered_r, context->filtered_i);

        // analyze zerocross
        const size_t offset = context->channel_offsets[ch];
        double fo = 0, rsd = 0;
        if (!analyze_fo_with_zerocross(context->filtered_r + offset, fftsize - offset, fs, &fo, &rsd)) {
            continue;
        }
        if (isnan(fo) || fo < fo_floor || fo > fo_ceil || rsd > 1.0) {
            continue;
        }

        // refine fo
        const double fo_refined = refine_fo(fo, fo_floor, fo_ceil, fs, context->ifreqf, context->pspec, numbins);

        // score (higher is better)
        const double score = get_harmonic_score(fo_refined, fs, context->pspec, numbins);

        // update if it is the best candidate
        if (best_score < score) {
            best_fo = fo_refined;
            best_score = score;
        }
    }

    // fo is 0 Hz when the estimatedfo is invalid
    if (best_fo < fo_floor || best_fo > fo_ceil || best_score < 0)
        return 0.0;

    // update previous fo
    context->fo_previous = best_fo;

    return best_fo;
}
