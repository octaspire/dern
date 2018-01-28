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
#include "octaspire-dern-amalgamated.c"

typedef struct octaspire_dern_animation_frame_t
{
    int                              srcX;
    int                              srcY;
    int                              srcW;
    int                              srcH;
    int                              dstX;
    int                              dstY;
    int                              dstW;
    int                              dstH;
    double                           secondsToShow;
}
octaspire_dern_animation_frame_t;

typedef struct octaspire_dern_animation_t
{
    octaspire_container_vector_t *      frames;
    octaspire_dern_value_t *            targetValueSrcX;
    octaspire_dern_value_t *            targetValueSrcY;
    octaspire_dern_value_t *            targetValueSrcW;
    octaspire_dern_value_t *            targetValueSrcH;
    octaspire_dern_value_t *            targetValueDstX;
    octaspire_dern_value_t *            targetValueDstY;
    octaspire_dern_value_t *            targetValueDstW;
    octaspire_dern_value_t *            targetValueDstH;
    octaspire_container_utf8_string_t * evalOnDone;
    double                              currentFrameUptime;
    size_t                              currentFrameIndex;
}
octaspire_dern_animation_t;

static octaspire_container_hash_map_t * dern_animation_private_animations               = 0;

// frame:     srcX srcY srcW srcH dstX dtsY dstW dstH secondsToShow

// animation: name optionalEvalOnDone optionalFrame0 optionalFrame1 optionalFrameN

