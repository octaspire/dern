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
#include "octaspire/dern/octaspire_dern_vm.h"

octaspire_dern_value_t *octaspire_dern_stdlib_private_validate_function(
    octaspire_dern_vm_t* vm,
    octaspire_dern_function_t *function);

octaspire_dern_value_t *octaspire_dern_vm_private_special_define_with_four_arguments(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    if (octaspire_container_vector_get_length(vec) != 4)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Four arguments expected. %zu arguments were given.",
            octaspire_container_vector_get_length(vec));
    }

    octaspire_dern_value_t *targetEnv = octaspire_container_vector_get_element_at(vec, 0);

    octaspire_helpers_verify(targetEnv && targetEnv->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_dern_value_t *name = octaspire_container_vector_get_element_at(vec, 1);

    bool nameIsEvaluated = false;

    if (name->typeTag != OCTASPIRE_DERN_VALUE_TAG_SYMBOL)
    {
        nameIsEvaluated = true;

        if (name->typeTag != OCTASPIRE_DERN_VALUE_TAG_VECTOR)
        {
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Special 'define': (define [optional-target-env] symbol...) name to be defined should be symbol or vector to be evaluated. Type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(name->typeTag));
        }

        name = octaspire_dern_vm_eval(vm, name, environment);

        if (name->typeTag != OCTASPIRE_DERN_VALUE_TAG_SYMBOL)
        {
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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

        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
        octaspire_dern_value_t * const valueToBeDefinedToBePopped = valueToBeDefined;
        valueToBeDefined = octaspire_dern_vm_create_new_value_copy(vm, valueToBeDefined);
        octaspire_dern_vm_pop_value(vm, valueToBeDefinedToBePopped);
    }

    octaspire_dern_vm_pop_value(vm, environment);

    if (valueToBeDefined->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
    {
        if (nameIsEvaluated)
        {
            octaspire_dern_vm_pop_value(vm, name);
        }

        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return valueToBeDefined;
    }
    else if (valueToBeDefined->typeTag == OCTASPIRE_DERN_VALUE_TAG_FUNCTION)
    {
        if (nameIsEvaluated)
        {
            octaspire_dern_vm_pop_value(vm, name);
        }

        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, status);
}

octaspire_dern_value_t *octaspire_dern_vm_private_special_define_with_five_arguments(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    if (octaspire_container_vector_get_length(vec) != 5)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Special 'define' expects five arguments. %zu arguments were given.",
            octaspire_container_vector_get_length(vec));
    }


    octaspire_dern_value_t *targetEnv = octaspire_container_vector_get_element_at(vec, 0);

    octaspire_helpers_verify(targetEnv && targetEnv->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_dern_value_t *name = octaspire_container_vector_get_element_at(vec, 1);

    bool nameIsEvaluated = false;

    if (name->typeTag != OCTASPIRE_DERN_VALUE_TAG_SYMBOL)
    {
        nameIsEvaluated = true;

        if (name->typeTag != OCTASPIRE_DERN_VALUE_TAG_VECTOR)
        {
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Special 'define': (define [optional-target-env] name...) Name must be symbol or vector to be evaluated. Type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(name->typeTag));
        }

        name = octaspire_dern_vm_eval(vm, name, environment);


        if (name->typeTag != OCTASPIRE_DERN_VALUE_TAG_SYMBOL)
        {
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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

        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Special 'define': (define [optional-target-env) name docstring...) docstring must be stringy. Type '%s' was given.",
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

        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));

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

        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return valueToBeDefined;
    }
    else if (valueToBeDefined->typeTag == OCTASPIRE_DERN_VALUE_TAG_FUNCTION)
    {
        octaspire_container_utf8_string_t *errorMessage =
            octaspire_dern_function_are_all_formals_mentioned_in_docvec(
                valueToBeDefined->value.function,
                docVec);

        octaspire_helpers_verify(errorMessage);

        if (!octaspire_container_utf8_string_is_empty(errorMessage))
        {
            if (nameIsEvaluated)
            {
                octaspire_dern_vm_pop_value(vm, name);
            }

            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
                        i);

                octaspire_helpers_verify(formalSym);

                octaspire_dern_value_t const * const formalDocStr =
                    octaspire_dern_value_as_vector_get_element_of_type_at_const(
                        docVec,
                        OCTASPIRE_DERN_VALUE_TAG_STRING,
                        i + 1);

                octaspire_helpers_verify(formalDocStr);

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

        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, status);
}

octaspire_dern_value_t *octaspire_dern_vm_special_define(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    size_t const numArgs = octaspire_container_vector_get_length(vec);

    if (numArgs != 3 && numArgs != 4 && numArgs != 5)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Special 'define' expects three, four, or five arguments. %zu arguments were given.",
            numArgs);
    }

    // TODO XXX is there need to push arguments?
    octaspire_dern_vm_push_value(vm, arguments);

    octaspire_dern_value_t *firstArg = octaspire_container_vector_get_element_at(vec, 0);
    octaspire_helpers_verify(firstArg);
    octaspire_dern_value_t *evaluatedFirstArg = octaspire_dern_vm_eval(vm, firstArg, environment);
    octaspire_helpers_verify(evaluatedFirstArg);

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
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return result;
        }
        else if (numArgs == 4)
        {
            octaspire_dern_value_t *result = octaspire_dern_vm_private_special_define_with_five_arguments(
                vm,
                arguments,
                environment);

            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return result;
        }
        else if (numArgs == 5)
        {
            octaspire_dern_value_t *result = octaspire_dern_vm_private_special_define_with_five_arguments(
                vm,
                arguments,
                environment);

            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return result;
        }
        else
        {
            abort();
        }
    }

    octaspire_dern_vm_pop_value(vm, arguments);
    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
}

octaspire_dern_value_t *octaspire_dern_vm_special_quote(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    if (octaspire_container_vector_get_length(vec) != 1)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Special 'quote' expects one argument. %zu arguments were given.",
            octaspire_container_vector_get_length(vec));
    }

    octaspire_dern_value_t *quotedValue = octaspire_container_vector_get_element_at(vec, 0);

    octaspire_helpers_verify(quotedValue);

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return quotedValue;
}

