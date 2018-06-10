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
#include "octaspire/dern/octaspire_dern_vm.h"
#include <assert.h>
#include <inttypes.h>
#include <string.h>

#ifndef OCTASPIRE_DERN_DO_NOT_USE_AMALGAMATED_CORE
    #include "octaspire-core-amalgamated.c"
#else
    #include <octaspire/core/octaspire_helpers.h>
    #include <octaspire/core/octaspire_input.h>
    #include <octaspire/core/octaspire_vector.h>
    #include <octaspire/core/octaspire_map.h>
    #include <octaspire/core/octaspire_helpers.h>
#endif

#include "octaspire/dern/octaspire_dern_value.h"
#include "octaspire/dern/octaspire_dern_environment.h"
#include "octaspire/dern/octaspire_dern_lexer.h"
#include "octaspire/dern/octaspire_dern_stdlib.h"


static void octaspire_dern_vm_private_release_value(
    octaspire_dern_vm_t *self,
    octaspire_dern_value_t *value);

static
octaspire_dern_value_t *octaspire_dern_vm_private_create_new_value_queue_from_queue(
    octaspire_dern_vm_t *self,
    octaspire_queue_t * const queue);

static octaspire_dern_value_t *octaspire_dern_vm_private_create_new_value_list_from_list(
    octaspire_dern_vm_t *self,
    octaspire_list_t * const list);


struct octaspire_dern_vm_t
{
    octaspire_vector_t        *stack;
    octaspire_allocator_t     *allocator;
    octaspire_stdio_t         *stdio;
    octaspire_vector_t        *all;
    octaspire_dern_value_t    *globalEnvironment;
    octaspire_dern_value_t    *valueNil;
    octaspire_dern_value_t    *valueTrue;
    octaspire_dern_value_t    *valueFalse;
    void                      *userData;
    octaspire_dern_value_t    *functionReturn;
    octaspire_map_t           *libraries;
    octaspire_vector_t        *commandLineArguments;
    octaspire_vector_t        *environmentVariables;
    size_t                     numAllocatedWithoutGc;
    size_t                     gcTriggerLimit;
    uintmax_t                  nextFreeUniqueIdForValues;
    int32_t                    exitCode;
    bool                       preventGc;
    bool                       quit;
    octaspire_dern_vm_config_t config;
};

octaspire_dern_value_t *octaspire_dern_vm_private_create_new_value_struct(
    octaspire_dern_vm_t* self,
    octaspire_dern_value_tag_t const typeTag);

bool octaspire_dern_vm_private_mark_all(octaspire_dern_vm_t *self);
bool octaspire_dern_vm_private_mark(octaspire_dern_vm_t *self, octaspire_dern_value_t *value);
bool octaspire_dern_vm_private_sweep(octaspire_dern_vm_t *self);

octaspire_dern_vm_config_t octaspire_dern_vm_config_default(void)
{
    octaspire_dern_vm_config_t result =
    {
        .preLoaderForRequireSrc  = 0,
        .debugModeOn             = false,
        .noDlClose               = false
    };

    return result;
}

octaspire_dern_vm_t *octaspire_dern_vm_new(
    octaspire_allocator_t *allocator,
    octaspire_stdio_t *octaspireStdio)
{
    return octaspire_dern_vm_new_with_config(
        allocator,
        octaspireStdio,
        octaspire_dern_vm_config_default());
}

