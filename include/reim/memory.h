#ifndef __REIM_ALLOC_H__
#define __REIM_ALLOC_H__
#include "reim/defines.h"
REIM_BEGIN_EXTERN_C
#include <stdlib.h>

#define REIM_ALLOC(length, type) ((type*)malloc((length) * sizeof(type)))
#define REIM_ALLOC_SINGLE(type) REIM_ALLOC(1, type)
#define REIM_FREE(p) free(p)

// Allocate memories for a vector (one-dimensional) buffer
static inline double* allocate_vector(size_t length)
{
    return REIM_ALLOC(length, double);
}

// Allocate memories for a matrix (two-dimensional) buffer
static inline double** allocate_matrix(size_t row, size_t column)
{
    double** mat = REIM_ALLOC(row, double*);
    for (size_t i = 0; i < row; i++)
        mat[i] = REIM_ALLOC(column, double);
    return mat;
}

// Free the memories
static inline void free_vector(double* memory)
{
    REIM_FREE(memory);
}

// Free the matrix memories
static inline void free_matrix(double** memory, size_t row)
{
    for (size_t i = 0; i < row; i++)
        REIM_FREE(memory[i]);
    REIM_FREE(memory);
}

REIM_END_EXTERN_C
#endif
