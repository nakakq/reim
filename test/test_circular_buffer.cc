#include "doctest.h"
#include "reim/circular_buffer.h"

TEST_CASE("circular buffer")
{
    double buffer[4] = { 12, 34, 56, 78 }; // dummy values
    circular_buffer_t* cb = create_circular_buffer(4);

    SUBCASE("check initial state")
    {
        copy_all_circular_buffer(cb, buffer);
        CHECK(buffer[0] == 0.0);
        CHECK(buffer[1] == 0.0);
        CHECK(buffer[2] == 0.0);
        CHECK(buffer[3] == 0.0);
    }

    SUBCASE("check intermediate state")
    {
        push_circular_buffer(cb, 1.0);
        push_circular_buffer(cb, 2.0);
        copy_all_circular_buffer(cb, buffer);
        CHECK(buffer[0] == 0.0);
        CHECK(buffer[1] == 0.0);
        CHECK(buffer[2] == 1.0);
        CHECK(buffer[3] == 2.0);
    }

    SUBCASE("check overflow state")
    {
        push_circular_buffer(cb, 1.0);
        push_circular_buffer(cb, 2.0);
        push_circular_buffer(cb, 3.0);
        push_circular_buffer(cb, 4.0);
        push_circular_buffer(cb, 5.0);
        copy_all_circular_buffer(cb, buffer);
        CHECK(buffer[0] == 2.0);
        CHECK(buffer[1] == 3.0);
        CHECK(buffer[2] == 4.0);
        CHECK(buffer[3] == 5.0);
    }

    destroy_circular_buffer(&cb);
}
