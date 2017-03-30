#include "../src/octaspire_container_pair.c"
#include "external/greatest.h"
#include "octaspire/core/octaspire_container_utf8_string.h"
#include "octaspire/core/octaspire_helpers.h"
#include "octaspire/core/octaspire_memory.h"
#include "octaspire/core/octaspire_core_config.h"

static octaspire_memory_allocator_t *allocator = 0;

TEST octaspire_container_pair_new_short_and_long_test(void)
{
    octaspire_container_pair_t *p = octaspire_container_pair_new(
        sizeof(short),
        false,
        sizeof(long),
        false,
        0,
        0,
        allocator);

    ASSERT(p);

    ASSERT(p->first);
    ASSERT(p->second);

    ASSERT_EQ(0, *(short const*)p->first);
    ASSERT_EQ(0, *(long const*)p->second);

    ASSERT_EQ(sizeof(short), p->firstSize);
    ASSERT_EQ(sizeof(long),  p->secondSize);
    ASSERT_EQ(0,             p->firstReleaseCallback);
    ASSERT_EQ(0,             p->secondReleaseCallback);
    ASSERT_EQ(allocator,     p->allocator);

    short const expectedFirst  = 11;
    long  const expectedSecond = 12;
    octaspire_container_pair_set(p, &expectedFirst, &expectedSecond);

    ASSERT_EQ(expectedFirst,  *(short const *)p->first);
    ASSERT_EQ(expectedSecond, *(long const *) p->second);

    octaspire_container_pair_release(p);
    p = 0;

    PASS();
}

TEST octaspire_container_pair_new_long_and_short_test(void)
{
    octaspire_container_pair_t *p = octaspire_container_pair_new(
        sizeof(long),
        false,
        sizeof(short),
        false,
        0,
        0,
        allocator);

    ASSERT(p);

    ASSERT(p->first);
    ASSERT(p->second);

    ASSERT_EQ(0, *(long const*)p->first);
    ASSERT_EQ(0, *(short const*)p->second);

    ASSERT_EQ(sizeof(long), p->firstSize);
    ASSERT_EQ(sizeof(short),  p->secondSize);
    ASSERT_EQ(0,             p->firstReleaseCallback);
    ASSERT_EQ(0,             p->secondReleaseCallback);
    ASSERT_EQ(allocator,     p->allocator);

    long  const expectedFirst  = 11;
    short const expectedSecond = 12;
    octaspire_container_pair_set(p, &expectedFirst, &expectedSecond);

    ASSERT_EQ(expectedFirst,  *(long const*) p->first);
    ASSERT_EQ(expectedSecond, *(short const*)p->second);

    octaspire_container_pair_release(p);
    p = 0;

    PASS();
}

TEST octaspire_container_pair_new_uint32_t_and_double_test(void)
{
    octaspire_container_pair_t *p = octaspire_container_pair_new(
        sizeof(uint32_t),
        false,
        sizeof(double),
        false,
        0,
        0,
        allocator);

    ASSERT(p);

    ASSERT(p->first);
    ASSERT(p->second);

    ASSERT_EQ(0, *(uint32_t*)p->first);
    ASSERT_EQ(0, *(double*)p->second);

    ASSERT_EQ(sizeof(uint32_t), p->firstSize);
    ASSERT_EQ(sizeof(double),   p->secondSize);
    ASSERT_EQ(0,                p->firstReleaseCallback);
    ASSERT_EQ(0,                p->secondReleaseCallback);
    ASSERT_EQ(allocator,        p->allocator);

    uint32_t  const expectedFirst  = 11;
    double    const expectedSecond = 12.12;
    octaspire_container_pair_set(p, &expectedFirst, &expectedSecond);

    ASSERT_EQ(expectedFirst,  *(uint32_t const*)p->first);
    ASSERT_IN_RANGE(expectedSecond, *(double const*)p->second, 0.000001);

    octaspire_container_pair_release(p);
    p = 0;

    PASS();
}

TEST octaspire_container_pair_new_allocation_failure_on_first_allocation_test(void)
{
    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(
        allocator,
        1,
        0);

    ASSERT_EQ(
        1,
        octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(
            allocator));

    octaspire_container_pair_t *p = octaspire_container_pair_new(
        sizeof(short),
        false,
        sizeof(long),
        false,
        0,
        0,
        allocator);

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 0, 0x00);

    ASSERT_FALSE(p);

    octaspire_container_pair_release(p);
    p = 0;

    PASS();
}

