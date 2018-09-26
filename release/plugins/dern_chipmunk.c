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
#include "chipmunk_structs.h"

static char const * const DERN_CHIPMUNK_PLUGIN_NAME = "dern_chipmunk";
static octaspire_string_t * dern_chipmunk_private_lib_name = 0;

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

void dern_chipmunk_cpBody_clean_up_callback(void *payload)
{
    octaspire_helpers_verify_not_null(payload);
    cpBodyDestroy((cpBody*)payload);
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
        return octaspire_dern_vm_create_new_value_error_format(
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
        "dern_chipmunk_to_string",
        false,
        space);

    return result;
}

octaspire_dern_value_t *dern_chipmunk_cpBodyNew(
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
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'chipmunk-cpBodyNew' expects two arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_dern_value_t const * const firstArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(firstArg);

    if (!octaspire_dern_value_is_number(firstArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'chipmunk-cpBodyNew' expects number for the mass of "
            "the new body  as the first argument. Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    float const mass = octaspire_dern_value_as_number_get_value(firstArg);

    octaspire_dern_value_t const * const secondArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(secondArg);

    if (!octaspire_dern_value_is_number(secondArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'chipmunk-cpBodyNew' expects number for the moment of "
            "the new body  as the second argument. Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }

    float const moment = octaspire_dern_value_as_number_get_value(secondArg);

    cpBody * const body = cpBodyNew(mass, moment);

    octaspire_dern_value_t * const result =
        octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_CHIPMUNK_PLUGIN_NAME,
        "cpBody",
        "dern_chipmunk_cpBody_clean_up_callback",
        "",
        "",
        "",
        "dern_chipmunk_to_string",
        false,
        body);

    return result;
}

octaspire_dern_value_t *dern_chipmunk_cpSpaceSetGravity(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const         stackLength  = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName = "chipmunk-cpSpaceSetGravity";
    char   const * const cpSpaceName  = "cpSpace";
    char   const * const cpVectName   = "cpVect";

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 2)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects two arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numArgs);
    }

    // cpSpace

    octaspire_dern_c_data_or_unpushed_error_t cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
            arguments,
            0,
            dernFuncName,
            cpSpaceName,
            DERN_CHIPMUNK_PLUGIN_NAME);

    if (cDataOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrError.unpushedError;
    }

    cpSpace * const space = cDataOrError.cData;

    // cpVect

    cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
            arguments,
            1,
            dernFuncName,
            cpVectName,
            DERN_CHIPMUNK_PLUGIN_NAME);

    if (cDataOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrError.unpushedError;
    }

    dern_chipmunk_allocation_context_t const * const context =
        cDataOrError.cData;

    octaspire_helpers_verify_not_null(context);

    cpVect const * const vect = context->payload;

    cpSpaceSetGravity(space, *vect);

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

octaspire_dern_value_t *dern_chipmunk_cpSpaceGetGravity(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const         stackLength  = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName = "chipmunk-cpSpaceGetGravity";
    char   const * const cpSpaceName  = "cpSpace";

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects one argument. "
            "%zu arguments were given.",
            dernFuncName,
            numArgs);
    }

    // cpSpace

    octaspire_dern_c_data_or_unpushed_error_t cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
            arguments,
            0,
            dernFuncName,
            cpSpaceName,
            DERN_CHIPMUNK_PLUGIN_NAME);

    if (cDataOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrError.unpushedError;
    }

    cpSpace * const space = cDataOrError.cData;

    cpVect const gravity = cpSpaceGetGravity(space);

    cpVect *vect = octaspire_allocator_malloc(
        octaspire_dern_vm_get_allocator(vm),
        sizeof(cpVect));

    if (!vect)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' failed to allocate memory.",
            dernFuncName);
    }

    vect->x = gravity.x;
    vect->y = gravity.y;

    dern_chipmunk_allocation_context_t * const context = octaspire_allocator_malloc(
        octaspire_dern_vm_get_allocator(vm),
        sizeof(dern_chipmunk_allocation_context_t));

    if (!context)
    {
        octaspire_allocator_free(octaspire_dern_vm_get_allocator(vm), vect);
        vect = 0;

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' failed to allocate memory for a cpVect context.",
            dernFuncName);
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
        "dern_chipmunk_to_string",
        false,
        context);

    return result;
}

