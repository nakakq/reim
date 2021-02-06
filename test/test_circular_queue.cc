#include "doctest.h"
#include "reim/circular_queue.h"

TEST_CASE("circular queue")
{
    circular_queue_t* queue = create_circular_queue(4);

    SUBCASE("check single push")
    {
        double buffer[2] = { 1.0, 2.0 };

        CHECK(get_remaining_circular_queue(queue) == 0);
        // [ 0 0 0 0 ]

        CHECK(pop_circular_queue(queue) == 0.0);
        CHECK(get_remaining_circular_queue(queue) == 0);
        // [ 0 0 0 0 ] -> 0

        push_additive_circular_queue(queue, buffer, 2);
        CHECK(get_remaining_circular_queue(queue) == 2);
        // [ 1 2 0 0 ]

        CHECK(pop_circular_queue(queue) == 1.0);
        CHECK(get_remaining_circular_queue(queue) == 1);
        // [ 2 0 0 0 ] -> 1

        CHECK(pop_circular_queue(queue) == 2.0);
        CHECK(get_remaining_circular_queue(queue) == 0);
        // [ 0 0 0 0 ] -> 2

        CHECK(pop_circular_queue(queue) == 0.0);
        CHECK(get_remaining_circular_queue(queue) == 0);
        // [ 0 0 0 0 ] -> 0
    }

    SUBCASE("check additive buffer")
    {
        double buffer[2] = { 1.0, 2.0 };

        push_additive_circular_queue(queue, buffer, 2);
        CHECK(get_remaining_circular_queue(queue) == 2);
        // [ 1 2 0 0 ]

        CHECK(pop_circular_queue(queue) == 1.0);
        CHECK(get_remaining_circular_queue(queue) == 1);
        // [ 2 0 0 0 ] -> 1

        push_additive_circular_queue(queue, buffer, 2);
        CHECK(get_remaining_circular_queue(queue) == 2);
        // [ 3 2 0 0 ]

        CHECK(pop_circular_queue(queue) == 3.0);
        CHECK(get_remaining_circular_queue(queue) == 1);
        // [ 2 0 0 0 ] -> 3

        CHECK(pop_circular_queue(queue) == 2.0);
        CHECK(get_remaining_circular_queue(queue) == 0);
        // [ 0 0 0 0 ] -> 2
    }

    SUBCASE("check fill")
    {
        double buffer[5] = { 1.0, 2.0, 3.0, 4.0, 5.0 };

        push_additive_circular_queue(queue, buffer, 5);
        CHECK(get_remaining_circular_queue(queue) == 4);
        // [ 2 3 4 5 ]

        CHECK(pop_circular_queue(queue) == 2.0);
        CHECK(get_remaining_circular_queue(queue) == 3);
        // [ 3 4 5 0 ] -> 2

        CHECK(pop_circular_queue(queue) == 3.0);
        CHECK(get_remaining_circular_queue(queue) == 2);
        // [ 4 5 0 0 ] -> 3

        CHECK(pop_circular_queue(queue) == 4.0);
        CHECK(get_remaining_circular_queue(queue) == 1);
        // [ 5 0 0 0 ] -> 4

        CHECK(pop_circular_queue(queue) == 5.0);
        CHECK(get_remaining_circular_queue(queue) == 0);
        // [ 0 0 0 0 ] -> 5
    }

    destroy_circular_queue(&queue);
}
