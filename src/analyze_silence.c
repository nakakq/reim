#include "reim/analyze_silence.h"

bool analyze_silence(vocoder_context_t* vocoder, const double* input, double threshold)
{
    const size_t fftsize = vocoder->fftsize;

    // calculate the RMS of the frame
    double frame_sum = 0.0, frame_sum_sqr = 0.0;
    for (size_t i = 1; i < fftsize + 1; i++) {
        const double x = input[i];
        frame_sum += x;
        frame_sum_sqr += x * x;
    }
    const double frame_rms2 = frame_sum_sqr / fftsize;

    // does the RMS below the silence threshold?
    return (frame_rms2 < threshold * threshold);
}