octaspire_dern_value_t *dern_chipmunk_cpSpaceStep(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const         stackLength  = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName = "chipmunk-cpSpaceStep";
    char   const * const cpSpaceName  = "cpSpace";

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 2)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects two arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numArgs);
    }

    // cpSpace

    octaspire_dern_c_data_or_unpushed_error_t cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
            arguments,
            0,
            dernFuncName,
            cpSpaceName,
            DERN_CHIPMUNK_PLUGIN_NAME);

    if (cDataOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrError.unpushedError;
    }

    cpSpace * const space = cDataOrError.cData;

    octaspire_dern_value_t const * const secondArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(secondArg);

    if (!octaspire_dern_value_is_number(secondArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects number for the dt "
            "as the second argument. Type '%s' was given.",
            dernFuncName,
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }

    float const dt = octaspire_dern_value_as_number_get_value(secondArg);

    cpSpaceStep(space, dt);

    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

octaspire_dern_value_t *dern_chipmunk_cpSpaceAddBody(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const         stackLength  = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName = "chipmunk-cpSpaceAddBody";
    char   const * const cpSpaceName  = "cpSpace";
    char   const * const cpBodyName   = "cpBody";

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 2)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects two arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numArgs);
    }

    // cpSpace

    octaspire_dern_c_data_or_unpushed_error_t cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
            arguments,
            0,
            dernFuncName,
            cpSpaceName,
            DERN_CHIPMUNK_PLUGIN_NAME);

    if (cDataOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrError.unpushedError;
    }

    cpSpace * const space = cDataOrError.cData;

    // cpBody

    cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
            arguments,
            1,
            dernFuncName,
            cpBodyName,
            DERN_CHIPMUNK_PLUGIN_NAME);

    if (cDataOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrError.unpushedError;
    }

    cpBody * const body = cDataOrError.cData;

    octaspire_helpers_verify_true(cpSpaceAddBody(space, body) == body);

    return cDataOrError.cData;
}

octaspire_dern_value_t *dern_chipmunk_cpSpaceRemoveBody(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const         stackLength  = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName = "chipmunk-cpSpaceRemoveBody";
    char   const * const cpSpaceName  = "cpSpace";
    char   const * const cpBodyName   = "cpBody";

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 2)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects two arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numArgs);
    }

    // cpSpace

    octaspire_dern_c_data_or_unpushed_error_t cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
            arguments,
            0,
            dernFuncName,
            cpSpaceName,
            DERN_CHIPMUNK_PLUGIN_NAME);

    if (cDataOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrError.unpushedError;
    }

    cpSpace * const space = cDataOrError.cData;

    // cpBody

    cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
            arguments,
            1,
            dernFuncName,
            cpBodyName,
            DERN_CHIPMUNK_PLUGIN_NAME);

    if (cDataOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrError.unpushedError;
    }

    cpBody * const body = cDataOrError.cData;

    cpSpaceRemoveBody(space, body);

    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

octaspire_dern_value_t *dern_chipmunk_cpSpaceContainsBody(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const         stackLength  = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName = "chipmunk-cpSpaceContainsBody";
    char   const * const cpSpaceName  = "cpSpace";
    char   const * const cpBodyName   = "cpBody";

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 2)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects two arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numArgs);
    }

    // cpSpace

    octaspire_dern_c_data_or_unpushed_error_t cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
            arguments,
            0,
            dernFuncName,
            cpSpaceName,
            DERN_CHIPMUNK_PLUGIN_NAME);

    if (cDataOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrError.unpushedError;
    }

    cpSpace * const space = cDataOrError.cData;

    // cpBody

    cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
            arguments,
            1,
            dernFuncName,
            cpBodyName,
            DERN_CHIPMUNK_PLUGIN_NAME);

    if (cDataOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrError.unpushedError;
    }

    cpBody * const body = cDataOrError.cData;

    return octaspire_dern_vm_create_new_value_boolean(
        vm,
        cpSpaceContainsBody(space, body));
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
        return octaspire_dern_vm_create_new_value_error_format(
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

    vect->x = octaspire_dern_value_as_number_get_value(firstArg);
    vect->y = octaspire_dern_value_as_number_get_value(secondArg);

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
        "dern_chipmunk_to_string",
        false,
        context);

    return result;
}

