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
#ifndef OCTASPIRE_DERN_VM_H
#define OCTASPIRE_DERN_VM_H

#include <stdbool.h>

#ifndef OCTASPIRE_DERN_DO_NOT_USE_AMALGAMATED_CORE
    #include "octaspire-core-amalgamated.c"
#else
    #include <octaspire/core/octaspire_memory.h>
    #include <octaspire/core/octaspire_input.h>
    #include <octaspire/core/octaspire_vector.h>
    #include <octaspire/core/octaspire_string.h>
#endif

#include "octaspire/dern/octaspire_dern_environment.h"
#include "octaspire/dern/octaspire_dern_value.h"
#include "octaspire/dern/octaspire_dern_lib.h"
#include "octaspire/dern/octaspire_dern_c_data.h"

#ifdef __cplusplus
extern "C"       {
#endif

typedef octaspire_input_t*
    (*octaspire_dern_vm_custom_require_source_file_loader_t)(
            char const * const,
            octaspire_allocator_t * const allocator);

typedef struct octaspire_dern_vm_config_t
{
    octaspire_dern_vm_custom_require_source_file_loader_t preLoaderForRequireSrc;
    bool debugModeOn;
    bool noDlClose;
}
octaspire_dern_vm_config_t;

octaspire_dern_vm_config_t octaspire_dern_vm_config_default(void);

typedef struct octaspire_dern_vm_t octaspire_dern_vm_t;
struct octaspire_dern_value_t;

octaspire_dern_vm_t *octaspire_dern_vm_new(
    octaspire_allocator_t *allocator,
    octaspire_stdio_t *octaspireStdio);

octaspire_dern_vm_t *octaspire_dern_vm_new_with_config(
    octaspire_allocator_t *allocator,
    octaspire_stdio_t *octaspireStdio,
    octaspire_dern_vm_config_t const config);

void octaspire_dern_vm_release(octaspire_dern_vm_t *self);

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_copy(
    octaspire_dern_vm_t *self,
    octaspire_dern_value_t *valueToBeCopied);

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_input_file(
    octaspire_dern_vm_t *self,
    char const * const path);

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_output_file(
    octaspire_dern_vm_t *self,
    char const * const path);

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_io_file(
    octaspire_dern_vm_t *self,
    char const * const path);

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_nil(octaspire_dern_vm_t *self);

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_boolean(
    octaspire_dern_vm_t *self,
    bool const value);

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_integer(
    octaspire_dern_vm_t *self,
    int32_t const value);

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_real(
    octaspire_dern_vm_t *self,
    double const value);

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_string(
    octaspire_dern_vm_t *self,
    octaspire_string_t * const value);

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_string_format(
    octaspire_dern_vm_t *self,
    char const * const fmt,
    ...);

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_string_from_c_string(
    octaspire_dern_vm_t *self,
    char const * const value);

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_character(
    octaspire_dern_vm_t *self,
    octaspire_string_t * const value);

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_character_from_uint32t(
    octaspire_dern_vm_t *self,
    uint32_t const value);

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_symbol(
    octaspire_dern_vm_t *self,
    octaspire_string_t * const value);

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_symbol_from_c_string(
    octaspire_dern_vm_t *self,
    char const * const value);

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_error(
    octaspire_dern_vm_t *self,
    octaspire_string_t * const value);

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_error_format(
    octaspire_dern_vm_t *self,
    char const * const fmt,
    ...);

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_error_from_c_string(
    octaspire_dern_vm_t *self,
    char const * const value);

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_vector_from_vector(
    octaspire_dern_vm_t *self,
    octaspire_vector_t * const value);

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_vector_from_values(
    octaspire_dern_vm_t *self,
    size_t const numArgs,
    ...);

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_vector(
    octaspire_dern_vm_t *self);

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_hash_map(
    octaspire_dern_vm_t *self);

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_queue(
    octaspire_dern_vm_t *self);

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_queue_with_max_length (
        octaspire_dern_vm_t * const self,
        size_t const maxLength);

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_list(octaspire_dern_vm_t *self);

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_hash_map_from_hash_map(
    octaspire_dern_vm_t *self,
    octaspire_map_t * const value);

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_environment(
    octaspire_dern_vm_t *self,
    octaspire_dern_value_t *enclosing);

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_environment_from_environment(
    octaspire_dern_vm_t *self,
    octaspire_dern_environment_t * const value);

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_function(
    octaspire_dern_vm_t *self,
    octaspire_dern_function_t * const value,
    char const * const docstr,
    octaspire_vector_t *docVec);

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_special(
    octaspire_dern_vm_t *self,
    octaspire_dern_special_t * const value,
    char const * const docstr);

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_builtin(
    octaspire_dern_vm_t *self,
    octaspire_dern_builtin_t * const value,
    char const * const docstr);

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_c_data(
    octaspire_dern_vm_t * const self,
    char const * const pluginName,
    char const * const typeNameForPayload,
    char const * const cleanUpCallbackName,
    char const * const stdLibLenCallbackName,
    char const * const stdLibLinkAtCallbackName,
    char const * const stdLibCopyAtCallbackName,
    bool const copyingAllowed,
    void * const payload);

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_c_data_from_existing(
    octaspire_dern_vm_t * const self,
    octaspire_dern_c_data_t * const cData);

bool octaspire_dern_vm_push_value(octaspire_dern_vm_t *self, struct octaspire_dern_value_t *value);

bool octaspire_dern_vm_pop_value(
    octaspire_dern_vm_t *self,
    struct octaspire_dern_value_t *valueForBalanceCheck);

void const * octaspire_dern_vm_get_top_value(octaspire_dern_vm_t const * const self);

struct octaspire_dern_value_t *octaspire_dern_vm_peek_value(octaspire_dern_vm_t *self);

bool octaspire_dern_vm_gc(octaspire_dern_vm_t *self);

octaspire_dern_value_t *octaspire_dern_vm_parse(
    octaspire_dern_vm_t *self,
    octaspire_input_t *input);

octaspire_dern_value_t *octaspire_dern_vm_eval_in_global_environment(
    octaspire_dern_vm_t *self,
    octaspire_dern_value_t *value);

octaspire_dern_value_t *octaspire_dern_vm_eval(
    octaspire_dern_vm_t *self,
    octaspire_dern_value_t *value,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_read_from_octaspire_input_and_eval_in_global_environment(
    octaspire_dern_vm_t *self,
    octaspire_input_t * const input);

octaspire_dern_value_t *octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
    octaspire_dern_vm_t *self,
    char const * const str);

octaspire_dern_value_t *octaspire_dern_vm_read_from_buffer_and_eval_in_global_environment(
    octaspire_dern_vm_t *self,
    char const * const buffer,
    size_t const lengthInOctets);

octaspire_dern_value_t *octaspire_dern_vm_read_from_path_and_eval_in_global_environment(
    octaspire_dern_vm_t *self,
    char const * const path);

octaspire_dern_value_t *octaspire_dern_vm_get_value_nil(
    octaspire_dern_vm_t *self);

octaspire_dern_value_t *octaspire_dern_vm_get_value_true(
    octaspire_dern_vm_t *self);

octaspire_dern_value_t *octaspire_dern_vm_get_value_false(
    octaspire_dern_vm_t *self);

octaspire_allocator_t *octaspire_dern_vm_get_allocator(
    octaspire_dern_vm_t *self);

void octaspire_dern_vm_set_exit_code(
    octaspire_dern_vm_t *self,
    int32_t const code);

int32_t octaspire_dern_vm_get_exit_code(
    octaspire_dern_vm_t const * const self);

bool octaspire_dern_vm_is_quit(
    octaspire_dern_vm_t const * const self);

void octaspire_dern_vm_quit(
    octaspire_dern_vm_t *self);

bool octaspire_dern_vm_create_and_register_new_builtin(
    octaspire_dern_vm_t * const self,
    char const * const name,
    octaspire_dern_c_function const funcPointer,
    size_t const numRequiredActualArguments,
    char const * const docStr,
    bool const howtoAllowed,
    octaspire_dern_environment_t * const targetEnv);

bool octaspire_dern_vm_create_and_register_new_special(
    octaspire_dern_vm_t * const self,
    char const * const name,
    octaspire_dern_c_function const funcPointer,
    size_t const numRequiredActualArguments,
    char const * const docStr,
    bool const howtoAllowed,
    octaspire_dern_environment_t * const targetEnv);

bool octaspire_dern_vm_create_and_define_new_integer(
    octaspire_dern_vm_t * const self,
    char const * const name,
    char const * const docstr,
    int32_t const value);

size_t octaspire_dern_vm_get_stack_length(
    octaspire_dern_vm_t const * const self);

void octaspire_dern_vm_print_stack(
    octaspire_dern_vm_t const * const self);

octaspire_dern_value_t *
octaspire_dern_vm_helper_create_new_value_vector_of_integers_from_vector_of_size_t(
    octaspire_dern_vm_t * const self,
    octaspire_vector_t const * const vectorContainingSizeTs);

octaspire_dern_value_t *octaspire_dern_vm_find_from_value(
    octaspire_dern_vm_t * const self,
    octaspire_dern_value_t * const value,
    octaspire_dern_value_t const * const key);

octaspire_dern_value_t *octaspire_dern_vm_get_global_environment(
    octaspire_dern_vm_t *self);

octaspire_dern_value_t const *octaspire_dern_vm_get_global_environment_const(
    octaspire_dern_vm_t const * const self);

void  octaspire_dern_vm_set_user_data(octaspire_dern_vm_t * const self, void *userData);
void *octaspire_dern_vm_get_user_data(octaspire_dern_vm_t const * const self);

void octaspire_dern_vm_clear_value_to_nil(
    octaspire_dern_vm_t *self,
    octaspire_dern_value_t *value);

octaspire_dern_value_t *octaspire_dern_vm_get_function_return(
    octaspire_dern_vm_t * const self);

void octaspire_dern_vm_set_function_return(
    octaspire_dern_vm_t * const self,
    octaspire_dern_value_t * const value);

void octaspire_dern_vm_set_prevent_gc(octaspire_dern_vm_t * const self, bool const prevent);

void octaspire_dern_vm_set_gc_trigger_limit(
    octaspire_dern_vm_t * const self,
    size_t const numAllocs);

octaspire_dern_vm_custom_require_source_file_loader_t
octaspire_dern_vm_get_custom_require_source_file_pre_loader(
        octaspire_dern_vm_t * const self);

bool octaspire_dern_vm_add_library(
    octaspire_dern_vm_t *self,
    char const * const name,
    octaspire_dern_lib_t *library);

bool octaspire_dern_vm_has_library(
    octaspire_dern_vm_t const * const self,
    char const * const name);

octaspire_dern_lib_t *octaspire_dern_vm_get_library(
    octaspire_dern_vm_t * const self,
    char const * const name);

bool octaspire_dern_vm_add_command_line_argument(
    octaspire_dern_vm_t * const self,
    char const * const argument);

bool octaspire_dern_vm_add_environment_variable(
    octaspire_dern_vm_t * const self,
    char const * const variable);

octaspire_stdio_t *octaspire_dern_vm_get_stdio(octaspire_dern_vm_t * const self);

size_t octaspire_dern_vm_get_number_of_command_line_arguments(
    octaspire_dern_vm_t const * const self);

char const *octaspire_dern_vm_get_command_line_argument_at(
    octaspire_dern_vm_t const * const self,
    ptrdiff_t const index);

size_t octaspire_dern_vm_get_number_of_environment_variables(
    octaspire_dern_vm_t const * const self);

char const *octaspire_dern_vm_get_environment_variable_at(
    octaspire_dern_vm_t const * const self,
    ptrdiff_t const index);

octaspire_dern_vm_config_t const * octaspire_dern_vm_get_config_const(
    octaspire_dern_vm_t const * const self);

#ifdef __cplusplus
/* extern "C" */ }
#endif

#endif

