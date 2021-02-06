#ifndef __REIM_CIRCULAR_BUFFER_H__
#define __REIM_CIRCULAR_BUFFER_H__
#include "reim/defines.h"
#include <stddef.h>
REIM_BEGIN_EXTERN_C

// Circular buffer is 
typedef struct {
    size_t head;
    size_t capacity;
    double* buffer;
} circular_buffer_t;

// Create a new circular buffer
circular_buffer_t* create_circular_buffer(size_t capacity);

// Destroy the circular buffer
void destroy_circular_buffer(circular_buffer_t** cb);

// Push the value to the circular buffer
void push_circular_buffer(circular_buffer_t* cb, double value);

// Copy the all buffer content to the destination buffer
void copy_all_circular_buffer(circular_buffer_t* cb, double* destination);

REIM_END_EXTERN_C
#endif
