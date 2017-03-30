#include "../src/octaspire_container_hash_map.c"
#include <assert.h>
#include <inttypes.h>
#include "external/greatest.h"
#include "octaspire/core/octaspire_container_hash_map.h"
#include "octaspire/core/octaspire_memory.h"
#include "octaspire/core/octaspire_container_utf8_string.h"
#include "octaspire/core/octaspire_helpers.h"
#include "octaspire/core/octaspire_core_config.h"

static octaspire_memory_allocator_t *allocator = 0;



TEST octaspire_container_hash_map_element_new_allocation_failure_on_first_allocation_test(void)
{
    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 1, 0);
    ASSERT_EQ(1, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    size_t const value = 0;
    octaspire_container_hash_map_element_t *element = octaspire_container_hash_map_element_new(
        0,
        sizeof(value),
        false,
        &value,
        sizeof(value),
        false,
        &value,
        allocator);

    ASSERT_FALSE(element);
    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 0, 0x00);

    octaspire_container_hash_map_element_release(element);
    element = 0;

    PASS();
}

TEST octaspire_container_hash_map_element_new_allocation_failure_on_second_allocation_test(void)
{
    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 2, 0x01);
    ASSERT_EQ(2, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    size_t const value = 0;
    octaspire_container_hash_map_element_t *element = octaspire_container_hash_map_element_new(
        0,
        sizeof(value),
        false,
        &value,
        sizeof(value),
        false,
        &value,
        allocator);

    ASSERT_FALSE(element);
    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 0, 0x00);

    octaspire_container_hash_map_element_release(element);
    element = 0;

    PASS();
}

TEST octaspire_container_hash_map_element_new_allocation_failure_on_third_allocation_test(void)
{
    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 3, 0x03);
    ASSERT_EQ(3, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    size_t const value = 0;
    octaspire_container_hash_map_element_t *element = octaspire_container_hash_map_element_new(
        0,
        sizeof(value),
        false,
        &value,
        sizeof(value),
        false,
        &value,
        allocator);

    ASSERT_FALSE(element);
    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 0, 0x00);

    octaspire_container_hash_map_element_release(element);
    element = 0;

    PASS();
}

static bool octaspire_container_hash_map_new_test_key_compare_function_for_size_t_keys(
    void const * const key1,
    void const * const key2);

static uint32_t octaspire_container_hash_map_new_test_key_hash_function_for_size_t_keys(
    void const * const key);

static bool octaspire_container_hash_map_new_test_key_compare_function_for_size_t_keys(
    void const * const key1,
    void const * const key2)
{
    return *(size_t const * const)key1 == *(size_t const * const)key2;
}

static uint32_t octaspire_container_hash_map_new_test_key_hash_function_for_size_t_keys(
    void const * const key)
{
    return *(size_t const * const)key;
}

TEST octaspire_container_hash_map_private_rehash_allocation_failure_on_first_allocation_test(void)
{
    octaspire_container_hash_map_t *hashMap = octaspire_container_hash_map_new(
        sizeof(size_t),
        false,
        sizeof(size_t),
        false,
        octaspire_container_hash_map_new_test_key_compare_function_for_size_t_keys,
        octaspire_container_hash_map_new_test_key_hash_function_for_size_t_keys,
        0,
        0,
        allocator);

    ASSERT(hashMap);

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 1, 0x00);
    ASSERT_EQ(1, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    ASSERT_FALSE(octaspire_container_hash_map_private_rehash(hashMap));
    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 0, 0x00);

    octaspire_container_hash_map_release(hashMap);
    hashMap = 0;

    PASS();
}

#if 0
TEST octaspire_container_hash_map_private_rehash_allocation_failure_on_second_allocation_test(void)
{
    octaspire_container_hash_map_t *hashMap = octaspire_container_hash_map_new(
        sizeof(size_t),
        sizeof(size_t),
        octaspire_container_hash_map_new_test_key_compare_function_for_size_t_keys,
        octaspire_container_hash_map_new_test_key_hash_function_for_size_t_keys,
        0,
        0,
        allocator);

    for (size_t value = 0; value < 5; ++value)
    {
        octaspire_container_hash_map_put(
            hashMap,
            0, //octaspire_helpers_calculate_murmur3_hash_for_size_t_argument(value),
            &value,
            &value);
    }

    // Should have 513 success, and number 514 should be failure
    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged_when_larger_than_32(
        allocator,
        515,
        0xFF, // 0
        0xFF, // 1
        0xFF, // 2
        0xFF, // 3
        0xFF, // 4
        0xFF, // 5
        0xFF, // 6
        0xFF, // 7
        0xFF, // 8
        0xFF, // 9
        0xFF, // 10
        0xFF, // 11
        0xFF, // 12
        0xFF, // 13
        0xFF, // 14
        0xFF, // 15
        0xFF, // 16  512 success at this point
        0x03, // 17  +2  success
        0x00, // 18
        0x00);

    ASSERT_EQ(515, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    ASSERT_FALSE(octaspire_container_hash_map_private_rehash(hashMap));

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 0, 0x00);

    octaspire_container_hash_map_release(hashMap);
    hashMap = 0;

    PASS();
}
#endif

