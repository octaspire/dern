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
#include "octaspire/dern/octaspire_dern_stdlib.h"
#include <assert.h>
#include <inttypes.h>
#include <octaspire/core/octaspire_helpers.h>
#include <octaspire/core/octaspire_container_list.h>
#include <octaspire/core/octaspire_container_queue.h>
#include "octaspire/dern/octaspire_dern_vm.h"
#include "octaspire/dern/octaspire_dern_config.h"

#ifdef OCTASPIRE_DERN_CONFIG_BINARY_PLUGINS
#include <dlfcn.h>
#endif

static octaspire_dern_value_t *octaspire_dern_stdlib_private_validate_function(
    octaspire_dern_vm_t* vm,
    octaspire_dern_function_t *function);

static octaspire_dern_value_t *octaspire_dern_vm_private_special_define_with_four_arguments(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

static octaspire_dern_value_t *octaspire_dern_vm_private_special_define_with_five_arguments(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

static octaspire_dern_value_t *octaspire_dern_vm_builtin_private_plus_numerical(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

static octaspire_dern_value_t *octaspire_dern_vm_builtin_private_plus_textual(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

static octaspire_dern_value_t *octaspire_dern_vm_builtin_private_minus_numerical(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

static octaspire_dern_value_t *octaspire_dern_vm_builtin_private_minus_textual(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

static octaspire_dern_value_t *octaspire_dern_vm_builtin_private_require_is_already_loaded(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

static octaspire_dern_value_t *octaspire_dern_vm_builtin_private_require_source_file(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);

static octaspire_dern_value_t *octaspire_dern_vm_builtin_private_require_binary_file(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment);


octaspire_dern_value_t *octaspire_dern_vm_private_special_define_with_four_arguments(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    octaspire_helpers_verify_true(octaspire_container_vector_get_length(vec) == 4);

    octaspire_dern_value_t *targetEnv = octaspire_container_vector_get_element_at(vec, 0);

    octaspire_helpers_verify_true(targetEnv && targetEnv->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_dern_value_t *name = octaspire_container_vector_get_element_at(vec, 1);

    bool nameIsEvaluated = false;

    if (name->typeTag != OCTASPIRE_DERN_VALUE_TAG_SYMBOL)
    {
        nameIsEvaluated = true;

        if (name->typeTag != OCTASPIRE_DERN_VALUE_TAG_VECTOR)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Special 'define': (define [optional-target-env] symbol...) name to be defined should be symbol or vector to be evaluated. Type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(name->typeTag));
        }

        name = octaspire_dern_vm_eval(vm, name, environment);

        if (name->typeTag != OCTASPIRE_DERN_VALUE_TAG_SYMBOL)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Special 'define': (define [optional-target-env] symbol...) vector for name to be defined should evaluate into symbol. Type '%s' was result of evaluation.",
                octaspire_dern_value_helper_get_type_as_c_string(name->typeTag));
        }

        octaspire_dern_vm_push_value(vm, name);
    }

    octaspire_dern_value_t *docstr = octaspire_container_vector_get_element_at(vec, 2);

    if (docstr->typeTag != OCTASPIRE_DERN_VALUE_TAG_STRING)
    {
        if (nameIsEvaluated)
        {
            octaspire_dern_vm_pop_value(vm, name);
        }

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Special 'define': (define [optional-target-env] symbol docstring...) docstring must be string. Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(docstr->typeTag));
    }

    octaspire_dern_value_t *valueToBeDefined = octaspire_container_vector_get_element_at(vec, 3);

    octaspire_dern_value_t * const valueToBeDefinedToBePopped = valueToBeDefined;

    octaspire_dern_vm_push_value(vm, environment);
    octaspire_dern_vm_push_value(vm, valueToBeDefined);

    valueToBeDefined = octaspire_dern_vm_eval(vm, valueToBeDefined, environment);

    octaspire_dern_vm_pop_value(vm, valueToBeDefinedToBePopped);

    if (octaspire_dern_value_is_atom(valueToBeDefined))
    {
        octaspire_dern_vm_push_value(vm, valueToBeDefined);
        octaspire_dern_value_t * const popThisVal = valueToBeDefined;
        valueToBeDefined = octaspire_dern_vm_create_new_value_copy(vm, valueToBeDefined);
        octaspire_dern_vm_pop_value(vm, popThisVal);
    }

    octaspire_dern_vm_pop_value(vm, environment);

    if (valueToBeDefined->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
    {
        if (nameIsEvaluated)
        {
            octaspire_dern_vm_pop_value(vm, name);
        }

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return valueToBeDefined;
    }
    else if (valueToBeDefined->typeTag == OCTASPIRE_DERN_VALUE_TAG_FUNCTION)
    {
        if (nameIsEvaluated)
        {
            octaspire_dern_vm_pop_value(vm, name);
        }

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "At definition of function '%s': functions cannot be defined with "
            "the three-argument function. Use four-argument function.",
            octaspire_dern_value_as_symbol_get_c_string(name));
    }

    valueToBeDefined->docstr = docstr;

    bool const status = octaspire_dern_environment_set(
        targetEnv->value.environment,
        name,
        valueToBeDefined);

    if (nameIsEvaluated)
    {
        octaspire_dern_vm_pop_value(vm, name);
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, status);
}

octaspire_dern_value_t *octaspire_dern_vm_private_special_define_with_five_arguments(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    octaspire_helpers_verify_true(octaspire_container_vector_get_length(vec) == 5);

    octaspire_dern_value_t *targetEnv = octaspire_container_vector_get_element_at(vec, 0);

    octaspire_helpers_verify_true(targetEnv && targetEnv->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_dern_value_t *name = octaspire_container_vector_get_element_at(vec, 1);

    bool nameIsEvaluated = false;

    if (name->typeTag != OCTASPIRE_DERN_VALUE_TAG_SYMBOL)
    {
        nameIsEvaluated = true;

        if (name->typeTag != OCTASPIRE_DERN_VALUE_TAG_VECTOR)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Special 'define': (define [optional-target-env] name...) Name must be symbol or vector to be evaluated. Type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(name->typeTag));
        }

        name = octaspire_dern_vm_eval(vm, name, environment);


        if (name->typeTag != OCTASPIRE_DERN_VALUE_TAG_SYMBOL)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Special 'define': (define [optional-target-env] name...) Vector for name must evaluate into symbol. Now it evaluated into type '%s'.",
                octaspire_dern_value_helper_get_type_as_c_string(name->typeTag));
        }

        octaspire_dern_vm_push_value(vm, name);
    }

    octaspire_dern_value_t *docstr = octaspire_container_vector_get_element_at(vec, 2);

    if (docstr->typeTag != OCTASPIRE_DERN_VALUE_TAG_STRING)
    {
        if (nameIsEvaluated)
        {
            octaspire_dern_vm_pop_value(vm, name);
        }

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Special 'define': (define [optional-target-env] name docstring...) docstring must be string. Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(docstr->typeTag));
    }

    octaspire_dern_value_t *docVecArg = octaspire_container_vector_get_element_at(vec, 3);

    octaspire_dern_vm_push_value(vm, docVecArg);
    octaspire_dern_vm_push_value(vm, environment);

    octaspire_dern_value_t *docVec = octaspire_dern_vm_eval(vm, docVecArg, environment);

    octaspire_dern_vm_pop_value(vm, environment);
    octaspire_dern_vm_pop_value(vm, docVecArg);

    if (docVec->typeTag != OCTASPIRE_DERN_VALUE_TAG_VECTOR)
    {
        if (nameIsEvaluated)
        {
            octaspire_dern_vm_pop_value(vm, name);
        }

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

        if (docVec->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
        {
            return docVec;
        }

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Special 'define': (define [optional-target-env] name docstring docvec...). DocVec must be vector. Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(docVec->typeTag));
    }

    octaspire_dern_value_t *valueToBeDefined = octaspire_container_vector_get_element_at(vec, 4);

    if (octaspire_dern_value_is_atom(valueToBeDefined))
    {
        valueToBeDefined = octaspire_dern_vm_create_new_value_copy(vm, valueToBeDefined);
    }

    octaspire_dern_vm_push_value(vm, docVec);
    octaspire_dern_vm_push_value(vm, valueToBeDefined);

    octaspire_dern_value_t *oldValueToBeDefined = valueToBeDefined;

    valueToBeDefined = octaspire_dern_vm_eval(vm, valueToBeDefined, environment);

    octaspire_dern_vm_pop_value(vm, oldValueToBeDefined);
    octaspire_dern_vm_pop_value(vm, docVec);

    if (valueToBeDefined->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
    {
        if (nameIsEvaluated)
        {
            octaspire_dern_vm_pop_value(vm, name);
        }

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return valueToBeDefined;
    }
    else if (valueToBeDefined->typeTag == OCTASPIRE_DERN_VALUE_TAG_FUNCTION)
    {
        octaspire_container_utf8_string_t *errorMessage =
            octaspire_dern_function_are_all_formals_mentioned_in_docvec(
                valueToBeDefined->value.function,
                docVec);

        octaspire_helpers_verify_not_null(errorMessage);

        if (!octaspire_container_utf8_string_is_empty(errorMessage))
        {
            if (nameIsEvaluated)
            {
                octaspire_dern_vm_pop_value(vm, name);
            }

            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error(vm, errorMessage);
        }
        else
        {
            octaspire_container_utf8_string_release(errorMessage);
            errorMessage = 0;

            octaspire_container_utf8_string_t *strToModify = docstr->value.string;

            if (!octaspire_container_utf8_string_concatenate_c_string(
                strToModify,
                "\nArguments are:"))
            {
                abort();
            }


            for (size_t i = 0; i < octaspire_dern_value_get_length(docVec); i += 2)
            {
                if (!octaspire_container_utf8_string_concatenate_c_string(
                    strToModify,
                    "\n"))
                {
                    abort();
                }

                octaspire_dern_value_t const * const formalSym =
                    octaspire_dern_value_as_vector_get_element_of_type_at_const(
                        docVec,
                        OCTASPIRE_DERN_VALUE_TAG_SYMBOL,
                        (ptrdiff_t)i);

                octaspire_helpers_verify_not_null(formalSym);

                octaspire_dern_value_t const * const formalDocStr =
                    octaspire_dern_value_as_vector_get_element_of_type_at_const(
                        docVec,
                        OCTASPIRE_DERN_VALUE_TAG_STRING,
                        (ptrdiff_t)(i + 1));

                octaspire_helpers_verify_not_null(formalDocStr);

                if (!octaspire_container_utf8_string_concatenate_format(
                    strToModify,
                    "%s -> %s",
                    octaspire_container_utf8_string_get_c_string(formalSym->value.symbol),
                    octaspire_container_utf8_string_get_c_string(formalDocStr->value.string)))
                {
                    abort();
                }
            }
        }
    }
    else
    {
        if (nameIsEvaluated)
        {
            octaspire_dern_vm_pop_value(vm, name);
        }

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Four/Five argument 'define' must be used to define functions. Definition of type '%s' was tried.",
            octaspire_dern_value_helper_get_type_as_c_string(valueToBeDefined->typeTag));
    }

    valueToBeDefined->docstr = docstr;
    valueToBeDefined->docvec = docVec;

    bool const status = octaspire_dern_environment_set(
        targetEnv->value.environment,
        name,
        valueToBeDefined);

    if (nameIsEvaluated)
    {
        octaspire_dern_vm_pop_value(vm, name);
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, status);
}

octaspire_dern_value_t *octaspire_dern_vm_special_define(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    size_t const numArgs = octaspire_container_vector_get_length(vec);

    if (numArgs != 3 && numArgs != 4 && numArgs != 5)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Special 'define' expects three, four, or five arguments. %zu arguments were given.",
            numArgs);
    }

    // TODO XXX is there need to push arguments?
    octaspire_dern_vm_push_value(vm, arguments);

    octaspire_dern_value_t *firstArg = octaspire_container_vector_get_element_at(vec, 0);
    octaspire_helpers_verify_not_null(firstArg);
    octaspire_dern_value_t *evaluatedFirstArg = octaspire_dern_vm_eval(vm, firstArg, environment);
    octaspire_helpers_verify_not_null(evaluatedFirstArg);

    if (evaluatedFirstArg->typeTag != OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT)
    {
        if (!octaspire_dern_value_as_vector_push_front_element(arguments, &environment))
        {
            abort();
        }

        if (numArgs == 3)
        {
            octaspire_dern_value_t *result = octaspire_dern_vm_private_special_define_with_four_arguments(
                vm,
                arguments,
                environment);

            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return result;
        }
        else if (numArgs == 4)
        {
            octaspire_dern_value_t *result = octaspire_dern_vm_private_special_define_with_five_arguments(
                vm,
                arguments,
                environment);

            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return result;
        }
        else
        {
            abort();
        }
    }
    else
    {
        // TODO if it would be checked that is the first already an env, there would not
        // be need to replace it with evaluated one (itself).
        // TODO should there be octaspire_dern_value_as_vector_replace_element_at...?
        if (!octaspire_container_vector_replace_element_at(arguments->value.vector, 0, &evaluatedFirstArg))
        {
            abort();
        }

        if (numArgs == 4)
        {
            octaspire_dern_value_t *result = octaspire_dern_vm_private_special_define_with_four_arguments(
                vm,
                arguments,
                environment);

            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return result;
        }
        else if (numArgs == 5)
        {
            octaspire_dern_value_t *result = octaspire_dern_vm_private_special_define_with_five_arguments(
                vm,
                arguments,
                environment);

            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return result;
        }
        else
        {
            abort();
        }
    }
}

octaspire_dern_value_t *octaspire_dern_vm_special_eval(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    if (octaspire_container_vector_get_length(vec) != 1 &&
        octaspire_container_vector_get_length(vec) != 2)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Special 'eval' expects one or two arguments. %zu arguments were given.",
            octaspire_container_vector_get_length(vec));
    }

    octaspire_dern_value_t *valueToBeEvaluated = octaspire_container_vector_get_element_at(vec, 0);
    octaspire_helpers_verify_not_null(valueToBeEvaluated);

    octaspire_dern_vm_push_value(vm, arguments);

    octaspire_dern_value_t *result = 0;

    if (octaspire_container_vector_get_length(vec) == 2)
    {
        octaspire_dern_value_t *envVal = octaspire_container_vector_get_element_at(vec, 1);
        octaspire_helpers_verify_not_null(envVal);

        if (envVal->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT)
        {
            result = octaspire_dern_vm_eval(vm, valueToBeEvaluated, envVal);
            octaspire_dern_vm_push_value(vm, result);
            octaspire_dern_value_t *tmpResult = octaspire_dern_vm_eval(vm, result, envVal);
            octaspire_dern_vm_pop_value(vm, result);
            result = tmpResult;
        }
        else
        {
            envVal = octaspire_dern_vm_eval(vm, envVal, environment);

            octaspire_helpers_verify_not_null(envVal);

            if (envVal->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
            {
                octaspire_dern_vm_pop_value(vm, arguments);
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

                return envVal;
            }

            if (envVal->typeTag != OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT)
            {
                octaspire_dern_vm_pop_value(vm, arguments);
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Second argument to special 'eval' must evaluate into environment value.\n"
                    "Now it evaluated into type %s.",
                    octaspire_dern_value_helper_get_type_as_c_string(envVal->typeTag));
            }

            octaspire_dern_vm_push_value(vm, envVal);
            result = octaspire_dern_vm_eval(vm, valueToBeEvaluated, envVal);
            octaspire_dern_vm_push_value(vm, result);
            octaspire_dern_value_t *tmpResult = octaspire_dern_vm_eval(vm, result, envVal);
            octaspire_dern_vm_pop_value(vm, result);
            octaspire_dern_vm_pop_value(vm, envVal);
            result = tmpResult;
        }
    }
    else
    {
        result = octaspire_dern_vm_eval(vm, valueToBeEvaluated, environment);
        octaspire_dern_vm_push_value(vm, result);
        octaspire_dern_value_t *tmpResult = octaspire_dern_vm_eval(vm, result, environment);
        octaspire_dern_vm_pop_value(vm, result);
        result = tmpResult;
    }

    octaspire_helpers_verify_not_null(result);

    octaspire_dern_vm_pop_value(vm, arguments);
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_special_quote(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    if (octaspire_container_vector_get_length(vec) != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Special 'quote' expects one argument. %zu arguments were given.",
            octaspire_container_vector_get_length(vec));
    }

    octaspire_dern_value_t *quotedValue = octaspire_container_vector_get_element_at(vec, 0);

    octaspire_helpers_verify_not_null(quotedValue);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return quotedValue;
}

octaspire_dern_value_t *octaspire_dern_vm_special_select(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs < 2 || numArgs % 2 != 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Special 'select' expects at least two arguments and the number of arguments must be "
            "multiple of two. Now %zu arguments were given.",
            numArgs);
    }

    octaspire_dern_vm_push_value(vm, arguments);

    for (size_t i = 0; i < numArgs; i += 2)
    {
        octaspire_dern_value_t * testResult =
            octaspire_dern_value_as_vector_get_element_at(arguments, (ptrdiff_t)i);

        octaspire_helpers_verify_not_null(testResult);

        if (testResult->typeTag == OCTASPIRE_DERN_VALUE_TAG_SYMBOL &&
            octaspire_container_utf8_string_is_equal_to_c_string(testResult->value.symbol, "default"))
        {
            if (i != (numArgs-2))
            {
                octaspire_dern_vm_pop_value(vm, arguments);
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_from_c_string(
                    vm,
                    "'default' must be the last selector in special 'select'.");
            }
            else
            {
                octaspire_dern_value_t * result = octaspire_dern_vm_eval(
                    vm,
                    octaspire_dern_value_as_vector_get_element_at(
                        arguments,
                        (ptrdiff_t)(i + 1)),
                    environment);

                octaspire_dern_vm_pop_value(vm, arguments);
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return result;
            }
        }
        else
        {
            testResult = octaspire_dern_vm_eval(
                vm,
                octaspire_dern_value_as_vector_get_element_at(
                    arguments,
                    (ptrdiff_t)i),
                environment);

            octaspire_helpers_verify_not_null(testResult);

            if (testResult->typeTag != OCTASPIRE_DERN_VALUE_TAG_BOOLEAN)
            {
                if (testResult->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                {
                    octaspire_dern_vm_pop_value(vm, arguments);
                    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                    return testResult;
                }
                else
                {
                    octaspire_dern_vm_pop_value(vm, arguments);
                    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                    return octaspire_dern_vm_create_new_value_error_format(
                        vm,
                        "Selectors of special 'select' must evaluate into booleans. Type '%s' was given.",
                        octaspire_dern_value_helper_get_type_as_c_string(testResult->typeTag));
                }
            }

            if (testResult->value.boolean)
            {
                octaspire_dern_value_t * result = octaspire_dern_vm_eval(
                    vm,
                    octaspire_dern_value_as_vector_get_element_at(
                        arguments,
                        (ptrdiff_t)(i + 1)),
                    environment);

                octaspire_dern_vm_pop_value(vm, arguments);
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return result;
            }
        }
    }

    octaspire_dern_vm_pop_value(vm, arguments);
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_nil(vm);
}

