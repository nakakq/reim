#include "doctest.h"
#include "isapprox.hh"
#include "reim/mathematics.h"

TEST_CASE("max/min")
{
    CHECK(MAX(1.0, 1.0) == 1.0);
    CHECK(MAX(1.5, 1.0) == 1.5);
    CHECK(MAX(1.0, 1.5) == 1.5);
    CHECK(MIN(1.0, 1.0) == 1.0);
    CHECK(MIN(0.5, 1.0) == 0.5);
    CHECK(MIN(1.0, 0.5) == 0.5);
}

TEST_CASE("clamp")
{
    CHECK(CLAMP(0.999, -1.0, 1.0) == 0.999);
    CHECK(CLAMP(-0.999, -1.0, 1.0) == -0.999);

    CHECK(CLAMP(8.0, -1.0, 1.0) == 1.0);
    CHECK(CLAMP(-8.0, -1.0, 1.0) == -1.0);

    CHECK(CLAMP(6.6, 2.2, 4.4) == 4.4);
    CHECK(CLAMP(-6.6, 2.2, 4.4) == 2.2);

    CHECK(CLAMP(6.6, 2.2, 2.2) == 2.2);
    CHECK(CLAMP(-6.6, 2.2, 2.2) == 2.2);
}

TEST_CASE("clampindex")
{
    CHECK(CLAMP_INDEX(0, 8) == 0);
    CHECK(CLAMP_INDEX(3, 8) == 3);
    CHECK(CLAMP_INDEX(7, 8) == 7);

    CHECK(CLAMP_INDEX(-1, 8) == 0);
    CHECK(CLAMP_INDEX(-999, 8) == 0);
    CHECK(CLAMP_INDEX(8, 8) == 7);
    CHECK(CLAMP_INDEX(999, 8) == 7);

    CHECK(CLAMP_INDEX(-1, 100) == 0);
    CHECK(CLAMP_INDEX(-999, 100) == 0);
    CHECK(CLAMP_INDEX(100, 100) == 99);
    CHECK(CLAMP_INDEX(999, 100) == 99);

    CHECK(CLAMP_INDEX(999, 0) == 0);
}

TEST_CASE("ispow2")
{
    SUBCASE("check 1-8")
    {
        CHECK(ISPOW2(1));
        CHECK(ISPOW2(2));
        CHECK(!ISPOW2(3));
        CHECK(ISPOW2(4));
        CHECK(!ISPOW2(5));
        CHECK(!ISPOW2(6));
        CHECK(!ISPOW2(7));
        CHECK(ISPOW2(8));
    }

    SUBCASE("check negative number")
    {
        CHECK(!ISPOW2(-3));
        CHECK(!ISPOW2(-2));
        CHECK(!ISPOW2(-1));
        CHECK(!ISPOW2(0));
    }
}

TEST_CASE("abs2")
{
    CHECK(COMPLEX_ABS2(0, 0) == 0);
    CHECK(COMPLEX_ABS2(1, 1) == 2);
    CHECK(COMPLEX_ABS2(1, -1) == 2);
    CHECK(COMPLEX_ABS2(-1, 1) == 2);
    CHECK(COMPLEX_ABS2(-1, -1) == 2);
}

TEST_CASE("abs")
{
    CHECK(COMPLEX_ABS(0, 0) == 0);
    CHECK(COMPLEX_ABS(3, 4) == 5);
    CHECK(COMPLEX_ABS(3, -4) == 5);
    CHECK(COMPLEX_ABS(-3, 4) == 5);
    CHECK(COMPLEX_ABS(-3, -4) == 5);
}

TEST_CASE("angle")
{
    CHECK(COMPLEX_ANGLE(0, 0) == 0.0);
    CHECK(isapprox(COMPLEX_ANGLE(1, 1), 45.0 * REIM_PI / 180.0, 1e-8));
    CHECK(isapprox(COMPLEX_ANGLE(1, -1), -45.0 * REIM_PI / 180.0, 1e-8));
    CHECK(isapprox(COMPLEX_ANGLE(-1, -1), -135.0 * REIM_PI / 180.0, 1e-8));
}

TEST_CASE("instfreq")
{
    CHECK(INSTFREQ(0, 0, 0, 0, 1) == 0);
    CHECK(isapprox(INSTFREQ(1.0, 0.0, 0.0, 1.0, 4.0), 1.0, 1e-8));
    CHECK(isapprox(INSTFREQ(0.0, 1.0, 1.0, 0.0, 4.0), 1.0, 1e-8));
}

TEST_CASE("ifftshift")
{
    double src[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    double dst[8] = { 9, 9, 9, 9, 9, 9, 9, 9 };
    ifftshift(src, dst, 5);
    CHECK(dst[0] == 5);
    CHECK(dst[1] == 6);
    CHECK(dst[2] == 7);
    CHECK(dst[3] == 8);
    CHECK(dst[4] == 1);
    CHECK(dst[5] == 2);
    CHECK(dst[6] == 3);
    CHECK(dst[7] == 4);
}