TEST octaspire_container_hash_map_new_keys_uint32_t_and_values_size_t_test(void)
{
    octaspire_container_hash_map_t *hashMap = octaspire_container_hash_map_new(
        sizeof(size_t),
        false,
        sizeof(size_t),
        false,
        octaspire_container_hash_map_new_test_key_compare_function_for_size_t_keys,
        octaspire_container_hash_map_new_test_key_hash_function_for_size_t_keys,
        0,
        0,
        allocator);

    ASSERT(hashMap);

    size_t const numElements = 128;

    for (size_t i = 0; i < numElements; ++i)
    {
        uint32_t hash = i;

        octaspire_container_hash_map_put(hashMap, hash, &i, &i);
    }

    ASSERT_EQ(numElements, octaspire_container_hash_map_get_number_of_elements(hashMap));

    for (size_t i = 0; i < numElements; ++i)
    {
        octaspire_container_hash_map_element_t *element =
            octaspire_container_hash_map_get(hashMap, i, &i);

        ASSERT_EQ(i,   octaspire_container_hash_map_element_get_hash(element));
        ASSERT_EQ(i,   *(size_t*)octaspire_container_hash_map_element_get_key(element));
        ASSERT_EQ(i,   *(size_t*)octaspire_container_hash_map_element_get_value(element));
    }

    octaspire_container_hash_map_release(hashMap);
    hashMap = 0;

    PASS();
}

TEST octaspire_container_hash_map_add_same_key_many_times_test(void)
{
    octaspire_container_hash_map_t *hashMap = octaspire_container_hash_map_new(
        sizeof(size_t),
        false,
        sizeof(size_t),
        false,
        octaspire_container_hash_map_new_test_key_compare_function_for_size_t_keys,
        octaspire_container_hash_map_new_test_key_hash_function_for_size_t_keys,
        0,
        0,
        allocator);

    ASSERT(hashMap);

    size_t const key = 1024;

    for (size_t i = 0; i < 100; ++i)
    {
        ASSERT(octaspire_container_hash_map_put(
            hashMap,
            octaspire_helpers_calculate_murmur3_hash_for_size_t_argument(key),
            &key,
            &i));

        octaspire_container_hash_map_element_t * const element = octaspire_container_hash_map_get(
            hashMap,
            octaspire_helpers_calculate_murmur3_hash_for_size_t_argument(key),
            &key);

        ASSERT(element);

        ASSERT_EQ(key, *(size_t*)octaspire_container_hash_map_element_get_key(element));

        octaspire_container_vector_t * const values =
            octaspire_container_hash_map_element_get_values(element);

        ASSERT_EQ(i + 1,   octaspire_container_vector_get_length(values));

        for (size_t j = 0; j < octaspire_container_vector_get_length(values) ; ++j)
        {
            ASSERT_EQ(j,   *(size_t*)octaspire_container_vector_get_element_at(values, j));
        }
    }

    octaspire_container_hash_map_release(hashMap);
    hashMap = 0;

    PASS();
}

TEST octaspire_container_hash_map_new_allocation_failure_on_first_allocation_test(void)
{
    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 1, 0);
    ASSERT_EQ(1, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    octaspire_container_hash_map_t *hashMap = octaspire_container_hash_map_new(
        sizeof(size_t),
        false,
        sizeof(size_t),
        false,
        octaspire_container_hash_map_new_test_key_compare_function_for_size_t_keys,
        octaspire_container_hash_map_new_test_key_hash_function_for_size_t_keys,
        0,
        0,
        allocator);

    ASSERT_FALSE(hashMap);
    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 0, 0x00);

    octaspire_container_hash_map_release(hashMap);
    hashMap = 0;

    PASS();
}

TEST octaspire_container_hash_map_new_allocation_failure_on_second_allocation_test(void)
{
    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 2, 0x01);
    ASSERT_EQ(2, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    octaspire_container_hash_map_t *hashMap = octaspire_container_hash_map_new(
        sizeof(size_t),
        false,
        sizeof(size_t),
        false,
        octaspire_container_hash_map_new_test_key_compare_function_for_size_t_keys,
        octaspire_container_hash_map_new_test_key_hash_function_for_size_t_keys,
        0,
        0,
        allocator);

    ASSERT_FALSE(hashMap);
    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 0, 0x00);

    octaspire_container_hash_map_release(hashMap);
    hashMap = 0;

    PASS();
}

