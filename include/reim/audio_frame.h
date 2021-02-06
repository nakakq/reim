#ifndef __REIM_AUDIO_FRAME_H__
#define __REIM_AUDIO_FRAME_H__
#include "reim/circular_buffer.h"
#include "reim/defines.h"
#include <stdbool.h>
#include <stddef.h>
REIM_BEGIN_EXTERN_C

typedef struct {
    double framesize;
    double position;

    size_t fftsize;
    size_t outputsize;
    circular_buffer_t* buffer_in;
} audio_frame_t;

// Create a new audio frame
audio_frame_t* create_audio_frame(double fs, double frame_period, size_t fftsize);

// Destroy the frame
void destroy_audio_frame(audio_frame_t** frame);

// Process new input sample
// If a new frame is available, returns true and sets the frame_waveform
// (frame_waveform: double[fftsize + 1])
bool next_audio_frame(audio_frame_t* frame, double input, double* frame_waveform);

REIM_END_EXTERN_C
#endif