TEST octaspire_container_pair_new_allocation_failure_on_second_allocation_test(void)
{
    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(
        allocator,
        2,
        0x01);

    ASSERT_EQ(
        2,
        octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(
            allocator));

    octaspire_container_pair_t *p = octaspire_container_pair_new(
        sizeof(short),
        false,
        sizeof(long),
        false,
        0,
        0,
        allocator);

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 0, 0x00);

    ASSERT_FALSE(p);

    octaspire_container_pair_release(p);
    p = 0;

    PASS();
}

TEST octaspire_container_pair_new_allocation_failure_on_third_allocation_test(void)
{
    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(
        allocator,
        3,
        0x03);

    ASSERT_EQ(
        3,
        octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(
            allocator));

    octaspire_container_pair_t *p = octaspire_container_pair_new(
        sizeof(short),
        false,
        sizeof(long),
        false,
        0,
        0,
        allocator);

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 0, 0x00);

    ASSERT_FALSE(p);

    octaspire_container_pair_release(p);
    p = 0;

    PASS();
}

TEST octaspire_container_pair_new_shallow_copy_uint32_t_and_double_test(void)
{
    octaspire_container_pair_t *expected = octaspire_container_pair_new(
        sizeof(uint32_t),
        false,
        sizeof(double),
        false,
        0,
        0,
        allocator);

    ASSERT(expected);

    uint32_t  const expectedFirst  = 11;
    double    const expectedSecond = 12.12;
    octaspire_container_pair_set(expected, &expectedFirst, &expectedSecond);

    octaspire_container_pair_t *p = octaspire_container_pair_new_shallow_copy(
        expected,
        allocator);

    ASSERT(p);

    ASSERT(p->first);
    ASSERT(p->second);

    ASSERT_EQ(*(uint32_t const*)expected->first,  *(uint32_t const*)p->first);
    ASSERT_IN_RANGE(*(double const*)expected->second, *(double const*)p->second, 0.000001);

    ASSERT_EQ(sizeof(uint32_t), p->firstSize);
    ASSERT_EQ(sizeof(double),   p->secondSize);
    ASSERT_EQ(0,                p->firstReleaseCallback);
    ASSERT_EQ(0,                p->secondReleaseCallback);
    ASSERT_EQ(allocator,        p->allocator);

    octaspire_container_pair_release(p);
    p = 0;

    octaspire_container_pair_release(expected);
    expected = 0;

    PASS();
}

TEST octaspire_container_pair_new_shallow_copy_failure_on_first_allocation_test(void)
{
    octaspire_container_pair_t *expected = octaspire_container_pair_new(
        sizeof(uint32_t),
        false,
        sizeof(double),
        false,
        0,
        0,
        allocator);

    ASSERT(expected);

    uint32_t  const expectedFirst  = 11;
    double    const expectedSecond = 12.12;
    octaspire_container_pair_set(expected, &expectedFirst, &expectedSecond);

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(
        allocator,
        1,
        0);

    ASSERT_EQ(
        1,
        octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(
            allocator));

    octaspire_container_pair_t *p = octaspire_container_pair_new_shallow_copy(
        expected,
        allocator);

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 0, 0x00);

    ASSERT_FALSE(p);

    octaspire_container_pair_release(p);
    p = 0;

    octaspire_container_pair_release(expected);
    expected = 0;

    PASS();
}

TEST octaspire_container_pair_new_shallow_copy_failure_on_second_allocation_test(void)
{
    octaspire_container_pair_t *expected = octaspire_container_pair_new(
        sizeof(uint32_t),
        false,
        sizeof(double),
        false,
        0,
        0,
        allocator);

    ASSERT(expected);

    uint32_t  const expectedFirst  = 11;
    double    const expectedSecond = 12.12;
    octaspire_container_pair_set(expected, &expectedFirst, &expectedSecond);

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(
        allocator,
        2,
        0x1);

    ASSERT_EQ(
        2,
        octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(
            allocator));

    octaspire_container_pair_t *p = octaspire_container_pair_new_shallow_copy(
        expected,
        allocator);

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 0, 0x00);

    ASSERT_FALSE(p);

    octaspire_container_pair_release(p);
    p = 0;

    octaspire_container_pair_release(expected);
    expected = 0;

    PASS();
}

