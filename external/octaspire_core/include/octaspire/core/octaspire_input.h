#ifndef OCTASPIRE_INPUT_H
#define OCTASPIRE_INPUT_H

#include <stdbool.h>
#include "octaspire_memory.h"
#include "octaspire_stdio.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct octaspire_input_t octaspire_input_t;

octaspire_input_t *octaspire_input_new_from_c_string(
    char const * const str,
    octaspire_memory_allocator_t *allocator);

octaspire_input_t *octaspire_input_new_from_buffer(
    char const * const buffer,
    size_t const lengthInOctets,
    octaspire_memory_allocator_t *allocator);

octaspire_input_t *octaspire_input_new_from_path(
    char const * const path,
    octaspire_memory_allocator_t *octaspireAllocator,
    octaspire_stdio_t *octaspireStdio);

void octaspire_input_release(octaspire_input_t *self);

size_t octaspire_input_get_length_in_ucs_characters(octaspire_input_t const * const self);

void   octaspire_input_clear(octaspire_input_t *self);

void   octaspire_input_rewind(octaspire_input_t *self);

uint32_t octaspire_input_peek_next_ucs_character(octaspire_input_t *self);
uint32_t octaspire_input_peek_next_next_ucs_character(octaspire_input_t *self);

bool octaspire_input_pop_next_ucs_character(octaspire_input_t *self);

bool octaspire_input_is_good(octaspire_input_t const * const self);

bool octaspire_input_push_back_from_string(
    octaspire_input_t * const self,
    octaspire_container_utf8_string_t const * const str);

bool octaspire_input_push_back_from_c_string(octaspire_input_t * const self, char const * const str);

size_t octaspire_input_get_line_number(octaspire_input_t const * const self);
size_t octaspire_input_get_column_number(octaspire_input_t const * const self);
size_t octaspire_input_get_ucs_character_index(octaspire_input_t const * const self);

void octaspire_input_print(octaspire_input_t const * const self);

#ifdef __cplusplus
}
#endif

#endif

