#include "octaspire/core/octaspire_helpers.h"
#include <assert.h>
#include <stdio.h>
#include "external/murmur3.h"

bool octaspire_helpers_test_bit(uint32_t const bitSet, size_t const index)
{
    assert(index < 32);
    return bitSet & ((uint32_t)0x01 << index);
}

char *octaspire_helpers_path_to_buffer(
    char const * const path,
    size_t *octetsAllocated,
    octaspire_memory_allocator_t *allocator,
    octaspire_stdio_t *stdio)
{
    *octetsAllocated = 0;
    FILE *f = fopen(path, "rb");

    if (!f)
    {
        return 0;
    }

    fseek(f, 0, SEEK_END);

    long const length = ftell(f);

    if (length <= 0)
    {
        fclose(f);
        f = 0;
        return 0;
    }

    fseek(f, 0, SEEK_SET);

    char *result = octaspire_memory_allocator_malloc(allocator, sizeof(char) * (size_t)length);

    if (!result)
    {
        fclose(f);
        f = 0;
        return 0;
    }

    *octetsAllocated = (size_t)length;

    if (*octetsAllocated != octaspire_stdio_fread(stdio, result, sizeof(char), *octetsAllocated, f))
    {
        fclose(f);
        f = 0;
        octaspire_memory_allocator_free(allocator, result);
        result = 0;
        *octetsAllocated = 0;
        return 0;
    }

    fclose(f);
    f = 0;

    return result;
}

uint32_t octaspire_helpers_calculate_murmur3_hash_for_size_t_argument(size_t const value)
{
    uint32_t result = 0;
    MurmurHash3_x86_32(&value, sizeof(value), OCTASPIRE_HELPERS_MURMUR3_HASH_SEED, &result);
    return result;
}

uint32_t octaspire_helpers_calculate_murmur3_hash_for_bool_argument(bool const value)
{
    uint32_t result = 0;
    MurmurHash3_x86_32(&value, sizeof(value), OCTASPIRE_HELPERS_MURMUR3_HASH_SEED, &result);
    return result;
}

uint32_t octaspire_helpers_calculate_murmur3_hash_for_int32_t_argument(int32_t const value)
{
    uint32_t result = 0;
    MurmurHash3_x86_32(&value, sizeof(value), OCTASPIRE_HELPERS_MURMUR3_HASH_SEED, &result);
    return result;
}

uint32_t octaspire_helpers_calculate_murmur3_hash_for_double_argument(double const value)
{
    uint32_t result = 0;
    MurmurHash3_x86_32(&value, sizeof(value), OCTASPIRE_HELPERS_MURMUR3_HASH_SEED, &result);
    return result;
}

uint32_t octaspire_helpers_calculate_murmur3_hash_for_void_pointer_argument(void const * const value)
{
    uint32_t result = 0;
    MurmurHash3_x86_32(&value, sizeof(value), OCTASPIRE_HELPERS_MURMUR3_HASH_SEED, &result);
    return result;
}

size_t octaspire_helpers_character_digit_to_number(uint32_t const c)
{
    return c - '0';
}

size_t octaspire_helpers_min_size_t(size_t const a, size_t const b)
{
    if (a < b)
    {
        return a;
    }

    return b;
}

size_t octaspire_helpers_max_size_t(size_t const a, size_t const b)
{
    if (a > b)
    {
        return a;
    }

    return b;
}