octaspire_dern_value_t *octaspire_dern_vm_special_if(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs != 2 && numArgs != 3)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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

    octaspire_helpers_verify(testResult);
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

    octaspire_helpers_verify(testResult);

    if (testResult->typeTag != OCTASPIRE_DERN_VALUE_TAG_BOOLEAN)
    {
        octaspire_dern_vm_pop_value(vm, arguments);

        if (testResult->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
        {
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return testResult;
        }

        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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

    octaspire_helpers_verify(result);

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_special_while(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs < 2)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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

        octaspire_helpers_verify(testResult);

        if (testResult->typeTag != OCTASPIRE_DERN_VALUE_TAG_BOOLEAN)
        {
            octaspire_dern_vm_pop_value(vm, arguments);

            if (testResult->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
            {
                octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return testResult;
            }

            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
                octaspire_dern_value_as_vector_get_element_at(arguments, i),
                environment);

            octaspire_helpers_verify(result);

            if (result->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
            {
                octaspire_dern_vm_pop_value(vm, arguments);
                octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return result;

            }

            if (octaspire_dern_vm_get_function_return(vm))
            {
                result = octaspire_dern_vm_get_function_return(vm);
                //octaspire_dern_vm_set_function_return(vm, 0);
                octaspire_dern_vm_pop_value(vm, arguments);
                octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return result;
            }
        }

        ++counter;
    };

    octaspire_dern_vm_pop_value(vm, arguments);

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_integer(vm, counter);
}

octaspire_dern_value_t *octaspire_dern_vm_special_for(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t stepSize = 1; // Used for containers and numerical iteration.

    octaspire_dern_vm_push_value(vm, arguments);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs < 4)
    {
        octaspire_dern_vm_pop_value(vm, arguments);

        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Special 'for' expects at least four (for iterating container) or five (for iterating numeric range) arguments. %zu arguments were given.",
            numArgs);
    }

    // 1. argument: counter symbol
    octaspire_dern_value_t *counterSymbol =
        octaspire_dern_value_as_vector_get_element_at(arguments, 0);

    octaspire_helpers_verify(counterSymbol);

    if (counterSymbol->typeTag != OCTASPIRE_DERN_VALUE_TAG_SYMBOL)
    {
        octaspire_dern_vm_pop_value(vm, arguments);

        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "First argument to special 'for' must be symbol value. "
            "Now it has type %s.",
            octaspire_dern_value_helper_get_type_as_c_string(counterSymbol->typeTag));
    }

    // 2. argument: 'in' or 'from' symbol
    octaspire_dern_value_t const * const inOrFromSymbol =
        octaspire_dern_value_as_vector_get_element_at(arguments, 1);

    octaspire_helpers_verify(inOrFromSymbol);

    if (inOrFromSymbol->typeTag != OCTASPIRE_DERN_VALUE_TAG_SYMBOL)
    {
        octaspire_dern_vm_pop_value(vm, arguments);

        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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

        octaspire_helpers_verify(container);

        container = octaspire_dern_vm_eval(vm, container, environment);

        octaspire_helpers_verify(container);

        octaspire_dern_vm_push_value(vm, container);

        if (container->typeTag != OCTASPIRE_DERN_VALUE_TAG_STRING      && 
            container->typeTag != OCTASPIRE_DERN_VALUE_TAG_VECTOR      &&
            container->typeTag != OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT &&
            container->typeTag != OCTASPIRE_DERN_VALUE_TAG_HASH_MAP)
        {
            octaspire_dern_value_t *result = octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Third argument to special 'for' using 'in' must be a container "
                "(string, vector, hash map or environment) "
                "Now it has type %s.",
                octaspire_dern_value_helper_get_type_as_c_string(container->typeTag));

            octaspire_dern_vm_pop_value(vm, container);
            octaspire_dern_vm_pop_value(vm, arguments);

            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return result;
        }




        // Check if there are optional 'step' and then integer
        size_t currentArgIdx = 3;

        // Optional 3 and 4. arguments: step integer
        octaspire_dern_value_t const * const optionalStep =
            octaspire_dern_value_as_vector_get_element_at(arguments, currentArgIdx);

        octaspire_helpers_verify(optionalStep);

        if (optionalStep->typeTag == OCTASPIRE_DERN_VALUE_TAG_SYMBOL &&
            octaspire_container_utf8_string_is_equal_to_c_string(optionalStep->value.symbol, "step"))
        {
            if (numArgs < 5)
            {
                octaspire_dern_vm_pop_value(vm, container);
                octaspire_dern_vm_pop_value(vm, arguments);

                octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Special 'for' expects at least five arguments for iterating containers with "
                    "a given step size. %zu arguments were given.",
                    numArgs);
            }

            // There is 'step'. Now an integer is required next
            ++currentArgIdx;

            octaspire_dern_value_t const * const requiredStepSize =
                octaspire_dern_value_as_vector_get_element_at(arguments, currentArgIdx);

            octaspire_helpers_verify(requiredStepSize);

            if (requiredStepSize->typeTag != OCTASPIRE_DERN_VALUE_TAG_INTEGER)
            {
                octaspire_dern_value_t *result = octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Fifth argument to special 'for' using 'step' with containers must be "
                    "an integer step size. Now it has type %s.",
                    octaspire_dern_value_helper_get_type_as_c_string(requiredStepSize->typeTag));

                octaspire_dern_vm_pop_value(vm, container);
                octaspire_dern_vm_pop_value(vm, arguments);

                octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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

                octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return result;
            }

            stepSize = requiredStepSize->value.integer;
        }




        // Extend env for the counter variable
        octaspire_dern_environment_t *extendedEnvironment =
            octaspire_dern_environment_new(
                environment,
                vm,
                octaspire_dern_vm_get_allocator(vm));

        octaspire_helpers_verify(extendedEnvironment);

        octaspire_dern_value_t *extendedEnvVal =
            octaspire_dern_vm_create_new_value_environment_from_environment(vm, extendedEnvironment);

        octaspire_helpers_verify(extendedEnvVal);

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
                    octaspire_container_utf8_string_get_ucs_character_at_index(str, i));

                octaspire_dern_environment_set(
                    extendedEnvironment,
                    counterSymbol,
                    octaspire_dern_vm_create_new_value_character(vm, charStr));

                for (size_t j = currentArgIdx; j < numArgs; ++j)
                {
                    octaspire_dern_value_t *result = octaspire_dern_vm_eval(
                        vm,
                        octaspire_dern_value_as_vector_get_element_at(arguments, j),
                        extendedEnvVal);

                    octaspire_helpers_verify(result);

                    if (result->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                    {
                        octaspire_dern_vm_pop_value(vm, extendedEnvVal);
                        octaspire_dern_vm_pop_value(vm, container);
                        octaspire_dern_vm_pop_value(vm, arguments);
                        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
                        return result;
                    }

                    if (octaspire_dern_vm_get_function_return(vm))
                    {
                        result = octaspire_dern_vm_get_function_return(vm);
                        //octaspire_dern_vm_set_function_return(vm, 0);
                        octaspire_dern_vm_pop_value(vm, extendedEnvVal);
                        octaspire_dern_vm_pop_value(vm, container);
                        octaspire_dern_vm_pop_value(vm, arguments);
                        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
                        return result;
                    }
                }

                ++counter;
            }

            octaspire_dern_vm_pop_value(vm, extendedEnvVal);
            octaspire_dern_vm_pop_value(vm, container);
            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
                    octaspire_container_vector_get_element_at(vec, i));

                for (size_t j = currentArgIdx; j < numArgs; ++j)
                {
                    octaspire_dern_value_t *result = octaspire_dern_vm_eval(
                        vm,
                        octaspire_dern_value_as_vector_get_element_at(arguments, j),
                        extendedEnvVal);

                    octaspire_helpers_verify(result);

                    if (result->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                    {
                        octaspire_dern_vm_pop_value(vm, extendedEnvVal);
                        octaspire_dern_vm_pop_value(vm, container);
                        octaspire_dern_vm_pop_value(vm, arguments);
                        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
                        return result;
                    }

                    if (octaspire_dern_vm_get_function_return(vm))
                    {
                        result = octaspire_dern_vm_get_function_return(vm);
                        //octaspire_dern_vm_set_function_return(vm, 0);
                        octaspire_dern_vm_pop_value(vm, extendedEnvVal);
                        octaspire_dern_vm_pop_value(vm, container);
                        octaspire_dern_vm_pop_value(vm, arguments);
                        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
                        return result;
                    }
                }

                ++counter;
            }

            octaspire_dern_vm_pop_value(vm, extendedEnvVal);
            octaspire_dern_vm_pop_value(vm, container);
            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
                    octaspire_dern_environment_get_at_index(env, i);

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
                        octaspire_dern_value_as_vector_get_element_at(arguments, j),
                        extendedEnvVal);

                    octaspire_helpers_verify(result);

                    if (result->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                    {
                        octaspire_dern_vm_pop_value(vm, extendedEnvVal);
                        octaspire_dern_vm_pop_value(vm, container);
                        octaspire_dern_vm_pop_value(vm, arguments);
                        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
                        return result;
                    }

                    if (octaspire_dern_vm_get_function_return(vm))
                    {
                        result = octaspire_dern_vm_get_function_return(vm);
                        //octaspire_dern_vm_set_function_return(vm, 0);
                        octaspire_dern_vm_pop_value(vm, extendedEnvVal);
                        octaspire_dern_vm_pop_value(vm, container);
                        octaspire_dern_vm_pop_value(vm, arguments);
                        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
                        return result;
                    }
                }

                ++counter;
            }

            octaspire_dern_vm_pop_value(vm, extendedEnvVal);
            octaspire_dern_vm_pop_value(vm, container);
            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
                    octaspire_container_hash_map_get_at_index(hashMap, i);

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
                        octaspire_dern_value_as_vector_get_element_at(arguments, j),
                        extendedEnvVal);

                    octaspire_helpers_verify(result);

                    if (result->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                    {
                        octaspire_dern_vm_pop_value(vm, extendedEnvVal);
                        octaspire_dern_vm_pop_value(vm, container);
                        octaspire_dern_vm_pop_value(vm, arguments);
                        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
                        return result;
                    }

                    if (octaspire_dern_vm_get_function_return(vm))
                    {
                        result = octaspire_dern_vm_get_function_return(vm);
                        //octaspire_dern_vm_set_function_return(vm, 0);
                        octaspire_dern_vm_pop_value(vm, extendedEnvVal);
                        octaspire_dern_vm_pop_value(vm, container);
                        octaspire_dern_vm_pop_value(vm, arguments);
                        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
                        return result;
                    }
                }

                ++counter;
            }

            octaspire_dern_vm_pop_value(vm, extendedEnvVal);
            octaspire_dern_vm_pop_value(vm, container);
            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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

        octaspire_helpers_verify(fromValue);

        fromValue = octaspire_dern_vm_eval(vm, fromValue, environment);

        octaspire_helpers_verify(fromValue);

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

            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return result;
        }




        // 100
        octaspire_dern_value_t *toValue =
            octaspire_dern_value_as_vector_get_element_at(arguments, 4);

        octaspire_helpers_verify(toValue);

        toValue = octaspire_dern_vm_eval(vm, toValue, environment);

        octaspire_helpers_verify(toValue);

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

            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return result;
        }




        // Check if there are optional 'step' and then integer
        size_t currentArgIdx = 5;

        // Optional 5. and 6. arguments: step integer
        octaspire_dern_value_t const * const optionalStep =
            octaspire_dern_value_as_vector_get_element_at(arguments, currentArgIdx);

        octaspire_helpers_verify(optionalStep);

        if (optionalStep->typeTag == OCTASPIRE_DERN_VALUE_TAG_SYMBOL &&
            octaspire_container_utf8_string_is_equal_to_c_string(optionalStep->value.symbol, "step"))
        {
            if (numArgs < 7)
            {
                octaspire_dern_vm_pop_value(vm, toValue);
                octaspire_dern_vm_pop_value(vm, fromValue);
                octaspire_dern_vm_pop_value(vm, arguments);

                octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Special 'for' expects at least seven arguments in numeric form with "
                    "a given step size. %zu arguments were given.",
                    numArgs);
            }

            // There is 'step'. Now an integer is required next
            ++currentArgIdx;

            octaspire_dern_value_t const * const requiredStepSize =
                octaspire_dern_value_as_vector_get_element_at(arguments, currentArgIdx);

            octaspire_helpers_verify(requiredStepSize);

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

                octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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

                octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return result;
            }


            stepSize = requiredStepSize->value.integer;
        }









        // Extend env for the counter variable
        octaspire_dern_environment_t *extendedEnvironment =
            octaspire_dern_environment_new(
                environment,
                vm,
                octaspire_dern_vm_get_allocator(vm));

        octaspire_helpers_verify(extendedEnvironment);

        octaspire_dern_value_t *extendedEnvVal =
            octaspire_dern_vm_create_new_value_environment_from_environment(vm, extendedEnvironment);

        octaspire_helpers_verify(extendedEnvVal);

        octaspire_dern_vm_push_value(vm, extendedEnvVal);

        bool const fromIsSmaller = octaspire_dern_value_is_less_than_or_equal(fromValue, toValue);

        if (fromIsSmaller)
        {
            size_t const numIterations = (toValue->value.integer - fromValue->value.integer) + 1;

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
                        octaspire_dern_value_as_vector_get_element_at(arguments, j),
                        extendedEnvVal);

                    octaspire_helpers_verify(result);

                    if (result->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                    {
                        octaspire_dern_vm_pop_value(vm, extendedEnvVal);
                        octaspire_dern_vm_pop_value(vm, toValue);
                        octaspire_dern_vm_pop_value(vm, fromValue);
                        octaspire_dern_vm_pop_value(vm, arguments);
                        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
                        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
                        return result;
                    }
                }

                ++counter;
            }

            octaspire_dern_vm_pop_value(vm, extendedEnvVal);
            octaspire_dern_vm_pop_value(vm, toValue);
            octaspire_dern_vm_pop_value(vm, fromValue);
            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_integer(vm, counter);
        }
        else
        {
            size_t const numIterations = (fromValue->value.integer - toValue->value.integer) + 1;

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
                        octaspire_dern_value_as_vector_get_element_at(arguments, j),
                        extendedEnvVal);

                    octaspire_helpers_verify(result);

                    if (result->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                    {
                        octaspire_dern_vm_pop_value(vm, extendedEnvVal);
                        octaspire_dern_vm_pop_value(vm, toValue);
                        octaspire_dern_vm_pop_value(vm, fromValue);
                        octaspire_dern_vm_pop_value(vm, arguments);
                        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
                        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
                        return result;
                    }
                }

                ++counter;
            }

            octaspire_dern_vm_pop_value(vm, extendedEnvVal);
            octaspire_dern_vm_pop_value(vm, toValue);
            octaspire_dern_vm_pop_value(vm, fromValue);
            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_integer(vm, counter);
        }
    }

    abort();
}

