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
#include "octaspire/dern/octaspire_dern_value.h"
#include <assert.h>
#include <string.h>
#include <inttypes.h>

#ifndef OCTASPIRE_DERN_DO_NOT_USE_AMALGAMATED_CORE
    #include "octaspire-core-amalgamated.c"
#else
    #include <octaspire/core/octaspire_string.h>
    #include <octaspire/core/octaspire_vector.h>
    #include <octaspire/core/octaspire_helpers.h>
#endif

#include "octaspire/dern/octaspire_dern_environment.h"
#include "octaspire/dern/octaspire_dern_vm.h"
#include "octaspire/dern/octaspire_dern_port.h"
#include "octaspire/dern/octaspire_dern_helpers.h"

static char const * const octaspire_dern_value_helper_type_tags_as_c_strings[] =
{
    "illegal",
    "nil",
    "boolean",
    "integer",
    "real",
    "string",
    "character",
    "symbol",
    "error",
    "vector",
    "hash map",
    "queue",
    "list",
    "environment",
    "function",
    "special",
    "builtin",
    "port",
    "C data",
    "semver"
};

static octaspire_string_t *octaspire_dern_function_private_is_string_in_vector(
    octaspire_allocator_t *allocator,
    octaspire_string_t const * const str,
    octaspire_vector_t const * const vec);

static octaspire_string_t *octaspire_dern_private_value_to_string(
    octaspire_dern_value_t const * const self,
    bool const plain,
    octaspire_allocator_t * const allocator);

static int octaspire_dern_value_private_compare_void_pointers(
    void const * const a,
    void const * const b);



octaspire_dern_function_t *octaspire_dern_function_new(
    struct octaspire_dern_value_t *formals,
    struct octaspire_dern_value_t *body,
    struct octaspire_dern_value_t *definitionEnvironment,
    octaspire_allocator_t  *allocator)
{
    octaspire_dern_function_t *self =
        octaspire_allocator_malloc(allocator, sizeof(octaspire_dern_function_t));

    if (!self)
    {
        return 0;
    }

    self->name                  = octaspire_string_new("",   allocator);
    self->docstr                = octaspire_string_new("", allocator);
    self->howtoAllowed          = false;
    self->formals               = formals;
    self->body                  = body;
    self->definitionEnvironment = definitionEnvironment;
    self->allocator             = allocator;

    return self;
}

octaspire_dern_function_t *octaspire_dern_function_new_copy(
    octaspire_dern_function_t const * const other,
    octaspire_dern_vm_t * const vm,
    octaspire_allocator_t  *allocator)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_dern_function_t *self =
        octaspire_allocator_malloc(allocator, sizeof(octaspire_dern_function_t));

    if (!self)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return 0;
    }

    self->name
        = octaspire_string_new_copy(other->name, allocator);

    self->docstr
        = octaspire_string_new_copy(other->docstr, allocator);

    self->howtoAllowed = other->howtoAllowed;

    self->formals =
        octaspire_dern_vm_create_new_value_copy(vm, other->formals);

    octaspire_dern_vm_push_value(vm, self->formals);

    self->body =
        octaspire_dern_vm_create_new_value_copy(vm, other->body);

    octaspire_dern_vm_push_value(vm, self->body);

    self->definitionEnvironment =
        octaspire_dern_vm_create_new_value_copy(vm, other->definitionEnvironment);

    octaspire_dern_vm_push_value(vm, self->definitionEnvironment);

    self->allocator             = allocator;

    octaspire_dern_vm_pop_value(vm, self->definitionEnvironment);
    octaspire_dern_vm_pop_value(vm, self->body);
    octaspire_dern_vm_pop_value(vm, self->formals);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return self;
}

void octaspire_dern_function_release(octaspire_dern_function_t *self)
{
    if (!self)
    {
        return;
    }

    octaspire_string_release(self->name);
    self->name = 0;

    octaspire_string_release(self->docstr);
    self->docstr = 0;

    octaspire_allocator_free(self->allocator, self);
}

bool octaspire_dern_function_is_equal(
    octaspire_dern_function_t const * const self,
    octaspire_dern_function_t const * const other)
{
    return (octaspire_dern_function_compare(self, other) == 0);
}

int octaspire_dern_function_compare(
    octaspire_dern_function_t const * const self,
    octaspire_dern_function_t const * const other)
{
    if (self == other)
    {
        return 0;
    }

    int cmp = octaspire_string_compare(self->name, other->name);

    if (cmp)
    {
        return cmp;
    }

    cmp = octaspire_string_compare(self->docstr, other->docstr);

    if (cmp)
    {
        return cmp;
    }

    cmp = octaspire_dern_value_compare(self->formals, other->formals);

    if (cmp)
    {
        return cmp;
    }

    cmp = octaspire_dern_value_compare(self->body, other->body);

    if (cmp)
    {
        return cmp;
    }

    cmp = octaspire_dern_value_compare(
        self->definitionEnvironment,
        other->definitionEnvironment);

    if (cmp)
    {
        return cmp;
    }

    return (self->howtoAllowed - other->howtoAllowed);
}

bool octaspire_dern_function_set_howto_data(
    octaspire_dern_function_t * const self,
    char const * const name,
    char const * const docstr,
    bool const howtoAllowed)
{
    octaspire_helpers_verify_not_null(self);
    octaspire_helpers_verify_not_null(self->name);
    octaspire_helpers_verify_not_null(self->docstr);

    if (!octaspire_string_set_from_c_string(self->name, name))
    {
        return false;
    }

    if (!octaspire_string_set_from_c_string(self->docstr, docstr))
    {
        return false;
    }

    self->howtoAllowed  = howtoAllowed;

    return true;
}

size_t octaspire_dern_function_get_number_of_required_arguments(
    octaspire_dern_function_t const * const self)
{
    octaspire_helpers_verify_not_null(self);
    octaspire_helpers_verify_not_null(self->formals);
    octaspire_helpers_verify_true(self->formals->typeTag == OCTASPIRE_DERN_VALUE_TAG_VECTOR);

    size_t const numFormalArgs = octaspire_vector_get_length(
        self->formals->value.vector);

    size_t numNormalArgs         = 0;

    for (size_t i = 0; i < numFormalArgs; ++i)
    {
        octaspire_dern_value_t *formal = octaspire_vector_get_element_at(
            self->formals->value.vector,
            (ptrdiff_t)i);

        octaspire_helpers_verify_true(formal->typeTag == OCTASPIRE_DERN_VALUE_TAG_SYMBOL);

        octaspire_string_t const * const formalAsStr = formal->value.string;

        if (octaspire_string_is_equal_to_c_string(formalAsStr, "."))
        {
            return numNormalArgs;
        }

        ++numNormalArgs;
    }

    return numNormalArgs;
}

octaspire_string_t *octaspire_dern_function_private_is_string_in_vector(
    octaspire_allocator_t *allocator,
    octaspire_string_t const * const str,
    octaspire_vector_t const * const vec)
{
    octaspire_helpers_verify_true(str && vec);

    // (symbol docstrforit symbol docstrforit symbol docstrforit ...)

    for (size_t i = 0; i < octaspire_vector_get_length(vec); i += 2)
    {
        octaspire_dern_value_t const * const symbol =
            octaspire_vector_get_element_at_const(vec, (ptrdiff_t)i);

        octaspire_helpers_verify_true(symbol && symbol->typeTag == OCTASPIRE_DERN_VALUE_TAG_SYMBOL);

        if (octaspire_string_is_equal(str, symbol->value.symbol))
        {
            if ((i+1) >= octaspire_vector_get_length(vec))
            {
                // No docstring for the formal
                return octaspire_string_new_format(
                    allocator,
                    "formal '%s' doesn't have docstring in docvec",
                    octaspire_string_get_c_string(str));
            }

            octaspire_dern_value_t const * const docstr =
                octaspire_vector_get_element_at_const(
                    vec,
                    (ptrdiff_t)(i + 1));

            if (!docstr)
            {
                abort();
            }
            else if (docstr->typeTag != OCTASPIRE_DERN_VALUE_TAG_STRING)
            {
                return octaspire_string_new_format(
                    allocator,
                    "type of docstring for formal '%s' is not string. It has type %s",
                    octaspire_string_get_c_string(str),
                    octaspire_dern_value_helper_get_type_as_c_string(docstr->typeTag));
            }

            if (octaspire_string_is_equal_to_c_string(symbol->value.symbol, "..."))
            {
                if (!octaspire_string_is_equal_to_c_string(
                    docstr->value.string,
                    "varargs"))
                {
                    return octaspire_string_new_format(
                        allocator,
                        "docstring for ... should be varargs, not it is '%s'",
                        octaspire_string_get_c_string(docstr->value.string));
                }
            }

            return 0;
        }
    }

    return octaspire_string_new_format(
        allocator,
        "formal '%s' is not mentioned in docvec",
        octaspire_string_get_c_string(str));
}

octaspire_string_t *octaspire_dern_function_are_all_formals_mentioned_in_docvec(
    octaspire_dern_function_t const * const self,
    octaspire_dern_value_t const * const docvec)
{
    octaspire_helpers_verify_true(
        docvec &&
        docvec->typeTag == OCTASPIRE_DERN_VALUE_TAG_VECTOR);

    octaspire_helpers_verify_true(
        self &&
        self->formals && self->formals->typeTag == OCTASPIRE_DERN_VALUE_TAG_VECTOR);

    octaspire_string_t *result =
        octaspire_string_new("", self->allocator);

    if (!result)
    {
        return octaspire_string_new("Allocation failure", self->allocator);
    }

    for (size_t i = 0; i < octaspire_dern_value_get_length(self->formals); ++i)
    {
        octaspire_dern_value_t const * const formal =
            octaspire_dern_value_as_vector_get_element_at_const(
                self->formals,
                (ptrdiff_t)i);

        octaspire_helpers_verify_true(
            formal &&
            formal->typeTag == OCTASPIRE_DERN_VALUE_TAG_SYMBOL);

        octaspire_string_t *errorString =
            octaspire_dern_function_private_is_string_in_vector(
                self->allocator,
                formal->value.symbol,
                docvec->value.vector);

        if (errorString)
        {
            if (!octaspire_string_concatenate_format(
                result,
                "formal '%s' is not handled correctly in docvec: %s\n",
                octaspire_string_get_c_string(formal->value.symbol),
                octaspire_string_get_c_string(errorString)))
            {
                abort();
            }

            octaspire_string_release(errorString);
            errorString = 0;
        }
    }

    return result;
}



octaspire_dern_special_t *octaspire_dern_special_new(
    octaspire_dern_c_function const cFunction,
    octaspire_allocator_t *allocator,
    char const * const name,
    size_t const numRequiredActualArguments,
    char const * const docstr,
    bool const howtoAllowed)
{
    octaspire_dern_special_t *self =
        octaspire_allocator_malloc(allocator, sizeof(octaspire_dern_special_t));

    if (!self)
    {
        return 0;
    }

    self->cFunction                  = cFunction;
    self->allocator                  = allocator;

    self->name                       =
        octaspire_string_new(name, allocator);

    self->numRequiredActualArguments = numRequiredActualArguments;

    self->docstr                     =
        octaspire_string_new(docstr, allocator);

    self->howtoAllowed               = howtoAllowed;

    return self;
}

octaspire_dern_special_t *octaspire_dern_special_new_copy(
    octaspire_dern_special_t * const other,
    octaspire_allocator_t * const allocator)
{
    octaspire_dern_special_t *self =
        octaspire_allocator_malloc(allocator, sizeof(octaspire_dern_special_t));

    if (!self)
    {
        return 0;
    }

    self->cFunction                  = other->cFunction;
    self->allocator                  = allocator;

    self->name                       =
        octaspire_string_new_copy(other->name, allocator);

    self->numRequiredActualArguments = other->numRequiredActualArguments;

    self->docstr                     =
        octaspire_string_new_copy(other->docstr, allocator);

    self->howtoAllowed               = other->howtoAllowed;

    return self;
}

void octaspire_dern_special_release(octaspire_dern_special_t *self)
{
    if (!self)
    {
        return;
    }

    octaspire_string_release(self->name);
    self->name = 0;

    octaspire_string_release(self->docstr);
    self->docstr = 0;

    octaspire_allocator_free(self->allocator, self);
}

size_t octaspire_dern_special_get_number_of_required_arguments(
    octaspire_dern_special_t const * const self)
{
    octaspire_helpers_verify_not_null(self);
    return self->numRequiredActualArguments;
}

bool octaspire_dern_special_is_howto_allowed(
    octaspire_dern_special_t const * const self)
{
    return self->howtoAllowed;
}

octaspire_string_t *octaspire_dern_special_to_string(
    octaspire_dern_special_t const * const self,
    octaspire_allocator_t * const allocator)
{
    /*
    return octaspire_string_new_format(
        allocator,
        "<builtin %s %s>",
        octaspire_string_get_c_string(self->name),
        octaspire_string_get_c_string(self->docstr));
    */

    return octaspire_string_new_format(
        allocator,
        "%s",
        octaspire_string_get_c_string(self->name));
}



octaspire_dern_error_message_t *octaspire_dern_error_message_new(
    octaspire_allocator_t *allocator,
    char const * const message,
    size_t const lineNumber)
{
    octaspire_dern_error_message_t *self =
        octaspire_allocator_malloc(allocator,
                                   sizeof(octaspire_dern_error_message_t));

    if (!self)
    {
        return 0;
    }

    self->allocator  = allocator;
    self->message    = octaspire_string_new(message, allocator);
    self->lineNumber = lineNumber;

    return self;
}

octaspire_dern_error_message_t *octaspire_dern_error_message_new_copy(
    octaspire_dern_error_message_t * const other,
    octaspire_allocator_t * const allocator)
{
    octaspire_dern_error_message_t *self =
        octaspire_allocator_malloc(allocator,
                                   sizeof(octaspire_dern_error_message_t));

    if (!self)
    {
        return 0;
    }

    self->allocator                  = allocator;

    self->message    = octaspire_string_new_copy(other->message, allocator);
    self->lineNumber = other->lineNumber;

    return self;
}

