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



typedef struct dern_chipmunk_collision_wildcard_context_t
{
    octaspire_allocator_t  *allocator;
    octaspire_dern_vm_t    *vm;
    octaspire_dern_value_t *environment;
    octaspire_dern_value_t *postSolveCallback;
    octaspire_dern_value_t *separateCallback;
    cpCollisionType         collisionType;
}
dern_chipmunk_collision_wildcard_context_t;

dern_chipmunk_collision_wildcard_context_t *
dern_chipmunk_collision_wildcard_context_new(
    octaspire_dern_vm_t    * const vm,
    octaspire_dern_value_t * const environment,
    octaspire_dern_value_t * const postSolveCallback,
    octaspire_dern_value_t * const separateCallback,
    cpCollisionType          const collisionType,
    octaspire_allocator_t  * const allocator)
{
    dern_chipmunk_collision_wildcard_context_t * self =
        octaspire_allocator_malloc(
            allocator,
            sizeof(dern_chipmunk_collision_wildcard_context_t));

    if (!self)
    {
        return 0;
    }

    self->allocator         = allocator;
    self->vm                = vm;
    self->environment       = environment;
    self->postSolveCallback = postSolveCallback;
    self->separateCallback  = separateCallback;
    self->collisionType     = collisionType;

    return self;
}

void dern_chipmunk_collision_wildcard_context_release(
    dern_chipmunk_collision_wildcard_context_t * self)
{
    if (!self)
    {
        return;
    }

    octaspire_allocator_free(self->allocator, self);
}

static octaspire_map_t * dern_chipmunk_private_collision_wildcard_contexts = 0;



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

void dern_chipmunk_cpShape_clean_up_callback(void *payload)
{
    octaspire_helpers_verify_not_null(payload);
    cpShapeFree((cpShape*)payload);
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
    cpSpaceSetIterations(space, 10);

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

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_CHIPMUNK_PLUGIN_NAME,
        "cpSpace",
        "dern_chipmunk_cpSpace_clean_up_callback",
        "",
        "",
        "",
        "dern_chipmunk_to_string",
        "dern_chipmunk_compare",
        false,
        space);
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

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_CHIPMUNK_PLUGIN_NAME,
        "cpBody",
        "dern_chipmunk_cpBody_clean_up_callback",
        "",
        "",
        "",
        "dern_chipmunk_to_string",
        "dern_chipmunk_compare",
        false,
        body);
}

octaspire_dern_value_t *dern_chipmunk_cpBodyNewStatic(
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
            "Builtin 'chipmunk-cpBodyNewStatic' expects no arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    cpBody * const body = cpBodyNewStatic();

    octaspire_helpers_verify_not_null(body);

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_CHIPMUNK_PLUGIN_NAME,
        "cpBody",
        "dern_chipmunk_cpBody_clean_up_callback",
        "",
        "",
        "",
        "dern_chipmunk_to_string",
        "dern_chipmunk_compare",
        false,
        body);
}

octaspire_dern_value_t *dern_chipmunk_cpBodyNewKinematic(
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
            "Builtin 'chipmunk-cpBodyNewKinematic' expects no arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    cpBody * const body = cpBodyNewKinematic();

    octaspire_helpers_verify_not_null(body);

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_CHIPMUNK_PLUGIN_NAME,
        "cpBody",
        "dern_chipmunk_cpBody_clean_up_callback",
        "",
        "",
        "",
        "dern_chipmunk_to_string",
        "dern_chipmunk_compare",
        false,
        body);
}

octaspire_dern_value_t *dern_chipmunk_cpCircleShapeNew(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const         stackLength  = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName = "chipmunk-cpCircleShapeNew";
    char   const * const cpBodyName   = "cpBody";
    char   const * const cpVectName   = "cpVect";

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 3)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects three arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numArgs);
    }

    // cpBody

    octaspire_dern_c_data_or_unpushed_error_t cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
            arguments,
            0,
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

    // radius

    octaspire_dern_value_t const * const secondArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(secondArg);

    if (!octaspire_dern_value_is_number(secondArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects number for the radius "
            "as the second argument. Type '%s' was given.",
            dernFuncName,
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }

    float const radius = octaspire_dern_value_as_number_get_value(secondArg);

    // cpVect

    cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
            arguments,
            2,
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

    cpVect const * const offset = context->payload;

    // Create the shape.

    cpShape * const shape = cpCircleShapeNew(body, radius, *offset);

    octaspire_helpers_verify_not_null(shape);

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_CHIPMUNK_PLUGIN_NAME,
        "cpShape",
        "dern_chipmunk_cpShape_clean_up_callback",
        "",
        "",
        "",
        "dern_chipmunk_to_string",
        "dern_chipmunk_compare",
        false,
        shape);
}

octaspire_dern_value_t *dern_chipmunk_cpBoxShapeNew(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const         stackLength  = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName = "chipmunk-cpBoxShapeNew";
    char   const * const cpBodyName   = "cpBody";

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 4)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects four arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numArgs);
    }

    // cpBody

    octaspire_dern_c_data_or_unpushed_error_t cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
            arguments,
            0,
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

    // width

    octaspire_dern_number_or_unpushed_error_t numberOrError =
        octaspire_dern_value_as_vector_get_element_at_as_number_or_unpushed_error_const(
            arguments,
            1,
            dernFuncName);

    if (numberOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return numberOrError.unpushedError;
    }

    float const width = numberOrError.number;

    // height

    numberOrError =
        octaspire_dern_value_as_vector_get_element_at_as_number_or_unpushed_error_const(
            arguments,
            2,
            dernFuncName);

    if (numberOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return numberOrError.unpushedError;
    }

    float const height = numberOrError.number;

    // (bevel) radius

    numberOrError =
        octaspire_dern_value_as_vector_get_element_at_as_number_or_unpushed_error_const(
            arguments,
            3,
            dernFuncName);

    if (numberOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return numberOrError.unpushedError;
    }

    float const radius = numberOrError.number;

    // Create the shape.

    cpShape * const shape = cpBoxShapeNew(body, width, height, radius);

    octaspire_helpers_verify_not_null(shape);

    // TODO XXX set from argument, or with separate function
    cpShapeSetCollisionType(shape, 1);

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_CHIPMUNK_PLUGIN_NAME,
        "cpShape",
        "dern_chipmunk_cpShape_clean_up_callback",
        "",
        "",
        "",
        "dern_chipmunk_to_string",
        "dern_chipmunk_compare",
        false,
        shape);
}

