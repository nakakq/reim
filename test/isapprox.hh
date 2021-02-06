#pragma once
#include <stddef.h>

namespace {

static bool isapprox(double a, double b, double eta)
{
    double diff = (a > b) ? a - b : b - a;
    return diff <= eta;
}

static bool isapprox_array(size_t size, double* a, double* b, double eta)
{
    bool is_all_matched = true;
    for (size_t i = 0; i < size; i++) {
        is_all_matched &= isapprox(a[i], b[i], eta);
    }
    return is_all_matched;
}

}