void octaspire_dern_error_message_release(octaspire_dern_error_message_t *self)
{
    if (!self)
    {
        return;
    }

    octaspire_string_release(self->message);
    self->message = 0;

    octaspire_allocator_free(self->allocator, self);
}

int octaspire_dern_error_message_compare(
    octaspire_dern_error_message_t const * const self,
    octaspire_dern_error_message_t const * const other)
{
    int const result = octaspire_string_compare(self->message, other->message);

    if (result != 0)
    {
        return result;
    }

    return self->lineNumber - other->lineNumber;
}


octaspire_dern_builtin_t *octaspire_dern_builtin_new(
    octaspire_dern_c_function const cFunction,
    octaspire_allocator_t *allocator,
    char const * const name,
    size_t const numRequiredActualArguments,
    char const * const docstr,
    bool const howtoAllowed)
{
    octaspire_dern_builtin_t *self =
        octaspire_allocator_malloc(allocator, sizeof(octaspire_dern_builtin_t));

    if (!self)
    {
        return 0;
    }

    self->cFunction                  = cFunction;
    self->allocator                  = allocator;

    self->name                       =
        octaspire_string_new(name, allocator);

    self->numRequiredActualArguments = numRequiredActualArguments;

    self->docstr                     =
        octaspire_string_new(docstr, allocator);

    self->howtoAllowed               = howtoAllowed;

    return self;
}

octaspire_dern_builtin_t *octaspire_dern_builtin_new_copy(
    octaspire_dern_builtin_t * const other,
    octaspire_allocator_t * const allocator)
{
    octaspire_dern_builtin_t *self =
        octaspire_allocator_malloc(allocator, sizeof(octaspire_dern_builtin_t));

    if (!self)
    {
        return 0;
    }

    self->cFunction                  = other->cFunction;
    self->allocator                  = allocator;

    self->name                       =
        octaspire_string_new_copy(other->name, allocator);

    self->numRequiredActualArguments = other->numRequiredActualArguments;

    self->docstr                     =
        octaspire_string_new_copy(other->docstr, allocator);

    self->howtoAllowed               = other->howtoAllowed;

    return self;
}

void octaspire_dern_builtin_release(octaspire_dern_builtin_t *self)
{
    if (!self)
    {
        return;
    }

    octaspire_string_release(self->name);
    self->name = 0;

    octaspire_string_release(self->docstr);
    self->docstr = 0;

    octaspire_allocator_free(self->allocator, self);
}

size_t octaspire_dern_builtin_get_number_of_required_arguments(
    octaspire_dern_builtin_t const * const self)
{
    octaspire_helpers_verify_not_null(self);
    return self->numRequiredActualArguments;
}

bool octaspire_dern_builtin_is_howto_allowed(
    octaspire_dern_builtin_t const * const self)
{
    return self->howtoAllowed;
}

octaspire_string_t *octaspire_dern_builtin_to_string(
    octaspire_dern_builtin_t const * const self,
    octaspire_allocator_t * const allocator)
{
    /*
    return octaspire_string_new_format(
        allocator,
        "<special %s %s>",
        octaspire_string_get_c_string(self->name),
        octaspire_string_get_c_string(self->docstr));
    */

    return octaspire_string_new_format(
        allocator,
        "%s",
        octaspire_string_get_c_string(self->name));
}



bool octaspire_dern_function_is_howto_allowed(
    octaspire_dern_function_t const * const self)
{
    return self->howtoAllowed;
}

octaspire_string_t *octaspire_dern_function_to_string(
    octaspire_dern_function_t const * const self,
    octaspire_allocator_t * const allocator)
{
    /*
    return octaspire_string_new_format(
        allocator,
        "<function %s %s>",
        octaspire_string_get_c_string(self->name),
        octaspire_string_get_c_string(self->docstr));
    */

    return octaspire_string_new_format(
        allocator,
        "%s",
        octaspire_string_get_c_string(self->name));
}

char const * octaspire_dern_value_helper_get_type_as_c_string(
    octaspire_dern_value_tag_t const typeTag)
{
    octaspire_helpers_verify_true(typeTag <
        (octaspire_dern_value_tag_t)(sizeof(octaspire_dern_value_helper_type_tags_as_c_strings) /
         sizeof(octaspire_dern_value_helper_type_tags_as_c_strings[0])));

    return octaspire_dern_value_helper_type_tags_as_c_strings[typeTag];
}

octaspire_dern_value_tag_t octaspire_dern_value_get_type(
    octaspire_dern_value_t const * const self)
{
    octaspire_helpers_verify_not_null(self);
    return self->typeTag;
}