octaspire_dern_value_t *dern_chipmunk_cpSegmentShapeNew(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const         stackLength  = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName = "chipmunk-cpSegmentShapeNew";
    char   const * const cpBodyName   = "cpBody";
    char   const * const cpVectName   = "cpVect";

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 4)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects four arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numArgs);
    }

    // cpBody

    octaspire_dern_c_data_or_unpushed_error_t cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
            arguments,
            0,
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

    // cpVect for a

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

    dern_chipmunk_allocation_context_t const * context =
        cDataOrError.cData;

    octaspire_helpers_verify_not_null(context);

    cpVect const * const a = context->payload;

    // cpVect for b

    cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
            arguments,
            2,
            dernFuncName,
            cpVectName,
            DERN_CHIPMUNK_PLUGIN_NAME);

    if (cDataOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrError.unpushedError;
    }

    context = cDataOrError.cData;
    octaspire_helpers_verify_not_null(context);

    cpVect const * const b = context->payload;

    // Radius

    octaspire_dern_number_or_unpushed_error_t numberOrError =
        octaspire_dern_value_as_vector_get_element_at_as_number_or_unpushed_error_const(
            arguments,
            3,
            dernFuncName);

    if (numberOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return numberOrError.unpushedError;
    }

    float const radius = numberOrError.number;

    // Create the shape.

    cpShape * const shape = cpSegmentShapeNew(body, *a, *b, radius);

    octaspire_helpers_verify_not_null(shape);

    // TODO XXX set from argument, or with separate function
    cpShapeSetCollisionType(shape, 1);

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_CHIPMUNK_PLUGIN_NAME,
        "cpShape",
        "dern_chipmunk_cpShape_clean_up_callback",
        "",
        "",
        "",
        "dern_chipmunk_to_string",
        "dern_chipmunk_compare",
        false,
        shape);
}

octaspire_dern_value_t *dern_chipmunk_cpShapeSetFriction(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const         stackLength  = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName = "chipmunk-cpShapeSetFriction";
    char   const * const cpShapeName  = "cpShape";

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 2)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects four arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numArgs);
    }

    // cpShape

    octaspire_dern_c_data_or_unpushed_error_t cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
            arguments,
            0,
            dernFuncName,
            cpShapeName,
            DERN_CHIPMUNK_PLUGIN_NAME);

    if (cDataOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrError.unpushedError;
    }

    cpShape * const shape = cDataOrError.cData;

    // Friction

    octaspire_dern_number_or_unpushed_error_t numberOrError =
        octaspire_dern_value_as_vector_get_element_at_as_number_or_unpushed_error_const(
            arguments,
            1,
            dernFuncName);

    if (numberOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return numberOrError.unpushedError;
    }

    float const friction = numberOrError.number;

    // Set the friction.

    octaspire_helpers_verify_not_null(shape);

    cpShapeSetFriction(shape, friction);

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_boolean(vm, true);
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

octaspire_dern_value_t *dern_chipmunk_cpSpaceReindexStatic(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const         stackLength  = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName = "chipmunk-cpSpaceReindexStatic";
    char   const * const cpSpaceName  = "cpSpace";

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 1)
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

    cpSpaceReindexStatic(space);

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

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_CHIPMUNK_PLUGIN_NAME,
        "cpVect",
        "dern_chipmunk_cpVect_clean_up_callback",
        "",
        "",
        "",
        "dern_chipmunk_to_string",
        "dern_chipmunk_compare",
        false,
        context);
}

octaspire_dern_value_t *dern_chipmunk_cpSpaceGetStaticBody(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const         stackLength  = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName = "chipmunk-cpSpaceGetStaticBody";
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

    octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

    // Return the space provided static body.

    return octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_CHIPMUNK_PLUGIN_NAME,
        "cpBody",
        "dern_chipmunk_cpBody_clean_up_callback",
        "",
        "",
        "",
        "dern_chipmunk_to_string",
        "dern_chipmunk_compare",
        false,
        cpSpaceGetStaticBody(space));
}

octaspire_dern_value_t *dern_chipmunk_cpBodySetVelocity(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const         stackLength  = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName = "chipmunk-cpBodySetVelocity";
    char   const * const cpBodyName   = "cpBody";
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

    // cpBody

    octaspire_dern_c_data_or_unpushed_error_t cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
            arguments,
            0,
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

    cpVect const * const velocity = context->payload;

    // Set the velocity of the body.

    cpBodySetVelocity(body, *velocity);

    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

octaspire_dern_value_t *dern_chipmunk_cpBodySetPosition(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const         stackLength  = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName = "chipmunk-cpBodySetPosition";
    char   const * const cpBodyName   = "cpBody";
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

    // cpBody

    octaspire_dern_c_data_or_unpushed_error_t cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
            arguments,
            0,
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

    cpVect const * const position = context->payload;

    // Set the position of the body.

    cpBodySetPosition(body, *position);

    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

octaspire_dern_value_t *dern_chipmunk_cpBodyGetAngle(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const         stackLength  = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName = "chipmunk-cpBodyGetAngle";
    char   const * const cpBodyName   = "cpBody";

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 1)
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

    // cpBody

    octaspire_dern_c_data_or_unpushed_error_t cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
            arguments,
            0,
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

    float const angle = cpBodyGetAngle(body);

    return octaspire_dern_vm_create_new_value_real(vm, angle);
}

octaspire_dern_value_t *dern_chipmunk_cpBodyGetPosition(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const         stackLength  = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName = "chipmunk-cpBodyGetPosition";
    char   const * const cpBodyName   = "cpBody";

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 1)
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

    // cpBody

    octaspire_dern_c_data_or_unpushed_error_t cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
            arguments,
            0,
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

    cpVect const position = cpBodyGetPosition(body);

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

    vect->x = position.x;
    vect->y = position.y;

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

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_CHIPMUNK_PLUGIN_NAME,
        "cpVect",
        "dern_chipmunk_cpVect_clean_up_callback",
        "",
        "",
        "",
        "dern_chipmunk_to_string",
        "dern_chipmunk_compare",
        false,
        context);
}

