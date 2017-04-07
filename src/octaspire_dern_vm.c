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
#include <octaspire/core/octaspire_helpers.h>
#include <octaspire/core/octaspire_input.h>
#include <octaspire/core/octaspire_container_vector.h>
#include <octaspire/core/octaspire_helpers.h>
#include "octaspire/dern/octaspire_dern_value.h"
#include "octaspire/dern/octaspire_dern_environment.h"
#include "octaspire/dern/octaspire_dern_lexer.h"
#include "octaspire/dern/octaspire_dern_stdlib.h"



static void octaspire_dern_vm_private_release_value(
    octaspire_dern_vm_t *self,
    octaspire_dern_value_t *value);



struct octaspire_dern_vm_t
{
    octaspire_container_vector_t *stack;
    octaspire_memory_allocator_t *allocator;
    octaspire_stdio_t            *stdio;
    octaspire_container_vector_t *all;
    octaspire_dern_value_t       *globalEnvironment;
    octaspire_dern_value_t       *valueNil;
    octaspire_dern_value_t       *valueTrue;
    octaspire_dern_value_t       *valueFalse;
    size_t                        numAllocatedWithoutGc;
    bool                          preventGc;
    size_t                        gcTriggerLimit;
    int32_t                       exitCode;
    bool                          quit;
    void                         *userData;
    uintmax_t                     nextFreeUniqueIdForValues;
    octaspire_dern_value_t       *functionReturn;
};

octaspire_dern_value_t *octaspire_dern_vm_private_create_new_value_struct(octaspire_dern_vm_t* self, octaspire_dern_value_tag_t const typeTag);

bool octaspire_dern_vm_private_mark_all(octaspire_dern_vm_t *self);
bool octaspire_dern_vm_private_mark(octaspire_dern_vm_t *self, octaspire_dern_value_t *value);
bool octaspire_dern_vm_private_sweep(octaspire_dern_vm_t *self);
octaspire_dern_value_t *octaspire_dern_vm_private_parse_token(
    octaspire_dern_vm_t * const self,
    octaspire_dern_lexer_token_t const * const token,
    octaspire_input_t *input);