bool octaspire_dern_value_set(
    octaspire_dern_value_t  * const self,
    octaspire_dern_value_t  * const value)
{
    if (self == value)
    {
        return true;
    }

    octaspire_dern_vm_clear_value_to_nil(self->vm, self);

    self->typeTag = value->typeTag;

    switch (value->typeTag)
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
            self->value.boolean = value->value.boolean;
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
        {
            self->value.integer = value->value.integer;
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_REAL:
        {
            self->value.real = value->value.real;
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_STRING:
        {
            self->value.string =
                octaspire_string_new_copy(
                    value->value.string,
                    octaspire_dern_vm_get_allocator(self->vm));
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_SEMVER:
        {
            self->value.semver =
                octaspire_semver_new_copy(
                    value->value.semver,
                    octaspire_dern_vm_get_allocator(self->vm));
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
        {
            self->value.character =
                octaspire_string_new_copy(
                    value->value.character,
                    octaspire_dern_vm_get_allocator(self->vm));
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_SYMBOL:
        {
            self->value.symbol =
                octaspire_string_new_copy(
                    value->value.symbol,
                    octaspire_dern_vm_get_allocator(self->vm));
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_ERROR:
        {
            self->value.error =
                octaspire_dern_error_message_new_copy(
                    value->value.error,
                    octaspire_dern_vm_get_allocator(self->vm));
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
        {
            self->value.vector = octaspire_vector_new(
                sizeof(octaspire_dern_value_t*),
                true,
                0,
                octaspire_dern_vm_get_allocator(self->vm));

            for (size_t i = 0; i < octaspire_vector_get_length(value->value.vector); ++i)
            {
                octaspire_dern_value_t * tmpVal =
                    octaspire_vector_get_element_at(
                        value->value.vector,
                        (ptrdiff_t)i);

                if (octaspire_dern_value_is_atom(tmpVal))
                {
                    tmpVal = octaspire_dern_vm_create_new_value_copy(self->vm, tmpVal);
                }

                octaspire_dern_vm_push_value(self->vm, tmpVal);

                if (!octaspire_vector_push_back_element(self->value.vector, &tmpVal))
                {
                    abort();
                }

                octaspire_dern_vm_pop_value(self->vm, tmpVal);
            }
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_HASH_MAP:
        {
            self->value.hashMap = octaspire_map_new(
                sizeof(octaspire_dern_value_t*),
                true,
                sizeof(octaspire_dern_value_t*),
                true,
                (octaspire_map_key_compare_function_t)octaspire_dern_value_is_equal,
                (octaspire_map_key_hash_function_t)octaspire_dern_value_get_hash,
                0,
                0,
                octaspire_dern_vm_get_allocator(self->vm));

            for (size_t i = 0;
                 i < octaspire_map_get_number_of_elements(
                     value->value.hashMap);
                 ++i)
            {
                octaspire_map_element_t * const element =
                    octaspire_map_get_at_index(
                        value->value.hashMap,
                        (ptrdiff_t)i);

                octaspire_dern_value_t *key = octaspire_map_element_get_key(element);

                octaspire_dern_value_t *val =
                    octaspire_map_element_get_value(element);

                if (octaspire_dern_value_is_atom(key))
                {
                    key = octaspire_dern_vm_create_new_value_copy(self->vm, key);
                }

                octaspire_dern_vm_push_value(self->vm, key);

                if (octaspire_dern_value_is_atom(val))
                {
                    val = octaspire_dern_vm_create_new_value_copy(self->vm, val);
                }

                octaspire_dern_vm_push_value(self->vm, val);

                // Any previous value must be removed, because hash map
                // can contain multiple values per key, and we must replace, not
                // add the value.
                octaspire_map_remove(
                    self->value.hashMap,
                    octaspire_map_element_get_hash(element),
                    &key);

                if (!octaspire_map_put(
                    self->value.hashMap,
                    octaspire_map_element_get_hash(element),
                    &key,
                    &val))
                {
                    abort();
                }

                octaspire_dern_vm_pop_value(self->vm, val);
                octaspire_dern_vm_pop_value(self->vm, key);
            }
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_QUEUE:
        {
            self->value.queue = octaspire_queue_new(
                sizeof(octaspire_dern_value_t*),
                true,
                0,
                octaspire_dern_vm_get_allocator(self->vm));

            // TODO Add iterator to queue and use it here
            for (size_t i = 0;
                 i < octaspire_queue_get_length(value->value.queue);
                 ++i)
            {
                octaspire_dern_value_t * tmpVal =
                    octaspire_queue_get_at(
                        value->value.queue,
                        (ptrdiff_t)i);

                if (octaspire_dern_value_is_atom(tmpVal))
                {
                    tmpVal = octaspire_dern_vm_create_new_value_copy(self->vm, tmpVal);
                }

                octaspire_dern_vm_push_value(self->vm, tmpVal);

                if (!octaspire_queue_push(self->value.queue, &tmpVal))
                {
                    abort();
                }

                octaspire_dern_vm_pop_value(self->vm, tmpVal);
            }
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_LIST:
        {
            self->value.list = octaspire_list_new(
                sizeof(octaspire_dern_value_t*),
                true,
                0,
                octaspire_dern_vm_get_allocator(self->vm));

            // TODO more efficient iteration
            for (size_t i = 0;
                 i < octaspire_list_get_length(value->value.list);
                 ++i)
            {
                octaspire_dern_value_t * tmpVal =
                    octaspire_list_node_get_element(
                        octaspire_list_get_at(
                            value->value.list,
                            (ptrdiff_t)i));

                if (octaspire_dern_value_is_atom(tmpVal))
                {
                    tmpVal = octaspire_dern_vm_create_new_value_copy(self->vm, tmpVal);
                }

                octaspire_dern_vm_push_value(self->vm, tmpVal);

                if (!octaspire_list_push_back(self->value.list, &tmpVal))
                {
                    abort();
                }

                octaspire_dern_vm_pop_value(self->vm, tmpVal);
            }
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT:
        case OCTASPIRE_DERN_VALUE_TAG_FUNCTION:
        case OCTASPIRE_DERN_VALUE_TAG_SPECIAL:
        case OCTASPIRE_DERN_VALUE_TAG_BUILTIN:
        case OCTASPIRE_DERN_VALUE_TAG_PORT:
        {
            abort();
        }

        case OCTASPIRE_DERN_VALUE_TAG_C_DATA:
        {
            if (!octaspire_dern_c_data_is_copying_allowed(value->value.cData))
            {
                return false;
            }

            self->value.cData =
                octaspire_dern_c_data_new_copy(
                    value->value.cData,
                    octaspire_dern_vm_get_allocator(self->vm));
        }
        break;
    }

    if (value->docstr)
    {
        // GC releases old if created
        self->docstr = octaspire_dern_vm_create_new_value_copy(self->vm, value->docstr);
    }

    if (value->docvec)
    {
        // GC releases old if created
        self->docvec = octaspire_dern_vm_create_new_value_copy(self->vm, value->docvec);
    }

    return true;
}

bool octaspire_dern_value_set_collection(
    octaspire_dern_value_t * const self,
    octaspire_dern_value_t const * const indexOrKey,
    octaspire_dern_value_t * const value)
{
    octaspire_helpers_verify_true(self && indexOrKey && value);

    if (self->typeTag == OCTASPIRE_DERN_VALUE_TAG_STRING)
    {
        if (value->typeTag      != OCTASPIRE_DERN_VALUE_TAG_CHARACTER ||
            indexOrKey->typeTag != OCTASPIRE_DERN_VALUE_TAG_INTEGER)
        {
            return false;
        }

        return octaspire_string_overwrite_with_string_at(
            self->value.string,
            value->value.character,
            indexOrKey->value.integer);
    }
    else if (self->typeTag == OCTASPIRE_DERN_VALUE_TAG_VECTOR)
    {
        if (indexOrKey->typeTag != OCTASPIRE_DERN_VALUE_TAG_INTEGER)
        {
            return false;
        }

        //bool const result = octaspire_vector_insert_element_before_the_element_at_index(

        if (indexOrKey->value.integer < 0)
        {
            // TODO index from back?
            return false;
        }

        // TODO XXX is this correct?
        if (octaspire_vector_get_length(self->value.vector) <=
            (size_t)indexOrKey->value.integer)
        {

            // octaspire_vector_insert_element_at cannot be used here, because
            // it fills the missing indices with 0. Now those must be nil-values.

            for (size_t i = octaspire_vector_get_length(self->value.vector);
                 i < (size_t)indexOrKey->value.integer;
                 ++i)
            {
                octaspire_dern_value_t *nilValue = octaspire_dern_vm_get_value_nil(self->vm);

                if (!octaspire_vector_push_back_element(
                    self->value.vector,
                    &nilValue))
                {
                    abort();
                }
            }

            octaspire_dern_value_t * const tmpValueForInsertion =
                octaspire_dern_value_is_atom(value) ?
                    octaspire_dern_vm_create_new_value_copy(self->vm, value) :
                    value;

            return octaspire_vector_push_back_element(
                self->value.vector,
                &tmpValueForInsertion);
        }
        else
        {
            octaspire_dern_value_t * const tmpValueForInsertion =
                octaspire_dern_value_is_atom(value) ?
                    octaspire_dern_vm_create_new_value_copy(self->vm, value) :
                    value;

            return octaspire_vector_replace_element_at(
                self->value.vector,
                indexOrKey->value.integer,
                &tmpValueForInsertion);
        }
    }
    else if (self->typeTag == OCTASPIRE_DERN_VALUE_TAG_HASH_MAP)
    {
        octaspire_dern_value_t * const tmpValueForInsertion =
            octaspire_dern_value_is_atom(value) ?
            octaspire_dern_vm_create_new_value_copy(self->vm, value) :
            value;

        // Any previous value must be removed, because hash map
        // can contain multiple values per key, and we must replace, not
        // add the value.
        octaspire_map_remove(
            self->value.hashMap,
            octaspire_dern_value_get_hash(indexOrKey),
            &indexOrKey);

        return octaspire_map_put(
            self->value.hashMap,
            octaspire_dern_value_get_hash(indexOrKey),
            &indexOrKey,
            &tmpValueForInsertion);
    }
    else if (self->typeTag == OCTASPIRE_DERN_VALUE_TAG_QUEUE)
    {
        // TODO XXX
        abort();
    }
    else if (self->typeTag == OCTASPIRE_DERN_VALUE_TAG_LIST)
    {
        // TODO XXX
        abort();
    }
    else if (self->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT)
    {
        // TODO XXX Should here a copy of atom be made or not?
        return octaspire_dern_environment_set(
            self->value.environment,
            indexOrKey,
            value);
    }

    return false;
}

uint32_t octaspire_dern_value_get_hash(
    octaspire_dern_value_t const * const self)
{
    switch (self->typeTag)
    {
        case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
        {
            abort();
        }

        case OCTASPIRE_DERN_VALUE_TAG_NIL:
            return octaspire_helpers_calculate_hash_for_bool_argument(false);

        case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
            return octaspire_helpers_calculate_hash_for_bool_argument(self->value.boolean);

        case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
            return octaspire_helpers_calculate_hash_for_int32_t_argument(self->value.integer);

        case OCTASPIRE_DERN_VALUE_TAG_REAL:
            return octaspire_helpers_calculate_hash_for_double_argument(self->value.real);

        case OCTASPIRE_DERN_VALUE_TAG_STRING:
            return octaspire_string_get_hash(self->value.string);

        case OCTASPIRE_DERN_VALUE_TAG_SEMVER:
        {
            octaspire_string_t * str =
                octaspire_semver_to_string(self->value.semver);

            uint32_t const result = octaspire_string_get_hash(str);

            octaspire_string_release(str);
            str = 0;
            return result;
        }

        case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
            return octaspire_string_get_hash(self->value.character);

        case OCTASPIRE_DERN_VALUE_TAG_SYMBOL:
            return octaspire_string_get_hash(self->value.symbol);

        case OCTASPIRE_DERN_VALUE_TAG_ERROR:
            return octaspire_string_get_hash(self->value.error->message);

        case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
            return octaspire_helpers_calculate_hash_for_void_pointer_argument(self->value.vector);

        case OCTASPIRE_DERN_VALUE_TAG_HASH_MAP:
            return octaspire_helpers_calculate_hash_for_void_pointer_argument(self->value.hashMap);

        case OCTASPIRE_DERN_VALUE_TAG_QUEUE:
            return octaspire_helpers_calculate_hash_for_void_pointer_argument(self->value.queue);

        case OCTASPIRE_DERN_VALUE_TAG_LIST:
            return octaspire_helpers_calculate_hash_for_void_pointer_argument(self->value.list);

        case OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT:
            return octaspire_helpers_calculate_hash_for_void_pointer_argument(
                self->value.environment);

        case OCTASPIRE_DERN_VALUE_TAG_FUNCTION:
            return octaspire_helpers_calculate_hash_for_void_pointer_argument(self->value.function);

        case OCTASPIRE_DERN_VALUE_TAG_SPECIAL:
            return octaspire_helpers_calculate_hash_for_void_pointer_argument(self->value.special);

        case OCTASPIRE_DERN_VALUE_TAG_BUILTIN:
            return octaspire_helpers_calculate_hash_for_void_pointer_argument(self->value.builtin);

        case OCTASPIRE_DERN_VALUE_TAG_PORT:
            return octaspire_helpers_calculate_hash_for_void_pointer_argument(self->value.port);

        case OCTASPIRE_DERN_VALUE_TAG_C_DATA:
            return octaspire_helpers_calculate_hash_for_void_pointer_argument(self->value.cData);
    }

    return 0;
}

bool octaspire_dern_value_is_equal(
    octaspire_dern_value_t const * const self,
    octaspire_dern_value_t const * const other)
{
    octaspire_helpers_verify_not_null(self);
    octaspire_helpers_verify_not_null(other);

    return (octaspire_dern_value_compare(self, other) == 0);
}

bool octaspire_dern_value_is_less_than(
    octaspire_dern_value_t const * const self,
    octaspire_dern_value_t const * const other)
{
    octaspire_helpers_verify_not_null(self);
    octaspire_helpers_verify_not_null(other);

    return (octaspire_dern_value_compare(self, other) < 0);
}

bool octaspire_dern_value_is_greater_than(
    octaspire_dern_value_t const * const self,
    octaspire_dern_value_t const * const other)
{
    octaspire_helpers_verify_not_null(self);
    octaspire_helpers_verify_not_null(other);

    return (octaspire_dern_value_compare(self, other) > 0);
}

bool octaspire_dern_value_is_less_than_or_equal(
    octaspire_dern_value_t const * const self,
    octaspire_dern_value_t const * const other)
{
    octaspire_helpers_verify_not_null(self);
    octaspire_helpers_verify_not_null(other);

    return (octaspire_dern_value_compare(self, other) <= 0);
}

bool octaspire_dern_value_is_greater_than_or_equal(
    octaspire_dern_value_t const * const self,
    octaspire_dern_value_t const * const other)
{
    octaspire_helpers_verify_not_null(self);
    octaspire_helpers_verify_not_null(other);

    return (octaspire_dern_value_compare(self, other) >= 0);
}

octaspire_string_t *octaspire_dern_private_value_to_string(
    octaspire_dern_value_t const * const self,
    bool const plain,
    octaspire_allocator_t * const allocator)
{
    octaspire_helpers_verify_not_null(self);

    switch (self->typeTag)
    {
        case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
        {
            abort();
        }

        case OCTASPIRE_DERN_VALUE_TAG_NIL:
        {
            return octaspire_string_new("nil", allocator);
        }

        case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
        {
            return octaspire_string_new_format(
                allocator,
                "%s",
                self->value.boolean ? "true" : "false");
        }

        case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
            {
                return octaspire_string_new_format(
                    allocator,
                    "%s%" PRId32 "}",
                    (self->value.integer >= 0) ? "{D+" : "{D",
                    self->value.integer);
            }

            case OCTASPIRE_DERN_VALUE_TAG_REAL:
            {
                return octaspire_string_new_format(
                    allocator,
                    "%s%g}",
                    (self->value.real >= 0) ? "{D+" : "{D",
                    self->value.real);
            }

            case OCTASPIRE_DERN_VALUE_TAG_STRING:
            {
                return octaspire_string_new_format(allocator, plain ? "%s" :"[%s]",
                    octaspire_string_get_c_string(self->value.string));
            }

            case OCTASPIRE_DERN_VALUE_TAG_SEMVER:
            {
                return octaspire_semver_to_string(self->value.semver);
            }

            case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
            {
                if (octaspire_string_is_equal_to_c_string(
                    self->value.character,
                    "|"))
                {
                    return octaspire_string_new(plain ? "|" : "|bar|", allocator);
                }
                else if (octaspire_string_is_equal_to_c_string(
                    self->value.character,
                    "\n"))
                {
                    return octaspire_string_new(
                        plain ? "\n" : "|newline|", allocator);
                }
                else if (octaspire_string_is_equal_to_c_string(
                    self->value.character,
                    "\t"))
                {
                    return octaspire_string_new(plain ? "\t" :"|tab|", allocator);
                }
                else
                {
                    return octaspire_string_new_format(
                        allocator,
                        plain ? "%s" : "|%s|",
                        octaspire_string_get_c_string(self->value.character));
                }
            }

            case OCTASPIRE_DERN_VALUE_TAG_SYMBOL:
            {
                return octaspire_string_new_format(allocator, "%s",
                    octaspire_string_get_c_string(self->value.symbol));
            }

            case OCTASPIRE_DERN_VALUE_TAG_ERROR:
            {
                return octaspire_string_new_format(allocator, "<error>: %s",
                    octaspire_string_get_c_string(self->value.error->message));
            }

            case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
            {
                octaspire_string_t *result =
                    octaspire_string_new("(", allocator);

                octaspire_helpers_verify_not_null(result);

                for (size_t i = 0;
                     i < octaspire_vector_get_length(self->value.vector); ++i)
                {
                    octaspire_dern_value_t *tmpValue =
                        octaspire_vector_get_element_at(
                            self->value.vector,
                            (ptrdiff_t)i);

                    octaspire_helpers_verify_not_null(tmpValue);

                    octaspire_string_t *tmpStr = octaspire_dern_value_to_string(
                        tmpValue,
                        allocator);

                    octaspire_helpers_verify_not_null(tmpStr);

                    if (!octaspire_string_concatenate_c_string(
                        result,
                        octaspire_string_get_c_string(tmpStr)))
                    {
                        abort();
                    }

                    octaspire_string_release(tmpStr);
                    tmpStr = 0;

                    if ((i+1) < octaspire_vector_get_length(self->value.vector))
                    {
                        octaspire_string_concatenate_c_string(result, " ");
                    }
                }

                if (!octaspire_string_concatenate_c_string(
                    result,
                    ")"))
                {
                    abort();
                }

                return result;
            }

            case OCTASPIRE_DERN_VALUE_TAG_HASH_MAP:
            {
                octaspire_string_t *result =
                    octaspire_string_new("(hash-map ", allocator);

                octaspire_helpers_verify_not_null(result);

                for (size_t i = 0;
                     i < octaspire_map_get_number_of_elements(
                         self->value.hashMap);
                    ++i)
                {
                    octaspire_map_element_t *element =
                        octaspire_map_get_at_index(
                            self->value.hashMap,
                            (ptrdiff_t)i);

                    // Key
                    octaspire_string_t *tmpStr = octaspire_dern_value_to_string(
                        octaspire_map_element_get_key(element),
                        allocator);

                    octaspire_helpers_verify_not_null(tmpStr);

                    if (!octaspire_string_concatenate_c_string(
                        result,
                        octaspire_string_get_c_string(tmpStr)))
                    {
                        abort();
                    }

                    octaspire_string_release(tmpStr);
                    tmpStr = 0;

                    // " "
                    if (!octaspire_string_concatenate_c_string(
                        result,
                        " "))
                    {
                        abort();
                    }

                    // Value
                    tmpStr = octaspire_dern_value_to_string(
                        octaspire_map_element_get_value(element),
                        allocator);

                    octaspire_helpers_verify_not_null(tmpStr);

                    if (!octaspire_string_concatenate_c_string(
                        result,
                        octaspire_string_get_c_string(tmpStr)))
                    {
                        abort();
                    }

                    octaspire_string_release(tmpStr);
                    tmpStr = 0;

                    if ((i+1) < octaspire_map_get_number_of_elements(
                        self->value.hashMap))
                    {
                        octaspire_string_concatenate_c_string(
                            result,
                            "\n          ");
                    }
                }

                if (!octaspire_string_concatenate_c_string(
                    result,
                    ")"))
                {
                    abort();
                }

                return result;
            }

            case OCTASPIRE_DERN_VALUE_TAG_QUEUE:
            {
                octaspire_string_t *result =
                    octaspire_string_new("(queue ", allocator);

                octaspire_helpers_verify_not_null(result);

                // Add iterator into queue and use it here
                for (size_t i = 0; i < octaspire_queue_get_length(self->value.queue); ++i)
                {
                    octaspire_dern_value_t *tmpValue =
                        octaspire_queue_get_at(
                            self->value.queue,
                            (ptrdiff_t)i);

                    octaspire_helpers_verify_not_null(tmpValue);

                    octaspire_string_t *tmpStr = octaspire_dern_value_to_string(
                        tmpValue,
                        allocator);

                    octaspire_helpers_verify_not_null(tmpStr);

                    if (!octaspire_string_concatenate_c_string(
                        result,
                        octaspire_string_get_c_string(tmpStr)))
                    {
                        abort();
                    }

                    octaspire_string_release(tmpStr);
                    tmpStr = 0;

                    if ((i+1) < octaspire_queue_get_length(self->value.queue))
                    {
                        octaspire_string_concatenate_c_string(result, " ");
                    }
                }

                if (!octaspire_string_concatenate_c_string(
                    result,
                    ")"))
                {
                    abort();
                }

                return result;
            }

            case OCTASPIRE_DERN_VALUE_TAG_LIST:
            {
                octaspire_string_t *result =
                    octaspire_string_new("(list ", allocator);

                octaspire_helpers_verify_not_null(result);

                // More efficient iteration
                for (size_t i = 0; i < octaspire_list_get_length(self->value.list); ++i)
                {
                    octaspire_dern_value_t *tmpValue =
                        octaspire_list_node_get_element(
                            octaspire_list_get_at(
                                self->value.list,
                                (ptrdiff_t)i));

                    octaspire_helpers_verify_not_null(tmpValue);

                    octaspire_string_t *tmpStr = octaspire_dern_value_to_string(
                        tmpValue,
                        allocator);

                    octaspire_helpers_verify_not_null(tmpStr);

                    if (!octaspire_string_concatenate_c_string(
                        result,
                        octaspire_string_get_c_string(tmpStr)))
                    {
                        abort();
                    }

                    octaspire_string_release(tmpStr);
                    tmpStr = 0;

                    if ((i+1) < octaspire_list_get_length(self->value.list))
                    {
                        octaspire_string_concatenate_c_string(result, " ");
                    }
                }

                if (!octaspire_string_concatenate_c_string(
                    result,
                    ")"))
                {
                    abort();
                }

                return result;
            }

            case OCTASPIRE_DERN_VALUE_TAG_FUNCTION:
            {
                return octaspire_dern_function_to_string(self->value.function, allocator);
            }

            case OCTASPIRE_DERN_VALUE_TAG_BUILTIN:
            {
                return octaspire_dern_builtin_to_string(self->value.builtin, allocator);
            }

            case OCTASPIRE_DERN_VALUE_TAG_PORT:
            {
                return octaspire_dern_port_to_string(self->value.port, allocator);
            }

            case OCTASPIRE_DERN_VALUE_TAG_C_DATA:
            {
                return octaspire_dern_c_data_to_string(self->value.cData, allocator);
            }

            case OCTASPIRE_DERN_VALUE_TAG_SPECIAL:
            {
                return octaspire_dern_special_to_string(self->value.special, allocator);
            }

            case OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT:
            {
                octaspire_helpers_verify_not_null(self->value.environment);
                return octaspire_dern_environment_to_string(self->value.environment);
            }
    }

    return 0;
}

octaspire_string_t *octaspire_dern_value_to_string(
octaspire_dern_value_t const * const self,
octaspire_allocator_t * const allocator)
{
    return octaspire_dern_private_value_to_string(self, false, allocator);
}

octaspire_string_t *octaspire_dern_value_to_string_plain(
octaspire_dern_value_t const * const self,
octaspire_allocator_t * const allocator)
{
    return octaspire_dern_private_value_to_string(self, true, allocator);
}

bool octaspire_dern_value_is_integer(
    octaspire_dern_value_t const * const self)
{
    return self->typeTag == OCTASPIRE_DERN_VALUE_TAG_INTEGER;
}

bool octaspire_dern_value_is_real(
    octaspire_dern_value_t const * const self)
{
    return self->typeTag == OCTASPIRE_DERN_VALUE_TAG_REAL;
}

bool octaspire_dern_value_is_number(
    octaspire_dern_value_t const * const self)
{
    return self->typeTag == OCTASPIRE_DERN_VALUE_TAG_INTEGER ||
           self->typeTag == OCTASPIRE_DERN_VALUE_TAG_REAL;
}

bool octaspire_dern_value_is_nil(
    octaspire_dern_value_t const * const self)
{
    return self->typeTag == OCTASPIRE_DERN_VALUE_TAG_NIL;
}

bool octaspire_dern_value_is_boolean(
    octaspire_dern_value_t const * const self)
{
    return self->typeTag == OCTASPIRE_DERN_VALUE_TAG_BOOLEAN;
}

bool octaspire_dern_value_is_character(
    octaspire_dern_value_t const * const self)
{
    return self->typeTag == OCTASPIRE_DERN_VALUE_TAG_CHARACTER;
}

bool octaspire_dern_value_is_string(
    octaspire_dern_value_t const * const self)
{
    return self->typeTag == OCTASPIRE_DERN_VALUE_TAG_STRING;
}

bool octaspire_dern_value_is_semver(
    octaspire_dern_value_t const * const self)
{
    return self->typeTag == OCTASPIRE_DERN_VALUE_TAG_SEMVER;
}

bool octaspire_dern_value_is_symbol(
    octaspire_dern_value_t const * const self)
{
    return self->typeTag == OCTASPIRE_DERN_VALUE_TAG_SYMBOL;
}

bool octaspire_dern_value_is_text(
    octaspire_dern_value_t const * const self)
{
    return self->typeTag == OCTASPIRE_DERN_VALUE_TAG_CHARACTER ||
           self->typeTag == OCTASPIRE_DERN_VALUE_TAG_STRING    ||
           self->typeTag == OCTASPIRE_DERN_VALUE_TAG_SYMBOL;
}

bool octaspire_dern_value_is_vector(
    octaspire_dern_value_t const * const self)
{
    return self->typeTag == OCTASPIRE_DERN_VALUE_TAG_VECTOR;
}

bool octaspire_dern_value_is_hash_map(
    octaspire_dern_value_t const * const self)
{
    return self->typeTag == OCTASPIRE_DERN_VALUE_TAG_HASH_MAP;
}

bool octaspire_dern_value_is_queue(
    octaspire_dern_value_t const * const self)
{
    return self->typeTag == OCTASPIRE_DERN_VALUE_TAG_QUEUE;
}

bool octaspire_dern_value_is_list(
    octaspire_dern_value_t const * const self)
{
    return self->typeTag == OCTASPIRE_DERN_VALUE_TAG_LIST;
}

bool octaspire_dern_value_is_port(
    octaspire_dern_value_t const * const self)
{
    return self->typeTag == OCTASPIRE_DERN_VALUE_TAG_PORT;
}

bool octaspire_dern_value_is_environment(
    octaspire_dern_value_t const * const self)
{
    return self->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT;
}

bool octaspire_dern_value_is_error(
    octaspire_dern_value_t const * const self)
{
    return self->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR;
}

void octaspire_dern_value_as_error_set_line_number(
    octaspire_dern_value_t const * const self,
    size_t const lineNumber)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR);
    self->value.error->lineNumber = lineNumber;
}

char const *octaspire_dern_value_as_error_get_c_string(
    octaspire_dern_value_t const * const self)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR);
    return octaspire_string_get_c_string(self->value.error->message);
}

octaspire_dern_environment_t *octaspire_dern_value_as_environment_get_value(
    octaspire_dern_value_t * const self)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);
    return self->value.environment;
}

octaspire_dern_environment_t const *octaspire_dern_value_as_environment_get_value_const(
    octaspire_dern_value_t const * const self)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);
    return self->value.environment;
}

bool octaspire_dern_value_is_function(
    octaspire_dern_value_t const * const self)
{
    return self->typeTag == OCTASPIRE_DERN_VALUE_TAG_FUNCTION;
}

bool octaspire_dern_value_is_builtin(
    octaspire_dern_value_t const * const self)
{
    return self->typeTag == OCTASPIRE_DERN_VALUE_TAG_BUILTIN;
}

bool octaspire_dern_value_is_special(
    octaspire_dern_value_t const * const self)
{
    return self->typeTag == OCTASPIRE_DERN_VALUE_TAG_SPECIAL;
}

bool octaspire_dern_value_is_howto_allowed(
    octaspire_dern_value_t const * const self)
{
    // TODO XXX make user functions to support setting
    // howto
    octaspire_helpers_verify_true(
        octaspire_dern_value_is_builtin(self) ||
        octaspire_dern_value_is_special(self) ||
        octaspire_dern_value_is_function(self));

    return self->howtoAllowed;
}

bool octaspire_dern_value_is_c_data(
    octaspire_dern_value_t const * const self)
{
    return self->typeTag == OCTASPIRE_DERN_VALUE_TAG_C_DATA;
}

octaspire_dern_c_data_t *octaspire_dern_value_as_c_data_get_value(
    octaspire_dern_value_t * const self)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_C_DATA);
    return self->value.cData;
}

