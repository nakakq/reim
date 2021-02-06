#include "reim/mathematics.h"

#include <limits.h>
#include <stdint.h>

void setup_random(random_state_t state)
{
    state[0] = 123456789;
    state[1] = 362436069;
    state[2] = 521288629;
    state[3] = 88675123;
}

double generate_uniform_random(random_state_t state)
{
    // Xorshift random number generator (2^128-1 period)
    uint32_t t = state[3];
    uint32_t s = state[0];
    state[3] = state[2];
    state[2] = state[1];
    state[1] = s;
    t ^= t << 11;
    state[0] = (t ^ (t >> 8)) ^ (s ^ (s >> 19));
    return (double)state[0] / UINT32_MAX;
}

void ifftshift(const double* source, double* destination, size_t numbins)
{
    for (size_t k = 0; k < numbins - 1; k++) {
        destination[k] = source[numbins - 1 + k];
        destination[numbins - 1 + k] = source[k];
    }
}
