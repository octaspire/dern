#include "../src/octaspire_memory.c"
#include <stdint.h>
#include "external/greatest.h"
#include "octaspire/core/octaspire_memory.h"
#include "octaspire/core/octaspire_helpers.h"

TEST octaspire_memory_allocator_new_test(void)
{
    octaspire_memory_allocator_t *allocator = octaspire_memory_allocator_new(0);

    ASSERT(allocator);

    ASSERT_EQ(0, allocator->numberOfFutureAllocationsToBeRigged);
    ASSERT_EQ(0, allocator->bitIndex);

    size_t const bitQueueLen = sizeof(allocator->bitQueue) / sizeof(allocator->bitQueue[0]);
    ASSERT_EQ(20, bitQueueLen);

    for (size_t i = 0; i < bitQueueLen; ++i)
    {
        ASSERT_EQ(0, allocator->bitQueue[i]);
    }


    octaspire_memory_allocator_release(allocator);
    allocator = 0;

    PASS();
}

// How to test this efficiently, etc.?
/*
TEST octaspire_memory_allocator_new_failure_test(void)
{
    octaspire_memory_allocator_t *allocator = octaspire_memory_allocator_new();

    ASSERT_FALSE(allocator);

    octaspire_memory_allocator_release(allocator);
    allocator = 0;

    PASS();
}
*/

TEST octaspire_memory_allocator_malloc_test(void)
{
    octaspire_memory_allocator_t *allocator = octaspire_memory_allocator_new(0);

    size_t *ptrs[100];

    size_t const elemsize = sizeof(ptrs[0]);
    size_t const nelems   = sizeof(ptrs) / elemsize;

    for (size_t i = 0; i < nelems; ++i)
    {
        ptrs[i] = octaspire_memory_allocator_malloc(allocator, elemsize);
        ASSERT(ptrs[i]);
        ASSERT_EQ(0, *(ptrs[i]));
        *(ptrs[i]) = i;
    }

    for (size_t i = 0; i < nelems; ++i)
    {
        ASSERT_EQ(i, *(ptrs[i]));
    }

    for (size_t i = 0; i < nelems; ++i)
    {
        octaspire_memory_allocator_free(allocator, ptrs[i]);
        ptrs[i] = 0;
    }

    octaspire_memory_allocator_release(allocator);
    allocator = 0;

    PASS();
}

#if 0
TEST octaspire_memory_allocator_realloc_failure_test(void)
{
    octaspire_memory_allocator_t *allocator = octaspire_memory_allocator_new(0);

    ASSERT_EQ(0, octaspire_memory_allocator_realloc(allocator, 0, SIZE_MAX));

    octaspire_memory_allocator_release(allocator);
    allocator = 0;

    PASS();
}
#endif

TEST octaspire_memory_allocator_free_test(void)
{
    octaspire_memory_allocator_t *allocator = octaspire_memory_allocator_new(0);

    octaspire_memory_allocator_free(allocator, 0);
    octaspire_memory_allocator_free(allocator, octaspire_memory_allocator_malloc(allocator, 10));

    octaspire_memory_allocator_release(allocator);
    allocator = 0;

    PASS();
}

#if 0
TEST octaspire_memory_allocator_malloc_failure_test(void)
{
    octaspire_memory_allocator_t *allocator = octaspire_memory_allocator_new(0);

    ASSERT_EQ(0, octaspire_memory_allocator_malloc(allocator, SIZE_MAX));

    octaspire_memory_allocator_release(allocator);
    allocator = 0;

    PASS();
}
#endif

