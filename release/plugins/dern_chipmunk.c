/******************************************************************************
Octaspire Dern - Programming language
Copyright 2017, 2018 www.octaspire.com

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
#include "octaspire-dern-amalgamated.c"
#include "chipmunk.h"

static char const * const DERN_CHIPMUNK_PLUGIN_NAME = "dern_chipmunk";

typedef struct dern_chipmunk_allocation_context_t
{
    octaspire_dern_vm_t       *vm;
    void                      *payload;
}
dern_chipmunk_allocation_context_t;

void dern_chipmunk_cpSpace_clean_up_callback(void *payload)
{
    octaspire_helpers_verify_not_null(payload);
    cpSpaceFree((cpSpace*)payload);
}

void dern_chipmunk_cpVect_clean_up_callback(void *payload)
{
    octaspire_helpers_verify_not_null(payload);
    dern_chipmunk_allocation_context_t * const context = payload;
    octaspire_helpers_verify_not_null(context->vm);
    octaspire_helpers_verify_not_null(context->payload);

    octaspire_allocator_t * const allocator =
        octaspire_dern_vm_get_allocator(context->vm);

    octaspire_allocator_free(allocator, (cpVect*)(context->payload));
    octaspire_allocator_free(allocator, context);
}

octaspire_dern_value_t *dern_chipmunk_cpSpaceNew(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_string_format(
            vm,
            "Builtin 'chipmunk-cpSpaceNew' expects no arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    cpSpace *space = cpSpaceNew();

    if (!space)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

        octaspire_dern_value_t * const result =
            octaspire_dern_vm_create_new_value_string_from_c_string(
                vm,
                "Builtin 'chipmunk-cpSpace' failed: cannot create "
                "new space");

        return result;
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    octaspire_dern_value_t * const result =
        octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_CHIPMUNK_PLUGIN_NAME,
        "cpSpace",
        "dern_chipmunk_cpSpace_clean_up_callback",
        "",
        "",
        "",
        false,
        space);

    return result;
}

octaspire_dern_value_t *dern_chipmunk_cpv(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 2)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_string_format(
            vm,
            "Builtin 'chipmunk-cpv' expects two arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    // X
    octaspire_dern_value_t const * const firstArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(firstArg);

    if (!octaspire_dern_value_is_number(firstArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'chipmunk-cpv' expects number as the first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    // Y
    octaspire_dern_value_t const * const secondArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(secondArg);

    if (!octaspire_dern_value_is_number(secondArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'chipmunk-cpv' expects number as the second argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    cpVect *vect = octaspire_allocator_malloc(
        octaspire_dern_vm_get_allocator(vm),
        sizeof(cpVect));

    if (!vect)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'chipmunk-cpv' failed to allocate memory.");
    }

    dern_chipmunk_allocation_context_t * const context = octaspire_allocator_malloc(
        octaspire_dern_vm_get_allocator(vm),
        sizeof(dern_chipmunk_allocation_context_t));

    if (!context)
    {
        octaspire_allocator_free(octaspire_dern_vm_get_allocator(vm), vect);
        vect = 0;

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "Builtin 'chipmunk-cpv' failed to allocate memory for a cpVect context.");
    }

    context->vm      = vm;
    context->payload = vect;

    octaspire_dern_value_t * const result =
        octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_CHIPMUNK_PLUGIN_NAME,
        "cpVect",
        "dern_chipmunk_cpVect_clean_up_callback",
        "",
        "",
        "",
        false,
        context);

    return result;
}

bool dern_chipmunk_init(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_environment_t * const targetEnv)
{
    octaspire_helpers_verify_true(vm && targetEnv);

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "chipmunk-cpSpaceNew",
            dern_chipmunk_cpSpaceNew,
            0,
            "NAME\n"
            "\tchipmunk-cpSpaceNew\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_chipmunk)\n"
            "\n"
            "\t(chipmunk-cpSpace) -> cpSpace or error message\n"
            "\n"
            "DESCRIPTION\n"
            "\tCreates and returns a new Chipmunk cpSpace.\n"
            "\n"
            "ARGUMENTS\n"
            "\n"
            "RETURN VALUE\n"
            "\tcpSpace to be used with those functions of this library that\n"
            "\texpect cpSpace argument.\n"
            "\n"
            "SEE ALSO\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "chipmunk-cpv",
            dern_chipmunk_cpv,
            2,
            "NAME\n"
            "\tchipmunk-cpv\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_chipmunk)\n"
            "\n"
            "\t(chipmunk-cpv) -> cpVect or error message\n"
            "\n"
            "DESCRIPTION\n"
            "\tCreates and returns a new Chipmunk 2D vector cpVect.\n"
            "\n"
            "ARGUMENTS\n"
            "\tx           the x coordinate component\n"
            "\ty           the y coordinate component\n"
            "\n"
            "RETURN VALUE\n"
            "\tcpVect to be used with those functions of this library that\n"
            "\texpect cpVect argument.\n"
            "\n"
            "SEE ALSO\n",
            false,
            targetEnv))
    {
        return false;
    }

    return true;
}

