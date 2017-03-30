/******************************************************************************
Octaspire Core - Containers and other utility libraries in standard C99
Copyright 2017 www.octaspire.com

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
******************************************************************************/
#ifndef OCTASPIRE_CONTAINER_UTF8_STRING_H
#define OCTASPIRE_CONTAINER_UTF8_STRING_H

#include <stdarg.h>
#include <stdlib.h>
#include "octaspire_utf8.h"
#include "octaspire_container_vector.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    OCTASPIRE_CONTAINER_UTF8_STRING_ERROR_STATUS_OK,
    OCTASPIRE_CONTAINER_UTF8_STRING_ERROR_STATUS_ENCODING_ERROR,
    OCTASPIRE_CONTAINER_UTF8_STRING_ERROR_STATUS_DECODING_ERROR
}
octaspire_container_utf8_string_error_status_t;

typedef struct octaspire_container_utf8_string_t octaspire_container_utf8_string_t;

octaspire_container_utf8_string_t *octaspire_container_utf8_string_new(
    char const * const str,
    octaspire_memory_allocator_t *allocator);

octaspire_container_utf8_string_t *octaspire_container_utf8_string_new_from_buffer(
    char const * const buffer,
    size_t const lengthInOctets,
    octaspire_memory_allocator_t *allocator);

octaspire_container_utf8_string_t *octaspire_container_utf8_string_new_format(
    octaspire_memory_allocator_t *allocator,
    char const * const fmt,
    ...);

octaspire_container_utf8_string_t *octaspire_container_utf8_string_new_vformat(
    octaspire_memory_allocator_t *allocator,
    char const * const fmt,
    va_list arguments);

octaspire_container_utf8_string_t *octaspire_container_utf8_string_new_copy(
    octaspire_container_utf8_string_t const * const other,
    octaspire_memory_allocator_t *allocator);

octaspire_container_utf8_string_t *octaspire_container_utf8_string_new_substring_from_given_index_and_length(
    octaspire_container_utf8_string_t const * const other,
    octaspire_memory_allocator_t *allocator,
    size_t const ucsCharStartIndex,
    size_t const lengthInUcsChars);

void octaspire_container_utf8_string_release(octaspire_container_utf8_string_t *self);

bool octaspire_container_utf8_string_is_empty(
    octaspire_container_utf8_string_t const * const self);

size_t octaspire_container_utf8_string_get_length_in_ucs_characters(
    octaspire_container_utf8_string_t const * const self);

size_t octaspire_container_utf8_string_get_length_in_octets(
    octaspire_container_utf8_string_t const * const self);

uint32_t octaspire_container_utf8_string_get_ucs_character_at_index(
    octaspire_container_utf8_string_t const * const self, size_t const index);

char const * octaspire_container_utf8_string_get_c_string(
    octaspire_container_utf8_string_t const * const self);

bool octaspire_container_utf8_string_is_error(
    octaspire_container_utf8_string_t const * const self);

octaspire_container_utf8_string_error_status_t octaspire_container_utf8_string_get_error_status(
    octaspire_container_utf8_string_t const * const self);

size_t octaspire_container_utf8_string_get_error_position_in_octets(
    octaspire_container_utf8_string_t const * const self);

void octaspire_container_utf8_string_reset_error_status(
    octaspire_container_utf8_string_t * const self);

bool octaspire_container_utf8_string_concatenate(
    octaspire_container_utf8_string_t * const self,
    octaspire_container_utf8_string_t const * const other);

bool octaspire_container_utf8_string_concatenate_c_string(
    octaspire_container_utf8_string_t * const self,
    char const * const str);

bool octaspire_container_utf8_string_concatenate_format(
    octaspire_container_utf8_string_t * const self,
    char const * const fmt,
    ...);

ptrdiff_t octaspire_container_utf8_string_find_first_substring(
    octaspire_container_utf8_string_t const * const self,
    size_t const startFromIndex,
    octaspire_container_utf8_string_t const * const substring);

bool octaspire_container_utf8_string_remove_character_at(
    octaspire_container_utf8_string_t * const self,
    size_t const index);

size_t octaspire_container_utf8_string_remove_characters_at(
    octaspire_container_utf8_string_t * const self,
    size_t const startFromIndex,
    size_t const numCharacters);

size_t octaspire_container_utf8_string_remove_all_substrings(
    octaspire_container_utf8_string_t * const self,
    octaspire_container_utf8_string_t const * const substring);

bool octaspire_container_utf8_string_clear(
    octaspire_container_utf8_string_t * const self);

bool octaspire_container_utf8_string_is_equal(
    octaspire_container_utf8_string_t const * const self,
    octaspire_container_utf8_string_t const * const other);

bool octaspire_container_utf8_string_is_equal_to_c_string(
    octaspire_container_utf8_string_t const * const self,
    char const * const str);

bool octaspire_container_utf8_string_starts_with(
    octaspire_container_utf8_string_t const * const self,
    octaspire_container_utf8_string_t const * const other);

bool octaspire_container_utf8_string_ends_with(
    octaspire_container_utf8_string_t const * const self,
    octaspire_container_utf8_string_t const * const other);

uint32_t octaspire_container_utf8_string_get_hash(
    octaspire_container_utf8_string_t const * const self);

bool octaspire_container_utf8_string_push_back_ucs_character(
    octaspire_container_utf8_string_t *self,
    uint32_t const character);

bool octaspire_container_utf8_string_insert_string_to(
    octaspire_container_utf8_string_t * const self,
    octaspire_container_utf8_string_t const * const str,
    ptrdiff_t const indexToPutFirstCharacter);

octaspire_container_vector_t *octaspire_container_utf8_string_split(
    octaspire_container_utf8_string_t *self,
    char const * const delimiter);

// TODO make API yhtenäinen, nyt this differs from find_char below
bool octaspire_container_utf8_string_contains_char(
    octaspire_container_utf8_string_t const * const self,
    uint32_t const character);

bool octaspire_container_utf8_string_contains_only_these_chars(
    octaspire_container_utf8_string_t const * const self,
    octaspire_container_utf8_string_t const * const chars);

octaspire_container_vector_t *octaspire_container_utf8_string_find_char(
    octaspire_container_utf8_string_t const * const self,
    octaspire_container_utf8_string_t const * const character,
    size_t const characterIndex);

octaspire_container_vector_t *octaspire_container_utf8_string_find_string(
    octaspire_container_utf8_string_t const * const self,
    octaspire_container_utf8_string_t const * const str,
    size_t const strStartIndex,
    size_t const strLength);

#ifdef __cplusplus
}
#endif

#endif

