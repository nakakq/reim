#ifndef __REIM_ANALYZE_FO_H__
#define __REIM_ANALYZE_FO_H__
#include "reim/defines.h"
REIM_BEGIN_EXTERN_C
#include "reim/vocoder.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    size_t num_candidates;    // number of candidates
    double** channel_filters; // filter bank for DIO
    size_t* channel_offsets;  // sample offsets of channels
    double* window;           // analysis window (fixed)

    double* spec_r;      // real spectrum of current frame
    double* spec_i;      // imag spectrum of current frame
    double* specd_r;     // real spectrum of current one-sample-delayed frame
    double* specd_i;     // imag spectrum of current one-sample-delayed frame
    double* pspec;       // power spectrum
    double* ifreqf;      // instantaneous frequency (frequency domain)
    double* spec_filt_r; // real spectrum of current frame (for filtering)
    double* spec_filt_i; // imag spectrum of current frame (for filtering)
    double* filtered_r;  // filtered waveform (real)
    double* filtered_i;  // filtered waveform (imag)

    double fo_previous; // estimated fo of previous frame
} fo_context_t;

fo_context_t* create_fo_context(vocoder_context_t* vocoder);
void destroy_fo_context(fo_context_t** context);
double analyze_fo(vocoder_context_t* vocoder, fo_context_t* context, const double* input, const double* input_delayed);

REIM_END_EXTERN_C
#endif
