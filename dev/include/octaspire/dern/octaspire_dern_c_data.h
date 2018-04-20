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
#ifndef OCTASPIRE_DERN_C_DATA_H
#define OCTASPIRE_DERN_C_DATA_H

#include <stdint.h>
#include <octaspire/core/octaspire_memory.h>
#include <octaspire/core/octaspire_string.h>

#ifdef __cplusplus
extern "C"       {
#endif

typedef struct octaspire_dern_c_data_t octaspire_dern_c_data_t;

octaspire_dern_c_data_t *octaspire_dern_c_data_new(
    char const * const pluginName,
    char const * const typeNameForPayload,
    void * const payload,
    char const * const cleanUpCallbackName,
    char const * const stdLibLenCallbackName,
    char const * const stdLibLinkAtCallbackName,
    char const * const stdLibCopyAtCallbackName,
    bool const copyingAllowed,
    octaspire_allocator_t *allocator);

octaspire_dern_c_data_t *octaspire_dern_c_data_new_copy(
    octaspire_dern_c_data_t * const other,
    octaspire_allocator_t *allocator);

void octaspire_dern_c_data_release(octaspire_dern_c_data_t *self);

octaspire_string_t *octaspire_dern_c_data_to_string(
    octaspire_dern_c_data_t const * const self,
    octaspire_allocator_t * const allocator);

bool octaspire_dern_c_data_is_equal(
    octaspire_dern_c_data_t const * const self,
    octaspire_dern_c_data_t const * const other);

int octaspire_dern_c_data_compare(
    octaspire_dern_c_data_t const * const self,
    octaspire_dern_c_data_t const * const other);

bool octaspire_dern_c_data_is_plugin_and_payload_type_name(
    octaspire_dern_c_data_t const * const self,
    char const * const pluginName,
    char const * const typeNameForPayload);

char const *octaspire_dern_c_data_get_plugin_name(
    octaspire_dern_c_data_t const * const self);

char const *octaspire_dern_c_data_get_payload_typename(
    octaspire_dern_c_data_t const * const self);

void *octaspire_dern_c_data_get_payload(
    octaspire_dern_c_data_t const * const self);

bool octaspire_dern_c_data_is_copying_allowed(
    octaspire_dern_c_data_t const * const self);

#ifdef __cplusplus
/* extern "C" */ }
#endif

#endif

