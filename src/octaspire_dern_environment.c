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
#include "octaspire/dern/octaspire_dern_environment.h"
#include <assert.h>
#include <inttypes.h>
#include <string.h>
#include <octaspire/core/octaspire_helpers.h>
#include "octaspire/dern/octaspire_dern_vm.h"
#include "octaspire/dern/octaspire_dern_value.h"
#include "octaspire/dern/octaspire_dern_helpers.h"

static octaspire_container_utf8_string_t *octaspire_dern_environment_private_to_string(
    octaspire_dern_environment_t const * const self,
    size_t const depth);

static int octaspire_dern_environment_helper_compare_function(
    void const * const a,
    void const * const b);

octaspire_dern_environment_t *octaspire_dern_environment_new(
    octaspire_dern_value_t *enclosing,
    octaspire_dern_vm_t *vm,
    octaspire_memory_allocator_t *allocator)
{
    octaspire_dern_environment_t *self =
        octaspire_memory_allocator_malloc(allocator, sizeof(octaspire_dern_environment_t));

    if (!self)
    {
        return 0;
    }

    self->allocator = allocator;
    self->vm        = vm;
    self->enclosing = enclosing;

    self->bindings  = octaspire_container_hash_map_new(
        sizeof(octaspire_dern_value_t*),
        true,
        sizeof(octaspire_dern_value_t*),
        true,
        (octaspire_container_hash_map_key_compare_function_t)octaspire_dern_value_is_equal,
        (octaspire_container_hash_map_key_hash_function_t)octaspire_dern_value_get_hash,
        0,
        0,
        allocator);

    return self;
}

octaspire_dern_environment_t *octaspire_dern_environment_new_copy(
    octaspire_dern_environment_t * const other,
    struct octaspire_dern_vm_t * const vm,
    octaspire_memory_allocator_t * const allocator)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_dern_environment_t *self =
        octaspire_memory_allocator_malloc(allocator, sizeof(octaspire_dern_environment_t));

    if (!self)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return 0;
    }

    self->allocator = allocator;
    self->vm        = vm;

    self->enclosing = octaspire_dern_vm_create_new_value_copy(vm, other->enclosing);


    self->bindings  = octaspire_container_hash_map_new(
        sizeof(octaspire_dern_value_t*),
        true,
        sizeof(octaspire_dern_value_t*),
        true,
        (octaspire_container_hash_map_key_compare_function_t)octaspire_dern_value_is_equal,
        (octaspire_container_hash_map_key_hash_function_t)octaspire_dern_value_get_hash,
        0,
        0,
        allocator);

    octaspire_container_hash_map_element_iterator_t iter =
        octaspire_container_hash_map_element_iterator_init(other->bindings);

    do
    {
        if (iter.element)
        {
            octaspire_dern_value_t * const keyVal =
                octaspire_container_hash_map_element_get_key(iter.element);

            octaspire_dern_value_t * const copyOfKeyVal =
                octaspire_dern_vm_create_new_value_copy(vm, keyVal);

            octaspire_dern_vm_push_value(vm, copyOfKeyVal);

            octaspire_dern_value_t * const valVal =
                octaspire_container_hash_map_element_get_value(iter.element);

            octaspire_dern_value_t * const copyOfValVal =
                octaspire_dern_vm_create_new_value_copy(vm, valVal);

            octaspire_dern_vm_push_value(vm, copyOfValVal);

            octaspire_dern_vm_pop_value(vm, copyOfValVal);
            octaspire_dern_vm_pop_value(vm, copyOfKeyVal);

            if (octaspire_container_hash_map_put(
                    self->bindings,
                    octaspire_dern_value_get_hash(copyOfKeyVal),
                    &copyOfKeyVal,
                    &copyOfValVal))
            {
                abort();
            }

        }
    }
    while (octaspire_container_hash_map_element_iterator_next(&iter));


    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return self;
}

void octaspire_dern_environment_release(octaspire_dern_environment_t *self)
{
    if (!self)
    {
        return;
    }

    octaspire_container_hash_map_release(self->bindings);
    //octaspire_dern_environment_release(self->enclosing);
    octaspire_memory_allocator_free(self->allocator, self);
}

