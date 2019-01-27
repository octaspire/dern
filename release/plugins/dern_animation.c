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
    octaspire_allocator_t *      allocator;
    octaspire_vector_t *                frames;
    octaspire_dern_value_t *            targetValueSrcX;
    octaspire_dern_value_t *            targetValueSrcY;
    octaspire_dern_value_t *            targetValueSrcW;
    octaspire_dern_value_t *            targetValueSrcH;
    octaspire_dern_value_t *            targetValueDstX;
    octaspire_dern_value_t *            targetValueDstY;
    octaspire_dern_value_t *            targetValueDstW;
    octaspire_dern_value_t *            targetValueDstH;
    octaspire_string_t * evalOnDone;
    double                              currentFrameUptime;
    size_t                              currentFrameIndex;
    int                                 loopCount;
    size_t                              numTimesPlayed;
    bool                                playing;
}
octaspire_dern_animation_t;

static octaspire_dern_animation_t * octaspire_dern_animation_new(
    octaspire_allocator_t * const allocator)
{
    octaspire_dern_animation_t * const self =
        octaspire_allocator_malloc(allocator, sizeof(octaspire_dern_animation_t));

    if (!self)
    {
        return 0;
    }

    self->allocator          = allocator;

    self->frames             = octaspire_vector_new(
        sizeof(octaspire_dern_animation_frame_t),
        false,
        0,
        allocator);

    self->targetValueSrcX    = 0;
    self->targetValueSrcY    = 0;
    self->targetValueSrcW    = 0;
    self->targetValueSrcH    = 0;
    self->targetValueDstX    = 0;
    self->targetValueDstY    = 0;
    self->targetValueDstW    = 0;
    self->targetValueDstH    = 0;

    self->evalOnDone         = octaspire_string_new(
        "",
        allocator);

    self->currentFrameUptime = 0;
    self->currentFrameIndex  = 0;
    self->loopCount          = 0;
    self->numTimesPlayed     = 0;
    self->playing            = false;

    return self;
}

static void octaspire_dern_animation_release(octaspire_dern_animation_t * const self)
{
    if (!self)
    {
        return;
    }

    octaspire_vector_release(self->frames);
    self->frames = 0;

    octaspire_string_release(self->evalOnDone);
    self->evalOnDone = 0;

    octaspire_allocator_free(self->allocator, self);
}

void octaspire_dern_animation_set_playing(
    octaspire_dern_animation_t * const self,
    bool const playing)
{
    self->playing            = playing;
    self->currentFrameIndex  = 0;
    self->currentFrameUptime = 0;
}

bool octaspire_dern_animation_is_playing(octaspire_dern_animation_t const * const self)
{
    return self->playing;
}

