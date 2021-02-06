#include "reim/synthesis.h"

#include "reim/mathematics.h"
#include "reim/memory.h"

static void generate_minimum_phase_spectrum(double* spec_r, double* spec_i, double gain, size_t fftsize, fft_t* fft, ifft_t* ifft)
{
    const size_t numbins = fftsize / 2 + 1;

    // cepstrum
    for (size_t k = 0; k < fftsize; k++) {
        spec_r[k] = log(spec_r[k] + 1e-12);
        spec_i[k] = 0.0;
    }
    execute_ifft(ifft, spec_r, spec_i);

    // liftering
    spec_r[0] *= 0.5;
    spec_i[0] *= 0.5;
    spec_r[numbins - 1] *= 0.5;
    spec_i[numbins - 1] *= 0.5;
    for (size_t k = numbins; k < fftsize; k++) {
        spec_r[k] = spec_i[k] = 0.0;
    }

    // complex spectrum
    execute_fft(fft, spec_r, spec_i);
    for (size_t k = 0; k < numbins; k++) {
        const double a = gain * exp(spec_r[k]);
        const double b = spec_i[k];
        spec_r[k] = a * cos(b);
        spec_i[k] = a * sin(b);
    }
    for (size_t k = 0; k < numbins - 2; k++) {
        spec_r[numbins + k] = spec_r[numbins - 2 - k];
        spec_i[numbins + k] = -spec_i[numbins - 2 - k];
    }
}

static void generate_impulse(double* impulse, const double* spec_r, const double* spec_i, double shift,
    const double* window, double* temp_r, double* temp_i,
    size_t fftsize, ifft_t* ifft)
{
    const size_t numbins = fftsize / 2 + 1;

    if (shift == 0.0) {
        // zero shift spectrum
        for (size_t k = 0; k < fftsize; k++) {
            temp_r[k] = spec_r[k];
            temp_i[k] = spec_i[k];
        }
    } else {
        // time shifted spectrum
        for (size_t k = 0; k < numbins; k++) {
            const double omega = -REIM_PI * shift * (double)k / (numbins - 1);
            temp_r[k] = cos(omega);
            temp_i[k] = sin(omega);
        }
        for (size_t k = 0; k < numbins - 2; k++) {
            temp_r[numbins + k] = temp_r[numbins - 2 - k];
            temp_i[numbins + k] = -temp_i[numbins - 2 - k];
        }
        for (size_t k = 0; k < fftsize; k++) {
            const double xr1 = temp_r[k];
            const double xi1 = temp_i[k];
            const double xr2 = spec_r[k];
            const double xi2 = spec_i[k];
            temp_r[k] = xr1 * xr2 - xi1 * xi2;
            temp_i[k] = xr1 * xi2 + xi1 * xr2;
        }
    }

    // generate impulse response
    execute_ifft(ifft, temp_r, temp_i);
    ifftshift(temp_r, impulse, numbins);

    // remove DC component
    double gain = 0;
    for (size_t k = 0; k < fftsize; k++) {
        gain += impulse[k];
    }
    for (size_t k = 0; k < fftsize; k++) {
        impulse[k] -= window[k] * gain;
    }
}

static double vorbis_window(double index, double fftsize)
{
    double wt = REIM_PI * (index + 0.5) / fftsize;
    const double s = sin(wt);
    return sin(REIM_PI / 2 * s * s);
}

synthesis_context_t* create_synthesis_context(const vocoder_context_t* vocoder)
{
    synthesis_context_t* context = REIM_ALLOC_SINGLE(synthesis_context_t);
    const double fs = vocoder->fs;
    const double fo_floor = vocoder->fo_floor;
    const size_t fftsize = vocoder->fftsize;

    context->has_pulse = false;
    context->has_noise = false;

    context->spec_pulse_r = allocate_vector(fftsize);
    context->spec_pulse_i = allocate_vector(fftsize);
    context->spec_noise_r = allocate_vector(fftsize);
    context->spec_noise_i = allocate_vector(fftsize);

    // window to remove DC component
    context->window = allocate_vector(fftsize);
    double gain = 0.0;
    for (size_t i = 0; i < fftsize; i++) {
        const double window = vorbis_window(i, fftsize);
        context->window[i] = window;
        gain += window;
    }
    for (size_t i = 0; i < fftsize; i++) {
        context->window[i] /= gain;
    }

    context->impulse_pulse = allocate_vector(fftsize);
    context->impulse_noise = allocate_vector(fftsize);
    context->temp_r = allocate_vector(fftsize);
    context->temp_i = allocate_vector(fftsize);
    for (size_t i = 0; i < fftsize; i++) {
        context->impulse_pulse[i] = 0.0;
        context->impulse_noise[i] = 0.0;
    }

    context->interval = fs / 300;
    context->pulse_int = 0;
    context->pulse_frc = 0.0;

    setup_random(context->random);
    context->interval_velvet = (size_t)round(fs / 2000.0);
    context->interval_random = 0;
    context->gain_noise = sqrt(context->interval_velvet);
    context->noise_int = 0;

    const size_t period_max = (size_t)ceil(fs / fo_floor);
    context->buffer = create_circular_queue(period_max + fftsize);

    return context;
}