octaspire_dern_c_data_t const *octaspire_dern_value_as_c_data_get_value_const(
    octaspire_dern_value_t const * const self)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_C_DATA);
    return self->value.cData;
}

void octaspire_dern_value_print(
    octaspire_dern_value_t const * const self,
    octaspire_allocator_t *allocator)
{
    octaspire_string_t *str = octaspire_dern_value_to_string(self, allocator);
    printf("%s", octaspire_string_get_c_string(str));
    octaspire_string_release(str);
    str = 0;
}

uintmax_t octaspire_dern_value_get_unique_id(
    octaspire_dern_value_t const * const self)
{
    return self->uniqueId;
}

bool octaspire_dern_value_as_boolean_get_value(
    octaspire_dern_value_t const * const self)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_BOOLEAN);
    return self->value.boolean;
}

int32_t octaspire_dern_value_as_integer_get_value(
    octaspire_dern_value_t const * const self)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_INTEGER);
    return self->value.integer;
}

void octaspire_dern_value_as_integer_set_value(
    octaspire_dern_value_t * const self,
    int32_t const value)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_INTEGER);
    self->value.integer = value;
}

double octaspire_dern_value_as_real_get_value(
    octaspire_dern_value_t const * const self)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_REAL);
    return self->value.real;
}

void octaspire_dern_value_as_real_set_value(
    octaspire_dern_value_t * const self,
    double const value)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_REAL);
    self->value.real = value;
}

void octaspire_dern_value_as_number_set_value(
    octaspire_dern_value_t * const self,
    double const value)
{
    octaspire_helpers_verify_true(
        octaspire_dern_value_is_number(self));

    if (octaspire_dern_value_is_real(self))
    {
        self->value.real = value;
    }
    else
    {
        self->value.integer = (int32_t)value;
    }
}

double octaspire_dern_value_as_number_get_value(
    octaspire_dern_value_t const * const self)
{
    octaspire_helpers_verify_true(octaspire_dern_value_is_number(self));

    if (octaspire_dern_value_is_integer(self))
    {
        return (double)self->value.integer;
    }

    return self->value.real;
}

bool octaspire_dern_value_as_hash_map_remove(
    octaspire_dern_value_t * const self,
    octaspire_dern_value_t * const keyValue)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_HASH_MAP);

    return octaspire_map_remove(
        self->value.hashMap,
        octaspire_dern_value_get_hash(keyValue),
        &keyValue);
}

octaspire_dern_function_t *octaspire_dern_value_as_function(
    octaspire_dern_value_t * const self)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_FUNCTION);
    return self->value.function;
}

bool octaspire_dern_value_as_hash_map_add(
    octaspire_dern_value_t * const self,
    octaspire_dern_value_t * const toBeAdded1,
    octaspire_dern_value_t * const toBeAdded2)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_HASH_MAP);

    switch (toBeAdded1->typeTag)
    {
        case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
        {
            abort();
        }

        case OCTASPIRE_DERN_VALUE_TAG_HASH_MAP:
        {
            octaspire_helpers_verify_null(toBeAdded2);

            return octaspire_map_add_hash_map(
                self->value.hashMap,
                toBeAdded1->value.hashMap);
        }

        case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
        {
            octaspire_helpers_verify_null(toBeAdded2);

            size_t const vecLen = octaspire_dern_value_as_vector_get_length(toBeAdded1);

            if (vecLen % 2 != 0)
            {
                return false;
            }

            bool result = true;
            for (size_t i = 0; i < vecLen; i += 2)
            {
                octaspire_dern_value_t * const key =
                    octaspire_dern_value_as_vector_get_element_at(
                        toBeAdded1,
                        (ptrdiff_t)i);

                octaspire_dern_value_t * const val =
                    octaspire_dern_value_as_vector_get_element_at(
                        toBeAdded1,
                        (ptrdiff_t)(i + 1));

                uint32_t const hash = octaspire_dern_value_get_hash(key);

                if (!octaspire_dern_value_as_hash_map_put(
                        self,
                        hash,
                        key,
                        val))
                {
                    result = false;
                }
            }

            return result;
        }

        case OCTASPIRE_DERN_VALUE_TAG_NIL:
        case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
        case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
        case OCTASPIRE_DERN_VALUE_TAG_REAL:
        case OCTASPIRE_DERN_VALUE_TAG_STRING:
        case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
        case OCTASPIRE_DERN_VALUE_TAG_SYMBOL:
        case OCTASPIRE_DERN_VALUE_TAG_ERROR:
        case OCTASPIRE_DERN_VALUE_TAG_QUEUE:
        case OCTASPIRE_DERN_VALUE_TAG_LIST:
        case OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT:
        case OCTASPIRE_DERN_VALUE_TAG_FUNCTION:
        case OCTASPIRE_DERN_VALUE_TAG_SPECIAL:
        case OCTASPIRE_DERN_VALUE_TAG_BUILTIN:
        case OCTASPIRE_DERN_VALUE_TAG_PORT:
        case OCTASPIRE_DERN_VALUE_TAG_C_DATA:
        case OCTASPIRE_DERN_VALUE_TAG_SEMVER:
        {
            if (!toBeAdded2)
            {
                return false;
            }

            uint32_t const hash = octaspire_dern_value_get_hash(toBeAdded1);

            if (!octaspire_dern_value_as_hash_map_put(
                    self,
                    hash,
                    toBeAdded1,
                    toBeAdded2))
            {
                return false;
            }

            return true;
        }
    }

    return false;
}

bool octaspire_dern_value_as_queue_push(
    octaspire_dern_value_t * const self,
    octaspire_dern_value_t * const toBeAdded)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_QUEUE);
if (octaspire_dern_value_is_atom(toBeAdded))
    {
        octaspire_dern_value_t * const copyVal =
            octaspire_dern_vm_create_new_value_copy(self->vm, toBeAdded);

        return octaspire_queue_push(self->value.queue, &copyVal);
    }

    return octaspire_queue_push(self->value.queue, &toBeAdded);
}

bool octaspire_dern_value_as_queue_pop(octaspire_dern_value_t * const self)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_QUEUE);
    return octaspire_queue_pop(self->value.queue);
}

size_t octaspire_dern_value_as_queue_get_length(
    octaspire_dern_value_t const * const self)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_QUEUE);
    return octaspire_queue_get_length(self->value.queue);
}

bool octaspire_dern_value_as_list_push_back(
    octaspire_dern_value_t * const self,
    octaspire_dern_value_t * const toBeAdded)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_LIST);

    if (octaspire_dern_value_is_atom(toBeAdded))
    {
        octaspire_dern_value_t * const copyVal =
            octaspire_dern_vm_create_new_value_copy(self->vm, toBeAdded);

        return octaspire_list_push_back(self->value.list, &copyVal);
    }

    return octaspire_list_push_back(self->value.list, &toBeAdded);
}

bool octaspire_dern_value_as_list_pop_back(octaspire_dern_value_t * const self)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_LIST);
    return octaspire_list_pop_back(self->value.list);
}