octaspire_dern_vm_t *octaspire_dern_vm_new(
    octaspire_memory_allocator_t *allocator,
    octaspire_stdio_t *octaspireStdio)
{
    octaspire_dern_vm_t *self =
        octaspire_memory_allocator_malloc(allocator, sizeof(octaspire_dern_vm_t));

    if (!self)
    {
        return 0;
    }

    self->allocator = allocator;
    self->stdio     = octaspireStdio;
    self->numAllocatedWithoutGc = 0;
    self->preventGc = false;
    self->gcTriggerLimit = 1024;
    self->exitCode = 0;
    self->quit = false;
    self->userData = 0;
    self->nextFreeUniqueIdForValues = 0;
    self->functionReturn = 0;

    self->stack = octaspire_container_vector_new_with_preallocated_elements(
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

    self->all = octaspire_container_vector_new(sizeof(octaspire_dern_value_t*), true, 0, self->allocator);

    if (!self->all)
    {
        octaspire_dern_vm_release(self);
        self = 0;
        return 0;
    }

    octaspire_dern_environment_t *env = octaspire_dern_environment_new(0, self, self->allocator);

    if (!env)
    {
        octaspire_dern_vm_release(self);
        self = 0;
        return 0;
    }

    self->globalEnvironment = octaspire_dern_vm_create_new_value_environment_from_environment(self, env);

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
        octaspire_dern_vm_create_new_value_symbol(self, octaspire_container_utf8_string_new(
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
        octaspire_dern_vm_create_new_value_symbol(self, octaspire_container_utf8_string_new(
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
        octaspire_dern_vm_create_new_value_symbol(self, octaspire_container_utf8_string_new(
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

    // not
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "not",
        octaspire_dern_vm_builtin_not,
        1,
        "Reverse boolean value",
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
        env))
    {
        abort();
    }

    // nth
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "nth",
        octaspire_dern_vm_builtin_nth,
        2,
        "Index collection; get element at the given index on the given collection",
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
        "Set atomic values, or elements of collections (vector, map, string) at the given index/key",
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
        "Remove first element of a vector",
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
        "Add number arguments",
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
        env))
    {
        abort();
    }

    // mutable
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "mutable",
        octaspire_dern_vm_builtin_mutable,
        1,
        "Make value mutable for count times, indefinitely or not at all (constant)",
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
        env))
    {
        abort();
    }

    // TODO XXX move to other builtins
    // ==
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "==",
        octaspire_dern_vm_builtin_equals_equals,
        1,
        "Predicate telling whether all the given values are equal. "
        "Takes 2..n arguments. Evaluates arguments only as long as those are equal. "
        "Does not evaluate rest of the arguments if one is not equal",
        env))
    {
        abort();
    }

    // TODO XXX move to other builtins
    // !=
    if (!octaspire_dern_vm_create_and_register_new_builtin(
        self,
        "!=",
        octaspire_dern_vm_builtin_exclamation_equals,
        1,
        "Predicate telling whether all the given values are not equal. "
        "Takes 2..n arguments. Evaluates arguments only as long as unequal is not found. "
        "Does not evaluate rest of the arguments if one is unequal",
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
        "Takes 2..n arguments. Evaluates arguments only as long as those are less than the previous. "
        "Does not evaluate rest of the arguments if one is larger or equal",
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
        "Takes 2..n arguments. Evaluates arguments only as long as those are greater than the previous. "
        "Does not evaluate rest of the arguments if one is smaller or equal",
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
        "Takes 2..n arguments. Evaluates arguments only as long as those are equal or less than the previous. "
        "Does not evaluate rest of the arguments if one is larger",
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
        env))
    {
        abort();
    }

    // uid
    if (!octaspire_dern_vm_create_and_register_new_special(
        self,
        "uid",
        octaspire_dern_vm_special_uid,
        2,
        "Get unique id of a value",
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

    // At this point stack had nil and self->globalEnvironment was tried to remove
    //octaspire_dern_vm_pop_value(self, self->globalEnvironment);

    octaspire_container_vector_clear(self->stack);
    octaspire_dern_vm_gc(self);

    octaspire_container_vector_release(self->stack);

    octaspire_container_vector_release(self->all);

    octaspire_memory_allocator_free(self->allocator, self);
}

bool octaspire_dern_vm_push_value(octaspire_dern_vm_t *self, octaspire_dern_value_t *value)
{
    return octaspire_container_vector_push_back_element(self->stack, &value);
}

bool octaspire_dern_vm_pop_value (octaspire_dern_vm_t *self, octaspire_dern_value_t *valueForBalanceCheck)
{
    if (octaspire_container_vector_peek_back_element(self->stack) != valueForBalanceCheck)
    {
        printf(
            "\n\n ----- STACK UNBALANCED! -----\n (real top)%p != (user says is top)%p\n\n",
            (void*)octaspire_container_vector_peek_back_element(self->stack),
            (void*)valueForBalanceCheck);

        printf("REAL TOP IS:\n");
        octaspire_dern_value_print(octaspire_container_vector_peek_back_element(self->stack), self->allocator);
        printf("USER SAYS THIS IS TOP:\n");
        octaspire_dern_value_print(valueForBalanceCheck, self->allocator);
    }

    if (octaspire_container_vector_peek_back_element(self->stack) != valueForBalanceCheck)
    {
        abort();
    }

    return octaspire_container_vector_pop_back_element(self->stack);
}

void const * octaspire_dern_vm_get_top_value(octaspire_dern_vm_t const * const self)
{
    return octaspire_container_vector_peek_back_element(self->stack);
}

octaspire_dern_value_t *octaspire_dern_vm_peek_value(octaspire_dern_vm_t *self)
{
    return octaspire_container_vector_peek_back_element(self->stack);
}




octaspire_dern_value_t *octaspire_dern_vm_private_create_new_value_struct(octaspire_dern_vm_t* self, octaspire_dern_value_tag_t const typeTag)
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
        octaspire_memory_allocator_malloc(self->allocator, sizeof(octaspire_dern_value_t));

    if (!result)
    {
        octaspire_helpers_verify(false);
        return 0;
    }

    octaspire_container_vector_push_back_element(self->all, &result);

    result->typeTag                        = typeTag;
    result->mark                           = false;
    result->containerLengthAtTimeOfMarking = 0;
    result->docstr                         = 0;
    result->vm                             = self;
    result->mutableCounter                 = -1;
    result->uniqueId                       = self->nextFreeUniqueIdForValues;

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

    if (valueToBeCopied->docstr)
    {
        result->docstr = octaspire_dern_vm_create_new_value_copy(self, valueToBeCopied->docstr);
    }

    if (valueToBeCopied->docvec)
    {
        result->docvec = octaspire_dern_vm_create_new_value_copy(self, valueToBeCopied->docvec);
    }

    switch (result->typeTag)
    {
        case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
        {
            abort();
        }
        break;

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
            result->value.string =
                octaspire_container_utf8_string_new_copy(valueToBeCopied->value.string, self->allocator);
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
        {
            result->value.character =
                octaspire_container_utf8_string_new_copy(valueToBeCopied->value.character, self->allocator);
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_SYMBOL:
        {
            result->value.symbol =
                octaspire_container_utf8_string_new_copy(valueToBeCopied->value.symbol, self->allocator);
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_ERROR:
        {
            result->value.error =
                octaspire_container_utf8_string_new_copy(valueToBeCopied->value.error, self->allocator);
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
        {
            result->value.vector = octaspire_container_vector_new(
                sizeof(octaspire_dern_value_t*),
                true,
                0,
                self->allocator);

            for (size_t i = 0; i < octaspire_container_vector_get_length(valueToBeCopied->value.vector); ++i)
            {
                if (!octaspire_container_vector_push_back_element(
                    result->value.vector,
                    octaspire_dern_vm_create_new_value_copy(
                        self,
                        octaspire_container_vector_get_element_at(valueToBeCopied->value.vector, i))))
                {
                    abort();
                }
            }
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_HASH_MAP:
        {
            abort();
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT:
        {
            abort();
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_FUNCTION:
        {
            abort();
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_SPECIAL:
        {
            abort();
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_BUILTIN:
        {
            abort();
        }
        break;
    }

    octaspire_dern_vm_pop_value(self, result);
    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_nil(octaspire_dern_vm_t *self)
{
    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(self, OCTASPIRE_DERN_VALUE_TAG_NIL);
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_boolean(octaspire_dern_vm_t *self, bool const value)
{
    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(self, OCTASPIRE_DERN_VALUE_TAG_BOOLEAN);
    result->value.boolean = value;
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_integer   (octaspire_dern_vm_t *self, int32_t const value)
{
    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(self, OCTASPIRE_DERN_VALUE_TAG_INTEGER);
    result->value.integer = value;
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_real      (octaspire_dern_vm_t *self, double  const value)
{
    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(self, OCTASPIRE_DERN_VALUE_TAG_REAL);
    result->value.real = value;
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_string    (octaspire_dern_vm_t *self, octaspire_container_utf8_string_t * const value)
{
    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(self, OCTASPIRE_DERN_VALUE_TAG_STRING);
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

    octaspire_container_utf8_string_t *str = octaspire_container_utf8_string_new_vformat(
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
        octaspire_container_utf8_string_new(value, self->allocator));
}




octaspire_dern_value_t *octaspire_dern_vm_create_new_value_character (octaspire_dern_vm_t *self, octaspire_container_utf8_string_t * const value)
{
    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(self, OCTASPIRE_DERN_VALUE_TAG_CHARACTER);
    result->value.character = value;
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_character_from_uint32t (octaspire_dern_vm_t *self, uint32_t const value)
{
    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(self, OCTASPIRE_DERN_VALUE_TAG_CHARACTER);
    result->value.character = octaspire_container_utf8_string_new("", self->allocator);
    octaspire_container_utf8_string_push_back_ucs_character(result->value.character, value);
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_symbol    (octaspire_dern_vm_t *self, octaspire_container_utf8_string_t * const value)
{
    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(self, OCTASPIRE_DERN_VALUE_TAG_SYMBOL);
    result->value.symbol = value;
    return result;
}

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_symbol_from_c_string    (octaspire_dern_vm_t *self, char const * const value)
{
    return octaspire_dern_vm_create_new_value_symbol(
        self,
        octaspire_container_utf8_string_new(value, self->allocator));
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_error     (octaspire_dern_vm_t *self, octaspire_container_utf8_string_t * const value)
{
    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(self, OCTASPIRE_DERN_VALUE_TAG_ERROR);
    result->value.error = value;
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_error_format(
    octaspire_dern_vm_t *self,
    char const * const fmt,
    ...)
{
    va_list arguments;
    va_start(arguments, fmt);

    octaspire_container_utf8_string_t *str = octaspire_container_utf8_string_new_vformat(
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
        octaspire_container_utf8_string_new(value, self->allocator));
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_vector_from_vector      (octaspire_dern_vm_t *self, octaspire_container_vector_t * const value)
{
    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(self, OCTASPIRE_DERN_VALUE_TAG_VECTOR);
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

















octaspire_dern_value_t *octaspire_dern_vm_create_new_value_vector      (octaspire_dern_vm_t *self)
{
    octaspire_container_vector_t *vec = octaspire_container_vector_new(
        sizeof(octaspire_dern_value_t*),
        true,
        0,
        self->allocator);

    return octaspire_dern_vm_create_new_value_vector_from_vector(self, vec);
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_hash_map_from_hash_map(octaspire_dern_vm_t *self, octaspire_container_hash_map_t * const value)
{
    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(self, OCTASPIRE_DERN_VALUE_TAG_HASH_MAP);
    result->value.hashMap = value;
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_hash_map(octaspire_dern_vm_t *self)
{
    octaspire_container_hash_map_t *hashMap = octaspire_container_hash_map_new(
        sizeof(octaspire_dern_value_t*),
        true,
        sizeof(octaspire_dern_value_t*),
        true,
        (octaspire_container_hash_map_key_compare_function_t)octaspire_dern_value_is_equal,
        (octaspire_container_hash_map_key_hash_function_t)octaspire_dern_value_get_hash,
        0,
        0,
        self->allocator);

    return octaspire_dern_vm_create_new_value_hash_map_from_hash_map(self, hashMap);
}

struct octaspire_dern_value_t *octaspire_dern_vm_create_new_value_environment (octaspire_dern_vm_t *self, octaspire_dern_value_t *enclosing)
{
    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(
        self,
        OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    result->value.environment =
        octaspire_dern_environment_new(enclosing, self, self->allocator);

    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_environment_from_environment(octaspire_dern_vm_t *self, octaspire_dern_environment_t * const value)
{
    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(self, OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);
    result->value.environment = value;
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_function(
    octaspire_dern_vm_t *self,
    octaspire_dern_function_t * const value,
    char const * const docstr,
    octaspire_container_vector_t *docVec)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(self);

    octaspire_dern_value_t *result =
        octaspire_dern_vm_private_create_new_value_struct(self, OCTASPIRE_DERN_VALUE_TAG_FUNCTION);

    octaspire_dern_vm_push_value(self, result);

    result->value.function = value;

    result->docstr = octaspire_dern_vm_create_new_value_string_from_c_string(self, docstr);

    result->docvec = docVec ? octaspire_dern_vm_create_new_value_vector_from_vector(self, docVec) : 0;

    octaspire_dern_vm_pop_value(self, result);
    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_special(
    octaspire_dern_vm_t *self,
    octaspire_dern_special_t * const value,
    char const * const docstr)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(self);

    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(self, OCTASPIRE_DERN_VALUE_TAG_SPECIAL);
    result->value.special = value;

    octaspire_dern_vm_push_value(self, result);

    result->docstr = octaspire_dern_vm_create_new_value_string_from_c_string(self, docstr);

    octaspire_dern_vm_pop_value(self, result);
    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_create_new_value_builtin(
    octaspire_dern_vm_t *self,
    octaspire_dern_builtin_t * const value,
    char const * const docstr)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(self);

    octaspire_dern_value_t *result = octaspire_dern_vm_private_create_new_value_struct(self, OCTASPIRE_DERN_VALUE_TAG_BUILTIN);
    result->value.builtin = value;

    octaspire_dern_vm_push_value(self, result);

    result->docstr = octaspire_dern_vm_create_new_value_string_from_c_string(self, docstr);

    octaspire_dern_vm_pop_value(self, result);

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
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
        break;

        case OCTASPIRE_DERN_VALUE_TAG_NIL:         break;
        case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:     break;
        case OCTASPIRE_DERN_VALUE_TAG_INTEGER:     break;
        case OCTASPIRE_DERN_VALUE_TAG_REAL:        break;
        case OCTASPIRE_DERN_VALUE_TAG_STRING:
        {
            octaspire_container_utf8_string_release(value->value.string);
            value->value.string = 0;
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
        {
            octaspire_container_utf8_string_release(value->value.character);
            value->value.character = 0;
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_SYMBOL:
        {
            octaspire_container_utf8_string_release(value->value.symbol);
            value->value.symbol = 0;
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_ERROR:
        {
            octaspire_container_utf8_string_release(value->value.error);
            value->value.error = 0;
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
        {
            // Elements are NOT released here, because it would lead to double free.
            // GC releases the elements (those are stored in the all-vector also).
            octaspire_container_vector_clear(value->value.vector);
            octaspire_container_vector_release(value->value.vector);
            value->value.vector = 0;
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_HASH_MAP:
        {
            // Elements are NOT released here, because it would lead to double free.
            // GC releases the elements (those are stored in the all-vector also).
            octaspire_container_hash_map_clear(value->value.hashMap);
            octaspire_container_hash_map_release(value->value.hashMap);
            value->value.hashMap = 0;
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

    octaspire_memory_allocator_free(self->allocator, value);
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
    /*
    octaspire_helpers_verify(self->globalEnvironment);
    // TODO XXX global env need not be in the stack anymore
    if (self->globalEnvironment)
    {
        octaspire_dern_vm_private_mark(self, self->globalEnvironment);
    }
    */

    size_t const stackLength = octaspire_dern_vm_get_stack_length(self);

    // Stack seems to grow during iter. Maybe push without pop somewhere?
    for (size_t i = 0; i < octaspire_container_vector_get_length(self->stack); ++i)
    {
        octaspire_dern_value_t *value = octaspire_container_vector_get_element_at(self->stack, i);

        if (!octaspire_dern_vm_private_mark(self, value))
        {
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
            octaspire_helpers_verify(false);
            return false;
        }
    }

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
    return true;
}

bool octaspire_dern_vm_private_mark(octaspire_dern_vm_t *self, octaspire_dern_value_t *value)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(self);
    return octaspire_dern_value_mark(value);
}

bool octaspire_dern_vm_private_sweep(octaspire_dern_vm_t *self)
{
    for (size_t i = 0; i < octaspire_container_vector_get_length(self->all); /* NOP */ )
    {
        octaspire_dern_value_t *value = octaspire_container_vector_get_element_at(self->all, i);

        if (value->mark)
        {
            value->mark = false;

            ++i;
        }
        else
        {
            octaspire_dern_vm_private_release_value(self, value);

            octaspire_dern_value_t *value2 =
                octaspire_container_vector_get_element_at(self->all, i);

            octaspire_helpers_verify(value == value2);
            if (!octaspire_container_vector_remove_element_at(self->all, i))
            {
                abort();
            }
        }
    }

    return true;
}

octaspire_dern_value_t *octaspire_dern_vm_private_parse_token(
    octaspire_dern_vm_t * const self,
    octaspire_dern_lexer_token_t const * const token,
    octaspire_input_t *input)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(self);

    octaspire_dern_value_t *result = 0;

    if (!token)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
        return result;
    }

    switch (octaspire_dern_lexer_token_get_type_tag(token))
    {
        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_LPAREN:
        {
            octaspire_container_vector_t *vec = octaspire_container_vector_new(
                sizeof(octaspire_dern_value_t*),
                true,
                0,
                self->allocator);

            if (!vec)
            {
                result = octaspire_dern_vm_create_new_value_error(
                    self,
                    octaspire_container_utf8_string_new("Allocation failure in private parse_token", self->allocator));
            }
            else
            {
                result = octaspire_dern_vm_create_new_value_vector_from_vector(self, vec);

                // Protect result (and all values inside it) from the garbage collector during
                // this phase.
                octaspire_dern_vm_push_value(self, result);

                bool error = false;
                octaspire_dern_lexer_token_t *token2 = 0;
                while (true)
                {
                    octaspire_dern_lexer_token_release(token2);
                    token2 = 0;
                    octaspire_helpers_verify(token2 == 0);

                    token2 = octaspire_dern_lexer_pop_next_token(input, self->allocator);

                    if (!token2)
                    {
                        // No more input
                        octaspire_dern_vm_pop_value(self, result);
                        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
                        return 0;
                    }
                    else if (octaspire_dern_lexer_token_get_type_tag(token2) == OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR)
                    {
                        error = true;
                        octaspire_dern_vm_pop_value(self, result);

                        result = octaspire_dern_vm_create_new_value_error(
                            self,
                            octaspire_container_utf8_string_new(
                                octaspire_dern_lexer_token_get_error_value_as_c_string(token2),
                                self->allocator));

                        octaspire_helpers_verify(result);

                        octaspire_dern_lexer_token_release(token2);
                        token2 = 0;

                        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
                        return result;
                    }
                    else
                    {
                        if (octaspire_dern_lexer_token_get_type_tag(token2) == OCTASPIRE_DERN_LEXER_TOKEN_TAG_RPAREN)
                        {
                            octaspire_dern_vm_pop_value(self, result);

                            octaspire_dern_lexer_token_release(token2);
                            token2 = 0;

                            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
                            return result;
                        }
                        else
                        {
                            octaspire_dern_value_t *element =
                                octaspire_dern_vm_private_parse_token(self, token2, input);

                            //octaspire_helpers_verify(element);

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
                            if (!octaspire_container_vector_push_back_element(result->value.vector, &element))
                            {
                                abort();
                            }
                        }
                    }
                }

                if (!error)
                {
                    octaspire_dern_vm_pop_value(self, result);
                }

                octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
                return result;
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

                octaspire_helpers_verify(quoteSym);

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
                octaspire_container_utf8_string_new(
                    octaspire_dern_lexer_token_get_string_value_as_c_string(token),
                    self->allocator));
        }
        break;

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_CHARACTER:
        {
            result = octaspire_dern_vm_create_new_value_character(
                self,
                octaspire_container_utf8_string_new(
                    octaspire_dern_lexer_token_get_character_value_as_c_string(token),
                    self->allocator));
        }
        break;

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_SYMBOL:
        {
            result = octaspire_dern_vm_create_new_value_symbol(
                self,
                octaspire_container_utf8_string_new(
                    octaspire_dern_lexer_token_get_symbol_value_as_c_string(token),
                    self->allocator));
        }
        break;

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR:
        {
            result = octaspire_dern_vm_create_new_value_error(
                self,
                octaspire_container_utf8_string_new(
                    octaspire_dern_lexer_token_get_error_value_as_c_string(token),
                    self->allocator));
        }
        break;

        // TODO XXX add rest of types
        default:
            octaspire_helpers_verify(false);
            break;
    }

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_parse(octaspire_dern_vm_t *self, octaspire_input_t *input)
{
    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, self->allocator);

    octaspire_dern_value_t *result = octaspire_dern_vm_private_parse_token(self, token, input);

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

    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    if (!value)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
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
        break;

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
                octaspire_container_utf8_string_t* str = octaspire_dern_value_to_string(
                    value,
                    self->allocator);

                result = octaspire_dern_vm_create_new_value_error(
                        self,
                        octaspire_container_utf8_string_new_format(
                            self->allocator,
                            "Unbound symbol '%s'",
                            octaspire_container_utf8_string_get_c_string(str)));

                octaspire_container_utf8_string_release(str);
                str = 0;
            }
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
        {
            octaspire_container_vector_t *vec = value->value.vector;

            if (octaspire_container_vector_is_empty(vec))
            {
                result = octaspire_dern_vm_create_new_value_error(
                    self,
                    octaspire_container_utf8_string_new(
                        "Cannot evaluate empty vector '()'",
                        self->allocator));
            }

            octaspire_dern_value_t *removeMe = octaspire_container_vector_get_element_at(vec, 0);

            octaspire_dern_value_t *operator = octaspire_dern_vm_eval(
                self,
                removeMe,
                environment);

            if (!operator)
            {
                octaspire_dern_vm_pop_value(self, environment);
                octaspire_dern_vm_pop_value(self, value);
                octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
                return 0;
            }

            octaspire_dern_vm_push_value(self, operator);

            switch (operator->typeTag)
            {
                case OCTASPIRE_DERN_VALUE_TAG_SPECIAL:
                {
                    octaspire_container_vector_t *argVec =
                        octaspire_container_vector_new_with_preallocated_elements(
                            sizeof(octaspire_dern_value_t*),
                            true,
                            octaspire_container_vector_get_length(vec) - 1,
                            0,
                            self->allocator);

                    octaspire_dern_value_t *arguments =
                        octaspire_dern_vm_create_new_value_vector_from_vector(self, argVec);

                    octaspire_dern_vm_push_value(self, arguments);

                    for (size_t i = 1; i < octaspire_container_vector_get_length(vec); ++i)
                    {
                        octaspire_dern_value_t * const tmpPtr = octaspire_container_vector_get_element_at(vec, i);
                        if (tmpPtr->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                        {
                            result = tmpPtr;
                            break;
                        }

                        octaspire_container_vector_push_back_element(argVec, &tmpPtr);
                    }

                    if (!result)
                    {
                        result = (operator->value.special->cFunction)(self, arguments, environment);

                        // TODO XXX add this error annoation to other places to
                        // (for example builtin and function calls)
                        if (result->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                        {
                            octaspire_container_utf8_string_t *tmpStr =
                                octaspire_dern_value_to_string(value, self->allocator);
                            
                            octaspire_container_utf8_string_concatenate_format(
                                result->value.string,
                                " At form:\n%s",
                                octaspire_container_utf8_string_get_c_string(tmpStr));

                            octaspire_container_utf8_string_release(tmpStr);
                            tmpStr = 0;
                        }
                    }

                    octaspire_dern_vm_pop_value(self, arguments);
                }
                break;

                case OCTASPIRE_DERN_VALUE_TAG_BUILTIN:
                {
                    octaspire_container_vector_t *argVec =
                        octaspire_container_vector_new_with_preallocated_elements(
                            sizeof(octaspire_dern_value_t*),
                            true,
                            octaspire_container_vector_get_length(vec) - 1,
                            0,
                            self->allocator);

                    octaspire_dern_value_t *arguments =
                        octaspire_dern_vm_create_new_value_vector_from_vector(self, argVec);

                    octaspire_dern_vm_push_value(self, arguments);

                    for (size_t i = 1; i < octaspire_container_vector_get_length(vec); ++i)
                    {
                        octaspire_dern_value_t *evaluated = octaspire_dern_vm_eval(
                            self,
                            octaspire_container_vector_get_element_at(vec, i),
                            environment);

                        if (evaluated->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                        {
                            result = evaluated;
                            break;
                        }

                        octaspire_container_vector_push_back_element(argVec, &evaluated);
                    }

                    if (!result)
                    {
                        result = (operator->value.builtin->cFunction)(self, arguments, environment);

                        if (operator->value.builtin->cFunction == octaspire_dern_vm_builtin_return)
                        {
                            //octaspire_helpers_verify(self->functionReturn == 0);
                            self->functionReturn = result;
                        }
                    }

                    octaspire_dern_vm_pop_value(self, arguments);
                }
                break;

                case OCTASPIRE_DERN_VALUE_TAG_FUNCTION:
                {
                    octaspire_container_vector_t *argVec =
                        octaspire_container_vector_new_with_preallocated_elements(
                            sizeof(octaspire_dern_value_t*),
                            true,
                            octaspire_container_vector_get_length(vec) - 1,
                            0,
                            self->allocator);

                    octaspire_dern_value_t *arguments =
                        octaspire_dern_vm_create_new_value_vector_from_vector(self, argVec);

                    octaspire_dern_vm_push_value(self, arguments);

                    for (size_t i = 1; i < octaspire_container_vector_get_length(vec); ++i)
                    {
                        octaspire_dern_value_t *evaluated = octaspire_dern_vm_eval(
                            self,
                            octaspire_container_vector_get_element_at(vec, i),
                            environment);


                        if (evaluated->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                        {
                            result = evaluated;
                            break;
                        }

                        octaspire_dern_value_as_vector_push_back_element(arguments, &evaluated);
                    }

                    if (!result)
                    {
                        octaspire_dern_function_t *function = operator->value.function;

                        octaspire_helpers_verify(function);
                        octaspire_helpers_verify(function->formals);
                        // Invalid read of size 4 below
                        octaspire_helpers_verify(function->formals->value.vector);
                        octaspire_helpers_verify(function->body);
                        octaspire_helpers_verify(function->body->value.vector);
                        octaspire_helpers_verify(function->definitionEnvironment);
                        octaspire_helpers_verify(function->definitionEnvironment->value.environment);

                        octaspire_dern_environment_t *extendedEnvironment =
                            octaspire_dern_environment_new(
                                function->definitionEnvironment,
                                self,
                                self->allocator);

                        octaspire_helpers_verify(extendedEnvironment);

                        octaspire_dern_value_t *extendedEnvVal =
                            octaspire_dern_vm_create_new_value_environment_from_environment(self, extendedEnvironment);

                        octaspire_helpers_verify(extendedEnvVal);

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

                            octaspire_helpers_verify(function->body->typeTag == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
                            octaspire_helpers_verify(function->body->value.vector);

                            for (size_t i = 0; i < octaspire_container_vector_get_length(function->body->value.vector); ++i)
                            {
                                octaspire_dern_value_t *toBeEvaluated =
                                    octaspire_container_vector_get_element_at(
                                        function->body->value.vector,
                                        i);

                                octaspire_dern_vm_push_value(self, toBeEvaluated);

                                result = octaspire_dern_vm_eval(
                                    self,
                                    toBeEvaluated,
                                    extendedEnvVal);

                                if (result->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                                {
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

                default:
                {
                    octaspire_container_utf8_string_t *str = octaspire_dern_value_to_string(
                        operator,
                        self->allocator);

                    result = octaspire_dern_vm_create_new_value_error(
                        self,
                        octaspire_container_utf8_string_new_format(
                            self->allocator,
                            "Cannot evaluate operator of type '%s' (%s)",
                            octaspire_dern_value_helper_get_type_as_c_string(operator->typeTag),
                            octaspire_container_utf8_string_get_c_string(str)));

                    octaspire_container_utf8_string_release(str);
                    str = 0;
                }
                break;
            }

            octaspire_dern_vm_pop_value(self, operator);
        }
        break;

        // TODO XXX add rest of types
        default:
        {
            result = octaspire_dern_vm_create_new_value_error(
                self,
                octaspire_container_utf8_string_new_format(
                    self->allocator,
                    "Cannot evaluate unknown type %i",
                    (int)value->typeTag));
        }
        break;
    }

    octaspire_dern_vm_pop_value(self, environment);
    octaspire_dern_vm_pop_value(self, value);


    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));

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
    return octaspire_dern_vm_read_from_buffer_and_eval_in_global_environment(self, str, strlen(str));
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
        return octaspire_dern_vm_create_new_value_error_from_c_string(self, "Allocation failure of input");
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
            break;
        }
    }

    octaspire_input_release(input);
    input = 0;

    if (!result && lastGoodResult)
    {
        return lastGoodResult;
    }

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

    octaspire_memory_allocator_free(self->allocator, buffer);
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

octaspire_memory_allocator_t *octaspire_dern_vm_get_allocator(
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
    octaspire_dern_environment_t * const targetEnv)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(self);

    octaspire_dern_builtin_t * const builtin =
        octaspire_dern_builtin_new(funcPointer, self->allocator, numRequiredActualArguments);

    if (!builtin)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
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
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
        return false;
    }

    octaspire_dern_vm_pop_value(self, builtinVal);
    octaspire_dern_vm_pop_value(self, symbol);

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));

    return true;
}

bool octaspire_dern_vm_create_and_register_new_special(
    octaspire_dern_vm_t * const self,
    char const * const name,
    octaspire_dern_c_function const funcPointer,
    size_t const numRequiredActualArguments,
    char const * const docStr,
    octaspire_dern_environment_t * const targetEnv)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(self);

    octaspire_dern_special_t * const special =
        octaspire_dern_special_new(funcPointer, self->allocator, numRequiredActualArguments);

    if (!special)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
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
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
        return false;
    }

    octaspire_dern_vm_pop_value(self, specialVal);
    octaspire_dern_vm_pop_value(self, symbol);
    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
    return true;
}

size_t octaspire_dern_vm_get_stack_length(
    octaspire_dern_vm_t const * const self)
{
    return octaspire_container_vector_get_length(self->stack);
}

void octaspire_dern_vm_print_stack(
    octaspire_dern_vm_t const * const self)
{
    if (octaspire_container_vector_is_empty(self->stack))
    {
        printf("\n\n-- STACK IS EMPTY --\n\n");
        return;
    }

    printf("Stack has %zu elements\n", octaspire_container_vector_get_length(self->stack));
    for (ptrdiff_t i = (ptrdiff_t)octaspire_container_vector_get_length(self->stack) - 1; i >= 0; --i)
    {
        printf("--------------------------- #%zu ------------------------\n", i);
        octaspire_dern_value_print(
            octaspire_container_vector_get_element_at(self->stack, i),
            self->allocator);
        printf("--------------------------- end ------------------------\n\n");
    }
}


octaspire_dern_value_t *octaspire_dern_vm_helper_create_new_value_vector_of_integers_from_vector_of_size_t(
    octaspire_dern_vm_t * const self,
    octaspire_container_vector_t const * const vectorContainingSizeTs)
{
    octaspire_helpers_verify(
        self &&
        vectorContainingSizeTs &&
        sizeof(size_t) == octaspire_container_vector_get_element_size_in_octets(vectorContainingSizeTs));

    size_t const stackLength = octaspire_dern_vm_get_stack_length(self);

    octaspire_dern_value_t * const result = octaspire_dern_vm_create_new_value_vector(self);
    if (!octaspire_dern_vm_push_value(self, result))
    {
        abort();
    }

    for (size_t i = 0; i < octaspire_container_vector_get_length(vectorContainingSizeTs); ++i)
    {
        size_t const idx = *(size_t*)octaspire_container_vector_get_element_at_const(vectorContainingSizeTs, i);

        octaspire_dern_value_t *tmpVal =
            octaspire_dern_vm_create_new_value_integer(self, idx);

        octaspire_helpers_verify(tmpVal);

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

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_find_from_value(
    octaspire_dern_vm_t * const self,
    octaspire_dern_value_t * const value,
    octaspire_dern_value_t const * const key)
{
    octaspire_helpers_verify(self && value && key);
    size_t const stackLength = octaspire_dern_vm_get_stack_length(self);

    switch (value->typeTag)
    {
        case OCTASPIRE_DERN_VALUE_TAG_STRING:
        {
            if (key->typeTag == OCTASPIRE_DERN_VALUE_TAG_CHARACTER)
            {
                octaspire_container_vector_t *foundIndices = octaspire_container_utf8_string_find_char(
                    value->value.string,
                    key->value.character,
                    0);

                octaspire_dern_value_t * const result =
                    octaspire_dern_vm_helper_create_new_value_vector_of_integers_from_vector_of_size_t(
                        self,
                        foundIndices);

                octaspire_helpers_verify(result);

                octaspire_container_vector_release(foundIndices);
                foundIndices = 0;

                octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
                return result;
            }
            else if (key->typeTag == OCTASPIRE_DERN_VALUE_TAG_STRING)
            {
                octaspire_container_vector_t *foundIndices = octaspire_container_utf8_string_find_string(
                    value->value.string,
                    key->value.string,
                    0,
                    octaspire_container_utf8_string_get_length_in_ucs_characters(key->value.string));

                octaspire_dern_value_t * const result =
                    octaspire_dern_vm_helper_create_new_value_vector_of_integers_from_vector_of_size_t(
                        self,
                        foundIndices);

                octaspire_helpers_verify(result);

                octaspire_container_vector_release(foundIndices);
                foundIndices = 0;

                octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
                return result;
            }
            else if (key->typeTag == OCTASPIRE_DERN_VALUE_TAG_SYMBOL)
            {
                octaspire_container_vector_t *foundIndices = octaspire_container_utf8_string_find_string(
                    value->value.string,
                    key->value.symbol,
                    0,
                    octaspire_container_utf8_string_get_length_in_ucs_characters(key->value.symbol));

                octaspire_dern_value_t * const result =
                    octaspire_dern_vm_helper_create_new_value_vector_of_integers_from_vector_of_size_t(
                        self,
                        foundIndices);

                octaspire_helpers_verify(result);

                octaspire_container_vector_release(foundIndices);
                foundIndices = 0;

                octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
                return result;
            }
            else if (key->typeTag == OCTASPIRE_DERN_VALUE_TAG_INTEGER)
            {
                octaspire_container_utf8_string_t *tmpStr =
                    octaspire_container_utf8_string_new_format(
                        self->allocator,
                        "%" PRId32 "",
                        key->value.integer);

                octaspire_container_vector_t *foundIndices = octaspire_container_utf8_string_find_string(
                    value->value.string,
                    tmpStr,
                    0,
                    octaspire_container_utf8_string_get_length_in_ucs_characters(tmpStr));

                octaspire_container_utf8_string_release(tmpStr);
                tmpStr = 0;

                octaspire_dern_value_t * const result =
                    octaspire_dern_vm_helper_create_new_value_vector_of_integers_from_vector_of_size_t(
                        self,
                        foundIndices);

                octaspire_helpers_verify(result);

                octaspire_container_vector_release(foundIndices);
                foundIndices = 0;

                octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
                return result;
            }
            else if (key->typeTag == OCTASPIRE_DERN_VALUE_TAG_REAL)
            {
                octaspire_container_utf8_string_t *tmpStr =
                    octaspire_container_utf8_string_new_format(
                        self->allocator,
                        "%g",
                        key->value.real);

                octaspire_container_vector_t *foundIndices = octaspire_container_utf8_string_find_string(
                    value->value.string,
                    tmpStr,
                    0,
                    octaspire_container_utf8_string_get_length_in_ucs_characters(tmpStr));

                octaspire_dern_value_t * const result =
                    octaspire_dern_vm_helper_create_new_value_vector_of_integers_from_vector_of_size_t(
                        self,
                        foundIndices);

                octaspire_helpers_verify(result);

                octaspire_container_vector_release(foundIndices);
                foundIndices = 0;

                octaspire_container_utf8_string_release(tmpStr);
                tmpStr = 0;

                octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
                return result;
            }
            else
            {
                octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
                return octaspire_dern_vm_create_new_value_error_format(
                    self,
                    "Type '%s' cannot be searched from type 'string'",
                    octaspire_dern_value_helper_get_type_as_c_string(key->typeTag));
            }
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_SYMBOL:
        {
            if (key->typeTag == OCTASPIRE_DERN_VALUE_TAG_CHARACTER)
            {
                octaspire_container_vector_t *foundIndices = octaspire_container_utf8_string_find_char(
                    value->value.symbol,
                    key->value.character,
                    0);

                octaspire_dern_value_t * const result =
                    octaspire_dern_vm_helper_create_new_value_vector_of_integers_from_vector_of_size_t(
                        self,
                        foundIndices);

                octaspire_helpers_verify(result);

                octaspire_container_vector_release(foundIndices);
                foundIndices = 0;

                octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
                return result;
            }
            else if (key->typeTag == OCTASPIRE_DERN_VALUE_TAG_STRING)
            {
                octaspire_container_vector_t *foundIndices = octaspire_container_utf8_string_find_string(
                    value->value.symbol,
                    key->value.string,
                    0,
                    octaspire_container_utf8_string_get_length_in_ucs_characters(key->value.string));

                octaspire_dern_value_t * const result =
                    octaspire_dern_vm_helper_create_new_value_vector_of_integers_from_vector_of_size_t(
                        self,
                        foundIndices);

                octaspire_helpers_verify(result);

                octaspire_container_vector_release(foundIndices);
                foundIndices = 0;

                octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
                return result;
            }
            else if (key->typeTag == OCTASPIRE_DERN_VALUE_TAG_SYMBOL)
            {
                octaspire_container_vector_t *foundIndices = octaspire_container_utf8_string_find_string(
                    value->value.symbol,
                    key->value.symbol,
                    0,
                    octaspire_container_utf8_string_get_length_in_ucs_characters(key->value.symbol));

                octaspire_dern_value_t * const result =
                    octaspire_dern_vm_helper_create_new_value_vector_of_integers_from_vector_of_size_t(
                        self,
                        foundIndices);

                octaspire_helpers_verify(result);

                octaspire_container_vector_release(foundIndices);
                foundIndices = 0;

                octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
                return result;
            }
            else if (key->typeTag == OCTASPIRE_DERN_VALUE_TAG_INTEGER)
            {
                octaspire_container_utf8_string_t *tmpStr =
                    octaspire_container_utf8_string_new_format(
                        self->allocator,
                        "%" PRId32 "",
                        key->value.integer);

                octaspire_container_vector_t *foundIndices = octaspire_container_utf8_string_find_string(
                    value->value.symbol,
                    tmpStr,
                    0,
                    octaspire_container_utf8_string_get_length_in_ucs_characters(tmpStr));

                octaspire_dern_value_t * const result =
                    octaspire_dern_vm_helper_create_new_value_vector_of_integers_from_vector_of_size_t(
                        self,
                        foundIndices);

                octaspire_helpers_verify(result);

                octaspire_container_vector_release(foundIndices);
                foundIndices = 0;

                octaspire_container_utf8_string_release(tmpStr);
                tmpStr = 0;

                octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
                return result;
            }
            else if (key->typeTag == OCTASPIRE_DERN_VALUE_TAG_REAL)
            {
                octaspire_container_utf8_string_t *tmpStr =
                    octaspire_container_utf8_string_new_format(
                        self->allocator,
                        "%g",
                        key->value.real);

                octaspire_container_vector_t *foundIndices = octaspire_container_utf8_string_find_string(
                    value->value.symbol,
                    tmpStr,
                    0,
                    octaspire_container_utf8_string_get_length_in_ucs_characters(tmpStr));

                octaspire_dern_value_t * const result =
                    octaspire_dern_vm_helper_create_new_value_vector_of_integers_from_vector_of_size_t(
                        self,
                        foundIndices);

                octaspire_helpers_verify(result);

                octaspire_container_vector_release(foundIndices);
                foundIndices = 0;

                octaspire_container_utf8_string_release(tmpStr);
                tmpStr = 0;

                octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
                return result;
            }
            else
            {
                octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
                return octaspire_dern_vm_create_new_value_error_format(
                    self,
                    "Type '%s' cannot be searched from type 'symbol'",
                    octaspire_dern_value_helper_get_type_as_c_string(key->typeTag));
            }
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
        {
            octaspire_dern_value_t *result = octaspire_dern_vm_create_new_value_vector(self);

            octaspire_dern_vm_push_value(self, result);

            for (size_t i = 0; i < octaspire_container_vector_get_length(value->value.vector); ++i)
            {
                octaspire_dern_value_t * const element =
                    octaspire_container_vector_get_element_at(value->value.vector, i);

                if (octaspire_dern_value_is_equal(element, key))
                {
                    octaspire_dern_value_t *tmpVal = octaspire_dern_vm_create_new_value_integer(self, i);
                    octaspire_dern_value_as_vector_push_back_element(result, &tmpVal);
                }
            }

            octaspire_dern_vm_pop_value(self, result);
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
            return result;
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_HASH_MAP:
        {
            uint32_t const hash = octaspire_dern_value_get_hash(key);

            octaspire_container_hash_map_element_t * const element =
                octaspire_container_hash_map_get(value->value.hashMap, hash, &key);

            if (element)
            {
                octaspire_dern_value_t * const resVal =
                    octaspire_container_hash_map_element_get_value(element);

                if (resVal)
                {
                    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
                    return resVal;
                }
            }

            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
            return octaspire_dern_vm_get_value_nil(self);
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT:
        {
            octaspire_dern_value_t *result =
                octaspire_dern_environment_get(value->value.environment, key);

            if (result)
            {
                octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
                return result;
            }

            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
            return octaspire_dern_vm_get_value_nil(self);
        }
        break;

        default:
        {
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(self));
            return octaspire_dern_vm_create_new_value_error_format(
                self,
                "'find' doesn't support search from type '%s'",
                octaspire_dern_value_helper_get_type_as_c_string(value->typeTag));
        }
        break;
    }
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

