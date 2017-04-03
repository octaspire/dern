/******************************************************************************
Octaspire Dern - Programming language
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
#ifndef OCTASPIRE_DERN_VALUE_H
#define OCTASPIRE_DERN_VALUE_H

#include <stdint.h>
#include <stdbool.h>
#include <octaspire/core/octaspire_container_vector.h>
#include <octaspire/core/octaspire_container_hash_map.h>
#include <octaspire/core/octaspire_container_utf8_string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    OCTASPIRE_DERN_VALUE_TAG_ILLEGAL,
    OCTASPIRE_DERN_VALUE_TAG_NIL,
    OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,
    OCTASPIRE_DERN_VALUE_TAG_INTEGER,
    OCTASPIRE_DERN_VALUE_TAG_REAL,
    OCTASPIRE_DERN_VALUE_TAG_STRING,
    OCTASPIRE_DERN_VALUE_TAG_CHARACTER,
    OCTASPIRE_DERN_VALUE_TAG_SYMBOL,
    OCTASPIRE_DERN_VALUE_TAG_ERROR,
    OCTASPIRE_DERN_VALUE_TAG_VECTOR,
    OCTASPIRE_DERN_VALUE_TAG_HASH_MAP,
    OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT,
    OCTASPIRE_DERN_VALUE_TAG_FUNCTION,
    OCTASPIRE_DERN_VALUE_TAG_SPECIAL,
    OCTASPIRE_DERN_VALUE_TAG_BUILTIN
}
octaspire_dern_value_tag_t;

struct octaspire_dern_vm_t;

char const * octaspire_dern_value_helper_get_type_as_c_string(octaspire_dern_value_tag_t const typeTag);

typedef struct octaspire_dern_value_t octaspire_dern_value_t;

typedef octaspire_dern_value_t *(*octaspire_dern_c_function)(
    struct octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

typedef struct octaspire_dern_function_t
{
    struct octaspire_dern_value_t *formals;
    struct octaspire_dern_value_t *body;
    struct octaspire_dern_value_t *definitionEnvironment;
    octaspire_memory_allocator_t  *allocator;
}
octaspire_dern_function_t;

octaspire_dern_function_t *octaspire_dern_function_new(
    struct octaspire_dern_value_t *formals,
    struct octaspire_dern_value_t *body,
    struct octaspire_dern_value_t *definitionEnvironment,
    octaspire_memory_allocator_t  *allocator);

void octaspire_dern_function_release(octaspire_dern_function_t *self);

size_t octaspire_dern_function_get_number_of_required_arguments(
    octaspire_dern_function_t const * const self);

octaspire_container_utf8_string_t *octaspire_dern_function_are_all_formals_mentioned_in_docvec(
    octaspire_dern_function_t const * const self,
    octaspire_dern_value_t const * const docvec);

typedef struct octaspire_dern_special_t
{
    octaspire_dern_c_function          cFunction;
    octaspire_memory_allocator_t      *allocator;
    size_t                             numRequiredActualArguments;
}
octaspire_dern_special_t;

octaspire_dern_special_t *octaspire_dern_special_new(
    octaspire_dern_c_function const cFunction,
    octaspire_memory_allocator_t *allocator,
    size_t const numRequiredActualArguments);

void octaspire_dern_special_release(octaspire_dern_special_t *self);

size_t octaspire_dern_special_get_number_of_required_arguments(
    octaspire_dern_special_t const * const self);

typedef struct octaspire_dern_builtin_t
{
    octaspire_dern_c_function          cFunction;
    octaspire_memory_allocator_t      *allocator;
    size_t                             numRequiredActualArguments;
}
octaspire_dern_builtin_t;

octaspire_dern_builtin_t *octaspire_dern_builtin_new(
    octaspire_dern_c_function const cFunction,
    octaspire_memory_allocator_t *allocator,
    size_t const numRequiredActualArguments);

void octaspire_dern_builtin_release(octaspire_dern_builtin_t *self);

size_t octaspire_dern_builtin_get_number_of_required_arguments(
    octaspire_dern_builtin_t const * const self);

struct octaspire_dern_environment_t;

struct octaspire_dern_value_t
{
    bool                         mark;
    size_t                       containerLengthAtTimeOfMarking;
    octaspire_dern_value_tag_t   typeTag;
    octaspire_dern_value_t      *docstr;
    octaspire_dern_value_t      *docvec;
    struct octaspire_dern_vm_t  *vm;
    int32_t                      mutableCounter;
    uintmax_t                    uniqueId;

    union
    {
        bool                                boolean;
        int32_t                             integer;
        double                              real;
        octaspire_container_utf8_string_t   *string;
        octaspire_container_utf8_string_t   *character;
        octaspire_container_utf8_string_t   *symbol;
        octaspire_container_utf8_string_t   *error;
        octaspire_container_vector_t        *vector;
        octaspire_container_hash_map_t      *hashMap;
        struct octaspire_dern_environment_t *environment;
        octaspire_dern_function_t           *function;
        octaspire_dern_special_t            *special;
        octaspire_dern_builtin_t            *builtin;
    }
    value;
};

bool octaspire_dern_value_set(
    octaspire_dern_value_t  * const self,
    octaspire_dern_value_t  * const value);

bool octaspire_dern_value_set_collection(
    octaspire_dern_value_t * const self,
    octaspire_dern_value_t const * const indexOrKey,
    octaspire_dern_value_t * const value);

uint32_t octaspire_dern_value_get_hash(
    octaspire_dern_value_t const * const self);

bool octaspire_dern_value_is_equal(
    octaspire_dern_value_t const * const self,
    octaspire_dern_value_t const * const other);

bool octaspire_dern_value_is_less_than(
    octaspire_dern_value_t const * const self,
    octaspire_dern_value_t const * const other);

bool octaspire_dern_value_is_greater_than(
    octaspire_dern_value_t const * const self,
    octaspire_dern_value_t const * const other);

bool octaspire_dern_value_is_less_than_or_equal(
    octaspire_dern_value_t const * const self,
    octaspire_dern_value_t const * const other);

bool octaspire_dern_value_is_greater_than_or_equal(
    octaspire_dern_value_t const * const self,
    octaspire_dern_value_t const * const other);

octaspire_container_utf8_string_t *octaspire_dern_value_to_string(
    octaspire_dern_value_t const * const self,
    octaspire_memory_allocator_t * const allocator);

octaspire_container_utf8_string_t *octaspire_dern_value_to_string_plain(
    octaspire_dern_value_t const * const self,
    octaspire_memory_allocator_t * const allocator);

bool octaspire_dern_value_is_number(
    octaspire_dern_value_t const * const self);

void octaspire_dern_value_print(
    octaspire_dern_value_t const * const self,
    octaspire_memory_allocator_t *allocator);

uintmax_t octaspire_dern_value_get_unique_id(
    octaspire_dern_value_t const * const self);

bool octaspire_dern_value_as_hash_map_add(
    octaspire_dern_value_t * const self,
    octaspire_dern_value_t * const toBeAdded1,
    octaspire_dern_value_t * const toBeAdded2);

bool octaspire_dern_value_as_character_add(
    octaspire_dern_value_t * const self,
    octaspire_dern_value_t * const other);

bool octaspire_dern_value_as_integer_add(
    octaspire_dern_value_t * const self,
    octaspire_dern_value_t * const other);

bool octaspire_dern_value_as_real_add(
    octaspire_dern_value_t * const self,
    octaspire_dern_value_t * const other);

char const *octaspire_dern_value_as_character_get_c_string(
    octaspire_dern_value_t const * const self);

bool octaspire_dern_value_as_string_push_back(
    octaspire_dern_value_t * const self,
    octaspire_dern_value_t * const value);

char const *octaspire_dern_value_as_string_get_c_string(
    octaspire_dern_value_t const * const self);

char const *octaspire_dern_value_as_symbol_get_c_string(
    octaspire_dern_value_t const * const self);

size_t octaspire_dern_value_as_vector_get_length(
    octaspire_dern_value_t const * const self);

bool octaspire_dern_value_as_vector_is_valid_index(
    octaspire_dern_value_t const * const self,
    ptrdiff_t const index);

bool octaspire_dern_value_as_vector_push_front_element(
    octaspire_dern_value_t *self,
    void const *element);

bool octaspire_dern_value_as_vector_push_back_element(
    octaspire_dern_value_t *self,
    void const *element);

octaspire_dern_value_t *octaspire_dern_value_as_vector_get_element_at(
    octaspire_dern_value_t * const self,
    size_t const index);

octaspire_dern_value_t const *octaspire_dern_value_as_vector_get_element_at_const(
    octaspire_dern_value_t const * const self,
    size_t const index);

octaspire_dern_value_t *octaspire_dern_value_as_vector_get_element_of_type_at(
    octaspire_dern_value_t * const self,
    octaspire_dern_value_tag_t const typeTag,
    size_t const index);

octaspire_dern_value_t const *octaspire_dern_value_as_vector_get_element_of_type_at_const(
    octaspire_dern_value_t const * const self,
    octaspire_dern_value_tag_t const typeTag,
    size_t const index);

// TODO how about as_vector, should it have void* replaced with octaspire_dern_value_t*?
bool octaspire_dern_value_as_hash_map_put(
    octaspire_dern_value_t *self,
    uint32_t const hash,
    octaspire_dern_value_t const * const key,
    octaspire_dern_value_t *value);

size_t octaspire_dern_value_as_hash_map_get_number_of_elements(
    octaspire_dern_value_t const * const self);

octaspire_container_hash_map_element_t *octaspire_dern_value_as_hash_map_get_at_index(
    octaspire_dern_value_t * const self,
    size_t const index);

octaspire_container_hash_map_element_t *octaspire_dern_value_as_hash_map_get(
    octaspire_dern_value_t * const self,
    uint32_t const hash,
    octaspire_dern_value_t const * const key);

size_t octaspire_dern_value_get_length(
    octaspire_dern_value_t const * const self);

bool octaspire_dern_value_mark(octaspire_dern_value_t *self);

int octaspire_dern_value_compare(
    octaspire_dern_value_t const * const self,
    octaspire_dern_value_t const * const other);

bool octaspire_dern_value_is_atom(octaspire_dern_value_t const * const self);

#ifdef __cplusplus
}
#endif

#endif