octaspire_dern_vm_t *octaspire_dern_vm_new_with_config(
    octaspire_allocator_t *allocator,
    octaspire_stdio_t *octaspireStdio,
    octaspire_dern_vm_config_t const config)
{
    octaspire_dern_vm_t *self =
        octaspire_allocator_malloc(allocator, sizeof(octaspire_dern_vm_t));

    if (!self)
    {
        return 0;
    }

    self->allocator                 = allocator;
    self->stdio                     = octaspireStdio;
    self->numAllocatedWithoutGc     = 0;
    self->preventGc                 = false;
    self->gcTriggerLimit            = 1024;
    self->exitCode                  = 0;
    self->quit                      = false;
    self->userData                  = 0;
    self->nextFreeUniqueIdForValues = 0;
    self->functionReturn            = 0;
    self->config                    = config;

    self->libraries =
        octaspire_map_new_with_octaspire_string_keys(
            sizeof(octaspire_dern_lib_t*),
            true,
            (octaspire_map_element_callback_t)
                octaspire_dern_lib_release,
            self->allocator);

    octaspire_helpers_verify_not_null(self->libraries);

    self->commandLineArguments = octaspire_vector_new(
        sizeof(octaspire_string_t*),
        true,
        (octaspire_vector_element_callback_t)
            octaspire_string_release,
        self->allocator);

    octaspire_helpers_verify_not_null(self->commandLineArguments);

    self->environmentVariables = octaspire_vector_new(
        sizeof(octaspire_string_t*),
        true,
        (octaspire_vector_element_callback_t)
            octaspire_string_release,
        self->allocator);

    octaspire_helpers_verify_not_null(self->environmentVariables);

    self->stack = octaspire_vector_new_with_preallocated_elements(
        sizeof(octaspire_dern_value_t*),
        true,
        256,
        0,
        self->allocator);

    if (!self->stack)
    {
        octaspire_dern_vm_release(self);
        self = 0;
        return 0;
    }

    self->all = octaspire_vector_new(
        sizeof(octaspire_dern_value_t*),
        true,
        0,
        self->allocator);

    if (!self->all)
    {
        octaspire_dern_vm_release(self);
        self = 0;
        return 0;
    }

    octaspire_dern_environment_t *env =
        octaspire_dern_environment_new(0, self, self->allocator);

    if (!env)
    {
        octaspire_dern_vm_release(self);
        self = 0;
        return 0;
    }

    self->globalEnvironment =
        octaspire_dern_vm_create_new_value_environment_from_environment(self, env);

    if (!self->globalEnvironment)
    {
        octaspire_dern_vm_release(self);
        self = 0;
        return 0;
    }

    if (!octaspire_dern_vm_push_value(self, self->globalEnvironment))
    {
        abort();
    }

    self->valueNil = octaspire_dern_vm_create_new_value_nil(self);

    if (!octaspire_dern_vm_push_value(self, self->valueNil))
    {
        abort();
    }

    self->valueNil->docstr = octaspire_dern_vm_create_new_value_string_from_c_string(
        self,
        "Represents missing value.");

    if (!octaspire_dern_vm_push_value(self, self->valueNil->docstr))
    {
        abort();
    }

    if (!octaspire_dern_environment_set(
        env,
        octaspire_dern_vm_create_new_value_symbol(self, octaspire_string_new(
            "nil",
            self->allocator)),
        self->valueNil))
    {
        abort();
    }

    if (!octaspire_dern_vm_pop_value(self, self->valueNil->docstr))
    {
        abort();
    }

    if (!octaspire_dern_vm_pop_value(self, self->valueNil))
    {
        abort();
    }

    self->valueTrue = octaspire_dern_vm_create_new_value_boolean(self, true);

    if (!octaspire_dern_vm_push_value(self, self->valueTrue))
    {
        abort();
    }

    self->valueTrue->docstr = octaspire_dern_vm_create_new_value_string_from_c_string(
        self,
        "Boolean true value. Opposite of false.");

    if (!octaspire_dern_vm_push_value(self, self->valueTrue->docstr))
    {
        abort();
    }

    if (!octaspire_dern_environment_set(
        env,
        octaspire_dern_vm_create_new_value_symbol(self, octaspire_string_new(
            "true",
            self->allocator)),
        self->valueTrue))
    {
        abort();
    }

    if (!octaspire_dern_vm_pop_value(self, self->valueTrue->docstr))
    {
        abort();
    }

    if (!octaspire_dern_vm_pop_value(self, self->valueTrue))
    {
        abort();
    }

    self->valueFalse = octaspire_dern_vm_create_new_value_boolean(self, false);

    if (!octaspire_dern_vm_push_value(self, self->valueFalse))
    {
        abort();
    }

    self->valueFalse->docstr = octaspire_dern_vm_create_new_value_string_from_c_string(
        self,
        "Boolean false value. Opposite of true.");

    if (!octaspire_dern_vm_push_value(self, self->valueFalse->docstr))
    {
        abort();
    }

    if (!octaspire_dern_environment_set(
        env,
        octaspire_dern_vm_create_new_value_symbol(self, octaspire_string_new(
            "false",
            self->allocator)),
        self->valueFalse))
    {
        abort();
    }

    if (!octaspire_dern_vm_pop_value(self, self->valueFalse->docstr))
    {
        abort();
    }

    if (!octaspire_dern_vm_pop_value(self, self->valueFalse))
    {
        abort();
    }


    ////////////////////////////////// Builtins and specials /////////////////////////////////////

    //////////////////////////////////////// Builtins ////////////////////////////////////////////

    // host-get-command-line-arguments
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "host-get-command-line-arguments",
        octaspire_dern_vm_builtin_host_get_command_line_arguments,
        0,
        "Get vector containing the host command line arguments",
        false,
        env))
    {
        abort();
    }

    // host-get-environment-variables
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "host-get-environment-variables",
        octaspire_dern_vm_builtin_host_get_environment_variables,
        0,
        "Get vector containing the host environment variables",
        false,
        env))
    {
        abort();
    }

    // ln@
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "ln@",
        octaspire_dern_vm_builtin_ln_at_sign,
        2,
        "Get reference to a value in a given collection at the given index",
        true,
        env))
    {
        abort();
    }

    // cp@
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "cp@",
        octaspire_dern_vm_builtin_cp_at_sign,
        2,
        "Get copy of a value in a given collection at the given index",
        true,
        env))
    {
        abort();
    }

    // require
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "require",
        octaspire_dern_vm_builtin_require,
        1,
        "Ensure that plugin is loaded (if dern is compiled with plugin support)",
        false,
        env))
    {
        abort();
    }

    // input-file-open
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "input-file-open",
        octaspire_dern_vm_builtin_input_file_open,
        1,
        "Open file-port for reading only",
        false,
        env))
    {
        abort();
    }

    // output-file-open
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "output-file-open",
        octaspire_dern_vm_builtin_output_file_open,
        1,
        "Open file-port for writing only",
        false,
        env))
    {
        abort();
    }

    // io-file-open
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "io-file-open",
        octaspire_dern_vm_builtin_io_file_open,
        1,
        "Open file-port for input and output",
        false,
        env))
    {
        abort();
    }

    // port-supports-output?
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "port-supports-output?",
        octaspire_dern_vm_builtin_port_supports_output_question_mark,
        1,
        "Predicate telling whether the given port supports writing or not",
        false,
        env))
    {
        abort();
    }

    // port-supports-input?
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "port-supports-input?",
        octaspire_dern_vm_builtin_port_supports_input_question_mark,
        1,
        "Predicate telling whether the given port supports reading or not",
        false,
        env))
    {
        abort();
    }

    // port-close
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "port-close",
        octaspire_dern_vm_builtin_port_close,
        1,
        "Close a port",
        false,
        env))
    {
        abort();
    }

    // port-read
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "port-read",
        octaspire_dern_vm_builtin_port_read,
        1,
        "Read from a port one or a given number of octets",
        false,
        env))
    {
        abort();
    }

    // port-write
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "port-write",
        octaspire_dern_vm_builtin_port_write,
        1,
        "Write one integer or all integers from a vector to a port supporting writing",
        false,
        env))
    {
        abort();
    }

    // port-seek
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "port-seek",
        octaspire_dern_vm_builtin_port_seek,
        1,
        "Seek to a position from the beginning (positive integer) or "
        "from the end (negative integer) on ports supporting seeking",
        false,
        env))
    {
        abort();
    }

    // port-dist
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "port-dist",
        octaspire_dern_vm_builtin_port_dist,
        1,
        "Get distance from the beginning on ports that support it, or minus one",
        false,
        env))
    {
        abort();
    }

    // port-length
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "port-length",
        octaspire_dern_vm_builtin_port_length,
        1,
        "Get size of port in octets on ports that support this operation, or minus one",
        false,
        env))
    {
        abort();
    }

    // port-flush
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "port-flush",
        octaspire_dern_vm_builtin_port_flush,
        1,
        "Flush an output port that supports flushing",
        false,
        env))
    {
        abort();
    }

    // not
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "not",
        octaspire_dern_vm_builtin_not,
        1,
        "Reverse boolean value",
        true,
        env))
    {
        abort();
    }

    // abort
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "abort",
        octaspire_dern_vm_builtin_abort,
        1,
        "Quit execution with error message",
        false,
        env))
    {
        abort();
    }

    // return
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "return",
        octaspire_dern_vm_builtin_return,
        1,
        "Return from function early with the given value",
        false,
        env))
    {
        abort();
    }

    // vector
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "vector",
        octaspire_dern_vm_builtin_vector,
        1,
        "Create new vector of the given values.",
        true,
        env))
    {
        abort();
    }

    // and
    if (!octaspire_dern_vm_create_and_register_new_special(
        self,
        "and",
        octaspire_dern_vm_special_and,
        0,
        "Evaluate values until 'false' is found. Any remaining values are not evaluated. "
        "Return the last evaluated value. "
        "If there are no values, then 'true' is returned.",
        true,
        env))
    {
        abort();
    }

    // or
    if (!octaspire_dern_vm_create_and_register_new_special(
        self,
        "or",
        octaspire_dern_vm_special_or,
        0,
        "Evaluate values until 'true' is found. Any remaining values are not evaluated. "
        "Return the last evaluated value. "
        "If there are no values, then 'false' is returned.",
        true,
        env))
    {
        abort();
    }

    // do
    if (!octaspire_dern_vm_create_and_register_new_special(
        self,
        "do",
        octaspire_dern_vm_special_do,
        1,
        "Evaluate sequence of values and return the value of the last evaluation",
        false,
        env))
    {
        abort();
    }

    // starts-with?
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "starts-with?",
        octaspire_dern_vm_builtin_starts_with_question_mark,
        1,
        "Does the first value start with the second?",
        true,
        env))
    {
        abort();
    }

    // =
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "=",
        octaspire_dern_vm_builtin_equals,
        1,
        "Set atomic values, or elements of collections (vector, map, string) at the given "
        "index/key",
        false,
        env))
    {
        abort();
    }



    // string-format
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "string-format",
        octaspire_dern_vm_builtin_string_format,
        1,
        "Create new string and allow formatting of value into it using {}",
        true,
        env))
    {
        abort();
    }

    // to-string
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "to-string",
        octaspire_dern_vm_builtin_to_string,
        1,
        "Give value or values as string(s)",
        true,
        env))
    {
        abort();
    }

    // to-integer
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "to-integer",
        octaspire_dern_vm_builtin_to_integer,
        1,
        "Give value or values as integer(s)",
        true,
        env))
    {
        abort();
    }

    // print
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "print",
        octaspire_dern_vm_builtin_print,
        0,
        "Print message for the user",
        false,
        env))
    {
        abort();
    }

    // println
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "println",
        octaspire_dern_vm_builtin_println,
        0,
        "Print message for the user and newline",
        false,
        env))
    {
        abort();
    }

    // env-new
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "env-new",
        octaspire_dern_vm_builtin_env_new,
        0,
        "Create new empty environment",
        false,
        env))
    {
        abort();
    }

    // env-current
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "env-current",
        octaspire_dern_vm_builtin_env_current,
        0,
        "Get the current environment used by the context where this is evaluated",
        false,
        env))
    {
        abort();
    }

    // env-global
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "env-global",
        octaspire_dern_vm_builtin_env_global,
        0,
        "Get the global environment",
        false,
        env))
    {
        abort();
    }

    // -=
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "-=",
        octaspire_dern_vm_builtin_minus_equals,
        1,
        "Subtract value or values from the first argument (modify it)",
        false,
        env))
    {
        abort();
    }

    // -==
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "-==",
        octaspire_dern_vm_builtin_minus_equals_equals,
        1,
        "Remove from supported collection a value that has the same unique id "
        "than the given value. Note that equal values are probably not the same",
        false,
        env))
    {
        abort();
    }

    // pop-back
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "pop-back",
        octaspire_dern_vm_builtin_pop_back,
        1,
        "Remove the last value from supported collection.",
        false,
        env))
    {
        abort();
    }

    // pop-front
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "pop-front",
        octaspire_dern_vm_builtin_pop_front,
        1,
        "Remove the first value from supported collection.",
        false,
        env))
    {
        abort();
    }

    // +=
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "+=",
        octaspire_dern_vm_builtin_plus_equals,
        1,
        "Add value or values into the first argument (modify it)",
        false,
        env))
    {
        abort();
    }

    // ++
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "++",
        octaspire_dern_vm_builtin_plus_plus,
        1,
        "Increase a value or values by one",
        false,
        env))
    {
        abort();
    }

    // --
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "--",
        octaspire_dern_vm_builtin_minus_minus,
        1,
        "Decrease a value or values by one",
        false,
        env))
    {
        abort();
    }

    // mod
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "mod",
        octaspire_dern_vm_builtin_mod,
        1,
        "Calculate modulo of two integers",
        true,
        env))
    {
        abort();
    }

    // /
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "/",
        octaspire_dern_vm_builtin_slash,
        1,
        "Divide number arguments",
        true,
        env))
    {
        abort();
    }

    // *
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "*",
        octaspire_dern_vm_builtin_times,
        1,
        "Multiply number arguments",
        true,
        env))
    {
        abort();
    }

    // +
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "+",
        octaspire_dern_vm_builtin_plus,
        1,
        "Add arguments to create new value",
        true,
        env))
    {
        abort();
    }

    // -
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "-",
        octaspire_dern_vm_builtin_minus,
        1,
        "Subtract number arguments, or negate one argument",
        true,
        env))
    {
        abort();
    }

    // find
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "find",
        octaspire_dern_vm_builtin_find,
        2,
        "Find value from collection",
        true,
        env))
    {
        abort();
    }

    // split
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "split",
        octaspire_dern_vm_builtin_split,
        2,
        "Split a collection by value",
        true,
        env))
    {
        abort();
    }


    // hash-map
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "hash-map",
        octaspire_dern_vm_builtin_hash_map,
        0,
        "Create new hash map",
        true,
        env))
     {
        abort();
     }

    // queue
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "queue",
        octaspire_dern_vm_builtin_queue,
        0,
        "Create new queue",
        true,
        env))
    {
        abort();
    }

    // queue-with-max-length
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "queue-with-max-length",
        octaspire_dern_vm_builtin_queue_with_max_length,
        0,
        "Create new queue with maximum length",
        true,
        env))
    {
        abort();
    }

    // list
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "list",
        octaspire_dern_vm_builtin_list,
        0,
        "Create new list",
        true,
        env))
    {
        abort();
    }

    // exit
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "exit",
        octaspire_dern_vm_builtin_exit,
        0,
        "Quit and exit the vm execution or REPL",
        false,
        env))
    {
        abort();
    }

    // doc
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "doc",
        octaspire_dern_vm_builtin_doc,
        1,
        "Get documentation string of a value or values",
        true,
        env))
    {
        abort();
    }

    // len
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "len",
        octaspire_dern_vm_builtin_len,
        1,
        "Get length of a value or values",
        true,
        env))
    {
        abort();
    }

    // read-and-eval-path
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "read-and-eval-path",
        octaspire_dern_vm_builtin_read_and_eval_path,
        1,
        "Read and evaluate a file from the given path",
        false,
        env))
    {
        abort();
    }

    // read-and-eval-string
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "read-and-eval-string",
        octaspire_dern_vm_builtin_read_and_eval_string,
        1,
        "Read and evaluate the given string",
        true,
        env))
    {
        abort();
    }

    // copy
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "copy",
        octaspire_dern_vm_builtin_copy,
        1,
        "Create full or partial copy of a value",
        true,
        env))
    {
        abort();
    }



    //////////////////////////////////////// Specials ////////////////////////////////////////////


    // define
    if (!octaspire_dern_vm_create_and_register_new_special(
        self,
        "define",
        octaspire_dern_vm_special_define,
        3,
        "Bind value to name and document the binding",
        false,
        env))
    {
        abort();
    }

    // eval
    if (!octaspire_dern_vm_create_and_register_new_special(
        self,
        "eval",
        octaspire_dern_vm_special_eval,
        1,
        "Evaluate a value (first argument) in global environment or, if given, in then "
        "given environment",
        true,
        env))
    {
        abort();
    }

    // quote
    if (!octaspire_dern_vm_create_and_register_new_special(
        self,
        "quote",
        octaspire_dern_vm_special_quote,
        1,
        "Quote a value",
        true,
        env))
    {
        abort();
    }

    // select
    if (!octaspire_dern_vm_create_and_register_new_special(
        self,
        "select",
        octaspire_dern_vm_special_select,
        1,
        "Select one of values, default, or no value and evaluate it according to boolean test",
        true,
        env))
    {
        abort();
    }

    // if
    if (!octaspire_dern_vm_create_and_register_new_special(
        self,
        "if",
        octaspire_dern_vm_special_if,
        1,
        "Select value or no value and evaluate it according to boolean test",
        true,
        env))
    {
        abort();
    }

    // while
    if (!octaspire_dern_vm_create_and_register_new_special(
        self,
        "while",
        octaspire_dern_vm_special_while,
        2,
        "Evaluate values repeatedly as long as predicate is true",
        true,
        env))
    {
        abort();
    }

    // for
    if (!octaspire_dern_vm_create_and_register_new_special(
        self,
        "for",
        octaspire_dern_vm_special_for,
        3, // or 5
        "Evaluate values repeatedly over a numeric range or container",
        true,
        env))
    {
        abort();
    }

    // ==
    if (!octaspire_dern_vm_create_and_register_new_special(
        self,
        "==",
        octaspire_dern_vm_special_equals_equals,
        1,
        "Predicate telling whether all the given values are equal. "
        "Takes 2..n arguments. Evaluates arguments only as long as those are equal. "
        "Does not evaluate rest of the arguments if one is not equal",
        true,
        env))
    {
        abort();
    }

    // ===
    if (!octaspire_dern_vm_create_and_register_new_special(
        self,
        "===",
        octaspire_dern_vm_special_equals_equals_equals,
        2,
        "Predicate telling whether all the given values are the same. "
        "Two values that are equal might not be the same value. "
        "Takes 2..n arguments. Evaluates arguments only as long as those are the same. "
        "Does not evaluate rest of the arguments if one is not the same",
        true,
        env))
    {
        abort();
    }

    // !=
    if (!octaspire_dern_vm_create_and_register_new_special( self,
        "!=",
        octaspire_dern_vm_special_exclamation_equals,
        1,
        "Predicate telling whether all the given values are not equal. "
        "Takes 2..n arguments. Evaluates arguments only as long as unequal is not found. "
        "Does not evaluate rest of the arguments if one is unequal",
        true,
        env))
    {
        abort();
    }

    // <
    if (!octaspire_dern_vm_create_and_register_new_special(
        self,
        "<",
        octaspire_dern_vm_special_less_than,
        1,
        "Predicate telling whether the arguments are less than the previous argument. "
        "Takes 2..n arguments. Evaluates arguments only as long as those are less than the "
        "previous. Does not evaluate rest of the arguments if one is larger or equal",
        true,
        env))
    {
        abort();
    }

    // >
    if (!octaspire_dern_vm_create_and_register_new_special(
        self,
        ">",
        octaspire_dern_vm_special_greater_than,
        1,
        "Predicate telling whether the arguments are greater than the previous argument. "
        "Takes 2..n arguments. Evaluates arguments only as long as those are greater than the "
        "previous. Does not evaluate rest of the arguments if one is smaller or equal",
        true,
        env))
    {
        abort();
    }

    // <=
    if (!octaspire_dern_vm_create_and_register_new_special(
        self,
        "<=",
        octaspire_dern_vm_special_less_than_or_equal,
        1,
        "Predicate telling whether the arguments are equal or less than the previous argument. "
        "Takes 2..n arguments. Evaluates arguments only as long as those are equal or less than "
        "the previous. Does not evaluate rest of the arguments if one is larger",
        true,
        env))
    {
        abort();
    }

    // >=
    if (!octaspire_dern_vm_create_and_register_new_special(
        self,
        ">=",
        octaspire_dern_vm_special_greater_than_or_equal,
        1,
        "Predicate telling whether the arguments are equal or greater than the previous argument. "
        "Takes 2..n arguments. Evaluates arguments only as long as those are equal or greater than "
        "the previous. Does not evaluate rest of the arguments if one is smaller",
        true,
        env))
    {
        abort();
    }

    // fn
    if (!octaspire_dern_vm_create_and_register_new_special(
        self,
        "fn",
        octaspire_dern_vm_special_fn,
        2,
        "Create new anonymous function",
        true,
        env))
    {
        abort();
    }

    // uid
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "uid",
        octaspire_dern_vm_builtin_uid,
        2,
        "Get unique id of a value",
        false,
        env))
    {
        abort();
    }

    // integer?
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "integer?",
        octaspire_dern_vm_builtin_integer_question_mark,
        1,
        "Predicate telling whether the argument is an integer",
        true,
        env))
    {
        abort();
    }

    // real?
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "real?",
        octaspire_dern_vm_builtin_real_question_mark,
        1,
        "Predicate telling whether the argument is a real",
        true,
        env))
    {
        abort();
    }

    // number?
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "number?",
        octaspire_dern_vm_builtin_number_question_mark,
        1,
        "Predicate telling whether the argument is a number (integer or real)",
        true,
        env))
    {
        abort();
    }

    // nil?
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "nil?",
        octaspire_dern_vm_builtin_nil_question_mark,
        1,
        "Predicate telling whether the argument is nil",
        true,
        env))
    {
        abort();
    }

    // boolean?
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "boolean?",
        octaspire_dern_vm_builtin_boolean_question_mark,
        1,
        "Predicate telling whether the argument is a boolean",
        true,
        env))
    {
        abort();
    }

    // character?
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "character?",
        octaspire_dern_vm_builtin_character_question_mark,
        1,
        "Predicate telling whether the argument is a character",
        true,
        env))
    {
        abort();
    }

    // string?
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "string?",
        octaspire_dern_vm_builtin_string_question_mark,
        1,
        "Predicate telling whether the argument is a string",
        true,
        env))
    {
        abort();
    }

    // symbol?
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "symbol?",
        octaspire_dern_vm_builtin_symbol_question_mark,
        1,
        "Predicate telling whether the argument is a symbol",
        true,
        env))
    {
        abort();
    }

    // vector?
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "vector?",
        octaspire_dern_vm_builtin_vector_question_mark,
        1,
        "Predicate telling whether the argument is a vector",
        true,
        env))
    {
        abort();
    }

    // hash-map?
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "hash-map?",
        octaspire_dern_vm_builtin_hash_map_question_mark,
        1,
        "Predicate telling whether the argument is a hash map",
        true,
        env))
    {
        abort();
    }

    // TODO move to specials
    // howto
    if (!octaspire_dern_vm_create_and_register_new_special(
        self,
        "howto",
        octaspire_dern_vm_special_howto,
        1,
        "Suggest functions by giving arguments and expected result",
        true,
        env))
    {
        abort();
    }

    return self;
}