// TODO move to other builtins
octaspire_dern_value_t *octaspire_dern_vm_builtin_starts_with_question_mark(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs != 2)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'starts-with?' expects two arguments.");
    }

    octaspire_dern_vm_push_value(vm, arguments);

    octaspire_dern_value_t *firstArg = octaspire_dern_value_as_vector_get_element_at(arguments, 0);
    octaspire_helpers_verify(firstArg);

    octaspire_dern_value_t *secondArg = octaspire_dern_value_as_vector_get_element_at(arguments, 1);
    octaspire_helpers_verify(secondArg);

    if (firstArg->typeTag != secondArg->typeTag)
    {
        octaspire_dern_vm_pop_value(vm, arguments);
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs < 2 || numArgs > 3)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin '=' expects two or three arguments.");
    }

    octaspire_dern_vm_push_value(vm, arguments);

    octaspire_dern_value_t *firstArg = octaspire_dern_value_as_vector_get_element_at(arguments, 0);
    octaspire_helpers_verify(firstArg);

    if (numArgs == 2)
    {
        octaspire_dern_value_t *secondArg = octaspire_dern_value_as_vector_get_element_at(arguments, 1);
        octaspire_helpers_verify(secondArg);

        if (octaspire_dern_value_set(firstArg, secondArg))
        {
            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            //return octaspire_dern_vm_get_value_true(vm);
            return firstArg;
        }

        octaspire_dern_vm_pop_value(vm, arguments);
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_get_value_false(vm);
    }
    else
    {
        octaspire_dern_value_t *secondArg = octaspire_dern_value_as_vector_get_element_at(arguments, 1);
        octaspire_helpers_verify(secondArg);

        octaspire_dern_value_t *thirdArg = octaspire_dern_value_as_vector_get_element_at(arguments, 2);
        octaspire_helpers_verify(thirdArg);

        if (octaspire_dern_value_set_collection(firstArg, secondArg, thirdArg))
        {
            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            //return octaspire_dern_vm_get_value_true(vm);
            return firstArg;
        }

        octaspire_dern_vm_pop_value(vm, arguments);
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_get_value_false(vm);
    }
}