bool dern_chipmunk_init(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_environment_t * const targetEnv,
    char const * const libName)
{
    octaspire_helpers_verify_true(vm && targetEnv && libName);

    dern_chipmunk_private_lib_name = octaspire_string_new(
        libName,
        octaspire_dern_vm_get_allocator(vm));

    if (!dern_chipmunk_private_lib_name)
    {
        return false;
    }

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
            "chipmunk-cpBodyNew",
            dern_chipmunk_cpBodyNew,
            0,
            "NAME\n"
            "\tchipmunk-cpBodyNew\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_chipmunk)\n"
            "\n"
            "\t(chipmunk-cpBodyNew) -> cpBody or error message\n"
            "\n"
            "DESCRIPTION\n"
            "\tCreates and returns a new cpBody.\n"
            "\n"
            "ARGUMENTS\n"
            "\tmass     the scalar mass of the new body\n"
            "\tmoment   the scalar moment of the new body\n"
            "\n"
            "RETURN VALUE\n"
            "\tcpBody to be used with those functions of this library that\n"
            "\texpect cpBody argument.\n"
            "\n"
            "SEE ALSO\n"
            "\tchipmunk-cpSpaceNew\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "chipmunk-cpSpaceSetGravity",
            dern_chipmunk_cpSpaceSetGravity,
            2,
            "NAME\n"
            "\tchipmunk-cpSpaceSetGravity\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_chipmunk)\n"
            "\n"
            "\t(chipmunk-cpSpaceSetGravity) -> true or error message\n"
            "\n"
            "DESCRIPTION\n"
            "\tSets the gravity vector of a cpSpace.\n"
            "\n"
            "ARGUMENTS\n"
            "\tspace       the cpSpace where gravity is to be set\n"
            "\tgravity     the cpVect of the gravity to be set\n"
            "\n"
            "RETURN VALUE\n"
            "\ttrue or error message.\n"
            "\n"
            "SEE ALSO\n"
            "\tchipmunk-cpSpaceNew\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "chipmunk-cpSpaceGetGravity",
            dern_chipmunk_cpSpaceGetGravity,
            1,
            "NAME\n"
            "\tchipmunk-cpSpaceGetGravity\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_chipmunk)\n"
            "\n"
            "\t(chipmunk-cpSpaceGetGravity cpSpace) -> cpVect or error message\n"
            "\n"
            "DESCRIPTION\n"
            "\tGets the gravity vector of the given cpSpace.\n"
            "\n"
            "ARGUMENTS\n"
            "\tspace       the cpSpace where gravity is to be set\n"
            "\n"
            "RETURN VALUE\n"
            "\tthe cpVect of the gravity or error message\n"
            "\n"
            "SEE ALSO\n"
            "\tchipmunk-cpSpaceNew\n"
            "\tchipmunk-cpSpaceSetGravity\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "chipmunk-cpSpaceStep",
            dern_chipmunk_cpSpaceStep,
            1,
            "NAME\n"
            "\tchipmunk-cpSpaceStep\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_chipmunk)\n"
            "\n"
            "\t(chipmunk-cpSpaceStep cpSpace dt) -> true or error message\n"
            "\n"
            "DESCRIPTION\n"
            "\tSteps the Chipmunk space cpSpace forward in time by dt.\n"
            "\n"
            "ARGUMENTS\n"
            "\tcpSpace       the cpSpace to be step forward in time\n"
            "\tdt            the delta time\n"
            "\n"
            "RETURN VALUE\n"
            "\ttrue or error message\n"
            "\n"
            "SEE ALSO\n"
            "\tchipmunk-cpSpaceNew\n"
            "\tchipmunk-cpSpaceSetGravity\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "chipmunk-cpSpaceAddBody",
            dern_chipmunk_cpSpaceAddBody,
            1,
            "NAME\n"
            "\tchipmunk-cpSpaceAddBody\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_chipmunk)\n"
            "\n"
            "\t(chipmunk-cpSpaceAddBody cpSpace cpBody) -> cpBody or error message\n"
            "\n"
            "DESCRIPTION\n"
            "\tAdd a rigid body cpBody into the simulation.\n"
            "\n"
            "ARGUMENTS\n"
            "\tcpSpace       the cpSpace where the body is added into.\n"
            "\tcpBody        the cpBody to be added into the simulation.\n"
            "\n"
            "RETURN VALUE\n"
            "\tthe cpBody that was added into the simulation, or an error message\n"
            "\n"
            "SEE ALSO\n"
            "\tchipmunk-cpSpaceNew\n"
            "\tchipmunk-cpSpaceStep\n"
            "\tchipmunk-cpSpaceSetGravity\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "chipmunk-cpSpaceRemoveBody",
            dern_chipmunk_cpSpaceRemoveBody,
            1,
            "NAME\n"
            "\tchipmunk-cpSpaceRemoveBody\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_chipmunk)\n"
            "\n"
            "\t(chipmunk-cpSpaceRemoveBody cpSpace cpBody) -> true or error\n"
            "\n"
            "DESCRIPTION\n"
            "\tRemove a rigid body cpBody from the simulation.\n"
            "\n"
            "ARGUMENTS\n"
            "\tcpSpace       the cpSpace where the body is to be removed.\n"
            "\tcpBody        the cpBody to be removed from the simulation.\n"
            "\n"
            "RETURN VALUE\n"
            "\ttrue or an error\n"
            "\n"
            "SEE ALSO\n"
            "\tchipmunk-cpSpaceAddBody\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "chipmunk-cpSpaceContainsBody",
            dern_chipmunk_cpSpaceContainsBody,
            1,
            "NAME\n"
            "\tchipmunk-cpSpaceContainsBody\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_chipmunk)\n"
            "\n"
            "\t(chipmunk-cpSpaceContainsBody cpSpace cpBody) -> true, false or error message\n"
            "\n"
            "DESCRIPTION\n"
            "\tTest if a rigid body cpBody has been added into the chipmunk space cpSpace.\n"
            "\n"
            "ARGUMENTS\n"
            "\tcpSpace       the cpSpace where to look for the body\n"
            "\tcpBody        the cpBody to be looked for.\n"
            "\n"
            "RETURN VALUE\n"
            "\ttrue if it is added, false if not, or an error message on error\n"
            "\n"
            "SEE ALSO\n"
            "\tchipmunk-cpSpaceNew\n"
            "\tchipmunk-cpSpaceAddBody\n"
            "\tchipmunk-cpSpaceSetGravity\n"
            "\tchipmunk-cpSpaceStep\n",
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

void * dern_chipmunk_to_string(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_environment_t * const targetEnv,
    octaspire_dern_c_data_t * const cData)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(targetEnv);

    if (octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData,
            DERN_CHIPMUNK_PLUGIN_NAME,
            "cpVect"))
    {
        dern_chipmunk_allocation_context_t const * const context =
            cData->payload;

        octaspire_helpers_verify_not_null(context);

        cpVect const * const vect = context->payload;

        octaspire_helpers_verify_not_null(vect);

        return octaspire_string_new_format(
            octaspire_dern_vm_get_allocator(vm),
            "(%f, %f)",
            vect->x,
            vect->y);
    }
    else if (octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData,
            DERN_CHIPMUNK_PLUGIN_NAME,
            "cpSpace"))
    {
        cpSpace const * const space = cData->payload;

        octaspire_helpers_verify_not_null(space);

        return octaspire_string_new_format(
            octaspire_dern_vm_get_allocator(vm),
            "cpSpace gravity: (%f, %f) damping: %f",
            space->gravity.x,
            space->gravity.y,
            space->damping);
    }
    else if (octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData,
            DERN_CHIPMUNK_PLUGIN_NAME,
            "cpBody"))
    {
        cpBody const * const body = cData->payload;

        octaspire_helpers_verify_not_null(body);

        cpVect const pos = cpBodyGetPosition(body);

        return octaspire_string_new_format(
            octaspire_dern_vm_get_allocator(vm),
            "cpBody at: (%f, %f)",
            pos.x,
            pos.y);
    }
    else
    {
        return octaspire_string_new(
            "Dern_chipmunk: not chipmunk C data.",
            octaspire_dern_vm_get_allocator(vm));
    }
}

bool dern_chipmunk_clean(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_environment_t * const targetEnv)
{
    octaspire_helpers_verify_true(vm && targetEnv);

    octaspire_string_release(dern_chipmunk_private_lib_name);
    dern_chipmunk_private_lib_name = 0;

    return true;
}
