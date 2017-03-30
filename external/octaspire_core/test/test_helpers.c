#include "../src/octaspire_helpers.c"
#include "external/greatest.h"
#include "octaspire/core/octaspire_stdio.h"
#include "octaspire/core/octaspire_core_config.h"

static octaspire_memory_allocator_t *allocator = 0;
static octaspire_stdio_t            *octaspireStdio = 0;

TEST octaspire_helpers_test_bit_when_only_highest_order_bit_is_set_test(void)
{
    uint32_t const bitSet = 0x80000000;

    for (size_t i = 0; i < 31; ++i)
    {
        ASSERT_FALSE(octaspire_helpers_test_bit(bitSet, i));
    }

    ASSERT(octaspire_helpers_test_bit(bitSet, 31));

    PASS();
}

TEST octaspire_helpers_test_bit_when_only_lowest_order_bit_is_set_test(void)
{
    uint32_t const bitSet = 0x00000001;

    ASSERT(octaspire_helpers_test_bit(bitSet, 0));

    for (size_t i = 1; i < 32; ++i)
    {
        ASSERT_FALSE(octaspire_helpers_test_bit(bitSet, i));
    }

    PASS();
}

TEST octaspire_helpers_test_bit_when_all_bits_are_set_test(void)
{
    uint32_t const bitSet = 0xFFFFFFFF;

    for (size_t i = 0; i < 32; ++i)
    {
        ASSERT(octaspire_helpers_test_bit(bitSet, i));
    }

    PASS();
}

TEST octaspire_helpers_test_bit_when_no_bits_are_set_test(void)
{
    uint32_t const bitSet = 0x00000000;

    for (size_t i = 0; i < 32; ++i)
    {
        ASSERT_FALSE(octaspire_helpers_test_bit(bitSet, i));
    }

    PASS();
}

TEST octaspire_helpers_test_bit_when_every_bit_at_even_index_is_set_test(void)
{
    uint32_t const bitSet = 0x55555555;

    for (size_t i = 0; i < 32; ++i)
    {
        if (i % 2 == 0)
        {
            ASSERT(octaspire_helpers_test_bit(bitSet, i));
        }
        else
        {
            ASSERT_FALSE(octaspire_helpers_test_bit(bitSet, i));
        }
    }

    PASS();
}

TEST octaspire_helpers_test_bit_when_every_bit_at_odd_index_is_set_test(void)
{
    uint32_t const bitSet = 0xAAAAAAAA;

    for (size_t i = 0; i < 32; ++i)
    {
        if (i % 2 != 0)
        {
            ASSERT(octaspire_helpers_test_bit(bitSet, i));
        }
        else
        {
            ASSERT_FALSE(octaspire_helpers_test_bit(bitSet, i));
        }
    }

    PASS();
}

TEST octaspire_helpers_path_to_buffer_test(void)
{
    size_t octetsAllocated = 0;

    char * buffer = octaspire_helpers_path_to_buffer(
        OCTASPIRE_CORE_CONFIG_TEST_RES_PATH "octaspire_helpers_path_to_buffer_test",
        &octetsAllocated,
        allocator,
        octaspireStdio);

    ASSERT(buffer);
    ASSERT_EQ(2 + 3 + 4, octetsAllocated);

    // Copyright sign 0xa9               2 octet
    // Equal sign     0x2260             3 octets
    // Linear b syllable b008 a 0x10000  4 octets
    char const * const expected = "\xC2\xA9\xE2\x89\xA0\xF0\x90\x80\x80";

    ASSERT_EQ(octetsAllocated, strlen(expected));

    ASSERT_MEM_EQ(expected, buffer, octetsAllocated);

    octaspire_memory_allocator_free(allocator, buffer);
    buffer = 0;

    PASS();
}

TEST octaspire_helpers_path_to_buffer_failure_on_nonexisting_file_test(void)
{
    size_t octetsAllocated = 0;

    char * buffer = octaspire_helpers_path_to_buffer(
        OCTASPIRE_CORE_CONFIG_TEST_RES_PATH "octaspire_helpers_path_to_buffer_failure_on_nonexisting_file_test",
        &octetsAllocated,
        allocator,
        octaspireStdio);

    ASSERT_FALSE(buffer);
    ASSERT_EQ(0, octetsAllocated);

    octaspire_memory_allocator_free(allocator, buffer);
    buffer = 0;

    PASS();
}

