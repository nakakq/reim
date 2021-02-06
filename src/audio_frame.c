#include "reim/audio_frame.h"

#include "reim/mathematics.h"
#include "reim/memory.h"
#include <math.h>
#include <stdlib.h>

audio_frame_t* create_audio_frame(double fs, double frame_period, size_t fftsize)
{
    audio_frame_t* frame = REIM_ALLOC_SINGLE(audio_frame_t);
    frame->framesize = frame_period / 1000.0 * fs;
    frame->position = 0.0;

    frame->fftsize = fftsize;
    frame->outputsize = 0;
    frame->buffer_in = create_circular_buffer(fftsize + 1);

    return frame;
}

void destroy_audio_frame(audio_frame_t** frame)
{
    destroy_circular_buffer(&(*frame)->buffer_in);
    REIM_FREE(*frame);
    *frame = NULL;
}

bool next_audio_frame(audio_frame_t* frame, double input, double* frame_waveform)
{
    push_circular_buffer(frame->buffer_in, input);

    const double position = frame->position;
    const size_t position_int = (size_t)floor(position);

    // a new frame is available when the position reaches at the beginning of the frame
    bool has_new_frame = (position_int == 0);
    if (has_new_frame) {
        // copy to the frame waveform buffer
        copy_all_circular_buffer(frame->buffer_in, frame_waveform);

        // calculate the number of the samples to output
        const double position_frac = position - position_int;
        frame->outputsize = (size_t)floor(frame->framesize + position_frac);
    }

    // update the position in the current frame
    if (frame->position >= frame->framesize - 1) {
        frame->position -= frame->framesize - 1;
    } else {
        frame->position += 1.0;
    }

    return has_new_frame;
}