octaspire_dern_value_t *dern_animation_add(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs < 9)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'animation-add' expects at least 9 argument. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_dern_animation_t animation =
    {
        .frames             = 0,
        .targetValueSrcX    = 0,
        .targetValueSrcY    = 0,
        .targetValueSrcW    = 0,
        .targetValueSrcH    = 0,
        .targetValueDstX    = 0,
        .targetValueDstY    = 0,
        .targetValueDstW    = 0,
        .targetValueDstH    = 0,
        .evalOnDone         = 0,
        .currentFrameUptime = 0,
        .currentFrameIndex  = 0
    };

    octaspire_dern_value_t const * const nameArg =
        octaspire_dern_value_as_vector_get_element_of_type_at_const(
            arguments,
            OCTASPIRE_DERN_VALUE_TAG_STRING,
            0);

    if (!nameArg)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'animation-add' expects string for the name as the first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(
                octaspire_dern_value_as_vector_get_element_type_at_const(arguments, 0)));
    }

    animation.frames = octaspire_container_vector_new(
        sizeof(octaspire_dern_animation_frame_t),
        false,
        0,
        octaspire_dern_vm_get_allocator(vm));

    octaspire_helpers_verify_not_null(animation.frames);

    for (size_t i = 0; i < 8; ++i)
    {
        octaspire_dern_value_t * const rectArg =
            octaspire_dern_value_as_vector_get_element_of_type_at(
                arguments,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                1 + i);

        if (!rectArg)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'animation-add' expects integer for the rect x,y,w,h as the %zu. "
                "argument. Type '%s' was given.",
                1 + i,
                octaspire_dern_value_helper_get_type_as_c_string(
                    octaspire_dern_value_as_vector_get_element_type_at_const(arguments, 1 + i)));
        }

        switch (i)
        {
            case 0:  animation.targetValueSrcX = rectArg; break;
            case 1:  animation.targetValueSrcY = rectArg; break;
            case 2:  animation.targetValueSrcW = rectArg; break;
            case 3:  animation.targetValueSrcH = rectArg; break;
            case 4:  animation.targetValueDstX = rectArg; break;
            case 5:  animation.targetValueDstY = rectArg; break;
            case 6:  animation.targetValueDstW = rectArg; break;
            case 7:  animation.targetValueDstH = rectArg; break;
            default: abort();
        }
    }

    size_t firstFrameIndex = 10;

    if (numArgs > 9)
    {
        octaspire_dern_value_t const * const stringOrFrameArg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, 9);

        octaspire_helpers_verify_not_null(stringOrFrameArg);

        if (octaspire_dern_value_is_string(stringOrFrameArg))
        {
            animation.evalOnDone = octaspire_container_utf8_string_new(
                octaspire_dern_value_as_string_get_c_string(stringOrFrameArg),
                octaspire_dern_vm_get_allocator(vm));
        }
        else if (octaspire_dern_value_is_vector(stringOrFrameArg))
        {
            firstFrameIndex = 9;
        }
        else
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'animation-add' expects string to be evaluated or vector for frame as "
                "the 10. argument. Type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(stringOrFrameArg->typeTag));
        }
    }

    for (size_t i = firstFrameIndex; i < numArgs; ++i)
    {
        octaspire_dern_value_t const * const frameArg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, i);

        octaspire_helpers_verify_not_null(frameArg);

        if (!octaspire_dern_value_is_vector(frameArg))
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'animation-add' expects vector for frame as the %zu. argument. "
                "Type '%s' was given.",
                i + 1,
                octaspire_dern_value_helper_get_type_as_c_string(frameArg->typeTag));
        }

        if (octaspire_dern_value_as_vector_get_length(frameArg) != 9)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'animation-add' expects vector with 9 elements (for a frame) as the %zu. "
                "argument. Vector has %zu elements.",
                i + 1,
                octaspire_dern_value_as_vector_get_length(frameArg));
        }
        
        octaspire_dern_animation_frame_t frame;

        for (size_t j = 0; j < 8; ++j)
        {
            octaspire_dern_value_t const * const elemVal =
                octaspire_dern_value_as_vector_get_element_at_const(frameArg, j);

            octaspire_helpers_verify_not_null(elemVal);

            if (!octaspire_dern_value_is_integer(elemVal))
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'animation-add' expects frame vector to have 8 integers and one real. "
                    "Type '%s' was given as the %zu. element.",
                    octaspire_dern_value_helper_get_type_as_c_string(elemVal->typeTag),
                    j + 1);
            }

            switch (j)
            {
                case 0: frame.srcX = octaspire_dern_value_as_integer_get_value(elemVal); break;
                case 1: frame.srcY = octaspire_dern_value_as_integer_get_value(elemVal); break;
                case 2: frame.srcW = octaspire_dern_value_as_integer_get_value(elemVal); break;
                case 3: frame.srcH = octaspire_dern_value_as_integer_get_value(elemVal); break;
                case 4: frame.dstX = octaspire_dern_value_as_integer_get_value(elemVal); break;
                case 5: frame.dstY = octaspire_dern_value_as_integer_get_value(elemVal); break;
                case 6: frame.dstW = octaspire_dern_value_as_integer_get_value(elemVal); break;
                case 7: frame.dstH = octaspire_dern_value_as_integer_get_value(elemVal); break;
            }

        }
        
        octaspire_dern_value_t const * const elemVal =
            octaspire_dern_value_as_vector_get_element_at_const(frameArg, 8);

        octaspire_helpers_verify_not_null(elemVal);

        if (!octaspire_dern_value_is_real(elemVal))
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'animation-add' expects frame vector to have 8 integers and one real. "
                "Type '%s' was given as the last element.",
                octaspire_dern_value_helper_get_type_as_c_string(elemVal->typeTag));
        }

       frame.secondsToShow = octaspire_dern_value_as_real_get_value(elemVal);

        octaspire_helpers_verify_true(
            octaspire_container_vector_push_back_element(
                animation.frames,
                &frame));
    }

    octaspire_container_utf8_string_t * const nameStr =
        octaspire_container_utf8_string_new(
            octaspire_dern_value_as_string_get_c_string(nameArg),
            octaspire_dern_vm_get_allocator(vm));

    octaspire_helpers_verify_not_null(nameStr);

    octaspire_helpers_verify_true(octaspire_container_hash_map_put(
        dern_animation_private_animations,
        octaspire_container_utf8_string_get_hash(nameStr),
        &nameStr,
        &animation));

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(
        vm,
        true);
}