void octaspire_dern_vm_release(octaspire_dern_vm_t *self)
{
    if (!self)
    {
        return;
    }

    octaspire_map_release(self->libraries);
    self->libraries = 0;

    octaspire_vector_release(self->commandLineArguments);
    self->commandLineArguments = 0;

    octaspire_vector_release(self->environmentVariables);
    self->environmentVariables = 0;

    // At this point stack had nil and self->globalEnvironment was tried to remove
    //octaspire_dern_vm_pop_value(self, self->globalEnvironment);

    octaspire_vector_clear(self->stack);
    octaspire_dern_vm_gc(self);

    octaspire_vector_release(self->stack);

    octaspire_vector_release(self->all);

    octaspire_allocator_free(self->allocator, self);
}

bool octaspire_dern_vm_push_value(octaspire_dern_vm_t *self, octaspire_dern_value_t *value)
{
    return octaspire_vector_push_back_element(self->stack, &value);
}

bool octaspire_dern_vm_pop_value(
    octaspire_dern_vm_t *self,
    octaspire_dern_value_t *valueForBalanceCheck)
{
    if (octaspire_vector_peek_back_element(self->stack) != valueForBalanceCheck)
    {
        printf(
            "\n\n ----- STACK UNBALANCED! -----\n (real top)%p != (user says is top)%p\n\n",
            (void*)octaspire_vector_peek_back_element(self->stack),
            (void*)valueForBalanceCheck);

        printf("REAL TOP IS:\n");

        octaspire_dern_value_print(
            octaspire_vector_peek_back_element(self->stack), self->allocator);

        printf("USER SAYS THIS IS TOP:\n");
        octaspire_dern_value_print(valueForBalanceCheck, self->allocator);
    }

    if (octaspire_vector_peek_back_element(self->stack) != valueForBalanceCheck)
    {
        abort();
    }

    return octaspire_vector_pop_back_element(self->stack);
}

void const * octaspire_dern_vm_get_top_value(octaspire_dern_vm_t const * const self)
{
    return octaspire_vector_peek_back_element(self->stack);
}

octaspire_dern_value_t *octaspire_dern_vm_peek_value(octaspire_dern_vm_t *self)
{
    return octaspire_vector_peek_back_element(self->stack);
}