octaspire_dern_value_t *octaspire_dern_vm_builtin_equals_equals(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs < 2)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
                    octaspire_dern_value_as_vector_get_element_at(arguments, i),
                    environment)))
        {
            octaspire_dern_vm_pop_value(vm, firstValue);
            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_get_value_false(vm);
        }
    }

    octaspire_dern_vm_pop_value(vm, firstValue);
    octaspire_dern_vm_pop_value(vm, arguments);

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs < 2)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
                    octaspire_dern_value_as_vector_get_element_at(arguments, i),
                    environment)))
        {
            octaspire_dern_vm_pop_value(vm, firstValue);
            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_get_value_false(vm);
        }
    }

    octaspire_dern_vm_pop_value(vm, firstValue);
    octaspire_dern_vm_pop_value(vm, arguments);

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_get_value_true(vm);
}

octaspire_dern_value_t *octaspire_dern_vm_special_less_than(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs < 2)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
                    octaspire_dern_value_as_vector_get_element_at(arguments, i),
                    environment)))
        {
            octaspire_dern_vm_pop_value(vm, firstValue);
            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_get_value_false(vm);
        }
    }

    octaspire_dern_vm_pop_value(vm, firstValue);
    octaspire_dern_vm_pop_value(vm, arguments);

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_get_value_true(vm);
}

octaspire_dern_value_t *octaspire_dern_vm_special_greater_than(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs < 2)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
                    octaspire_dern_value_as_vector_get_element_at(arguments, i),
                    environment)))
        {
            octaspire_dern_vm_pop_value(vm, firstValue);
            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_get_value_false(vm);
        }
    }

    octaspire_dern_vm_pop_value(vm, firstValue);
    octaspire_dern_vm_pop_value(vm, arguments);

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_get_value_true(vm);
}

octaspire_dern_value_t *octaspire_dern_vm_special_greater_than_or_equal(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs < 2)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
                    octaspire_dern_value_as_vector_get_element_at(arguments, i),
                    environment)))
        {
            octaspire_dern_vm_pop_value(vm, firstValue);
            octaspire_dern_vm_pop_value(vm, arguments);
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_get_value_false(vm);
        }
    }

    octaspire_dern_vm_pop_value(vm, firstValue);
    octaspire_dern_vm_pop_value(vm, arguments);

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_get_value_true(vm);
}

octaspire_dern_value_t *octaspire_dern_stdlib_private_validate_function(
    octaspire_dern_vm_t* vm,
    octaspire_dern_function_t *function)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(function);
    octaspire_helpers_verify(function->formals);
    octaspire_helpers_verify(function->formals->typeTag == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(function->body);
    octaspire_helpers_verify(function->body->typeTag == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(function->definitionEnvironment);
    octaspire_helpers_verify(function->definitionEnvironment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);
    octaspire_helpers_verify(function->allocator);

    size_t const numFormalArgs = octaspire_container_vector_get_length(
        function->formals->value.vector);

    size_t numDotArgs            = 0;
    size_t numNormalArgs         = 0;
    size_t numNormalArgsAfterDot = 0;

    for (size_t i = 0; i < numFormalArgs; ++i)
    {
        octaspire_dern_value_t *formal = octaspire_container_vector_get_element_at(
            function->formals->value.vector,
            i);

        octaspire_helpers_verify(formal->typeTag == OCTASPIRE_DERN_VALUE_TAG_SYMBOL);

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
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Function can have only one formal . for varargs. Now %zu dots were given.",
            numDotArgs);
    }

    if (numNormalArgsAfterDot > 1)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Function can have only one formal argument after . "
            "for varargs. Now %zu formals were given after dot.",
            numNormalArgsAfterDot);
    }

    if (numDotArgs == 0 && numNormalArgs != numFormalArgs)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Number of formal and actual arguments must be equal for functions without "
            "varargs using the dot-formal. Now %zu formal and %zu actual arguments were given.",
            numFormalArgs,
            numNormalArgs);
    }

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return 0;
}