TEST octaspire_container_pair_new_shallow_copy_failure_on_third_allocation_test(void)
{
    octaspire_container_pair_t *expected = octaspire_container_pair_new(
        sizeof(uint32_t),
        false,
        sizeof(double),
        false,
        0,
        0,
        allocator);

    ASSERT(expected);

    uint32_t  const expectedFirst  = 11;
    double    const expectedSecond = 12.12;
    octaspire_container_pair_set(expected, &expectedFirst, &expectedSecond);

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(
        allocator,
        3,
        0x3);

    ASSERT_EQ(
        3,
        octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(
            allocator));

    octaspire_container_pair_t *p = octaspire_container_pair_new_shallow_copy(
        expected,
        allocator);

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 0, 0x00);

    ASSERT_FALSE(p);

    octaspire_container_pair_release(p);
    p = 0;

    octaspire_container_pair_release(expected);
    expected = 0;

    PASS();
}

static size_t octaspire_container_pair_release_test_private_release_callback_for_first_callcount = 0;

static void octaspire_container_pair_release_test_private_release_callback_for_first(void *element)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(element);
    ++octaspire_container_pair_release_test_private_release_callback_for_first_callcount;
}

static size_t octaspire_container_pair_release_test_private_release_callback_for_second_callcount = 0;

static void octaspire_container_pair_release_test_private_release_callback_for_second(void *element)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(element);
    ++octaspire_container_pair_release_test_private_release_callback_for_second_callcount;
}

TEST octaspire_container_pair_release_test(void)
{
    octaspire_container_pair_release_test_private_release_callback_for_first_callcount  = 0;
    octaspire_container_pair_release_test_private_release_callback_for_second_callcount = 0;

    octaspire_container_utf8_string_t *second = octaspire_container_utf8_string_new(
        "second element of the pair in octaspire_container_pair_release_test",
        allocator);

    octaspire_container_pair_t *p = octaspire_container_pair_new(
        sizeof(size_t),
        false,
        sizeof(octaspire_container_utf8_string_t*),
        true,
        octaspire_container_pair_release_test_private_release_callback_for_first,
        octaspire_container_pair_release_test_private_release_callback_for_second,
        allocator);

    ASSERT(p);

    size_t const first  = 11;
    octaspire_container_pair_set(p, &first, &second);

    ASSERT_EQ(first,  *(size_t const *)p->first);
    ASSERT_EQ(second, *(octaspire_container_utf8_string_t **)p->second);

    octaspire_container_pair_release(p);
    p = 0;

    octaspire_container_utf8_string_release(second);
    second = 0;

    ASSERT_EQ(1, octaspire_container_pair_release_test_private_release_callback_for_first_callcount);
    ASSERT_EQ(1, octaspire_container_pair_release_test_private_release_callback_for_second_callcount);

    PASS();
}

TEST octaspire_container_pair_get_first_test(void)
{
    octaspire_container_pair_t *p = octaspire_container_pair_new(
        sizeof(long),
        false,
        sizeof(short),
        false,
        0,
        0,
        allocator);

    ASSERT(p);

    long  const expectedFirst = -11;
    short const second        =  12;
    octaspire_container_pair_set(p, &expectedFirst, &second);

    ASSERT_EQ(expectedFirst, *(long const*)octaspire_container_pair_get_first(p));

    octaspire_container_pair_release(p);
    p = 0;

    PASS();
}

TEST octaspire_container_pair_get_first_const_test(void)
{
    octaspire_container_pair_t *p = octaspire_container_pair_new(
        sizeof(long),
        false,
        sizeof(short),
        false,
        0,
        0,
        allocator);

    ASSERT(p);

    long  const expectedFirst = -11;
    short const second        =  12;
    octaspire_container_pair_set(p, &expectedFirst, &second);

    ASSERT_EQ(expectedFirst, *(long const*)octaspire_container_pair_get_first_const(p));

    octaspire_container_pair_release(p);
    p = 0;

    PASS();
}

TEST octaspire_container_pair_get_second_test(void)
{
    octaspire_container_pair_t *p = octaspire_container_pair_new(
        sizeof(long),
        false,
        sizeof(short),
        false,
        0,
        0,
        allocator);

    ASSERT(p);

    long  const first           =  11;
    short const expectedSecond  = -12;
    octaspire_container_pair_set(p, &first, &expectedSecond);

    ASSERT_EQ(expectedSecond, *(short const*)octaspire_container_pair_get_second(p));

    octaspire_container_pair_release(p);
    p = 0;

    PASS();
}