TEST octaspire_helpers_path_to_buffer_failure_on_empty_file_test(void)
{
    size_t octetsAllocated = 0;

    char * buffer = octaspire_helpers_path_to_buffer(
        OCTASPIRE_CORE_CONFIG_TEST_RES_PATH "octaspire_helpers_path_to_buffer_failure_on_empty_file_test",
        &octetsAllocated,
        allocator,
        octaspireStdio);

    ASSERT_FALSE(buffer);
    ASSERT_EQ(0, octetsAllocated);

    octaspire_memory_allocator_free(allocator, buffer);
    buffer = 0;

    PASS();
}

TEST octaspire_helpers_path_to_buffer_allocation_failure_test(void)
{
    size_t octetsAllocated = 0;

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 1, 0);
    ASSERT_EQ(1, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    char * buffer = octaspire_helpers_path_to_buffer(
        OCTASPIRE_CORE_CONFIG_TEST_RES_PATH "octaspire_helpers_path_to_buffer_test",
        &octetsAllocated,
        allocator,
        octaspireStdio);

    ASSERT_EQ(0, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    ASSERT_FALSE(buffer);
    ASSERT_EQ(0, octetsAllocated);

    octaspire_memory_allocator_free(allocator, buffer);
    buffer = 0;

    PASS();
}

TEST octaspire_helpers_path_to_buffer_read_failure_test(void)
{
    size_t octetsAllocated = 0;

    octaspire_stdio_set_number_and_type_of_future_reads_to_be_rigged(octaspireStdio, 1, 0);
    ASSERT_EQ(1, octaspire_stdio_get_number_of_future_reads_to_be_rigged(octaspireStdio));

    char * buffer = octaspire_helpers_path_to_buffer(
        OCTASPIRE_CORE_CONFIG_TEST_RES_PATH "octaspire_helpers_path_to_buffer_test",
        &octetsAllocated,
        allocator,
        octaspireStdio);

    ASSERT_EQ(0, octaspire_stdio_get_number_of_future_reads_to_be_rigged(octaspireStdio));

    ASSERT_FALSE(buffer);
    ASSERT_EQ(0, octetsAllocated);

    octaspire_memory_allocator_free(allocator, buffer);
    buffer = 0;

    PASS();
}

GREATEST_SUITE(octaspire_helpers_suite)
{
    size_t numTimesRun = 0;

    allocator       = octaspire_memory_allocator_new_create_region(
        OCTASPIRE_CORE_CONFIG_MEMORY_ALLOCATOR_REGION_MIN_BLOCK_SIZE_IN_OCTETS);

    octaspireStdio  = octaspire_stdio_new(allocator);

second_run:

    assert(allocator);
    assert(octaspireStdio);

    RUN_TEST(octaspire_helpers_test_bit_when_only_highest_order_bit_is_set_test);
    RUN_TEST(octaspire_helpers_test_bit_when_only_lowest_order_bit_is_set_test);
    RUN_TEST(octaspire_helpers_test_bit_when_all_bits_are_set_test);
    RUN_TEST(octaspire_helpers_test_bit_when_no_bits_are_set_test);
    RUN_TEST(octaspire_helpers_test_bit_when_every_bit_at_even_index_is_set_test);
    RUN_TEST(octaspire_helpers_test_bit_when_every_bit_at_odd_index_is_set_test);
    RUN_TEST(octaspire_helpers_path_to_buffer_test);
    RUN_TEST(octaspire_helpers_path_to_buffer_failure_on_nonexisting_file_test);
    RUN_TEST(octaspire_helpers_path_to_buffer_failure_on_empty_file_test);
    RUN_TEST(octaspire_helpers_path_to_buffer_allocation_failure_test);
    RUN_TEST(octaspire_helpers_path_to_buffer_read_failure_test);

    octaspire_stdio_release(octaspireStdio);
    octaspireStdio = 0;

    octaspire_memory_allocator_release(allocator);
    allocator = 0;

    ++numTimesRun;

    if (numTimesRun < 2)
    {
        // Second run without region allocator

        allocator      = octaspire_memory_allocator_new(0);
        octaspireStdio = octaspire_stdio_new(allocator);

        goto second_run;
    }
}

