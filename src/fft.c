#include "reim/fft.h"

#include "reim/memory.h"
#include <stdlib.h>

// Library dependent implementations
#ifdef REIM_USE_MKL // Intel MKL

#include <mkl.h>

typedef struct {
    size_t fftsize;
    DFTI_DESCRIPTOR_HANDLE descriptor;
    double* buffer;
} fft_mkl_t;

fft_t* create_fft(size_t fftsize)
{
    fft_mkl_t* mkl = REIM_ALLOC_SINGLE(fft_mkl_t);
    mkl->fftsize = fftsize;

    MKL_LONG err;
    if ((err = DftiCreateDescriptor(&mkl->descriptor, DFTI_DOUBLE, DFTI_COMPLEX, 1, mkl->fftsize))) {
        // puts(DftiErrorMessage(err));
        REIM_FREE(mkl);
        return NULL;
    }

    if ((err = DftiCommitDescriptor(mkl->descriptor))) {
        // puts(DftiErrorMessage(err));
        REIM_FREE(mkl);
        return NULL;
    }

    mkl->buffer = allocate_vector(mkl->fftsize * 2);
    return (fft_t*)mkl;
}

ifft_t* create_ifft(size_t fftsize)
{
    return (ifft_t*)create_fft(fftsize);
}

void destroy_fft(fft_t** fft)
{
    fft_mkl_t* mkl = (fft_mkl_t*)*fft;
    DftiFreeDescriptor(&mkl->descriptor);
    free_vector(mkl->buffer);
    REIM_FREE(mkl);
    *fft = NULL;
}

void destroy_ifft(ifft_t** ifft)
{
    destroy_fft((fft_t**)ifft);
}

void execute_fft(fft_t* fft, double* real, double* imag)
{
    fft_mkl_t* mkl = (fft_mkl_t*)fft;

    for (size_t i = 0; i < mkl->fftsize; i++) {
        mkl->buffer[i * 2 + 0] = real[i];
        mkl->buffer[i * 2 + 1] = imag[i];
    }
    DftiComputeForward(mkl->descriptor, mkl->buffer);
    for (size_t i = 0; i < mkl->fftsize; i++) {
        real[i] = mkl->buffer[i * 2 + 0];
        imag[i] = mkl->buffer[i * 2 + 1];
    }
}

void execute_ifft(ifft_t* ifft, double* real, double* imag)
{
    fft_mkl_t* mkl = (fft_mkl_t*)ifft;
    double scale = mkl->fftsize;

    for (size_t i = 0; i < mkl->fftsize; i++) {
        mkl->buffer[i * 2 + 0] = real[i];
        mkl->buffer[i * 2 + 1] = imag[i];
    }
    DftiComputeBackward(mkl->descriptor, mkl->buffer);
    for (size_t i = 0; i < mkl->fftsize; i++) {
        real[i] = mkl->buffer[i * 2 + 0] / scale;
        imag[i] = mkl->buffer[i * 2 + 1] / scale;
    }
}

const char* get_fft_library_name()
{
    return "MKL";
}

#elif defined REIM_USE_FFTW3 // FFTW 3

#include <fftw3.h>

typedef struct {
    size_t fftsize;
    fftw_complex* buffer;
    fftw_plan plan;
} fftw3_t;

fft_t* create_fft(size_t fftsize)
{
    fftw3_t* fftw = REIM_ALLOC_SINGLE(fftw3_t);
    fftw->fftsize = fftsize;
    fftw->buffer = (fftw_complex*)fftw_malloc(fftw->fftsize * sizeof(fftw_complex));
    fftw->plan = fftw_plan_dft_1d(fftsize, fftw->buffer, fftw->buffer, FFTW_FORWARD, FFTW_MEASURE);
    return (fft_t*)fftw;
}

ifft_t* create_ifft(size_t fftsize)
{
    fftw3_t* fftw = REIM_ALLOC_SINGLE(fftw3_t);
    fftw->fftsize = fftsize;
    fftw->buffer = (fftw_complex*)fftw_malloc(fftw->fftsize * sizeof(fftw_complex));
    fftw->plan = fftw_plan_dft_1d(fftsize, fftw->buffer, fftw->buffer, FFTW_BACKWARD, FFTW_MEASURE);
    return (ifft_t*)fftw;
}