octaspire_dern_value_t *dern_chipmunk_cpBodyApplyImpulseAtLocalPoint(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const         stackLength  = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName = "chipmunk-cpBodyApplyImpulseAtLocalPoint";
    char   const * const cpBodyName   = "cpBody";
    char   const * const cpVectName   = "cpVect";

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 3)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects three arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numArgs);
    }

    // cpBody

    octaspire_dern_c_data_or_unpushed_error_t cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
            arguments,
            0,
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

    // cpVect for the impulse

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

    dern_chipmunk_allocation_context_t const * context =
        cDataOrError.cData;

    octaspire_helpers_verify_not_null(context);

    cpVect const * const impulse = context->payload;

    // cpVect for the point

    cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
            arguments,
            2,
            dernFuncName,
            cpVectName,
            DERN_CHIPMUNK_PLUGIN_NAME);

    if (cDataOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrError.unpushedError;
    }

    context = cDataOrError.cData;

    octaspire_helpers_verify_not_null(context);

    cpVect const * const point = context->payload;

    // Apply the impulse

    cpBodyApplyImpulseAtLocalPoint(body, *impulse, *point);

    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

octaspire_dern_value_t *dern_chipmunk_cpBodyApplyImpulseAtWorldPoint(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const         stackLength  = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName = "chipmunk-cpBodyApplyImpulseAtWorldPoint";
    char   const * const cpBodyName   = "cpBody";
    char   const * const cpVectName   = "cpVect";

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 3)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects three arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numArgs);
    }

    // cpBody

    octaspire_dern_c_data_or_unpushed_error_t cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
            arguments,
            0,
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

    // cpVect for the impulse

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

    dern_chipmunk_allocation_context_t const * context =
        cDataOrError.cData;

    octaspire_helpers_verify_not_null(context);

    cpVect const * const impulse = context->payload;

    // cpVect for the point

    cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
            arguments,
            2,
            dernFuncName,
            cpVectName,
            DERN_CHIPMUNK_PLUGIN_NAME);

    if (cDataOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrError.unpushedError;
    }

    context = cDataOrError.cData;

    octaspire_helpers_verify_not_null(context);

    cpVect const * const point = context->payload;

    // Apply the impulse

    cpBodyApplyImpulseAtWorldPoint(body, *impulse, *point);

    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

octaspire_dern_value_t *dern_chipmunk_cpBodyApplyForceAtLocalPoint(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const         stackLength  = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName = "chipmunk-cpBodyApplyForceAtLocalPoint";
    char   const * const cpBodyName   = "cpBody";
    char   const * const cpVectName   = "cpVect";

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 3)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects three arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numArgs);
    }

    // cpBody

    octaspire_dern_c_data_or_unpushed_error_t cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
            arguments,
            0,
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

    // cpVect for the force

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

    dern_chipmunk_allocation_context_t const * context =
        cDataOrError.cData;

    octaspire_helpers_verify_not_null(context);

    cpVect const * const force = context->payload;

    // cpVect for the point

    cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
            arguments,
            2,
            dernFuncName,
            cpVectName,
            DERN_CHIPMUNK_PLUGIN_NAME);

    if (cDataOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrError.unpushedError;
    }

    context = cDataOrError.cData;

    octaspire_helpers_verify_not_null(context);

    cpVect const * const point = context->payload;

    // Apply the force

    cpBodyApplyForceAtLocalPoint(body, *force, *point);

    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

octaspire_dern_value_t *dern_chipmunk_cpBodyApplyForceAtWorldPoint(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const         stackLength  = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName = "chipmunk-cpBodyApplyForceAtWorldPoint";
    char   const * const cpBodyName   = "cpBody";
    char   const * const cpVectName   = "cpVect";

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 3)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects three arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numArgs);
    }

    // cpBody

    octaspire_dern_c_data_or_unpushed_error_t cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
            arguments,
            0,
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

    // cpVect for the force

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

    dern_chipmunk_allocation_context_t const * context =
        cDataOrError.cData;

    octaspire_helpers_verify_not_null(context);

    cpVect const * const force = context->payload;

    // cpVect for the point

    cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
            arguments,
            2,
            dernFuncName,
            cpVectName,
            DERN_CHIPMUNK_PLUGIN_NAME);

    if (cDataOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrError.unpushedError;
    }

    context = cDataOrError.cData;

    octaspire_helpers_verify_not_null(context);

    cpVect const * const point = context->payload;

    // Apply the force

    cpBodyApplyForceAtWorldPoint(body, *force, *point);

    return octaspire_dern_vm_create_new_value_boolean(vm, true);
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
    octaspire_helpers_verify_not_null(space);

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

octaspire_dern_value_t *dern_chipmunk_cpSpaceAddShape(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const         stackLength  = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName = "chipmunk-cpSpaceAddShape";
    char   const * const cpSpaceName  = "cpSpace";
    char   const * const cpShapeName  = "cpShape";

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

    // cpShape

    cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
            arguments,
            1,
            dernFuncName,
            cpShapeName,
            DERN_CHIPMUNK_PLUGIN_NAME);

    if (cDataOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrError.unpushedError;
    }

    cpShape * const shape = cDataOrError.cData;

    octaspire_helpers_verify_true(cpSpaceAddShape(space, shape) == shape);

    return cDataOrError.cData;
}

