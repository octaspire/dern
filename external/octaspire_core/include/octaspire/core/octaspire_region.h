#ifndef OCTASPIRE_REGION_H
#define OCTASPIRE_REGION_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct octaspire_region_t octaspire_region_t;

octaspire_region_t *octaspire_region_new(size_t const minBlockSizeInOctets);

void octaspire_region_release(octaspire_region_t *self);

void *octaspire_region_malloc(
    octaspire_region_t *self,
    size_t const size);

void *octaspire_region_realloc(
    octaspire_region_t *self,
    void *ptr, size_t const size);

void octaspire_region_free(
    octaspire_region_t *self,
    void *ptr);

void octaspire_region_compact(octaspire_region_t *self);

void octaspire_region_print(octaspire_region_t const * const self);

#ifdef __cplusplus
}
#endif

#endif