octaspire_dern_value_t *octaspire_dern_vm_special_if(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs != 2 && numArgs != 3)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Special 'if' expects two or three arguments. %zu arguments were given.",
            octaspire_dern_value_get_length(arguments));
    }

    octaspire_dern_vm_push_value(vm, arguments);

    octaspire_dern_value_t * testResult = octaspire_dern_vm_eval(
        vm,
        octaspire_dern_value_as_vector_get_element_at(arguments, 0),
        environment);

    octaspire_helpers_verify_not_null(testResult);
    octaspire_dern_vm_push_value(vm, testResult);

    if (testResult->typeTag == OCTASPIRE_DERN_VALUE_TAG_FUNCTION)
    {
        // Allow calling with   (fn () x)  instead of   ((fn (x) x))
        octaspire_dern_value_t *wrapperVecVal = octaspire_dern_vm_create_new_value_vector(vm);
        octaspire_dern_vm_push_value(vm, wrapperVecVal);
        octaspire_dern_value_as_vector_push_back_element(wrapperVecVal, &testResult);

        octaspire_dern_value_t * tmpVal = octaspire_dern_vm_eval(
            vm,
            wrapperVecVal,
            environment);

        octaspire_dern_vm_pop_value(vm, wrapperVecVal);
        // TODO is this needed, or does vector popping pop this too? Check...
        //octaspire_dern_vm_pop_value(vm, testResult);
        octaspire_dern_vm_pop_value(vm, testResult);
        testResult = tmpVal;
    }
    else
    {
        octaspire_dern_vm_pop_value(vm, testResult);
    }

    octaspire_helpers_verify_not_null(testResult);

    if (testResult->typeTag != OCTASPIRE_DERN_VALUE_TAG_BOOLEAN)
    {
        octaspire_dern_vm_pop_value(vm, arguments);

        if (testResult->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return testResult;
        }

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "First argument to special 'if' must evaluate into boolean value. "
            "Now it evaluated into type %s.",
            octaspire_dern_value_helper_get_type_as_c_string(testResult->typeTag));
    }

    octaspire_dern_value_t *result = 0;

    if (octaspire_dern_value_get_length(arguments) == 2)
    {
        if (testResult->value.boolean)
        {
            result = octaspire_dern_vm_eval(
                vm,
                octaspire_dern_value_as_vector_get_element_at(arguments, 1),
                environment);
        }
        else
        {
            result = octaspire_dern_vm_get_value_nil(vm);
        }
    }
    else
    {
        result = octaspire_dern_vm_eval(
            vm,
            octaspire_dern_value_as_vector_get_element_at(arguments, (testResult->value.boolean) ? 1 : 2),
            environment);
    }

    octaspire_dern_vm_pop_value(vm, arguments);

    octaspire_helpers_verify_not_null(result);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_special_while(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs < 2)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Special 'while' expects at least two arguments. %zu arguments were given.",
            octaspire_dern_value_get_length(arguments));
    }

    octaspire_dern_vm_push_value(vm, arguments);

    bool testStat = false;
    int32_t counter = 0;

    while (true)
    {
        octaspire_dern_value_t * const testResult = octaspire_dern_vm_eval(
            vm,
            octaspire_dern_value_as_vector_get_element_at(arguments, 0),
            environment);

        octaspire_helpers_verify_not_null(testResult);

        if (testResult->typeTag != OCTASPIRE_DERN_VALUE_TAG_BOOLEAN)
        {
            octaspire_dern_vm_pop_value(vm, arguments);

            if (testResult->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return testResult;
            }

            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "First argument to special 'while' must evaluate into boolean value. "
                "Now it evaluated into type %s.",
                octaspire_dern_value_helper_get_type_as_c_string(testResult->typeTag));
        }

        testStat = testResult->value.boolean;

        if (!testStat)
        {
            break;
        }

        for (size_t i = 1; i < numArgs; ++i)
        {
            octaspire_dern_value_t *result = octaspire_dern_vm_eval(
                vm,
                octaspire_dern_value_as_vector_get_element_at(
                    arguments,
                    (ptrdiff_t)i),
                environment);

            octaspire_helpers_verify_not_null(result);

            if (result->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
            {
                octaspire_dern_vm_pop_value(vm, arguments);
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return result;

            }

            if (octaspire_dern_vm_get_function_return(vm))
            {
                result = octaspire_dern_vm_get_function_return(vm);
                //octaspire_dern_vm_set_function_return(vm, 0);
                octaspire_dern_vm_pop_value(vm, arguments);
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return result;
            }
        }

        ++counter;
    };

    octaspire_dern_vm_pop_value(vm, arguments);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_integer(vm, counter);
}

octaspire_dern_value_t *octaspire_dern_vm_special_for(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t stepSize = 1; // Used for containers and numerical iteration.

    octaspire_dern_vm_push_value(vm, arguments);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs < 4)
    {
        octaspire_dern_vm_pop_value(vm, arguments);

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Special 'for' expects at least four (for iterating container or port) or five (for iterating numeric range) arguments. %zu arguments were given.",
            numArgs);
    }

    // 1. argument: counter symbol
    octaspire_dern_value_t *counterSymbol =
        octaspire_dern_value_as_vector_get_element_at(arguments, 0);

    octaspire_helpers_verify_not_null(counterSymbol);

    if (counterSymbol->typeTag != OCTASPIRE_DERN_VALUE_TAG_SYMBOL)
    {
        octaspire_dern_vm_pop_value(vm, arguments);

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "First argument to special 'for' must be symbol value. "
            "Now it has type %s.",
            octaspire_dern_value_helper_get_type_as_c_string(counterSymbol->typeTag));
    }

    // 2. argument: 'in' or 'from' symbol
    octaspire_dern_value_t const * const inOrFromSymbol =
        octaspire_dern_value_as_vector_get_element_at(arguments, 1);

    octaspire_helpers_verify_not_null(inOrFromSymbol);

    if (inOrFromSymbol->typeTag != OCTASPIRE_DERN_VALUE_TAG_SYMBOL)
    {
        octaspire_dern_vm_pop_value(vm, arguments);

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Second argument to special 'for' must be symbol 'in' or 'from'. "
            "Now it has type %s.",
            octaspire_dern_value_helper_get_type_as_c_string(inOrFromSymbol->typeTag));
    }

    if (octaspire_container_utf8_string_is_equal_to_c_string(inOrFromSymbol->value.symbol, "in"))
    {
        // (for i in '(1 2 3))
        // (for i in (hash-map a 1 b 2))
        // (for i in [kissa])

        // container
        octaspire_dern_value_t *container =
            octaspire_dern_value_as_vector_get_element_at(arguments, 2);

        octaspire_helpers_verify_not_null(container);

        container = octaspire_dern_vm_eval(vm, container, environment);

        octaspire_helpers_verify_not_null(container);

        octaspire_dern_vm_push_value(vm, container);

        if (container->typeTag != OCTASPIRE_DERN_VALUE_TAG_STRING      && 
            container->typeTag != OCTASPIRE_DERN_VALUE_TAG_VECTOR      &&
            container->typeTag != OCTASPIRE_DERN_VALUE_TAG_LIST        &&
            container->typeTag != OCTASPIRE_DERN_VALUE_TAG_QUEUE       &&
            container->typeTag != OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT &&
            container->typeTag != OCTASPIRE_DERN_VALUE_TAG_HASH_MAP    &&
            container->typeTag != OCTASPIRE_DERN_VALUE_TAG_PORT)
        {
            octaspire_dern_value_t *result = octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Third argument to special 'for' using 'in' must be a container "
                "(string, vector, list, queue, hash map or environment) or a port. "
                "Now it has type %s.",
                octaspire_dern_value_helper_get_type_as_c_string(container->typeTag));

            octaspire_dern_vm_pop_value(vm, container);
            octaspire_dern_vm_pop_value(vm, arguments);

            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return result;
        }




        // Check if there are optional 'step' and then integer
        size_t currentArgIdx = 3;

        // Optional 3 and 4. arguments: step integer
        octaspire_dern_value_t const * const optionalStep =
            octaspire_dern_value_as_vector_get_element_at(
                arguments,
                (ptrdiff_t)currentArgIdx);

        octaspire_helpers_verify_not_null(optionalStep);

        if (optionalStep->typeTag == OCTASPIRE_DERN_VALUE_TAG_SYMBOL &&
            octaspire_container_utf8_string_is_equal_to_c_string(optionalStep->value.symbol, "step"))
        {
            if (numArgs < 5)
            {
                octaspire_dern_vm_pop_value(vm, container);
                octaspire_dern_vm_pop_value(vm, arguments);

                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Special 'for' expects at least five arguments for iterating containers or ports with "
                    "a given step size. %zu arguments were given.",
                    numArgs);
            }

            // There is 'step'. Now an integer is required next
            ++currentArgIdx;

            octaspire_dern_value_t const * const requiredStepSize =
                octaspire_dern_value_as_vector_get_element_at(
                    arguments,
                    (ptrdiff_t)currentArgIdx);

            octaspire_helpers_verify_not_null(requiredStepSize);

            if (requiredStepSize->typeTag != OCTASPIRE_DERN_VALUE_TAG_INTEGER)
            {
                octaspire_dern_value_t *result = octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Fifth argument to special 'for' using 'step' with containers must be "
                    "an integer step size. Now it has type %s.",
                    octaspire_dern_value_helper_get_type_as_c_string(requiredStepSize->typeTag));

                octaspire_dern_vm_pop_value(vm, container);
                octaspire_dern_vm_pop_value(vm, arguments);

                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return result;
            }

            if (requiredStepSize->value.integer <= 0)
            {
                octaspire_dern_value_t *result = octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "The 'step' of special 'for' must be larger than zero. "
                    "Now it is %" PRId32 ".",
                    requiredStepSize->value.integer);

                octaspire_dern_vm_pop_value(vm, container);
                octaspire_dern_vm_pop_value(vm, arguments);

                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return result;
            }

            stepSize = (size_t)requiredStepSize->value.integer;
        }




        // Extend env for the counter variable
        octaspire_dern_environment_t *extendedEnvironment =
            octaspire_dern_environment_new(
                environment,
                vm,
                octaspire_dern_vm_get_allocator(vm));

        octaspire_helpers_verify_not_null(extendedEnvironment);

        octaspire_dern_value_t *extendedEnvVal =
            octaspire_dern_vm_create_new_value_environment_from_environment(vm, extendedEnvironment);

        octaspire_helpers_verify_not_null(extendedEnvVal);

        octaspire_dern_vm_push_value(vm, extendedEnvVal);





        if (container->typeTag == OCTASPIRE_DERN_VALUE_TAG_STRING)
        {
            octaspire_container_utf8_string_t const * const str = container->value.string;
            size_t const strLen = octaspire_container_utf8_string_get_length_in_ucs_characters(str);

            int32_t counter = 0;

            for (size_t i = 0; i < strLen; i += stepSize)
            {
                octaspire_container_utf8_string_t *charStr = octaspire_container_utf8_string_new(
                    "",
                    octaspire_dern_vm_get_allocator(vm));

                octaspire_container_utf8_string_push_back_ucs_character(
                    charStr,
                    octaspire_container_utf8_string_get_ucs_character_at_index(
                        str,
                        (ptrdiff_t)i));

                octaspire_dern_environment_set(
                    extendedEnvironment,
                    counterSymbol,
                    octaspire_dern_vm_create_new_value_character(vm, charStr));

                for (size_t j = currentArgIdx; j < numArgs; ++j)
                {
                    octaspire_dern_value_t *result = octaspire_dern_vm_eval(
                        vm,
                        octaspire_dern_value_as_vector_get_element_at(
                            arguments,
                            (ptrdiff_t)j),
                        extendedEnvVal);

                    octaspire_helpers_verify_not_null(result);

                    if (result->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                    {
                        octaspire_dern_vm_pop_value(vm, extendedEnvVal);
                        octaspire_dern_vm_pop_value(vm, container);
                        octaspire_dern_vm_pop_value(vm, arguments);
                        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                        return result;
                    }

                    if (octaspire_dern_vm_get_function_return(vm))
                    {
                        result = octaspire_dern_vm_get_function_return(vm);
                        //octaspire_dern_vm_set_function_return(vm, 0);
                        octaspire_dern_vm_pop_value(vm, extendedEnvVal);
                        octaspire_dern_vm_pop_value(vm, container);
                        octaspire_dern_vm_pop_value(vm, arguments);
                        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                        return result;
                    }
                }

                ++counter;
            }

            octaspire_dern_vm_pop_value(vm, extendedEnvVal);
            octaspire_dern_vm_pop_value(vm, container);
            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_integer(vm, counter);
        }
        else if (container->typeTag == OCTASPIRE_DERN_VALUE_TAG_VECTOR)
        {
            octaspire_container_vector_t * const vec = container->value.vector;
            size_t const vecLen = octaspire_container_vector_get_length(vec);

            int32_t counter = 0;

            for (size_t i = 0; i < vecLen; i += stepSize)
            {
                octaspire_dern_environment_set(
                    extendedEnvironment,
                    counterSymbol,
                    octaspire_container_vector_get_element_at(
                        vec,
                        (ptrdiff_t)i));

                for (size_t j = currentArgIdx; j < numArgs; ++j)
                {
                    octaspire_dern_value_t *result = octaspire_dern_vm_eval(
                        vm,
                        octaspire_dern_value_as_vector_get_element_at(
                            arguments,
                            (ptrdiff_t)j),
                        extendedEnvVal);

                    octaspire_helpers_verify_not_null(result);

                    if (result->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                    {
                        octaspire_dern_vm_pop_value(vm, extendedEnvVal);
                        octaspire_dern_vm_pop_value(vm, container);
                        octaspire_dern_vm_pop_value(vm, arguments);
                        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                        return result;
                    }

                    if (octaspire_dern_vm_get_function_return(vm))
                    {
                        result = octaspire_dern_vm_get_function_return(vm);
                        //octaspire_dern_vm_set_function_return(vm, 0);
                        octaspire_dern_vm_pop_value(vm, extendedEnvVal);
                        octaspire_dern_vm_pop_value(vm, container);
                        octaspire_dern_vm_pop_value(vm, arguments);
                        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                        return result;
                    }
                }

                ++counter;
            }

            octaspire_dern_vm_pop_value(vm, extendedEnvVal);
            octaspire_dern_vm_pop_value(vm, container);
            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_integer(vm, counter);
        }



        else if (container->typeTag == OCTASPIRE_DERN_VALUE_TAG_LIST)
        {
            octaspire_container_list_t * const list = container->value.list;
            size_t const listLen = octaspire_container_list_get_length(list);

            int32_t counter = 0;

            // TODO more efficient iteration of list
            for (size_t i = 0; i < listLen; i += stepSize)
            {
                octaspire_container_list_node_t *node =
                    octaspire_container_list_get_at(
                        list,
                        (ptrdiff_t)i);

                octaspire_helpers_verify_not_null(node);

                octaspire_dern_environment_set(
                    extendedEnvironment,
                    counterSymbol,
                    octaspire_container_list_node_get_element(node));

                for (size_t j = currentArgIdx; j < numArgs; ++j)
                {
                    octaspire_dern_value_t *result = octaspire_dern_vm_eval(
                        vm,
                        octaspire_dern_value_as_vector_get_element_at(
                            arguments,
                            (ptrdiff_t)j),
                        extendedEnvVal);

                    octaspire_helpers_verify_not_null(result);

                    if (result->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                    {
                        octaspire_dern_vm_pop_value(vm, extendedEnvVal);
                        octaspire_dern_vm_pop_value(vm, container);
                        octaspire_dern_vm_pop_value(vm, arguments);
                        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                        return result;
                    }

                    if (octaspire_dern_vm_get_function_return(vm))
                    {
                        result = octaspire_dern_vm_get_function_return(vm);
                        //octaspire_dern_vm_set_function_return(vm, 0);
                        octaspire_dern_vm_pop_value(vm, extendedEnvVal);
                        octaspire_dern_vm_pop_value(vm, container);
                        octaspire_dern_vm_pop_value(vm, arguments);
                        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                        return result;
                    }
                }

                ++counter;
            }

            octaspire_dern_vm_pop_value(vm, extendedEnvVal);
            octaspire_dern_vm_pop_value(vm, container);
            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_integer(vm, counter);
        }
        else if (container->typeTag == OCTASPIRE_DERN_VALUE_TAG_QUEUE)
        {
            octaspire_container_queue_t * const queue = container->value.queue;
            size_t const queueLen = octaspire_container_queue_get_length(queue);

            int32_t counter = 0;

            // TODO more efficient iteration of queue
            for (size_t i = 0; i < queueLen; i += stepSize)
            {
                octaspire_dern_environment_set(
                    extendedEnvironment,
                    counterSymbol,
                    octaspire_container_queue_get_at(
                        queue,
                        (ptrdiff_t)i));

                for (size_t j = currentArgIdx; j < numArgs; ++j)
                {
                    octaspire_dern_value_t *result = octaspire_dern_vm_eval(
                        vm,
                        octaspire_dern_value_as_vector_get_element_at(
                            arguments,
                            (ptrdiff_t)j),
                        extendedEnvVal);

                    octaspire_helpers_verify_not_null(result);

                    if (result->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                    {
                        octaspire_dern_vm_pop_value(vm, extendedEnvVal);
                        octaspire_dern_vm_pop_value(vm, container);
                        octaspire_dern_vm_pop_value(vm, arguments);
                        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                        return result;
                    }

                    if (octaspire_dern_vm_get_function_return(vm))
                    {
                        result = octaspire_dern_vm_get_function_return(vm);
                        //octaspire_dern_vm_set_function_return(vm, 0);
                        octaspire_dern_vm_pop_value(vm, extendedEnvVal);
                        octaspire_dern_vm_pop_value(vm, container);
                        octaspire_dern_vm_pop_value(vm, arguments);
                        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                        return result;
                    }
                }

                ++counter;
            }

            octaspire_dern_vm_pop_value(vm, extendedEnvVal);
            octaspire_dern_vm_pop_value(vm, container);
            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_integer(vm, counter);
        }










        else if (container->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT)
        {
            octaspire_dern_environment_t * const env = container->value.environment;
            size_t const envLen = octaspire_dern_environment_get_length(env);

            int32_t counter = 0;

            for (size_t i = 0; i < envLen; i += stepSize)
            {
                octaspire_container_hash_map_element_t *element =
                    octaspire_dern_environment_get_at_index(
                        env,
                        (ptrdiff_t)i);

                octaspire_dern_environment_set(
                    extendedEnvironment,
                    counterSymbol,
                    octaspire_dern_vm_create_new_value_vector_from_values(
                        vm,
                        2,
                        octaspire_container_hash_map_element_get_key(element),
                        octaspire_container_hash_map_element_get_value(element)));

                for (size_t j = currentArgIdx; j < numArgs; ++j)
                {
                    octaspire_dern_value_t *result = octaspire_dern_vm_eval(
                        vm,
                        octaspire_dern_value_as_vector_get_element_at(
                            arguments,
                            (ptrdiff_t)j),
                        extendedEnvVal);

                    octaspire_helpers_verify_not_null(result);

                    if (result->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                    {
                        octaspire_dern_vm_pop_value(vm, extendedEnvVal);
                        octaspire_dern_vm_pop_value(vm, container);
                        octaspire_dern_vm_pop_value(vm, arguments);
                        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                        return result;
                    }

                    if (octaspire_dern_vm_get_function_return(vm))
                    {
                        result = octaspire_dern_vm_get_function_return(vm);
                        //octaspire_dern_vm_set_function_return(vm, 0);
                        octaspire_dern_vm_pop_value(vm, extendedEnvVal);
                        octaspire_dern_vm_pop_value(vm, container);
                        octaspire_dern_vm_pop_value(vm, arguments);
                        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                        return result;
                    }
                }

                ++counter;
            }

            octaspire_dern_vm_pop_value(vm, extendedEnvVal);
            octaspire_dern_vm_pop_value(vm, container);
            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_integer(vm, counter);
        }
        else if(container->typeTag == OCTASPIRE_DERN_VALUE_TAG_HASH_MAP)
        {
            octaspire_container_hash_map_t * const hashMap = container->value.hashMap;
            size_t const hashMapLen = octaspire_container_hash_map_get_number_of_elements(hashMap);

            int32_t counter = 0;

            for (size_t i = 0; i < hashMapLen; i += stepSize)
            {
                octaspire_container_hash_map_element_t *element =
                    octaspire_container_hash_map_get_at_index(
                        hashMap,
                        (ptrdiff_t)i);

                octaspire_dern_environment_set(
                    extendedEnvironment,
                    counterSymbol,
                    octaspire_dern_vm_create_new_value_vector_from_values(
                        vm,
                        2,
                        octaspire_container_hash_map_element_get_key(element),
                        octaspire_container_hash_map_element_get_value(element)));

                for (size_t j = currentArgIdx; j < numArgs; ++j)
                {
                    octaspire_dern_value_t *result = octaspire_dern_vm_eval(
                        vm,
                        octaspire_dern_value_as_vector_get_element_at(
                            arguments,
                            (ptrdiff_t)j),
                        extendedEnvVal);

                    octaspire_helpers_verify_not_null(result);

                    if (result->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                    {
                        octaspire_dern_vm_pop_value(vm, extendedEnvVal);
                        octaspire_dern_vm_pop_value(vm, container);
                        octaspire_dern_vm_pop_value(vm, arguments);
                        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                        return result;
                    }

                    if (octaspire_dern_vm_get_function_return(vm))
                    {
                        result = octaspire_dern_vm_get_function_return(vm);
                        //octaspire_dern_vm_set_function_return(vm, 0);
                        octaspire_dern_vm_pop_value(vm, extendedEnvVal);
                        octaspire_dern_vm_pop_value(vm, container);
                        octaspire_dern_vm_pop_value(vm, arguments);
                        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                        return result;
                    }
                }

                ++counter;
            }

            octaspire_dern_vm_pop_value(vm, extendedEnvVal);
            octaspire_dern_vm_pop_value(vm, container);
            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_integer(vm, counter);
        }
        else if(container->typeTag == OCTASPIRE_DERN_VALUE_TAG_PORT)
        {
            octaspire_dern_port_t * const port = container->value.port;

            int32_t counter = 0;
            size_t  skip    = 0;

            bool noMoreInput = false;

            while (!noMoreInput)
            {
                char buffer[1];

                for (size_t i = 0; i < skip; ++i)
                {
                    if (octaspire_dern_port_read(port, buffer, sizeof(char)) < 1)
                    {
                        noMoreInput = true;
                        break;
                    }
                }

                if (noMoreInput)
                {
                    break;
                }

                if (octaspire_dern_port_read(port, buffer, sizeof(char)) < 1)
                {
                    noMoreInput = true;
                    break;
                }

                octaspire_dern_environment_set(
                    extendedEnvironment,
                    counterSymbol,
                    octaspire_dern_vm_create_new_value_integer(vm, (int32_t)buffer[0]));

                for (size_t j = currentArgIdx; j < numArgs; ++j)
                {
                    octaspire_dern_value_t *result = octaspire_dern_vm_eval(
                        vm,
                        octaspire_dern_value_as_vector_get_element_at(
                            arguments,
                            (ptrdiff_t)j),
                        extendedEnvVal);

                    octaspire_helpers_verify_not_null(result);

                    if (result->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                    {
                        octaspire_dern_vm_pop_value(vm, extendedEnvVal);
                        octaspire_dern_vm_pop_value(vm, container);
                        octaspire_dern_vm_pop_value(vm, arguments);
                        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                        return result;
                    }

                    if (octaspire_dern_vm_get_function_return(vm))
                    {
                        result = octaspire_dern_vm_get_function_return(vm);
                        //octaspire_dern_vm_set_function_return(vm, 0);
                        octaspire_dern_vm_pop_value(vm, extendedEnvVal);
                        octaspire_dern_vm_pop_value(vm, container);
                        octaspire_dern_vm_pop_value(vm, arguments);
                        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                        return result;
                    }

                }

                skip = (stepSize - 1);

                ++counter;
            }

            octaspire_dern_vm_pop_value(vm, extendedEnvVal);
            octaspire_dern_vm_pop_value(vm, container);
            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_integer(vm, counter);
        }
        else
        {
            abort();
        }
    }
    else if (octaspire_container_utf8_string_is_equal_to_c_string(inOrFromSymbol->value.symbol, "from"))
    {
        // (for i from 0 to 100)
        // TODO XXX



        // 0
        octaspire_dern_value_t *fromValue =
            octaspire_dern_value_as_vector_get_element_at(arguments, 2);

        octaspire_helpers_verify_not_null(fromValue);

        fromValue = octaspire_dern_vm_eval(vm, fromValue, environment);

        octaspire_helpers_verify_not_null(fromValue);

        octaspire_dern_vm_push_value(vm, fromValue);

        if (fromValue->typeTag != OCTASPIRE_DERN_VALUE_TAG_INTEGER)
        {
            octaspire_dern_value_t *result = octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Third argument to special 'for' using 'from' must be integer. "
                "Now it has type %s.",
                octaspire_dern_value_helper_get_type_as_c_string(fromValue->typeTag));

            octaspire_dern_vm_pop_value(vm, fromValue);
            octaspire_dern_vm_pop_value(vm, arguments);

            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return result;
        }




        // 100
        octaspire_dern_value_t *toValue =
            octaspire_dern_value_as_vector_get_element_at(arguments, 4);

        octaspire_helpers_verify_not_null(toValue);

        toValue = octaspire_dern_vm_eval(vm, toValue, environment);

        octaspire_helpers_verify_not_null(toValue);

        octaspire_dern_vm_push_value(vm, toValue);

        if (toValue->typeTag != fromValue->typeTag)
        {
            octaspire_dern_value_t *result = octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Fifth argument to special 'for' using 'from' must be of same type than the third (%s). "
                "Now it has type %s.",
                octaspire_dern_value_helper_get_type_as_c_string(fromValue->typeTag),
                octaspire_dern_value_helper_get_type_as_c_string(toValue->typeTag));

            octaspire_dern_vm_pop_value(vm, toValue);
            octaspire_dern_vm_pop_value(vm, fromValue);
            octaspire_dern_vm_pop_value(vm, arguments);

            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return result;
        }




        // Check if there are optional 'step' and then integer
        size_t currentArgIdx = 5;

        // Optional 5. and 6. arguments: step integer
        octaspire_dern_value_t const * const optionalStep =
            octaspire_dern_value_as_vector_get_element_at(
                arguments,
                (ptrdiff_t)currentArgIdx);

        octaspire_helpers_verify_not_null(optionalStep);

        if (optionalStep->typeTag == OCTASPIRE_DERN_VALUE_TAG_SYMBOL &&
            octaspire_container_utf8_string_is_equal_to_c_string(optionalStep->value.symbol, "step"))
        {
            if (numArgs < 7)
            {
                octaspire_dern_vm_pop_value(vm, toValue);
                octaspire_dern_vm_pop_value(vm, fromValue);
                octaspire_dern_vm_pop_value(vm, arguments);

                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Special 'for' expects at least seven arguments in numeric form with "
                    "a given step size. %zu arguments were given.",
                    numArgs);
            }

            // There is 'step'. Now an integer is required next
            ++currentArgIdx;

            octaspire_dern_value_t const * const requiredStepSize =
                octaspire_dern_value_as_vector_get_element_at(
                    arguments,
                    (ptrdiff_t)currentArgIdx);

            octaspire_helpers_verify_not_null(requiredStepSize);

            if (requiredStepSize->typeTag != OCTASPIRE_DERN_VALUE_TAG_INTEGER)
            {
                octaspire_dern_value_t *result = octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Seventh argument to special 'for' using 'step' with numeric iteration must be "
                    "an integer step size. Now it has type %s.",
                    octaspire_dern_value_helper_get_type_as_c_string(requiredStepSize->typeTag));

                octaspire_dern_vm_pop_value(vm, toValue);
                octaspire_dern_vm_pop_value(vm, fromValue);
                octaspire_dern_vm_pop_value(vm, arguments);

                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return result;
            }

            if (requiredStepSize->value.integer <= 0)
            {
                octaspire_dern_value_t *result = octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "The 'step' of special 'for' must be larger than zero. "
                    "Now it is %" PRId32 ".",
                    requiredStepSize->value.integer);

                octaspire_dern_vm_pop_value(vm, toValue);
                octaspire_dern_vm_pop_value(vm, fromValue);
                octaspire_dern_vm_pop_value(vm, arguments);

                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return result;
            }


            stepSize = (size_t)requiredStepSize->value.integer;
        }









        // Extend env for the counter variable
        octaspire_dern_environment_t *extendedEnvironment =
            octaspire_dern_environment_new(
                environment,
                vm,
                octaspire_dern_vm_get_allocator(vm));

        octaspire_helpers_verify_not_null(extendedEnvironment);

        octaspire_dern_value_t *extendedEnvVal =
            octaspire_dern_vm_create_new_value_environment_from_environment(vm, extendedEnvironment);

        octaspire_helpers_verify_not_null(extendedEnvVal);

        octaspire_dern_vm_push_value(vm, extendedEnvVal);

        bool const fromIsSmaller = octaspire_dern_value_is_less_than_or_equal(fromValue, toValue);

        if (fromIsSmaller)
        {
            size_t const numIterations =
                (size_t)(toValue->value.integer - fromValue->value.integer) + 1;

            int32_t counter = 0;

            for (size_t i = 0; i < numIterations; i += stepSize)
            {
                octaspire_dern_value_t *iterator =
                    octaspire_dern_vm_create_new_value_copy(vm, fromValue);

                iterator->value.integer += i;

                octaspire_dern_environment_set(
                    extendedEnvironment,
                    counterSymbol,
                    iterator);

                for (size_t j = currentArgIdx; j < numArgs; ++j)
                {
                    octaspire_dern_value_t *result = octaspire_dern_vm_eval(
                        vm,
                        octaspire_dern_value_as_vector_get_element_at(
                            arguments,
                            (ptrdiff_t)j),
                        extendedEnvVal);

                    octaspire_helpers_verify_not_null(result);

                    if (result->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                    {
                        octaspire_dern_vm_pop_value(vm, extendedEnvVal);
                        octaspire_dern_vm_pop_value(vm, toValue);
                        octaspire_dern_vm_pop_value(vm, fromValue);
                        octaspire_dern_vm_pop_value(vm, arguments);
                        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                        return result;
                    }

                    if (octaspire_dern_vm_get_function_return(vm))
                    {
                        result = octaspire_dern_vm_get_function_return(vm);
                        //octaspire_dern_vm_set_function_return(vm, 0);
                        octaspire_dern_vm_pop_value(vm, extendedEnvVal);
                        octaspire_dern_vm_pop_value(vm, toValue);
                        octaspire_dern_vm_pop_value(vm, fromValue);
                        octaspire_dern_vm_pop_value(vm, arguments);
                        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                        return result;
                    }
                }

                ++counter;
            }

            octaspire_dern_vm_pop_value(vm, extendedEnvVal);
            octaspire_dern_vm_pop_value(vm, toValue);
            octaspire_dern_vm_pop_value(vm, fromValue);
            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_integer(vm, counter);
        }
        else
        {
            size_t const numIterations =
                (size_t)(fromValue->value.integer - toValue->value.integer) + 1;

            int32_t counter = 0;

            for (size_t i = 0; i < numIterations; i += stepSize)
            {
                octaspire_dern_value_t *iterator =
                    octaspire_dern_vm_create_new_value_copy(vm, fromValue);

                iterator->value.integer -= i;

                octaspire_dern_environment_set(
                    extendedEnvironment,
                    counterSymbol,
                    iterator);

                for (size_t j = currentArgIdx; j < numArgs; ++j)
                {
                    octaspire_dern_value_t *result = octaspire_dern_vm_eval(
                        vm,
                        octaspire_dern_value_as_vector_get_element_at(
                            arguments,
                            (ptrdiff_t)j),
                        extendedEnvVal);

                    octaspire_helpers_verify_not_null(result);

                    if (result->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                    {
                        octaspire_dern_vm_pop_value(vm, extendedEnvVal);
                        octaspire_dern_vm_pop_value(vm, toValue);
                        octaspire_dern_vm_pop_value(vm, fromValue);
                        octaspire_dern_vm_pop_value(vm, arguments);
                        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                        return result;
                    }

                    if (octaspire_dern_vm_get_function_return(vm))
                    {
                        result = octaspire_dern_vm_get_function_return(vm);
                        //octaspire_dern_vm_set_function_return(vm, 0);
                        octaspire_dern_vm_pop_value(vm, extendedEnvVal);
                        octaspire_dern_vm_pop_value(vm, toValue);
                        octaspire_dern_vm_pop_value(vm, fromValue);
                        octaspire_dern_vm_pop_value(vm, arguments);
                        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                        return result;
                    }
                }

                ++counter;
            }

            octaspire_dern_vm_pop_value(vm, extendedEnvVal);
            octaspire_dern_vm_pop_value(vm, toValue);
            octaspire_dern_vm_pop_value(vm, fromValue);
            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_integer(vm, counter);
        }
    }

    return 0;
}

// TODO move to other builtins
octaspire_dern_value_t *octaspire_dern_vm_builtin_starts_with_question_mark(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs != 2)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'starts-with?' expects two arguments.");
    }

    octaspire_dern_vm_push_value(vm, arguments);

    octaspire_dern_value_t *firstArg = octaspire_dern_value_as_vector_get_element_at(arguments, 0);
    octaspire_helpers_verify_not_null(firstArg);

    octaspire_dern_value_t *secondArg = octaspire_dern_value_as_vector_get_element_at(arguments, 1);
    octaspire_helpers_verify_not_null(secondArg);

    if (firstArg->typeTag != secondArg->typeTag)
    {
        octaspire_dern_vm_pop_value(vm, arguments);
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_get_value_false(vm);
    }

    if (firstArg->typeTag != OCTASPIRE_DERN_VALUE_TAG_STRING)
    {
        // TODO XXX implement rest of the fitting types
        abort();
    }

    bool const result = octaspire_container_utf8_string_starts_with(
        firstArg->value.string,
        secondArg->value.string);

    octaspire_dern_vm_pop_value(vm, arguments);
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, result);
}

