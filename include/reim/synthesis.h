#ifndef __REIM_SYNTHESIS_H__
#define __REIM_SYNTHESIS_H__
#include "reim/defines.h"
REIM_BEGIN_EXTERN_C
#include "reim/circular_queue.h"
#include "reim/mathematics.h"
#include "reim/vocoder.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    bool has_pulse;
    bool has_noise;

    double* spec_pulse_r; // real spectrum of periodic component
    double* spec_pulse_i; // imag spectrum of periodic component
    double* spec_noise_r; // real spectrum of aperiodic component
    double* spec_noise_i; // imag spectrum of aperiodic component

    double* window; // window to remove DC

    double* impulse_pulse; // impulse response of periodic component
    double* impulse_noise; // impulse response of aperiodic component
    double* temp_r;        // real temporary buffer for impulse generation
    double* temp_i;        // imag temporary buffer for impulse generation

    double interval;   // time interval of periodic excitation
    int32_t pulse_int; // samples left until next excitation (integer part)
    double pulse_frc;  // samples left until next excitation (fractional part)

    random_state_t random;  // random state
    size_t interval_velvet; // time interval of aperiodic excitation
    size_t interval_random; // random offset of aperiodic excitation
    double gain_noise;      // gain of aperiodic excitation
    size_t noise_int;       // samples left until next interval

    circular_queue_t* buffer;
} synthesis_context_t;

synthesis_context_t* create_synthesis_context(const vocoder_context_t* vocoder);
void destroy_synthesis_context(synthesis_context_t** context);
void synthesize_new_frame(vocoder_context_t* vocoder, synthesis_context_t* context, double fo, bool isvoiced, bool issilence, double* ap, double* sp);
double synthesize_next_sample(vocoder_context_t* vocoder, synthesis_context_t* context);

REIM_END_EXTERN_C
#endif