bool octaspire_dern_value_as_list_pop_front(octaspire_dern_value_t * const self)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_LIST);
    return octaspire_list_pop_front(self->value.list);
}

size_t octaspire_dern_value_as_list_get_length(
    octaspire_dern_value_t const * const self)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_LIST);
    return octaspire_list_get_length(self->value.list);
}

bool octaspire_dern_value_as_character_add(
    octaspire_dern_value_t * const self,
    octaspire_dern_value_t * const other)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_CHARACTER);

    switch (other->typeTag)
    {
        case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
        {
            abort();
        }

        // TODO should adding of char, string and vector be allowed?
        // It could turn this character into a string.

        case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
        {
            uint32_t const charAsInt = octaspire_string_get_ucs_character_at_index(
                self->value.character,
                0);

            octaspire_string_clear(self->value.character);

            octaspire_string_push_back_ucs_character(
                self->value.character,
                (uint32_t)((int32_t)charAsInt + other->value.integer));

            return true;
        }

        case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
        {
           octaspire_string_t *newStr = octaspire_string_new_copy(
                self->value.character,
                octaspire_dern_vm_get_allocator(self->vm));

            octaspire_string_release(self->value.character);
            self->value.character = 0;

            self->value.string = newStr;

            octaspire_string_concatenate(self->value.string, other->value.character);

            self->typeTag = OCTASPIRE_DERN_VALUE_TAG_STRING;

            return true;
        }

        case OCTASPIRE_DERN_VALUE_TAG_NIL:
        case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
        case OCTASPIRE_DERN_VALUE_TAG_REAL:
        case OCTASPIRE_DERN_VALUE_TAG_STRING:
        case OCTASPIRE_DERN_VALUE_TAG_SYMBOL:
        case OCTASPIRE_DERN_VALUE_TAG_ERROR:
        case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
        case OCTASPIRE_DERN_VALUE_TAG_HASH_MAP:
        case OCTASPIRE_DERN_VALUE_TAG_QUEUE:
        case OCTASPIRE_DERN_VALUE_TAG_LIST:
        case OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT:
        case OCTASPIRE_DERN_VALUE_TAG_FUNCTION:
        case OCTASPIRE_DERN_VALUE_TAG_SPECIAL:
        case OCTASPIRE_DERN_VALUE_TAG_BUILTIN:
        case OCTASPIRE_DERN_VALUE_TAG_PORT:
        case OCTASPIRE_DERN_VALUE_TAG_C_DATA:
        case OCTASPIRE_DERN_VALUE_TAG_SEMVER:
        {
            return false;
        }
    }

    abort();
    return false;
}

bool octaspire_dern_value_as_character_subtract(
    octaspire_dern_value_t * const self,
    octaspire_dern_value_t * const other)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_CHARACTER);

    switch (other->typeTag)
    {
        case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
        {
            abort();
        }

        case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
        {
            uint32_t const charAsInt =
                octaspire_string_get_ucs_character_at_index(
                    self->value.character,
                    0);

            octaspire_string_clear(self->value.character);

            octaspire_string_push_back_ucs_character(
                self->value.character,
                (uint32_t)((int32_t)charAsInt - other->value.integer));

            return true;
        }

        case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
        {
            uint32_t const charAsInt = octaspire_string_get_ucs_character_at_index(
                self->value.character,
                0);

            uint32_t const otherAsInt = octaspire_string_get_ucs_character_at_index(
                other->value.character,
                0);

            octaspire_string_clear(self->value.character);

            octaspire_string_push_back_ucs_character(
                self->value.character,
                charAsInt - otherAsInt);

            return true;
        }

        case OCTASPIRE_DERN_VALUE_TAG_NIL:
        case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
        case OCTASPIRE_DERN_VALUE_TAG_REAL:
        case OCTASPIRE_DERN_VALUE_TAG_STRING:
        case OCTASPIRE_DERN_VALUE_TAG_SYMBOL:
        case OCTASPIRE_DERN_VALUE_TAG_ERROR:
        case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
        case OCTASPIRE_DERN_VALUE_TAG_HASH_MAP:
        case OCTASPIRE_DERN_VALUE_TAG_QUEUE:
        case OCTASPIRE_DERN_VALUE_TAG_LIST:
        case OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT:
        case OCTASPIRE_DERN_VALUE_TAG_FUNCTION:
        case OCTASPIRE_DERN_VALUE_TAG_SPECIAL:
        case OCTASPIRE_DERN_VALUE_TAG_BUILTIN:
        case OCTASPIRE_DERN_VALUE_TAG_PORT:
        case OCTASPIRE_DERN_VALUE_TAG_C_DATA:
        case OCTASPIRE_DERN_VALUE_TAG_SEMVER:
        {
            return false;
        }
    }

    abort();
    return false;
}

bool octaspire_dern_value_as_integer_add(
    octaspire_dern_value_t * const self,
    octaspire_dern_value_t * const other)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_INTEGER);

    switch (other->typeTag)
    {
        case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
        {
            abort();
        }

        case OCTASPIRE_DERN_VALUE_TAG_STRING:
        {
            octaspire_string_t *chars = octaspire_string_new(
                "-0123456789.",
                octaspire_dern_vm_get_allocator(self->vm));

            if (octaspire_string_contains_only_these_chars(
                other->value.string,
                chars))
            {
                if (octaspire_string_contains_char(
                    other->value.string,
                    (uint32_t)'.'))
                {
                    self->typeTag = OCTASPIRE_DERN_VALUE_TAG_REAL;
                    self->value.real = self->value.integer;

                    self->value.real += octaspire_dern_helpers_atof(
                        octaspire_string_get_c_string(other->value.string),
                        octaspire_dern_vm_get_allocator(self->vm));

                    return true;
                }
                else
                {
                    self->value.integer +=
                        atol(octaspire_string_get_c_string(other->value.string));

                    return true;
                }
            }
            else
            {
                return false;
            }
        }

        case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
        {
            self->value.integer += other->value.integer;
            return true;
        }

        case OCTASPIRE_DERN_VALUE_TAG_REAL:
        {
            self->typeTag = OCTASPIRE_DERN_VALUE_TAG_REAL;
            self->value.real = self->value.integer;
            self->value.real += other->value.real;
            return true;
        }

        case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
        {
            bool result = true;
            for (size_t i = 0; i < octaspire_dern_value_as_vector_get_length(other); ++i)
            {
                if (!octaspire_dern_value_as_integer_add(
                        self,
                        octaspire_dern_value_as_vector_get_element_at(
                            other,
                            (ptrdiff_t)i)))
                {
                    result = false;
                }
            }

            return result;
        }

        case OCTASPIRE_DERN_VALUE_TAG_NIL:
        case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
        case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
        case OCTASPIRE_DERN_VALUE_TAG_SYMBOL:
        case OCTASPIRE_DERN_VALUE_TAG_ERROR:
        case OCTASPIRE_DERN_VALUE_TAG_HASH_MAP:
        case OCTASPIRE_DERN_VALUE_TAG_QUEUE:
        case OCTASPIRE_DERN_VALUE_TAG_LIST:
        case OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT:
        case OCTASPIRE_DERN_VALUE_TAG_FUNCTION:
        case OCTASPIRE_DERN_VALUE_TAG_SPECIAL:
        case OCTASPIRE_DERN_VALUE_TAG_BUILTIN:
        case OCTASPIRE_DERN_VALUE_TAG_PORT:
        case OCTASPIRE_DERN_VALUE_TAG_C_DATA:
        case OCTASPIRE_DERN_VALUE_TAG_SEMVER:
        {
            return false;
        }
    }

    abort();
    return false;
}

bool octaspire_dern_value_as_integer_subtract(
    octaspire_dern_value_t * const self,
    octaspire_dern_value_t * const other)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_INTEGER);

    switch (other->typeTag)
    {
        case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
        {
            abort();
        }

        case OCTASPIRE_DERN_VALUE_TAG_STRING:
        {
            octaspire_string_t *chars = octaspire_string_new(
                "-0123456789.",
                octaspire_dern_vm_get_allocator(self->vm));

            if (octaspire_string_contains_only_these_chars(
                    other->value.string,
                    chars))
            {
                if (octaspire_string_contains_char(
                    other->value.string,
                    (uint32_t)'.'))
                {
                    self->typeTag = OCTASPIRE_DERN_VALUE_TAG_REAL;
                    self->value.real = self->value.integer;

                    self->value.real -= octaspire_dern_helpers_atof(
                        octaspire_string_get_c_string(other->value.string),
                        octaspire_dern_vm_get_allocator(self->vm));

                    return true;
                }
                else
                {
                    self->value.integer -=
                        atol(octaspire_string_get_c_string(other->value.string));

                    return true;
                }
            }
            else
            {
                return false;
            }
        }

        case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
        {
            self->value.integer -= other->value.integer;
            return true;
        }

        case OCTASPIRE_DERN_VALUE_TAG_REAL:
        {
            self->typeTag = OCTASPIRE_DERN_VALUE_TAG_REAL;
            self->value.real = self->value.integer;
            self->value.real -= other->value.real;
            return true;
        }

        case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
        {
            bool result = true;
            for (size_t i = 0; i < octaspire_dern_value_as_vector_get_length(other); ++i)
            {
                if (!octaspire_dern_value_as_integer_subtract(
                        self,
                        octaspire_dern_value_as_vector_get_element_at(
                            other,
                            (ptrdiff_t)i)))
                {
                    result = false;
                }
            }

            return result;
        }

        case OCTASPIRE_DERN_VALUE_TAG_NIL:
        case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
        case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
        case OCTASPIRE_DERN_VALUE_TAG_SYMBOL:
        case OCTASPIRE_DERN_VALUE_TAG_ERROR:
        case OCTASPIRE_DERN_VALUE_TAG_HASH_MAP:
        case OCTASPIRE_DERN_VALUE_TAG_QUEUE:
        case OCTASPIRE_DERN_VALUE_TAG_LIST:
        case OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT:
        case OCTASPIRE_DERN_VALUE_TAG_FUNCTION:
        case OCTASPIRE_DERN_VALUE_TAG_SPECIAL:
        case OCTASPIRE_DERN_VALUE_TAG_BUILTIN:
        case OCTASPIRE_DERN_VALUE_TAG_PORT:
        case OCTASPIRE_DERN_VALUE_TAG_C_DATA:
        case OCTASPIRE_DERN_VALUE_TAG_SEMVER:
        {
            return false;
        }
    }

    return false;
}

bool octaspire_dern_value_as_semver_add_or_subtract(
    octaspire_dern_value_t * const self,
    octaspire_dern_value_t * const other,
    bool                     const add)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_SEMVER);

    switch (other->typeTag)
    {
        case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
        {
            abort();
        }

        case OCTASPIRE_DERN_VALUE_TAG_SEMVER:
        {
            return octaspire_semver_add_or_subtract(
                self->value.semver,
                other->value.semver,
                add);
        }

        case OCTASPIRE_DERN_VALUE_TAG_STRING:
        case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
        case OCTASPIRE_DERN_VALUE_TAG_REAL:
        case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
        case OCTASPIRE_DERN_VALUE_TAG_NIL:
        case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
        case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
        case OCTASPIRE_DERN_VALUE_TAG_SYMBOL:
        case OCTASPIRE_DERN_VALUE_TAG_ERROR:
        case OCTASPIRE_DERN_VALUE_TAG_HASH_MAP:
        case OCTASPIRE_DERN_VALUE_TAG_QUEUE:
        case OCTASPIRE_DERN_VALUE_TAG_LIST:
        case OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT:
        case OCTASPIRE_DERN_VALUE_TAG_FUNCTION:
        case OCTASPIRE_DERN_VALUE_TAG_SPECIAL:
        case OCTASPIRE_DERN_VALUE_TAG_BUILTIN:
        case OCTASPIRE_DERN_VALUE_TAG_PORT:
        case OCTASPIRE_DERN_VALUE_TAG_C_DATA:
        {
            return false;
        }
    }

    return false;
}

bool octaspire_dern_value_as_semver_pop_back(
    octaspire_dern_value_t * const self)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_SEMVER);
    return octaspire_semver_pop_back(self->value.semver);
}

bool octaspire_dern_value_as_semver_pop_front(
    octaspire_dern_value_t * const self)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_SEMVER);
    return octaspire_semver_pop_front(self->value.semver);
}


bool octaspire_dern_value_as_real_add(
    octaspire_dern_value_t * const self,
    octaspire_dern_value_t * const other)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_REAL);

    switch (other->typeTag)
    {
        case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
        {
            abort();
        }

        case OCTASPIRE_DERN_VALUE_TAG_STRING:
        {
            octaspire_string_t *chars = octaspire_string_new(
                "-0123456789.",
                octaspire_dern_vm_get_allocator(self->vm));

            if (octaspire_string_contains_only_these_chars(
                    other->value.string,
                    chars))
            {
                if (octaspire_string_contains_char(
                        other->value.string,
                        (uint32_t)'.'))
                {
                    self->value.real += octaspire_dern_helpers_atof(
                        octaspire_string_get_c_string(other->value.string),
                        octaspire_dern_vm_get_allocator(self->vm));

                    return true;
                }
                else
                {
                    self->value.real +=
                        atol(octaspire_string_get_c_string(other->value.string));

                    return true;
                }
            }
            else
            {
                return false;
            }
        }

        case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
        {
            self->value.real += other->value.integer;
            return true;
        }

        case OCTASPIRE_DERN_VALUE_TAG_REAL:
        {
            self->value.real += other->value.real;
            return true;
        }

        case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
        {
            bool result = true;
            for (size_t i = 0; i < octaspire_dern_value_as_vector_get_length(other); ++i)
            {
                if (!octaspire_dern_value_as_real_add(
                        self,
                        octaspire_dern_value_as_vector_get_element_at(
                            other,
                            (ptrdiff_t)i)))
                {
                    result = false;
                }
            }

            return result;
        }

        case OCTASPIRE_DERN_VALUE_TAG_NIL:
        case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
        case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
        case OCTASPIRE_DERN_VALUE_TAG_SYMBOL:
        case OCTASPIRE_DERN_VALUE_TAG_ERROR:
        case OCTASPIRE_DERN_VALUE_TAG_HASH_MAP:
        case OCTASPIRE_DERN_VALUE_TAG_QUEUE:
        case OCTASPIRE_DERN_VALUE_TAG_LIST:
        case OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT:
        case OCTASPIRE_DERN_VALUE_TAG_FUNCTION:
        case OCTASPIRE_DERN_VALUE_TAG_SPECIAL:
        case OCTASPIRE_DERN_VALUE_TAG_BUILTIN:
        case OCTASPIRE_DERN_VALUE_TAG_PORT:
        case OCTASPIRE_DERN_VALUE_TAG_C_DATA:
        case OCTASPIRE_DERN_VALUE_TAG_SEMVER:
        {
            return false;
        }
    }

    abort();
    return false;
}

