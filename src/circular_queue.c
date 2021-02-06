#include "reim/circular_queue.h"

#include "reim/mathematics.h"
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

circular_queue_t* create_circular_queue(size_t capacity)
{
    circular_queue_t* queue = REIM_ALLOC_SINGLE(circular_queue_t);
    queue->head = 0;
    queue->remaining = 0;
    queue->capacity = capacity;
    queue->buffer = allocate_vector(capacity);
    for (size_t i = 0; i < capacity; i++) {
        queue->buffer[i] = 0.0;
    }
    return queue;
}

void destroy_circular_queue(circular_queue_t** queue)
{
    free_vector((*queue)->buffer);
    REIM_FREE(*queue);
    *queue = NULL;
}

size_t get_remaining_circular_queue(const circular_queue_t* queue)
{
    return queue->remaining;
}

void push_additive_circular_queue(circular_queue_t* queue, const double* buffer, size_t size)
{
    size_t index = queue->head;
    for (size_t i = 0; i < size; i++) {
        double value = buffer[i];
        if (i >= queue->capacity) {
            queue->buffer[index] = value; // overwrite when overflow
            queue->head = next(queue->head, queue->capacity);
        } else {
            queue->buffer[index] += value;
        }
        index = next(index, queue->capacity);
    }
    queue->remaining = CLAMP(size, queue->remaining, queue->capacity);
}

double pop_circular_queue(circular_queue_t* queue)
{
    if (queue->remaining == 0) {
        return 0.0;
    }

    double value = queue->buffer[queue->head];
    queue->buffer[queue->head] = 0.0;

    queue->head = next(queue->head, queue->capacity);
    queue->remaining--;

    return value;
}
