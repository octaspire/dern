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
#ifndef OCTASPIRE_DERN_STDLIB_H
#define OCTASPIRE_DERN_STDLIB_H

#include "octaspire/dern/octaspire_dern_vm.h"

#ifdef __cplusplus
extern "C" {
#endif

octaspire_dern_value_t *octaspire_dern_vm_builtin_not(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_builtin_abort(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_builtin_return(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_builtin_vector(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_special_and(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_special_or(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_special_do(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_builtin_nth(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_builtin_exit(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_builtin_doc(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_builtin_mutable(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_builtin_len(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_builtin_read_and_eval_path(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_builtin_read_and_eval_string(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_builtin_string_format(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_builtin_to_string(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_builtin_to_integer(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_builtin_print(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_builtin_println(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_builtin_env_new(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_builtin_env_current(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_builtin_env_global(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_builtin_minus_equals(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_builtin_plus_equals(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_builtin_plus_plus(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_builtin_minus_minus(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_builtin_pop_front(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_builtin_mod(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_builtin_slash(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_builtin_times(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_builtin_plus(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_builtin_minus(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_builtin_find(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_builtin_hash_map(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_special_define(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_special_quote(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_special_if(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_special_while(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_special_for(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

// TODO move to other builtins
octaspire_dern_value_t *octaspire_dern_vm_builtin_starts_with_question_mark(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

// TODO move to other builtins
octaspire_dern_value_t *octaspire_dern_vm_builtin_equals(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

// TODO move to other builtins
octaspire_dern_value_t *octaspire_dern_vm_builtin_equals_equals(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

// TODO move to other builtins
octaspire_dern_value_t *octaspire_dern_vm_builtin_exclamation_equals(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_special_less_than(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_special_greater_than(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_special_less_than_or_equal(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_special_greater_than_or_equal(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_special_fn(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

octaspire_dern_value_t *octaspire_dern_vm_builtin_uid(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

#ifdef __cplusplus
}
#endif

#endif