bool octaspire_dern_value_as_real_subtract(
    octaspire_dern_value_t * const self,
    octaspire_dern_value_t * const other)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_REAL);

    switch (other->typeTag)
    {
        case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
        {
            abort();
        }

        case OCTASPIRE_DERN_VALUE_TAG_STRING:
        {
            octaspire_string_t *chars = octaspire_string_new(
                "-0123456789.",
                octaspire_dern_vm_get_allocator(self->vm));

            if (octaspire_string_contains_only_these_chars(
                    other->value.string,
                    chars))
            {
                if (octaspire_string_contains_char(
                        other->value.string,
                        (uint32_t)'.'))
                {
                    self->value.real -= octaspire_dern_helpers_atof(
                        octaspire_string_get_c_string(other->value.string),
                        octaspire_dern_vm_get_allocator(self->vm));

                    return true;
                }
                else
                {
                    self->value.real -=
                        atol(octaspire_string_get_c_string(other->value.string));

                    return true;
                }
            }
            else
            {
                return false;
            }
        }

        case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
        {
            self->value.real -= other->value.integer;
            return true;
        }

        case OCTASPIRE_DERN_VALUE_TAG_REAL:
        {
            self->value.real -= other->value.real;
            return true;
        }

        case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
        {
            bool result = true;
            for (size_t i = 0; i < octaspire_dern_value_as_vector_get_length(other); ++i)
            {
                if (!octaspire_dern_value_as_real_subtract(
                        self,
                        octaspire_dern_value_as_vector_get_element_at(
                            other,
                            (ptrdiff_t)i)))
                {
                    result = false;
                }
            }

            return result;
        }

        case OCTASPIRE_DERN_VALUE_TAG_NIL:
        case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
        case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
        case OCTASPIRE_DERN_VALUE_TAG_SYMBOL:
        case OCTASPIRE_DERN_VALUE_TAG_ERROR:
        case OCTASPIRE_DERN_VALUE_TAG_HASH_MAP:
        case OCTASPIRE_DERN_VALUE_TAG_QUEUE:
        case OCTASPIRE_DERN_VALUE_TAG_LIST:
        case OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT:
        case OCTASPIRE_DERN_VALUE_TAG_FUNCTION:
        case OCTASPIRE_DERN_VALUE_TAG_SPECIAL:
        case OCTASPIRE_DERN_VALUE_TAG_BUILTIN:
        case OCTASPIRE_DERN_VALUE_TAG_PORT:
        case OCTASPIRE_DERN_VALUE_TAG_C_DATA:
        case OCTASPIRE_DERN_VALUE_TAG_SEMVER:
        {
            return false;
        }
    }

    abort();
    return 0;
}

char const *octaspire_dern_value_as_character_get_c_string(
    octaspire_dern_value_t const * const self)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_CHARACTER);
    return octaspire_string_get_c_string(self->value.character);
}

bool octaspire_dern_value_as_string_push_back(
    octaspire_dern_value_t * const self,
    octaspire_dern_value_t * const value)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_STRING);

    switch (value->typeTag)
    {
        case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
        {
            abort();
        }

        case OCTASPIRE_DERN_VALUE_TAG_STRING:
        {
            return octaspire_string_concatenate(
                self->value.string,
                value->value.string);
        }

        case OCTASPIRE_DERN_VALUE_TAG_SYMBOL:
        {
            return octaspire_string_concatenate(
                self->value.string,
                value->value.symbol);
        }

        case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
        {
            return octaspire_string_concatenate(
                self->value.string,
                value->value.character);
        }

        case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
        {
            bool result = true;
            for (size_t i = 0; i < octaspire_dern_value_as_vector_get_length(value); ++i)
            {
                if (!octaspire_dern_value_as_string_push_back(
                        self,
                        octaspire_dern_value_as_vector_get_element_at(
                            value,
                            (ptrdiff_t)i)))
                {
                    result = false;
                }
            }

            return result;
        }

        case OCTASPIRE_DERN_VALUE_TAG_NIL:
        case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
        case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
        case OCTASPIRE_DERN_VALUE_TAG_REAL:
        case OCTASPIRE_DERN_VALUE_TAG_ERROR:
        case OCTASPIRE_DERN_VALUE_TAG_HASH_MAP:
        case OCTASPIRE_DERN_VALUE_TAG_QUEUE:
        case OCTASPIRE_DERN_VALUE_TAG_LIST:
        case OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT:
        case OCTASPIRE_DERN_VALUE_TAG_FUNCTION:
        case OCTASPIRE_DERN_VALUE_TAG_SPECIAL:
        case OCTASPIRE_DERN_VALUE_TAG_BUILTIN:
        case OCTASPIRE_DERN_VALUE_TAG_PORT:
        case OCTASPIRE_DERN_VALUE_TAG_C_DATA:
        case OCTASPIRE_DERN_VALUE_TAG_SEMVER:
        {
            octaspire_string_t *tmpStr =
                octaspire_dern_value_to_string(value, octaspire_dern_vm_get_allocator(self->vm));

            bool result = octaspire_string_concatenate(self->value.string, tmpStr);

            octaspire_string_release(tmpStr);
            tmpStr = 0;

            return result;
        }
    }

    return false;
}

bool octaspire_dern_value_as_symbol_push_back(
    octaspire_dern_value_t * const self,
    octaspire_dern_value_t * const value)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_SYMBOL);

    switch (value->typeTag)
    {
        case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
        {
            abort();
        }

        case OCTASPIRE_DERN_VALUE_TAG_STRING:
        {
            return octaspire_string_concatenate(
                self->value.symbol,
                value->value.string);
        }

        case OCTASPIRE_DERN_VALUE_TAG_SYMBOL:
        {
            return octaspire_string_concatenate(
                self->value.symbol,
                value->value.symbol);
        }

        case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
        {
            return octaspire_string_concatenate(
                self->value.symbol,
                value->value.character);
        }

        case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
        {
            bool result = true;
            for (size_t i = 0; i < octaspire_dern_value_as_vector_get_length(value); ++i)
            {
                if (!octaspire_dern_value_as_symbol_push_back(
                        self,
                        octaspire_dern_value_as_vector_get_element_at(
                            value,
                            (ptrdiff_t)i)))
                {
                    result = false;
                }
            }

            return result;
        }

        case OCTASPIRE_DERN_VALUE_TAG_NIL:
        case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
        case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
        case OCTASPIRE_DERN_VALUE_TAG_REAL:
        case OCTASPIRE_DERN_VALUE_TAG_ERROR:
        case OCTASPIRE_DERN_VALUE_TAG_HASH_MAP:
        case OCTASPIRE_DERN_VALUE_TAG_QUEUE:
        case OCTASPIRE_DERN_VALUE_TAG_LIST:
        case OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT:
        case OCTASPIRE_DERN_VALUE_TAG_FUNCTION:
        case OCTASPIRE_DERN_VALUE_TAG_SPECIAL:
        case OCTASPIRE_DERN_VALUE_TAG_BUILTIN:
        case OCTASPIRE_DERN_VALUE_TAG_PORT:
        case OCTASPIRE_DERN_VALUE_TAG_C_DATA:
        case OCTASPIRE_DERN_VALUE_TAG_SEMVER:
        {
            octaspire_string_t *tmpStr =
                octaspire_dern_value_to_string(
                    value,
                    octaspire_dern_vm_get_allocator(self->vm));

            bool const result = octaspire_string_concatenate(
                self->value.symbol,
                tmpStr);

            octaspire_string_release(tmpStr);
            tmpStr = 0;

            return result;
        }
    }

    return false;
}

bool octaspire_dern_value_as_symbol_pop_back(
    octaspire_dern_value_t * const self)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_SYMBOL);
    return octaspire_string_pop_back_ucs_character(self->value.symbol);
}

bool octaspire_dern_value_as_symbol_pop_front(
    octaspire_dern_value_t * const self)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_SYMBOL);
    return octaspire_string_pop_front_ucs_character(self->value.symbol);
}

bool octaspire_dern_value_as_string_pop_back_ucs_character(
    octaspire_dern_value_t * const self)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_STRING);

    return octaspire_string_pop_back_ucs_character(self->value.string);
}

bool octaspire_dern_value_as_string_pop_front_ucs_character(
    octaspire_dern_value_t * const self)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_STRING);

    return octaspire_string_pop_front_ucs_character(self->value.string);
}

bool octaspire_dern_value_as_string_remove_all_substrings(
    octaspire_dern_value_t * const self,
    octaspire_dern_value_t * const value)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_STRING);

    switch (value->typeTag)
    {
        case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
        {
            abort();
        }

        case OCTASPIRE_DERN_VALUE_TAG_STRING:
        {
            octaspire_string_remove_all_substrings(
                self->value.string,
                value->value.string);
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
        {
            octaspire_string_remove_all_substrings(
                self->value.string,
                value->value.character);
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_NIL:
        case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
        case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
        case OCTASPIRE_DERN_VALUE_TAG_REAL:
        case OCTASPIRE_DERN_VALUE_TAG_ERROR:
        case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
        case OCTASPIRE_DERN_VALUE_TAG_SYMBOL:
        case OCTASPIRE_DERN_VALUE_TAG_HASH_MAP:
        case OCTASPIRE_DERN_VALUE_TAG_QUEUE:
        case OCTASPIRE_DERN_VALUE_TAG_LIST:
        case OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT:
        case OCTASPIRE_DERN_VALUE_TAG_FUNCTION:
        case OCTASPIRE_DERN_VALUE_TAG_SPECIAL:
        case OCTASPIRE_DERN_VALUE_TAG_BUILTIN:
        case OCTASPIRE_DERN_VALUE_TAG_PORT:
        case OCTASPIRE_DERN_VALUE_TAG_C_DATA:
        case OCTASPIRE_DERN_VALUE_TAG_SEMVER:
        {
            return false;
        }
    }

    return true;
}

bool octaspire_dern_value_as_string_is_index_valid(
    octaspire_dern_value_t const * const self,
    ptrdiff_t const possiblyNegativeIndex)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_STRING);

    return octaspire_string_is_index_valid(
        self->value.string,
        possiblyNegativeIndex);
}

bool octaspire_dern_value_as_string_is_empty(
    octaspire_dern_value_t const * const self)
{
    octaspire_helpers_verify_true(
        self->typeTag == OCTASPIRE_DERN_VALUE_TAG_STRING);

    return octaspire_string_is_empty(self->value.string);
}

octaspire_semver_t const *octaspire_dern_value_as_semver_const(
    octaspire_dern_value_t const * const self)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_SEMVER);
    return self->value.semver;
}

octaspire_semver_t *octaspire_dern_value_as_semver(
    octaspire_dern_value_t const * const self)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_SEMVER);
    return self->value.semver;
}

char const *octaspire_dern_value_as_string_get_c_string(
    octaspire_dern_value_t const * const self)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_STRING);
    return octaspire_string_get_c_string(self->value.string);
}

size_t octaspire_dern_value_as_string_get_length_in_octets(
    octaspire_dern_value_t const * const self)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_STRING);
    return octaspire_string_get_length_in_octets(self->value.string);
}

char const *octaspire_dern_value_as_symbol_get_c_string(
    octaspire_dern_value_t const * const self)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_SYMBOL);
    return octaspire_string_get_c_string(self->value.symbol);
}

bool octaspire_dern_value_is_symbol_and_equal_to_c_string(
    octaspire_dern_value_t const * const self,
    char const * const str)
{
    if (self->typeTag != OCTASPIRE_DERN_VALUE_TAG_SYMBOL)
    {
        return false;
    }

    return octaspire_string_is_equal_to_c_string(self->value.symbol, str);
}

bool octaspire_dern_value_as_symbol_is_equal_to_c_string(
    octaspire_dern_value_t const * const self,
    char const * const str)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_SYMBOL);
    return octaspire_string_is_equal_to_c_string(self->value.symbol, str);
}

bool octaspire_dern_value_as_text_is_equal_to_c_string(
    octaspire_dern_value_t const * const self,
    char const * const str)
{
    switch (self->typeTag)
    {
        case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
        {
            abort();
        }

        case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
        {
            return octaspire_string_is_equal_to_c_string(self->value.character, str);
        }

        case OCTASPIRE_DERN_VALUE_TAG_STRING:
        {
            return octaspire_string_is_equal_to_c_string(self->value.string, str);
        }

        case OCTASPIRE_DERN_VALUE_TAG_SEMVER:
        {
            return octaspire_semver_is_equal_to_c_string(self->value.semver, str);
        }

        case OCTASPIRE_DERN_VALUE_TAG_SYMBOL:
        {
            return octaspire_string_is_equal_to_c_string(self->value.symbol, str);
        }

        case OCTASPIRE_DERN_VALUE_TAG_NIL:
        case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
        case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
        case OCTASPIRE_DERN_VALUE_TAG_REAL:
        case OCTASPIRE_DERN_VALUE_TAG_ERROR:
        case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
        case OCTASPIRE_DERN_VALUE_TAG_HASH_MAP:
        case OCTASPIRE_DERN_VALUE_TAG_QUEUE:
        case OCTASPIRE_DERN_VALUE_TAG_LIST:
        case OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT:
        case OCTASPIRE_DERN_VALUE_TAG_FUNCTION:
        case OCTASPIRE_DERN_VALUE_TAG_SPECIAL:
        case OCTASPIRE_DERN_VALUE_TAG_BUILTIN:
        case OCTASPIRE_DERN_VALUE_TAG_PORT:
        case OCTASPIRE_DERN_VALUE_TAG_C_DATA:
        {
            octaspire_helpers_verify_true(false);
        }
        break;
    }

    return 0;
}

