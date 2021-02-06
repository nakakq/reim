#include "doctest.h"
#include "isapprox.hh"
#include "reim/fft.h"
#include <stdio.h>

TEST_CASE("FFT information")
{
    printf("Current FFT library: %s\n", get_fft_library_name());
}

TEST_CASE("FFT")
{
    const size_t fftsize = 8;
    fft_t* fft = create_fft(fftsize);

    SUBCASE("check FFT: complex-exp (1)")
    {
        double xr[8] = { +1, 0, -1, 0, +1, 0, -1, 0 };
        double xi[8] = { 0, +1, 0, -1, 0, +1, 0, -1 };
        double Xr[8] = { 0, 0, 8, 0, 0, 0, 0, 0 };
        double Xi[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
        execute_fft(fft, xr, xi);
        CHECK(isapprox_array(fftsize, xr, Xr, 1e-8));
        CHECK(isapprox_array(fftsize, xi, Xi, 1e-8));
    }

    SUBCASE("check FFT: complex-exp (2)")
    {
        double xr[8] = { 0, -1, 0, +1, 0, -1, 0, +1 };
        double xi[8] = { +1, 0, -1, 0, +1, 0, -1, 0 };
        double Xr[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
        double Xi[8] = { 0, 0, 8, 0, 0, 0, 0, 0 };
        execute_fft(fft, xr, xi);
        CHECK(isapprox_array(fftsize, xr, Xr, 1e-8));
        CHECK(isapprox_array(fftsize, xi, Xi, 1e-8));
    }

    destroy_fft(&fft);
}

TEST_CASE("IFFT")
{
    const size_t fftsize = 8;
    ifft_t* ifft = create_ifft(fftsize);

    SUBCASE("check IFFT: complex-exp (1)")
    {
        double Xr[8] = { 0, 0, 8, 0, 0, 0, 0, 0 };
        double Xi[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
        double xr[8] = { +1, 0, -1, 0, +1, 0, -1, 0 };
        double xi[8] = { 0, +1, 0, -1, 0, +1, 0, -1 };
        execute_ifft(ifft, Xr, Xi);
        CHECK(isapprox_array(fftsize, Xr, xr, 1e-8));
        CHECK(isapprox_array(fftsize, Xi, xi, 1e-8));
    }

    SUBCASE("check IFFT: complex-exp (2)")
    {
        double Xr[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
        double Xi[8] = { 0, 0, 8, 0, 0, 0, 0, 0 };
        double xr[8] = { 0, -1, 0, +1, 0, -1, 0, +1 };
        double xi[8] = { +1, 0, -1, 0, +1, 0, -1, 0 };
        execute_ifft(ifft, Xr, Xi);
        CHECK(isapprox_array(fftsize, Xr, xr, 1e-8));
        CHECK(isapprox_array(fftsize, Xi, xi, 1e-8));
    }

    destroy_ifft(&ifft);
}

// void check_fft()
// {
//     const int fftsize = 2048;
//     double xr[fftsize], xi[fftsize];
//     for (int i = 0; i < fftsize; i++) {
//         xr[i] = random_uniform();
//         xi[i] = random_uniform();
//     }
//     // original
//     {
//         FILE* fp = fopen("orig.csv", "w");
//         for (int i = 0; i < fftsize; i++) {
//             fprintf(fp, "%.15lf, %.15lf\n", xr[i], xi[i]);
//         }
//         fclose(fp);
//     }
//     // FFT result
//     {
//         fft_t* fft = create_fft(fftsize);
//         execute_fft(fft, xr, xi);
//         destroy_fft(&fft);
//         FILE* fp = fopen("fft.csv", "w");
//         for (int i = 0; i < fftsize; i++) {
//             fprintf(fp, "%.15lf, %.15lf\n", xr[i], xi[i]);
//         }
//         fclose(fp);
//     }
//     // IFFT result
//     {
//         ifft_t* ifft = create_ifft(fftsize);
//         execute_ifft(ifft, xr, xi);
//         destroy_ifft(&ifft);
//         FILE* fp = fopen("ifft.csv", "w");
//         for (int i = 0; i < fftsize; i++) {
//             fprintf(fp, "%.15lf, %.15lf\n", xr[i], xi[i]);
//         }
//         fclose(fp);
//     }
// }