octaspire_dern_value_t *octaspire_dern_vm_special_fn(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_dern_vm_push_value(vm, arguments);
    octaspire_dern_vm_push_value(vm, environment);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    if (octaspire_container_vector_get_length(vec) < 2)
    {
        octaspire_dern_vm_pop_value(vm, environment);
        octaspire_dern_vm_pop_value(vm, arguments);

        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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

        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "First argument to special 'fn' must be vector (formals). Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(formals->typeTag));
    }

    octaspire_dern_value_t *body = octaspire_dern_vm_create_new_value_vector(vm);

    octaspire_dern_vm_push_value(vm, body);

    for (size_t i = 1; i < octaspire_container_vector_get_length(vec); ++i)
    {
        octaspire_dern_value_t *tmpPtr = octaspire_container_vector_get_element_at(vec, i);

        if (tmpPtr->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
        {
            octaspire_dern_vm_pop_value(vm, body);
            octaspire_dern_vm_pop_value(vm, environment);
            octaspire_dern_vm_pop_value(vm, arguments);

            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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

        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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

        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return error;
    }

    // Real docstring is set by define
    octaspire_dern_value_t * result = octaspire_dern_vm_create_new_value_function(vm, function, "", 0);

    octaspire_dern_vm_pop_value(vm, body);
    octaspire_dern_vm_pop_value(vm, environment);
    octaspire_dern_vm_pop_value(vm, arguments);

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_uid(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_dern_vm_push_value(vm, arguments);
    octaspire_dern_vm_push_value(vm, environment);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    if (octaspire_container_vector_get_length(vec) != 1)
    {
        octaspire_dern_vm_pop_value(vm, environment);
        octaspire_dern_vm_pop_value(vm, arguments);

        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_abort(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'abort' expects one argument.");
    }

    octaspire_dern_vm_push_value(vm, arguments);

    octaspire_dern_value_t *firstArg = octaspire_dern_value_as_vector_get_element_at(arguments, 0);
    octaspire_helpers_verify(firstArg);

    octaspire_dern_value_print(firstArg, octaspire_dern_vm_get_allocator(vm));

    octaspire_dern_vm_pop_value(vm, arguments);
    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    abort();
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_not(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'not' expects one argument.");
    }

    octaspire_dern_vm_push_value(vm, arguments);

    octaspire_dern_value_t *firstArg = octaspire_dern_value_as_vector_get_element_at(arguments, 0);
    octaspire_helpers_verify(firstArg);

    if (firstArg->typeTag != OCTASPIRE_DERN_VALUE_TAG_BOOLEAN)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'not' expects boolean argument.");
    }

    bool const given = firstArg->value.boolean;

    octaspire_dern_vm_pop_value(vm, arguments);
    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, !given);
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_return(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'return' expects one argument.");
    }

    octaspire_dern_vm_push_value(vm, arguments);

    octaspire_dern_value_t *firstArg = octaspire_dern_value_as_vector_get_element_at(arguments, 0);
    octaspire_helpers_verify(firstArg);

    octaspire_dern_vm_pop_value(vm, arguments);
    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    octaspire_helpers_verify(octaspire_dern_vm_get_function_return(vm) == 0);

    octaspire_dern_vm_set_function_return(vm, firstArg);

    return firstArg;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_vector(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_dern_value_t *result = octaspire_dern_vm_create_new_value_vector(vm);

    for (size_t i = 0; i < octaspire_dern_value_as_vector_get_length(arguments); ++i)
    {
        octaspire_dern_value_t * const arg =
            octaspire_dern_value_as_vector_get_element_at(
                arguments,
                i);

        octaspire_dern_value_as_vector_push_back_element(result, &arg);
    }

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_special_alias(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    if (octaspire_dern_value_as_vector_get_length(arguments) != 2)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Special 'alias' expects two arguments.");
    }

    octaspire_dern_value_t * const firstArg =
        octaspire_dern_value_as_vector_get_element_at(arguments, 0);

    octaspire_dern_value_t * const secondArg =
        octaspire_dern_value_as_vector_get_element_at(arguments, 1);

    octaspire_dern_vm_push_value(vm, environment); // TODO is this necessary?

    octaspire_dern_value_t * const secondArgEvaluated = octaspire_dern_vm_eval(vm, secondArg, environment);

    octaspire_dern_vm_push_value(vm, secondArgEvaluated);

    octaspire_helpers_verify(octaspire_dern_environment_set(
        environment->value.environment,
        firstArg,
        secondArgEvaluated));

    octaspire_dern_vm_pop_value(vm, secondArgEvaluated);
    octaspire_dern_vm_pop_value(vm, environment);
    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return firstArg;
}

octaspire_dern_value_t *octaspire_dern_vm_special_and(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    if (octaspire_dern_value_as_vector_get_length(arguments) == 0)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_boolean(vm, true);
    }

    octaspire_dern_value_t *result = 0;

    for (size_t i = 0; i < octaspire_dern_value_as_vector_get_length(arguments); ++i)
    {
        octaspire_dern_value_t * const arg =
            octaspire_dern_value_as_vector_get_element_at(
                arguments,
                i);

        octaspire_helpers_verify(arg);

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

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_special_do(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    if (octaspire_dern_value_as_vector_get_length(arguments) == 0)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
                i);

        octaspire_helpers_verify(arg);

        result = octaspire_dern_vm_eval(vm, arg, environment);

        if (octaspire_dern_vm_get_function_return(vm))
        {
            result = octaspire_dern_vm_get_function_return(vm);
            //octaspire_dern_vm_set_function_return(vm, 0);
            break;
        }
    }

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_nth(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    if (octaspire_dern_value_as_vector_get_length(arguments) != 2)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'nth' expects two arguments.");
    }

    octaspire_dern_value_t const * const firstArg =
        octaspire_dern_value_as_vector_get_element_at_const(
            arguments,
            0);

    octaspire_helpers_verify(firstArg);

    if (firstArg->typeTag != OCTASPIRE_DERN_VALUE_TAG_INTEGER)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "First argument to builtin 'nth' must be integer. Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    size_t const index = firstArg->value.integer;

    octaspire_dern_value_t * const secondArg =
        octaspire_dern_value_as_vector_get_element_at(arguments, 1);

    octaspire_helpers_verify(secondArg);

    if (secondArg->typeTag == OCTASPIRE_DERN_VALUE_TAG_STRING)
    {
        if (index >= octaspire_container_utf8_string_get_length_in_ucs_characters(secondArg->value.string))
        {
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'nth' cannot index string of length %zu from index %zu.",
                octaspire_container_utf8_string_get_length_in_ucs_characters(secondArg->value.string),
                index);
        }

        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_character_from_uint32t(
            vm, octaspire_container_utf8_string_get_ucs_character_at_index(secondArg->value.string, index));
    }
    else if (secondArg->typeTag == OCTASPIRE_DERN_VALUE_TAG_VECTOR)
    {
        if (index >= octaspire_container_vector_get_length(secondArg->value.vector))
        {
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'nth' cannot index vector of length %zu from index %zu.",
                octaspire_container_vector_get_length(secondArg->value.vector),
                index);
        }

        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_container_vector_get_element_at(secondArg->value.vector, index);
    }
    else if (secondArg->typeTag == OCTASPIRE_DERN_VALUE_TAG_HASH_MAP)
    {
        if (index >= octaspire_container_hash_map_get_number_of_elements(secondArg->value.hashMap))
        {
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'nth' cannot index hash map of length %zu from index %zu.",
                octaspire_container_hash_map_get_number_of_elements(secondArg->value.hashMap),
                index);
        }

        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));

        octaspire_container_hash_map_element_t *element =
            octaspire_container_hash_map_get_at_index(secondArg->value.hashMap, index);

        octaspire_helpers_verify(element);

        return octaspire_container_hash_map_element_get_value(element);
    }
    else
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Second argument to builtin 'nth' must be one of the collection types. Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_exit(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    if (octaspire_container_vector_get_length(vec) > 1)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'exit' expects zero or one argument.");
    }

    if (octaspire_container_vector_get_length(vec) == 1)
    {
        octaspire_dern_value_t *value = octaspire_container_vector_get_element_at(vec, 0);

        if (value->typeTag != OCTASPIRE_DERN_VALUE_TAG_INTEGER)
        {
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'exit' expects integer argument. Type %s was given.",
                octaspire_dern_value_helper_get_type_as_c_string(value->typeTag));
        }

        octaspire_dern_vm_set_exit_code(vm, value->value.integer);
    }

    octaspire_dern_vm_quit(vm);

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_get_value_true(vm);
}


octaspire_dern_value_t *octaspire_dern_vm_builtin_doc(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    if (octaspire_container_vector_get_length(vec) < 1)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'doc' expects at least one argument.");
    }

    if (octaspire_container_vector_get_length(vec) == 1)
    {
        octaspire_dern_value_t *value = octaspire_container_vector_get_element_at(vec, 0);

        if (value->docstr)
        {
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return value->docstr;
        }

        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
            octaspire_dern_value_t *value = octaspire_container_vector_get_element_at(vec, i);

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

        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return resultVal;
    }

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_mutable(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    if (octaspire_container_vector_get_length(vec) < 1)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'mutable' expects at least one argument.");
    }

    if (octaspire_container_vector_get_length(vec) == 1)
    {
        octaspire_dern_value_t *value = octaspire_container_vector_get_element_at(vec, 0);

        value->mutableCounter = -1;

        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_get_value_true(vm);
    }
    else if (octaspire_container_vector_get_length(vec) == 2)
    {
        octaspire_dern_value_t *value = octaspire_container_vector_get_element_at(vec, 0);
        octaspire_dern_value_t *count = octaspire_container_vector_get_element_at(vec, 1);

        if (count->typeTag != OCTASPIRE_DERN_VALUE_TAG_INTEGER)
        {
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Second argument to builtin 'mutable' must be integer (mutable count). Type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(count->typeTag));
        }

        value->mutableCounter = count->value.integer;

        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_get_value_true(vm);
    }
    else
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'mutable' expects zero or one arguments.");
    }
}