void dern_chipmunk_private_do_wildcard_callback(
    cpArbiter              *       arb,
    cpSpace                *       space,
    void                   *       data,
    octaspire_dern_value_t * const callbackValue)
{
    octaspire_helpers_verify_not_null(arb);
    octaspire_helpers_verify_not_null(space);
    octaspire_helpers_verify_not_null(data);
    octaspire_helpers_verify_not_null(callbackValue);

    dern_chipmunk_collision_wildcard_context_t * const context = data;

    size_t const stackLength = octaspire_dern_vm_get_stack_length(context->vm);

    octaspire_helpers_verify_true(
        octaspire_dern_value_is_function(callbackValue));

    octaspire_dern_value_t * const arguments =
        octaspire_dern_vm_create_new_value_vector(context->vm);

    octaspire_dern_vm_push_value(context->vm, arguments);

    cpBody * bodyA = 0;
    cpBody * bodyB = 0;

    cpArbiterGetBodies(arb, &bodyA, &bodyB);

    printf("\n????????????????????? bodyA is %p   and   bodyB is %p\n\n", bodyA, bodyB);

    octaspire_dern_value_t * argument1 =
        octaspire_dern_vm_create_new_value_c_data(
            context->vm,
            DERN_CHIPMUNK_PLUGIN_NAME,
            "cpBody",
            "dern_chipmunk_cpBody_clean_up_callback",
            "",
            "",
            "",
            "dern_chipmunk_to_string",
            "dern_chipmunk_compare",
            false,
            bodyA);

    octaspire_helpers_verify_not_null(argument1);
    octaspire_dern_value_as_vector_push_back_element(arguments, &argument1);

    octaspire_dern_value_t * argument2 =
        octaspire_dern_vm_create_new_value_c_data(
            context->vm,
            DERN_CHIPMUNK_PLUGIN_NAME,
            "cpBody",
            "dern_chipmunk_cpBody_clean_up_callback",
            "",
            "",
            "",
            "dern_chipmunk_to_string",
            "dern_chipmunk_compare",
            false,
            bodyB);

    octaspire_helpers_verify_not_null(argument2);
    octaspire_dern_value_as_vector_push_back_element(arguments, &argument2);

    dern_chipmunk_allocation_context_t * const cpVectContext = octaspire_allocator_malloc(
        octaspire_dern_vm_get_allocator(context->vm),
        sizeof(dern_chipmunk_allocation_context_t));

    if (!cpVectContext)
    {
        abort();
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(context->vm));

        octaspire_dern_value_as_vector_clear(arguments);

        argument1 = octaspire_dern_vm_create_new_value_error_from_c_string(
            context->vm,
            "Builtin 'chipmunk-cpv' failed to allocate memory for a cpVect context.");

        octaspire_dern_value_as_vector_push_back_element(arguments, &argument1);

        octaspire_dern_vm_call_lambda(
            context->vm,
            octaspire_dern_value_as_function(callbackValue),
            arguments,
            context->environment);
    }
    else
    {
        cpVect *vect = octaspire_allocator_malloc(
            octaspire_dern_vm_get_allocator(context->vm),
            sizeof(cpVect));

        octaspire_helpers_verify_not_null(vect);

        cpVect const normalVect = cpArbiterGetNormal(arb);
        vect->x = normalVect.x;
        vect->y = normalVect.y;

        cpVectContext->vm      = context->vm;
        cpVectContext->payload = vect;

        octaspire_dern_value_t * argument3 =
            octaspire_dern_vm_create_new_value_c_data(
                cpVectContext->vm,
                DERN_CHIPMUNK_PLUGIN_NAME,
                "cpVect",
                "dern_chipmunk_cpVect_clean_up_callback",
                "",
                "",
                "",
                "dern_chipmunk_to_string",
                "dern_chipmunk_compare",
                false,
                cpVectContext);

        octaspire_helpers_verify_not_null(argument3);
        octaspire_dern_value_as_vector_push_back_element(arguments, &argument3);

        octaspire_dern_value_t * argument4 =
            octaspire_dern_vm_create_new_value_boolean(
                context->vm,
                cpArbiterIsFirstContact(arb));

        octaspire_helpers_verify_not_null(argument4);
        octaspire_dern_value_as_vector_push_back_element(arguments, &argument4);

        octaspire_dern_value_t * argument5 =
            octaspire_dern_vm_create_new_value_real(
                context->vm,
                cpArbiterTotalKE(arb));

        octaspire_helpers_verify_not_null(argument5);
        octaspire_dern_value_as_vector_push_back_element(arguments, &argument5);

        octaspire_dern_vm_call_lambda(
            context->vm,
            octaspire_dern_value_as_function(callbackValue),
            arguments,
            context->environment);
    }

    octaspire_dern_vm_pop_value(context->vm, arguments);

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(context->vm));
}

void dern_chipmunk_private_wildcard_post_solve_handler(
    cpArbiter * arb,
    cpSpace   * space,
    void      * data)
{
    octaspire_helpers_verify_not_null(arb);
    octaspire_helpers_verify_not_null(space);
    octaspire_helpers_verify_not_null(data);

    dern_chipmunk_collision_wildcard_context_t * const context = data;
    size_t const stackLength = octaspire_dern_vm_get_stack_length(context->vm);
    octaspire_dern_value_t * const callbackValue = context->postSolveCallback;
    octaspire_helpers_verify_not_null(callbackValue);
    dern_chipmunk_private_do_wildcard_callback(arb, space, data, callbackValue);

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(context->vm));
}

void dern_chipmunk_private_wildcard_separate_handler(
    cpArbiter * arb,
    cpSpace   * space,
    void      * data)
{
    octaspire_helpers_verify_not_null(arb);
    octaspire_helpers_verify_not_null(space);
    octaspire_helpers_verify_not_null(data);

    dern_chipmunk_collision_wildcard_context_t * const context = data;
    size_t const stackLength = octaspire_dern_vm_get_stack_length(context->vm);
    octaspire_dern_value_t * const callbackValue = context->separateCallback;
    octaspire_helpers_verify_not_null(callbackValue);
    dern_chipmunk_private_do_wildcard_callback(arb, space, data, callbackValue);

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(context->vm));
}

