#include "octaspire/core/octaspire_stdio.h"
#include <string.h>
#include <wchar.h>
#include <stdlib.h>
#include "octaspire/core/octaspire_container_vector.h"
#include "octaspire/core/octaspire_helpers.h"

struct octaspire_stdio_t
{
    size_t   numberOfFutureReadsToBeRigged;
    size_t   bitIndex;
    uint32_t bitQueue;
    octaspire_memory_allocator_t *allocator;
};

octaspire_stdio_t *octaspire_stdio_new(octaspire_memory_allocator_t *allocator)
{
    size_t const size = sizeof(octaspire_stdio_t);

    octaspire_stdio_t *self = octaspire_memory_allocator_malloc(allocator, size);

    if (!self)
    {
        return self;
    }

    memset(self, 0, size);

    self->allocator = allocator;

    return self;
}

void octaspire_stdio_release(octaspire_stdio_t *self)
{
    if (!self)
    {
        return;
    }

    octaspire_memory_allocator_free(self->allocator, self);
}

size_t octaspire_stdio_fread(
    octaspire_stdio_t *self,
    void *ptr,
    size_t const size,
    size_t const nmemb,
    FILE *stream)
{
    if (self->numberOfFutureReadsToBeRigged)
    {
        --(self->numberOfFutureReadsToBeRigged);

        if (!octaspire_helpers_test_bit(self->bitQueue, self->bitIndex))
        {
            ++(self->bitIndex);
            return 0;
        }

        ++(self->bitIndex);
    }

    return fread(ptr, size, nmemb, stream);
}

void octaspire_stdio_set_number_and_type_of_future_reads_to_be_rigged(
    octaspire_stdio_t *self,
    size_t const count,
    uint32_t const bitQueue)
{
    self->numberOfFutureReadsToBeRigged = count;
    self->bitIndex = 0;
    self->bitQueue = bitQueue;
}

size_t octaspire_stdio_get_number_of_future_reads_to_be_rigged(
    octaspire_stdio_t const * const self)
{
    return self->numberOfFutureReadsToBeRigged;
}

octaspire_container_utf8_string_t *octaspire_stdio_read_line(octaspire_stdio_t *self, FILE *stream)
{
    octaspire_container_vector_t *vec = octaspire_container_vector_new(
        sizeof(char),
        false,
        0,
        self->allocator);

    while (true)
    {
        int c = fgetc(stream);
        char const ch = (char)c;

        if (c == EOF)
        {
            octaspire_container_vector_release(vec);
            return 0;
        }
        else if (c == '\n')
        {
            octaspire_container_vector_push_back_element(vec, &ch);
            break;
        }

        octaspire_container_vector_push_back_element(vec, &ch);
    }

    octaspire_container_utf8_string_t* result = octaspire_container_utf8_string_new_from_buffer(
        octaspire_container_vector_get_element_at_const(vec, 0),
        octaspire_container_vector_get_length_in_octets(vec),
        self->allocator);

    octaspire_container_vector_release(vec);
    vec = 0;
    return result;
}