octaspire_dern_value_t *octaspire_dern_vm_builtin_len(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    if (octaspire_container_vector_get_length(vec) < 1)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'len' expects at least one argument.");
    }

    if (octaspire_container_vector_get_length(vec) == 1)
    {
        octaspire_dern_value_t *value = octaspire_container_vector_get_element_at(vec, 0);

        // TODO XXX check number ranges for too large size_t value for int32_t?
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_integer(vm, (int32_t)octaspire_dern_value_get_length(value));
    }
    else
    {
        octaspire_dern_value_t *resultVal = octaspire_dern_vm_create_new_value_vector(vm);

        octaspire_dern_vm_push_value(vm, resultVal);

        for (size_t i = 0; i < octaspire_container_vector_get_length(vec); ++i)
        {
            octaspire_dern_value_t *value = octaspire_container_vector_get_element_at(vec, i);

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

        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return resultVal;
    }

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_read_and_eval_path(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    if (octaspire_container_vector_get_length(vec) != 1)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'read-and-eval-path' expects one argument. %zu arguments were given.",
            octaspire_container_vector_get_length(vec));
    }

    octaspire_dern_value_t *path = octaspire_container_vector_get_element_at(vec, 0);

    if (path->typeTag != OCTASPIRE_DERN_VALUE_TAG_STRING)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "First argument to builtin 'read-and-eval-path' must be string (path). Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(path->typeTag));
    }

    octaspire_dern_value_t *result = octaspire_dern_vm_read_from_path_and_eval_in_global_environment(
        vm,
        octaspire_container_utf8_string_get_c_string(path->value.string));

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_read_and_eval_string(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    if (octaspire_container_vector_get_length(vec) != 1)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'read-and-eval-string' expects one argument. %zu arguments were given.",
            octaspire_container_vector_get_length(vec));
    }

    octaspire_dern_value_t *stringToEval = octaspire_container_vector_get_element_at(vec, 0);

    if (stringToEval->typeTag != OCTASPIRE_DERN_VALUE_TAG_STRING)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "First argument to builtin 'read-and-eval-string' must be string (to be evaluated). Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(stringToEval->typeTag));
    }

    octaspire_dern_value_t *result = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        octaspire_container_utf8_string_get_c_string(stringToEval->value.string));

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_string_format(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs == 0)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'string-format' expects one or more arguments.");
    }

    octaspire_dern_value_t const * const fmtStr =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify(fmtStr);

    octaspire_container_utf8_string_t *resultStr = 0;

    if (numArgs == 1)
    {
        octaspire_container_utf8_string_t *str =
            octaspire_dern_value_to_string_plain(fmtStr, octaspire_dern_vm_get_allocator(vm));

        octaspire_dern_value_t *result = octaspire_dern_vm_create_new_value_string(vm, str);

        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return result;
    }
    else
    {
        if (fmtStr->typeTag != OCTASPIRE_DERN_VALUE_TAG_STRING)
        {
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
            curChar = octaspire_container_utf8_string_get_ucs_character_at_index(fmtStr->value.string, c);

            if ((c + 1) < fmtStrLen)
            {
                nextChar = octaspire_container_utf8_string_get_ucs_character_at_index(fmtStr->value.string, c + 1);
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
                        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
                        return octaspire_dern_vm_create_new_value_error_from_c_string(
                            vm,
                            "Not enough arguments for the format string of 'string-format'.");
                    }

                    octaspire_container_utf8_string_t *tmpStr = octaspire_dern_value_to_string_plain(
                        octaspire_dern_value_as_vector_get_element_at(
                            arguments,
                            fmtValueIndex),
                        octaspire_dern_vm_get_allocator(vm));

                    octaspire_container_utf8_string_concatenate(resultStr, tmpStr);

                    octaspire_container_utf8_string_release(tmpStr);
                    tmpStr = 0;

                    ++c;
                    ++fmtValueIndex;
                }
                else
                {
                    octaspire_container_utf8_string_push_back_ucs_character(resultStr, (char)curChar);
                }
            }
            else
            {
                octaspire_container_utf8_string_push_back_ucs_character(resultStr, (char)curChar);
            }

            prevChar = curChar;
        }
    }

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_string(vm, resultStr);
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_to_string(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs == 0)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'to-string' expects one or more arguments.");
    }
    else if (numArgs == 1)
    {
        octaspire_container_utf8_string_t * str =
            octaspire_dern_value_to_string(octaspire_dern_value_as_vector_get_element_at_const(
                arguments,
                0), octaspire_dern_vm_get_allocator(vm));

        return octaspire_dern_vm_create_new_value_string(vm, str);
    }
    else
    {
        // TODO XXX
        abort();
    }
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_to_integer(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs == 0)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
        if (value->typeTag != OCTASPIRE_DERN_VALUE_TAG_REAL)
        {
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_from_c_string(
                vm,
                "First argument to 'to-integer' is not real number.");
        }

        return octaspire_dern_vm_create_new_value_integer(vm, (int32_t)value->value.real);
    }
    else
    {
        // TODO XXX
        abort();
    }
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_print(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs == 0)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'println' one or more arguments.");
    }

    octaspire_dern_value_t const * const fmtStr =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify(fmtStr);

    if (fmtStr->typeTag != OCTASPIRE_DERN_VALUE_TAG_STRING)
    {
        octaspire_dern_value_print(fmtStr, octaspire_dern_vm_get_allocator(vm));

        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
            curChar = octaspire_container_utf8_string_get_ucs_character_at_index(fmtStr->value.string, c);

            if ((c + 1) < fmtStrLen)
            {
                nextChar = octaspire_container_utf8_string_get_ucs_character_at_index(fmtStr->value.string, c + 1);
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
                        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
                        return octaspire_dern_vm_create_new_value_error_from_c_string(
                            vm,
                            "Not enough arguments for the format string of 'println'.");
                    }

                    octaspire_container_utf8_string_t *tmpStr = octaspire_dern_value_to_string(
                        octaspire_dern_value_as_vector_get_element_at(
                            arguments,
                            fmtValueIndex),
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

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_get_value_true(vm);
}


octaspire_dern_value_t *octaspire_dern_vm_builtin_println(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    octaspire_dern_value_t *result = octaspire_dern_vm_builtin_print(vm, arguments, environment);
    printf("\n");
    return result;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_env_new(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    if (octaspire_container_vector_get_length(vec) == 0)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_environment(vm, 0);
    }
    else if (octaspire_container_vector_get_length(vec) == 1)
    {
        octaspire_dern_value_t *value = octaspire_container_vector_get_element_at(vec, 0);
        octaspire_helpers_verify(value);

        if (value->typeTag != OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT)
        {
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Argument to builtin 'env-new' must be an environment. Now argument has type '%s'.",
                octaspire_dern_value_helper_get_type_as_c_string(value->typeTag));
        }

        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_environment(vm, value);
    }
    else
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    if (octaspire_container_vector_get_length(vec) != 0)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    if (octaspire_container_vector_get_length(vec) != 0)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    if (octaspire_container_vector_get_length(vec) < 1)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin '-=' expects at least one argument.");
    }

    octaspire_dern_value_t * const firstArg = octaspire_container_vector_get_element_at(vec, 0);

    switch (firstArg->typeTag)
    {
        case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
        {
            for (size_t i = 1; i < octaspire_container_vector_get_length(vec); ++i)
            {
                octaspire_dern_value_t * const anotherArg =
                    octaspire_container_vector_get_element_at(vec, i);

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
                    octaspire_container_vector_get_element_at(vec, i);

                octaspire_dern_value_as_real_subtract(firstArg, anotherArg);
            }
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
        {
            for (size_t i = 1; i < octaspire_container_vector_get_length(vec); ++i)
            {
                octaspire_dern_value_t * const anotherArg =
                    octaspire_container_vector_get_element_at(vec, i);

                octaspire_dern_value_as_integer_subtract(firstArg, anotherArg);
            }
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
        {
            for (size_t i = 1; i < octaspire_container_vector_get_length(vec); ++i)
            {
                octaspire_dern_value_t * const anotherArg =
                    octaspire_container_vector_get_element_at(vec, i);

                // TODO XXX remove the need for '&' in &another for vectors!!!!!
                for (size_t j = 0; j < octaspire_dern_value_as_vector_get_length(firstArg); /*NOP*/)
                {
                    octaspire_dern_value_t * const val =
                        octaspire_dern_value_as_vector_get_element_at(firstArg, j);

                    if (octaspire_dern_value_is_equal(anotherArg, val))
                    {
                        octaspire_dern_value_as_vector_remove_element_at(firstArg, j);
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
                    octaspire_container_vector_get_element_at(vec, i);

                if (!octaspire_dern_value_as_string_remove_all_substrings(firstArg, anotherArg))
                {
                    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
                    octaspire_container_vector_get_element_at(vec, i);

                if (!octaspire_dern_value_as_hash_map_remove(firstArg, anotherArg))
                {
                    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
                    return octaspire_dern_vm_create_new_value_error_from_c_string(
                        vm,
                        "Builtin '-=' failed");
                }
            }
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_ERROR:
        {
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return firstArg;
        }

        default:
        {
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "First argument to builtin '-=' cannot be of type '%s'.",
                octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
        }
    }

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return firstArg;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_plus_equals(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    if (octaspire_container_vector_get_length(vec) < 1)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin '+=' expects at least one argument.");
    }

    octaspire_dern_value_t * const firstArg = octaspire_container_vector_get_element_at(vec, 0);

    switch (firstArg->typeTag)
    {
        case OCTASPIRE_DERN_VALUE_TAG_CHARACTER:
        {
            for (size_t i = 1; i < octaspire_container_vector_get_length(vec); ++i)
            {
                octaspire_dern_value_t * const anotherArg =
                    octaspire_container_vector_get_element_at(vec, i);

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
                    octaspire_container_vector_get_element_at(vec, i);

                octaspire_dern_value_as_real_add(firstArg, anotherArg);
            }
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_INTEGER:
        {
            for (size_t i = 1; i < octaspire_container_vector_get_length(vec); ++i)
            {
                octaspire_dern_value_t * const anotherArg =
                    octaspire_container_vector_get_element_at(vec, i);

                octaspire_dern_value_as_integer_add(firstArg, anotherArg);
            }
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_VECTOR:
        {
            for (size_t i = 1; i < octaspire_container_vector_get_length(vec); ++i)
            {
                octaspire_dern_value_t * const anotherArg =
                    octaspire_container_vector_get_element_at(vec, i);

                // TODO XXX remove the need for '&' in &another for vectors!!!!!
                octaspire_dern_value_as_vector_push_back_element(firstArg, &anotherArg);
            }
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_STRING:
        {
            //bool success = true;
            for (size_t i = 1; i < octaspire_container_vector_get_length(vec); ++i)
            {
                octaspire_dern_value_t * const anotherArg =
                    octaspire_container_vector_get_element_at(vec, i);

                if (!octaspire_dern_value_as_string_push_back(firstArg, anotherArg))
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
                    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
                    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
                    return octaspire_dern_vm_create_new_value_error_from_c_string(
                        vm,
                        "Builtin '+=' failed");
                }
            }
            else
            {
                octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_from_c_string(
                    vm,
                    "Builtin '+=' expects one or two additional arguments for hash map");
            }
        }
        break;

        case OCTASPIRE_DERN_VALUE_TAG_ERROR:
        {
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return firstArg;
        }

        default:
        {
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "First argument to builtin '+=' cannot be of type '%s'.",
                octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
        }
    }

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return firstArg;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_plus_plus(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    if (octaspire_container_vector_get_length(vec) < 1)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin '++' expects at least one argument.");
    }

    octaspire_dern_value_t *value = 0;

    for (size_t i = 0; i < octaspire_container_vector_get_length(vec); ++i)
    {
        value = octaspire_container_vector_get_element_at(vec, i);

        if (!octaspire_dern_value_is_number(value))
        {
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return value;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_minus_minus(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_container_vector_t * const vec = arguments->value.vector;

    if (octaspire_container_vector_get_length(vec) < 1)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin '--' expects at least one argument.");
    }

    octaspire_dern_value_t *value = 0;

    for (size_t i = 0; i < octaspire_container_vector_get_length(vec); ++i)
    {
        value = octaspire_container_vector_get_element_at(vec, i);

        if (!octaspire_dern_value_is_number(value))
        {
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Arguments to builtin '--' must be numbers. %zuth argument has type '%s'.",
                i + 1,
                octaspire_dern_value_helper_get_type_as_c_string(value->typeTag));
        }

        if (value->mutableCounter > 0)
        {
            --(value->mutableCounter);
        }
        else if (value->mutableCounter == 0)
        {
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin '--' tried to modify a constant value at %zuth argument.",
                i + 1);
        }

        if (value->typeTag == OCTASPIRE_DERN_VALUE_TAG_INTEGER)
        {
            --(value->value.integer);
        }
        else
        {
            --(value->value.real);
        }
    }

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return value;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_pop_front(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    if (octaspire_dern_value_as_vector_get_length(arguments) != 1)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'pop_front' expects exactly one argument.");
    }

    octaspire_dern_value_t *value = octaspire_dern_value_as_vector_get_element_at(arguments, 0);

    if (value->typeTag != OCTASPIRE_DERN_VALUE_TAG_VECTOR)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'pop_front' expects vector argument.");
    }

    octaspire_container_vector_t *vec = value->value.vector;

    octaspire_helpers_verify(vec);

    bool const result = octaspire_container_vector_pop_front_element(vec);

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, result);
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_times(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    bool allArgsAreIntegers = true;
    double realResult = 1;
    int32_t integerResult = 1;

    for (size_t i = 0; i < numArgs; ++i)
    {
        octaspire_dern_value_t *currentArg =
            octaspire_dern_value_as_vector_get_element_at(arguments, i);

        octaspire_helpers_verify(currentArg);

        switch (currentArg->typeTag)
        {
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

            default:
            {
                if (currentArg->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                {
                    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
                    return currentArg;
                }

                octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin '*' expects numeric arguments (integer or real). %zuth argument has type %s.",
                    i + 1,
                    octaspire_dern_value_helper_get_type_as_c_string(currentArg->typeTag));
            }
            break;
        }
    }

    if (allArgsAreIntegers)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_integer(vm, integerResult);
    }

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_real(vm, realResult);
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_private_plus_numerical(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    bool allArgsAreIntegers = true;
    double realResult = 0;
    int32_t integerResult = 0;

    for (size_t i = 0; i < numArgs; ++i)
    {
        octaspire_dern_value_t *currentArg =
            octaspire_dern_value_as_vector_get_element_at(arguments, i);

        octaspire_helpers_verify(currentArg);

        switch (currentArg->typeTag)
        {
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

            default:
            {
                if (currentArg->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                {
                    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
                    return currentArg;
                }

                octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin '+' expects numeric arguments (integer or real). %zuth argument has type %s.",
                    i + 1,
                    octaspire_dern_value_helper_get_type_as_c_string(currentArg->typeTag));
            }
            break;
        }
    }

    if (allArgsAreIntegers)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_integer(vm, integerResult);
    }

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_real(vm, realResult);
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_private_plus_textual(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    octaspire_dern_value_t *firstArg =
        octaspire_dern_value_as_vector_get_element_at(arguments, 0);

    octaspire_helpers_verify(firstArg);
    octaspire_helpers_verify(firstArg->typeTag == OCTASPIRE_DERN_VALUE_TAG_STRING);

    for (size_t i = 1; i < numArgs; ++i)
    {
        octaspire_dern_value_t *currentArg =
            octaspire_dern_value_as_vector_get_element_at(arguments, i);

        octaspire_helpers_verify(currentArg);

        switch (currentArg->typeTag)
        {
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

            default:
            {
                octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin '+' expects textual arguments if the first argument is textual. %zuth argument has type %s.",
                    i + 1,
                    octaspire_dern_value_helper_get_type_as_c_string(currentArg->typeTag));
            }
            break;
        }
    }

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    // TODO what to return? Count of removals or the modified string-value?
    return firstArg;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_private_minus_numerical(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    bool allArgsAreIntegers = true;
    double realResult = 0;
    int32_t integerResult = 0;

    for (size_t i = 0; i < numArgs; ++i)
    {
        octaspire_dern_value_t *currentArg =
            octaspire_dern_value_as_vector_get_element_at(arguments, i);

        octaspire_helpers_verify(currentArg);

        switch (currentArg->typeTag)
        {
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
                    integerResult = -currentArg->value.real;
                    realResult    = -currentArg->value.real;
                }
                else
                {
                    if (i == 0)
                    {
                        integerResult = currentArg->value.real;
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

            default:
            {
                if (currentArg->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                {
                    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
                    return currentArg;
                }

                octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin '-' expects numeric arguments (integer or real). %zuth argument has type %s.",
                    i + 1,
                    octaspire_dern_value_helper_get_type_as_c_string(currentArg->typeTag));
            }
            break;
        }
    }

    if (allArgsAreIntegers)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_integer(vm, integerResult);
    }

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_real(vm, realResult);
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_private_minus_textual(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    octaspire_dern_value_t *firstArg =
        octaspire_dern_value_as_vector_get_element_at(arguments, 0);

    octaspire_helpers_verify(firstArg);
    octaspire_helpers_verify(firstArg->typeTag == OCTASPIRE_DERN_VALUE_TAG_STRING);

    for (size_t i = 1; i < numArgs; ++i)
    {
        octaspire_dern_value_t *currentArg =
            octaspire_dern_value_as_vector_get_element_at(arguments, i);

        octaspire_helpers_verify(currentArg);

        switch (currentArg->typeTag)
        {
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

            default:
            {
                octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin '-' expects textual arguments if the first argument is textual. %zuth argument has type %s.",
                    i + 1,
                    octaspire_dern_value_helper_get_type_as_c_string(currentArg->typeTag));
            }
            break;
        }
    }

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    // TODO what to return? Count of removals or the modified string-value?
    return firstArg;
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_plus(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs == 0)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_builtin_private_plus_numerical(vm, arguments, environment);
    }

    octaspire_dern_value_t *firstArg =
        octaspire_dern_value_as_vector_get_element_at(arguments, 0);

    octaspire_helpers_verify(firstArg);

    switch (firstArg->typeTag)
    {
        case OCTASPIRE_DERN_VALUE_TAG_STRING:
        {
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_builtin_private_plus_textual(vm, arguments, environment);
        }
        break;

        default:
        {
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_builtin_private_plus_numerical(vm, arguments, environment);
        }
        break;
    }
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_minus(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs == 0)
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_builtin_private_minus_numerical(vm, arguments, environment);
    }

    octaspire_dern_value_t *firstArg =
        octaspire_dern_value_as_vector_get_element_at(arguments, 0);

    octaspire_helpers_verify(firstArg);

    switch (firstArg->typeTag)
    {
        case OCTASPIRE_DERN_VALUE_TAG_STRING:
        {
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_builtin_private_minus_textual(vm, arguments, environment);
        }
        break;

        default:
        {
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_builtin_private_minus_numerical(vm, arguments, environment);
        }
        break;
    }
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_find(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    octaspire_dern_vm_push_value(vm, arguments);
    octaspire_dern_vm_push_value(vm, environment);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    if (numArgs < 2)
    {
        octaspire_dern_vm_pop_value(vm, environment);
        octaspire_dern_vm_pop_value(vm, arguments);
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'find' expects at least two arguments. %zu arguments was given.",
            numArgs);
    }

    octaspire_dern_value_t *container = octaspire_dern_value_as_vector_get_element_at(arguments, 0);

    octaspire_helpers_verify(container);

    octaspire_dern_value_t *result = 0;

    if (numArgs > 2)
    {
        result = octaspire_dern_vm_create_new_value_vector(vm);
        octaspire_dern_vm_push_value(vm, result);

        for (size_t i = 1; i < numArgs; ++i)
        {
            octaspire_dern_value_t *keyArg =
                octaspire_dern_value_as_vector_get_element_at(arguments, i);

            octaspire_helpers_verify(keyArg);

            octaspire_dern_value_t *tmpVal = octaspire_dern_vm_find_from_value(vm, container, keyArg);
            octaspire_dern_value_as_vector_push_back_element(
                result,
                &tmpVal);
        }

        octaspire_dern_vm_pop_value(vm, result);
        octaspire_dern_vm_pop_value(vm, environment);
        octaspire_dern_vm_pop_value(vm, arguments);
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return result;
    }
    else
    {
        octaspire_dern_value_t *keyArg =
            octaspire_dern_value_as_vector_get_element_at(arguments, 1);

        octaspire_helpers_verify(keyArg);

        result = octaspire_dern_vm_find_from_value(vm, container, keyArg);

        octaspire_dern_vm_pop_value(vm, environment);
        octaspire_dern_vm_pop_value(vm, arguments);
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return result;
    }
}

octaspire_dern_value_t *octaspire_dern_vm_builtin_hash_map(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    octaspire_helpers_verify(arguments->typeTag   == OCTASPIRE_DERN_VALUE_TAG_VECTOR);
    octaspire_helpers_verify(environment->typeTag == OCTASPIRE_DERN_VALUE_TAG_ENVIRONMENT);

    size_t const numArgs = octaspire_dern_value_get_length(arguments);

    octaspire_dern_value_t *result = octaspire_dern_vm_create_new_value_hash_map(vm);
    octaspire_dern_vm_push_value(vm, result);

    for (size_t i = 0; i < numArgs; i += 2)
    {
        octaspire_dern_value_t *keyArg =
            octaspire_dern_value_as_vector_get_element_at(arguments, i);

        if (!keyArg)
        {
            octaspire_dern_vm_pop_value(vm, result);
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_from_c_string(
                vm,
                "Builtin 'hash-map' expects key here.");
        }

        octaspire_dern_value_t *valArg =
            octaspire_dern_value_as_vector_get_element_at(arguments, i + 1);

        if (!valArg)
        {
            octaspire_dern_vm_pop_value(vm, result);
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return result;
}