char const *octaspire_dern_value_as_text_get_c_string(
    octaspire_dern_value_t const * const self)
{
    switch (self->typeTag)
    {
        case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
        {
            abort();
        }

        case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
        {
            return octaspire_string_get_c_string(self->value.character);
        }

        case OCTASPIRE_DERN_VALUE_TAG_STRING:
        {
            return octaspire_string_get_c_string(self->value.string);
        }

        case OCTASPIRE_DERN_VALUE_TAG_SYMBOL:
        {
            return octaspire_string_get_c_string(self->value.symbol);
        }

        case OCTASPIRE_DERN_VALUE_TAG_NIL:
        case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
        case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
        case OCTASPIRE_DERN_VALUE_TAG_REAL:
        case OCTASPIRE_DERN_VALUE_TAG_ERROR:
        case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
        case OCTASPIRE_DERN_VALUE_TAG_HASH_MAP:
        case OCTASPIRE_DERN_VALUE_TAG_QUEUE:
        case OCTASPIRE_DERN_VALUE_TAG_LIST:
        case OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT:
        case OCTASPIRE_DERN_VALUE_TAG_FUNCTION:
        case OCTASPIRE_DERN_VALUE_TAG_SPECIAL:
        case OCTASPIRE_DERN_VALUE_TAG_BUILTIN:
        case OCTASPIRE_DERN_VALUE_TAG_PORT:
        case OCTASPIRE_DERN_VALUE_TAG_C_DATA:
        case OCTASPIRE_DERN_VALUE_TAG_SEMVER:
        {
            octaspire_helpers_verify_true(false);
        }
        break;
    }

    return 0;
}

size_t octaspire_dern_value_as_text_get_length_in_octets(
    octaspire_dern_value_t const * const self)
{
    switch (self->typeTag)
    {
        case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
        {
            abort();
        }

        case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
        {
            return octaspire_string_get_length_in_octets(
                self->value.character);
        }

        case OCTASPIRE_DERN_VALUE_TAG_STRING:
        {
            return octaspire_string_get_length_in_octets(
                self->value.string);
        }

        case OCTASPIRE_DERN_VALUE_TAG_SYMBOL:
        {
            return octaspire_string_get_length_in_octets(
                self->value.symbol);
        }

        case OCTASPIRE_DERN_VALUE_TAG_NIL:
        case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
        case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
        case OCTASPIRE_DERN_VALUE_TAG_REAL:
        case OCTASPIRE_DERN_VALUE_TAG_ERROR:
        case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
        case OCTASPIRE_DERN_VALUE_TAG_HASH_MAP:
        case OCTASPIRE_DERN_VALUE_TAG_QUEUE:
        case OCTASPIRE_DERN_VALUE_TAG_LIST:
        case OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT:
        case OCTASPIRE_DERN_VALUE_TAG_FUNCTION:
        case OCTASPIRE_DERN_VALUE_TAG_SPECIAL:
        case OCTASPIRE_DERN_VALUE_TAG_BUILTIN:
        case OCTASPIRE_DERN_VALUE_TAG_PORT:
        case OCTASPIRE_DERN_VALUE_TAG_C_DATA:
        case OCTASPIRE_DERN_VALUE_TAG_SEMVER:
        {
            octaspire_helpers_verify_true(false);
        }
        break;
    }

    return 0;
}

bool octaspire_dern_value_as_vector_is_index_valid(
    octaspire_dern_value_t const * const self,
    ptrdiff_t const possiblyNegativeIndex)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    return octaspire_vector_is_valid_index(
        self->value.vector,
        possiblyNegativeIndex);
}

size_t octaspire_dern_value_as_vector_get_length(
    octaspire_dern_value_t const * const self)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    return octaspire_vector_get_length(self->value.vector);
}

bool octaspire_dern_value_as_vector_is_valid_index(
    octaspire_dern_value_t const * const self,
    ptrdiff_t const index)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    return octaspire_vector_is_valid_index(self->value.vector, index);
}

bool octaspire_dern_value_as_vector_push_front_element(
    octaspire_dern_value_t *self,
    void const *element)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_not_null(element);

    return octaspire_vector_push_front_element(self->value.vector, element);
}

bool octaspire_dern_value_as_vector_push_back_element(
    octaspire_dern_value_t *self,
    void const *element)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_not_null(element);

    return octaspire_vector_push_back_element(self->value.vector, element);
}

bool octaspire_dern_value_as_vector_remove_element_at(
    octaspire_dern_value_t *self,
    ptrdiff_t const possiblyNegativeIndex)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_VECTOR);

    return octaspire_vector_remove_element_at(
        self->value.vector,
        possiblyNegativeIndex);
}

bool octaspire_dern_value_as_vector_pop_back_element(octaspire_dern_value_t *self)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_VECTOR);

    return octaspire_vector_pop_back_element(self->value.vector);
}

bool octaspire_dern_value_as_vector_pop_front_element(octaspire_dern_value_t *self)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_VECTOR);

    return octaspire_vector_pop_front_element(self->value.vector);
}

octaspire_dern_value_t *octaspire_dern_value_as_vector_get_element_at(
    octaspire_dern_value_t * const self,
    ptrdiff_t const possiblyNegativeIndex)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_VECTOR);

    return octaspire_vector_get_element_at(
        self->value.vector,
        possiblyNegativeIndex);
}

octaspire_dern_value_t const *octaspire_dern_value_as_vector_get_element_at_const(
    octaspire_dern_value_t const * const self,
    ptrdiff_t const possiblyNegativeIndex)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_VECTOR);

    return octaspire_vector_get_element_at_const(
        self->value.vector,
        possiblyNegativeIndex);
}

octaspire_dern_c_data_or_unpushed_error_t
octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
    octaspire_dern_value_t const * const self,
    ptrdiff_t const possiblyNegativeIndex,
    char const * const dernFuncName,
    char const * const cDataName,
    char const * const pluginName)
{
    octaspire_helpers_verify_not_null(self);

    octaspire_dern_vm_t * const vm = self->vm;

    octaspire_helpers_verify_not_null(vm);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    octaspire_dern_c_data_or_unpushed_error_t result = {0, 0};

    octaspire_dern_value_t const * const arg =
        octaspire_dern_value_as_vector_get_element_at_const(self, possiblyNegativeIndex);

    octaspire_helpers_verify_not_null(arg);

    if (!octaspire_dern_value_is_c_data(arg))
    {
        result.unpushedError = octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects C data '%s' as "
            "%i. argument. Type '%s' was given.",
            dernFuncName,
            cDataName,
            possiblyNegativeIndex,
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));

        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return result;
    }

    octaspire_dern_c_data_t * cData = arg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData,
            pluginName,
            cDataName))
    {
        result.unpushedError = octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' "
            "expects '%s' and '%s' as "
            "plugin name and payload type name for the C data of the %d. "
            "argument. Names '%s' and '%s' were given.",
            dernFuncName,
            pluginName,
            cDataName,
            possiblyNegativeIndex,
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));

        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return result;
    }

    result.cData = octaspire_dern_c_data_get_payload(cData);
    return result;
}

octaspire_dern_number_or_unpushed_error_t
octaspire_dern_value_as_vector_get_element_at_as_number_or_unpushed_error_const(
    octaspire_dern_value_t const * const self,
    ptrdiff_t const possiblyNegativeIndex,
    char const * const dernFuncName)
{
    octaspire_helpers_verify_not_null(self);

    octaspire_dern_vm_t * const vm = self->vm;

    octaspire_helpers_verify_not_null(vm);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    octaspire_dern_number_or_unpushed_error_t result = {0, 0};

    octaspire_dern_value_t const * const arg =
        octaspire_dern_value_as_vector_get_element_at_const(self, possiblyNegativeIndex);

    octaspire_helpers_verify_not_null(arg);

    if (!octaspire_dern_value_is_number(arg))
    {
        result.unpushedError = octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects number as "
            "%d. argument. Type '%s' was given.",
            dernFuncName,
            possiblyNegativeIndex,
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));

        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return result;
    }

    result.number = octaspire_dern_value_as_number_get_value(arg);
    return result;
}

octaspire_dern_value_t *octaspire_dern_value_as_vector_get_element_of_type_at(
    octaspire_dern_value_t * const self,
    octaspire_dern_value_tag_t const typeTag,
    ptrdiff_t const possiblyNegativeIndex)
{
    octaspire_dern_value_t * result =
        octaspire_dern_value_as_vector_get_element_at(
            self,
            possiblyNegativeIndex);

    octaspire_helpers_verify_not_null(result);

    if (result->typeTag == typeTag)
    {
        return result;
    }

    return 0;
}

octaspire_dern_value_t const *octaspire_dern_value_as_vector_get_element_of_type_at_const(
    octaspire_dern_value_t const * const self,
    octaspire_dern_value_tag_t const typeTag,
    ptrdiff_t const possiblyNegativeIndex)
{
    octaspire_dern_value_t const *result =
        octaspire_dern_value_as_vector_get_element_at_const(
            self,
            possiblyNegativeIndex);

    octaspire_helpers_verify_not_null(result);

    if (result->typeTag == typeTag)
    {
        return result;
    }

    return 0;
}

octaspire_dern_value_tag_t octaspire_dern_value_as_vector_get_element_type_at_const(
    octaspire_dern_value_t const * const self,
    ptrdiff_t const possiblyNegativeIndex)
{
    octaspire_dern_value_t const *result =
        octaspire_dern_value_as_vector_get_element_at_const(
            self,
            possiblyNegativeIndex);

    if (!result)
    {
        return OCTASPIRE_DERN_VALUE_TAG_ILLEGAL;
    }

    return result->typeTag;
}

octaspire_dern_value_t *octaspire_dern_value_as_list_get_element_at(
    octaspire_dern_value_t * const self,
    ptrdiff_t const possiblyNegativeIndex)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_LIST);

    octaspire_list_node_t * const node =
        octaspire_list_get_at(self->value.list, possiblyNegativeIndex);

    if (!node)
    {
        return 0;
    }

    return octaspire_list_node_get_element(node);
}

// TODO how about as_vector, should it have void* replaced with octaspire_dern_value_t*?
bool octaspire_dern_value_as_hash_map_put(
    octaspire_dern_value_t *self,
    uint32_t const hash,
    octaspire_dern_value_t const * const key,
    octaspire_dern_value_t *value)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_HASH_MAP);
    return octaspire_map_put(self->value.hashMap, hash, &key, &value);
}

size_t octaspire_dern_value_as_hash_map_get_number_of_elements(
    octaspire_dern_value_t const * const self)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_HASH_MAP);
    return octaspire_map_get_number_of_elements(self->value.hashMap);
}

octaspire_map_element_t *octaspire_dern_value_as_hash_map_get_at_index(
    octaspire_dern_value_t * const self,
    ptrdiff_t const possiblyNegativeIndex)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_HASH_MAP);

    return octaspire_map_get_at_index(
        self->value.hashMap,
        possiblyNegativeIndex);
}

octaspire_map_element_t *octaspire_dern_value_as_hash_map_get(
    octaspire_dern_value_t * const self,
    uint32_t const hash,
    octaspire_dern_value_t const * const key)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_HASH_MAP);
    return octaspire_map_get(self->value.hashMap, hash, &key);
}

octaspire_map_element_t const * octaspire_dern_value_as_hash_map_get_const(
    octaspire_dern_value_t const * const self,
    uint32_t const hash,
    octaspire_dern_value_t const * const key)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_HASH_MAP);
    return octaspire_map_get_const(self->value.hashMap, hash, &key);
}

octaspire_dern_value_t *octaspire_dern_value_as_hash_map_get_value_for_symbol_key_using_c_string(
    octaspire_dern_value_t * const self,
    char const * const keySymbolsContentAsCString)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_HASH_MAP);

    octaspire_dern_value_t * const key = octaspire_dern_vm_create_new_value_symbol_from_c_string(
        self->vm,
        keySymbolsContentAsCString);

    octaspire_helpers_verify_not_null(key);

    octaspire_map_element_t * const element = octaspire_dern_value_as_hash_map_get(
        self,
        octaspire_dern_value_get_hash(key),
        key);

    if (!element)
    {
        return 0;
    }

    return octaspire_map_element_get_value(element);
}

octaspire_dern_value_t const *
octaspire_dern_value_as_hash_map_get_value_for_symbol_key_using_c_string_const(
    octaspire_dern_value_t const * const self,
    char const * const keySymbolsContentAsCString)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(self->vm);
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_VALUE_TAG_HASH_MAP);

    octaspire_dern_value_t * const key =
        octaspire_dern_vm_create_new_value_symbol_from_c_string(
            self->vm,
            keySymbolsContentAsCString);

    octaspire_helpers_verify_not_null(key);

    octaspire_dern_vm_push_value(self->vm, key);

    octaspire_map_element_t const * const element =
        octaspire_dern_value_as_hash_map_get_const(
            self,
            octaspire_dern_value_get_hash(key),
            key);

    octaspire_dern_vm_pop_value(self->vm, key);
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(self->vm));

    if (!element)
    {
        return 0;
    }

    return octaspire_map_element_get_value(element);
}