void destroy_fft(fft_t** fft)
{
    fftw3_t* fftw = (fftw3_t*)*fft;
    fftw_destroy_plan(fftw->plan);
    fftw_free(fftw->buffer);
    REIM_FREE(fftw);
    *fft = NULL;
}

void destroy_ifft(ifft_t** ifft)
{
    destroy_fft((fft_t**)ifft);
}

void execute_fft(fft_t* fft, double* real, double* imag)
{
    fftw3_t* fftw = (fftw3_t*)fft;

    for (size_t i = 0; i < fftw->fftsize; i++) {
        fftw->buffer[i][0] = real[i];
        fftw->buffer[i][1] = imag[i];
    }
    fftw_execute(fftw->plan);
    for (size_t i = 0; i < fftw->fftsize; i++) {
        real[i] = fftw->buffer[i][0];
        imag[i] = fftw->buffer[i][1];
    }
}

void execute_ifft(ifft_t* ifft, double* real, double* imag)
{
    fftw3_t* fftw = (fftw3_t*)ifft;
    double scale = fftw->fftsize;

    for (size_t i = 0; i < fftw->fftsize; i++) {
        fftw->buffer[i][0] = real[i];
        fftw->buffer[i][1] = imag[i];
    }
    fftw_execute(fftw->plan);
    for (size_t i = 0; i < fftw->fftsize; i++) {
        real[i] = fftw->buffer[i][0] / scale;
        imag[i] = fftw->buffer[i][1] / scale;
    }
}

const char* get_fft_library_name()
{
    return "FFTW3";
}

#else // fftsg (internal implementation)

#include <math.h>

// fftsg.c
void cdft(int n, int isgn, double* a, int* ip, double* w);

typedef struct {
    size_t fftsize;
    double* buffer;
    int* work;
    double* table;
} fftsg_t;

fft_t* create_fft(size_t fftsize)
{
    fftsg_t* ooura = REIM_ALLOC_SINGLE(fftsg_t);
    ooura->fftsize = fftsize;
    ooura->buffer = allocate_vector(ooura->fftsize * 2);
    ooura->work = REIM_ALLOC(2 + ceil(sqrt(ooura->fftsize)), int);
    ooura->table = allocate_vector(ooura->fftsize / 2);
    ooura->work[0] = 0.0;
    return (fft_t*)ooura;
}

ifft_t* create_ifft(size_t fftsize)
{
    return (ifft_t*)create_fft(fftsize);
}

void destroy_fft(fft_t** fft)
{
    fftsg_t* ooura = (fftsg_t*)*fft;
    free_vector(ooura->buffer);
    REIM_FREE(ooura->work);
    free_vector(ooura->table);
    REIM_FREE(ooura);
    *fft = NULL;
}

void destroy_ifft(ifft_t** ifft)
{
    destroy_fft((fft_t**)ifft);
}

void execute_fft(fft_t* fft, double* real, double* imag)
{
    fftsg_t* ooura = (fftsg_t*)fft;

    for (size_t i = 0; i < ooura->fftsize; i++) {
        ooura->buffer[i * 2 + 0] = real[i];
        ooura->buffer[i * 2 + 1] = imag[i];
    }
    cdft(ooura->fftsize * 2, -1, ooura->buffer, ooura->work, ooura->table);
    for (size_t i = 0; i < ooura->fftsize; i++) {
        real[i] = ooura->buffer[i * 2 + 0];
        imag[i] = ooura->buffer[i * 2 + 1];
    }
}

void execute_ifft(ifft_t* ifft, double* real, double* imag)
{
    fftsg_t* ooura = (fftsg_t*)ifft;
    double scale = ooura->fftsize;

    for (size_t i = 0; i < ooura->fftsize; i++) {
        ooura->buffer[i * 2 + 0] = real[i];
        ooura->buffer[i * 2 + 1] = imag[i];
    }
    cdft(ooura->fftsize * 2, +1, ooura->buffer, ooura->work, ooura->table);
    for (size_t i = 0; i < ooura->fftsize; i++) {
        real[i] = ooura->buffer[i * 2 + 0] / scale;
        imag[i] = ooura->buffer[i * 2 + 1] / scale;
    }
}

const char* get_fft_library_name()
{
    return "fftsg";
}

#endif