TEST octaspire_container_pair_get_second_const_test(void)
{
    octaspire_container_pair_t *p = octaspire_container_pair_new(
        sizeof(long),
        false,
        sizeof(short),
        false,
        0,
        0,
        allocator);

    ASSERT(p);

    long  const first           =  11;
    short const expectedSecond  = -12;
    octaspire_container_pair_set(p, &first, &expectedSecond);

    ASSERT_EQ(expectedSecond, *(short const*)octaspire_container_pair_get_second_const(p));

    octaspire_container_pair_release(p);
    p = 0;

    PASS();
}

TEST octaspire_container_pair_get_size_of_first_element_in_octets_test(void)
{
    octaspire_container_pair_t *p = octaspire_container_pair_new(
        sizeof(long),
        false,
        sizeof(short),
        false,
        0,
        0,
        allocator);

    ASSERT(p);

    ASSERT_EQ(sizeof(long), octaspire_container_pair_get_size_of_first_element_in_octets(p));

    octaspire_container_pair_release(p);
    p = 0;

    PASS();
}

TEST octaspire_container_pair_get_size_of_second_element_in_octets_test(void)
{
    octaspire_container_pair_t *p = octaspire_container_pair_new(
        sizeof(long),
        false,
        sizeof(short),
        false,
        0,
        0,
        allocator);

    ASSERT(p);

    ASSERT_EQ(sizeof(short), octaspire_container_pair_get_size_of_second_element_in_octets(p));

    octaspire_container_pair_release(p);
    p = 0;

    PASS();
}

TEST octaspire_container_pair_set_test(void)
{
    octaspire_container_pair_t *p = octaspire_container_pair_new(
        sizeof(short),
        false,
        sizeof(long),
        false,
        0,
        0,
        allocator);

    ASSERT(p);

    short const expectedFirst  = 11;
    long  const expectedSecond = 12;
    octaspire_container_pair_set(p, &expectedFirst, &expectedSecond);

    ASSERT_EQ(expectedFirst,  *(short const *)p->first);
    ASSERT_EQ(expectedSecond, *(long const *) p->second);

    octaspire_container_pair_release(p);
    p = 0;

    PASS();
}

TEST octaspire_container_pair_set_first_test(void)
{
    octaspire_container_pair_t *p = octaspire_container_pair_new(
        sizeof(short),
        false,
        sizeof(long),
        false,
        0,
        0,
        allocator);

    ASSERT(p);

    short const expectedFirst  = 11;
    long  const expectedSecond = 0;
    octaspire_container_pair_set_first(p, &expectedFirst);

    ASSERT_EQ(expectedFirst,  *(short const *)p->first);
    ASSERT_EQ(expectedSecond, *(long const *) p->second);

    octaspire_container_pair_release(p);
    p = 0;

    PASS();
}

TEST octaspire_container_pair_set_second_test(void)
{
    octaspire_container_pair_t *p = octaspire_container_pair_new(
        sizeof(short),
        false,
        sizeof(long),
        false,
        0,
        0,
        allocator);

    ASSERT(p);

    short const expectedFirst  = 0;
    long  const expectedSecond = 11;
    octaspire_container_pair_set_second(p, &expectedSecond);

    ASSERT_EQ(expectedFirst,  *(short const *)p->first);
    ASSERT_EQ(expectedSecond, *(long const *) p->second);

    octaspire_container_pair_release(p);
    p = 0;

    PASS();
}

TEST octaspire_container_pair_set_first_to_void_pointer_test(void)
{
    octaspire_container_pair_t *p = octaspire_container_pair_new(
        sizeof(void*),
        true,
        sizeof(long),
        false,
        0,
        0,
        allocator);

    ASSERT(p);

    void *expectedFirst = p;
    long  const expectedSecond = 0;
    ASSERT(octaspire_container_pair_set_first_to_void_pointer(p, expectedFirst));

    ASSERT_EQ(expectedFirst,  *(void **)p->first);
    ASSERT_EQ(expectedSecond, *(long const *)p->second);

    octaspire_container_pair_release(p);
    p = 0;

    PASS();
}

TEST octaspire_container_pair_set_first_to_void_pointer_failure_on_element_size_test(void)
{
    octaspire_container_pair_t *p = octaspire_container_pair_new(
        sizeof(char),
        false,
        sizeof(long),
        false,
        0,
        0,
        allocator);

    ASSERT(p);

    void *ptr = 0;
    ASSERT_FALSE(octaspire_container_pair_set_first_to_void_pointer(p, ptr));

    octaspire_container_pair_release(p);
    p = 0;

    PASS();
}

