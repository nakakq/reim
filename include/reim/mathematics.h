#ifndef __REIM_MATHEMATICS_H__
#define __REIM_MATHEMATICS_H__
#include "reim/defines.h"
#include <stddef.h>
#include <stdint.h>
#include <math.h>
REIM_BEGIN_EXTERN_C

#define REIM_PI 3.14159265358979
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define CLAMP(x, a, b) MIN(MAX(x, a), b)
#define CLAMP_INDEX(x, len) MIN(MAX(x, 0), ((len) == 0) ? 0 : (len)-1)
#define ISPOW2(x) (((x) > 0) && ((((x) & ((x)-1))) == 0))
#define COMPLEX_ABS2(re, im) ((re) * (re) + (im) * (im))
#define COMPLEX_ABS(re, im) sqrt(COMPLEX_ABS2(re, im))
#define COMPLEX_ANGLE(re, im) atan2(im, re)
#define INSTFREQ(xr1, xi1, xr2, xi2, fs) fabs((fs) / (2 * REIM_PI) * COMPLEX_ANGLE((xr1) * (xr2) + (xi1) * (xi2), (xi1) * (xr2) - (xr1) * (xi2)))

typedef uint32_t random_state_t[4];

// Initialize the random state
void setup_random(random_state_t state);

// Generate a random number in [0, 1] from uniform distribution
double generate_uniform_random(random_state_t state);

// Do ifftshift processing
void ifftshift(const double* source, double* destination, size_t numbins);

REIM_END_EXTERN_C
#endif