octaspire_dern_value_t *dern_chipmunk_cpSpaceAddWildCardHandler(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const         stackLength  = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName = "chipmunk-cpSpaceAddWildCardHandler";
    char   const * const cpSpaceName  = "cpSpace";

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 4)
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

    // Collision type

    octaspire_dern_value_t const * const secondArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(secondArg);

    if (!octaspire_dern_value_is_number(secondArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects number for the collision type "
            "as the second argument. Type '%s' was given.",
            dernFuncName,
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }

    cpCollisionType const collisionType =
        octaspire_dern_value_as_number_get_value(secondArg);

    // (Lambda) function for postSolve callback

    octaspire_dern_value_t * const thirdArg =
        octaspire_dern_value_as_vector_get_element_at(arguments, 2);

    octaspire_helpers_verify_not_null(thirdArg);

    if (!octaspire_dern_value_is_function(thirdArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects function for the postSolve callback"
            "as the third argument. Type '%s' was given.",
            dernFuncName,
            octaspire_dern_value_helper_get_type_as_c_string(thirdArg->typeTag));
    }

    // (Lambda) function for separate callback

    octaspire_dern_value_t * const fourthArg =
        octaspire_dern_value_as_vector_get_element_at(arguments, 3);

    octaspire_helpers_verify_not_null(fourthArg);

    if (!octaspire_dern_value_is_function(fourthArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects function for the separate callback"
            "as the fourth argument. Type '%s' was given.",
            dernFuncName,
            octaspire_dern_value_helper_get_type_as_c_string(fourthArg->typeTag));
    }

    cpCollisionHandler * const handler =
        cpSpaceAddWildcardHandler(space, collisionType);

    octaspire_helpers_verify_not_null(handler);

    handler->postSolveFunc = dern_chipmunk_private_wildcard_post_solve_handler;
    handler->separateFunc  = dern_chipmunk_private_wildcard_separate_handler;

    size_t const collisionTypeAsSizeT = (size_t)collisionType;

    dern_chipmunk_collision_wildcard_context_t * const wildcardContext =
        dern_chipmunk_collision_wildcard_context_new(
            vm,
            environment,
            thirdArg,
            fourthArg,
            collisionType,
            octaspire_dern_vm_get_allocator(vm));

    octaspire_helpers_verify_not_null(wildcardContext);

    handler->userData = wildcardContext;

    octaspire_map_put(
        dern_chipmunk_private_collision_wildcard_contexts,
        octaspire_map_helper_size_t_get_hash(collisionTypeAsSizeT),
        &collisionTypeAsSizeT,
        &wildcardContext);

    return octaspire_dern_vm_create_new_value_boolean(vm, true);
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

    if (numArgs != 2 && numArgs != 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'chipmunk-cpv' expects two or zero arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    float x = 0;
    float y = 0;

    if (numArgs == 2)
    {
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

        x = octaspire_dern_value_as_number_get_value(firstArg);

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

        y = octaspire_dern_value_as_number_get_value(secondArg);
    }

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

    vect->x = x;
    vect->y = y;

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

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_CHIPMUNK_PLUGIN_NAME,
        "cpVect",
        "dern_chipmunk_cpVect_clean_up_callback",
        "",
        "",
        "",
        "dern_chipmunk_to_string",
        "dern_chipmunk_compare",
        false,
        context);
}

octaspire_dern_value_t *dern_chipmunk_cpv_get_x(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName = "chipmunk-cpv-get-x";
    char   const * const cpVectName   = "cpVect";

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects one arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numArgs);
    }

    // cpVect

    octaspire_dern_c_data_or_unpushed_error_t cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
            arguments,
            0,
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

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_real(vm, vect->x);
}