octaspire_dern_value_t *octaspire_dern_vm_private_create_new_value_struct(
    octaspire_dern_vm_t* self,
    octaspire_dern_value_tag_t const typeTag)
{
    if (self->numAllocatedWithoutGc >= self->gcTriggerLimit && !self->preventGc)
    {
        octaspire_dern_vm_gc(self);
        self->numAllocatedWithoutGc = 0;
    }
    else
    {
        ++(self->numAllocatedWithoutGc);
    }

    octaspire_dern_value_t *result =
        octaspire_allocator_malloc(self->allocator, sizeof(octaspire_dern_value_t));

    if (!result)
    {
        octaspire_helpers_verify_not_null(false);
        return 0;
    }

    octaspire_vector_push_back_element(self->all, &result);

    result->typeTag      = typeTag;
    result->mark         = false;
    result->docstr       = 0;
    result->vm           = self;
    result->uniqueId     = self->nextFreeUniqueIdForValues;
    result->howtoAllowed = false;

    if (self->nextFreeUniqueIdForValues == UINTMAX_MAX)
    {
        abort();
    }

    ++(self->nextFreeUniqueIdForValues);

    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_copy(
    octaspire_dern_vm_t *self,
    octaspire_dern_value_t *valueToBeCopied)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(self);

    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(
        self,
        valueToBeCopied->typeTag);

    octaspire_dern_vm_push_value(self, result);

    switch (result->typeTag)
    {
        case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
        {
            abort();
        }

        case OCTASPIRE_DERN_VALUE_TAG_NIL:
        {
            // NOP
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
        {
            result->value.boolean = valueToBeCopied->value.boolean;
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
        {
            result->value.integer = valueToBeCopied->value.integer;
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_REAL:
        {
            result->value.real = valueToBeCopied->value.real;
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_STRING:
        {
            result->value.string = octaspire_string_new_copy(
                valueToBeCopied->value.string,
                self->allocator);
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
        {
            result->value.character = octaspire_string_new_copy(
                valueToBeCopied->value.character,
                self->allocator);
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_SYMBOL:
        {
            result->value.symbol = octaspire_string_new_copy(
                valueToBeCopied->value.symbol,
                self->allocator);
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_ERROR:
        {
            result->value.error = octaspire_dern_error_message_new_copy(
                valueToBeCopied->value.error,
                self->allocator);
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
        {
            result->value.vector = octaspire_vector_new(
                sizeof(octaspire_dern_value_t*),
                true,
                0,
                self->allocator);

            for (size_t i = 0;
                 i < octaspire_vector_get_length(valueToBeCopied->value.vector); ++i)
            {
                octaspire_dern_value_t * const tmpValToCopy =
                    octaspire_vector_get_element_at(
                        valueToBeCopied->value.vector,
                        (ptrdiff_t)i);

                assert(tmpValToCopy);

                octaspire_dern_value_t * const tmpValCopied =
                    octaspire_dern_vm_create_new_value_copy(self, tmpValToCopy);

                if (!octaspire_vector_push_back_element(
                    result->value.vector,
                    &tmpValCopied))
                {
                    abort();
                }
            }
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_QUEUE:
        {
            result->value.queue = octaspire_queue_new(
                sizeof(octaspire_dern_value_t*),
                true,
                0,
                self->allocator);

            for (size_t i = 0;
                 i < octaspire_queue_get_length(valueToBeCopied->value.queue);
                 ++i)
            {
                octaspire_dern_value_t * const tmpValToCopy =
                    octaspire_queue_get_at(
                        valueToBeCopied->value.queue,
                        (ptrdiff_t)i);

                assert(tmpValToCopy);

                octaspire_dern_value_t * const tmpValCopied =
                    octaspire_dern_vm_create_new_value_copy(self, tmpValToCopy);

                if (!octaspire_queue_push(
                    result->value.queue,
                    &tmpValCopied))
                {
                    abort();
                }
            }
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_LIST:
        {
            result->value.list = octaspire_list_new(
                sizeof(octaspire_dern_value_t*),
                true,
                0,
                self->allocator);

            // TODO more efficient iteration
            for (size_t i = 0;
                 i < octaspire_list_get_length(valueToBeCopied->value.list);
                 ++i)
            {
                octaspire_dern_value_t * const tmpValToCopy =
                    octaspire_list_node_get_element(
                        octaspire_list_get_at(
                            valueToBeCopied->value.list,
                            (ptrdiff_t)i));

                assert(tmpValToCopy);

                octaspire_dern_value_t * const tmpValCopied =
                    octaspire_dern_vm_create_new_value_copy(self, tmpValToCopy);

                if (!octaspire_list_push_back(
                    result->value.list,
                    &tmpValCopied))
                {
                    abort();
                }
            }
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_HASH_MAP:
        {
            result->value.hashMap = octaspire_map_new(
                sizeof(octaspire_dern_value_t*),
                true,
                sizeof(octaspire_dern_value_t*),
                true,
                (octaspire_map_key_compare_function_t)
                    octaspire_dern_value_is_equal,
                (octaspire_map_key_hash_function_t)
                    octaspire_dern_value_get_hash,
                0,
                0,
                self->allocator);

            octaspire_map_element_iterator_t iter =
                octaspire_map_element_iterator_init(
                    valueToBeCopied->value.hashMap);
            do
            {
                if (iter.element)
                {
                    octaspire_dern_value_t * const keyToCopy =
                        octaspire_map_element_get_key(iter.element);

                    octaspire_dern_value_t * const valToCopy =
                        octaspire_map_element_get_value(iter.element);

                    octaspire_helpers_verify_not_null(keyToCopy);
                    octaspire_helpers_verify_not_null(valToCopy);



                    octaspire_dern_value_t * const copyOfKeyVal =
                        octaspire_dern_vm_create_new_value_copy(self, keyToCopy);

                    octaspire_helpers_verify_not_null(copyOfKeyVal);

                    octaspire_dern_vm_push_value(self, copyOfKeyVal);



                    octaspire_dern_value_t * const copyOfValVal =
                        octaspire_dern_vm_create_new_value_copy(self, valToCopy);

                    octaspire_helpers_verify_not_null(copyOfValVal);

                    octaspire_dern_vm_push_value(self, copyOfValVal);



                    if (!octaspire_map_put(
                            result->value.hashMap,
                            octaspire_dern_value_get_hash(copyOfKeyVal),
                            &copyOfKeyVal,
                            &copyOfValVal))
                    {
                        abort();
                    }

                    octaspire_dern_vm_pop_value(self, copyOfValVal);
                    octaspire_dern_vm_pop_value(self, copyOfKeyVal);
                }
            }
            while (octaspire_map_element_iterator_next(&iter));
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT:
        {
            result->value.environment = octaspire_dern_environment_new_copy(
                valueToBeCopied->value.environment,
                self,
                self->allocator);
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_FUNCTION:
        {
            result->value.function = octaspire_dern_function_new_copy(
                valueToBeCopied->value.function,
                self,
                self->allocator);
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_SPECIAL:
        {
            result->value.special = octaspire_dern_special_new_copy(
                valueToBeCopied->value.special,
                self->allocator);
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_BUILTIN:
        {
            result->value.builtin = octaspire_dern_builtin_new_copy(
                valueToBeCopied->value.builtin,
                self->allocator);
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_PORT:
        {
            result->value.port =
                octaspire_dern_port_new_copy(valueToBeCopied->value.port, self->allocator);
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_C_DATA:
        {
            if (!octaspire_dern_c_data_is_copying_allowed(valueToBeCopied->value.cData))
            {
                octaspire_dern_vm_pop_value(self, result);

                octaspire_string_t *str =
                    octaspire_dern_c_data_to_string(valueToBeCopied->value.cData, self->allocator);

                octaspire_helpers_verify_not_null(str);

                octaspire_dern_value_t * const errorVal =
                    octaspire_dern_vm_create_new_value_error_format(
                        self,
                        "C data '%s' cannot be copied.",
                        octaspire_string_get_c_string(str));

                octaspire_helpers_verify_not_null(errorVal);

                octaspire_string_release(str);
                str = 0;

                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(self));

                return errorVal;
            }

            result->value.cData =
                octaspire_dern_c_data_new_copy(valueToBeCopied->value.cData, self->allocator);
        }
        break;
    }

    if (valueToBeCopied->docstr)
    {
        result->docstr = octaspire_dern_vm_create_new_value_copy(self, valueToBeCopied->docstr);
    }

    if (valueToBeCopied->docvec)
    {
        result->docvec = octaspire_dern_vm_create_new_value_copy(self, valueToBeCopied->docvec);
    }

    octaspire_dern_vm_pop_value(self, result);
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_input_file(
    octaspire_dern_vm_t *self,
    char const * const path)
{
    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(
        self,
        OCTASPIRE_DERN_VALUE_TAG_PORT);

    result->value.port = octaspire_dern_port_new_input_file(path, self->allocator);
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_output_file(
    octaspire_dern_vm_t *self,
    char const * const path)
{
    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(
        self,
        OCTASPIRE_DERN_VALUE_TAG_PORT);

    result->value.port = octaspire_dern_port_new_output_file(path, self->allocator);
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_io_file(
    octaspire_dern_vm_t *self,
    char const * const path)
{
    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(
        self,
        OCTASPIRE_DERN_VALUE_TAG_PORT);

    result->value.port = octaspire_dern_port_new_io_file(path, self->allocator);
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_nil(octaspire_dern_vm_t *self)
{
    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(
        self,
        OCTASPIRE_DERN_VALUE_TAG_NIL);

    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_boolean(
    octaspire_dern_vm_t *self,
    bool const value)
{
    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(
        self,
        OCTASPIRE_DERN_VALUE_TAG_BOOLEAN);

    result->value.boolean = value;
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_integer(
    octaspire_dern_vm_t *self,
    int32_t const value)
{
    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(
        self,
        OCTASPIRE_DERN_VALUE_TAG_INTEGER);

    result->value.integer = value;
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_real(
    octaspire_dern_vm_t *self,
    double const value)
{
    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(
        self,
        OCTASPIRE_DERN_VALUE_TAG_REAL);

    result->value.real = value;
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_string(
    octaspire_dern_vm_t *self,
    octaspire_string_t * const value)
{
    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(
        self,
        OCTASPIRE_DERN_VALUE_TAG_STRING);

    result->value.string = value;
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_string_format(
    octaspire_dern_vm_t *self,
    char const * const fmt,
    ...)
{
    va_list arguments;
    va_start(arguments, fmt);

    octaspire_string_t *str = octaspire_string_new_vformat(
        self->allocator,
        fmt,
        arguments);

    va_end(arguments);

    octaspire_dern_value_t *result = octaspire_dern_vm_create_new_value_string(self, str);

    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_string_from_c_string(
    octaspire_dern_vm_t *self,
    char const * const value)
{
    return octaspire_dern_vm_create_new_value_string(
        self,
        octaspire_string_new(value, self->allocator));
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_character(
    octaspire_dern_vm_t *self,
    octaspire_string_t * const value)
{
    octaspire_dern_value_t *result =
        octaspire_dern_vm_private_create_new_value_struct(self, OCTASPIRE_DERN_VALUE_TAG_CHARACTER);

    result->value.character = value;
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_character_from_uint32t(
    octaspire_dern_vm_t *self,
    uint32_t const value)
{
    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(
        self,
        OCTASPIRE_DERN_VALUE_TAG_CHARACTER);

    result->value.character = octaspire_string_new("", self->allocator);
    octaspire_string_push_back_ucs_character(result->value.character, value);
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_symbol(
    octaspire_dern_vm_t *self,
    octaspire_string_t * const value)
{
    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(
        self,
        OCTASPIRE_DERN_VALUE_TAG_SYMBOL);

    result->value.symbol = value;
    return result;
}

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_symbol_from_c_string(
    octaspire_dern_vm_t *self,
    char const * const value)
{
    return octaspire_dern_vm_create_new_value_symbol(
        self,
        octaspire_string_new(value, self->allocator));
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_error(
    octaspire_dern_vm_t *self,
    octaspire_string_t * value)
{
    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(
        self,
        OCTASPIRE_DERN_VALUE_TAG_ERROR);

    octaspire_dern_error_message_t * const message =
        octaspire_dern_error_message_new(
            octaspire_dern_vm_get_allocator(self),
            octaspire_string_get_c_string(value),
            0);

    result->value.error = message;

    octaspire_string_release(value);
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_error_format(
    octaspire_dern_vm_t *self,
    char const * const fmt,
    ...)
{
    va_list arguments;
    va_start(arguments, fmt);

    octaspire_string_t *str = octaspire_string_new_vformat(
        self->allocator,
        fmt,
        arguments);

    va_end(arguments);

    octaspire_dern_value_t *result = octaspire_dern_vm_create_new_value_error(self, str);

    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_error_from_c_string(
    octaspire_dern_vm_t *self,
    char const * const value)
{
    return octaspire_dern_vm_create_new_value_error(
        self,
        octaspire_string_new(value, self->allocator));
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_vector_from_vector(
    octaspire_dern_vm_t *self,
    octaspire_vector_t * const value)
{
    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(
        self,
        OCTASPIRE_DERN_VALUE_TAG_VECTOR);

    result->value.vector = value;
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_vector_from_values(
    octaspire_dern_vm_t *self,
    size_t const numArgs,
    ...)
{
    // TODO preallocate numArgs elements?
    octaspire_dern_value_t *result = octaspire_dern_vm_create_new_value_vector(self);

    va_list arguments;
    va_start(arguments, numArgs);

    for (size_t i = 0; i < numArgs; ++i)
    {
        octaspire_dern_value_t *value = va_arg(arguments, octaspire_dern_value_t*);
        octaspire_dern_value_as_vector_push_back_element(result, &value);
    }

    va_end(arguments);

    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_vector(octaspire_dern_vm_t *self)
{
    octaspire_vector_t *vec = octaspire_vector_new(
        sizeof(octaspire_dern_value_t*),
        true,
        0,
        self->allocator);

    return octaspire_dern_vm_create_new_value_vector_from_vector(self, vec);
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_hash_map_from_hash_map(
    octaspire_dern_vm_t *self,
    octaspire_map_t * const value)
{
    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(
        self,
        OCTASPIRE_DERN_VALUE_TAG_HASH_MAP);

    result->value.hashMap = value;
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_private_create_new_value_queue_from_queue(
    octaspire_dern_vm_t *self,
    octaspire_queue_t * const queue)
{
    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(
        self,
        OCTASPIRE_DERN_VALUE_TAG_QUEUE);

    result->value.queue = queue;
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_private_create_new_value_list_from_list(
    octaspire_dern_vm_t *self,
    octaspire_list_t * const list)
{
    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(
        self,
        OCTASPIRE_DERN_VALUE_TAG_LIST);

    result->value.list = list;
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_hash_map(octaspire_dern_vm_t *self)
{
    octaspire_map_t *hashMap = octaspire_map_new(
        sizeof(octaspire_dern_value_t*),
        true,
        sizeof(octaspire_dern_value_t*),
        true,
        (octaspire_map_key_compare_function_t)octaspire_dern_value_is_equal,
        (octaspire_map_key_hash_function_t)octaspire_dern_value_get_hash,
        0,
        0,
        self->allocator);

    return octaspire_dern_vm_create_new_value_hash_map_from_hash_map(self, hashMap);
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_queue(octaspire_dern_vm_t *self)
{
    octaspire_queue_t *queue = octaspire_queue_new(
        sizeof(octaspire_dern_value_t*),
        true,
        0,
        self->allocator);

    return octaspire_dern_vm_private_create_new_value_queue_from_queue(self, queue);
}

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_queue_with_max_length (
        octaspire_dern_vm_t * const self,
        size_t const maxLength)
{
    octaspire_queue_t * const queue = octaspire_queue_new_with_max_length(
        maxLength,
        sizeof(octaspire_dern_value_t*),
        true,
        0,
        self->allocator);

    return octaspire_dern_vm_private_create_new_value_queue_from_queue(self, queue);
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_list(octaspire_dern_vm_t *self)
{
    octaspire_list_t *list = octaspire_list_new(
        sizeof(octaspire_dern_value_t*),
        true,
        0,
        self->allocator);

    return octaspire_dern_vm_private_create_new_value_list_from_list(self, list);
}

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_environment(
    octaspire_dern_vm_t *self,
    octaspire_dern_value_t *enclosing)
{
    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(
        self,
        OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    result->value.environment =
        octaspire_dern_environment_new(enclosing, self, self->allocator);

    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_environment_from_environment(
    octaspire_dern_vm_t *self,
    octaspire_dern_environment_t * const value)
{
    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(
        self,
        OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    result->value.environment = value;
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_function(
    octaspire_dern_vm_t *self,
    octaspire_dern_function_t * const value,
    char const * const docstr,
    octaspire_vector_t *docVec)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(self);

    octaspire_dern_value_t *result =
        octaspire_dern_vm_private_create_new_value_struct(self, OCTASPIRE_DERN_VALUE_TAG_FUNCTION);

    octaspire_dern_vm_push_value(self, result);

    result->value.function = value;

    result->docstr = octaspire_dern_vm_create_new_value_string_from_c_string(self, docstr);

    result->docvec =
        docVec ? octaspire_dern_vm_create_new_value_vector_from_vector(self, docVec) : 0;

    octaspire_dern_vm_pop_value(self, result);
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_special(
    octaspire_dern_vm_t *self,
    octaspire_dern_special_t * const value,
    char const * const docstr)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(self);

    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(
        self,
        OCTASPIRE_DERN_VALUE_TAG_SPECIAL);

    result->value.special = value;
    result->howtoAllowed = octaspire_dern_special_is_howto_allowed(value);

    octaspire_dern_vm_push_value(self, result);

    result->docstr = octaspire_dern_vm_create_new_value_string_from_c_string(self, docstr);

    octaspire_dern_vm_pop_value(self, result);
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_builtin(
    octaspire_dern_vm_t *self,
    octaspire_dern_builtin_t * const value,
    char const * const docstr)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(self);

    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(
        self,
        OCTASPIRE_DERN_VALUE_TAG_BUILTIN);

    result->value.builtin = value;
    result->howtoAllowed = octaspire_dern_builtin_is_howto_allowed(value);

    octaspire_dern_vm_push_value(self, result);

    result->docstr = octaspire_dern_vm_create_new_value_string_from_c_string(self, docstr);

    octaspire_dern_vm_pop_value(self, result);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_c_data(
    octaspire_dern_vm_t * const self,
    char const * const pluginName,
    char const * const typeNameForPayload,
    char const * const cleanUpCallbackName,
    char const * const stdLibLenCallbackName,
    char const * const stdLibLinkAtCallbackName,
    char const * const stdLibCopyAtCallbackName,
    bool const copyingAllowed,
    void * const payload)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(self);

    octaspire_dern_value_t * const result =
        octaspire_dern_vm_private_create_new_value_struct(self, OCTASPIRE_DERN_VALUE_TAG_C_DATA);

    result->value.cData = octaspire_dern_c_data_new(
        pluginName,
        typeNameForPayload,
        payload,
        cleanUpCallbackName,
        stdLibLenCallbackName,
        stdLibLinkAtCallbackName,
        stdLibCopyAtCallbackName,
        copyingAllowed,
        self->allocator);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));
    return result;
}

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_c_data_from_existing(
    octaspire_dern_vm_t * const self,
    octaspire_dern_c_data_t * const cData)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(self);

    octaspire_dern_value_t * const result =
        octaspire_dern_vm_private_create_new_value_struct(
            self,
            OCTASPIRE_DERN_VALUE_TAG_C_DATA);

    result->value.cData = cData;

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(self));

    return result;
}

void octaspire_dern_vm_clear_value_to_nil(
    octaspire_dern_vm_t *self,
    octaspire_dern_value_t *value)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(self);

    if (!value)
    {
        return;
    }

    switch (value->typeTag)
    {
        case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
        {
            abort();
        }

        case OCTASPIRE_DERN_VALUE_TAG_NIL:         break;
        case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:     break;
        case OCTASPIRE_DERN_VALUE_TAG_INTEGER:     break;
        case OCTASPIRE_DERN_VALUE_TAG_REAL:        break;

        case OCTASPIRE_DERN_VALUE_TAG_STRING:
        {
            octaspire_string_release(value->value.string);
            value->value.string = 0;
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
        {
            octaspire_string_release(value->value.character);
            value->value.character = 0;
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_SYMBOL:
        {
            octaspire_string_release(value->value.symbol);
            value->value.symbol = 0;
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_ERROR:
        {
            octaspire_dern_error_message_release(value->value.error);
            value->value.error = 0;
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
        {
            // Elements are NOT released here, because it would lead to double free.
            // GC releases the elements (those are stored in the all-vector also).
            octaspire_vector_clear(value->value.vector);
            octaspire_vector_release(value->value.vector);
            value->value.vector = 0;
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_HASH_MAP:
        {
            // Elements are NOT released here, because it would lead to double free.
            // GC releases the elements (those are stored in the all-vector also).
            octaspire_map_clear(value->value.hashMap);
            octaspire_map_release(value->value.hashMap);
            value->value.hashMap = 0;
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_QUEUE:
        {
            // Elements are NOT released here, because it would lead to double free.
            // GC releases the elements (those are stored in the all-vector also).
            octaspire_queue_clear(value->value.queue);
            octaspire_queue_release(value->value.queue);
            value->value.queue = 0;
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_LIST:
        {
            // Elements are NOT released here, because it would lead to double free.
            // GC releases the elements (those are stored in the all-vector also).
            octaspire_list_clear(value->value.list);
            octaspire_list_release(value->value.list);
            value->value.list = 0;
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT:
        {
            octaspire_dern_environment_release(value->value.environment);
            value->value.environment = 0;
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_FUNCTION:
        {
            octaspire_dern_function_release(value->value.function);
            value->value.function = 0;

            //octaspire_dern_vm_private_release_value(self, fun->formals);
            //octaspire_dern_vm_private_release_value(self, fun->body);
            //octaspire_dern_vm_private_release_value(self, fun->definitionEnvironment);
            //fun->definitionEnvironment = 0;
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_SPECIAL:
        {
            octaspire_dern_special_release(value->value.special);
            value->value.special = 0;
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_BUILTIN:
        {
            octaspire_dern_builtin_release(value->value.builtin);
            value->value.builtin = 0;
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_PORT:
        {
            octaspire_dern_port_release(value->value.port);
            value->value.port = 0;
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_C_DATA:
        {
            octaspire_dern_c_data_release(value->value.cData);
            value->value.cData = 0;
        }
        break;
    }

    value->typeTag = OCTASPIRE_DERN_VALUE_TAG_NIL;
}

static void octaspire_dern_vm_private_release_value(
    octaspire_dern_vm_t *self,
    octaspire_dern_value_t *value)
{
    if (!value)
    {
        return;
    }

    octaspire_dern_vm_clear_value_to_nil(self, value);
    value->typeTag = OCTASPIRE_DERN_VALUE_TAG_ILLEGAL;

    octaspire_allocator_free(self->allocator, value);
}

bool octaspire_dern_vm_gc(octaspire_dern_vm_t *self)
{
    if (!octaspire_dern_vm_private_mark_all(self))
    {
        return false;
    }

    return octaspire_dern_vm_private_sweep(self);
}

bool octaspire_dern_vm_private_mark_all(octaspire_dern_vm_t *self)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(self);

    for (size_t i = 0; i < octaspire_vector_get_length(self->stack); ++i)
    {
        octaspire_dern_value_t * const value =
            octaspire_vector_get_element_at(
                self->stack,
                (ptrdiff_t)i);

        if (!octaspire_dern_vm_private_mark(self, value))
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(self));

            octaspire_helpers_verify_not_null(false);
            return false;
        }
    }

    if (self->libraries)
    {
        octaspire_map_element_iterator_t iterator =
            octaspire_map_element_iterator_init(self->libraries);

        while (iterator.element)
        {
            octaspire_dern_lib_mark_all(
                ((octaspire_dern_lib_t*)octaspire_map_element_get_value(
                    iterator.element)));

            octaspire_map_element_iterator_next(&iterator);
        }
    }

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(self));

    return true;
}

bool octaspire_dern_vm_private_mark(octaspire_dern_vm_t *self, octaspire_dern_value_t *value)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(self);
    return octaspire_dern_value_mark(value);
}

bool octaspire_dern_vm_private_sweep(octaspire_dern_vm_t *self)
{
    for (size_t i = 0; i < octaspire_vector_get_length(self->all); /* NOP */ )
    {
        octaspire_dern_value_t * const value =
            octaspire_vector_get_element_at(
                self->all,
                (ptrdiff_t)i);

        if (value->mark)
        {
            value->mark = false;

            ++i;
        }
        else
        {
            octaspire_dern_vm_private_release_value(self, value);

            octaspire_dern_value_t *value2 =
                octaspire_vector_get_element_at(
                    self->all,
                    (ptrdiff_t)i);

            octaspire_helpers_verify_true(value == value2);

            if (!octaspire_vector_remove_element_at(
                    self->all,
                    (ptrdiff_t)i))
            {
                abort();
            }
        }
    }

    return true;
}

octaspire_dern_value_t *octaspire_dern_vm_parse_token(
    octaspire_dern_vm_t * const self,
    octaspire_dern_lexer_token_t const * const token,
    octaspire_input_t *input)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(self);

    octaspire_dern_value_t *result = 0;

    if (!token)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));
        return result;
    }

    switch (octaspire_dern_lexer_token_get_type_tag(token))
    {
        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_LPAREN:
        {
            octaspire_vector_t *vec = octaspire_vector_new(
                sizeof(octaspire_dern_value_t*),
                true,
                0,
                self->allocator);

            if (!vec)
            {
                result = octaspire_dern_vm_create_new_value_error(
                    self,
                    octaspire_string_new(
                        "Allocation failure in private parse_token", self->allocator));
            }
            else
            {
                result = octaspire_dern_vm_create_new_value_vector_from_vector(self, vec);

                // Protect result (and all values inside it) from the garbage collector during
                // this phase.
                octaspire_dern_vm_push_value(self, result);

                //bool error = false;
                octaspire_dern_lexer_token_t *token2 = 0;
                while (true)
                {
                    octaspire_dern_lexer_token_release(token2);
                    token2 = 0;
                    octaspire_helpers_verify_true(token2 == 0);

                    token2 = octaspire_dern_lexer_pop_next_token(input, self->allocator);

                    if (!token2)
                    {
                        // No more input
                        octaspire_dern_vm_pop_value(self, result);

                        octaspire_helpers_verify_true(
                            stackLength == octaspire_dern_vm_get_stack_length(self));

                        return 0;
                    }
                    else if (
                        octaspire_dern_lexer_token_get_type_tag(token2) ==
                        OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR)
                    {
                        //error = true;
                        octaspire_dern_vm_pop_value(self, result);

                        result = octaspire_dern_vm_create_new_value_error(
                            self,
                            octaspire_string_new(
                                octaspire_dern_lexer_token_get_error_value_as_c_string(
                                    token2),
                                self->allocator));

                        octaspire_helpers_verify_not_null(result);

                        octaspire_dern_lexer_token_release(token2);
                        token2 = 0;

                        octaspire_helpers_verify_true(
                            stackLength == octaspire_dern_vm_get_stack_length(self));

                        return result;
                    }
                    else
                    {
                        if (octaspire_dern_lexer_token_get_type_tag(token2) ==
                            OCTASPIRE_DERN_LEXER_TOKEN_TAG_RPAREN)
                        {
                            octaspire_dern_vm_pop_value(self, result);

                            octaspire_dern_lexer_token_release(token2);
                            token2 = 0;

                            octaspire_helpers_verify_true(
                                stackLength == octaspire_dern_vm_get_stack_length(self));

                            return result;
                        }
                        else
                        {
                            octaspire_dern_value_t *element =
                                octaspire_dern_vm_parse_token(self, token2, input);

                            //octaspire_helpers_verify_not_null(element);

                            octaspire_dern_lexer_token_release(token2);
                            token2 = 0;

                            if (!element)
                            {
                                return element;
                            }

                            if (element->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                            {
                                return element;
                            }

                            // TODO report allocation error instead of asserting
                            if (!octaspire_vector_push_back_element(
                                result->value.vector,
                                &element))
                            {
                                abort();
                            }
                        }
                    }
                }

                /*
                Clang says this code is never executed
                if (!error)
                {
                    octaspire_dern_vm_pop_value(self, result);
                }

                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(self));

                return result;
                */
            }
        }
        break;

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_QUOTE:
        {
            result = octaspire_dern_vm_create_new_value_vector(self);

            if (!result)
            {
                result = octaspire_dern_vm_create_new_value_error_from_c_string(
                    self,
                    "Allocation failure");
            }
            else
            {
                octaspire_dern_vm_push_value(self, result);

                octaspire_dern_value_t *quoteSym =
                    octaspire_dern_vm_create_new_value_symbol_from_c_string(self, "quote");

                octaspire_helpers_verify_not_null(quoteSym);

                if (!octaspire_dern_value_as_vector_push_back_element(result, &quoteSym))
                {
                    abort();
                }

                octaspire_dern_value_t *quotedValue = octaspire_dern_vm_parse(
                    self,
                    input);

                if (!quotedValue || quotedValue->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                {
                    octaspire_dern_vm_pop_value(self, result);
                    result = quotedValue; // report error to caller
                }
                else
                {
                    if (!octaspire_dern_value_as_vector_push_back_element(
                        result,
                        &quotedValue))
                    {
                        abort();
                    }

                    octaspire_dern_vm_pop_value(self, result);
                }
            }
        }
        break;

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_TRUE:
        {
            result = octaspire_dern_vm_get_value_true(self);
        }
        break;

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_FALSE:
        {
            result = octaspire_dern_vm_get_value_false(self);
        }
        break;

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_NIL:
        {
            result = octaspire_dern_vm_get_value_nil(self);
        }
        break;

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_INTEGER:
        {
            result = octaspire_dern_vm_create_new_value_integer(
                self,
                octaspire_dern_lexer_token_get_integer_value(token));
        }
        break;

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_REAL:
        {
            result = octaspire_dern_vm_create_new_value_real(
                self,
                octaspire_dern_lexer_token_get_real_value(token));
        }
        break;

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_STRING:
        {
            result = octaspire_dern_vm_create_new_value_string(
                self,
                octaspire_string_new(
                    octaspire_dern_lexer_token_get_string_value_as_c_string(token),
                    self->allocator));
        }
        break;

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_CHARACTER:
        {
            result = octaspire_dern_vm_create_new_value_character(
                self,
                octaspire_string_new(
                    octaspire_dern_lexer_token_get_character_value_as_c_string(token),
                    self->allocator));
        }
        break;

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_SYMBOL:
        {
            result = octaspire_dern_vm_create_new_value_symbol(
                self,
                octaspire_string_new(
                    octaspire_dern_lexer_token_get_symbol_value_as_c_string(token),
                    self->allocator));
        }
        break;

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR:
        {
            result = octaspire_dern_vm_create_new_value_error(
                self,
                octaspire_string_new(
                    octaspire_dern_lexer_token_get_error_value_as_c_string(token),
                    self->allocator));
        }
        break;

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_MORE_INPUT_REQUIRED:
        {
            return 0;
        }

        default:
        {
            octaspire_string_t *str =
                octaspire_dern_lexer_token_to_string(token);

            octaspire_helpers_verify_not_null(str);

            result = octaspire_dern_vm_create_new_value_error_format(
                self,
                "unexpected %s",
                octaspire_string_get_c_string(str));

            octaspire_string_release(str);
            str = 0;
        }
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_parse(
    octaspire_dern_vm_t *self,
    octaspire_input_t *input)
{
    octaspire_dern_lexer_token_t *token =
        octaspire_dern_lexer_pop_next_token(input, self->allocator);

    octaspire_dern_value_t *result =
        octaspire_dern_vm_parse_token(self, token, input);

    octaspire_dern_lexer_token_release(token);
    token = 0;

    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_eval_in_global_environment(
    octaspire_dern_vm_t *self,
    octaspire_dern_value_t *value)
{
    return octaspire_dern_vm_eval(self, value, self->globalEnvironment);
}

octaspire_dern_value_t *octaspire_dern_vm_eval(
    octaspire_dern_vm_t *self,
    octaspire_dern_value_t *value,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(self);

    if (octaspire_dern_vm_is_quit(self))
    {
        return octaspire_dern_vm_create_new_value_nil(self);
    }

    if (self->config.debugModeOn)
    {
        octaspire_string_t *str =
            octaspire_dern_value_to_string(
                value,
                octaspire_dern_vm_get_allocator(self));

        fprintf(stderr,
            "[:::DEBUG:::] %s\n",
            octaspire_string_get_c_string(str));

        octaspire_string_release(str);
        str = 0;
    }

    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    if (!value)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));
        //return self->valueNil;
        return 0;
    }

    octaspire_dern_vm_push_value(self, value);
    octaspire_dern_vm_push_value(self, environment);

    octaspire_dern_value_t *result = 0;

    switch (value->typeTag)
    {
        case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
        {
            abort();
        }

        case OCTASPIRE_DERN_VALUE_TAG_NIL:
        case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
        case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
        case OCTASPIRE_DERN_VALUE_TAG_REAL:
        case OCTASPIRE_DERN_VALUE_TAG_STRING:
        case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
        case OCTASPIRE_DERN_VALUE_TAG_ERROR:
        // How about these? Self evaluating or not?
        case OCTASPIRE_DERN_VALUE_TAG_BUILTIN:
        case OCTASPIRE_DERN_VALUE_TAG_SPECIAL:
        case OCTASPIRE_DERN_VALUE_TAG_FUNCTION:
        {
            result = value;
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_SYMBOL:
        {
            result = octaspire_dern_environment_get(
                environment->value.environment,
                value);

            if (!result)
            {
                octaspire_string_t* str = octaspire_dern_value_to_string(
                    value,
                    self->allocator);

                result = octaspire_dern_vm_create_new_value_error(
                        self,
                        octaspire_string_new_format(
                            self->allocator,
                            "Unbound symbol '%s'",
                            octaspire_string_get_c_string(str)));

                octaspire_string_release(str);
                str = 0;
            }
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
        {
            octaspire_vector_t *vec = value->value.vector;

            if (octaspire_vector_is_empty(vec))
            {
                octaspire_dern_vm_pop_value(self, environment);
                octaspire_dern_vm_pop_value(self, value);

                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(self));

                return octaspire_dern_vm_create_new_value_error(
                    self,
                    octaspire_string_new(
                        "Cannot evaluate empty vector '()'",
                        self->allocator));
            }

            octaspire_dern_value_t *removeMe = octaspire_vector_get_element_at(vec, 0);

            octaspire_dern_value_t *operator = octaspire_dern_vm_eval(
                self,
                removeMe,
                environment);

            if (!operator)
            {
                octaspire_dern_vm_pop_value(self, environment);
                octaspire_dern_vm_pop_value(self, value);

                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(self));

                return 0;
            }

            octaspire_dern_vm_push_value(self, operator);

            switch (operator->typeTag)
            {
                case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
                {
                    abort();
                }

                case OCTASPIRE_DERN_VALUE_TAG_SPECIAL:
                {
                    octaspire_vector_t *argVec =
                        octaspire_vector_new_with_preallocated_elements(
                            sizeof(octaspire_dern_value_t*),
                            true,
                            octaspire_vector_get_length(vec) - 1,
                            0,
                            self->allocator);

                    octaspire_dern_value_t *arguments =
                        octaspire_dern_vm_create_new_value_vector_from_vector(
                            self,
                            argVec);

                    octaspire_dern_vm_push_value(self, arguments);

                    for (size_t i = 1;
                         i < octaspire_vector_get_length(vec);
                         ++i)
                    {
                        octaspire_dern_value_t * const tmpPtr =
                            octaspire_vector_get_element_at(
                                vec,
                                (ptrdiff_t)i);

                        if (tmpPtr->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                        {
                            result = tmpPtr;
                            break;
                        }

                        octaspire_vector_push_back_element(argVec, &tmpPtr);
                    }

                    if (!result)
                    {
                        result = (operator->value.special->cFunction)(
                            self,
                            arguments,
                            environment);

                        // TODO XXX add this error annotation to other places too
                        // (for example builtin and function calls)
                        if (result->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                        {
                            octaspire_string_t *tmpStr =
                                octaspire_dern_value_to_string(value, self->allocator);

                            octaspire_string_concatenate_format(
                                result->value.error->message,
                                "\n\tAt form: >>>>>>>>>>%s<<<<<<<<<<\n",
                                octaspire_string_get_c_string(tmpStr));

                            octaspire_string_release(tmpStr);
                            tmpStr = 0;
                        }
                    }

                    octaspire_dern_vm_pop_value(self, arguments);
                }
                break;

                case OCTASPIRE_DERN_VALUE_TAG_BUILTIN:
                {
                    octaspire_vector_t *argVec =
                        octaspire_vector_new_with_preallocated_elements(
                            sizeof(octaspire_dern_value_t*),
                            true,
                            octaspire_vector_get_length(vec) - 1,
                            0,
                            self->allocator);

                    octaspire_dern_value_t *arguments =
                        octaspire_dern_vm_create_new_value_vector_from_vector(
                            self,
                            argVec);

                    octaspire_dern_vm_push_value(self, arguments);

                    for (size_t i = 1;
                         i < octaspire_vector_get_length(vec);
                         ++i)
                    {
                        octaspire_dern_value_t *evaluated = octaspire_dern_vm_eval(
                            self,
                            octaspire_vector_get_element_at(
                                vec,
                                (ptrdiff_t)i),
                            environment);

                        if (evaluated->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                        {
                            result = evaluated;


                            // TODO XXX add this error annotation to other places too
                            // (for example builtin and function calls)
                            octaspire_string_t *tmpStr =
                                octaspire_dern_value_to_string(value, self->allocator);

                            octaspire_string_concatenate_format(
                                result->value.error->message,
                                "\n\tAt form: >>>>>>>>>>%s<<<<<<<<<<\n",
                                octaspire_string_get_c_string(tmpStr));

                            octaspire_string_release(tmpStr);
                            tmpStr = 0;


                            break;
                        }

                        octaspire_vector_push_back_element(argVec, &evaluated);
                    }

                    if (!result)
                    {
                        result = (operator->value.builtin->cFunction)(
                            self,
                            arguments,
                            environment);

                        // TODO XXX add this error annotation to other places too
                        // (for example builtin and function calls)
                        if (result->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                        {
                            octaspire_string_t *tmpStr =
                                octaspire_dern_value_to_string(value, self->allocator);

                            octaspire_string_concatenate_format(
                                result->value.error->message,
                                "\n\tAt form: >>>>>>>>>>%s<<<<<<<<<<\n",
                                octaspire_string_get_c_string(tmpStr));

                            octaspire_string_release(tmpStr);
                            tmpStr = 0;
                        }


                        if (operator->value.builtin->cFunction == octaspire_dern_vm_builtin_return)
                        {
                            //octaspire_helpers_verify_true(self->functionReturn == 0);
                            self->functionReturn = result;
                        }
                    }

                    octaspire_dern_vm_pop_value(self, arguments);
                }
                break;

                case OCTASPIRE_DERN_VALUE_TAG_FUNCTION:
                {
                    octaspire_vector_t *argVec =
                        octaspire_vector_new_with_preallocated_elements(
                            sizeof(octaspire_dern_value_t*),
                            true,
                            octaspire_vector_get_length(vec) - 1,
                            0,
                            self->allocator);

                    octaspire_dern_value_t *arguments =
                        octaspire_dern_vm_create_new_value_vector_from_vector(self, argVec);

                    octaspire_dern_vm_push_value(self, arguments);

                    for (size_t i = 1;
                         i < octaspire_vector_get_length(vec);
                         ++i)
                    {
                        octaspire_dern_value_t *evaluated = octaspire_dern_vm_eval(
                            self,
                            octaspire_vector_get_element_at(
                                vec,
                                (ptrdiff_t)i),
                            environment);


                        if (evaluated->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                        {
                            result = evaluated;

                            // TODO XXX add this error annotation to other places too
                            // (for example builtin and function calls)
                            octaspire_string_t *tmpStr =
                                octaspire_dern_value_to_string(value, self->allocator);

                            octaspire_string_concatenate_format(
                                result->value.error->message,
                                "\n\tAt form: >>>>>>>>>>%s<<<<<<<<<<\n",
                                octaspire_string_get_c_string(tmpStr));

                            octaspire_string_release(tmpStr);
                            tmpStr = 0;


                            break;
                        }

                        octaspire_dern_value_as_vector_push_back_element(arguments, &evaluated);
                    }

                    if (!result)
                    {
                        octaspire_dern_function_t *function = operator->value.function;

                        octaspire_helpers_verify_not_null(function);
                        octaspire_helpers_verify_not_null(function->formals);
                        // Invalid read of size 4 below
                        octaspire_helpers_verify_not_null(function->formals->value.vector);
                        octaspire_helpers_verify_not_null(function->body);
                        octaspire_helpers_verify_not_null(function->body->value.vector);
                        octaspire_helpers_verify_not_null(function->definitionEnvironment);

                        octaspire_helpers_verify_not_null(
                            function->definitionEnvironment->value.environment);

                        octaspire_dern_environment_t *extendedEnvironment =
                            octaspire_dern_environment_new(
                                function->definitionEnvironment,
                                self,
                                self->allocator);

                        octaspire_helpers_verify_not_null(extendedEnvironment);

                        octaspire_dern_value_t *extendedEnvVal =
                            octaspire_dern_vm_create_new_value_environment_from_environment(
                                self,
                                extendedEnvironment);

                        octaspire_helpers_verify_not_null(extendedEnvVal);

                        octaspire_dern_vm_push_value(self, extendedEnvVal);

                        octaspire_dern_value_t *error = octaspire_dern_environment_extend(
                            extendedEnvironment,
                            function->formals,
                            arguments);

                        if (error)
                        {
                            result = error;
                        }
                        else
                        {
                            // TODO push function->body?

                            octaspire_helpers_verify_true(
                                function->body->typeTag ==
                                OCTASPIRE_DERN_VALUE_TAG_VECTOR);

                            octaspire_helpers_verify_not_null(
                                function->body->value.vector);

                            for (size_t i = 0;
                                 i < octaspire_vector_get_length(
                                     function->body->value.vector);
                                 ++i)
                            {
                                octaspire_dern_value_t *toBeEvaluated =
                                    octaspire_vector_get_element_at(
                                        function->body->value.vector,
                                        (ptrdiff_t)i);

                                octaspire_dern_vm_push_value(self, toBeEvaluated);

                                result = octaspire_dern_vm_eval(
                                    self,
                                    toBeEvaluated,
                                    extendedEnvVal);

                                if (result->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                                {
                                    // TODO XXX add this error annotation to other places too
                                    // (for example builtin and function calls)
                                    octaspire_string_t *tmpStr =
                                        octaspire_dern_value_to_string(value, self->allocator);

                                    octaspire_string_concatenate_format(
                                        result->value.error->message,
                                        "\n\tAt form: >>>>>>>>>>%s<<<<<<<<<<\n",
                                        octaspire_string_get_c_string(tmpStr));

                                    octaspire_string_release(tmpStr);
                                    tmpStr = 0;


                                    octaspire_dern_vm_pop_value(self, toBeEvaluated);
                                    break;
                                }

                                octaspire_dern_vm_pop_value(self, toBeEvaluated);

                                if (self->functionReturn)
                                {
                                    result = self->functionReturn;
                                    self->functionReturn = 0;
                                    break;
                                }
                            }
                        }

                        // TODO pop function->body?

                        octaspire_dern_vm_pop_value(self, extendedEnvVal);
                        octaspire_dern_vm_pop_value(self, arguments);
                    }
                    else
                    {
                        octaspire_dern_vm_pop_value(self, arguments);
                    }
                }
                break;

                case OCTASPIRE_DERN_VALUE_TAG_NIL:
                case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
                case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
                case OCTASPIRE_DERN_VALUE_TAG_REAL:
                case OCTASPIRE_DERN_VALUE_TAG_STRING:
                case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
                case OCTASPIRE_DERN_VALUE_TAG_SYMBOL:
                case OCTASPIRE_DERN_VALUE_TAG_ERROR:
                case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
                case OCTASPIRE_DERN_VALUE_TAG_HASH_MAP:
                case OCTASPIRE_DERN_VALUE_TAG_QUEUE:
                case OCTASPIRE_DERN_VALUE_TAG_LIST:
                case OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT:
                case OCTASPIRE_DERN_VALUE_TAG_PORT:
                case OCTASPIRE_DERN_VALUE_TAG_C_DATA:
                {
                    octaspire_string_t *str = octaspire_dern_value_to_string(
                        operator,
                        self->allocator);

                    result = octaspire_dern_vm_create_new_value_error(
                        self,
                        octaspire_string_new_format(
                            self->allocator,
                            "Cannot evaluate operator of type '%s' (%s)",
                            octaspire_dern_value_helper_get_type_as_c_string(operator->typeTag),
                            octaspire_string_get_c_string(str)));

                    octaspire_string_release(str);
                    str = 0;
                }
                break;
            }

            octaspire_dern_vm_pop_value(self, operator);
        }
        break;

        // TODO XXX add rest of types
        case OCTASPIRE_DERN_VALUE_TAG_HASH_MAP:
        case OCTASPIRE_DERN_VALUE_TAG_QUEUE:
        case OCTASPIRE_DERN_VALUE_TAG_LIST:
        case OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT:
        case OCTASPIRE_DERN_VALUE_TAG_PORT:
        case OCTASPIRE_DERN_VALUE_TAG_C_DATA:
        {
            result = octaspire_dern_vm_create_new_value_error(
                self,
                octaspire_string_new_format(
                    self->allocator,
                    "Cannot evaluate unknown type %i",
                    (int)value->typeTag));
        }
        break;
    }

    octaspire_dern_vm_pop_value(self, environment);
    octaspire_dern_vm_pop_value(self, value);


    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));

    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_read_from_octaspire_input_and_eval_in_global_environment(
    octaspire_dern_vm_t *self,
    octaspire_input_t * const input)
{
    if (!input || !octaspire_input_is_good(input))
    {
        return octaspire_dern_vm_create_new_value_error_from_c_string(self, "No input");
    }

    octaspire_dern_value_t *lastGoodResult = 0;
    octaspire_dern_value_t *result = 0;

    while (octaspire_input_is_good(input))
    {
        result = octaspire_dern_vm_eval_in_global_environment(
            self,
            octaspire_dern_vm_parse(self, input));

        if (!result)
        {
            break;
        }

        lastGoodResult = result;

        if (result->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
        {
            octaspire_dern_value_as_error_set_line_number(
                result,
                octaspire_input_get_line_number(input));

            break;
        }
    }

    //octaspire_input_release(input);
    //input = 0;

    if (!result && lastGoodResult)
    {
        return lastGoodResult;
    }

    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
    octaspire_dern_vm_t *self,
    char const * const str)
{
    if (!str)
    {
        return octaspire_dern_vm_create_new_value_error_from_c_string(self, "No input");
    }

    // TODO should more efficient version without strlen be used?
    return octaspire_dern_vm_read_from_buffer_and_eval_in_global_environment(
        self,
        str,
        strlen(str));
}

octaspire_dern_value_t *octaspire_dern_vm_read_from_buffer_and_eval_in_global_environment(
    octaspire_dern_vm_t *self,
    char const * const buffer,
    size_t const lengthInOctets)
{
    if (!buffer || lengthInOctets == 0)
    {
        return octaspire_dern_vm_create_new_value_error_from_c_string(self, "No input");
    }

    octaspire_input_t *input =
        octaspire_input_new_from_buffer(buffer, lengthInOctets, self->allocator);

    if (!input)
    {
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            self,
            "Allocation failure of input");
    }

    octaspire_dern_value_t * const result =
        octaspire_dern_vm_read_from_octaspire_input_and_eval_in_global_environment(self, input);

    octaspire_input_release(input);
    input = 0;

    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_read_from_path_and_eval_in_global_environment(
    octaspire_dern_vm_t *self,
    char const * const path)
{
    size_t bufLen = 0;
    char *buffer = octaspire_helpers_path_to_buffer(path, &bufLen, self->allocator, self->stdio);

    if (!buffer || !bufLen)
    {
        return octaspire_dern_vm_create_new_value_error_from_c_string(self, "No input");
    }

    octaspire_dern_value_t *result =
        octaspire_dern_vm_read_from_buffer_and_eval_in_global_environment(self, buffer, bufLen);

    octaspire_allocator_free(self->allocator, buffer);
    buffer = 0;
    bufLen = 0;

    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_get_value_nil(
    octaspire_dern_vm_t *self)
{
    //return self->valueNil;
    return octaspire_dern_vm_create_new_value_copy(self, self->valueNil);
}

octaspire_dern_value_t *octaspire_dern_vm_get_value_true(
    octaspire_dern_vm_t *self)
{
    //return self->valueTrue;
    return octaspire_dern_vm_create_new_value_copy(self, self->valueTrue);
}

octaspire_dern_value_t *octaspire_dern_vm_get_value_false(
    octaspire_dern_vm_t *self)
{
    //return self->valueFalse;
    return octaspire_dern_vm_create_new_value_copy(self, self->valueFalse);
}

octaspire_allocator_t *octaspire_dern_vm_get_allocator(
    octaspire_dern_vm_t *self)
{
    return self->allocator;
}

void octaspire_dern_vm_set_exit_code(
    octaspire_dern_vm_t *self,
    int32_t const code)
{
    self->exitCode = code;
}

int32_t octaspire_dern_vm_get_exit_code(
    octaspire_dern_vm_t const * const self)
{
    return self->exitCode;
}

bool octaspire_dern_vm_is_quit(
    octaspire_dern_vm_t const * const self)
{
    return self->quit;
}

void octaspire_dern_vm_quit(
    octaspire_dern_vm_t *self)
{
    self->quit = true;
}

// Create some helper methods.

bool octaspire_dern_vm_create_and_register_new_builtin(
    octaspire_dern_vm_t * const self,
    char const * const name,
    octaspire_dern_c_function const funcPointer,
    size_t const numRequiredActualArguments,
    char const * const docStr,
    bool const howtoAllowed,
    octaspire_dern_environment_t * const targetEnv)
{
    octaspire_helpers_verify_not_null(self);
    octaspire_helpers_verify_not_null(targetEnv);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(self);

    octaspire_dern_builtin_t * const builtin = octaspire_dern_builtin_new(
        funcPointer,
        self->allocator,
        name,
        numRequiredActualArguments,
        docStr,
        howtoAllowed);

    if (!builtin)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));
        return false;
    }

    octaspire_dern_value_t * const symbol =
        octaspire_dern_vm_create_new_value_symbol_from_c_string(self, name);

    octaspire_dern_vm_push_value(self, symbol);

    octaspire_dern_value_t * const builtinVal =
        octaspire_dern_vm_create_new_value_builtin(self, builtin, docStr);

    octaspire_dern_vm_push_value(self, builtinVal);

    if (!octaspire_dern_environment_set(targetEnv, symbol, builtinVal))
    {
        octaspire_dern_vm_pop_value(self, builtinVal);
        octaspire_dern_vm_pop_value(self, symbol);
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));
        return false;
    }

    octaspire_dern_vm_pop_value(self, builtinVal);
    octaspire_dern_vm_pop_value(self, symbol);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));

    return true;
}

bool octaspire_dern_vm_create_and_register_new_special(
    octaspire_dern_vm_t * const self,
    char const * const name,
    octaspire_dern_c_function const funcPointer,
    size_t const numRequiredActualArguments,
    char const * const docStr,
    bool const howtoAllowed,
    octaspire_dern_environment_t * const targetEnv)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(self);

    octaspire_dern_special_t * const special = octaspire_dern_special_new(
        funcPointer,
        self->allocator,
        name,
        numRequiredActualArguments,
        docStr,
        howtoAllowed);

    if (!special)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));
        return false;
    }

    octaspire_dern_value_t * const symbol =
        octaspire_dern_vm_create_new_value_symbol_from_c_string(self, name);

    octaspire_dern_vm_push_value(self, symbol);

    octaspire_dern_value_t * const specialVal =
        octaspire_dern_vm_create_new_value_special(self, special, docStr);

    octaspire_dern_vm_push_value(self, specialVal);

    if (!octaspire_dern_environment_set(targetEnv, symbol, specialVal))
    {
        octaspire_dern_vm_pop_value(self, specialVal);
        octaspire_dern_vm_pop_value(self, symbol);
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));
        return false;
    }

    octaspire_dern_vm_pop_value(self, specialVal);
    octaspire_dern_vm_pop_value(self, symbol);
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));
    return true;
}

bool octaspire_dern_vm_create_and_define_new_integer(
    octaspire_dern_vm_t * const self,
    char const * const name,
    char const * const docstr,
    int32_t const value)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(self);

    octaspire_helpers_verify_not_null(self);
    octaspire_helpers_verify_not_null(self);
    octaspire_helpers_verify_not_null(self);

    octaspire_helpers_verify_true(strlen(name) > 0);
    octaspire_helpers_verify_true(strlen(docstr) > 0);

    octaspire_string_t *str = octaspire_string_new_format(
        octaspire_dern_vm_get_allocator(self),
        "(define %s [%s] %" PRId32 ")",
        name,
        docstr,
        value);

    octaspire_helpers_verify_not_null(str);

    octaspire_dern_value_t *result =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            self,
            octaspire_string_get_c_string(str));

    octaspire_helpers_verify_not_null(result);

    octaspire_string_release(str);
    str = 0;

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));

    if (octaspire_dern_value_is_boolean(result) &&
        octaspire_dern_value_as_boolean_get_value(result))
    {
        return true;
    }

    return false;
}

size_t octaspire_dern_vm_get_stack_length(
    octaspire_dern_vm_t const * const self)
{
    return octaspire_vector_get_length(self->stack);
}

void octaspire_dern_vm_print_stack(
    octaspire_dern_vm_t const * const self)
{
    if (octaspire_vector_is_empty(self->stack))
    {
        printf("\n\n-- STACK IS EMPTY --\n\n");
        return;
    }

    printf("Stack has %zu elements\n", octaspire_vector_get_length(self->stack));
    for (ptrdiff_t i = (ptrdiff_t)octaspire_vector_get_length(self->stack) - 1;
         i >= 0; --i)
    {
#ifdef __AROS__
        printf("--------------------------- #%ld ------------------------\n", (long)i);
#else
        printf("--------------------------- #%td ------------------------\n", i);
#endif
        octaspire_dern_value_print(
            octaspire_vector_get_element_at(self->stack, i),
            self->allocator);
        printf("--------------------------- end ------------------------\n\n");
    }
}


octaspire_dern_value_t *
octaspire_dern_vm_helper_create_new_value_vector_of_integers_from_vector_of_size_t(
    octaspire_dern_vm_t * const self,
    octaspire_vector_t const * const vectorContainingSizeTs)
{
    octaspire_helpers_verify_true(
        self &&
        vectorContainingSizeTs &&
        sizeof(size_t) == octaspire_vector_get_element_size_in_octets(
            vectorContainingSizeTs));

    size_t const stackLength = octaspire_dern_vm_get_stack_length(self);

    octaspire_dern_value_t * const result = octaspire_dern_vm_create_new_value_vector(self);
    if (!octaspire_dern_vm_push_value(self, result))
    {
        abort();
    }

    for (size_t i = 0; i < octaspire_vector_get_length(vectorContainingSizeTs); ++i)
    {
        size_t const idx =
            *(size_t const * const)octaspire_vector_get_element_at_const(
                vectorContainingSizeTs,
                (ptrdiff_t)i);

        octaspire_dern_value_t *tmpVal =
            octaspire_dern_vm_create_new_value_integer(self, (int32_t)idx);

        octaspire_helpers_verify_not_null(tmpVal);

        if (!octaspire_dern_value_as_vector_push_back_element(
                result,
                &tmpVal))
        {
            abort();
        }
    }

    if (!octaspire_dern_vm_pop_value(self, result))
    {
        abort();
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_find_from_value(
    octaspire_dern_vm_t * const self,
    octaspire_dern_value_t * const value,
    octaspire_dern_value_t const * const key)
{
    octaspire_helpers_verify_true(self && value && key);
    size_t const stackLength = octaspire_dern_vm_get_stack_length(self);

    switch (value->typeTag)
    {
        case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
        {
            abort();
        }

        case OCTASPIRE_DERN_VALUE_TAG_STRING:
        {
            if (key->typeTag == OCTASPIRE_DERN_VALUE_TAG_CHARACTER)
            {
                octaspire_vector_t *foundIndices =
                    octaspire_string_find_char(
                        value->value.string,
                        key->value.character,
                        0);

                octaspire_dern_value_t * const result =
                    octaspire_dern_vm_helper_create_new_value_vector_of_integers_from_vector_of_size_t(
                        self,
                        foundIndices);

                octaspire_helpers_verify_not_null(result);

                octaspire_vector_release(foundIndices);
                foundIndices = 0;

                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));
                return result;
            }
            else if (key->typeTag == OCTASPIRE_DERN_VALUE_TAG_STRING)
            {
                octaspire_vector_t *foundIndices = octaspire_string_find_string(
                    value->value.string,
                    key->value.string,
                    0,
                    octaspire_string_get_length_in_ucs_characters(key->value.string));

                if (!foundIndices)
                {
                    octaspire_helpers_verify_true(
                        stackLength == octaspire_dern_vm_get_stack_length(self));

                    return octaspire_dern_vm_create_new_value_vector(self);
                }

                octaspire_dern_value_t * const result =
                    octaspire_dern_vm_helper_create_new_value_vector_of_integers_from_vector_of_size_t(
                        self,
                        foundIndices);

                octaspire_helpers_verify_not_null(result);

                octaspire_vector_release(foundIndices);
                foundIndices = 0;

                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));
                return result;
            }
            else if (key->typeTag == OCTASPIRE_DERN_VALUE_TAG_SYMBOL)
            {
                octaspire_vector_t *foundIndices = octaspire_string_find_string(
                    value->value.string,
                    key->value.symbol,
                    0,
                    octaspire_string_get_length_in_ucs_characters(key->value.symbol));

                octaspire_dern_value_t * const result =
                    octaspire_dern_vm_helper_create_new_value_vector_of_integers_from_vector_of_size_t(
                        self,
                        foundIndices);

                octaspire_helpers_verify_not_null(result);

                octaspire_vector_release(foundIndices);
                foundIndices = 0;

                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));
                return result;
            }
            else if (key->typeTag == OCTASPIRE_DERN_VALUE_TAG_INTEGER)
            {
                octaspire_string_t *tmpStr =
                    octaspire_string_new_format(
                        self->allocator,
                        "%" PRId32 "",
                        key->value.integer);

                octaspire_vector_t *foundIndices = octaspire_string_find_string(
                    value->value.string,
                    tmpStr,
                    0,
                    octaspire_string_get_length_in_ucs_characters(tmpStr));

                octaspire_string_release(tmpStr);
                tmpStr = 0;

                octaspire_dern_value_t * const result =
                    octaspire_dern_vm_helper_create_new_value_vector_of_integers_from_vector_of_size_t(
                        self,
                        foundIndices);

                octaspire_helpers_verify_not_null(result);

                octaspire_vector_release(foundIndices);
                foundIndices = 0;

                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));
                return result;
            }
            else if (key->typeTag == OCTASPIRE_DERN_VALUE_TAG_REAL)
            {
                octaspire_string_t *tmpStr =
                    octaspire_string_new_format(
                        self->allocator,
                        "%g",
                        key->value.real);

                octaspire_vector_t *foundIndices = octaspire_string_find_string(
                    value->value.string,
                    tmpStr,
                    0,
                    octaspire_string_get_length_in_ucs_characters(tmpStr));

                octaspire_dern_value_t * const result =
                    octaspire_dern_vm_helper_create_new_value_vector_of_integers_from_vector_of_size_t(
                        self,
                        foundIndices);

                octaspire_helpers_verify_not_null(result);

                octaspire_vector_release(foundIndices);
                foundIndices = 0;

                octaspire_string_release(tmpStr);
                tmpStr = 0;

                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));
                return result;
            }
            else
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));
                return octaspire_dern_vm_create_new_value_error_format(
                    self,
                    "Type '%s' cannot be searched from type 'string'",
                    octaspire_dern_value_helper_get_type_as_c_string(key->typeTag));
            }
        }

        case OCTASPIRE_DERN_VALUE_TAG_SYMBOL:
        {
            if (key->typeTag == OCTASPIRE_DERN_VALUE_TAG_CHARACTER)
            {
                octaspire_vector_t *foundIndices = octaspire_string_find_char(
                    value->value.symbol,
                    key->value.character,
                    0);

                octaspire_dern_value_t * const result =
                    octaspire_dern_vm_helper_create_new_value_vector_of_integers_from_vector_of_size_t(
                        self,
                        foundIndices);

                octaspire_helpers_verify_not_null(result);

                octaspire_vector_release(foundIndices);
                foundIndices = 0;

                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));
                return result;
            }
            else if (key->typeTag == OCTASPIRE_DERN_VALUE_TAG_STRING)
            {
                octaspire_vector_t *foundIndices = octaspire_string_find_string(
                    value->value.symbol,
                    key->value.string,
                    0,
                    octaspire_string_get_length_in_ucs_characters(key->value.string));

                octaspire_dern_value_t * const result =
                    octaspire_dern_vm_helper_create_new_value_vector_of_integers_from_vector_of_size_t(
                        self,
                        foundIndices);

                octaspire_helpers_verify_not_null(result);

                octaspire_vector_release(foundIndices);
                foundIndices = 0;

                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));
                return result;
            }
            else if (key->typeTag == OCTASPIRE_DERN_VALUE_TAG_SYMBOL)
            {
                octaspire_vector_t *foundIndices = octaspire_string_find_string(
                    value->value.symbol,
                    key->value.symbol,
                    0,
                    octaspire_string_get_length_in_ucs_characters(key->value.symbol));

                octaspire_dern_value_t * const result =
                    octaspire_dern_vm_helper_create_new_value_vector_of_integers_from_vector_of_size_t(
                        self,
                        foundIndices);

                octaspire_helpers_verify_not_null(result);

                octaspire_vector_release(foundIndices);
                foundIndices = 0;

                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));
                return result;
            }
            else if (key->typeTag == OCTASPIRE_DERN_VALUE_TAG_INTEGER)
            {
                octaspire_string_t *tmpStr =
                    octaspire_string_new_format(
                        self->allocator,
                        "%" PRId32 "",
                        key->value.integer);

                octaspire_vector_t *foundIndices = octaspire_string_find_string(
                    value->value.symbol,
                    tmpStr,
                    0,
                    octaspire_string_get_length_in_ucs_characters(tmpStr));

                octaspire_dern_value_t * const result =
                    octaspire_dern_vm_helper_create_new_value_vector_of_integers_from_vector_of_size_t(
                        self,
                        foundIndices);

                octaspire_helpers_verify_not_null(result);

                octaspire_vector_release(foundIndices);
                foundIndices = 0;

                octaspire_string_release(tmpStr);
                tmpStr = 0;

                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));
                return result;
            }
            else if (key->typeTag == OCTASPIRE_DERN_VALUE_TAG_REAL)
            {
                octaspire_string_t *tmpStr =
                    octaspire_string_new_format(
                        self->allocator,
                        "%g",
                        key->value.real);

                octaspire_vector_t *foundIndices = octaspire_string_find_string(
                    value->value.symbol,
                    tmpStr,
                    0,
                    octaspire_string_get_length_in_ucs_characters(tmpStr));

                octaspire_dern_value_t * const result =
                    octaspire_dern_vm_helper_create_new_value_vector_of_integers_from_vector_of_size_t(
                        self,
                        foundIndices);

                octaspire_helpers_verify_not_null(result);

                octaspire_vector_release(foundIndices);
                foundIndices = 0;

                octaspire_string_release(tmpStr);
                tmpStr = 0;

                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));
                return result;
            }
            else
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));
                return octaspire_dern_vm_create_new_value_error_format(
                    self,
                    "Type '%s' cannot be searched from type 'symbol'",
                    octaspire_dern_value_helper_get_type_as_c_string(key->typeTag));
            }
        }

        case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
        {
            octaspire_dern_value_t *result = octaspire_dern_vm_create_new_value_vector(self);

            octaspire_dern_vm_push_value(self, result);

            for (size_t i = 0; i < octaspire_vector_get_length(value->value.vector); ++i)
            {
                octaspire_dern_value_t * const element =
                    octaspire_vector_get_element_at(
                        value->value.vector,
                        (ptrdiff_t)i);

                if (octaspire_dern_value_is_equal(element, key))
                {
                    octaspire_dern_value_t * const tmpVal =
                        octaspire_dern_vm_create_new_value_integer(self, (int32_t)i);

                    octaspire_dern_value_as_vector_push_back_element(result, &tmpVal);
                }
            }

            octaspire_dern_vm_pop_value(self, result);
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));
            return result;
        }

        case OCTASPIRE_DERN_VALUE_TAG_HASH_MAP:
        {
            uint32_t const hash = octaspire_dern_value_get_hash(key);

            octaspire_map_element_t * const element =
                octaspire_map_get(value->value.hashMap, hash, &key);

            if (element)
            {
                octaspire_dern_value_t * const resVal =
                    octaspire_map_element_get_value(element);

                if (resVal)
                {
                    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));
                    return resVal;
                }
            }

            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));
            return octaspire_dern_vm_get_value_nil(self);
        }

        case OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT:
        {
            octaspire_dern_value_t *result =
                octaspire_dern_environment_get(value->value.environment, key);

            if (result)
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));
                return result;
            }

            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));
            return octaspire_dern_vm_get_value_nil(self);
        }

        case OCTASPIRE_DERN_VALUE_TAG_NIL:
        case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
        case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
        case OCTASPIRE_DERN_VALUE_TAG_REAL:
        case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
        case OCTASPIRE_DERN_VALUE_TAG_ERROR:
        case OCTASPIRE_DERN_VALUE_TAG_QUEUE:
        case OCTASPIRE_DERN_VALUE_TAG_LIST:
        case OCTASPIRE_DERN_VALUE_TAG_FUNCTION:
        case OCTASPIRE_DERN_VALUE_TAG_SPECIAL:
        case OCTASPIRE_DERN_VALUE_TAG_BUILTIN:
        case OCTASPIRE_DERN_VALUE_TAG_PORT:
        case OCTASPIRE_DERN_VALUE_TAG_C_DATA:
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self));
            return octaspire_dern_vm_create_new_value_error_format(
                self,
                "'find' doesn't support search from type '%s'",
                octaspire_dern_value_helper_get_type_as_c_string(value->typeTag));
        }
    }

    abort();
    return 0;
}

