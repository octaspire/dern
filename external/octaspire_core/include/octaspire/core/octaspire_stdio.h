#ifndef OCTASPIRE_STDIO_H
#define OCTASPIRE_STDIO_H

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include "octaspire_memory.h"
#include "octaspire_container_utf8_string.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct octaspire_stdio_t octaspire_stdio_t;

octaspire_stdio_t *octaspire_stdio_new(octaspire_memory_allocator_t *allocator);

void octaspire_stdio_release(octaspire_stdio_t *self);

size_t octaspire_stdio_fread(
    octaspire_stdio_t *self,
    void *ptr,
    size_t const size,
    size_t const nmemb,
    FILE *stream);

void octaspire_stdio_set_number_and_type_of_future_reads_to_be_rigged(
    octaspire_stdio_t *self,
    size_t const count,
    uint32_t const bitQueue);

size_t octaspire_stdio_get_number_of_future_reads_to_be_rigged(
    octaspire_stdio_t const * const self);

octaspire_container_utf8_string_t *octaspire_stdio_read_line(octaspire_stdio_t *self, FILE *stream);

#ifdef __cplusplus
}
#endif

#endif