octaspire_dern_value_t *dern_animation_update(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'animatio-update' expects one argument. "
            "%zu arguments were given.",
            numArgs);
    }

    double dt = 0;

    octaspire_dern_value_t const * const firstArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(firstArg);

    if (!octaspire_dern_value_is_real(firstArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'animation-update' expects real dt as the first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    dt = octaspire_dern_value_as_number_get_value(firstArg);

    octaspire_container_hash_map_element_iterator_t iterator =
        octaspire_container_hash_map_element_iterator_init(dern_animation_private_animations);

    while (iterator.element)
    {
        octaspire_dern_animation_t * const animation =
            (octaspire_dern_animation_t * const)octaspire_container_hash_map_element_get_value(
                iterator.element);

        octaspire_helpers_verify_not_null(animation);
        octaspire_helpers_verify_not_null(animation->frames);

        size_t const numFrames = octaspire_container_vector_get_length(animation->frames);

        for (size_t i = 0; i < numFrames; ++i)
        {
            octaspire_dern_animation_frame_t const * const frame =
                (octaspire_dern_animation_frame_t const * const)octaspire_container_vector_get_element_at(
                    animation->frames, i);

            octaspire_helpers_verify_not_null(frame);

            animation->currentFrameUptime += dt;

            if (animation->currentFrameUptime >= frame->secondsToShow)
            {
                animation->currentFrameUptime = 0;

                if (animation->currentFrameIndex >= numFrames)
                {
                    animation->currentFrameIndex = 0;
                }
                else
                {
                    ++(animation->currentFrameIndex);
                }

                octaspire_dern_value_as_integer_set_value(animation->targetValueSrcX, frame->srcX);
                octaspire_dern_value_as_integer_set_value(animation->targetValueSrcY, frame->srcY);
                octaspire_dern_value_as_integer_set_value(animation->targetValueSrcW, frame->srcW);
                octaspire_dern_value_as_integer_set_value(animation->targetValueSrcH, frame->srcH);
                octaspire_dern_value_as_integer_set_value(animation->targetValueDstX, frame->dstX);
                octaspire_dern_value_as_integer_set_value(animation->targetValueDstY, frame->dstY);
                octaspire_dern_value_as_integer_set_value(animation->targetValueDstW, frame->dstW);
                octaspire_dern_value_as_integer_set_value(animation->targetValueDstH, frame->dstH);
            }
        }

        octaspire_container_hash_map_element_iterator_next(&iterator);
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(
        vm,
        true);
}

octaspire_dern_value_t *dern_animation_has_any(
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
            "Builtin 'animation-has-any' expects no arguments. "
            "%zu arguments were given.",
            numArgs);
    }
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(
        vm,
        !octaspire_container_hash_map_is_empty(dern_animation_private_animations));
}

bool dern_animation_init(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_environment_t * const targetEnv)
{
    octaspire_helpers_verify_true(vm && targetEnv);

    dern_animation_private_animations =
        octaspire_container_hash_map_new_with_octaspire_container_utf8_string_keys(
            sizeof(octaspire_dern_animation_t),
            false,
            0,
            octaspire_dern_vm_get_allocator(vm));

    if (!dern_animation_private_animations)
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "animation-add",
            dern_animation_add,
            1,
            "(animation-add name optionalEvalOnDone optionalFrame0 optionalFrame1 .. "
            "optionalFrameN -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "animation-update",
            dern_animation_update,
            1,
            "(animation-update dt) -> true",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "animation-has-any",
            dern_animation_has_any,
            1,
            "(animation-has-any) -> <true or false>",
            true,
            targetEnv))
    {
        return false;
    }

    return true;
}

bool dern_animation_mark_all(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_environment_t * const targetEnv)
{
    octaspire_helpers_verify_true(vm && targetEnv);

    octaspire_container_hash_map_element_iterator_t iterator =
        octaspire_container_hash_map_element_iterator_init(dern_animation_private_animations);

    while (iterator.element)
    {
        octaspire_dern_animation_t * const animation =
            (octaspire_dern_animation_t * const)octaspire_container_hash_map_element_get_value(
                iterator.element);

        octaspire_helpers_verify_not_null(animation);

        octaspire_dern_value_mark(animation->targetValueSrcX);
        octaspire_dern_value_mark(animation->targetValueSrcY);
        octaspire_dern_value_mark(animation->targetValueSrcW);
        octaspire_dern_value_mark(animation->targetValueSrcH);
        octaspire_dern_value_mark(animation->targetValueDstX);
        octaspire_dern_value_mark(animation->targetValueDstY);
        octaspire_dern_value_mark(animation->targetValueDstW);
        octaspire_dern_value_mark(animation->targetValueDstH);

        octaspire_container_hash_map_element_iterator_next(&iterator);
    }

    return true;
}

bool dern_animation_clean(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_environment_t * const targetEnv)
{
    octaspire_helpers_verify_true(vm && targetEnv);

    octaspire_container_hash_map_release(dern_animation_private_animations);
    dern_animation_private_animations = 0;

    return true;
}