octaspire_dern_value_t *dern_chipmunk_cpv_get_y(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName = "chipmunk-cpv-get-y";
    char   const * const cpVectName   = "cpVect";

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects one arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numArgs);
    }

    // cpVect

    octaspire_dern_c_data_or_unpushed_error_t cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
            arguments,
            0,
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

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_real(vm, vect->y);
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

    dern_chipmunk_private_collision_wildcard_contexts =
        octaspire_map_new_with_size_t_keys(
            sizeof(dern_chipmunk_collision_wildcard_context_t*),
            true,
            (octaspire_map_element_callback_t)
                dern_chipmunk_collision_wildcard_context_release,
            octaspire_dern_vm_get_allocator(vm));

    if (!dern_chipmunk_private_collision_wildcard_contexts)
    {
        octaspire_string_release(dern_chipmunk_private_lib_name);
        dern_chipmunk_private_lib_name = 0;
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
            2,
            "NAME\n"
            "\tchipmunk-cpBodyNew\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_chipmunk)\n"
            "\n"
            "\t(chipmunk-cpBodyNew mass moment) -> cpBody or error message\n"
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
            "chipmunk-cpBodyNewStatic",
            dern_chipmunk_cpBodyNewStatic,
            0,
            "NAME\n"
            "\tchipmunk-cpBodyNewStatic\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_chipmunk)\n"
            "\n"
            "\t(chipmunk-cpBodyNewStatic) -> cpBody or error message\n"
            "\n"
            "DESCRIPTION\n"
            "\tCreates and returns a new static cpBody.\n"
            "\n"
            "ARGUMENTS\n"
            "\n"
            "RETURN VALUE\n"
            "\tcpBody to be used with those functions of this library that\n"
            "\texpect cpBody argument.\n"
            "\n"
            "SEE ALSO\n"
            "\tchipmunk-cpBodyNew\n"
            "\tchipmunk-cpBodyNewKinematic\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "chipmunk-cpBodyNewKinematic",
            dern_chipmunk_cpBodyNewKinematic,
            0,
            "NAME\n"
            "\tchipmunk-cpBodyNewKinematic\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_chipmunk)\n"
            "\n"
            "\t(chipmunk-cpBodyNewKinematic) -> cpBody or error message\n"
            "\n"
            "DESCRIPTION\n"
            "\tCreates and returns a new kinematic cpBody.\n"
            "\n"
            "ARGUMENTS\n"
            "\n"
            "RETURN VALUE\n"
            "\tcpBody to be used with those functions of this library that\n"
            "\texpect cpBody argument.\n"
            "\n"
            "SEE ALSO\n"
            "\tchipmunk-cpBodyNew\n"
            "\tchipmunk-cpBodyNewStatic\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "chipmunk-cpCircleShapeNew",
            dern_chipmunk_cpCircleShapeNew,
            3,
            "NAME\n"
            "\tchipmunk-cpCircleShapeNew\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_chipmunk)\n"
            "\n"
            "\t(chipmunk-cpCircleShapeNew cpBody radius offset) -> cpShape or error\n"
            "\n"
            "DESCRIPTION\n"
            "\tCreates, attaches and returns a new circle collision shape.\n"
            "\n"
            "ARGUMENTS\n"
            "\tcpBody shape is attached into this body\n"
            "\tradius radius of the circle shape.\n"
            "\toffset cpVect giving offset from body's center of gravity.\n"
            "\n"
            "RETURN VALUE\n"
            "\tcpShape to be used with those functions of this library that\n"
            "\texpect cpShape argument.\n"
            "\n"
            "SEE ALSO\n"
            "\tchipmunk-cpBodyNew\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "chipmunk-cpBoxShapeNew",
            dern_chipmunk_cpBoxShapeNew,
            4,
            "NAME\n"
            "\tchipmunk-cpBoxShapeNew\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_chipmunk)\n"
            "\n"
            "\t(chipmunk-cpBoxShapeNew cpBody width height radius) -> cpShape or error\n"
            "\n"
            "DESCRIPTION\n"
            "\tCreates, attaches and returns a new box shaped polygon collision shape.\n"
            "\n"
            "ARGUMENTS\n"
            "\tcpBody shape is attached into this body at the center of gravity\n"
            "\twidth  width  of the box shape.\n"
            "\theight height of the box shape.\n"
            "\tradius radius to bevel the corners of the box.\n"
            "\n"
            "RETURN VALUE\n"
            "\tcpShape to be used with those functions of this library that\n"
            "\texpect cpShape argument.\n"
            "\n"
            "SEE ALSO\n"
            "\tchipmunk-cpCircleShapeNew\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "chipmunk-cpSegmentShapeNew",
            dern_chipmunk_cpSegmentShapeNew,
            4,
            "NAME\n"
            "\tchipmunk-cpSegmentShapeNew\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_chipmunk)\n"
            "\n"
            "\t(chipmunk-cpSegmentShapeNew body a b radius) -> cpShape or error\n"
            "\n"
            "DESCRIPTION\n"
            "\tCreates, attaches and returns a new segment collision shape.\n"
            "\n"
            "ARGUMENTS\n"
            "\tbody     shape is attached into this cpBody.\n"
            "\ta        first endpoint of the shape.\n"
            "\tb        second endpoint of the shape.\n"
            "\tradius   radius.\n"
            "\n"
            "RETURN VALUE\n"
            "\tcpShape to be used with those functions of this library that\n"
            "\texpect cpShape argument.\n"
            "\n"
            "SEE ALSO\n"
            "\tchipmunk-cpBoxShapeNew\n"
            "\tchipmunk-cpCircleShapeNew\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "chipmunk-cpShapeSetFriction",
            dern_chipmunk_cpShapeSetFriction,
            2,
            "NAME\n"
            "\tchipmunk-cpShapeSetFriction\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_chipmunk)\n"
            "\n"
            "\t(chipmunk-cpShapeSetFriction shape friction) -> true or error\n"
            "\n"
            "DESCRIPTION\n"
            "\tSets the friction of the given cpShape.\n"
            "\n"
            "ARGUMENTS\n"
            "\tshape    the target cpShape to be modified.\n"
            "\tfriction the friction to be set.\n"
            "\n"
            "RETURN VALUE\n"
            "\ttrue or error if something went wrong.\n"
            "\n"
            "SEE ALSO\n"
            "\tchipmunk-cpBoxShapeNew\n"
            "\tchipmunk-cpCircleShapeNew\n"
            "\tchipmunk-cpSegmentShapeNew\n",
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
            "chipmunk-cpSpaceReindexStatic",
            dern_chipmunk_cpSpaceReindexStatic,
            1,
            "NAME\n"
            "\tchipmunk-cpSpaceReindexStatic\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_chipmunk)\n"
            "\n"
            "\t(chipmunk-cpSpaceReindexStatic space) -> true or error message\n"
            "\n"
            "DESCRIPTION\n"
            "\tUpdate the collision detection info for the static\n"
            "\tshapes in the cpSpace.\n"
            "\n"
            "ARGUMENTS\n"
            "\tspace       the cpSpace where updates are done.\n"
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
            "chipmunk-cpSpaceGetStaticBody",
            dern_chipmunk_cpSpaceGetStaticBody,
            1,
            "NAME\n"
            "\tchipmunk-cpSpaceGetStaticBody\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_chipmunk)\n"
            "\n"
            "\t(chipmunk-cpSpaceGetStaticBody space) -> cpBody or error\n"
            "\n"
            "DESCRIPTION\n"
            "\tGet the space provided static body of the given cpSpace.\n"
            "\n"
            "ARGUMENTS\n"
            "\tspace       the cpSpace where to get the static body from\n"
            "\n"
            "RETURN VALUE\n"
            "\tthe space provided static body or error\n"
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
            "chipmunk-cpBodySetVelocity",
            dern_chipmunk_cpBodySetVelocity,
            2,
            "NAME\n"
            "\tchipmunk-cpBodySetVelocity\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_chipmunk)\n"
            "\n"
            "\t(chipmunk-cpBodySetVelocity body velocity) -> true or error\n"
            "\n"
            "DESCRIPTION\n"
            "\tSets the velocity of the given cpBody. Note, that usually\n"
            "\twhen dealing with dynamic bodies, you should use forces or\n"
            "\timpulses and not set the velocity of the body directly.\n"
            "\n"
            "ARGUMENTS\n"
            "\tbody       the target cpBody\n"
            "\tvelocity   the cpVect velocity 2D vector\n"
            "\n"
            "RETURN VALUE\n"
            "\ttrue or error if something went wrong\n"
            "\n"
            "SEE ALSO\n"
            "",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "chipmunk-cpBodySetPosition",
            dern_chipmunk_cpBodySetPosition,
            2,
            "NAME\n"
            "\tchipmunk-cpBodySetPosition\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_chipmunk)\n"
            "\n"
            "\t(chipmunk-cpBodySetPosition body position) -> true or error\n"
            "\n"
            "DESCRIPTION\n"
            "\tSets the position of the given cpBody.\n"
            "\n"
            "ARGUMENTS\n"
            "\tbody       the target cpBody\n"
            "\tposition   the cpVect position 2D vector\n"
            "\n"
            "RETURN VALUE\n"
            "\ttrue or error if something went wrong\n"
            "\n"
            "SEE ALSO\n"
            "\tchipmunk-cpBodySetVelocity\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "chipmunk-cpBodyGetAngle",
            dern_chipmunk_cpBodyGetAngle,
            1,
            "NAME\n"
            "\tchipmunk-cpBodyGetAngle\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_chipmunk)\n"
            "\n"
            "\t(chipmunk-cpBodyGetAngle body) -> angle or error\n"
            "\n"
            "DESCRIPTION\n"
            "\tGets the angle of the given cpBody.\n"
            "\n"
            "ARGUMENTS\n"
            "\tbody       the cpBody\n"
            "\n"
            "RETURN VALUE\n"
            "\tangle or error if something went wrong\n"
            "\n"
            "SEE ALSO\n"
            "\tchipmunk-cpBodyGetPosition\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "chipmunk-cpBodyGetPosition",
            dern_chipmunk_cpBodyGetPosition,
            1,
            "NAME\n"
            "\tchipmunk-cpBodyGetPosition\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_chipmunk)\n"
            "\n"
            "\t(chipmunk-cpBodyGetPosition body) -> position or error\n"
            "\n"
            "DESCRIPTION\n"
            "\tGets the 2D position vector of the given cpBody.\n"
            "\n"
            "ARGUMENTS\n"
            "\tbody       the cpBody\n"
            "\n"
            "RETURN VALUE\n"
            "\tposition vector or error if something went wrong\n"
            "\n"
            "SEE ALSO\n"
            "\tchipmunk-cpBodyGetAngle\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "chipmunk-cpBodyApplyImpulseAtLocalPoint",
            dern_chipmunk_cpBodyApplyImpulseAtLocalPoint,
            3,
            "NAME\n"
            "\tchipmunk-cpBodyApplyImpulseAtLocalPoint\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_chipmunk)\n"
            "\n"
            "\t(chipmunk-cpBodyApplyImpulseAtLocalPoint body impulse point) -> true or error\n"
            "\n"
            "DESCRIPTION\n"
            "\tApply local impulse to a body.\n"
            "\n"
            "ARGUMENTS\n"
            "\tbody       the target cpBody\n"
            "\timpulse    the cpVect impulse to apply\n"
            "\tpoint      the cpVect body local point where the impulse is applied\n"
            "\n"
            "RETURN VALUE\n"
            "\ttrue or error if something went wrong\n"
            "\n"
            "SEE ALSO\n"
            "\tchipmunk-cpBodyApplyForceAtLocalPoint\n"
            "\tchipmunk-cpBodySetVelocity\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "chipmunk-cpBodyApplyImpulseAtWorldPoint",
            dern_chipmunk_cpBodyApplyImpulseAtWorldPoint,
            3,
            "NAME\n"
            "\tchipmunk-cpBodyApplyImpulseAtWorldPoint\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_chipmunk)\n"
            "\n"
            "\t(chipmunk-cpBodyApplyImpulseAtWorldPoint body impulse point) -> true or error\n"
            "\n"
            "DESCRIPTION\n"
            "\tApply impulse to body as if applied from world position point.\n"
            "\n"
            "ARGUMENTS\n"
            "\tbody       the target cpBody\n"
            "\timpulse    the cpVect impulse to apply\n"
            "\tpoint      the cpVect world point from which the impulse is applied\n"
            "\n"
            "RETURN VALUE\n"
            "\ttrue or error if something went wrong\n"
            "\n"
            "SEE ALSO\n"
            "\tchipmunk-cpBodyApplyForceAtLocalPoint\n"
            "\tchipmunk-cpBodyApplyForceAtWorldPoint\n"
            "\tchipmunk-cpBodySetVelocity\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "chipmunk-cpBodyApplyForceAtLocalPoint",
            dern_chipmunk_cpBodyApplyForceAtLocalPoint,
            3,
            "NAME\n"
            "\tchipmunk-cpBodyApplyForceAtLocalPoint\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_chipmunk)\n"
            "\n"
            "\t(chipmunk-cpBodyApplyForceAtLocalPoint body force point) -> true or error\n"
            "\n"
            "DESCRIPTION\n"
            "\tApply local force to a body.\n"
            "\n"
            "ARGUMENTS\n"
            "\tbody       the target cpBody\n"
            "\tforce      the cpVect force to apply\n"
            "\tpoint      the cpVect body local point where the force is applied\n"
            "\n"
            "RETURN VALUE\n"
            "\ttrue or error if something went wrong\n"
            "\n"
            "SEE ALSO\n"
            "\tchipmunk-cpBodyApplyImpulseAtLocalPoint\n"
            "\tchipmunk-cpBodySetVelocity\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "chipmunk-cpBodyApplyForceAtWorldPoint",
            dern_chipmunk_cpBodyApplyForceAtWorldPoint,
            3,
            "NAME\n"
            "\tchipmunk-cpBodyApplyForceAtWorldPoint\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_chipmunk)\n"
            "\n"
            "\t(chipmunk-cpBodyApplyForceAtWorldPoint body force point) -> true or error\n"
            "\n"
            "DESCRIPTION\n"
            "\tApply force to body as if applied from world point point.\n"
            "\n"
            "ARGUMENTS\n"
            "\tbody       the target cpBody\n"
            "\tforce      the cpVect force to apply\n"
            "\tpoint      the cpVect world point from which the force is applied\n"
            "\n"
            "RETURN VALUE\n"
            "\ttrue or error if something went wrong\n"
            "\n"
            "SEE ALSO\n"
            "\tchipmunk-cpBodyApplyImpulseAtLocalPoint\n"
            "\tchipmunk-cpBodyApplyForceAtLocalPoint\n"
            "\tchipmunk-cpBodySetVelocity\n",
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
            "chipmunk-cpSpaceAddShape",
            dern_chipmunk_cpSpaceAddShape,
            1,
            "NAME\n"
            "\tchipmunk-cpSpaceAddShape\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_chipmunk)\n"
            "\n"
            "\t(chipmunk-cpSpaceAddShape space shape) -> cpShape or error\n"
            "\n"
            "DESCRIPTION\n"
            "\tAdd a shape into the space.\n"
            "\n"
            "ARGUMENTS\n"
            "\tspace         the cpSpace where the shape is added into.\n"
            "\tshape         the cpShape to be added into the space.\n"
            "\n"
            "RETURN VALUE\n"
            "\tthe cpShape that was added into the simulation, or an error\n"
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
            "chipmunk-cpSpaceAddWildCardHandler",
            dern_chipmunk_cpSpaceAddWildCardHandler,
            4,
            "NAME\n"
            "\tchipmunk-cpSpaceAddWildCardHandler\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_chipmunk)\n"
            "\n"
            "\t(chipmunk-cpSpaceAddWildCardHandler cpSpace collisionType postSolveFun separateFun) -> true or an error\n"
            "\n"
            "DESCRIPTION\n"
            "\tAdd collision callbacks for collisions of collisionType with anything.\n"
            "\n"
            "ARGUMENTS\n"
            "\tcpSpace       the cpSpace where the callbacks are added.\n"
            "\tcollisionType the type that collides with something.\n"
            "\tpostSolveFun  the postSolve callback function.\n"
            "\tseparateFun   the separate  callback function.\n"
            "\n"
            "RETURN VALUE\n"
            "\ttrue or an error.\n"
            "\n"
            "SEE ALSO\n"
            "\t\n",
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
            "\t(chipmunk-cpv tx ty) -> cpVect or error\n"
            "\n"
            "DESCRIPTION\n"
            "\tCreates and returns a new Chipmunk 2D vector cpVect.\n"
            "\tIf called without arguments, returns a zero vector.\n"
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

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "chipmunk-cpv-get-x",
            dern_chipmunk_cpv_get_x,
            1,
            "NAME\n"
            "\tchipmunk-cpv-get-x\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_chipmunk)\n"
            "\n"
            "\t(chipmunk-cpv-get-x vect) -> number or error\n"
            "\n"
            "DESCRIPTION\n"
            "\tReturns the X component of a Chipmunk 2D vector cpVect.\n"
            "\n"
            "ARGUMENTS\n"
            "\tvect          the cpVect.\n"
            "\n"
            "RETURN VALUE\n"
            "\tthe x component or error if something went wrong.\n"
            "\n"
            "SEE ALSO\n"
            "\tchipmunk-cpv\n"
            "\tchipmunk-cpv-get-y\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "chipmunk-cpv-get-y",
            dern_chipmunk_cpv_get_y,
            1,
            "NAME\n"
            "\tchipmunk-cpv-get-y\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_chipmunk)\n"
            "\n"
            "\t(chipmunk-cpv-get-y vect) -> number or error\n"
            "\n"
            "DESCRIPTION\n"
            "\tReturns the Y component of a Chipmunk 2D vector cpVect.\n"
            "\n"
            "ARGUMENTS\n"
            "\tvect          the cpVect.\n"
            "\n"
            "RETURN VALUE\n"
            "\tthe y component or error if something went wrong.\n"
            "\n"
            "SEE ALSO\n"
            "\tchipmunk-cpv\n"
            "\tchipmunk-cpv-get-x\n",
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

        cpVect const pos   = cpBodyGetPosition(body);
        float  const angle = cpBodyGetAngle(body);

        return octaspire_string_new_format(
            octaspire_dern_vm_get_allocator(vm),
            "cpBody at: (%f, %f) angle: %f",
            pos.x,
            pos.y,
            angle);
    }
    else if (octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData,
            DERN_CHIPMUNK_PLUGIN_NAME,
            "cpShape"))
    {
        // TODO convert more info from the shape into the string.
        //cpShape const * const shape = cData->payload;

        //octaspire_helpers_verify_not_null(shape);

        return octaspire_string_new(
            "cpShape",
            octaspire_dern_vm_get_allocator(vm));
    }
    else
    {
        return octaspire_string_new(
            "Dern_chipmunk: not chipmunk C data.",
            octaspire_dern_vm_get_allocator(vm));
    }
}