static bool octaspire_container_hash_map_new_test_key_compare_function_for_ostring_t_keys(
    void const * const key1,
    void const * const key2);

static uint32_t octaspire_container_hash_map_new_test_key_hash_function_for_ostring_t_keys(
    void const * const key);

static bool octaspire_container_hash_map_new_test_key_compare_function_for_ostring_t_keys(
    void const * const key1,
    void const * const key2)
{
    assert(key1);
    assert(key2);

    return octaspire_container_utf8_string_is_equal(
        (octaspire_container_utf8_string_t const * const)key1,
        (octaspire_container_utf8_string_t const * const)key2);
}

static uint32_t octaspire_container_hash_map_new_test_key_hash_function_for_ostring_t_keys(
    void const * const key)
{
    assert(key);
    return octaspire_container_utf8_string_get_hash((octaspire_container_utf8_string_t const * const)key);
}

TEST octaspire_container_hash_map_new_keys_ostring_t_and_values_ostring_t_test(void)
{
    octaspire_container_hash_map_t *hashMap = octaspire_container_hash_map_new(
        sizeof(octaspire_container_utf8_string_t *),
        true,
        sizeof(octaspire_container_utf8_string_t *),
        true,
        octaspire_container_hash_map_new_test_key_compare_function_for_ostring_t_keys,
        octaspire_container_hash_map_new_test_key_hash_function_for_ostring_t_keys,
        (octaspire_container_hash_map_element_callback_function_t)octaspire_container_utf8_string_release,
        (octaspire_container_hash_map_element_callback_function_t)octaspire_container_utf8_string_release,
        allocator);

    size_t const numElements = 32;

    for (size_t i = 0; i < numElements; ++i)
    {
        octaspire_container_utf8_string_t *str = octaspire_container_utf8_string_new_format(
            allocator,
            "%zu",
            i);

        uint32_t const hash = octaspire_container_utf8_string_get_hash(str);

        octaspire_container_utf8_string_t *cpyStr = octaspire_container_utf8_string_new_copy(str, allocator);
        ASSERT(octaspire_container_utf8_string_is_equal(str, cpyStr));
        octaspire_container_hash_map_put(hashMap, hash, &cpyStr, &str);

        ASSERT_EQ(i+1, octaspire_container_hash_map_get_number_of_elements(hashMap));
    }

    ASSERT_EQ(numElements, octaspire_container_hash_map_get_number_of_elements(hashMap));

    for (size_t i = 0; i < numElements; ++i)
    {
        octaspire_container_utf8_string_t *str = octaspire_container_utf8_string_new_format(
            allocator,
            "%zu",
            i);

        uint32_t hash = octaspire_container_utf8_string_get_hash(str);

        octaspire_container_hash_map_element_t *element =
            octaspire_container_hash_map_get(hashMap, hash, &str);

        ASSERT(element);

        ASSERT_EQ(hash, octaspire_container_hash_map_element_get_hash(element));

        ASSERT(octaspire_container_utf8_string_is_equal(str, (octaspire_container_utf8_string_t*)octaspire_container_hash_map_element_get_key(element)));

        ASSERT(octaspire_container_utf8_string_is_equal(str, (octaspire_container_utf8_string_t*)octaspire_container_hash_map_element_get_value(element)));

        octaspire_container_utf8_string_release(str);
        str = 0;
    }

    octaspire_container_hash_map_release(hashMap);
    hashMap = 0;

    PASS();
}

GREATEST_SUITE(octaspire_container_hash_map_suite)
{
    size_t numTimesRun = 0;

    allocator = octaspire_memory_allocator_new_create_region(
        OCTASPIRE_CORE_CONFIG_MEMORY_ALLOCATOR_REGION_MIN_BLOCK_SIZE_IN_OCTETS);

second_run:

    assert(allocator);

    RUN_TEST(octaspire_container_hash_map_element_new_allocation_failure_on_first_allocation_test);
    RUN_TEST(octaspire_container_hash_map_element_new_allocation_failure_on_second_allocation_test);
    RUN_TEST(octaspire_container_hash_map_element_new_allocation_failure_on_third_allocation_test);
    RUN_TEST(octaspire_container_hash_map_private_rehash_allocation_failure_on_first_allocation_test);
    //RUN_TEST(octaspire_container_hash_map_private_rehash_allocation_failure_on_second_allocation_test);
    RUN_TEST(octaspire_container_hash_map_new_keys_uint32_t_and_values_size_t_test);
    RUN_TEST(octaspire_container_hash_map_add_same_key_many_times_test);
    RUN_TEST(octaspire_container_hash_map_new_allocation_failure_on_first_allocation_test);
    RUN_TEST(octaspire_container_hash_map_new_allocation_failure_on_second_allocation_test);
    RUN_TEST(octaspire_container_hash_map_new_keys_ostring_t_and_values_ostring_t_test);

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
