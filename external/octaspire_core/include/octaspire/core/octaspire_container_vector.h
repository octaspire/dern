#ifndef OCTASPIRE_CONTAINER_VECTOR_H
#define OCTASPIRE_CONTAINER_VECTOR_H

#include <stdbool.h>
#include <stddef.h>
#include "octaspire_memory.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct octaspire_container_vector_t octaspire_container_vector_t;

typedef void  (*octaspire_container_vector_element_callback_t)(void *element);

typedef int (*octaspire_container_vector_element_compare_function_t)(void const *a, void const *b);

octaspire_container_vector_t *octaspire_container_vector_new(
    size_t const elementSize,
    bool const elementIsPointer,
    octaspire_container_vector_element_callback_t elementReleaseCallback,
    octaspire_memory_allocator_t *allocator);

octaspire_container_vector_t *octaspire_container_vector_new_with_preallocated_elements(
    size_t const elementSize,
    bool const elementIsPointer,
    size_t const numElementsPreAllocated,
    octaspire_container_vector_element_callback_t elementReleaseCallback,
    octaspire_memory_allocator_t *allocator);

octaspire_container_vector_t *octaspire_container_vector_new_shallow_copy(
    octaspire_container_vector_t * other,
    octaspire_memory_allocator_t * allocator);

void octaspire_container_vector_release(octaspire_container_vector_t *self);

// Vector can never be compacted smaller than this limit, if set
void octaspire_container_vector_set_compacting_limit_for_preallocated_elements(
    octaspire_container_vector_t * const self,
    size_t const numPreAllocatedElementsAtLeastPresentAtAnyMoment);

size_t octaspire_container_vector_get_length(
    octaspire_container_vector_t const * const self);

size_t octaspire_container_vector_get_length_in_octets(
    octaspire_container_vector_t const * const self);

bool octaspire_container_vector_is_empty(
    octaspire_container_vector_t const * const self);

bool octaspire_container_vector_remove_element_at(
    octaspire_container_vector_t * const self,
    size_t const index);

void *octaspire_container_vector_get_element_at(
    octaspire_container_vector_t * const self,
    size_t const index);

void const *octaspire_container_vector_get_element_at_const(
    octaspire_container_vector_t const * const self,
    size_t const index);

size_t octaspire_container_vector_get_element_size_in_octets(
    octaspire_container_vector_t const * const self);

bool octaspire_container_vector_insert_element_before_the_element_at_index(
    octaspire_container_vector_t *self,
    void const *element,
    ptrdiff_t const index);

bool octaspire_container_vector_insert_element_at(
    octaspire_container_vector_t * const self,
    void const * const element,
    size_t const index);

bool octaspire_container_vector_replace_element_at(
    octaspire_container_vector_t *self,
    size_t const index,
    void const *element);

bool octaspire_container_vector_push_front_element(
    octaspire_container_vector_t *self,
    void const *element);

bool octaspire_container_vector_push_back_element(
    octaspire_container_vector_t * const self,
    void const * const element);

bool octaspire_container_vector_push_back_char(
    octaspire_container_vector_t *self,
    char const element);

void octaspire_container_vector_for_each(
    octaspire_container_vector_t *self,
    octaspire_container_vector_element_callback_t callback);

bool octaspire_container_vector_pop_back_element(
    octaspire_container_vector_t *self);

void *octaspire_container_vector_peek_back_element(
    octaspire_container_vector_t *self);

void const * octaspire_container_vector_peek_back_element_const(
    octaspire_container_vector_t const * const self);

bool octaspire_container_vector_pop_front_element(
    octaspire_container_vector_t *self);

void *octaspire_container_vector_peek_front_element(
    octaspire_container_vector_t *self);

void const * octaspire_container_vector_peek_front_element_const(
    octaspire_container_vector_t const * const self);

octaspire_container_vector_element_callback_t
octaspire_container_vector_get_element_release_callback_const(
    octaspire_container_vector_t const * const self);

bool octaspire_container_vector_clear(
    octaspire_container_vector_t * const self);

void octaspire_container_vector_sort(
    octaspire_container_vector_t * const self,
    octaspire_container_vector_element_compare_function_t elementCompareFunction);

bool octaspire_container_vector_is_valid_index(
    octaspire_container_vector_t const * const self,
    ptrdiff_t const index);

#ifdef __cplusplus
}
#endif

#endif

