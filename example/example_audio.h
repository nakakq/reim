#ifndef __AUDIO_H__
#define __AUDIO_H__
#include "reim/defines.h"
REIM_BEGIN_EXTERN_C
#include <stdbool.h>
#include <stddef.h>

typedef void* (*audio_initializer_t)(size_t buffer_size, double fs);
typedef void (*audio_terminator_t)(void** userdata);
typedef void (*audio_callback_t)(const double* input_buffer, double* output_buffer, size_t buffer_size, void* userdata);

bool audio_process_file(const char* input_filename, const char* output_filename, size_t buffer_size,
    const audio_initializer_t initializer, const audio_terminator_t terminator,
    const audio_callback_t callback);

void audio_process_realtime(size_t buffer_size, double fs,
    const audio_initializer_t initializer, const audio_terminator_t terminator,
    const audio_callback_t callback);

REIM_END_EXTERN_C
#endif
