#include "example_audio.h"

#include "reim/memory.h"
#include <portaudio.h>
#include <sndfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

bool audio_process_file(const char* input_filename, const char* output_filename, size_t buffer_size,
    const audio_initializer_t initializer, const audio_terminator_t terminator,
    const audio_callback_t callback)
{
    SNDFILE *fin, *fout;
    SF_INFO fin_info, fout_info;

    // open the input file
    fin_info.format = 0;
    if ((fin = sf_open(input_filename, SFM_READ, &fin_info)) == NULL) {
        puts(sf_strerror(NULL));
        return false;
    }
    if (fin_info.channels != 1) {
        sf_close(fin);
        puts("Only supports monaural audio files.");
        return false;
    }

    // open the output file
    fout_info.channels = 1;
    fout_info.samplerate = fin_info.samplerate;
    fout_info.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
    if ((fout = sf_open(output_filename, SFM_WRITE, &fout_info)) == NULL) {
        sf_close(fin);
        puts(sf_strerror(NULL));
        return 0;
    }

    // initialize
    void* userdata = initializer(buffer_size, fin_info.samplerate);

    // create the buffers
    double* input_buffer = (double*)malloc(buffer_size * sizeof(double));
    double* output_buffer = (double*)malloc(buffer_size * sizeof(double));
    for (size_t i = 0; i < buffer_size; i++)
        output_buffer[i] = 0.0;

    // process the audio
    size_t num_read;
    puts("Processing...");
    const clock_t time_begin = clock();
    clock_t cum_time = 0;
    while ((num_read = sf_read_double(fin, input_buffer, buffer_size))) {
        // fill the rest of the input buffer with zero
        for (size_t i = num_read; i < buffer_size; i++)
            input_buffer[i] = 0.0;

        const clock_t time_begin_per_call = clock();
        callback(input_buffer, output_buffer, buffer_size, userdata);
        const clock_t time_end_per_call = clock();
        cum_time += time_end_per_call - time_begin_per_call;

        // write to the file
        sf_write_double(fout, output_buffer, buffer_size);
    }
    const clock_t time_end = clock();
    printf("Processing done in %.3f (%.3f) s.\n", (double)(time_end - time_begin) / CLOCKS_PER_SEC, (double)cum_time / CLOCKS_PER_SEC);

    // terminate
    sf_close(fin);
    sf_close(fout);
    free(input_buffer);
    free(output_buffer);
    terminator(&userdata);

    return true;
}

typedef struct {
    audio_callback_t callback;
    double* input_buffer;
    double* output_buffer;
    void* userdata;
} realtime_audio_callback_t;

static int pa_callback(const void* input_buffer, void* output_buffer, unsigned long buffer_size,
    const PaStreamCallbackTimeInfo* time_info, PaStreamCallbackFlags status_flags, void* userdata)
{
    realtime_audio_callback_t* data = (realtime_audio_callback_t*)userdata;
    float* device_input = (float*)input_buffer;
    float* device_output = (float*)output_buffer;
    double* wrap_input = data->input_buffer;
    double* wrap_output = data->output_buffer;

    for (unsigned long i = 0; i < buffer_size; i++) {
        *wrap_input++ = *device_input++;
    }
    data->callback(data->input_buffer, data->output_buffer, buffer_size, data->userdata);
    for (unsigned long i = 0; i < buffer_size; i++) {
        *device_output++ = *wrap_output++;
    }

    return 0;
}

void audio_process_realtime(size_t buffer_size, double fs,
    const audio_initializer_t initializer, const audio_terminator_t terminator,
    const audio_callback_t callback)
{
    PaError err;

    err = Pa_Initialize();
    if (err != paNoError) {
        fprintf(stderr, "PortAudio error on initializing: %s\n", Pa_GetErrorText(err));
        abort();
    }

    const size_t channels_input = 1;
    const size_t channels_output = 1;
    realtime_audio_callback_t userdata;
    userdata.callback = callback;
    userdata.input_buffer = REIM_ALLOC(buffer_size, double);
    userdata.output_buffer = REIM_ALLOC(buffer_size, double);
    userdata.userdata = initializer(buffer_size, fs);

    PaStream* stream;
    err = Pa_OpenDefaultStream(&stream, channels_input, channels_output, paFloat32, fs, buffer_size, pa_callback, &userdata);
    if (err != paNoError) {
        fprintf(stderr, "PortAudio error on opening stream: %s\n", Pa_GetErrorText(err));
        abort();
    }

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        fprintf(stderr, "PortAudio error on starting stream: %s\n", Pa_GetErrorText(err));
        abort();
    }

    puts("Press Enter key to stop");
    fflush(stdout);
    getchar();

    err = Pa_StopStream(stream);
    if (err != paNoError) {
        fprintf(stderr, "PortAudio error on stopping stream: %s\n", Pa_GetErrorText(err));
        abort();
    }

    err = Pa_CloseStream(stream);
    if (err != paNoError) {
        fprintf(stderr, "PortAudio error on closing stream: %s\n", Pa_GetErrorText(err));
        abort();
    }

    REIM_FREE(userdata.input_buffer);
    REIM_FREE(userdata.output_buffer);
    terminator(&userdata.userdata);

    err = Pa_Terminate();
    if (err != paNoError) {
        fprintf(stderr, "PortAudio error on terminating: %s\n", Pa_GetErrorText(err));
        abort();
    }
}