/*
    (= num 10)                         ; set variable to
    (= vec 1 [kissa])                  ; set vector index to
    (= map [john] [305-3848506428])    ; set map for key to
    (= str 1 |a|)                      ; set character of string at index to
*/
octaspire_dern_value_t *octaspire_dern_vm_builtin_equals(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs < 2 || numArgs > 3)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin '=' expects two or three arguments.");
    }

    octaspire_dern_vm_push_value(vm, arguments);

    octaspire_dern_value_t *firstArg = octaspire_dern_value_as_vector_get_element_at(arguments, 0);
    octaspire_helpers_verify_not_null(firstArg);

    if (numArgs == 2)
    {
        octaspire_dern_value_t *secondArg = octaspire_dern_value_as_vector_get_element_at(arguments, 1);
        octaspire_helpers_verify_not_null(secondArg);

        if (octaspire_dern_value_set(firstArg, secondArg))
        {
            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            //return octaspire_dern_vm_get_value_true(vm);
            return firstArg;
        }

        octaspire_dern_vm_pop_value(vm, arguments);
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(vm, "Builtin '=' failed");
    }
    else
    {
        octaspire_dern_value_t *secondArg = octaspire_dern_value_as_vector_get_element_at(arguments, 1);
        octaspire_helpers_verify_not_null(secondArg);

        octaspire_dern_value_t *thirdArg = octaspire_dern_value_as_vector_get_element_at(arguments, 2);
        octaspire_helpers_verify_not_null(thirdArg);

        if (octaspire_dern_value_set_collection(firstArg, secondArg, thirdArg))
        {
            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            //return octaspire_dern_vm_get_value_true(vm);
            return firstArg;
        }

        octaspire_dern_vm_pop_value(vm, arguments);
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(vm, "Builtin '=' failed");
    }
}


octaspire_dern_value_t *octaspire_dern_vm_builtin_equals_equals(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs < 2)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin '==' expects at least two arguments.");
    }

    octaspire_dern_vm_push_value(vm, arguments);

    octaspire_dern_value_t *firstValue = octaspire_dern_vm_eval(
        vm,
        octaspire_dern_value_as_vector_get_element_at(arguments, 0),
        environment);

    octaspire_dern_vm_push_value(vm, firstValue);

    for (size_t i = 1; i < numArgs; ++i)
    {
        if (!octaspire_dern_value_is_equal(
                firstValue,
                octaspire_dern_vm_eval(
                    vm,
                    octaspire_dern_value_as_vector_get_element_at(
                        arguments,
                        (ptrdiff_t)i),
                    environment)))
        {
            octaspire_dern_vm_pop_value(vm, firstValue);
            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_get_value_false(vm);
        }
    }

    octaspire_dern_vm_pop_value(vm, firstValue);
    octaspire_dern_vm_pop_value(vm, arguments);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_get_value_true(vm);
}

// TODO move to other builtins
octaspire_dern_value_t *octaspire_dern_vm_builtin_exclamation_equals(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    octaspire_dern_value_t *tmpVal =
        octaspire_dern_vm_builtin_equals_equals(vm, arguments, environment);

    if (tmpVal->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
    {
        return tmpVal;
    }

    if (tmpVal->typeTag != OCTASPIRE_DERN_VALUE_TAG_BOOLEAN)
    {
        abort();
    }

    return octaspire_dern_vm_create_new_value_boolean(vm, !tmpVal->value.boolean);
}

octaspire_dern_value_t *octaspire_dern_vm_special_less_than_or_equal(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs < 2)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Special '<=' expects at least two arguments.");
    }

    octaspire_dern_vm_push_value(vm, arguments);

    octaspire_dern_value_t *firstValue = octaspire_dern_vm_eval(
        vm,
        octaspire_dern_value_as_vector_get_element_at(arguments, 0),
        environment);

    octaspire_dern_vm_push_value(vm, firstValue);

    for (size_t i = 1; i < numArgs; ++i)
    {
        if (!octaspire_dern_value_is_less_than_or_equal(
                firstValue,
                octaspire_dern_vm_eval(
                    vm,
                    octaspire_dern_value_as_vector_get_element_at(
                        arguments,
                        (ptrdiff_t)i),
                    environment)))
        {
            octaspire_dern_vm_pop_value(vm, firstValue);
            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_get_value_false(vm);
        }
    }

    octaspire_dern_vm_pop_value(vm, firstValue);
    octaspire_dern_vm_pop_value(vm, arguments);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_get_value_true(vm);
}

octaspire_dern_value_t *octaspire_dern_vm_special_less_than(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs < 2)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Special '<' expects at least two arguments.");
    }

    octaspire_dern_vm_push_value(vm, arguments);

    octaspire_dern_value_t *firstValue = octaspire_dern_vm_eval(
        vm,
        octaspire_dern_value_as_vector_get_element_at(arguments, 0),
        environment);

    octaspire_dern_vm_push_value(vm, firstValue);

    for (size_t i = 1; i < numArgs; ++i)
    {
        if (!octaspire_dern_value_is_less_than(
                firstValue,
                octaspire_dern_vm_eval(
                    vm,
                    octaspire_dern_value_as_vector_get_element_at(
                        arguments,
                        (ptrdiff_t)i),
                    environment)))
        {
            octaspire_dern_vm_pop_value(vm, firstValue);
            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_get_value_false(vm);
        }
    }

    octaspire_dern_vm_pop_value(vm, firstValue);
    octaspire_dern_vm_pop_value(vm, arguments);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_get_value_true(vm);
}

octaspire_dern_value_t *octaspire_dern_vm_special_greater_than(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs < 2)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Special '>' expects at least two arguments.");
    }

    octaspire_dern_vm_push_value(vm, arguments);

    octaspire_dern_value_t *firstValue = octaspire_dern_vm_eval(
        vm,
        octaspire_dern_value_as_vector_get_element_at(arguments, 0),
        environment);

    octaspire_dern_vm_push_value(vm, firstValue);

    for (size_t i = 1; i < numArgs; ++i)
    {
        if (!octaspire_dern_value_is_greater_than(
                firstValue,
                octaspire_dern_vm_eval(
                    vm,
                    octaspire_dern_value_as_vector_get_element_at(
                        arguments,
                        (ptrdiff_t)i),
                    environment)))
        {
            octaspire_dern_vm_pop_value(vm, firstValue);
            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_get_value_false(vm);
        }
    }

    octaspire_dern_vm_pop_value(vm, firstValue);
    octaspire_dern_vm_pop_value(vm, arguments);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_get_value_true(vm);
}

octaspire_dern_value_t *octaspire_dern_vm_special_greater_than_or_equal(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs < 2)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Special '>=' expects at least two arguments.");
    }

    octaspire_dern_vm_push_value(vm, arguments);

    octaspire_dern_value_t *firstValue = octaspire_dern_vm_eval(
        vm,
        octaspire_dern_value_as_vector_get_element_at(arguments, 0),
        environment);

    octaspire_dern_vm_push_value(vm, firstValue);

    for (size_t i = 1; i < numArgs; ++i)
    {
        if (!octaspire_dern_value_is_greater_than_or_equal(
                firstValue,
                octaspire_dern_vm_eval(
                    vm,
                    octaspire_dern_value_as_vector_get_element_at(
                        arguments,
                        (ptrdiff_t)i),
                    environment)))
        {
            octaspire_dern_vm_pop_value(vm, firstValue);
            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_get_value_false(vm);
        }
    }

    octaspire_dern_vm_pop_value(vm, firstValue);
    octaspire_dern_vm_pop_value(vm, arguments);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_get_value_true(vm);
}

octaspire_dern_value_t *octaspire_dern_stdlib_private_validate_function(
    octaspire_dern_vm_t* vm,
    octaspire_dern_function_t *function)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_not_null(function);
    octaspire_helpers_verify_not_null(function->formals);
    octaspire_helpers_verify_true(function->formals->typeTag == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_not_null(function->body);
    octaspire_helpers_verify_true(function->body->typeTag == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_not_null(function->definitionEnvironment);
    octaspire_helpers_verify_true(function->definitionEnvironment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);
    octaspire_helpers_verify_not_null(function->allocator);

    size_t const numFormalArgs = octaspire_container_vector_get_length(
        function->formals->value.vector);

    size_t numDotArgs            = 0;
    size_t numNormalArgs         = 0;
    size_t numNormalArgsAfterDot = 0;

    for (size_t i = 0; i < numFormalArgs; ++i)
    {
        octaspire_dern_value_t *formal = octaspire_container_vector_get_element_at(
            function->formals->value.vector,
            (ptrdiff_t)i);

        octaspire_helpers_verify_true(formal->typeTag == OCTASPIRE_DERN_VALUE_TAG_SYMBOL);

        octaspire_container_utf8_string_t const * const formalAsStr = formal->value.string;

        if (octaspire_container_utf8_string_is_equal_to_c_string(formalAsStr, "."))
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
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Function can have only one formal . for varargs. Now %zu dots were given.",
            numDotArgs);
    }

    if (numNormalArgsAfterDot > 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Function can have only one formal argument after . "
            "for varargs. Now %zu formals were given after dot.",
            numNormalArgsAfterDot);
    }

    if (numDotArgs == 0 && numNormalArgs != numFormalArgs)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Number of formal and actual arguments must be equal for functions without "
            "varargs using the dot-formal. Now %zu formal and %zu actual arguments were given.",
            numFormalArgs,
            numNormalArgs);
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return 0;
}

