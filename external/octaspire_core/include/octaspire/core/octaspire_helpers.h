#ifndef OCTASPIRE_HELPERS_H
#define OCTASPIRE_HELPERS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "octaspire_memory.h"
#include "octaspire_stdio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OCTASPIRE_HELPERS_UNUSED_PARAMETER(x) (void)(x)
#define OCTASPIRE_HELPERS_MURMUR3_HASH_SEED 0xFF00FF00

bool  octaspire_helpers_test_bit(uint32_t const bitSet, size_t const index);

char *octaspire_helpers_path_to_buffer(
    char const * const path,
    size_t *octetsAllocated,
    octaspire_memory_allocator_t *allocator,
    octaspire_stdio_t *stdio);

uint32_t octaspire_helpers_calculate_murmur3_hash_for_size_t_argument(size_t const value);
uint32_t octaspire_helpers_calculate_murmur3_hash_for_bool_argument(bool const value);
uint32_t octaspire_helpers_calculate_murmur3_hash_for_int32_t_argument(int32_t const value);
uint32_t octaspire_helpers_calculate_murmur3_hash_for_double_argument(double const value);
uint32_t octaspire_helpers_calculate_murmur3_hash_for_void_pointer_argument(void const * const value);

size_t octaspire_helpers_character_digit_to_number(uint32_t const c);

size_t octaspire_helpers_min_size_t(size_t const a, size_t const b);
size_t octaspire_helpers_max_size_t(size_t const a, size_t const b);

#ifdef __cplusplus
}
#endif

#endif