void destroy_synthesis_context(synthesis_context_t** context)
{
    free_vector((*context)->spec_pulse_r);
    free_vector((*context)->spec_pulse_i);
    free_vector((*context)->spec_noise_r);
    free_vector((*context)->spec_noise_i);

    free_vector((*context)->window);

    free_vector((*context)->impulse_pulse);
    free_vector((*context)->impulse_noise);
    free_vector((*context)->temp_r);
    free_vector((*context)->temp_i);

    destroy_circular_queue(&(*context)->buffer);

    REIM_FREE(*context);
    *context = NULL;
}

void synthesize_new_frame(vocoder_context_t* vocoder, synthesis_context_t* context, double fo, bool isvoiced, bool issilence, double* ap, double* sp)
{
    const double fs = vocoder->fs;
    const size_t fftsize = vocoder->fftsize;
    const size_t numbins = vocoder->numbins;

    for (size_t k = 0; k < numbins; k++) {
        const double spec = sp[k];
        const double aper = ap[k] * ap[k];
        context->spec_pulse_r[k] = spec * (1.0 - aper);
        context->spec_noise_r[k] = spec * aper;
    }
    for (size_t k = 0; k < numbins - 2; k++) {
        context->spec_pulse_r[numbins + k] = context->spec_pulse_r[numbins - 2 - k];
        context->spec_noise_r[numbins + k] = context->spec_noise_r[numbins - 2 - k];
    }

    // periodic component
    context->has_pulse = (isvoiced && !issilence);
    if (context->has_pulse) {
        context->interval = fs / fo;
        double gain_pulse = sqrt(context->interval);

        // create minimum phase filter
        generate_minimum_phase_spectrum(context->spec_pulse_r, context->spec_pulse_i, gain_pulse, fftsize, vocoder->fft, vocoder->ifft);
    }

    // aperiodic component
    context->has_noise = !issilence;
    if (context->has_noise) {
        double gain_noise = context->gain_noise;

        // create minimum phase filter
        generate_minimum_phase_spectrum(context->spec_noise_r, context->spec_noise_i, gain_noise, fftsize, vocoder->fft, vocoder->ifft);

        // create impulse response for aperiodic component
        generate_impulse(context->impulse_noise, context->spec_noise_r, context->spec_noise_i, 0.0,
            context->window, context->temp_r, context->temp_i, fftsize, vocoder->ifft);
    }
}

double synthesize_next_sample(vocoder_context_t* vocoder, synthesis_context_t* context)
{
    const size_t fftsize = vocoder->fftsize;

    // periodic component
    if (context->has_pulse) {
        if (context->pulse_int == 0) {
            // create impulse response for periodic component
            generate_impulse(context->impulse_pulse, context->spec_pulse_r, context->spec_pulse_i, context->pulse_frc,
                context->window, context->temp_r, context->temp_i, fftsize, vocoder->ifft);

            // write impulse
            push_additive_circular_queue(context->buffer, context->impulse_pulse, fftsize);

            // update excitation position
            const double interval_int = floor(context->interval);
            const double interval_frc = context->interval - interval_int;
            const double next = context->pulse_frc + interval_frc;
            const double carry = floor(next);
            context->pulse_int += (int32_t)(interval_int + carry);
            context->pulse_frc = next - carry;
        }
        context->pulse_int--;
    }

    // aperiodic component
    if (context->has_noise) {
        if (context->noise_int == context->interval_random) {
            // write impulse
            push_additive_circular_queue(context->buffer, context->impulse_noise, fftsize);
        }
        if (context->noise_int == context->interval_velvet - 1) {
            // update excitation position
            const double r = generate_uniform_random(context->random);
            context->interval_random = (size_t)floor(r * (context->interval_velvet - 1));
            context->noise_int = 0;
        }
        context->noise_int++;
    }

    // get from circular queue
    return pop_circular_queue(context->buffer);
}
