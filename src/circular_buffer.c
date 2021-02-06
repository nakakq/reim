#include "reim/circular_buffer.h"

#include "reim/memory.h"
#include <stdlib.h>

static inline size_t next(size_t index, size_t capacity)
{
    index++;
    if (index >= capacity) {
        index = 0;
    }
    return index;
}

circular_buffer_t* create_circular_buffer(size_t capacity)
{
    circular_buffer_t* cb = REIM_ALLOC_SINGLE(circular_buffer_t);
    cb->head = 0;
    cb->capacity = capacity;
    cb->buffer = allocate_vector(capacity);
    for (size_t i = 0; i < capacity; i++) {
        cb->buffer[i] = 0.0;
    }
    return cb;
}

void destroy_circular_buffer(circular_buffer_t** cb)
{
    free_vector((*cb)->buffer);
    REIM_FREE(*cb);
    *cb = NULL;
}

void push_circular_buffer(circular_buffer_t* cb, double value)
{
    cb->head = next(cb->head, cb->capacity);
    cb->buffer[cb->head] = value;
}

void copy_all_circular_buffer(circular_buffer_t* cb, double* destination)
{
    size_t index = cb->head;
    for (size_t i = 0; i < cb->capacity; i++) {
        index = next(index, cb->capacity);
        destination[i] = cb->buffer[index];
    }
}