size_t octaspire_dern_value_get_length(
    octaspire_dern_value_t const * const self)
{
    switch (self->typeTag)
    {
        case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
        {
            abort();
        }

        case OCTASPIRE_DERN_VALUE_TAG_NIL:
        case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
        case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
        case OCTASPIRE_DERN_VALUE_TAG_REAL:
        case OCTASPIRE_DERN_VALUE_TAG_C_DATA:
        case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
        {
            return 1;
        }

        case OCTASPIRE_DERN_VALUE_TAG_STRING:
        {
            return octaspire_string_get_length_in_ucs_characters(
                self->value.string);
        }
        case OCTASPIRE_DERN_VALUE_TAG_SYMBOL:
        {
            return octaspire_string_get_length_in_ucs_characters(
                self->value.symbol);
        }
        case OCTASPIRE_DERN_VALUE_TAG_ERROR:
        {
            return octaspire_string_get_length_in_ucs_characters(
                self->value.error->message);
        }
        case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
        {
            return octaspire_vector_get_length(self->value.vector);
        }
        case OCTASPIRE_DERN_VALUE_TAG_HASH_MAP:
        {
            return octaspire_map_get_number_of_elements(
                self->value.hashMap);
        }
        case OCTASPIRE_DERN_VALUE_TAG_QUEUE:
        {
            return octaspire_queue_get_length(self->value.queue);
        }
        case OCTASPIRE_DERN_VALUE_TAG_LIST:
        {
            return octaspire_list_get_length(self->value.list);
        }
        case OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT:
        {
            return octaspire_dern_environment_get_length(self->value.environment);
        }
        case OCTASPIRE_DERN_VALUE_TAG_FUNCTION:
        {
            return octaspire_dern_function_get_number_of_required_arguments(
                self->value.function);
        }
        case OCTASPIRE_DERN_VALUE_TAG_SPECIAL:
        {
            return self->value.special->numRequiredActualArguments;
        }
        case OCTASPIRE_DERN_VALUE_TAG_BUILTIN:
        {
            return self->value.builtin->numRequiredActualArguments;
        }
        case OCTASPIRE_DERN_VALUE_TAG_PORT:
        {
            return (size_t)octaspire_dern_port_get_length_in_octets(self->value.port);
        }
        case OCTASPIRE_DERN_VALUE_TAG_SEMVER:
        {
            return
                3 +
                octaspire_semver_get_num_pre_release_identifiers(self->value.semver) +
                octaspire_semver_get_num_build_metadata_identifiers(self->value.semver);
        }
    }

    return 0;
}

bool octaspire_dern_value_mark(octaspire_dern_value_t *self)
{
    if (self->mark)
    {
        return true;
    }

    self->mark = true;

    if (self->docstr)
    {
        if (!octaspire_dern_value_mark(self->docstr))
        {
            return false;
        }
    }

    if (self->typeTag == OCTASPIRE_DERN_VALUE_TAG_VECTOR)
    {
        for (size_t i = 0;
             i < octaspire_vector_get_length(self->value.vector);
             ++i)
        {
            octaspire_dern_value_t * const tmpVal =
                octaspire_vector_get_element_at(
                    self->value.vector,
                    (ptrdiff_t)i);

            octaspire_helpers_verify_not_null(tmpVal);

            if (!octaspire_dern_value_mark(tmpVal))
            {
                return false;
            }
        }
    }
    else if (self->typeTag == OCTASPIRE_DERN_VALUE_TAG_HASH_MAP)
    {
        octaspire_map_element_iterator_t iter =
            octaspire_map_element_iterator_init(self->value.hashMap);

        while (iter.element)
        {
            if (!octaspire_dern_value_mark(
                    octaspire_map_element_get_key(iter.element)))
            {
                return false;
            }

            if (!octaspire_dern_value_mark(
                    octaspire_map_element_get_value(iter.element)))
            {
                return false;
            }

            octaspire_map_element_iterator_next(&iter);
        }
    }
    else if (self->typeTag == OCTASPIRE_DERN_VALUE_TAG_QUEUE)
    {
        // TODO Add iterator into queue and use it here
        for (size_t i = 0; i < octaspire_queue_get_length(self->value.queue); ++i)
        {
            octaspire_dern_value_t * const tmpVal =
                octaspire_queue_get_at(
                    self->value.queue,
                    (ptrdiff_t)i);

            octaspire_helpers_verify_not_null(tmpVal);

            if (!octaspire_dern_value_mark(tmpVal))
            {
                return false;
            }
        }
    }
    else if (self->typeTag == OCTASPIRE_DERN_VALUE_TAG_LIST)
    {
        // TODO More efficient iteration
        for (size_t i = 0;
             i < octaspire_list_get_length(self->value.list);
             ++i)
        {
            octaspire_dern_value_t * const tmpVal =
                octaspire_list_node_get_element(
                    octaspire_list_get_at(
                        self->value.list,
                        (ptrdiff_t)i));

            octaspire_helpers_verify_not_null(tmpVal);

            if (!octaspire_dern_value_mark(tmpVal))
            {
                return false;
            }
        }
    }
    else if (self->typeTag == OCTASPIRE_DERN_VALUE_TAG_FUNCTION)
    {
        bool status = true;

        if (!octaspire_dern_value_mark(self->value.function->formals))
        {
            status = false;
        }

        if (!octaspire_dern_value_mark(self->value.function->body))
        {
            status = false;
        }

        if (!octaspire_dern_value_mark(self->value.function->definitionEnvironment))
        {
            status = false;
        }

        return status;
    }
    else if (self->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT)
    {
        return octaspire_dern_environment_mark(self->value.environment);
    }

    return true;
}

static int octaspire_dern_value_private_compare_void_pointers(
    void const * const a,
    void const * const b)
{
    ptrdiff_t const tmp = (ptrdiff_t)a - (ptrdiff_t)b;

    if (tmp < 0)
    {
        return -1;
    }

    if (tmp > 0)
    {
        return 1;
    }

    return 0;
}

int octaspire_dern_value_compare(
    octaspire_dern_value_t const * const self,
    octaspire_dern_value_t const * const other)
{
    octaspire_helpers_verify_not_null(self);
    octaspire_helpers_verify_not_null(other);

    if (self == other)
    {
        return 0;
    }

    if (octaspire_dern_value_is_number(self))
    {
        if (octaspire_dern_value_is_number(other))
        {
            if (octaspire_dern_value_is_real(self) ||
                octaspire_dern_value_is_real(other))
            {
                double const a = octaspire_dern_value_as_number_get_value(self);
                double const b = octaspire_dern_value_as_number_get_value(other);

                if (a < b)
                {
                    return -1;
                }

                if (a > b)
                {
                    return 1;
                }

                return 0;
            }
            else
            {
                return octaspire_dern_value_as_integer_get_value(self) -
                    octaspire_dern_value_as_integer_get_value(other);
            }
        }
    }

    if (self->typeTag != other->typeTag)
    {
        return (int)(self->typeTag) - (int)(other->typeTag);
    }

    switch (self->typeTag)
    {
        case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
        {
            abort();
        }

        case OCTASPIRE_DERN_VALUE_TAG_NIL:
        {
            return 0;
        }
        case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
        {
            return self->value.boolean - other->value.boolean;
        }
        case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
        {
            return self->value.integer - other->value.integer;
        }
        case OCTASPIRE_DERN_VALUE_TAG_REAL:
        {
            if (self->value.real < other->value.real)
            {
                return -1;
            }

            if (self->value.real > other->value.real)
            {
                return 1;
            }

            return 0;
        }
        case OCTASPIRE_DERN_VALUE_TAG_STRING:
        {
            return octaspire_string_compare(
                self->value.string,
                other->value.string);
        }
        case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
        {
            return octaspire_string_compare(
                self->value.character,
                other->value.character);
        }
        case OCTASPIRE_DERN_VALUE_TAG_SYMBOL:
        {
            return octaspire_string_compare(
                self->value.symbol,
                other->value.symbol);
        }
        case OCTASPIRE_DERN_VALUE_TAG_ERROR:
        {
            return octaspire_dern_error_message_compare(
                self->value.error,
                other->value.error);
        }
        case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
        {
            if (octaspire_dern_value_as_vector_get_length(self) !=
                octaspire_dern_value_as_vector_get_length(other))
            {
                return octaspire_dern_value_as_vector_get_length(self) -
                    octaspire_dern_value_as_vector_get_length(other);
            }

            for (size_t i = 0; i < octaspire_dern_value_as_vector_get_length(self); ++i)
            {
                octaspire_dern_value_t const * const valA =
                    octaspire_dern_value_as_vector_get_element_at_const(
                        self,
                        i);

                octaspire_dern_value_t const * const valB =
                    octaspire_dern_value_as_vector_get_element_at_const(
                        other,
                        i);

                octaspire_helpers_verify_not_null(valA);
                octaspire_helpers_verify_not_null(valB);

                int const cmp = octaspire_dern_value_compare(valA, valB);

                if (cmp)
                {
                    return cmp;
                }
            }

            return 0;
        }
        case OCTASPIRE_DERN_VALUE_TAG_HASH_MAP:
        {
            return octaspire_dern_helpers_compare_value_hash_maps(
                self->value.hashMap,
                other->value.hashMap);
        }
        case OCTASPIRE_DERN_VALUE_TAG_QUEUE:
        {
            if (octaspire_dern_value_as_queue_get_length(self) !=
                octaspire_dern_value_as_queue_get_length(other))
            {
                return octaspire_dern_value_as_queue_get_length(self) -
                    octaspire_dern_value_as_queue_get_length(other);
            }

            octaspire_queue_iterator_t myIter =
                octaspire_queue_iterator_init(self->value.queue);

            octaspire_queue_iterator_t otherIter =
                octaspire_queue_iterator_init(other->value.queue);

            while (myIter.iterator.currentNode)
            {
                octaspire_helpers_verify_not_null(otherIter.iterator.currentNode);

                octaspire_dern_value_t const * const myVal =
                    octaspire_list_node_get_element(
                        myIter.iterator.currentNode);

                octaspire_dern_value_t const * const otherVal =
                    octaspire_list_node_get_element(
                        otherIter.iterator.currentNode);

                octaspire_helpers_verify_not_null(myVal);
                octaspire_helpers_verify_not_null(otherVal);

                int const cmp = octaspire_dern_value_compare(myVal, otherVal);

                if (cmp)
                {
                    return cmp;
                }

                octaspire_queue_iterator_next(&myIter);
                octaspire_queue_iterator_next(&otherIter);
            }

            return 0;
        }
        case OCTASPIRE_DERN_VALUE_TAG_LIST:
        {
            if (octaspire_dern_value_as_list_get_length(self) !=
                octaspire_dern_value_as_list_get_length(other))
            {
                return octaspire_dern_value_as_list_get_length(self) -
                    octaspire_dern_value_as_list_get_length(other);
            }

            octaspire_list_node_iterator_t myIter =
                octaspire_list_node_iterator_init(self->value.list);

            octaspire_list_node_iterator_t otherIter =
                octaspire_list_node_iterator_init(other->value.list);

            while (myIter.currentNode)
            {
                octaspire_helpers_verify_not_null(otherIter.currentNode);

                octaspire_dern_value_t const * const myVal =
                    octaspire_list_node_get_element(
                        myIter.currentNode);

                octaspire_dern_value_t const * const otherVal =
                    octaspire_list_node_get_element(
                        otherIter.currentNode);

                octaspire_helpers_verify_not_null(myVal);
                octaspire_helpers_verify_not_null(otherVal);

                int const cmp = octaspire_dern_value_compare(myVal, otherVal);

                if (cmp)
                {
                    return cmp;
                }

                octaspire_list_node_iterator_next(&myIter);
                octaspire_list_node_iterator_next(&otherIter);
            }

            return 0;
        }
        case OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT:
        {
            return octaspire_dern_environment_compare(
                    self->value.environment,
                    other->value.environment);
        }
        case OCTASPIRE_DERN_VALUE_TAG_FUNCTION:
        {
            return octaspire_dern_function_compare(
                       self->value.function, other->value.function);
        }
        case OCTASPIRE_DERN_VALUE_TAG_SPECIAL:
        {
            return octaspire_dern_value_private_compare_void_pointers(
                       self->value.special, other->value.special);
        }
        case OCTASPIRE_DERN_VALUE_TAG_BUILTIN:
        {
            return octaspire_dern_value_private_compare_void_pointers(
                       self->value.builtin, other->value.builtin);
        }
        case OCTASPIRE_DERN_VALUE_TAG_PORT:
        {
            return octaspire_dern_value_private_compare_void_pointers(
                       self->value.port, other->value.port);
        }
        case OCTASPIRE_DERN_VALUE_TAG_C_DATA:
        {
            return octaspire_dern_c_data_compare(self->value.cData, other->value.cData);
        }
        case OCTASPIRE_DERN_VALUE_TAG_SEMVER:
        {
            return octaspire_semver_compare(self->value.semver, other->value.semver);
        }
    }

    return 0;
}

bool octaspire_dern_value_is_atom(octaspire_dern_value_t const * const self)
{
    switch (self->typeTag)
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
        case OCTASPIRE_DERN_VALUE_TAG_SYMBOL:
        case OCTASPIRE_DERN_VALUE_TAG_ERROR:
        case OCTASPIRE_DERN_VALUE_TAG_PORT: // TODO XXX atom or not? Think about renaming this func.
        {
            return true;
        }

        case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
        case OCTASPIRE_DERN_VALUE_TAG_HASH_MAP:
        case OCTASPIRE_DERN_VALUE_TAG_QUEUE:
        case OCTASPIRE_DERN_VALUE_TAG_LIST:
        case OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT:
        case OCTASPIRE_DERN_VALUE_TAG_FUNCTION:
        case OCTASPIRE_DERN_VALUE_TAG_SPECIAL:
        case OCTASPIRE_DERN_VALUE_TAG_BUILTIN:
        case OCTASPIRE_DERN_VALUE_TAG_C_DATA:
        case OCTASPIRE_DERN_VALUE_TAG_SEMVER:
        {
            return false;
        }
    }

    abort();
    return false;
}

