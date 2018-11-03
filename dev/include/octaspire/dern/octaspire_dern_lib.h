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
#ifndef OCTASPIRE_DERN_LIB_H
#define OCTASPIRE_DERN_LIB_H

#include <stdint.h>
#include <stdbool.h>

#ifndef OCTASPIRE_DERN_DO_NOT_USE_AMALGAMATED_CORE
    #include "octaspire-core-amalgamated.c"
#else
    #include <octaspire/core/octaspire_vector.h>
    #include <octaspire/core/octaspire_map.h>
    #include <octaspire/core/octaspire_string.h>
    #include <octaspire/core/octaspire_input.h>
#endif

#ifdef __cplusplus
extern "C"       {
#endif

typedef enum
{
    OCTASPIRE_DERN_LIB_TAG_SOURCE,
    OCTASPIRE_DERN_LIB_TAG_BINARY,
}
octaspire_dern_lib_tag_t;

typedef struct octaspire_dern_lib_t octaspire_dern_lib_t;
struct octaspire_dern_vm_t;
struct octaspire_dern_c_data_t;

octaspire_dern_lib_t *octaspire_dern_lib_new_source(
    char const * const name,
    octaspire_input_t * const input,
    struct octaspire_dern_vm_t *vm,
    octaspire_allocator_t *allocator);

octaspire_dern_lib_t *octaspire_dern_lib_new_binary(
    char const * const name,
    char const * const fileName,
    struct octaspire_dern_vm_t *vm,
    octaspire_allocator_t *allocator);

void * octaspire_dern_lib_dycall(
    octaspire_dern_lib_t * const self,
    char const * const funcName,
    struct octaspire_dern_c_data_t * const cData);

void * octaspire_dern_lib_dycall_2_const(
    octaspire_dern_lib_t * const self,
    char const * const funcName,
    struct octaspire_dern_c_data_t const * const cData1,
    struct octaspire_dern_c_data_t const * const cData2);

void octaspire_dern_lib_release(octaspire_dern_lib_t *self);

bool octaspire_dern_lib_is_good(octaspire_dern_lib_t const * const self);

char const *octaspire_dern_lib_get_error_message(octaspire_dern_lib_t const * const self);

bool octaspire_dern_lib_mark_all(octaspire_dern_lib_t * const self);

void *octaspire_dern_lib_get_handle(octaspire_dern_lib_t * const self);

struct octaspire_dern_vm_t * octaspire_dern_lib_get_vm(octaspire_dern_lib_t * const self);

#ifdef __cplusplus
/* extern "C" */ }
#endif

#endif

