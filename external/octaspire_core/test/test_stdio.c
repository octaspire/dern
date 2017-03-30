#include "../src/octaspire_stdio.c"
#include <assert.h>
#include "external/greatest.h"
#include "octaspire/core/octaspire_stdio.h"
#include "octaspire/core/octaspire_core_config.h"

static octaspire_memory_allocator_t *allocator = 0;

TEST octaspire_stdio_new_test(void)
{
    octaspire_stdio_t *stdio = octaspire_stdio_new(allocator);

    ASSERT(stdio);

    ASSERT_EQ(0, stdio->numberOfFutureReadsToBeRigged);
    ASSERT_EQ(0, stdio->bitIndex);
    ASSERT_EQ(0, stdio->bitQueue);
    ASSERT_EQ(allocator, stdio->allocator);

    octaspire_stdio_release(stdio);
    stdio = 0;

    PASS();
}

TEST octaspire_stdio_new_allocation_failure_test(void)
{
    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(
        allocator,
        1,
        0);

    ASSERT_EQ(
        1,
        octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    octaspire_stdio_t *stdio = octaspire_stdio_new(allocator);

    ASSERT_FALSE(stdio);

    ASSERT_EQ(
        0,
        octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    octaspire_stdio_release(stdio);
    stdio = 0;

    PASS();
}

TEST octaspire_stdio_fread_test(void)
{
    octaspire_stdio_t *stdio = octaspire_stdio_new(allocator);

    ASSERT(stdio);

    FILE *f = fopen(OCTASPIRE_CORE_CONFIG_TEST_RES_PATH "octaspire_stdio_fread_test", "rb");

    ASSERT(f);

    char octet = 0;
    char *expected = "0123456789qwertyuiop";

    for (size_t i = 0; i < strlen(expected); ++i)
    {
        ASSERT_EQ(1, octaspire_stdio_fread(stdio, &octet, sizeof(octet), 1, f));
        ASSERT_EQ(expected[i], octet);
    }

    fclose(f);
    f = 0;

    octaspire_stdio_release(stdio);
    stdio = 0;

    PASS();
}

TEST octaspire_stdio_fread_rigging_and_failure_test(void)
{
    octaspire_stdio_t *stdio = octaspire_stdio_new(allocator);

    ASSERT(stdio);

    FILE *f = fopen(OCTASPIRE_CORE_CONFIG_TEST_RES_PATH "octaspire_stdio_fread_test", "rb");

    ASSERT(f);

    char octet = 0;
    char *expected = "0123456789qwertyuiop";

    // 0101 0101 0101 0101 0101
    //    5    5    5    5    5
    octaspire_stdio_set_number_and_type_of_future_reads_to_be_rigged(stdio, 20, 0x55555);

    ASSERT_EQ(20, octaspire_stdio_get_number_of_future_reads_to_be_rigged(stdio));

    size_t index = 0;
    for (size_t i = 0; i < strlen(expected); ++i)
    {
        if (i % 2)
        {
            ASSERT_EQ(0, octaspire_stdio_fread(stdio, &octet, sizeof(octet), 1, f));
        }
        else
        {
            ASSERT_EQ(1, octaspire_stdio_fread(stdio, &octet, sizeof(octet), 1, f));
            ASSERT_EQ(expected[index], octet);
            ++index;
        }
    }

    ASSERT_EQ(0, octaspire_stdio_get_number_of_future_reads_to_be_rigged(stdio));

    fclose(f);
    f = 0;

    octaspire_stdio_release(stdio);
    stdio = 0;

    PASS();
}

GREATEST_SUITE(octaspire_stdio_suite)
{
    size_t numTimesRun = 0;

    allocator = octaspire_memory_allocator_new_create_region(
        OCTASPIRE_CORE_CONFIG_MEMORY_ALLOCATOR_REGION_MIN_BLOCK_SIZE_IN_OCTETS);

second_run:

    assert(allocator);

    RUN_TEST(octaspire_stdio_new_test);
    RUN_TEST(octaspire_stdio_new_allocation_failure_test);
    RUN_TEST(octaspire_stdio_fread_test);
    RUN_TEST(octaspire_stdio_fread_rigging_and_failure_test);

    octaspire_memory_allocator_release(allocator);
    allocator = 0;

    ++numTimesRun;

    if (numTimesRun < 2)
    {
        // Second run without region allocator

        allocator      = octaspire_memory_allocator_new(0);

        goto second_run;
    }
}