TEST octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged_when_larger_than_32_test(void)
{
    octaspire_memory_allocator_t *allocator = octaspire_memory_allocator_new(0);

    ASSERT_EQ(0, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    size_t const count = 640;

    uint32_t const bitPattern = 0x55; // 0101 0101

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged_when_larger_than_32(
        allocator,
        count,
        bitPattern,
        bitPattern,
        bitPattern,
        bitPattern,
        bitPattern,
        bitPattern,
        bitPattern,
        bitPattern,
        bitPattern,
        bitPattern,
        bitPattern,
        bitPattern,
        bitPattern,
        bitPattern,
        bitPattern,
        bitPattern,
        bitPattern,
        bitPattern,
        bitPattern,
        bitPattern);

   ASSERT_EQ(count, allocator->numberOfFutureAllocationsToBeRigged);
   ASSERT_EQ(0,     allocator->bitIndex);

   for (size_t i = 0; i < (sizeof(allocator->bitQueue) / sizeof(allocator->bitQueue[0])); ++i)
   {
       ASSERT_EQ(bitPattern, allocator->bitQueue[i]);
   }

   ASSERT_EQ(count, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

   for (size_t i = 0; i < count; ++i)
   {
       if (i % 2 == 0)
       {
           ASSERT(octaspire_memory_allocator_private_test_bit(allocator));
       }
       else
       {
           ASSERT_FALSE(octaspire_memory_allocator_private_test_bit(allocator));
       }

       ++(allocator->bitIndex);
   }

   ASSERT_EQ(count, allocator->bitIndex);

   octaspire_memory_allocator_release(allocator);
   allocator = 0;

   PASS();
}

TEST octaspire_memory_allocator_setting_and_getting_future_allocations_to_fail_and_using_with_malloc_test(void)
{
    octaspire_memory_allocator_t *allocator = octaspire_memory_allocator_new(0);

    ASSERT_EQ(0, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    size_t count = 32;

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, count, 0);

    ASSERT_EQ(count, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    for (size_t i = count; i > 0; --i)
    {
        ASSERT_FALSE(octaspire_memory_allocator_malloc(allocator, 1));
        ASSERT_EQ(i - 1, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));
    }

    ASSERT_EQ(0, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    void *ptr = octaspire_memory_allocator_malloc(allocator, 1);
    ASSERT(ptr);
    octaspire_memory_allocator_free(allocator, ptr);
    ptr = 0;

    ASSERT_EQ(0, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    octaspire_memory_allocator_release(allocator);
    allocator = 0;

    PASS();
}

TEST octaspire_memory_allocator_setting_and_getting_future_allocations_to_fail_and_using_with_realloc_test(void)
{
    octaspire_memory_allocator_t *allocator = octaspire_memory_allocator_new(0);

    ASSERT_EQ(0, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    void *buffer = octaspire_memory_allocator_malloc(allocator, 1);

    ASSERT(buffer);

    size_t count = 32;

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, count, 0);

    ASSERT_EQ(count, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    for (size_t i = count; i > 0; --i)
    {
        ASSERT_FALSE(octaspire_memory_allocator_realloc(allocator, buffer, 2));
        ASSERT_EQ(i - 1, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));
    }

    ASSERT_EQ(0, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    void *ptr = octaspire_memory_allocator_realloc(allocator, buffer, 2);
    ASSERT(ptr);
    octaspire_memory_allocator_free(allocator, ptr);
    ptr = 0;

    ASSERT_EQ(0, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    octaspire_memory_allocator_release(allocator);
    allocator = 0;

    PASS();
}



GREATEST_SUITE(octaspire_memory_suite)
{
    RUN_TEST(octaspire_memory_allocator_new_test);
    //RUN_TEST(octaspire_memory_allocator_new_failure_test);
    RUN_TEST(octaspire_memory_allocator_malloc_test);
    //RUN_TEST(octaspire_memory_allocator_realloc_failure_test);
    RUN_TEST(octaspire_memory_allocator_free_test);
    //RUN_TEST(octaspire_memory_allocator_malloc_failure_test);
    RUN_TEST(octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged_when_larger_than_32_test);
    RUN_TEST(octaspire_memory_allocator_setting_and_getting_future_allocations_to_fail_and_using_with_malloc_test);
    RUN_TEST(octaspire_memory_allocator_setting_and_getting_future_allocations_to_fail_and_using_with_realloc_test);
}

