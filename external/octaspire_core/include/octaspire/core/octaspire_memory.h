#ifndef OCTASPIRE_MEMORY_H
#define OCTASPIRE_MEMORY_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct octaspire_region_t;
typedef struct octaspire_memory_allocator_t octaspire_memory_allocator_t;

octaspire_memory_allocator_t *octaspire_memory_allocator_new_create_region(size_t const minBlockSizeInOctets);
octaspire_memory_allocator_t *octaspire_memory_allocator_new(struct octaspire_region_t *region);

void octaspire_memory_allocator_release(octaspire_memory_allocator_t *self);

void *octaspire_memory_allocator_malloc(
    octaspire_memory_allocator_t *self,
    size_t const size);

void *octaspire_memory_allocator_realloc(
    octaspire_memory_allocator_t *self,
    void *ptr, size_t const size);

void octaspire_memory_allocator_free(
    octaspire_memory_allocator_t *self,
    void *ptr);

void octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(
    octaspire_memory_allocator_t *self,
    size_t const count,
    uint32_t const bitQueue0);

void octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged_when_larger_than_32(
    octaspire_memory_allocator_t *self,
    size_t const count,
    uint32_t const bitQueue0,
    uint32_t const bitQueue1,
    uint32_t const bitQueue2,
    uint32_t const bitQueue3,
    uint32_t const bitQueue4,
    uint32_t const bitQueue5,
    uint32_t const bitQueue6,
    uint32_t const bitQueue7,
    uint32_t const bitQueue8,
    uint32_t const bitQueue9,
    uint32_t const bitQueue10,
    uint32_t const bitQueue11,
    uint32_t const bitQueue12,
    uint32_t const bitQueue13,
    uint32_t const bitQueue14,
    uint32_t const bitQueue15,
    uint32_t const bitQueue16,
    uint32_t const bitQueue17,
    uint32_t const bitQueue18,
    uint32_t const bitQueue19);

size_t octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(
    octaspire_memory_allocator_t const * const self);

#ifdef __cplusplus
}
#endif

#endif

