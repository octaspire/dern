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
#ifndef OCTASPIRE_CONTAINER_HASH_MAP_H
#define OCTASPIRE_CONTAINER_HASH_MAP_H

#include "octaspire_container_vector.h"
#include "octaspire_memory.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Hash map element
typedef struct octaspire_container_hash_map_element_t octaspire_container_hash_map_element_t;

octaspire_container_hash_map_element_t *octaspire_container_hash_map_element_new(
    uint32_t const hash,
    size_t const keySizeInOctets,
    bool const keyIsPointer,
    void const * const key,
    size_t const valueSizeInOctets,
    bool const valueIsPointer,
    void const * const value,
    octaspire_memory_allocator_t * const allocator);

void octaspire_container_hash_map_element_release(octaspire_container_hash_map_element_t *self);

uint32_t octaspire_container_hash_map_element_get_hash(
    octaspire_container_hash_map_element_t const * const self);

void *octaspire_container_hash_map_element_get_key(
    octaspire_container_hash_map_element_t const * const self);

octaspire_container_vector_t *octaspire_container_hash_map_element_get_values(
    octaspire_container_hash_map_element_t * const self);

void *octaspire_container_hash_map_element_get_value(
    octaspire_container_hash_map_element_t const * const self);

// Hash map
typedef struct octaspire_container_hash_map_t octaspire_container_hash_map_t;

typedef bool (*octaspire_container_hash_map_key_compare_function_t)(
    void const * const key1,
    void const * const key2);

typedef uint32_t (*octaspire_container_hash_map_key_hash_function_t)(
    void const * const key);

typedef void (*octaspire_container_hash_map_element_callback_function_t)(
    void * element);

octaspire_container_hash_map_t *octaspire_container_hash_map_new(
    size_t const keySizeInOctets,
    bool const keyIsPointer,
    size_t const valueSizeInOctets,
    bool const valueIsPointer,
    octaspire_container_hash_map_key_compare_function_t keyCompareFunction,
    octaspire_container_hash_map_key_hash_function_t keyHashFunction,
    octaspire_container_hash_map_element_callback_function_t keyReleaseCallback,
    octaspire_container_hash_map_element_callback_function_t valueReleaseCallback,
    octaspire_memory_allocator_t *allocator);

void octaspire_container_hash_map_release(octaspire_container_hash_map_t *self);

bool octaspire_container_hash_map_remove(
    octaspire_container_hash_map_t *self,
    uint32_t const hash,
    void const * const key);

bool octaspire_container_hash_map_clear(
    octaspire_container_hash_map_t * const self);

bool octaspire_container_hash_map_add_hash_map(
    octaspire_container_hash_map_t * const self,
    octaspire_container_hash_map_t * const other);

bool octaspire_container_hash_map_put(
    octaspire_container_hash_map_t *self,
    uint32_t const hash,
    void const * const key,
    void const * const value);

octaspire_container_hash_map_element_t *octaspire_container_hash_map_get(
    octaspire_container_hash_map_t *self,
    uint32_t const hash,
    void const * const key);

octaspire_container_hash_map_element_t const * octaspire_container_hash_map_get_const(
    octaspire_container_hash_map_t const * const self,
    uint32_t const hash,
    void const * const key);

size_t octaspire_container_hash_map_get_number_of_elements(
    octaspire_container_hash_map_t const * const self);

octaspire_container_hash_map_element_t *octaspire_container_hash_map_get_at_index(
    octaspire_container_hash_map_t *self,
    size_t const index);

#ifdef __cplusplus
}
#endif

#endif