octaspire_dern_value_t *octaspire_dern_vm_special_fn(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_dern_vm_push_value(vm, arguments);
    octaspire_dern_vm_push_value(vm, environment);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    if (octaspire_container_vector_get_length(vec) < 2)
    {
        octaspire_dern_vm_pop_value(vm, environment);
        octaspire_dern_vm_pop_value(vm, arguments);

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Special 'fn' expects at least two arguments. %zu arguments were given.",
            octaspire_container_vector_get_length(vec));
    }

    octaspire_dern_value_t *formals = octaspire_container_vector_get_element_at(vec, 0);

    if (formals->typeTag != OCTASPIRE_DERN_VALUE_TAG_VECTOR)
    {
        octaspire_dern_vm_pop_value(vm, environment);
        octaspire_dern_vm_pop_value(vm, arguments);

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "First argument to special 'fn' must be vector (formals). Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(formals->typeTag));
    }

    octaspire_dern_value_t *body = octaspire_dern_vm_create_new_value_vector(vm);

    octaspire_dern_vm_push_value(vm, body);

    for (size_t i = 1; i < octaspire_container_vector_get_length(vec); ++i)
    {
        octaspire_dern_value_t *tmpPtr =
            octaspire_container_vector_get_element_at(
                vec,
                (ptrdiff_t)i);

        if (tmpPtr->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
        {
            octaspire_dern_vm_pop_value(vm, body);
            octaspire_dern_vm_pop_value(vm, environment);
            octaspire_dern_vm_pop_value(vm, arguments);

            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return tmpPtr;
        }

        if (!octaspire_dern_value_as_vector_push_back_element(
            body,
            &tmpPtr))
        {
            abort();
        }
    }

    octaspire_dern_function_t *function = octaspire_dern_function_new(
        formals,
        body,
        environment,
        octaspire_dern_vm_get_allocator(vm));

    if (!function)
    {
        octaspire_dern_vm_pop_value(vm, body);
        octaspire_dern_vm_pop_value(vm, environment);
        octaspire_dern_vm_pop_value(vm, arguments);

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Allocation failure when creating function.");
    }

    octaspire_dern_value_t *error = octaspire_dern_stdlib_private_validate_function(vm, function);

    if (error)
    {
        octaspire_dern_vm_pop_value(vm, body);
        octaspire_dern_vm_pop_value(vm, environment);
        octaspire_dern_vm_pop_value(vm, arguments);

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return error;
    }

    // Real docstring is set by define
    octaspire_dern_value_t * result = octaspire_dern_vm_create_new_value_function(vm, function, "", 0);

    octaspire_dern_vm_pop_value(vm, body);
    octaspire_dern_vm_pop_value(vm, environment);
    octaspire_dern_vm_pop_value(vm, arguments);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_uid(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_dern_vm_push_value(vm, arguments);
    octaspire_dern_vm_push_value(vm, environment);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    if (octaspire_container_vector_get_length(vec) != 1)
    {
        octaspire_dern_vm_pop_value(vm, environment);
        octaspire_dern_vm_pop_value(vm, arguments);

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Special 'uid' expects exactly one argument. %zu arguments were given.",
            octaspire_container_vector_get_length(vec));
    }

    uintmax_t const uid =
        octaspire_dern_value_get_unique_id(octaspire_container_vector_get_element_at(vec, 0));

    if (uid > INT32_MAX)
    {
        abort();
    }

    octaspire_dern_value_t * result =
        octaspire_dern_vm_create_new_value_integer(vm, (int32_t)uid);

    octaspire_dern_vm_pop_value(vm, environment);
    octaspire_dern_vm_pop_value(vm, arguments);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_abort(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'abort' expects one argument.");
    }

    octaspire_dern_vm_push_value(vm, arguments);

    octaspire_dern_value_t *firstArg = octaspire_dern_value_as_vector_get_element_at(arguments, 0);
    octaspire_helpers_verify_not_null(firstArg);

    octaspire_dern_value_print(firstArg, octaspire_dern_vm_get_allocator(vm));

    octaspire_dern_vm_pop_value(vm, arguments);
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    abort();
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_input_file_open(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    if (!octaspire_dern_vm_is_file_system_access_allowed(vm))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'input-file-open' cannot be executed; file system access is denied by VM. "
            "Enable file system access in VM before trying to run this code.");
    }

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'input-file-open' expects one argument.");
    }

    octaspire_dern_vm_push_value(vm, arguments);

    octaspire_dern_value_t *firstArg = octaspire_dern_value_as_vector_get_element_at(arguments, 0);
    octaspire_helpers_verify_not_null(firstArg);

    if (firstArg->typeTag != OCTASPIRE_DERN_VALUE_TAG_STRING)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'input-file-open' expects string argument.");
    }

    octaspire_dern_value_t * const result = octaspire_dern_vm_create_new_value_input_file(
        vm,
        octaspire_dern_value_as_string_get_c_string(firstArg));

    octaspire_dern_vm_pop_value(vm, arguments);
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_output_file_open(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    if (!octaspire_dern_vm_is_file_system_access_allowed(vm))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'output-file-open' cannot be executed; file system access is denied by VM. "
            "Enable file system access in VM before trying to run this code.");
    }

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'output-file-open' expects one argument.");
    }

    octaspire_dern_vm_push_value(vm, arguments);

    octaspire_dern_value_t *firstArg = octaspire_dern_value_as_vector_get_element_at(arguments, 0);
    octaspire_helpers_verify_not_null(firstArg);

    if (firstArg->typeTag != OCTASPIRE_DERN_VALUE_TAG_STRING)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'output-file-open' expects string argument.");
    }

    octaspire_dern_value_t * const result = octaspire_dern_vm_create_new_value_output_file(
        vm,
        octaspire_dern_value_as_string_get_c_string(firstArg));

    octaspire_dern_vm_pop_value(vm, arguments);
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_io_file_open(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    if (!octaspire_dern_vm_is_file_system_access_allowed(vm))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'io-file-open' cannot be executed; file system access is denied by VM. "
            "Enable file system access in VM before trying to run this code.");
    }

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'io-file-open' expects one argument.");
    }

    octaspire_dern_vm_push_value(vm, arguments);

    octaspire_dern_value_t *firstArg = octaspire_dern_value_as_vector_get_element_at(arguments, 0);
    octaspire_helpers_verify_not_null(firstArg);

    if (firstArg->typeTag != OCTASPIRE_DERN_VALUE_TAG_STRING)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'io-file-open' expects string argument.");
    }

    octaspire_dern_value_t * const result = octaspire_dern_vm_create_new_value_io_file(
        vm,
        octaspire_dern_value_as_string_get_c_string(firstArg));

    octaspire_dern_vm_pop_value(vm, arguments);
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_port_supports_output_question_mark(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'port-supports-output?' expects one argument.");
    }

    octaspire_dern_vm_push_value(vm, arguments);

    octaspire_dern_value_t *firstArg = octaspire_dern_value_as_vector_get_element_at(arguments, 0);
    octaspire_helpers_verify_not_null(firstArg);

    if (firstArg->typeTag != OCTASPIRE_DERN_VALUE_TAG_PORT)
    {
        octaspire_dern_vm_pop_value(vm, arguments);
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'port-supports-output?' expects port argument.");
    }

    bool const result = octaspire_dern_port_supports_output(firstArg->value.port);

    octaspire_dern_vm_pop_value(vm, arguments);
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, result);
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_port_supports_input_question_mark(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'port-supports-input?' expects one argument.");
    }

    octaspire_dern_vm_push_value(vm, arguments);

    octaspire_dern_value_t *firstArg = octaspire_dern_value_as_vector_get_element_at(arguments, 0);
    octaspire_helpers_verify_not_null(firstArg);

    if (firstArg->typeTag != OCTASPIRE_DERN_VALUE_TAG_PORT)
    {
        octaspire_dern_vm_pop_value(vm, arguments);
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'port-supports-input?' expects port argument.");
    }

    bool const result = octaspire_dern_port_supports_input(firstArg->value.port);

    octaspire_dern_vm_pop_value(vm, arguments);
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, result);
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_port_close(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'port-close' expects one argument.");
    }

    octaspire_dern_vm_push_value(vm, arguments);

    octaspire_dern_value_t *firstArg = octaspire_dern_value_as_vector_get_element_at(arguments, 0);
    octaspire_helpers_verify_not_null(firstArg);

    if (firstArg->typeTag != OCTASPIRE_DERN_VALUE_TAG_PORT)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'port-close' expects port argument.");
    }

    bool const wasClosed = octaspire_dern_port_close(firstArg->value.port);

    octaspire_dern_vm_pop_value(vm, arguments);
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, wasClosed);
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_port_read(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs != 1 && numArgs != 2)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'port-read' expects one or two arguments.");
    }

    octaspire_dern_vm_push_value(vm, arguments);

    octaspire_dern_value_t *firstArg = octaspire_dern_value_as_vector_get_element_at(arguments, 0);
    octaspire_helpers_verify_not_null(firstArg);

    if (firstArg->typeTag != OCTASPIRE_DERN_VALUE_TAG_PORT)
    {
        octaspire_dern_vm_pop_value(vm, arguments);
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "The first argument to builtin 'port-read' must be a port. Now type %s was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    octaspire_dern_value_t *result = 0;

    if (numArgs == 2)
    {
        octaspire_dern_value_t *secondArg =
            octaspire_dern_value_as_vector_get_element_at(arguments, 1);

        octaspire_helpers_verify_not_null(secondArg);

        if (secondArg->typeTag != OCTASPIRE_DERN_VALUE_TAG_INTEGER)
        {
            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "The second argument to builtin 'port-read' must be an integer. Now type %s was given.",
                octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
        }

        result = octaspire_dern_vm_create_new_value_vector(vm);

        octaspire_helpers_verify_not_null(result);

        octaspire_dern_vm_push_value(vm, result);

        // TODO better implementation
        for (ptrdiff_t i = 0; i < secondArg->value.integer; ++i)
        {
            char buffer[1];

            ptrdiff_t const numOctetsRead =
                octaspire_dern_port_read(firstArg->value.port, buffer, sizeof(buffer));

            if (numOctetsRead != 1)
            {
                break;
            }

            octaspire_dern_value_t *elem =
                octaspire_dern_vm_create_new_value_integer(vm, (int32_t)buffer[0]);

            octaspire_helpers_verify_not_null(elem);

            octaspire_dern_vm_push_value(vm, elem);
            octaspire_dern_value_as_vector_push_back_element(result, &elem);
            octaspire_dern_vm_pop_value(vm, elem);
        }

        octaspire_helpers_verify_not_null(result);

        octaspire_dern_vm_pop_value(vm, result);
        octaspire_dern_vm_pop_value(vm, arguments);
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return result;
    }
    else
    {
        char buffer[1];

        ptrdiff_t const numOctetsRead =
            octaspire_dern_port_read(firstArg->value.port, buffer, sizeof(buffer));

        if (numOctetsRead != 1)
        {
            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

            return octaspire_dern_vm_create_new_value_error_from_c_string(
                vm,
                "Builtin 'port-read' failed to read the requested one octet.");
        }

        result = octaspire_dern_vm_create_new_value_integer(vm, (int32_t)buffer[0]);

        octaspire_helpers_verify_not_null(result);

        octaspire_dern_vm_pop_value(vm, arguments);
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return result;
    }
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_port_write(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs != 2)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'port-write' expects exactly two arguments. %zu argument were given.",
            numArgs);
    }

    octaspire_dern_vm_push_value(vm, arguments);

    octaspire_dern_value_t *firstArg = octaspire_dern_value_as_vector_get_element_at(arguments, 0);
    octaspire_helpers_verify_not_null(firstArg);

    if (firstArg->typeTag != OCTASPIRE_DERN_VALUE_TAG_PORT)
    {
        octaspire_dern_vm_pop_value(vm, arguments);
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "The first argument to builtin 'port-write' must be a port. Now type %s was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    if (!octaspire_dern_port_supports_output(firstArg->value.port))
    {
        octaspire_dern_vm_pop_value(vm, arguments);
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "The first argument to builtin 'port-write' must be a port supporting writing.");
    }

    octaspire_dern_value_t *secondArg = octaspire_dern_value_as_vector_get_element_at(arguments, 1);
    octaspire_helpers_verify_not_null(secondArg);

    if (secondArg->typeTag == OCTASPIRE_DERN_VALUE_TAG_INTEGER)
    {
        // TODO XXX check that value fits into one octet and report
        // error otherwise
        char buffer[1];
        buffer[0] = (char)secondArg->value.integer;

        ptrdiff_t const numWritten =
            octaspire_dern_port_write(firstArg->value.port, buffer, sizeof(buffer));

        octaspire_helpers_verify_true(numWritten >= 0);

        octaspire_dern_vm_pop_value(vm, arguments);
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_integer(vm, (int32_t)numWritten);
    }
    else if (secondArg->typeTag == OCTASPIRE_DERN_VALUE_TAG_CHARACTER)
    {
        octaspire_container_utf8_string_t *str = secondArg->value.character;
        char const * const buffer = octaspire_container_utf8_string_get_c_string(str);
        size_t bufferLen = octaspire_container_utf8_string_get_length_in_octets(str);

        ptrdiff_t const numWritten =
            octaspire_dern_port_write(firstArg->value.port, buffer, bufferLen);

            if (numWritten < 0 || (size_t)numWritten != bufferLen)
            {
                octaspire_dern_vm_pop_value(vm, arguments);
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Writing of %zu octets of character failed. Only %zu octets were written.",
                    bufferLen,
                    numWritten);
            }
            else
            {
                octaspire_dern_vm_pop_value(vm, arguments);
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_integer(
                    vm,
                    (int32_t)bufferLen);
            }
    }
    else if (secondArg->typeTag == OCTASPIRE_DERN_VALUE_TAG_STRING)
    {
        octaspire_container_utf8_string_t *str = secondArg->value.string;
        char const * const buffer = octaspire_container_utf8_string_get_c_string(str);
        size_t bufferLen = octaspire_container_utf8_string_get_length_in_octets(str);

        ptrdiff_t const numWritten =
            octaspire_dern_port_write(firstArg->value.port, buffer, bufferLen);

            if (numWritten < 0 || (size_t)numWritten != bufferLen)
            {
                octaspire_dern_vm_pop_value(vm, arguments);
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Writing of %zu octets of string failed. Only %zu octets were written.",
                    bufferLen,
                    numWritten);
            }
            else
            {
                octaspire_dern_vm_pop_value(vm, arguments);
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_integer(
                    vm,
                    (int32_t)bufferLen);
            }
    }
    else if (secondArg->typeTag == OCTASPIRE_DERN_VALUE_TAG_VECTOR)
    {
        int32_t counter = 0;
        for (size_t i = 0; i < octaspire_dern_value_as_vector_get_length(secondArg); ++i)
        {
            octaspire_dern_value_t * const elem =
                octaspire_dern_value_as_vector_get_element_at(
                    secondArg,
                    (ptrdiff_t)i);

            octaspire_helpers_verify_not_null(elem);

            octaspire_dern_value_t *tmpArgs = octaspire_dern_vm_create_new_value_vector(vm);
            octaspire_dern_vm_push_value(vm, tmpArgs);

            octaspire_dern_value_as_vector_push_back_element(tmpArgs, &firstArg);
            octaspire_dern_value_as_vector_push_back_element(tmpArgs, &elem);

            octaspire_dern_value_t * const countVal =
                octaspire_dern_vm_builtin_port_write(vm, tmpArgs, environment);

            octaspire_dern_vm_pop_value(vm, tmpArgs);

            octaspire_helpers_verify_not_null(countVal);

            if (countVal->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
            {
                octaspire_dern_vm_pop_value(vm, arguments);
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return countVal;
            }

            octaspire_helpers_verify_true(countVal->typeTag == OCTASPIRE_DERN_VALUE_TAG_INTEGER);

            counter += countVal->value.integer;
        }

        octaspire_dern_vm_pop_value(vm, arguments);
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_integer(vm, counter);
    }
    else
    {
        octaspire_dern_vm_pop_value(vm, arguments);
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "The second argument to builtin 'port-write' must be an integer, character, string\n"
            "or a vector of these types. Now type %s was given.",
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_port_seek(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs != 2 && numArgs != 3)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'port-seek' expects two or three arguments.");
    }

    octaspire_dern_vm_push_value(vm, arguments);

    octaspire_dern_value_t *firstArg = octaspire_dern_value_as_vector_get_element_at(arguments, 0);
    octaspire_helpers_verify_not_null(firstArg);

    if (firstArg->typeTag != OCTASPIRE_DERN_VALUE_TAG_PORT)
    {
        octaspire_dern_vm_pop_value(vm, arguments);
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "The first argument to builtin 'port-seek' must be a port. Now type %s was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    octaspire_dern_value_t *secondArg = octaspire_dern_value_as_vector_get_element_at(arguments, 1);
    octaspire_helpers_verify_not_null(secondArg);

    if (secondArg->typeTag != OCTASPIRE_DERN_VALUE_TAG_INTEGER)
    {
        octaspire_dern_vm_pop_value(vm, arguments);
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "The second argument to builtin 'port-seek' must be an integer. Now type %s was given.",
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }

    bool seekFromCurrentPosition = false;

    if (numArgs == 3)
    {
        octaspire_dern_value_t *thirdArg = octaspire_dern_value_as_vector_get_element_at(arguments, 2);
        octaspire_helpers_verify_not_null(thirdArg);

        if (thirdArg->typeTag != OCTASPIRE_DERN_VALUE_TAG_SYMBOL)
        {
            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "The third argument to builtin 'port-seek' must be symbol 'from-current'. "
                "Now type %s was given.",
                octaspire_dern_value_helper_get_type_as_c_string(thirdArg->typeTag));
        }

        seekFromCurrentPosition = true;
    }

    bool const success = octaspire_dern_port_seek(
        firstArg->value.port,
        secondArg->value.integer,
        seekFromCurrentPosition);

    octaspire_dern_vm_pop_value(vm, arguments);
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, success);
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_port_dist(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'port-dist' expects exactly one argument.");
    }

    octaspire_dern_vm_push_value(vm, arguments);

    octaspire_dern_value_t *firstArg = octaspire_dern_value_as_vector_get_element_at(arguments, 0);
    octaspire_helpers_verify_not_null(firstArg);

    if (firstArg->typeTag != OCTASPIRE_DERN_VALUE_TAG_PORT)
    {
        octaspire_dern_vm_pop_value(vm, arguments);
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "The first argument to builtin 'port-dist' must be a port. Now type %s was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    ptrdiff_t dist = octaspire_dern_port_distance(firstArg->value.port);

    octaspire_dern_vm_pop_value(vm, arguments);
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    // TODO check that dist fits into int32_t and report error if it doesn'tk
    return octaspire_dern_vm_create_new_value_integer(vm, (int32_t)dist);
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_port_length(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'port-length' expects exactly one argument.");
    }

    octaspire_dern_vm_push_value(vm, arguments);

    octaspire_dern_value_t *firstArg = octaspire_dern_value_as_vector_get_element_at(arguments, 0);
    octaspire_helpers_verify_not_null(firstArg);

    if (firstArg->typeTag != OCTASPIRE_DERN_VALUE_TAG_PORT)
    {
        octaspire_dern_vm_pop_value(vm, arguments);
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "The first argument to builtin 'port-length' must be a port. Now type %s was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    ptrdiff_t length  = octaspire_dern_port_get_length_in_octets(firstArg->value.port);

    octaspire_dern_vm_pop_value(vm, arguments);
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    // TODO check that length fits into int32_t and report error if it doesn'tk
    return octaspire_dern_vm_create_new_value_integer(vm, (int32_t)length);
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_port_flush(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'port-flush' expects exactly one argument.");
    }

    octaspire_dern_vm_push_value(vm, arguments);

    octaspire_dern_value_t *firstArg = octaspire_dern_value_as_vector_get_element_at(arguments, 0);
    octaspire_helpers_verify_not_null(firstArg);

    if (firstArg->typeTag != OCTASPIRE_DERN_VALUE_TAG_PORT)
    {
        octaspire_dern_vm_pop_value(vm, arguments);
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "The first argument to builtin 'port-flush' must be a port. Now type %s was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    bool const success = octaspire_dern_port_flush(firstArg->value.port);

    octaspire_dern_vm_pop_value(vm, arguments);
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, success);
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_not(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'not' expects one argument.");
    }

    octaspire_dern_vm_push_value(vm, arguments);

    octaspire_dern_value_t *firstArg = octaspire_dern_value_as_vector_get_element_at(arguments, 0);
    octaspire_helpers_verify_not_null(firstArg);

    if (firstArg->typeTag != OCTASPIRE_DERN_VALUE_TAG_BOOLEAN)
    {
        octaspire_dern_vm_pop_value(vm, arguments);
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'not' expects boolean argument.");
    }

    bool const given = firstArg->value.boolean;

    octaspire_dern_vm_pop_value(vm, arguments);
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, !given);
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_return(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs > 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'return' expects zero or one arguments. %zu arguments were given.",
            numArgs);
    }

    if (numArgs == 1)
    {
        octaspire_dern_value_t *firstArg = octaspire_dern_value_as_vector_get_element_at(arguments, 0);
        octaspire_helpers_verify_not_null(firstArg);

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        octaspire_helpers_verify_true(octaspire_dern_vm_get_function_return(vm) == 0);

        octaspire_dern_vm_set_function_return(vm, firstArg);
        return firstArg;
    }
    else
    {
        octaspire_dern_value_t *result = octaspire_dern_vm_create_new_value_nil(vm);
        octaspire_helpers_verify_not_null(result);

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        octaspire_helpers_verify_true(octaspire_dern_vm_get_function_return(vm) == 0);

        octaspire_dern_vm_set_function_return(vm, result);
        return result;
    }
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_vector(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_dern_value_t *result = octaspire_dern_vm_create_new_value_vector(vm);
    octaspire_dern_vm_push_value(vm, result);

    for (size_t i = 0; i < octaspire_dern_value_as_vector_get_length(arguments); ++i)
    {
        octaspire_dern_value_t * arg =
            octaspire_dern_value_as_vector_get_element_at(
                arguments,
                (ptrdiff_t)i);

        if (octaspire_dern_value_is_atom(arg))
        {
            arg = octaspire_dern_vm_create_new_value_copy(vm, arg);
        }

        octaspire_dern_value_as_vector_push_back_element(result, &arg);
    }

    octaspire_dern_vm_pop_value(vm, result);
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_special_and(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    if (octaspire_dern_value_as_vector_get_length(arguments) == 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_boolean(vm, true);
    }

    octaspire_dern_value_t *result = 0;

    for (size_t i = 0; i < octaspire_dern_value_as_vector_get_length(arguments); ++i)
    {
        octaspire_dern_value_t * const arg =
            octaspire_dern_value_as_vector_get_element_at(
                arguments,
                (ptrdiff_t)i);

        octaspire_helpers_verify_not_null(arg);

        result = octaspire_dern_vm_eval(vm, arg, environment);

        if (octaspire_dern_vm_get_function_return(vm))
        {
            result = octaspire_dern_vm_get_function_return(vm);
            break;
        }

        if (result->typeTag == OCTASPIRE_DERN_VALUE_TAG_BOOLEAN)
        {
            if (!result->value.boolean)
            {
                break;
            }
        }
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_special_or(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    if (octaspire_dern_value_as_vector_get_length(arguments) == 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_boolean(vm, false);
    }

    octaspire_dern_value_t *result = 0;

    for (size_t i = 0; i < octaspire_dern_value_as_vector_get_length(arguments); ++i)
    {
        octaspire_dern_value_t * const arg =
            octaspire_dern_value_as_vector_get_element_at(
                arguments,
                (ptrdiff_t)i);

        octaspire_helpers_verify_not_null(arg);

        result = octaspire_dern_vm_eval(vm, arg, environment);

        if (octaspire_dern_vm_get_function_return(vm))
        {
            result = octaspire_dern_vm_get_function_return(vm);
            break;
        }

        if (result->typeTag == OCTASPIRE_DERN_VALUE_TAG_BOOLEAN)
        {
            if (result->value.boolean)
            {
                break;
            }
        }
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_special_do(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    if (octaspire_dern_value_as_vector_get_length(arguments) == 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Special 'do' expects at least one argument.");
    }

    octaspire_dern_value_t *result = 0;

    for (size_t i = 0; i < octaspire_dern_value_as_vector_get_length(arguments); ++i)
    {
        octaspire_dern_value_t * const arg =
            octaspire_dern_value_as_vector_get_element_at(
                arguments,
                (ptrdiff_t)i);

        octaspire_helpers_verify_not_null(arg);

        result = octaspire_dern_vm_eval(vm, arg, environment);

        if (octaspire_dern_vm_get_function_return(vm))
        {
            result = octaspire_dern_vm_get_function_return(vm);
            //octaspire_dern_vm_set_function_return(vm, 0);
            break;
        }

        if (result->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
        {
            octaspire_container_utf8_string_t *tmpStr =
                octaspire_dern_value_to_string(arg, octaspire_dern_vm_get_allocator(vm));

            octaspire_container_utf8_string_concatenate_format(
                result->value.string,
                "\n\tAt form: >>>>>>>>>>%s<<<<<<<<<<\n",
                octaspire_container_utf8_string_get_c_string(tmpStr));

            octaspire_container_utf8_string_release(tmpStr);
            tmpStr = 0;

            break;
        }
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_exit(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    if (octaspire_container_vector_get_length(vec) > 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'exit' expects zero or one argument.");
    }

    if (octaspire_container_vector_get_length(vec) == 1)
    {
        octaspire_dern_value_t *value = octaspire_container_vector_get_element_at(vec, 0);

        if (value->typeTag != OCTASPIRE_DERN_VALUE_TAG_INTEGER)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'exit' expects integer argument. Type %s was given.",
                octaspire_dern_value_helper_get_type_as_c_string(value->typeTag));
        }

        octaspire_dern_vm_set_exit_code(vm, value->value.integer);
    }

    octaspire_dern_vm_quit(vm);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_get_value_true(vm);
}


octaspire_dern_value_t *octaspire_dern_vm_builtin_doc(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    if (octaspire_container_vector_get_length(vec) < 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'doc' expects at least one argument.");
    }

    if (octaspire_container_vector_get_length(vec) == 1)
    {
        octaspire_dern_value_t *value = octaspire_container_vector_get_element_at(vec, 0);

        if (value->docstr)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return value->docstr;
        }

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_get_value_nil(vm);
    }
    else
    {
        octaspire_container_vector_t *resultVec = octaspire_container_vector_new(
            sizeof(octaspire_dern_value_t*),
            true,
            0,
            octaspire_dern_vm_get_allocator(vm));

        octaspire_dern_value_t *resultVal = octaspire_dern_vm_create_new_value_vector_from_vector(vm, resultVec);

        octaspire_dern_vm_push_value(vm, resultVal);

        for (size_t i = 0; i < octaspire_container_vector_get_length(vec); ++i)
        {
            octaspire_dern_value_t * const value =
                octaspire_container_vector_get_element_at(vec, (ptrdiff_t)i);

            if (value->docstr)
            {
                octaspire_container_vector_push_back_element(resultVec, &(value->docstr));
            }
            else
            {
                octaspire_dern_value_t *valueNil = octaspire_dern_vm_get_value_nil(vm);
                octaspire_container_vector_push_back_element(resultVec, &valueNil);
            }
        }

        octaspire_dern_vm_pop_value(vm, resultVal);

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return resultVal;
    }
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_len(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    if (octaspire_container_vector_get_length(vec) < 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'len' expects at least one argument.");
    }

    if (octaspire_container_vector_get_length(vec) == 1)
    {
        octaspire_dern_value_t *value = octaspire_container_vector_get_element_at(vec, 0);

        // TODO XXX check number ranges for too large size_t value for int32_t?
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_integer(vm, (int32_t)octaspire_dern_value_get_length(value));
    }
    else
    {
        octaspire_dern_value_t *resultVal = octaspire_dern_vm_create_new_value_vector(vm);

        octaspire_dern_vm_push_value(vm, resultVal);

        for (size_t i = 0; i < octaspire_container_vector_get_length(vec); ++i)
        {
            octaspire_dern_value_t * const value =
                octaspire_container_vector_get_element_at(vec, (ptrdiff_t)i);

            // TODO XXX check number ranges for too large size_t value for int32_t?
            octaspire_dern_value_t *valueLen = octaspire_dern_vm_create_new_value_integer(
                vm,
                (int32_t)octaspire_dern_value_get_length(value));

            // TODO & or not?
            if (!octaspire_dern_value_as_vector_push_back_element(
                resultVal,
                &valueLen))
            {
                abort();
            }
        }

        octaspire_dern_vm_pop_value(vm, resultVal);

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return resultVal;
    }
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_read_and_eval_path(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    if (octaspire_container_vector_get_length(vec) != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'read-and-eval-path' expects one argument. %zu arguments were given.",
            octaspire_container_vector_get_length(vec));
    }

    octaspire_dern_value_t *path = octaspire_container_vector_get_element_at(vec, 0);

    if (path->typeTag != OCTASPIRE_DERN_VALUE_TAG_STRING)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "First argument to builtin 'read-and-eval-path' must be string (path). Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(path->typeTag));
    }

    octaspire_dern_value_t *result = octaspire_dern_vm_read_from_path_and_eval_in_global_environment(
        vm,
        octaspire_container_utf8_string_get_c_string(path->value.string));

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_read_and_eval_string(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    if (octaspire_container_vector_get_length(vec) != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'read-and-eval-string' expects one argument. %zu arguments were given.",
            octaspire_container_vector_get_length(vec));
    }

    octaspire_dern_value_t *stringToEval = octaspire_container_vector_get_element_at(vec, 0);

    if (stringToEval->typeTag != OCTASPIRE_DERN_VALUE_TAG_STRING)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "First argument to builtin 'read-and-eval-string' must be string (to be evaluated). Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(stringToEval->typeTag));
    }

    octaspire_dern_value_t *result = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        octaspire_container_utf8_string_get_c_string(stringToEval->value.string));

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_string_format(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs == 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'string-format' expects one or more arguments.");
    }

    octaspire_dern_value_t const * const fmtStr =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(fmtStr);

    octaspire_container_utf8_string_t *resultStr = 0;

    if (numArgs == 1)
    {
        octaspire_container_utf8_string_t *str =
            octaspire_dern_value_to_string_plain(fmtStr, octaspire_dern_vm_get_allocator(vm));

        octaspire_dern_value_t *result = octaspire_dern_vm_create_new_value_string(vm, str);

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return result;
    }
    else
    {
        if (fmtStr->typeTag != OCTASPIRE_DERN_VALUE_TAG_STRING)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "First argument to builtin 'string-format' must be format string if there are more "
                "than one argument. Type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(fmtStr->typeTag));

        }

        resultStr = octaspire_container_utf8_string_new(
            "",
            octaspire_dern_vm_get_allocator(vm));

        size_t const fmtStrLen = octaspire_container_utf8_string_get_length_in_ucs_characters(fmtStr->value.string);

        uint32_t prevChar = 0;
        uint32_t curChar = 0;
        uint32_t nextChar = 0;

        size_t fmtValueIndex = 1;

        for (size_t c = 0; c < fmtStrLen; ++c)
        {
            curChar =
                octaspire_container_utf8_string_get_ucs_character_at_index(
                    fmtStr->value.string,
                    (ptrdiff_t)c);

            if ((c + 1) < fmtStrLen)
            {
                nextChar =
                    octaspire_container_utf8_string_get_ucs_character_at_index(
                        fmtStr->value.string,
                        (ptrdiff_t)(c + 1));
            }
            else
            {
                nextChar = 0;
            }

            if (curChar == '\'' && prevChar == '\'')
            {
                octaspire_container_utf8_string_push_back_ucs_character(resultStr, '\'');
                curChar = 0; // To prevent prevChar from becoming '
            }
            else if (curChar == '{' && prevChar != '\'')
            {
                if (nextChar == '}')
                {
                    if (fmtValueIndex >= numArgs)
                    {
                        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                        return octaspire_dern_vm_create_new_value_error_from_c_string(
                            vm,
                            "Not enough arguments for the format string of 'string-format'.");
                    }

                    octaspire_container_utf8_string_t *tmpStr = octaspire_dern_value_to_string_plain(
                        octaspire_dern_value_as_vector_get_element_at(
                            arguments,
                            (ptrdiff_t)fmtValueIndex),
                        octaspire_dern_vm_get_allocator(vm));

                    octaspire_container_utf8_string_concatenate(resultStr, tmpStr);

                    octaspire_container_utf8_string_release(tmpStr);
                    tmpStr = 0;

                    ++c;
                    ++fmtValueIndex;
                }
                else
                {
                    octaspire_container_utf8_string_push_back_ucs_character(
                        resultStr,
                        curChar);
                }
            }
            else
            {
                octaspire_container_utf8_string_push_back_ucs_character(
                    resultStr,
                    curChar);
            }

            prevChar = curChar;
        }
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_string(vm, resultStr);
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_to_string(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs == 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'to-string' expects one or more arguments.");
    }
    else
    {
        octaspire_container_utf8_string_t * const str = octaspire_dern_value_to_string(
            octaspire_dern_value_as_vector_get_element_at_const(
                arguments,
                0),
            octaspire_dern_vm_get_allocator(vm));

        for (size_t i = 1; i < numArgs; ++i)
        {
            octaspire_container_utf8_string_t * tmpStr =
                octaspire_dern_value_to_string(
                octaspire_dern_value_as_vector_get_element_at_const(
                    arguments,
                    (ptrdiff_t)i),
                octaspire_dern_vm_get_allocator(vm));

            octaspire_container_utf8_string_concatenate(str, tmpStr);

            octaspire_container_utf8_string_release(tmpStr);
            tmpStr = 0;
        }

        return octaspire_dern_vm_create_new_value_string(vm, str);
    }
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_to_integer(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs == 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'to-integer' expects one or more arguments.");
    }
    else if (numArgs == 1)
    {
        octaspire_dern_value_t const * const value =
            octaspire_dern_value_as_vector_get_element_at_const(
                arguments,
                0);

        // TODO other types
        if (octaspire_dern_value_is_number(value))
        {
            return octaspire_dern_vm_create_new_value_integer(
                vm,
                (int32_t)(octaspire_dern_value_as_number_get_value(value)));
        }
        else if (octaspire_dern_value_is_string(value))
        {
            int32_t valueAsInt = (int32_t)strtoimax(
                octaspire_dern_value_as_string_get_c_string(value),
                0,
                10);

            return octaspire_dern_vm_create_new_value_integer(vm, valueAsInt);
        }
        else
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "First argument to 'to-integer' is currently unsupported type. "
                "Type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(value->typeTag));
        }
    }
    else
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'to-integer' supports at the moment only one argument.");
    }
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_print(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs == 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'println' one or more arguments.");
    }

    octaspire_dern_value_t const * const fmtStr =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(fmtStr);

    if (fmtStr->typeTag != OCTASPIRE_DERN_VALUE_TAG_STRING)
    {
        octaspire_dern_value_print(fmtStr, octaspire_dern_vm_get_allocator(vm));

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_get_value_true(vm);
    }

    if (numArgs == 1)
    {
        printf("%s", octaspire_container_utf8_string_get_c_string(fmtStr->value.string));
    }
    else
    {
        size_t const fmtStrLen = octaspire_container_utf8_string_get_length_in_ucs_characters(fmtStr->value.string);

        uint32_t prevChar = 0;
        uint32_t curChar = 0;
        uint32_t nextChar = 0;

        size_t fmtValueIndex = 1;

        for (size_t c = 0; c < fmtStrLen; ++c)
        {
            curChar =
                octaspire_container_utf8_string_get_ucs_character_at_index(
                    fmtStr->value.string,
                    (ptrdiff_t)c);

            if ((c + 1) < fmtStrLen)
            {
                nextChar =
                    octaspire_container_utf8_string_get_ucs_character_at_index(
                        fmtStr->value.string,
                        (ptrdiff_t)(c + 1));
            }
            else
            {
                nextChar = 0;
            }

            if (curChar == '\'' && prevChar == '\'')
            {
                printf("'");
                curChar = 0; // To prevent prevChar from becoming '
            }
            else if (curChar == '{' && prevChar != '\'')
            {
                if (nextChar == '}')
                {
                    if (fmtValueIndex >= numArgs)
                    {
                        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                        return octaspire_dern_vm_create_new_value_error_from_c_string(
                            vm,
                            "Not enough arguments for the format string of 'println'.");
                    }

                    octaspire_container_utf8_string_t *tmpStr = octaspire_dern_value_to_string(
                        octaspire_dern_value_as_vector_get_element_at(
                            arguments,
                            (ptrdiff_t)fmtValueIndex),
                        octaspire_dern_vm_get_allocator(vm));

                    printf("%s", octaspire_container_utf8_string_get_c_string(tmpStr));

                    octaspire_container_utf8_string_release(tmpStr);
                    tmpStr = 0;

                    ++c;
                    ++fmtValueIndex;
                }
                else
                {
                    printf("%c",(char)curChar);
                }
            }
            else
            {
                printf("%c",(char)curChar);
            }

            prevChar = curChar;
        }
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_get_value_true(vm);
}


octaspire_dern_value_t *octaspire_dern_vm_builtin_println(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    if (octaspire_dern_value_as_vector_get_length(arguments) == 0)
    {
        printf("\n");
        return octaspire_dern_vm_get_value_true(vm);
    }
    else
    {
        octaspire_dern_value_t *result = octaspire_dern_vm_builtin_print(vm, arguments, environment);
        printf("\n");
        return result;
    }
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_env_new(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    if (octaspire_container_vector_get_length(vec) == 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_environment(vm, 0);
    }
    else if (octaspire_container_vector_get_length(vec) == 1)
    {
        octaspire_dern_value_t *value = octaspire_container_vector_get_element_at(vec, 0);
        octaspire_helpers_verify_not_null(value);

        if (value->typeTag != OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Argument to builtin 'env-new' must be an environment. Now argument has type '%s'.",
                octaspire_dern_value_helper_get_type_as_c_string(value->typeTag));
        }

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_environment(vm, value);
    }
    else
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'env-new' expects zero or one arguments.");
    }
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_env_current(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    if (octaspire_container_vector_get_length(vec) != 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'env-current' expects zero arguments.");
    }

    return environment;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_env_global(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    if (octaspire_container_vector_get_length(vec) != 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'env-global' expects zero arguments.");
    }

    return octaspire_dern_vm_get_global_environment(vm);
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_minus_equals(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    if (octaspire_container_vector_get_length(vec) < 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin '-=' expects at least one argument.");
    }

    octaspire_dern_value_t * const firstArg = octaspire_container_vector_get_element_at(vec, 0);

    switch (firstArg->typeTag)
    {
        case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
        {
            abort();
        }

        case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
        {
            for (size_t i = 1; i < octaspire_container_vector_get_length(vec); ++i)
            {
                octaspire_dern_value_t * const anotherArg =
                    octaspire_container_vector_get_element_at(
                        vec,
                        (ptrdiff_t)i);

                if (i == 1)
                {
                    octaspire_dern_value_as_character_subtract(firstArg, anotherArg);
                }
                else
                {
                    octaspire_dern_value_as_string_pop_back_ucs_character(firstArg);
                }
            }
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_REAL:
        {
            for (size_t i = 1; i < octaspire_container_vector_get_length(vec); ++i)
            {
                octaspire_dern_value_t * const anotherArg =
                    octaspire_container_vector_get_element_at(
                        vec,
                        (ptrdiff_t)i);

                octaspire_dern_value_as_real_subtract(firstArg, anotherArg);
            }
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
        {
            for (size_t i = 1; i < octaspire_container_vector_get_length(vec); ++i)
            {
                octaspire_dern_value_t * const anotherArg =
                    octaspire_container_vector_get_element_at(
                        vec,
                        (ptrdiff_t)i);

                octaspire_dern_value_as_integer_subtract(firstArg, anotherArg);
            }
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
        {
            for (size_t i = 1; i < octaspire_container_vector_get_length(vec); ++i)
            {
                octaspire_dern_value_t * const anotherArg =
                    octaspire_container_vector_get_element_at(
                        vec,
                        (ptrdiff_t)i);

                // TODO XXX remove the need for '&' in &another for vectors!!!!!
                for (size_t j = 0; j < octaspire_dern_value_as_vector_get_length(firstArg); /*NOP*/)
                {
                    octaspire_dern_value_t * const val =
                        octaspire_dern_value_as_vector_get_element_at(
                            firstArg,
                            (ptrdiff_t)j);

                    if (octaspire_dern_value_is_equal(anotherArg, val))
                    {
                        octaspire_dern_value_as_vector_remove_element_at(
                            firstArg,
                            (ptrdiff_t)j);
                    }
                    else
                    {
                        ++j;
                    }
                }
            }
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_STRING:
        {
            for (size_t i = 1; i < octaspire_container_vector_get_length(vec); ++i)
            {
                octaspire_dern_value_t * const anotherArg =
                    octaspire_container_vector_get_element_at(
                        vec,
                        (ptrdiff_t)i);

                if (!octaspire_dern_value_as_string_remove_all_substrings(firstArg, anotherArg))
                {
                    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                    return octaspire_dern_vm_create_new_value_error_from_c_string(
                        vm,
                        "Builtin '-=' failed");
                }
            }
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_HASH_MAP:
        {
            for (size_t i = 1; i < octaspire_container_vector_get_length(vec); ++i)
            {
                octaspire_dern_value_t * const anotherArg =
                    octaspire_container_vector_get_element_at(
                        vec,
                        (ptrdiff_t)i);

                if (!octaspire_dern_value_as_hash_map_remove(firstArg, anotherArg))
                {
                    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                    return octaspire_dern_vm_create_new_value_error_from_c_string(
                        vm,
                        "Builtin '-=' failed");
                }
            }
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_ERROR:
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return firstArg;
        }

        case OCTASPIRE_DERN_VALUE_TAG_NIL:
        case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
        case OCTASPIRE_DERN_VALUE_TAG_MULTILINE_COMMENT:
        case OCTASPIRE_DERN_VALUE_TAG_SYMBOL:
        case OCTASPIRE_DERN_VALUE_TAG_QUEUE:
        case OCTASPIRE_DERN_VALUE_TAG_LIST:
        case OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT:
        case OCTASPIRE_DERN_VALUE_TAG_FUNCTION:
        case OCTASPIRE_DERN_VALUE_TAG_SPECIAL:
        case OCTASPIRE_DERN_VALUE_TAG_BUILTIN:
        case OCTASPIRE_DERN_VALUE_TAG_PORT:
        case OCTASPIRE_DERN_VALUE_TAG_C_DATA:
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "First argument to builtin '-=' cannot be of type '%s'.",
                octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
        }
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return firstArg;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_plus_equals(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    if (octaspire_container_vector_get_length(vec) < 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin '+=' expects at least one argument.");
    }

    octaspire_dern_value_t * const firstArg = octaspire_container_vector_get_element_at(vec, 0);

    switch (firstArg->typeTag)
    {
        case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
        {
            abort();
        }

        case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
        {
            for (size_t i = 1; i < octaspire_container_vector_get_length(vec); ++i)
            {
                octaspire_dern_value_t * const anotherArg =
                    octaspire_container_vector_get_element_at(
                        vec,
                        (ptrdiff_t)i);

                if (i == 1)
                {
                    octaspire_dern_value_as_character_add(firstArg, anotherArg);
                }
                else
                {
                    octaspire_dern_value_as_string_push_back(firstArg, anotherArg);
                }
            }
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_REAL:
        {
            for (size_t i = 1; i < octaspire_container_vector_get_length(vec); ++i)
            {
                octaspire_dern_value_t * const anotherArg =
                    octaspire_container_vector_get_element_at(
                        vec,
                        (ptrdiff_t)i);

                octaspire_dern_value_as_real_add(firstArg, anotherArg);
            }
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
        {
            for (size_t i = 1; i < octaspire_container_vector_get_length(vec); ++i)
            {
                octaspire_dern_value_t * const anotherArg =
                    octaspire_container_vector_get_element_at(
                        vec,
                        (ptrdiff_t)i);

                octaspire_dern_value_as_integer_add(firstArg, anotherArg);
            }
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
        {
            for (size_t i = 1; i < octaspire_container_vector_get_length(vec); ++i)
            {
                octaspire_dern_value_t * const anotherArg =
                    octaspire_container_vector_get_element_at(
                        vec,
                        (ptrdiff_t)i);

                // TODO XXX remove the need for '&' in &another for vectors!!!!!
                octaspire_dern_value_as_vector_push_back_element(firstArg, &anotherArg);
            }
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_LIST:
        {
            for (size_t i = 1; i < octaspire_container_vector_get_length(vec); ++i)
            {
                octaspire_dern_value_t * const anotherArg =
                    octaspire_container_vector_get_element_at(
                        vec,
                        (ptrdiff_t)i);

                octaspire_dern_value_as_list_push_back(firstArg, anotherArg);
            }
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_QUEUE:
        {
            for (size_t i = 1; i < octaspire_container_vector_get_length(vec); ++i)
            {
                octaspire_dern_value_t * const anotherArg =
                    octaspire_container_vector_get_element_at(
                        vec,
                        (ptrdiff_t)i);

                octaspire_dern_value_as_queue_push(firstArg, anotherArg);
            }
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_PORT:
        {
            for (size_t i = 1; i < octaspire_container_vector_get_length(vec); ++i)
            {
                octaspire_dern_value_t * const anotherArg =
                    octaspire_container_vector_get_element_at(
                        vec,
                        (ptrdiff_t)i);

                octaspire_dern_value_t *tmpArgs = octaspire_dern_vm_create_new_value_vector(vm);

                octaspire_dern_vm_push_value(vm, tmpArgs);

                octaspire_dern_value_as_vector_push_back_element(tmpArgs, &firstArg);
                octaspire_dern_value_as_vector_push_back_element(tmpArgs, &anotherArg);

                octaspire_dern_vm_builtin_port_write(vm, tmpArgs, environment);

                octaspire_dern_vm_pop_value(vm, tmpArgs);
            }
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_STRING:
        {
            //bool success = true;
            for (size_t i = 1; i < octaspire_container_vector_get_length(vec); ++i)
            {
                octaspire_dern_value_t * const anotherArg =
                    octaspire_container_vector_get_element_at(
                        vec,
                        (ptrdiff_t)i);

                if (!octaspire_dern_value_as_string_push_back(firstArg, anotherArg))
                {
                    //success = false;
                }
            }

            // TODO report possible error?
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_SYMBOL:
        {
            //bool success = true;
            for (size_t i = 1; i < octaspire_container_vector_get_length(vec); ++i)
            {
                octaspire_dern_value_t * const anotherArg =
                    octaspire_container_vector_get_element_at(
                        vec,
                        (ptrdiff_t)i);

                if (!octaspire_dern_value_as_symbol_push_back(firstArg, anotherArg))
                {
                    //success = false;
                }
            }

            // TODO report possible error?
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_HASH_MAP:
        {
            if (octaspire_container_vector_get_length(vec) == 2)
            {
                if (!octaspire_dern_value_as_hash_map_add(
                        firstArg,
                        octaspire_container_vector_get_element_at(vec, 1),
                        0))
                {
                    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                    return octaspire_dern_vm_create_new_value_error_from_c_string(
                        vm,
                        "Builtin '+=' failed");
                }
            }
            else if (octaspire_container_vector_get_length(vec) == 3)
            {
                if (!octaspire_dern_value_as_hash_map_add(
                        firstArg,
                        octaspire_container_vector_get_element_at(vec, 1),
                        octaspire_container_vector_get_element_at(vec, 2)))
                {
                    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                    return octaspire_dern_vm_create_new_value_error_from_c_string(
                        vm,
                        "Builtin '+=' failed");
                }
            }
            else
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_from_c_string(
                    vm,
                    "Builtin '+=' expects one or two additional arguments for hash map");
            }
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_ERROR:
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return firstArg;
        }

        case OCTASPIRE_DERN_VALUE_TAG_NIL:
        case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
        case OCTASPIRE_DERN_VALUE_TAG_MULTILINE_COMMENT:
        case OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT:
        case OCTASPIRE_DERN_VALUE_TAG_FUNCTION:
        case OCTASPIRE_DERN_VALUE_TAG_SPECIAL:
        case OCTASPIRE_DERN_VALUE_TAG_BUILTIN:
        case OCTASPIRE_DERN_VALUE_TAG_C_DATA:
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "First argument to builtin '+=' cannot be of type '%s'.",
                octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
        }
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return firstArg;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_plus_plus(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    if (octaspire_container_vector_get_length(vec) < 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin '++' expects at least one argument.");
    }

    octaspire_dern_value_t *value = 0;

    for (size_t i = 0; i < octaspire_container_vector_get_length(vec); ++i)
    {
        value = octaspire_container_vector_get_element_at(vec, (ptrdiff_t)i);

        if (!octaspire_dern_value_is_number(value))
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Arguments to builtin '++' must be numbers. %zuth argument has type '%s'.",
                i + 1,
                octaspire_dern_value_helper_get_type_as_c_string(value->typeTag));
        }

        if (value->typeTag == OCTASPIRE_DERN_VALUE_TAG_INTEGER)
        {
            ++(value->value.integer);
        }
        else
        {
            ++(value->value.real);
        }
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return value;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_minus_minus(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    if (octaspire_container_vector_get_length(vec) < 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin '--' expects at least one argument.");
    }

    octaspire_dern_value_t *value = 0;

    for (size_t i = 0; i < octaspire_container_vector_get_length(vec); ++i)
    {
        value = octaspire_container_vector_get_element_at(vec, (ptrdiff_t)i);

        if (octaspire_dern_value_is_integer(value))
        {
            --(value->value.integer);
        }
        else if (octaspire_dern_value_is_real(value))
        {
            --(value->value.real);
        }
        else if (octaspire_dern_value_is_queue(value))
        {
            if (!octaspire_dern_value_as_queue_pop(value))
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin '--' failed on %zuth argument (queue)",
                    i + 1);
            }
        }
        else if (octaspire_dern_value_is_list(value))
        {
            if (!octaspire_dern_value_as_list_pop_back(value))
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin '--' failed on %zuth argument (list)",
                    i + 1);
            }
        }
        else if (octaspire_dern_value_is_vector(value))
        {
            if (!octaspire_dern_value_as_vector_pop_back_element(value))
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin '--' failed on %zuth argument (vector)",
                    i + 1);
            }
        }
        else if (octaspire_dern_value_is_string(value))
        {
            if (!octaspire_dern_value_as_string_pop_back_ucs_character(value))
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin '--' failed on %zuth argument (string)",
                    i + 1);
            }
        }
        else
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "%zuth argument to builtin '--' has unsupported type '%s'.",
                i + 1,
                octaspire_dern_value_helper_get_type_as_c_string(value->typeTag));
        }
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return value;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_pop_front(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    if (octaspire_dern_value_as_vector_get_length(arguments) != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'pop_front' expects exactly one argument.");
    }

    octaspire_dern_value_t *value = octaspire_dern_value_as_vector_get_element_at(arguments, 0);

    if (value->typeTag != OCTASPIRE_DERN_VALUE_TAG_VECTOR)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'pop_front' expects vector argument.");
    }

    octaspire_container_vector_t *vec = value->value.vector;

    octaspire_helpers_verify_not_null(vec);

    bool const result = octaspire_container_vector_pop_front_element(vec);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, result);
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_mod(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs != 2)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'mod' expects two arguments (integers). %zu arguments were given.",
            numArgs);
    }

    octaspire_dern_value_t * const firstArgVal =
        octaspire_dern_value_as_vector_get_element_at(arguments, 0);

    if (firstArgVal->typeTag != OCTASPIRE_DERN_VALUE_TAG_INTEGER)
    {
        if (firstArgVal->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return firstArgVal;
        }

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "The first argument to builtin 'mod' must be integer. Type %s was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArgVal->typeTag));
    }

    octaspire_dern_value_t * const secondArgVal =
        octaspire_dern_value_as_vector_get_element_at(arguments, 1);

    if (secondArgVal->typeTag != OCTASPIRE_DERN_VALUE_TAG_INTEGER)
    {
        if (secondArgVal->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return secondArgVal;
        }

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "The second argument to builtin 'mod' must be integer. Type %s was given.",
            octaspire_dern_value_helper_get_type_as_c_string(secondArgVal->typeTag));
    }

    if (secondArgVal->value.integer == 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "The second argument to builtin 'mod' cannot be zero. "
            "It would cause division by zero.");
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_integer(
        vm,
        firstArgVal->value.integer % secondArgVal->value.integer);
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_slash(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs < 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin '/' expects at least one numeric argument (integer or real).");
    }

    if (numArgs == 1)
    {
        octaspire_dern_value_t *currentArg =
            octaspire_dern_value_as_vector_get_element_at(arguments, 0);

        octaspire_helpers_verify_not_null(currentArg);

        switch (currentArg->typeTag)
        {
            case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
            {
                abort();
            }

            case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

                double const currentArgAsReal = currentArg->value.integer;

                // Fix warning on clang's -Weverything level
                if (currentArgAsReal >= 0 &&
                    currentArgAsReal <= 0)
                {
                    return octaspire_dern_vm_create_new_value_error_from_c_string(
                        vm,
                        "First argument to builtin '/' cannot be zero. "
                        "It would cause division by zero.");
                }
                else
                {
                    return octaspire_dern_vm_create_new_value_real(vm, 1.0 / currentArgAsReal);
                }
            }

            case OCTASPIRE_DERN_VALUE_TAG_REAL:
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

                double const currentArgAsReal = currentArg->value.real;

                if (currentArgAsReal >= 0 && currentArgAsReal <= 0)
                {
                    return octaspire_dern_vm_create_new_value_error_from_c_string(
                        vm,
                        "First argument to builtin '/' cannot be zero. "
                        "It would cause division by zero.");
                }
                else
                {
                    return octaspire_dern_vm_create_new_value_real(vm, 1.0 / currentArgAsReal);
                }
            }

            case OCTASPIRE_DERN_VALUE_TAG_NIL:
            case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
            case OCTASPIRE_DERN_VALUE_TAG_STRING:
            case OCTASPIRE_DERN_VALUE_TAG_MULTILINE_COMMENT:
            case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
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
            {
                if (currentArg->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                {
                    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                    return currentArg;
                }

                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin '/' expects numeric arguments (integer or real). First argument has type %s.",
                    octaspire_dern_value_helper_get_type_as_c_string(currentArg->typeTag));
            }
        }
    }

    double realResult = 1;

    for (size_t i = 0; i < numArgs; ++i)
    {
        octaspire_dern_value_t *currentArg =
            octaspire_dern_value_as_vector_get_element_at(
                arguments,
                (ptrdiff_t)i);

        octaspire_helpers_verify_not_null(currentArg);

        double currentValueAsNumber = 0;

        switch (currentArg->typeTag)
        {
            case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
            {
                abort();
            }

            case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
            {
                currentValueAsNumber = currentArg->value.integer;
            }
            break;

            case OCTASPIRE_DERN_VALUE_TAG_REAL:
            {
                currentValueAsNumber = currentArg->value.real;
            }
            break;

            case OCTASPIRE_DERN_VALUE_TAG_NIL:
            case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
            case OCTASPIRE_DERN_VALUE_TAG_STRING:
            case OCTASPIRE_DERN_VALUE_TAG_MULTILINE_COMMENT:
            case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
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
            {
                if (currentArg->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                {
                    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                    return currentArg;
                }

                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin '/' expects numeric arguments (integer or real). %zuth argument has type %s.",
                    i + 1,
                    octaspire_dern_value_helper_get_type_as_c_string(currentArg->typeTag));
            }
        }

        if (i == 0)
        {
            realResult = currentValueAsNumber;
        }
        else
        {
            if (currentValueAsNumber >= 0 && currentValueAsNumber <= 0)
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Argument number %zu to builtin '/' cannot be zero. "
                    "It would cause division by zero.",
                    i + 1);
            }

            realResult /= currentValueAsNumber;
        }
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_real(vm, realResult);
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_times(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    bool allArgsAreIntegers = true;
    double realResult = 1;
    int32_t integerResult = 1;

    for (size_t i = 0; i < numArgs; ++i)
    {
        octaspire_dern_value_t *currentArg =
            octaspire_dern_value_as_vector_get_element_at(
                arguments,
                (ptrdiff_t)i);

        octaspire_helpers_verify_not_null(currentArg);

        switch (currentArg->typeTag)
        {
            case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
            {
                abort();
            }

            case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
            {
                integerResult *= currentArg->value.integer;
                realResult    *= (double)currentArg->value.integer;
            }
            break;

            case OCTASPIRE_DERN_VALUE_TAG_REAL:
            {
                allArgsAreIntegers = false;

                integerResult *= currentArg->value.real;
                realResult    *= currentArg->value.real;
            }
            break;

            case OCTASPIRE_DERN_VALUE_TAG_NIL:
            case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
            case OCTASPIRE_DERN_VALUE_TAG_STRING:
            case OCTASPIRE_DERN_VALUE_TAG_MULTILINE_COMMENT:
            case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
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
            {
                if (currentArg->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                {
                    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                    return currentArg;
                }

                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin '*' expects numeric arguments (integer or real). %zuth argument has type %s.",
                    i + 1,
                    octaspire_dern_value_helper_get_type_as_c_string(currentArg->typeTag));
            }
        }
    }

    if (allArgsAreIntegers)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_integer(vm, integerResult);
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_real(vm, realResult);
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_private_plus_numerical(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    bool allArgsAreIntegers = true;
    double realResult = 0;
    int32_t integerResult = 0;

    for (size_t i = 0; i < numArgs; ++i)
    {
        octaspire_dern_value_t *currentArg =
            octaspire_dern_value_as_vector_get_element_at(
                arguments,
                (ptrdiff_t)i);

        octaspire_helpers_verify_not_null(currentArg);

        switch (currentArg->typeTag)
        {
            case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
            {
                abort();
            }

            case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
            {
                integerResult += currentArg->value.integer;
                realResult    += (double)currentArg->value.integer;
            }
            break;

            case OCTASPIRE_DERN_VALUE_TAG_REAL:
            {
                allArgsAreIntegers = false;

                integerResult += currentArg->value.real;
                realResult    += currentArg->value.real;
            }
            break;

            case OCTASPIRE_DERN_VALUE_TAG_NIL:
            case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
            case OCTASPIRE_DERN_VALUE_TAG_STRING:
            case OCTASPIRE_DERN_VALUE_TAG_MULTILINE_COMMENT:
            case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
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
            {
                if (currentArg->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                {
                    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                    return currentArg;
                }

                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin '+' expects numeric arguments (integer or real). %zuth argument has type %s.",
                    i + 1,
                    octaspire_dern_value_helper_get_type_as_c_string(currentArg->typeTag));
            }
        }
    }

    if (allArgsAreIntegers)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_integer(vm, integerResult);
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_real(vm, realResult);
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_private_plus_textual(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    octaspire_dern_value_t *firstArg =
        octaspire_dern_value_as_vector_get_element_at(arguments, 0);

    octaspire_helpers_verify_not_null(firstArg);
    octaspire_helpers_verify_true(firstArg->typeTag == OCTASPIRE_DERN_VALUE_TAG_STRING);

    for (size_t i = 1; i < numArgs; ++i)
    {
        octaspire_dern_value_t *currentArg =
            octaspire_dern_value_as_vector_get_element_at(
                arguments,
                (ptrdiff_t)i);

        octaspire_helpers_verify_not_null(currentArg);

        switch (currentArg->typeTag)
        {
            case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
            {
                abort();
            }

            case OCTASPIRE_DERN_VALUE_TAG_STRING:
            {
                octaspire_container_utf8_string_concatenate(
                    firstArg->value.string,
                    currentArg->value.string);
            }
            break;

            case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
            {
                octaspire_container_utf8_string_concatenate(
                    firstArg->value.string,
                    currentArg->value.character);
            }
            break;

            // TODO Add symbol and maybe automatic conversion from number -> string

            case OCTASPIRE_DERN_VALUE_TAG_NIL:
            case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
            case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
            case OCTASPIRE_DERN_VALUE_TAG_REAL:
            case OCTASPIRE_DERN_VALUE_TAG_MULTILINE_COMMENT:
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
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin '+' expects textual arguments if the first argument is textual. %zuth argument has type %s.",
                    i + 1,
                    octaspire_dern_value_helper_get_type_as_c_string(currentArg->typeTag));
            }
        }
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    // TODO what to return? Count of removals or the modified string-value?
    return firstArg;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_private_minus_numerical(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    bool allArgsAreIntegers = true;
    double realResult = 0;
    int32_t integerResult = 0;

    for (size_t i = 0; i < numArgs; ++i)
    {
        octaspire_dern_value_t *currentArg =
            octaspire_dern_value_as_vector_get_element_at(
                arguments,
                (ptrdiff_t)i);

        octaspire_helpers_verify_not_null(currentArg);

        switch (currentArg->typeTag)
        {
            case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
            {
                abort();
            }

            case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
            {
                if (numArgs == 1)
                {
                    integerResult = -currentArg->value.integer;
                    realResult    = -(double)currentArg->value.integer;
                }
                else
                {
                    if (i == 0)
                    {
                        integerResult = currentArg->value.integer;
                        realResult    = (double)currentArg->value.integer;
                    }
                    else
                    {
                        integerResult -= currentArg->value.integer;
                        realResult    -= (double)currentArg->value.integer;
                    }
                }
            }
            break;

            case OCTASPIRE_DERN_VALUE_TAG_REAL:
            {
                allArgsAreIntegers = false;

                if (numArgs == 1)
                {
                    integerResult = (int32_t)-currentArg->value.real;
                    realResult    = -currentArg->value.real;
                }
                else
                {
                    if (i == 0)
                    {
                        integerResult = (int32_t)currentArg->value.real;
                        realResult    = currentArg->value.real;
                    }
                    else
                    {
                        integerResult -= currentArg->value.real;
                        realResult    -= currentArg->value.real;
                    }
                }
            }
            break;

            case OCTASPIRE_DERN_VALUE_TAG_NIL:
            case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
            case OCTASPIRE_DERN_VALUE_TAG_STRING:
            case OCTASPIRE_DERN_VALUE_TAG_MULTILINE_COMMENT:
            case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
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
            {
                if (currentArg->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                {
                    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                    return currentArg;
                }

                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin '-' expects numeric arguments (integer or real). %zuth argument has type %s.",
                    i + 1,
                    octaspire_dern_value_helper_get_type_as_c_string(currentArg->typeTag));
            }
        }
    }

    if (allArgsAreIntegers)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_integer(vm, integerResult);
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_real(vm, realResult);
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_private_minus_textual(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    octaspire_dern_value_t *firstArg =
        octaspire_dern_value_as_vector_get_element_at(arguments, 0);

    octaspire_helpers_verify_not_null(firstArg);
    octaspire_helpers_verify_true(firstArg->typeTag == OCTASPIRE_DERN_VALUE_TAG_STRING);

    for (size_t i = 1; i < numArgs; ++i)
    {
        octaspire_dern_value_t *currentArg =
            octaspire_dern_value_as_vector_get_element_at(
                arguments,
                (ptrdiff_t)i);

        octaspire_helpers_verify_not_null(currentArg);

        switch (currentArg->typeTag)
        {
            case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
            {
                abort();
            }

            case OCTASPIRE_DERN_VALUE_TAG_STRING:
            {
                octaspire_container_utf8_string_remove_all_substrings(
                    firstArg->value.string,
                    currentArg->value.string);
            }
            break;

            case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
            {
                octaspire_container_utf8_string_remove_all_substrings(
                    firstArg->value.string,
                    currentArg->value.character);
            }
            break;

            // TODO Add symbol and maybe automatic conversion from number -> string

            case OCTASPIRE_DERN_VALUE_TAG_NIL:
            case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
            case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
            case OCTASPIRE_DERN_VALUE_TAG_REAL:
            case OCTASPIRE_DERN_VALUE_TAG_MULTILINE_COMMENT:
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
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin '-' expects textual arguments if the first argument is textual. %zuth argument has type %s.",
                    i + 1,
                    octaspire_dern_value_helper_get_type_as_c_string(currentArg->typeTag));
            }
        }
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    // TODO what to return? Count of removals or the modified string-value?
    return firstArg;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_plus(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs == 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_builtin_private_plus_numerical(vm, arguments, environment);
    }

    octaspire_dern_value_t *firstArg =
        octaspire_dern_value_as_vector_get_element_at(arguments, 0);

    octaspire_helpers_verify_not_null(firstArg);

    switch (firstArg->typeTag)
    {
        case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
        {
            abort();
        }

        case OCTASPIRE_DERN_VALUE_TAG_STRING:
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_builtin_private_plus_textual(vm, arguments, environment);
        }

        case OCTASPIRE_DERN_VALUE_TAG_NIL:
        case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
        case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
        case OCTASPIRE_DERN_VALUE_TAG_REAL:
        case OCTASPIRE_DERN_VALUE_TAG_MULTILINE_COMMENT:
        case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
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
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_builtin_private_plus_numerical(vm, arguments, environment);
        }
    }

    abort();
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_minus(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs == 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_builtin_private_minus_numerical(vm, arguments, environment);
    }

    octaspire_dern_value_t *firstArg =
        octaspire_dern_value_as_vector_get_element_at(arguments, 0);

    octaspire_helpers_verify_not_null(firstArg);

    switch (firstArg->typeTag)
    {
        case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
        {
            abort();
        }

        case OCTASPIRE_DERN_VALUE_TAG_STRING:
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_builtin_private_minus_textual(vm, arguments, environment);
        }

        case OCTASPIRE_DERN_VALUE_TAG_NIL:
        case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
        case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
        case OCTASPIRE_DERN_VALUE_TAG_REAL:
        case OCTASPIRE_DERN_VALUE_TAG_MULTILINE_COMMENT:
        case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
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
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_builtin_private_minus_numerical(vm, arguments, environment);
        }
    }

    abort();
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_find(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_dern_vm_push_value(vm, arguments);
    octaspire_dern_vm_push_value(vm, environment);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs < 2)
    {
        octaspire_dern_vm_pop_value(vm, environment);
        octaspire_dern_vm_pop_value(vm, arguments);
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'find' expects at least two arguments. %zu arguments was given.",
            numArgs);
    }

    octaspire_dern_value_t *container = octaspire_dern_value_as_vector_get_element_at(arguments, 0);

    octaspire_helpers_verify_not_null(container);

    octaspire_dern_value_t *result = 0;

    if (numArgs > 2)
    {
        result = octaspire_dern_vm_create_new_value_vector(vm);
        octaspire_dern_vm_push_value(vm, result);

        for (size_t i = 1; i < numArgs; ++i)
        {
            octaspire_dern_value_t *keyArg =
                octaspire_dern_value_as_vector_get_element_at(
                    arguments,
                    (ptrdiff_t)i);

            octaspire_helpers_verify_not_null(keyArg);

            octaspire_dern_value_t *tmpVal = octaspire_dern_vm_find_from_value(vm, container, keyArg);
            octaspire_dern_value_as_vector_push_back_element(
                result,
                &tmpVal);
        }

        octaspire_dern_vm_pop_value(vm, result);
        octaspire_dern_vm_pop_value(vm, environment);
        octaspire_dern_vm_pop_value(vm, arguments);
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return result;
    }
    else
    {
        octaspire_dern_value_t *keyArg =
            octaspire_dern_value_as_vector_get_element_at(arguments, 1);

        octaspire_helpers_verify_not_null(keyArg);

        result = octaspire_dern_vm_find_from_value(vm, container, keyArg);

        octaspire_dern_vm_pop_value(vm, environment);
        octaspire_dern_vm_pop_value(vm, arguments);
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return result;
    }
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_split(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs != 2)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'split' expects two arguments. %zu arguments was given.",
            numArgs);
    }

    octaspire_dern_value_t *container = octaspire_dern_value_as_vector_get_element_at(arguments, 0);
    octaspire_helpers_verify_not_null(container);

    octaspire_dern_value_t *splitByArg = octaspire_dern_value_as_vector_get_element_at(arguments, 1);
    octaspire_helpers_verify_not_null(splitByArg);

    switch (container->typeTag)
    {
        case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
        {
            abort();
        }

        case OCTASPIRE_DERN_VALUE_TAG_STRING:
        {
            if (!octaspire_dern_value_is_character(splitByArg))
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "The second argument to builtin 'split' must be a character when the first "
                    "is a string. Type '%s' was given.",
                    octaspire_dern_value_helper_get_type_as_c_string(splitByArg->typeTag));
            }

            octaspire_dern_value_t * const result = octaspire_dern_vm_create_new_value_vector(vm);
            octaspire_helpers_verify_not_null(result);

            octaspire_dern_vm_push_value(vm, result);

            octaspire_container_utf8_string_t *containerAsStr = container->value.string;
            octaspire_helpers_verify_not_null(containerAsStr);

            octaspire_container_vector_t *tokens = octaspire_container_utf8_string_split(
                containerAsStr,
                octaspire_dern_value_as_character_get_c_string(splitByArg));

            octaspire_helpers_verify_not_null(tokens);

            for (size_t i = 0; i < octaspire_container_vector_get_length(tokens); ++i)
            {
                octaspire_container_utf8_string_t const * const token =
                    (octaspire_container_utf8_string_t const * const)
                        octaspire_container_vector_get_element_at(
                            tokens,
                            (ptrdiff_t)i);

                octaspire_helpers_verify_not_null(token);

                octaspire_container_utf8_string_t * const copy =
                    octaspire_container_utf8_string_new_copy(
                        token,
                        octaspire_dern_vm_get_allocator(vm));

                octaspire_dern_value_t * const copyVal = octaspire_dern_vm_create_new_value_string(
                    vm,
                    copy);

                octaspire_dern_value_as_vector_push_back_element(result, &copyVal);
            }

            octaspire_container_vector_release(tokens);
            tokens = 0;

            octaspire_dern_vm_pop_value(vm, result);
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return result;
        }

        case OCTASPIRE_DERN_VALUE_TAG_NIL:
        case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
        case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
        case OCTASPIRE_DERN_VALUE_TAG_REAL:
        case OCTASPIRE_DERN_VALUE_TAG_MULTILINE_COMMENT:
        case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
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
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "The first argument to builtin 'split' must be a container. Currently only "
                "strings are supported. Type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(container->typeTag));
        }
    }

    abort();
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_hash_map(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    octaspire_dern_value_t *result = octaspire_dern_vm_create_new_value_hash_map(vm);
    octaspire_dern_vm_push_value(vm, result);

    for (size_t i = 0; i < numArgs; i += 2)
    {
        octaspire_dern_value_t *keyArg =
            octaspire_dern_value_as_vector_get_element_at(
                arguments,
                (ptrdiff_t)i);

        if (!keyArg)
        {
            octaspire_dern_vm_pop_value(vm, result);
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_from_c_string(
                vm,
                "Builtin 'hash-map' expects key here.");
        }

        octaspire_dern_value_t *valArg =
            octaspire_dern_value_as_vector_get_element_at(
                arguments,
                (ptrdiff_t)(i + 1));

        if (!valArg)
        {
            octaspire_dern_vm_pop_value(vm, result);
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_from_c_string(
                vm,
                "Builtin 'hash-map' expects key here.");
        }

        if (!octaspire_dern_value_as_hash_map_put(
            result,
            octaspire_dern_value_get_hash(keyArg),
            keyArg,
            valArg))
        {
            abort();
        }
    }

    octaspire_dern_vm_pop_value(vm, result);
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_queue(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    octaspire_dern_value_t *result = octaspire_dern_vm_create_new_value_queue(vm);
    octaspire_dern_vm_push_value(vm, result);

    for (size_t i = 0; i < numArgs; ++i)
    {
        octaspire_dern_value_t *arg =
            octaspire_dern_value_as_vector_get_element_at(
                arguments,
                (ptrdiff_t)i);

        if (!arg)
        {
            octaspire_dern_vm_pop_value(vm, result);
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_from_c_string(
                vm,
                "Builtin 'queue' expects value here.");
        }

        if (!octaspire_dern_value_as_queue_push(result, arg))
        {
            abort();
        }
    }

    octaspire_dern_vm_pop_value(vm, result);
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_queue_with_max_length(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs < 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'queue-with-max-length' expects at least one argument. "
            "Now %zu arguments were given.",
            numArgs);
    }

    octaspire_dern_value_t *firstArg = octaspire_dern_value_as_vector_get_element_at(arguments, 0);

    octaspire_helpers_verify_not_null(firstArg);

    if (!octaspire_dern_value_is_integer(firstArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "The first argument to builtin 'queue-with-max-length' must be integer. Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    int32_t const maxQueueLen = octaspire_dern_value_as_integer_get_value(firstArg);

    if (maxQueueLen < 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "The first argument to builtin 'queue-with-max-length' must be non negative integer. "
            "Negative integer %" PRId32 " was given.",
            maxQueueLen);
    }

    octaspire_dern_value_t *result =
        octaspire_dern_vm_create_new_value_queue_with_max_length(
            vm,
            (size_t)maxQueueLen);

    octaspire_dern_vm_push_value(vm, result);

    for (size_t i = 1; i < numArgs; ++i)
    {
        octaspire_dern_value_t *arg =
            octaspire_dern_value_as_vector_get_element_at(
                arguments,
                (ptrdiff_t)i);

        if (!arg)
        {
            octaspire_dern_vm_pop_value(vm, result);
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_from_c_string(
                vm,
                "Builtin 'queue-with-max-length' expects value here.");
        }

        if (!octaspire_dern_value_as_queue_push(result, arg))
        {
            abort();
        }
    }

    octaspire_dern_vm_pop_value(vm, result);
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_list(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    octaspire_dern_value_t *result = octaspire_dern_vm_create_new_value_list(vm);
    octaspire_dern_vm_push_value(vm, result);

    for (size_t i = 0; i < numArgs; ++i)
    {
        octaspire_dern_value_t *arg =
            octaspire_dern_value_as_vector_get_element_at(
                arguments,
                (ptrdiff_t)i);

        if (!arg)
        {
            octaspire_dern_vm_pop_value(vm, result);
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_from_c_string(
                vm,
                "Builtin 'list' expects value here.");
        }

        if (!octaspire_dern_value_as_list_push_back(result, arg))
        {
            abort();
        }
    }

    octaspire_dern_vm_pop_value(vm, result);
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_private_require_is_already_loaded(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    if (octaspire_dern_value_as_vector_get_length(arguments) != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'require' expects exactly one argument.");
    }

    octaspire_dern_value_t *firstArg = octaspire_dern_value_as_vector_get_element_at(arguments, 0);

    octaspire_helpers_verify_not_null(firstArg);

    if (firstArg->typeTag != OCTASPIRE_DERN_VALUE_TAG_SYMBOL)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "The first argument to builtin 'require' must be a symbol. Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    char const * const name = octaspire_dern_value_as_symbol_get_c_string(firstArg);

    octaspire_helpers_verify_not_null(name);

    if (octaspire_dern_vm_has_library(vm, name))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_boolean(vm, true);
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return 0;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_private_require_source_file(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    if (octaspire_dern_value_as_vector_get_length(arguments) != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'require' expects exactly one argument.");
    }

    octaspire_dern_value_t *firstArg = octaspire_dern_value_as_vector_get_element_at(arguments, 0);

    octaspire_helpers_verify_not_null(firstArg);

    if (firstArg->typeTag != OCTASPIRE_DERN_VALUE_TAG_SYMBOL)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "The first argument to builtin 'require' must be a symbol. Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    char const * const name = octaspire_dern_value_as_symbol_get_c_string(firstArg);

    octaspire_helpers_verify_not_null(name);

    octaspire_container_utf8_string_t *fileName = octaspire_container_utf8_string_new_format(
        octaspire_dern_vm_get_allocator(vm),
        "%s.dern",
        name);

    octaspire_helpers_verify_not_null(fileName);

    octaspire_input_t *input = octaspire_input_new_from_path(
        octaspire_container_utf8_string_get_c_string(fileName),
        octaspire_dern_vm_get_allocator(vm),
        octaspire_dern_vm_get_stdio(vm));

    if (!input)
    {
        octaspire_container_utf8_string_release(fileName);
        fileName = 0;

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return 0;
    }

    octaspire_dern_lib_t *library = octaspire_dern_lib_new_source(
        name,
        input,
        vm,
        octaspire_dern_vm_get_allocator(vm));

    octaspire_helpers_verify_not_null(library);

    if (!octaspire_dern_lib_is_good(library))
    {
        octaspire_dern_value_t *result = octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Source library '%s' failed to load: %s",
            name,
            octaspire_dern_lib_get_error_message(library));

        octaspire_helpers_verify_not_null(result);

        octaspire_container_utf8_string_release(fileName);
        fileName = 0;

        octaspire_input_release(input);
        input = 0;

        octaspire_dern_lib_release(library);
        library = 0;

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return result;
    }

    octaspire_input_release(input);
    input = 0;

    if (!octaspire_dern_vm_add_library(vm, name, library))
    {
        abort();
    }

    octaspire_dern_value_t *result = octaspire_dern_vm_create_new_value_string(vm, fileName);

    octaspire_helpers_verify_not_null(result);

    octaspire_container_utf8_string_release(fileName);
    fileName = 0;

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_private_require_binary_file(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    if (octaspire_dern_value_as_vector_get_length(arguments) != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'require' expects exactly one argument.");
    }

    octaspire_dern_value_t *firstArg = octaspire_dern_value_as_vector_get_element_at(arguments, 0);

    octaspire_helpers_verify_not_null(firstArg);

    if (firstArg->typeTag != OCTASPIRE_DERN_VALUE_TAG_SYMBOL)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "The first argument to builtin 'require' must be a symbol. Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    char const * const name = octaspire_dern_value_as_symbol_get_c_string(firstArg);

    octaspire_helpers_verify_not_null(name);

    octaspire_container_utf8_string_t *fileName = octaspire_container_utf8_string_new_format(
        octaspire_dern_vm_get_allocator(vm),
        "lib%s.so",
        name);

    octaspire_helpers_verify_not_null(fileName);

    octaspire_dern_lib_t *library = octaspire_dern_lib_new_binary(
        name,
        octaspire_container_utf8_string_get_c_string(fileName),
        vm,
        octaspire_dern_vm_get_allocator(vm));

    octaspire_helpers_verify_not_null(library);

    if (!octaspire_dern_lib_is_good(library))
    {
        octaspire_dern_value_t *result = octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Binary library '%s' failed to load: %s",
            name,
            octaspire_dern_lib_get_error_message(library));

        octaspire_helpers_verify_not_null(result);

        octaspire_container_utf8_string_release(fileName);
        fileName = 0;

        octaspire_dern_lib_release(library);
        library = 0;

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return result;
    }

    if (!octaspire_dern_vm_add_library(vm, name, library))
    {
        abort();
    }

    octaspire_dern_value_t *result = octaspire_dern_vm_create_new_value_string(vm, fileName);

    octaspire_helpers_verify_not_null(result);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_ln_at_sign(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs < 2)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'ln@' expects at least two arguments.");
    }

    octaspire_dern_value_t * const collectionVal =
        octaspire_dern_value_as_vector_get_element_at(arguments, 0);

    octaspire_helpers_verify_not_null(collectionVal);

    switch (octaspire_dern_value_get_type(collectionVal))
    {
        case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
        {
            abort();
        }

        case OCTASPIRE_DERN_VALUE_TAG_STRING:
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return octaspire_dern_vm_create_new_value_error_from_c_string(
                vm,
                "Builtin 'ln@' cannot be used with strings. Use 'cp@' instead.");
        }

        case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
        {
            if (numArgs > 2)
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_from_c_string(
                    vm,
                    "Builtin 'ln@' expects exactly two arguments when used with vector.");
            }

            octaspire_dern_value_t const * const indexVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

            octaspire_helpers_verify_not_null(indexVal);

            if (!octaspire_dern_value_is_integer(indexVal))
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'ln@' expects integer as second argument when indexing a vector. "
                    "Now type '%s' was given.",
                    octaspire_dern_value_helper_get_type_as_c_string(
                        indexVal->typeTag));
            }

            ptrdiff_t const index =
                (ptrdiff_t)octaspire_dern_value_as_integer_get_value(indexVal);

            if (!octaspire_dern_value_as_vector_is_index_valid(
                    collectionVal,
                    index))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Index to builtin 'ln@' is not valid for the given vector. "
                    "Index '%td' was given.",
                    index);
            }

            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_value_as_vector_get_element_at(
                collectionVal,
                octaspire_dern_value_as_integer_get_value(indexVal));
        }

        case OCTASPIRE_DERN_VALUE_TAG_HASH_MAP:
        {
            if (numArgs != 3)
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_from_c_string(
                    vm,
                    "Builtin 'ln@' expects exactly three arguments when used with hash map.");
            }

            octaspire_dern_value_t const * const symbolVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, 2);

            octaspire_helpers_verify_not_null(symbolVal);

            if (!octaspire_dern_value_is_symbol(symbolVal))
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'ln@' expects symbol as third argument when indexing a hash map. "
                    "Now type '%s' was given.",
                    octaspire_dern_value_helper_get_type_as_c_string(
                        symbolVal->typeTag));
            }

            octaspire_dern_value_t const * const indexVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

            octaspire_helpers_verify_not_null(indexVal);

            if (octaspire_dern_value_as_text_is_equal_to_c_string(
                    symbolVal,
                    "index"))
            {
                if (!octaspire_dern_value_is_integer(indexVal))
                {
                    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                    return octaspire_dern_vm_create_new_value_error_format(
                        vm,
                        "Builtin 'ln@' expects integer as second argument when indexing a hash map with given symbol 'index'. "
                        "Now type '%s' was given.",
                        octaspire_dern_value_helper_get_type_as_c_string(
                            indexVal->typeTag));
                }

                octaspire_container_hash_map_element_t * const element =
                    octaspire_dern_value_as_hash_map_get_at_index(
                        collectionVal,
                        octaspire_dern_value_as_integer_get_value(indexVal));

                if (!element)
                {
                    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                    return octaspire_dern_vm_create_new_value_error_from_c_string(
                        vm,
                        "Builtin 'ln@' could not find the requested element from hash map.");

                }

                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_container_hash_map_element_get_value(element);
            }
            else if (octaspire_dern_value_as_text_is_equal_to_c_string(
                    symbolVal,
                    "hash"))
            {
                octaspire_container_hash_map_element_t * const element =
                    octaspire_dern_value_as_hash_map_get(
                        collectionVal,
                        octaspire_dern_value_get_hash(indexVal),
                        indexVal);

                if (!element)
                {
                    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                    return octaspire_dern_vm_create_new_value_error_from_c_string(
                        vm,
                        "Builtin 'ln@' could not find the requested element from hash map.");

                }

                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_container_hash_map_element_get_value(element);
            }
            else
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'ln@' expects symbol 'hash' or 'index' as third argument when indexing a hash map. "
                    "Now symbol '%s' was given.",
                    octaspire_dern_value_as_text_get_c_string(symbolVal));
            }
        }

        case OCTASPIRE_DERN_VALUE_TAG_QUEUE:
        {
            abort();
        }

        case OCTASPIRE_DERN_VALUE_TAG_LIST:
        {
            if (numArgs > 2)
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_from_c_string(
                    vm,
                    "Builtin 'ln@' expects exactly two arguments when used with list.");
            }

            octaspire_dern_value_t const * const indexVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

            octaspire_helpers_verify_not_null(indexVal);

            if (!octaspire_dern_value_is_integer(indexVal))
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'ln@' expects integer as second argument when indexing a list. "
                    "Now type '%s' was given.",
                    octaspire_dern_value_helper_get_type_as_c_string(
                        indexVal->typeTag));
            }

            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_value_as_list_get_element_at(
                collectionVal,
                octaspire_dern_value_as_integer_get_value(indexVal));
        }

        case OCTASPIRE_DERN_VALUE_TAG_NIL:
        case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
        case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
        case OCTASPIRE_DERN_VALUE_TAG_REAL:
        case OCTASPIRE_DERN_VALUE_TAG_MULTILINE_COMMENT:
        case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
        case OCTASPIRE_DERN_VALUE_TAG_SYMBOL:
        case OCTASPIRE_DERN_VALUE_TAG_ERROR:
        case OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT:
        case OCTASPIRE_DERN_VALUE_TAG_FUNCTION:
        case OCTASPIRE_DERN_VALUE_TAG_SPECIAL:
        case OCTASPIRE_DERN_VALUE_TAG_BUILTIN:
        case OCTASPIRE_DERN_VALUE_TAG_PORT:
        case OCTASPIRE_DERN_VALUE_TAG_C_DATA:
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'ln@' expects vector, hash map, queue or list as first argument. "
                "Now type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(collectionVal->typeTag));
        }
    }

    abort();
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_cp_at_sign(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs < 2)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'cp@' expects at least two arguments.");
    }

    octaspire_dern_value_t * const collectionVal =
        octaspire_dern_value_as_vector_get_element_at(arguments, 0);

    octaspire_helpers_verify_not_null(collectionVal);

    switch (octaspire_dern_value_get_type(collectionVal))
    {
        case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
        {
            abort();
        }

        case OCTASPIRE_DERN_VALUE_TAG_STRING:
        {
            if (numArgs > 2)
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_from_c_string(
                    vm,
                    "Builtin 'cp@' expects exactly two arguments when used with string.");
            }

            octaspire_dern_value_t const * const indexVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

            octaspire_helpers_verify_not_null(indexVal);

            if (!octaspire_dern_value_is_integer(indexVal))
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'cp@' expects integer as second argument when indexing a string. "
                    "Now type '%s' was given.",
                    octaspire_dern_value_helper_get_type_as_c_string(
                        indexVal->typeTag));
            }

            ptrdiff_t const index =
                (ptrdiff_t)octaspire_dern_value_as_integer_get_value(indexVal);

            if (!octaspire_dern_value_as_string_is_index_valid(
                    collectionVal,
                    index))
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Index to builtin 'cp@' is not valid for the given string. "
                    "Index '%td' was given.",
                    index);
            }

            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return octaspire_dern_vm_create_new_value_character_from_uint32t(
                vm,
                octaspire_container_utf8_string_get_ucs_character_at_index(
                    collectionVal->value.string,
                    index));
        }

        case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
        {
            if (numArgs > 2)
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_from_c_string(
                    vm,
                    "Builtin 'cp@' expects exactly two arguments when used with vector.");
            }

            octaspire_dern_value_t const * const indexVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

            octaspire_helpers_verify_not_null(indexVal);

            if (!octaspire_dern_value_is_integer(indexVal))
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'cp@' expects integer as second argument when indexing a vector. "
                    "Now type '%s' was given.",
                    octaspire_dern_value_helper_get_type_as_c_string(
                        indexVal->typeTag));
            }

            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_copy(
                    vm,
                    octaspire_dern_value_as_vector_get_element_at(
                        collectionVal,
                        octaspire_dern_value_as_integer_get_value(indexVal)));
        }

        case OCTASPIRE_DERN_VALUE_TAG_HASH_MAP:
        {
            if (numArgs != 3)
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_from_c_string(
                    vm,
                    "Builtin 'cp@' expects exactly three arguments when used with hash map.");
            }

            octaspire_dern_value_t const * const symbolVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, 2);

            octaspire_helpers_verify_not_null(symbolVal);

            if (!octaspire_dern_value_is_symbol(symbolVal))
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'cp@' expects symbol as third argument when indexing a hash map. "
                    "Now type '%s' was given.",
                    octaspire_dern_value_helper_get_type_as_c_string(
                        symbolVal->typeTag));
            }

            octaspire_dern_value_t const * const indexVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

            octaspire_helpers_verify_not_null(indexVal);

            if (octaspire_dern_value_as_text_is_equal_to_c_string(
                    symbolVal,
                    "index"))
            {
                if (!octaspire_dern_value_is_integer(indexVal))
                {
                    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                    return octaspire_dern_vm_create_new_value_error_format(
                        vm,
                        "Builtin 'cp@' expects integer as second argument when indexing a hash map with given symbol 'index'. "
                        "Now type '%s' was given.",
                        octaspire_dern_value_helper_get_type_as_c_string(
                            indexVal->typeTag));
                }

                octaspire_container_hash_map_element_t * const element =
                    octaspire_dern_value_as_hash_map_get_at_index(
                        collectionVal,
                        octaspire_dern_value_as_integer_get_value(indexVal));

                if (!element)
                {
                    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                    return octaspire_dern_vm_create_new_value_error_from_c_string(
                        vm,
                        "Builtin 'cp@' could not find the requested element from hash map.");

                }

                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_copy(
                    vm,
                    octaspire_container_hash_map_element_get_value(element));
            }
            else if (octaspire_dern_value_as_text_is_equal_to_c_string(
                    symbolVal,
                    "hash"))
            {
                octaspire_container_hash_map_element_t * const element =
                    octaspire_dern_value_as_hash_map_get(
                        collectionVal,
                        octaspire_dern_value_get_hash(indexVal),
                        indexVal);

                if (!element)
                {
                    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                    return octaspire_dern_vm_create_new_value_error_from_c_string(
                        vm,
                        "Builtin 'cp@' could not find the requested element from hash map.");

                }

                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_copy(
                    vm,
                    octaspire_container_hash_map_element_get_value(element));
            }
            else
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'cp@' expects symbol 'hash' or 'index' as third argument when indexing a hash map. "
                    "Now symbol '%s' was given.",
                    octaspire_dern_value_as_text_get_c_string(symbolVal));
            }
        }

        case OCTASPIRE_DERN_VALUE_TAG_QUEUE:
        {
            abort();
        }

        case OCTASPIRE_DERN_VALUE_TAG_LIST:
        {
            if (numArgs > 2)
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_from_c_string(
                    vm,
                    "Builtin 'cp@' expects exactly two arguments when used with list.");
            }

            octaspire_dern_value_t const * const indexVal =
                octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

            octaspire_helpers_verify_not_null(indexVal);

            if (!octaspire_dern_value_is_integer(indexVal))
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'cp@' expects integer as second argument when indexing a list. "
                    "Now type '%s' was given.",
                    octaspire_dern_value_helper_get_type_as_c_string(
                        indexVal->typeTag));
            }

            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_copy(
                vm,
                octaspire_dern_value_as_list_get_element_at(
                    collectionVal,
                    octaspire_dern_value_as_integer_get_value(indexVal)));
        }

        case OCTASPIRE_DERN_VALUE_TAG_NIL:
        case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
        case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
        case OCTASPIRE_DERN_VALUE_TAG_REAL:
        case OCTASPIRE_DERN_VALUE_TAG_MULTILINE_COMMENT:
        case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
        case OCTASPIRE_DERN_VALUE_TAG_SYMBOL:
        case OCTASPIRE_DERN_VALUE_TAG_ERROR:
        case OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT:
        case OCTASPIRE_DERN_VALUE_TAG_FUNCTION:
        case OCTASPIRE_DERN_VALUE_TAG_SPECIAL:
        case OCTASPIRE_DERN_VALUE_TAG_BUILTIN:
        case OCTASPIRE_DERN_VALUE_TAG_PORT:
        case OCTASPIRE_DERN_VALUE_TAG_C_DATA:
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'cp@' expects vector, hash map, queue or list as first argument. "
                "Now type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(collectionVal->typeTag));
        }
    }

    abort();
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_require(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_dern_value_t *result =
        octaspire_dern_vm_builtin_private_require_is_already_loaded(
            vm,
            arguments,
            environment);

    if (result)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return result;
    }

    result =
        octaspire_dern_vm_builtin_private_require_source_file(
            vm,
            arguments,
            environment);

    if (result)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return result;
    }

    result =
        octaspire_dern_vm_builtin_private_require_binary_file(
            vm,
            arguments,
            environment);

    if (result)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return result;
    }

    octaspire_dern_value_t * const nameVal =
        octaspire_dern_value_as_vector_get_element_at(arguments, 0);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_error_format(
        vm,
        "Builtin 'require': '%s' could not be loaded.",
        octaspire_dern_value_as_symbol_get_c_string(nameVal));
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_integer_question_mark(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'integer?' expects one argument.");
    }

    octaspire_dern_value_t const * const value =
        octaspire_dern_value_as_vector_get_element_at(arguments, 0);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_boolean(vm, octaspire_dern_value_is_integer(value));
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_real_question_mark(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'real?' expects one argument.");
    }

    octaspire_dern_value_t const * const value =
        octaspire_dern_value_as_vector_get_element_at(arguments, 0);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_boolean(vm, octaspire_dern_value_is_real(value));
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_number_question_mark(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'number?' expects one argument.");
    }

    octaspire_dern_value_t const * const value =
        octaspire_dern_value_as_vector_get_element_at(arguments, 0);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_boolean(vm, octaspire_dern_value_is_number(value));
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_nil_question_mark(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'nil?' expects one argument.");
    }

    octaspire_dern_value_t const * const value =
        octaspire_dern_value_as_vector_get_element_at(arguments, 0);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_boolean(vm, octaspire_dern_value_is_nil(value));
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_boolean_question_mark(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'boolean?' expects one argument.");
    }

    octaspire_dern_value_t const * const value =
        octaspire_dern_value_as_vector_get_element_at(arguments, 0);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_boolean(vm, octaspire_dern_value_is_boolean(value));
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_character_question_mark(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'character?' expects one argument.");
    }

    octaspire_dern_value_t const * const value =
        octaspire_dern_value_as_vector_get_element_at(arguments, 0);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_boolean(vm, octaspire_dern_value_is_character(value));
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_string_question_mark(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'string?' expects one argument.");
    }

    octaspire_dern_value_t const * const value =
        octaspire_dern_value_as_vector_get_element_at(arguments, 0);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_boolean(vm, octaspire_dern_value_is_string(value));
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_symbol_question_mark(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'symbol?' expects one argument.");
    }

    octaspire_dern_value_t const * const value =
        octaspire_dern_value_as_vector_get_element_at(arguments, 0);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_boolean(vm, octaspire_dern_value_is_symbol(value));
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_vector_question_mark(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'vector?' expects one argument.");
    }

    octaspire_dern_value_t const * const value =
        octaspire_dern_value_as_vector_get_element_at(arguments, 0);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_boolean(vm, octaspire_dern_value_is_vector(value));
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_hash_map_question_mark(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'hash-map?' expects one argument.");
    }

    octaspire_dern_value_t const * const value =
        octaspire_dern_value_as_vector_get_element_at(arguments, 0);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_boolean(vm, octaspire_dern_value_is_hash_map(value));
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_copy(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs < 1 || numArgs > 2)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'copy' expects one or two arguments.");
    }

    octaspire_dern_value_t * const collectionVal =
        octaspire_dern_value_as_vector_get_element_at(arguments, 0);

    octaspire_dern_value_t * predicateVal = 0;

    if (numArgs == 2)
    {
        predicateVal = octaspire_dern_value_as_vector_get_element_at(arguments, 1);
        octaspire_helpers_verify_not_null(predicateVal);

        if (!octaspire_dern_value_is_function(predicateVal))
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'copy' expects function (predicate) as second argument. "
                "Now type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(predicateVal->typeTag));
        }
    }

    switch (collectionVal->typeTag)
    {
        case OCTASPIRE_DERN_VALUE_TAG_ILLEGAL:
        {
            abort();
        }

        case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
        {
            octaspire_container_vector_t * const copyVec =
                octaspire_container_vector_new(
                    sizeof(octaspire_dern_value_t*),
                    true,
                    0,
                    octaspire_dern_vm_get_allocator(vm));

            octaspire_helpers_verify_not_null(copyVec);

            octaspire_dern_value_t * const result =
                octaspire_dern_vm_create_new_value_vector_from_vector(vm, copyVec);

            octaspire_dern_vm_push_value(vm, result);

            for (size_t i = 0; i < octaspire_dern_value_get_length(collectionVal); ++i)
            {
                bool doCopy = true;

                octaspire_dern_value_t * valueThatMightBeCopied =
                    octaspire_dern_value_as_vector_get_element_at(
                        collectionVal,
                        (ptrdiff_t)i);

                if (predicateVal)
                {
                    octaspire_dern_value_t * callVec =
                        octaspire_dern_vm_create_new_value_vector(vm);

                    octaspire_helpers_verify_not_null(callVec);

                    octaspire_dern_vm_push_value(vm, callVec);

                    octaspire_dern_value_as_vector_push_back_element(
                        callVec,
                        &predicateVal);

                    octaspire_dern_value_as_vector_push_back_element(
                        callVec,
                        &valueThatMightBeCopied);

                    octaspire_dern_value_t * const counterVal =
                        octaspire_dern_vm_create_new_value_integer(vm, (int32_t)i);

                    octaspire_dern_value_as_vector_push_back_element(
                        callVec,
                        &counterVal);

                    octaspire_dern_value_t * resultValFromPredicate =
                        octaspire_dern_vm_eval(vm, callVec, environment);

                    octaspire_dern_vm_pop_value(vm, callVec);

                    octaspire_helpers_verify_not_null(resultValFromPredicate);

                    if (!octaspire_dern_value_is_boolean(resultValFromPredicate))
                    {
                        octaspire_dern_vm_pop_value(vm, result);

                        octaspire_helpers_verify_true(stackLength ==
                            octaspire_dern_vm_get_stack_length(vm));

                        return octaspire_dern_vm_create_new_value_error_format(
                            vm,
                            "Second argument to builtin 'copy' must be a function that returns "
                            "boolean value. Now type '%s' was returned.",
                            octaspire_dern_value_helper_get_type_as_c_string(
                                resultValFromPredicate->typeTag));
                    }

                    doCopy = resultValFromPredicate->value.boolean;
                }

                if (doCopy)
                {
                    octaspire_dern_value_t *copyVal =
                        octaspire_dern_vm_create_new_value_copy(
                            vm,
                            valueThatMightBeCopied);

                    octaspire_helpers_verify_true(
                        octaspire_dern_value_as_vector_push_back_element(result, &copyVal));
                }
            }

            octaspire_dern_vm_pop_value(vm, result);
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return result;
        }

        case OCTASPIRE_DERN_VALUE_TAG_STRING:
        {
            if (numArgs == 1)
            {
                octaspire_container_utf8_string_t * const copyStr =
                    octaspire_container_utf8_string_new_copy(
                        collectionVal->value.string,
                        octaspire_dern_vm_get_allocator(vm));

                octaspire_helpers_verify_not_null(copyStr);

                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_string(vm, copyStr);
            }

            octaspire_container_utf8_string_t * const copyStr =
                octaspire_container_utf8_string_new("", octaspire_dern_vm_get_allocator(vm));

            octaspire_helpers_verify_not_null(copyStr);

            for (size_t i = 0; i < octaspire_dern_value_get_length(collectionVal); ++i)
            {
                bool doCopy = true;

                octaspire_dern_value_t * valueThatMightBeCopied =
                    octaspire_dern_vm_create_new_value_character_from_uint32t(
                        vm,
                        octaspire_container_utf8_string_get_ucs_character_at_index(
                            collectionVal->value.string,
                            (ptrdiff_t)i));

                octaspire_helpers_verify_not_null(predicateVal);

                octaspire_dern_vm_push_value(vm, valueThatMightBeCopied);

                octaspire_dern_value_t * callVec =
                    octaspire_dern_vm_create_new_value_vector(vm);

                octaspire_helpers_verify_not_null(callVec);

                octaspire_dern_vm_push_value(vm, callVec);

                octaspire_dern_value_as_vector_push_back_element(
                    callVec,
                    &predicateVal);

                octaspire_dern_value_as_vector_push_back_element(
                    callVec,
                    &valueThatMightBeCopied);

                octaspire_dern_value_t * const counterVal =
                    octaspire_dern_vm_create_new_value_integer(vm, (int32_t)i);

                octaspire_dern_value_as_vector_push_back_element(
                    callVec,
                    &counterVal);

                octaspire_dern_value_t * resultValFromPredicate =
                    octaspire_dern_vm_eval(vm, callVec, environment);

                octaspire_dern_vm_pop_value(vm, callVec);

                octaspire_helpers_verify_not_null(resultValFromPredicate);

                if (!octaspire_dern_value_is_boolean(resultValFromPredicate))
                {
                    octaspire_dern_vm_pop_value(vm, valueThatMightBeCopied);

                    octaspire_helpers_verify_true(stackLength ==
                        octaspire_dern_vm_get_stack_length(vm));

                    return octaspire_dern_vm_create_new_value_error_format(
                        vm,
                        "Second argument to builtin 'copy' must be a function that returns "
                        "boolean value. Now type '%s' was returned.",
                        octaspire_dern_value_helper_get_type_as_c_string(
                            resultValFromPredicate->typeTag));
                }

                octaspire_dern_vm_pop_value(vm, valueThatMightBeCopied);

                doCopy = resultValFromPredicate->value.boolean;

                if (doCopy)
                {
                    octaspire_helpers_verify_true(
                        octaspire_container_utf8_string_push_back_ucs_character(
                            copyStr,
                            octaspire_container_utf8_string_get_ucs_character_at_index(
                                collectionVal->value.string,
                                (ptrdiff_t)i)));
                }
            }

            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_string(vm, copyStr);
        }

        case OCTASPIRE_DERN_VALUE_TAG_C_DATA:
        {
            if (numArgs == 1)
            {
                octaspire_dern_c_data_t *originalCData =
                    octaspire_dern_value_as_c_data_get_value(collectionVal);

                if (!octaspire_dern_c_data_is_copying_allowed(originalCData))
                {
                    octaspire_container_utf8_string_t *tmpStr =
                        octaspire_dern_c_data_to_string(
                            originalCData,
                            octaspire_dern_vm_get_allocator(vm));

                    octaspire_dern_value_t * const result =
                        octaspire_dern_vm_create_new_value_error_format(
                            vm,
                            "Copying of C data '%s' is not allowed (it forbids copying)",
                            octaspire_container_utf8_string_get_c_string(tmpStr));

                    octaspire_container_utf8_string_release(tmpStr);
                    tmpStr = 0;

                    octaspire_helpers_verify_true(
                        stackLength == octaspire_dern_vm_get_stack_length(vm));

                    return result;
                }

                octaspire_dern_c_data_t * const copyCData =
                    octaspire_dern_c_data_new_copy(
                        octaspire_dern_value_as_c_data_get_value(collectionVal),
                        octaspire_dern_vm_get_allocator(vm));

                octaspire_helpers_verify_not_null(copyCData);

                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_c_data_from_existing(
                    vm,
                    copyCData);
            }

            // TODO XXX
            abort();
        }

        case OCTASPIRE_DERN_VALUE_TAG_NIL:
        case OCTASPIRE_DERN_VALUE_TAG_BOOLEAN:
        case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
        case OCTASPIRE_DERN_VALUE_TAG_REAL:
        case OCTASPIRE_DERN_VALUE_TAG_MULTILINE_COMMENT:
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
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'copy' does not support copying of type '%s' at the moment.",
                octaspire_dern_value_helper_get_type_as_c_string(collectionVal->typeTag));
        }
    }

    abort();
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_host_get_command_line_arguments(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'host-get-command-line-arguments' expects zero arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_dern_value_t *result = octaspire_dern_vm_create_new_value_vector(
        vm);

    octaspire_dern_vm_push_value(vm, result);

    for (size_t i = 0;
         i < octaspire_dern_vm_get_number_of_command_line_arguments(vm);
         ++i)
    {
        char const * const str =
            octaspire_dern_vm_get_command_line_argument_at(vm, (ptrdiff_t)i);

        octaspire_helpers_verify_not_null(str);

        octaspire_dern_value_t * const value =
            octaspire_dern_vm_create_new_value_string_from_c_string(vm, str);

        octaspire_helpers_verify_not_null(value);

        octaspire_dern_value_as_vector_push_back_element(
            result,
            &value);
    }

    octaspire_dern_vm_pop_value(vm, result);
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_host_get_environment_variables(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify_true(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify_true(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'host-get-environment-variables' expects zero arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_dern_value_t *result = octaspire_dern_vm_create_new_value_vector(
        vm);

    octaspire_dern_vm_push_value(vm, result);

    for (size_t i = 0;
         i < octaspire_dern_vm_get_number_of_environment_variables(vm);
         ++i)
    {
        char const * const str =
            octaspire_dern_vm_get_environment_variable_at(vm, (ptrdiff_t)i);

        octaspire_helpers_verify_not_null(str);

        octaspire_dern_value_t * const value =
            octaspire_dern_vm_create_new_value_string_from_c_string(vm, str);

        octaspire_helpers_verify_not_null(value);

        octaspire_dern_value_as_vector_push_back_element(
            result,
            &value);
    }

    octaspire_dern_vm_pop_value(vm, result);
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return result;
}

