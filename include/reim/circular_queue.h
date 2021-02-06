#ifndef __REIM_CIRCULAR_QUEUE_H__
#define __REIM_CIRCULAR_QUEUE_H__
#include "reim/defines.h"
#include <stdbool.h>
#include <stddef.h>
REIM_BEGIN_EXTERN_C

typedef struct {
    size_t head;
    size_t remaining;
    size_t capacity;
    double* buffer;
} circular_queue_t;

// Create a new queue
circular_queue_t* create_circular_queue(size_t capacity);

// Destroy the queue
void destroy_circular_queue(circular_queue_t** queue);

// Get the number of remaining values to pop 
size_t get_remaining_circular_queue(const circular_queue_t* queue);

// Push the value to the queue
void push_additive_circular_queue(circular_queue_t* queue, const double* buffer, size_t size);

// Pop the value from the queue
double pop_circular_queue(circular_queue_t* queue);

REIM_END_EXTERN_C
#endif