// Returns 0 or error
octaspire_dern_value_t *octaspire_dern_environment_extend(
    octaspire_dern_environment_t *self,
    octaspire_dern_value_t *formals,
    octaspire_dern_value_t *arguments)
{
    assert(formals->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    assert(arguments->typeTag == OCTASPIRE_DERN_VALUE_TAG_VECTOR);

    octaspire_container_vector_t *formalsVec   = formals->value.vector;
    octaspire_container_vector_t *argumentsVec = arguments->value.vector;

    assert(formalsVec && argumentsVec);

    size_t const numFormalArgs   = octaspire_container_vector_get_length(formalsVec);

    size_t numDotArgs            = 0;
    size_t numNormalArgs         = 0;
    size_t numNormalArgsAfterDot = 0;

    for (size_t i = 0; i < numFormalArgs; ++i)
    {
        octaspire_dern_value_t *formal =
            octaspire_container_vector_get_element_at(formalsVec, (ptrdiff_t)i);

        assert(formal->typeTag == OCTASPIRE_DERN_VALUE_TAG_SYMBOL);

        octaspire_container_utf8_string_t const * const formalAsStr = formal->value.string;

        if (octaspire_container_utf8_string_is_equal_to_c_string(formalAsStr, "..."))
        {
            ++numDotArgs;
        }
        else
        {
            if (numDotArgs)
            {
                ++numNormalArgsAfterDot;
            }
            else
            {
                ++numNormalArgs;
            }
        }
    }

    if (numDotArgs > 1)
    {
        return octaspire_dern_vm_create_new_value_error_format(
            self->vm,
            "Function can have only one formal ... argument for varargs. Now %zu were given.",
            numDotArgs);
    }

    if (numNormalArgsAfterDot > 0)
    {
        return octaspire_dern_vm_create_new_value_error_format(
            self->vm,
            "Function can have no formal arguments after ... "
            "for varargs. Now %zu formals were given after ...",
            numNormalArgsAfterDot);
    }

    if (numDotArgs)
    {
        assert(numNormalArgs);
        // for the vector name
        --numNormalArgs;
    }

    if (numDotArgs == 0 && numNormalArgs != numFormalArgs)
    {
        return octaspire_dern_vm_create_new_value_error_format(
            self->vm,
            "Number of formal and actual arguments must be equal for functions without "
            "varargs using the ...-formal. Now %zu formal and %zu actual arguments were given.",
            numFormalArgs,
            numNormalArgs);
    }

    size_t const numActualArgumentsGiven = octaspire_container_vector_get_length(argumentsVec);

    // If varargs, subtract 2 for the ... and the formal name before it.
    size_t const numArgumentsRequiredAtLeast = numFormalArgs - (numDotArgs ? 2 : 0);

    if (numActualArgumentsGiven < numArgumentsRequiredAtLeast)
    {
        return octaspire_dern_vm_create_new_value_error_format(
            self->vm,
            "Function expects %zu arguments. Now %zu arguments were given.",
            numArgumentsRequiredAtLeast,
            numActualArgumentsGiven);
    }

    for (size_t i = 0; i < numNormalArgs; ++i)
    {
        assert(i < octaspire_container_vector_get_length(formalsVec));
        assert(i < octaspire_container_vector_get_length(argumentsVec));

        octaspire_dern_value_t *formal =
            octaspire_container_vector_get_element_at(
                formalsVec,
                (ptrdiff_t)i);

        octaspire_dern_value_t *actual =
            octaspire_container_vector_get_element_at(
                argumentsVec,
                (ptrdiff_t)i);

        if (!octaspire_dern_environment_set(self, formal, actual))
        {
            abort();
        }
    }

    size_t const numActualRestArgs = numActualArgumentsGiven - numNormalArgs;

    if (numActualRestArgs)
    {
        if (numDotArgs != 1)
        {
            // TODO XXX better error message
            return octaspire_dern_vm_create_new_value_error_format(
                self->vm,
                "Function can have zero or one dot args. Now %zu dot-arguments were given.",
                numDotArgs);
        }

        octaspire_dern_value_t *formal = octaspire_container_vector_get_element_at(
            formalsVec,
            (ptrdiff_t)(octaspire_container_vector_get_length(formalsVec) - 2));

        octaspire_container_vector_t *actualVec = octaspire_container_vector_new_with_preallocated_elements(
            sizeof(octaspire_dern_value_t*),
            true,
            numNormalArgsAfterDot,
            0,
            self->allocator);

        octaspire_dern_value_t *actual = octaspire_dern_vm_create_new_value_vector_from_vector(self->vm, actualVec);

        //octaspire_dern_vm_push_value(self->vm, actual);

        for (size_t i = numNormalArgs; i < octaspire_container_vector_get_length(argumentsVec); ++i)
        {
            octaspire_dern_value_t *actualAfterDot =
                octaspire_container_vector_get_element_at(
                    argumentsVec,
                    (ptrdiff_t)i);

            if (!octaspire_container_vector_push_back_element(actualVec, &actualAfterDot))
            {
                abort();
            }
        }

        if (!octaspire_dern_environment_set(self, formal, actual))
        {
            abort();
        }
        //octaspire_dern_vm_pop_value(self->vm, actual); // actual
    }
    else if (numDotArgs > 0)
    {
        // Add empty rest-vector

        assert(numDotArgs == 1);

        octaspire_dern_value_t *formal = octaspire_container_vector_get_element_at(
            formalsVec,
            (ptrdiff_t)(octaspire_container_vector_get_length(formalsVec) - 2));

        octaspire_dern_value_t *actual = octaspire_dern_vm_create_new_value_vector(self->vm);

        //octaspire_dern_vm_push_value(self->vm, actual);

        if (!octaspire_dern_environment_set(self, formal, actual))
        {
            abort();
        }
        //octaspire_dern_vm_pop_value(self->vm, actual); // actual
    }

    return 0;
}

octaspire_dern_value_t *octaspire_dern_environment_get(
    octaspire_dern_environment_t *self,
    octaspire_dern_value_t const * const key)
{
    octaspire_container_hash_map_element_t *element = octaspire_container_hash_map_get(
        self->bindings,
        octaspire_dern_value_get_hash(key),
        &key);

    if (!element)
    {
        if (self->enclosing)
        {
            assert(self->enclosing->value.environment);
            return octaspire_dern_environment_get(self->enclosing->value.environment, key);
        }

        return 0;
    }

    return octaspire_container_hash_map_element_get_value(element);
}

bool octaspire_dern_environment_set(
    octaspire_dern_environment_t *self,
    octaspire_dern_value_t const * const key,
    octaspire_dern_value_t *value)
{
    uint32_t const hash = octaspire_dern_value_get_hash(key);

    // TODO XXX should this be made more efficient? Now the element is searched
    // twice. There could be a method octaspire_container_hash_map_put_overwriting etc.
    octaspire_container_hash_map_remove(self->bindings, hash, &key);

    return octaspire_container_hash_map_put(
        self->bindings,
        octaspire_dern_value_get_hash(key),
        &key,
        &value);
}

static int octaspire_dern_environment_helper_compare_function(
    void const * const a,
    void const * const b)
{
    octaspire_container_hash_map_element_t const * const elemA =
        *(octaspire_container_hash_map_element_t const * const *)a;

    octaspire_container_hash_map_element_t const * const elemB =
        *(octaspire_container_hash_map_element_t const * const *)b;

    octaspire_dern_value_t const * const keyA =
       (octaspire_dern_value_t*)octaspire_container_hash_map_element_get_key(elemA);

    octaspire_dern_value_t const * const keyB =
       (octaspire_dern_value_t*)octaspire_container_hash_map_element_get_key(elemB);

    return octaspire_dern_value_compare(keyA, keyB);
}

static octaspire_container_utf8_string_t *octaspire_dern_environment_private_to_string(
    octaspire_dern_environment_t const * const self,
    size_t const depth)
{
    octaspire_container_utf8_string_t *indent = octaspire_container_utf8_string_new("", self->allocator);

    for (size_t i = 0; i < depth; ++i)
    {
        octaspire_container_utf8_string_concatenate_c_string(indent, "\t");
    }

    octaspire_container_utf8_string_t *result = octaspire_container_utf8_string_new_format(
        self->allocator,
        "%s---------- environment ----------\n",
        octaspire_container_utf8_string_get_c_string(indent));

    if (self->enclosing)
    {
        octaspire_container_utf8_string_t *enclosing =
            octaspire_dern_environment_private_to_string(self->enclosing->value.environment, depth + 1);

        octaspire_container_utf8_string_concatenate_format(
            result,
            "%s",
            octaspire_container_utf8_string_get_c_string(enclosing));

        octaspire_container_utf8_string_release(enclosing);
        enclosing = 0;
    }

    octaspire_container_vector_t *sortVec = octaspire_container_vector_new(
        sizeof(octaspire_container_hash_map_element_t*),
        true,
        0,
        self->allocator);

    size_t numCharsInLongestKey = 0;
    for (size_t i = 0; i < octaspire_container_hash_map_get_number_of_elements(self->bindings); ++i)
    {
        octaspire_container_hash_map_element_t const * const element =
            octaspire_container_hash_map_get_at_index(
                self->bindings,
                (ptrdiff_t)i);

        assert(element);

        numCharsInLongestKey = octaspire_helpers_max_size_t(
            numCharsInLongestKey,
            octaspire_dern_value_get_length(octaspire_container_hash_map_element_get_key(element)));

        if (!octaspire_container_vector_push_back_element(sortVec, &element))
        {
            abort();
        }
    }

    octaspire_container_vector_sort(
        sortVec,
        (octaspire_container_vector_element_compare_function_t)octaspire_dern_environment_helper_compare_function);

    for (size_t i = 0; i < octaspire_container_vector_get_length(sortVec); ++i)
    {
        octaspire_container_hash_map_element_t const * const element =
            octaspire_container_vector_get_element_at(
                sortVec,
                (ptrdiff_t)i);

        octaspire_dern_value_t const * const key =
            octaspire_container_hash_map_element_get_key(element);

        octaspire_dern_value_t const * const value =
            octaspire_container_hash_map_element_get_value(element);

        octaspire_container_utf8_string_t *keyAsStr = octaspire_dern_value_to_string(key, self->allocator);

        octaspire_container_utf8_string_t *valueAsStr =
            octaspire_dern_value_to_string(value, self->allocator);

        octaspire_container_utf8_string_concatenate_format(
            result,
            "%s%-*s -> %s\n",
            octaspire_container_utf8_string_get_c_string(indent),
            (int)numCharsInLongestKey,
            octaspire_container_utf8_string_get_c_string(keyAsStr),
            octaspire_container_utf8_string_get_c_string(valueAsStr));

        octaspire_container_utf8_string_release(keyAsStr);
        keyAsStr = 0;

        octaspire_container_utf8_string_release(valueAsStr);
        valueAsStr = 0;
    }

    octaspire_container_utf8_string_concatenate_format(
        result,
        "%s---------------------------------\n",
        octaspire_container_utf8_string_get_c_string(indent));

    octaspire_container_utf8_string_release(indent);
    indent = 0;

    octaspire_container_vector_release(sortVec);
    sortVec = 0;

    return result;
}

octaspire_container_utf8_string_t *octaspire_dern_environment_to_string(
    octaspire_dern_environment_t const * const self)
{
    return octaspire_dern_environment_private_to_string(self, 0);
}

bool octaspire_dern_environment_print(
    octaspire_dern_environment_t const * const self)
{
    octaspire_container_utf8_string_t *str = octaspire_dern_environment_to_string(self);

    if (!str)
    {
        return false;
    }

    if (printf("%s", octaspire_container_utf8_string_get_c_string(str)) < 0)
    {
        return false;
    }

    return true;
}


size_t octaspire_dern_environment_get_length(
    octaspire_dern_environment_t const * const self)
{
    return octaspire_container_hash_map_get_number_of_elements(self->bindings);
}

octaspire_container_hash_map_element_t *octaspire_dern_environment_get_at_index(
    octaspire_dern_environment_t * const self,
    ptrdiff_t const index)
{
    return octaspire_container_hash_map_get_at_index(self->bindings, index);
}

bool octaspire_dern_environment_mark(octaspire_dern_environment_t *self)
{
    if (!self)
    {
        return true;
    }

    bool statusKey = true;
    bool statusVal = true;

    octaspire_container_hash_map_element_iterator_t iter =
        octaspire_container_hash_map_element_iterator_init(self->bindings);

    while (iter.element)
    {
        octaspire_dern_value_t * const key =
            octaspire_container_hash_map_element_get_key(iter.element);

        octaspire_dern_value_t * const val =
            octaspire_container_hash_map_element_get_value(iter.element);

        statusKey = octaspire_dern_value_mark(key);
        statusVal = octaspire_dern_value_mark(val);

        octaspire_container_hash_map_element_iterator_next(&iter);
    }

    if (self->enclosing && self->enclosing->value.environment != self)
    {
        return octaspire_dern_value_mark(self->enclosing) && statusKey && statusVal;
    }

    return true;
}

bool octaspire_dern_environment_is_equal(
    octaspire_dern_environment_t const * const self,
    octaspire_dern_environment_t const * const other)
{
    return octaspire_dern_helpers_are_value_hash_maps_equal(
        self->bindings,
        other->bindings);
}