TEST octaspire_container_pair_set_second_to_void_pointer_test(void)
{
    octaspire_container_pair_t *p = octaspire_container_pair_new(
        sizeof(long),
        false,
        sizeof(void*),
        true,
        0,
        0,
        allocator);

    ASSERT(p);

    void *expectedSecond = p;
    long  const expectedFirst = 0;
    ASSERT(octaspire_container_pair_set_second_to_void_pointer(p, expectedSecond));

    ASSERT_EQ(expectedFirst,  *(long const *)p->first);
    ASSERT_EQ(expectedSecond, *(void const **)p->second);

    octaspire_container_pair_release(p);
    p = 0;

    PASS();
}

TEST octaspire_container_pair_set_second_to_void_pointer_failure_on_element_size_test(void)
{
    octaspire_container_pair_t *p = octaspire_container_pair_new(
        sizeof(long),
        false,
        sizeof(char),
        false,
        0,
        0,
        allocator);

    ASSERT(p);

    void *ptr = 0;
    ASSERT_FALSE(octaspire_container_pair_set_second_to_void_pointer(p, ptr));

    octaspire_container_pair_release(p);
    p = 0;

    PASS();
}

TEST octaspire_container_pair_clear_test(void)
{
    octaspire_container_pair_t *p = octaspire_container_pair_new(
        sizeof(short),
        false,
        sizeof(long),
        false,
        0,
        0,
        allocator);

    ASSERT(p);

    short const expectedFirst  = 11;
    long  const expectedSecond = 12;
    octaspire_container_pair_set(p, &expectedFirst, &expectedSecond);

    ASSERT_EQ(11, *(short const*)p->first);
    ASSERT_EQ(12, *(long const*)p->second);

    octaspire_container_pair_clear(p);

    ASSERT_EQ(0, *(short const*)p->first);
    ASSERT_EQ(0, *(long const*)p->second);

    octaspire_container_pair_release(p);
    p = 0;

    PASS();
}

GREATEST_SUITE(octaspire_container_pair_suite)
{
    size_t numTimesRun = 0;

    allocator = octaspire_memory_allocator_new_create_region(
        OCTASPIRE_CORE_CONFIG_MEMORY_ALLOCATOR_REGION_MIN_BLOCK_SIZE_IN_OCTETS);

second_run:

    RUN_TEST(octaspire_container_pair_new_short_and_long_test);
    RUN_TEST(octaspire_container_pair_new_long_and_short_test);
    RUN_TEST(octaspire_container_pair_new_uint32_t_and_double_test);
    RUN_TEST(octaspire_container_pair_new_allocation_failure_on_first_allocation_test);
    RUN_TEST(octaspire_container_pair_new_allocation_failure_on_second_allocation_test);
    RUN_TEST(octaspire_container_pair_new_allocation_failure_on_third_allocation_test);
    RUN_TEST(octaspire_container_pair_new_shallow_copy_uint32_t_and_double_test);
    RUN_TEST(octaspire_container_pair_new_shallow_copy_failure_on_first_allocation_test);
    RUN_TEST(octaspire_container_pair_new_shallow_copy_failure_on_second_allocation_test);
    RUN_TEST(octaspire_container_pair_new_shallow_copy_failure_on_third_allocation_test);
    RUN_TEST(octaspire_container_pair_release_test);
    RUN_TEST(octaspire_container_pair_get_first_test);
    RUN_TEST(octaspire_container_pair_get_first_const_test);
    RUN_TEST(octaspire_container_pair_get_second_test);
    RUN_TEST(octaspire_container_pair_get_second_const_test);
    RUN_TEST(octaspire_container_pair_get_size_of_first_element_in_octets_test);
    RUN_TEST(octaspire_container_pair_get_size_of_second_element_in_octets_test);
    RUN_TEST(octaspire_container_pair_set_test);
    RUN_TEST(octaspire_container_pair_set_first_test);
    RUN_TEST(octaspire_container_pair_set_second_test);
    RUN_TEST(octaspire_container_pair_set_first_to_void_pointer_test);
    RUN_TEST(octaspire_container_pair_set_first_to_void_pointer_failure_on_element_size_test);
    RUN_TEST(octaspire_container_pair_set_second_to_void_pointer_test);
    RUN_TEST(octaspire_container_pair_set_second_to_void_pointer_failure_on_element_size_test);
    RUN_TEST(octaspire_container_pair_clear_test);

    octaspire_memory_allocator_release(allocator);
    allocator = 0;

    ++numTimesRun;

    if (numTimesRun < 2)
    {
        // Second run without region allocator

        allocator = octaspire_memory_allocator_new(0);

        goto second_run;
    }
}