octaspire_dern_value_t *octaspire_dern_vm_get_global_environment(
    octaspire_dern_vm_t *self)
{
    return self->globalEnvironment;
}

octaspire_dern_value_t const *octaspire_dern_vm_get_global_environment_const(
    octaspire_dern_vm_t const * const self)
{
    return self->globalEnvironment;
}

void  octaspire_dern_vm_set_user_data(octaspire_dern_vm_t * const self, void *userData)
{
    self->userData = userData;
}

void *octaspire_dern_vm_get_user_data(octaspire_dern_vm_t const * const self)
{
    return self->userData;
}

octaspire_dern_value_t *octaspire_dern_vm_get_function_return(
    octaspire_dern_vm_t * const self)
{
    return self->functionReturn;
}

void octaspire_dern_vm_set_function_return(
    octaspire_dern_vm_t * const self,
    octaspire_dern_value_t * const value)
{
    if (!value)
    {
        // Only VM is allowed to set functionReturn to NULL
        abort();
    }

    self->functionReturn = value;
}

void octaspire_dern_vm_set_prevent_gc(octaspire_dern_vm_t * const self, bool const prevent)
{
    self->preventGc = prevent;
}

void octaspire_dern_vm_set_gc_trigger_limit(octaspire_dern_vm_t * const self, size_t const numAllocs)
{
    self->gcTriggerLimit = numAllocs;
}