static octaspire_map_t    * dern_animation_private_animations = 0;
static octaspire_string_t * dern_animation_private_lib_name   = 0;

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_animation_add(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs < 10)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'animation-add' expects at least 10 argument. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_dern_animation_t * animation = octaspire_dern_animation_new(
        octaspire_dern_vm_get_allocator(vm));

    octaspire_helpers_verify_not_null(animation);

    octaspire_dern_value_t const * const nameArg =
        octaspire_dern_value_as_vector_get_element_of_type_at_const(
            arguments,
            OCTASPIRE_DERN_VALUE_TAG_STRING,
            0);

    if (!nameArg)
    {
        octaspire_dern_animation_release(animation);
        animation = 0;

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'animation-add' expects string for the name as the first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(
                octaspire_dern_value_as_vector_get_element_type_at_const(arguments, 0)));
    }

    for (size_t i = 0; i < 8; ++i)
    {
        octaspire_dern_value_t * const rectArg =
            octaspire_dern_value_as_vector_get_element_of_type_at(
                arguments,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                1 + i);

        if (!rectArg)
        {
            octaspire_dern_animation_release(animation);
            animation = 0;

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
            case 0:  animation->targetValueSrcX = rectArg; break;
            case 1:  animation->targetValueSrcY = rectArg; break;
            case 2:  animation->targetValueSrcW = rectArg; break;
            case 3:  animation->targetValueSrcH = rectArg; break;
            case 4:  animation->targetValueDstX = rectArg; break;
            case 5:  animation->targetValueDstY = rectArg; break;
            case 6:  animation->targetValueDstW = rectArg; break;
            case 7:  animation->targetValueDstH = rectArg; break;
            default: abort();
        }
    }

    // String to be evaluated when done.

    octaspire_dern_value_t const * const stringToBeEvalOnDone =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 9);

    if (octaspire_dern_value_is_string(stringToBeEvalOnDone))
    {
        octaspire_string_set_from_c_string(
            animation->evalOnDone,
            octaspire_dern_value_as_string_get_c_string(stringToBeEvalOnDone));
    }
    else
    {
        octaspire_dern_animation_release(animation);
        animation = 0;

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'animation-add' expects string to be evaluated as "
            "the 10. argument. Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(stringToBeEvalOnDone->typeTag));
    }

    octaspire_dern_value_t const * const loopCountArg =
        octaspire_dern_value_as_vector_get_element_of_type_at_const(
            arguments,
            OCTASPIRE_DERN_VALUE_TAG_INTEGER,
            10);

    if (!loopCountArg)
    {
        octaspire_dern_animation_release(animation);
        animation = 0;

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'animation-add' expects integer for the loop count as the 11. argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(
                octaspire_dern_value_as_vector_get_element_type_at_const(arguments, 10)));
    }

    animation->loopCount = octaspire_dern_value_as_integer_get_value(loopCountArg);

    for (size_t i = 11; i < numArgs; ++i)
    {
        octaspire_dern_value_t const * const frameArg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, i);

        octaspire_helpers_verify_not_null(frameArg);

        if (!octaspire_dern_value_is_vector(frameArg))
        {
            octaspire_dern_animation_release(animation);
            animation = 0;

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
            octaspire_dern_animation_release(animation);
            animation = 0;

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
                octaspire_dern_animation_release(animation);
                animation = 0;

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
            octaspire_dern_animation_release(animation);
            animation = 0;

            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'animation-add' expects frame vector to have 8 integers and one real. "
                "Type '%s' was given as the last element.",
                octaspire_dern_value_helper_get_type_as_c_string(elemVal->typeTag));
        }

       frame.secondsToShow = octaspire_dern_value_as_real_get_value(elemVal);

        octaspire_helpers_verify_true(
            octaspire_vector_push_back_element(
                animation->frames,
                &frame));
    }

    octaspire_string_t * const nameStr =
        octaspire_string_new(
            octaspire_dern_value_as_string_get_c_string(nameArg),
            octaspire_dern_vm_get_allocator(vm));

    octaspire_helpers_verify_not_null(nameStr);

    octaspire_map_remove(
        dern_animation_private_animations,
        octaspire_string_get_hash(nameStr),
        &nameStr);

    octaspire_helpers_verify_true(octaspire_map_put(
        dern_animation_private_animations,
        octaspire_string_get_hash(nameStr),
        &nameStr,
        &animation));

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(
        vm,
        true);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_animation_remove(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    size_t numRemoved = 0;

    if (!numArgs)
    {
        // Remove all
        octaspire_helpers_verify_true(
            octaspire_map_clear(dern_animation_private_animations));

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_integer(vm, numRemoved);
    }

    for (size_t i = 0; i < numArgs; ++i)
    {
        octaspire_dern_value_t const * const nameArg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, i);

        octaspire_helpers_verify_not_null(nameArg);

        if (!octaspire_dern_value_is_text(nameArg))
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'animation-remove' expects text (string or symbol) as the %zu. argument. "
                "Type '%s' was given.",
                i + 1,
                octaspire_dern_value_helper_get_type_as_c_string(nameArg->typeTag));
        }

        octaspire_string_t * nameStr =
            octaspire_string_new(
                octaspire_dern_value_as_text_get_c_string(nameArg),
                octaspire_dern_vm_get_allocator(vm));

        octaspire_helpers_verify_not_null(nameStr);

        if (octaspire_map_remove(
            dern_animation_private_animations,
            octaspire_string_get_hash(nameStr),
            &nameStr))
        {
            ++numRemoved;
        }

        octaspire_string_release(nameStr);
        nameStr = 0;
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_integer(vm, numRemoved);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_animation_loop(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs < 1 || numArgs > 2)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'animation-loop' expects one or two arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    int loopCount = 0;

    octaspire_dern_value_t const * const nameArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(nameArg);

    if (!octaspire_dern_value_is_text(nameArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'animation-loop' expects text (string or symbol) as the first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(nameArg->typeTag));
    }

    octaspire_string_t * nameStr =
        octaspire_string_new(
            octaspire_dern_value_as_text_get_c_string(nameArg),
            octaspire_dern_vm_get_allocator(vm));

    octaspire_helpers_verify_not_null(nameStr);

    octaspire_map_element_t * const element = octaspire_map_get(
        dern_animation_private_animations,
        octaspire_string_get_hash(nameStr),
        &nameStr);

    if (!element)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        octaspire_dern_value_t * const result = octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'animation-loop' cannot find animation with name '%s'.",
            octaspire_string_get_c_string(nameStr));

        octaspire_string_release(nameStr);
        nameStr = 0;

        return result;;
    }

    octaspire_dern_animation_t * const animation =
        (octaspire_dern_animation_t * const)octaspire_map_element_get_value(element);

    octaspire_helpers_verify_not_null(animation);

    if (numArgs > 1)
    {
        octaspire_dern_value_t const * const loopCountArg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

        if (!octaspire_dern_value_is_integer(loopCountArg))
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'animation-loop' expects integer as the second argument. "
                "Type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(loopCountArg->typeTag));
        }

        animation->loopCount = octaspire_dern_value_as_integer_get_value(loopCountArg);
    }

    loopCount = animation->loopCount;

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_integer(vm, loopCount);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
octaspire_dern_value_t *dern_animation_playing(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs < 1 || numArgs > 2)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'animation-playing' expects one or two arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    bool isPlaying = false;

    octaspire_dern_value_t const * const nameArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(nameArg);

    if (!octaspire_dern_value_is_text(nameArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'animation-playing' expects text (string or symbol) as the first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(nameArg->typeTag));
    }

    octaspire_string_t * nameStr =
        octaspire_string_new(
            octaspire_dern_value_as_text_get_c_string(nameArg),
            octaspire_dern_vm_get_allocator(vm));

    octaspire_helpers_verify_not_null(nameStr);

    octaspire_map_element_t * const element = octaspire_map_get(
        dern_animation_private_animations,
        octaspire_string_get_hash(nameStr),
        &nameStr);

    if (!element)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        octaspire_dern_value_t * const result = octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'animation-playing' cannot find animation with name '%s'.",
            octaspire_string_get_c_string(nameStr));

        octaspire_string_release(nameStr);
        nameStr = 0;

        return result;;
    }

    octaspire_dern_animation_t * const animation =
        (octaspire_dern_animation_t * const)octaspire_map_element_get_value(element);

    octaspire_helpers_verify_not_null(animation);

    if (numArgs > 1)
    {
        octaspire_dern_value_t const * const playingArg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

        if (!octaspire_dern_value_is_boolean(playingArg))
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'animation-playing' expects boolean as the second argument. "
                "Type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(playingArg->typeTag));
        }

        octaspire_dern_animation_set_playing(
            animation,
            octaspire_dern_value_as_boolean_get_value(playingArg));
    }

    isPlaying = octaspire_dern_animation_is_playing(animation);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, isPlaying);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
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

    octaspire_map_element_iterator_t iterator =
        octaspire_map_element_iterator_init(dern_animation_private_animations);

    while (iterator.element)
    {
        octaspire_dern_animation_t * const animation =
            (octaspire_dern_animation_t * const)octaspire_map_element_get_value(
                iterator.element);

        octaspire_helpers_verify_not_null(animation);
        octaspire_helpers_verify_not_null(animation->frames);

        if (octaspire_dern_animation_is_playing(animation))
        {
            size_t const numFrames = octaspire_vector_get_length(animation->frames);

            octaspire_dern_animation_frame_t const * const frame =
                (octaspire_dern_animation_frame_t const * const)
                    octaspire_vector_get_element_at(
                        animation->frames, animation->currentFrameIndex);

            if (frame)
            {
                if (animation->currentFrameUptime >= frame->secondsToShow)
                {
                    animation->currentFrameUptime = 0;
                    ++(animation->currentFrameIndex);

                    if (animation->currentFrameIndex >= numFrames)
                    {
                        ++(animation->numTimesPlayed);

                        if ((animation->loopCount >= 0) &&
                            ((int)(animation->numTimesPlayed) >= animation->loopCount))
                        {
                            octaspire_dern_animation_set_playing(animation, false);
                        }
                        else
                        {
                            animation->currentFrameIndex = 0;
                        }
                    }
                }
                else
                {
                    animation->currentFrameUptime += dt;
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

        octaspire_map_element_iterator_next(&iterator);
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(
        vm,
        true);
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
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
        !octaspire_map_is_empty(dern_animation_private_animations));
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
bool dern_animation_init(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_environment_t * const targetEnv,
    char const * const libName)
{
    octaspire_helpers_verify_true(vm && targetEnv && libName);

    dern_animation_private_lib_name = octaspire_string_new(
        libName,
        octaspire_dern_vm_get_allocator(vm));

    if (!dern_animation_private_lib_name)
    {
        return false;
    }

    dern_animation_private_animations =
        octaspire_map_new_with_octaspire_string_keys(
            sizeof(octaspire_dern_animation_t*),
            true,
            (octaspire_map_element_callback_t)
                octaspire_dern_animation_release,
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
            "(animation-add name sx sy sw sh tx ty tw th evalOnDone loopCount frame ..) -> "
            "<true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "animation-remove",
            dern_animation_remove,
            1,
            "(animation-remove optionalName..) -> numRemoved",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "animation-loop",
            dern_animation_loop,
            1,
            "(animation-loop name optionalLoopCount) -> <loopCount or error message> ",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "animation-playing",
            dern_animation_playing,
            1,
            "(animation-playing name optionalBoolean) -> <isPlaying orr error message> ",
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

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
bool dern_animation_mark_all(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_environment_t * const targetEnv)
{
    octaspire_helpers_verify_true(vm && targetEnv);

    octaspire_map_element_iterator_t iterator =
        octaspire_map_element_iterator_init(dern_animation_private_animations);

    while (iterator.element)
    {
        octaspire_dern_animation_t * const animation =
            (octaspire_dern_animation_t * const)octaspire_map_element_get_value(
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

        octaspire_map_element_iterator_next(&iterator);
    }

    return true;
}

#ifdef _MSC_VER
extern __declspec(dllexport)
#endif
bool dern_animation_clean(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_environment_t * const targetEnv)
{
    octaspire_helpers_verify_true(vm && targetEnv);

    octaspire_map_release(dern_animation_private_animations);
    dern_animation_private_animations = 0;

    octaspire_string_release(dern_animation_private_lib_name);
    dern_animation_private_lib_name = 0;

    return true;
}