void * dern_chipmunk_compare(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_environment_t * const targetEnv,
    octaspire_dern_c_data_t const * const cData1,
    octaspire_dern_c_data_t const * const cData2)
{
    static int result_0       =  0;
    static int result_plus_1  =  1;
    static int result_minus_1 = -1;

    OCTASPIRE_HELPERS_UNUSED_PARAMETER(vm);
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(targetEnv);

    if (octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData1,
            DERN_CHIPMUNK_PLUGIN_NAME,
            "cpVect"))
    {
        // TODO XXX implement
        abort();
    }
    else if (octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData1,
            DERN_CHIPMUNK_PLUGIN_NAME,
            "cpSpace"))
    {
        // TODO XXX implement
        abort();
    }
    else if (octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData1,
            DERN_CHIPMUNK_PLUGIN_NAME,
            "cpBody"))
    {
        cpBody const * const body1 = cData1->payload;
        octaspire_helpers_verify_not_null(body1);

        // TODO XXX implement
        if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData2,
            DERN_CHIPMUNK_PLUGIN_NAME,
            "cpBody"))
        {
            abort();
        }

        cpBody const * const body2 = cData2->payload;
        octaspire_helpers_verify_not_null(body2);

        printf("\n --- body1 %p   body2 %p ---\n", body1, body2);
        if (body1 == body2)
        {
            return &result_0;
        }

        ptrdiff_t const diff = body1 - body2;

        octaspire_helpers_verify_true(diff != 0);

        if (diff < 0)
        {
            return &result_minus_1;
        }

        return &result_plus_1;
    }
    else if (octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData1,
            DERN_CHIPMUNK_PLUGIN_NAME,
            "cpShape"))
    {
        // TODO XXX implement
        abort();
    }
    else
    {
        // TODO XXX implement
        abort();
    }
}

bool dern_chipmunk_mark_all(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_environment_t * const targetEnv)
{
    octaspire_helpers_verify_true(vm && targetEnv);

    // TODO XXX implement.
    octaspire_map_element_iterator_t iterator =
        octaspire_map_element_iterator_init(
            dern_chipmunk_private_collision_wildcard_contexts);

    while (iterator.element)
    {
        dern_chipmunk_collision_wildcard_context_t * const wildcard =
            (dern_chipmunk_collision_wildcard_context_t * const)
            octaspire_map_element_get_value(
                iterator.element);

        octaspire_helpers_verify_not_null(wildcard);

        octaspire_dern_value_mark(wildcard->postSolveCallback);
        octaspire_dern_value_mark(wildcard->separateCallback);

        octaspire_map_element_iterator_next(&iterator);
    }

    return true;
}

bool dern_chipmunk_clean(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_environment_t * const targetEnv)
{
    octaspire_helpers_verify_true(vm && targetEnv);

    octaspire_map_release(dern_chipmunk_private_collision_wildcard_contexts);
    dern_chipmunk_private_collision_wildcard_contexts = 0;

    octaspire_string_release(dern_chipmunk_private_lib_name);
    dern_chipmunk_private_lib_name = 0;

    return true;
}