octaspire_dern_vm_custom_require_source_file_loader_t
octaspire_dern_vm_get_custom_require_source_file_pre_loader(
        octaspire_dern_vm_t * const self)

{
    return self->config.preLoaderForRequireSrc;
}

bool octaspire_dern_vm_add_library(
    octaspire_dern_vm_t *self,
    char const * const name,
    octaspire_dern_lib_t *library)
{
    if (octaspire_dern_vm_has_library(self, name))
    {
        return false;
    }

    octaspire_string_t *str =
        octaspire_string_new(name, self->allocator);

    return octaspire_map_put(
        self->libraries,
        octaspire_string_get_hash(str),
        &str,
        &library);
}

bool octaspire_dern_vm_has_library(
    octaspire_dern_vm_t const * const self,
    char const * const name)
{
    octaspire_string_t *str = octaspire_string_new(
        name,
        self->allocator);

    octaspire_helpers_verify_not_null(str);

    bool const result = (octaspire_map_get(
            self->libraries,
            octaspire_string_get_hash(str),
            &str) != 0);

    octaspire_string_release(str);
    str = 0;

    return result;
}

octaspire_dern_lib_t *octaspire_dern_vm_get_library(
    octaspire_dern_vm_t * const self,
    char const * const name)
{
    octaspire_string_t *str = octaspire_string_new(
        name,
        self->allocator);

    octaspire_helpers_verify_not_null(str);

    octaspire_map_element_t *element = octaspire_map_get(
        self->libraries,
        octaspire_string_get_hash(str),
        &str);

    octaspire_string_release(str);
    str = 0;

    if (!element)
    {
        return 0;
    }

    return octaspire_map_element_get_value(element);
}

