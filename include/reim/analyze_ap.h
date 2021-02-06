#ifndef __REIM_ANALYZE_AP_H__
#define __REIM_ANALYZE_AP_H__
#include "reim/defines.h"
REIM_BEGIN_EXTERN_C
#include "reim/vocoder.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    double* x_real;
    double* x_imag;
} ap_context_t;

// Create a new aperiodicity context
ap_context_t* create_ap_context(vocoder_context_t* vocoder);

// Destroy the aperiodicity context
void destroy_ap_context(ap_context_t** context);

// Analyze aperiodicity
// Return true for voiced frame
bool analyze_ap(vocoder_context_t* vocoder, ap_context_t* context, const double* input, double fo, bool issilence, double* ap);

REIM_END_EXTERN_C
#endif
