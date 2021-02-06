#include "example_audio.h"

#include "reim/analyze_ap.h"
#include "reim/analyze_fo.h"
#include "reim/analyze_silence.h"
#include "reim/analyze_sp.h"
#include "reim/audio_frame.h"
#include "reim/mathematics.h"
#include "reim/memory.h"
#include "reim/synthesis.h"
#include "reim/vocoder.h"
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    audio_frame_t* frame;
    vocoder_context_t* vocoder;
    fo_context_t* fo_context;
    ap_context_t* ap_context;
    sp_context_t* sp_context;
    synthesis_context_t* synthesis;

    double* waveform;
    double* ap;
    double* sp;
} audio_data_t;

void* audio_initializer(size_t buffer_size, double fs)
{
    audio_data_t* data = REIM_ALLOC_SINGLE(audio_data_t);
    const double period = 5.0;
    const double fo_floor = 71.0;
    const double fo_ceil = 800.0;
    const size_t fftsize = 2048;
    const size_t numbins = fftsize / 2 + 1;

    data->frame = create_audio_frame(fs, period, fftsize);
    data->vocoder = create_vocoder_context(period, fftsize, fo_floor, fo_ceil, fs);
    data->fo_context = create_fo_context(data->vocoder);
    data->sp_context = create_sp_context(data->vocoder);
    data->ap_context = create_ap_context(data->vocoder);
    data->synthesis = create_synthesis_context(data->vocoder);

    data->waveform = allocate_vector(fftsize + 1);
    data->ap = allocate_vector(numbins);
    data->sp = allocate_vector(numbins);

    return data;
}

void audio_terminator(void** userdata)
{
    audio_data_t* data = (audio_data_t*)*userdata;

    destroy_audio_frame(&data->frame);
    destroy_vocoder_context(&data->vocoder);
    destroy_fo_context(&data->fo_context);
    destroy_ap_context(&data->ap_context);
    destroy_sp_context(&data->sp_context);
    destroy_synthesis_context(&data->synthesis);

    free_vector(data->waveform);
    free_vector(data->ap);
    free_vector(data->sp);

    REIM_FREE(data);
    *userdata = NULL;
}

void audio_callback(const double* input, double* output, size_t buffer_size, void* userdata)
{
    audio_data_t* data = (audio_data_t*)userdata;

    const double* waveform = data->waveform + 1;
    const double* waveform_delayed = data->waveform;
    for (size_t i = 0; i < buffer_size; i++) {
        // frame analysis and synthesis
        if (next_audio_frame(data->frame, input[i], data->waveform)) {
            // silence analysis
            const bool issilence = analyze_silence(data->vocoder, waveform, REIM_SILENCE_THRESHOLD);

            // fo analysis
            const double fo = analyze_fo(data->vocoder, data->fo_context, waveform, waveform_delayed);

            // ap analysis
            const bool isvoiced = analyze_ap(data->vocoder, data->ap_context, waveform, fo, issilence, data->ap);

            // sp analysis
            analyze_sp(data->vocoder, data->sp_context, waveform, fo, isvoiced, issilence, data->sp);

            // synthesis: new frame
            synthesize_new_frame(data->vocoder, data->synthesis, fo, isvoiced, issilence, data->ap, data->sp);
        }
        output[i] = synthesize_next_sample(data->vocoder, data->synthesis);
        // assert(!isnan(output[i]));
        // assert(isfinite(output[i]));
    }
}

int main()
{
    const size_t buffer_size = 4096;
    const double fs = 48000;

    audio_process_file("vocal_denden_cut.wav", "output.wav", buffer_size, audio_initializer, audio_terminator, audio_callback);
    audio_process_realtime(buffer_size, fs, audio_initializer, audio_terminator, audio_callback);

    return 0;
}