octaspire_stdio_t *octaspire_dern_vm_get_stdio(octaspire_dern_vm_t * const self)
{
    return self->stdio;
}

bool octaspire_dern_vm_add_command_line_argument(
    octaspire_dern_vm_t * const self,
    char const * const argument)
{
    octaspire_string_t *str =
        octaspire_string_new(
            argument,
            self->allocator);

    if (!str)
    {
        return false;
    }

    const bool result = octaspire_vector_push_back_element(
        self->commandLineArguments,
        &str);

    if (!result)
    {
        octaspire_string_release(str);
        str = 0;
    }

    return result;
}

bool octaspire_dern_vm_add_environment_variable(
    octaspire_dern_vm_t * const self,
    char const * const variable)
{
    octaspire_string_t *str =
        octaspire_string_new(
            variable,
            self->allocator);

    if (!str)
    {
        return false;
    }

    const bool result = octaspire_vector_push_back_element(
        self->environmentVariables,
        &str);

    if (!result)
    {
        octaspire_string_release(str);
        str = 0;
    }

    return result;
}

size_t octaspire_dern_vm_get_number_of_command_line_arguments(
    octaspire_dern_vm_t const * const self)
{
    return octaspire_vector_get_length(
        self->commandLineArguments);
}

char const *octaspire_dern_vm_get_command_line_argument_at(
    octaspire_dern_vm_t const * const self,
    ptrdiff_t const index)
{
    octaspire_string_t *str =
        octaspire_vector_get_element_at(
            self->commandLineArguments, index);

    if (!str)
    {
        return 0;
    }

    return octaspire_string_get_c_string(str);
}

size_t octaspire_dern_vm_get_number_of_environment_variables(
    octaspire_dern_vm_t const * const self)
{
    return octaspire_vector_get_length(
        self->environmentVariables);
}

char const *octaspire_dern_vm_get_environment_variable_at(
    octaspire_dern_vm_t const * const self,
    ptrdiff_t const index)
{
    octaspire_string_t *str =
        octaspire_vector_get_element_at(
            self->environmentVariables, index);

    if (!str)
    {
        return 0;
    }

    return octaspire_string_get_c_string(str);
}

octaspire_dern_vm_config_t const * octaspire_dern_vm_get_config_const(
    octaspire_dern_vm_t const * const self)
{
    return &(self->config);
}

