#ifndef OCTASPIRE_DERN_ENVIRONMENT_H
#define OCTASPIRE_DERN_ENVIRONMENT_H

#include <octaspire/core/octaspire_container_hash_map.h>
#include "octaspire/dern/octaspire_dern_value.h"

#ifdef __cplusplus
extern "C" {
#endif

struct octaspire_dern_vm_t;

typedef struct octaspire_dern_environment_t
{
    octaspire_container_hash_map_t      *bindings;
    struct octaspire_dern_value_t       *enclosing;
    struct octaspire_dern_vm_t          *vm;
    octaspire_memory_allocator_t        *allocator;
}
octaspire_dern_environment_t;

struct octaspire_dern_vm_t;

octaspire_dern_environment_t *octaspire_dern_environment_new(
    octaspire_dern_value_t *enclosing,
    struct octaspire_dern_vm_t *vm,
    octaspire_memory_allocator_t *allocator);

void octaspire_dern_environment_release(octaspire_dern_environment_t *self);

// Returns 0 or error
octaspire_dern_value_t *octaspire_dern_environment_extend(
    octaspire_dern_environment_t *self,
    octaspire_dern_value_t *formals,
    octaspire_dern_value_t *arguments);

octaspire_dern_value_t *octaspire_dern_environment_get(
    octaspire_dern_environment_t *self,
    octaspire_dern_value_t const * const key);

bool octaspire_dern_environment_set(
    octaspire_dern_environment_t *self,
    octaspire_dern_value_t const * const key,
    octaspire_dern_value_t *value);

octaspire_container_utf8_string_t *octaspire_dern_environment_to_string(
    octaspire_dern_environment_t const * const self);

bool octaspire_dern_environment_print(
    octaspire_dern_environment_t const * const self);

size_t octaspire_dern_environment_get_length(
    octaspire_dern_environment_t const * const self);

octaspire_container_hash_map_element_t *octaspire_dern_environment_get_at_index(
    octaspire_dern_environment_t *self,
    size_t const index);

bool octaspire_dern_environment_mark(octaspire_dern_environment_t *self);

#ifdef __cplusplus
}
#endif

#endif

