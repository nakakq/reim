#ifndef __REIM_ANALYZE_SP_H__
#define __REIM_ANALYZE_SP_H__
#include "reim/defines.h"
REIM_BEGIN_EXTERN_C
#include "reim/vocoder.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    double* window;
    double* x_real;
    double* x_imag;
    double* pspec;
    double* spec_cumsum;
} sp_context_t;

// Create a new spectral envelope context
sp_context_t* create_sp_context(vocoder_context_t* vocoder);

// Destroy the spectral envelope context
void destroy_sp_context(sp_context_t** context);

// Analyze spectral envelope
void analyze_sp(vocoder_context_t* vocoder, sp_context_t* context, const double* input, double fo, bool isvoiced, bool issilence, double* sp);

REIM_END_EXTERN_C
#endif
