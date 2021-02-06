#ifndef __REIM_ANALYZE_SILENCE_H__
#define __REIM_ANALYZE_SILENCE_H__
#include "reim/defines.h"
REIM_BEGIN_EXTERN_C
#include "reim/vocoder.h"
#include <stdbool.h>
#include <stddef.h>

#define REIM_SILENCE_THRESHOLD 0.00025 // -72 dB

// Analyze silence of the frame
// Return true when the frame is silence
bool analyze_silence(vocoder_context_t* vocoder, const double* input, double threshold);

REIM_END_EXTERN_C
#endif
