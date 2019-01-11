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
#include "SDL.h"
#include <SDL_opengl.h>
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_GL2_IMPLEMENTATION
#include "nuklear.h"
#include "nuklear_sdl_gl2.h"

static char const * const DERN_NUKLEAR_PLUGIN_NAME = "dern_nuklear";
static octaspire_string_t * dern_nuklear_private_lib_name = 0;

typedef struct dern_nuklear_allocation_context_t
{
    octaspire_dern_vm_t       *vm;
    void                      *payload;
}
dern_nuklear_allocation_context_t;

void dern_nuklear_nk_color_clean_up_callback(void *payload)
{
    octaspire_helpers_verify_not_null(payload);
    dern_nuklear_allocation_context_t * const context = payload;
    octaspire_helpers_verify_not_null(context->vm);
    octaspire_helpers_verify_not_null(context->payload);

    octaspire_allocator_t * const allocator =
        octaspire_dern_vm_get_allocator(context->vm);

    octaspire_allocator_free(allocator, (struct nk_color*)(context->payload));
    octaspire_allocator_free(allocator, context);
}

octaspire_dern_value_t *dern_nuklear_sdl_init(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName    = "nuklear-sdl-init";
    char   const * const windowName      = "window";
    char   const * const sdl2PluginName  = "dern_sdl2";
    size_t const         numExpectedArgs = 1;

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != numExpectedArgs)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects %zu arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numExpectedArgs,
            numArgs);
    }

    // SDL2 window

    octaspire_dern_c_data_or_unpushed_error_t cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error(
            arguments,
            0,
            dernFuncName,
            windowName,
            sdl2PluginName);

    if (cDataOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrError.unpushedError;
    }

    SDL_Window * const window = cDataOrError.cData;

    struct nk_context * ctx = nk_sdl_init(window);

    octaspire_helpers_verify_not_null(ctx);

    struct nk_font_atlas * atlas;
    nk_sdl_font_stash_begin(&atlas);
    nk_sdl_font_stash_end();

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_NUKLEAR_PLUGIN_NAME,
        "ctx",
        "dern_nuklear_ctx_clean_up_callback",
        "",
        "",
        "",
        "",
        "",
        true,
        ctx);
}

octaspire_dern_value_t *dern_nuklear_begin(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName    = "nuklear-begin";
    char   const * const ctxName         = "ctx";
    size_t const         numExpectedArgs = 6;

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs < numExpectedArgs)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects at least %zu arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numExpectedArgs,
            numArgs);
    }

    // ctx

    octaspire_dern_c_data_or_unpushed_error_t cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error(
            arguments,
            0,
            dernFuncName,
            ctxName,
            DERN_NUKLEAR_PLUGIN_NAME);

    if (cDataOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrError.unpushedError;
    }

    struct nk_context * const ctx = cDataOrError.cData;

    octaspire_helpers_verify_not_null(ctx);

    octaspire_dern_value_t const * const secondArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(secondArg);

    if (!octaspire_dern_value_is_text(secondArg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects text (string or symbol) as the second "
            "argument. Type '%s' was given.",
            dernFuncName,
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }

    char const * const title = octaspire_dern_value_as_text_get_c_string(secondArg);

    double numbers[4] = {0};
    size_t const numNumbers = sizeof(numbers) / sizeof(numbers[0]);

    for (size_t i = 0; i < numNumbers; ++i)
    {
        octaspire_dern_number_or_unpushed_error_const_t const numberOrError =
            octaspire_dern_value_as_vector_get_element_at_as_number_or_unpushed_error_const(
                arguments,
                i + 2,
                dernFuncName);

        if (numberOrError.unpushedError)
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return numberOrError.unpushedError;
        }

        numbers[i] = numberOrError.number;
    }

    nk_flags flags = 0;

    for (size_t i = numExpectedArgs; i < numArgs; ++i)
    {
        octaspire_dern_text_or_unpushed_error_const_t const textOrError =
            octaspire_dern_value_as_vector_get_element_at_as_text_or_unpushed_error_const(
                arguments,
                i,
                dernFuncName);

        if (textOrError.unpushedError)
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return textOrError.unpushedError;
        }

        if (octaspire_string_is_equal_to_c_string(textOrError.text, "BORDER"))
        {
            flags |= NK_WINDOW_BORDER;
        }
        else if (octaspire_string_is_equal_to_c_string(textOrError.text, "MOVABLE"))
        {
            flags |= NK_WINDOW_MOVABLE;
        }
        else if (octaspire_string_is_equal_to_c_string(textOrError.text, "SCALABLE"))
        {
            flags |= NK_WINDOW_SCALABLE;
        }
        else if (octaspire_string_is_equal_to_c_string(textOrError.text, "MINIMIZABLE"))
        {
            flags |= NK_WINDOW_MINIMIZABLE;
        }
        else if (octaspire_string_is_equal_to_c_string(textOrError.text, "TITLE"))
        {
            flags |= NK_WINDOW_TITLE;
        }
        else
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin '%s' expects known textual flag as the %zu. "
                "argument. Text '%s' was given.",
                dernFuncName,
                i + 1,
                octaspire_string_get_c_string(textOrError.text));
        }
    }

    bool const result = nk_begin(
        ctx,
        title,
        nk_rect(numbers[0], numbers[1], numbers[2], numbers[3]),
        flags);

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_boolean(vm, result);
}

octaspire_dern_value_t *dern_nuklear_label(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName     = "nuklear-label";
    char   const * const ctxName          = "ctx";
    char   const * const nkColorName      = "nk_color";
    size_t const         numExpectedArgs1 = 3;
    size_t const         numExpectedArgs2 = 4;

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != numExpectedArgs1 && numArgs != numExpectedArgs2)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects %zu or %zu arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numExpectedArgs1,
            numExpectedArgs2,
            numArgs);
    }

    // ctx

    octaspire_dern_c_data_or_unpushed_error_t cDataOrErrorCtx =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error(
            arguments,
            0,
            dernFuncName,
            ctxName,
            DERN_NUKLEAR_PLUGIN_NAME);

    if (cDataOrErrorCtx.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrErrorCtx.unpushedError;
    }

    struct nk_context * const ctx = cDataOrErrorCtx.cData;

    octaspire_helpers_verify_not_null(ctx);

    octaspire_dern_value_t const * const secondArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(secondArg);

    if (!octaspire_dern_value_is_text(secondArg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects text (string or symbol) as the second "
            "argument. Type '%s' was given.",
            dernFuncName,
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }

    char const * const text = octaspire_dern_value_as_text_get_c_string(secondArg);

    octaspire_dern_value_t const * const thirdArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 2);

    octaspire_helpers_verify_not_null(thirdArg);

    if (!octaspire_dern_value_is_text(thirdArg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects text (string or symbol) as the third "
            "argument. Type '%s' was given.",
            dernFuncName,
            octaspire_dern_value_helper_get_type_as_c_string(thirdArg->typeTag));
    }

    octaspire_string_t const * const alignment = octaspire_dern_value_as_text_get_string_const(thirdArg);

    nk_flags flags = 0;

    if (octaspire_string_is_equal_to_c_string(alignment, "LEFT"))
    {
        flags = NK_TEXT_LEFT;
    }
    else if (octaspire_string_is_equal_to_c_string(alignment, "CENTERED"))
    {
        flags = NK_TEXT_CENTERED;
    }
    else if (octaspire_string_is_equal_to_c_string(alignment, "RIGHT"))
    {
        flags = NK_TEXT_RIGHT;
    }
    else
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects text (string or symbol) LEFT, CENTERED or "
            "RIGHT as the third argument. Text '%s' was given.",
            dernFuncName,
            octaspire_string_get_c_string(alignment));
    }

    if (numArgs == 3)
    {
        nk_label(ctx, text, flags);
    }
    else
    {
        octaspire_dern_c_data_or_unpushed_error_t cDataOrErrorNkColor =
            octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error(
                arguments,
                3,
                dernFuncName,
                nkColorName,
                DERN_NUKLEAR_PLUGIN_NAME);

        if (cDataOrErrorNkColor.unpushedError)
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return cDataOrErrorNkColor.unpushedError;
        }

        dern_nuklear_allocation_context_t const * const context =
            cDataOrErrorNkColor.cData;

        octaspire_helpers_verify_not_null(context);

        struct nk_color * const color = context->payload;

        octaspire_helpers_verify_not_null(color);

        nk_label_colored(ctx, text, flags, *color);
    }

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

octaspire_dern_value_t *dern_nuklear_label_wrap(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName     = "nuklear-label";
    char   const * const ctxName          = "ctx";
    char   const * const nkColorName      = "nk_color";
    size_t const         numExpectedArgs1 = 2;
    size_t const         numExpectedArgs2 = 3;

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != numExpectedArgs1 && numArgs != numExpectedArgs2)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects %zu or %zu arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numExpectedArgs1,
            numExpectedArgs2,
            numArgs);
    }

    // ctx

    octaspire_dern_c_data_or_unpushed_error_t cDataOrErrorCtx =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error(
            arguments,
            0,
            dernFuncName,
            ctxName,
            DERN_NUKLEAR_PLUGIN_NAME);

    if (cDataOrErrorCtx.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrErrorCtx.unpushedError;
    }

    struct nk_context * const ctx = cDataOrErrorCtx.cData;

    octaspire_helpers_verify_not_null(ctx);

    octaspire_dern_value_t const * const secondArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(secondArg);

    if (!octaspire_dern_value_is_text(secondArg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects text (string or symbol) as the second "
            "argument. Type '%s' was given.",
            dernFuncName,
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }

    char const * const text = octaspire_dern_value_as_text_get_c_string(secondArg);

    if (numArgs == 2)
    {
        nk_label_wrap(ctx, text);
    }
    else
    {
        octaspire_dern_c_data_or_unpushed_error_t cDataOrErrorNkColor =
            octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error(
                arguments,
                2,
                dernFuncName,
                nkColorName,
                DERN_NUKLEAR_PLUGIN_NAME);

        if (cDataOrErrorNkColor.unpushedError)
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return cDataOrErrorNkColor.unpushedError;
        }

        dern_nuklear_allocation_context_t const * const context =
            cDataOrErrorNkColor.cData;

        octaspire_helpers_verify_not_null(context);

        struct nk_color * const color = context->payload;

        octaspire_helpers_verify_not_null(color);

        nk_label_colored_wrap(ctx, text, *color);
    }

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

octaspire_dern_value_t *dern_nuklear_button_label(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName    = "nuklear-button-label";
    char   const * const ctxName         = "ctx";
    size_t const         numExpectedArgs = 2;

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != numExpectedArgs)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects %zu arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numExpectedArgs,
            numArgs);
    }

    // ctx

    octaspire_dern_c_data_or_unpushed_error_t cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error(
            arguments,
            0,
            dernFuncName,
            ctxName,
            DERN_NUKLEAR_PLUGIN_NAME);

    if (cDataOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrError.unpushedError;
    }

    struct nk_context * const ctx = cDataOrError.cData;

    octaspire_helpers_verify_not_null(ctx);

    // text

    octaspire_dern_value_t const * const secondArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(secondArg);

    if (!octaspire_dern_value_is_text(secondArg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects text (string or symbol) as the second "
            "argument. Type '%s' was given.",
            dernFuncName,
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }

    char const * const text = octaspire_dern_value_as_text_get_c_string(secondArg);

    bool const result = nk_button_label(ctx, text);

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_boolean(vm, result);
}

octaspire_dern_value_t *dern_nuklear_checkbox_label(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName    = "nuklear-checkbox-label";
    char   const * const ctxName         = "ctx";
    size_t const         numExpectedArgs = 3;

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != numExpectedArgs)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects %zu arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numExpectedArgs,
            numArgs);
    }

    // ctx

    octaspire_dern_c_data_or_unpushed_error_t cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error(
            arguments,
            0,
            dernFuncName,
            ctxName,
            DERN_NUKLEAR_PLUGIN_NAME);

    if (cDataOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrError.unpushedError;
    }

    struct nk_context * const ctx = cDataOrError.cData;

    octaspire_helpers_verify_not_null(ctx);

    // text

    octaspire_dern_value_t const * const secondArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(secondArg);

    if (!octaspire_dern_value_is_text(secondArg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects text (string or symbol) as the second "
            "argument. Type '%s' was given.",
            dernFuncName,
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }

    char const * const text = octaspire_dern_value_as_text_get_c_string(secondArg);

    // active

    octaspire_dern_value_t * const thirdArg =
        octaspire_dern_value_as_vector_get_element_at(arguments, 2);

    octaspire_helpers_verify_not_null(thirdArg);

    if (!octaspire_dern_value_is_boolean(thirdArg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects boolean as the third "
            "argument. Type '%s' was given.",
            dernFuncName,
            octaspire_dern_value_helper_get_type_as_c_string(thirdArg->typeTag));
    }

    int active = octaspire_dern_value_as_boolean_get_value(thirdArg);

    bool const result = nk_checkbox_label(ctx, text, &active);

    octaspire_dern_value_as_boolean_set_value(thirdArg, active == nk_true);

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_boolean(vm, result);
}

octaspire_dern_value_t *dern_nuklear_slider_int(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName    = "nuklear-slider-int";
    char   const * const ctxName         = "ctx";
    size_t const         numExpectedArgs = 5;

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != numExpectedArgs)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects %zu arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numExpectedArgs,
            numArgs);
    }

    // ctx

    octaspire_dern_c_data_or_unpushed_error_t cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error(
            arguments,
            0,
            dernFuncName,
            ctxName,
            DERN_NUKLEAR_PLUGIN_NAME);

    if (cDataOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrError.unpushedError;
    }

    struct nk_context * const ctx = cDataOrError.cData;

    octaspire_helpers_verify_not_null(ctx);

    // min

    octaspire_dern_number_or_unpushed_error_const_t numberOrErrorMin =
        octaspire_dern_value_as_vector_get_element_at_as_number_or_unpushed_error_const(
            arguments,
            1,
            dernFuncName);

    if (numberOrErrorMin.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return numberOrErrorMin.unpushedError;
    }

    int const minVal = numberOrErrorMin.number;

    // value

    octaspire_dern_number_or_unpushed_error_t numberOrErrorValue =
        octaspire_dern_value_as_vector_get_element_at_as_number_or_unpushed_error(
            arguments,
            2,
            dernFuncName);

    if (numberOrErrorValue.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return numberOrErrorValue.unpushedError;
    }

    int  valueVal = numberOrErrorValue.number;

    // max

    octaspire_dern_number_or_unpushed_error_const_t numberOrErrorMax =
        octaspire_dern_value_as_vector_get_element_at_as_number_or_unpushed_error_const(
            arguments,
            3,
            dernFuncName);

    if (numberOrErrorMax.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return numberOrErrorMax.unpushedError;
    }

    int const maxVal = numberOrErrorMax.number;

    // step

    octaspire_dern_number_or_unpushed_error_const_t numberOrErrorStep =
        octaspire_dern_value_as_vector_get_element_at_as_number_or_unpushed_error_const(
            arguments,
            4,
            dernFuncName);

    if (numberOrErrorStep.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return numberOrErrorStep.unpushedError;
    }

    int const stepVal = numberOrErrorStep.number;

    // Show the widget.

    bool const result =
        nk_slider_int(ctx, minVal, &valueVal, maxVal, stepVal);

    octaspire_dern_value_as_number_set_value(
        numberOrErrorValue.value,
        (double)valueVal);

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_boolean(vm, result);
}

octaspire_dern_value_t *dern_nuklear_slider_float(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName    = "nuklear-slider-float";
    char   const * const ctxName         = "ctx";
    size_t const         numExpectedArgs = 5;

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != numExpectedArgs)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects %zu arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numExpectedArgs,
            numArgs);
    }

    // ctx

    octaspire_dern_c_data_or_unpushed_error_t cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error(
            arguments,
            0,
            dernFuncName,
            ctxName,
            DERN_NUKLEAR_PLUGIN_NAME);

    if (cDataOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrError.unpushedError;
    }

    struct nk_context * const ctx = cDataOrError.cData;

    octaspire_helpers_verify_not_null(ctx);

    // min

    octaspire_dern_number_or_unpushed_error_const_t numberOrErrorMin =
        octaspire_dern_value_as_vector_get_element_at_as_number_or_unpushed_error_const(
            arguments,
            1,
            dernFuncName);

    if (numberOrErrorMin.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return numberOrErrorMin.unpushedError;
    }

    float const minVal = numberOrErrorMin.number;

    // value

    octaspire_dern_number_or_unpushed_error_t numberOrErrorValue =
        octaspire_dern_value_as_vector_get_element_at_as_number_or_unpushed_error(
            arguments,
            2,
            dernFuncName);

    if (numberOrErrorValue.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return numberOrErrorValue.unpushedError;
    }

    float valueVal = numberOrErrorValue.number;

    // max

    octaspire_dern_number_or_unpushed_error_const_t numberOrErrorMax =
        octaspire_dern_value_as_vector_get_element_at_as_number_or_unpushed_error_const(
            arguments,
            3,
            dernFuncName);

    if (numberOrErrorMax.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return numberOrErrorMax.unpushedError;
    }

    float const maxVal = numberOrErrorMax.number;

    // step

    octaspire_dern_number_or_unpushed_error_const_t numberOrErrorStep =
        octaspire_dern_value_as_vector_get_element_at_as_number_or_unpushed_error_const(
            arguments,
            4,
            dernFuncName);

    if (numberOrErrorStep.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return numberOrErrorStep.unpushedError;
    }

    float const stepVal = numberOrErrorStep.number;

    // Show the widget.

    bool const result =
        nk_slider_float(ctx, minVal, &valueVal, maxVal, stepVal);

    octaspire_dern_value_as_number_set_value(
        numberOrErrorValue.value,
        (double)valueVal);

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_boolean(vm, result);
}

octaspire_dern_value_t *dern_nuklear_option_label(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName    = "nuklear-option-label";
    char   const * const ctxName         = "ctx";
    size_t const         numExpectedArgs = 4;

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != numExpectedArgs)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects %zu arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numExpectedArgs,
            numArgs);
    }

    // ctx

    octaspire_dern_c_data_or_unpushed_error_t cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error(
            arguments,
            0,
            dernFuncName,
            ctxName,
            DERN_NUKLEAR_PLUGIN_NAME);

    if (cDataOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrError.unpushedError;
    }

    struct nk_context * const ctx = cDataOrError.cData;

    octaspire_helpers_verify_not_null(ctx);

    // label

    octaspire_dern_text_or_unpushed_error_const_t textOrErrorLabel =
        octaspire_dern_value_as_vector_get_element_at_as_text_or_unpushed_error_const(
            arguments,
            1,
            dernFuncName);

    if (textOrErrorLabel.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return textOrErrorLabel.unpushedError;
    }

    char const * const labelVal =
        octaspire_string_get_c_string(textOrErrorLabel.text);

    // id

    octaspire_dern_text_or_unpushed_error_const_t textOrErrorId =
        octaspire_dern_value_as_vector_get_element_at_as_text_or_unpushed_error_const(
            arguments,
            2,
            dernFuncName);

    if (textOrErrorId.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return textOrErrorId.unpushedError;
    }

    // active-id

    octaspire_dern_symbol_or_unpushed_error_t symbolOrErrorActiveId =
        octaspire_dern_value_as_vector_get_element_at_as_symbol_or_unpushed_error(
            arguments,
            3,
            dernFuncName);

    if (symbolOrErrorActiveId.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return symbolOrErrorActiveId.unpushedError;
    }

    // Show the widget.

    bool const result = nk_option_label(
        ctx,
        labelVal,
        octaspire_dern_value_as_text_is_equal_to_c_string(
            textOrErrorId.value,
            (octaspire_dern_value_as_text_get_c_string(
                symbolOrErrorActiveId.value))));

    if (result)
    {
        octaspire_helpers_verify_true(
            octaspire_dern_value_as_symbol_set_c_string(
                symbolOrErrorActiveId.value,
                octaspire_string_get_c_string(textOrErrorId.text)));
    }

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_boolean(vm, result);
}

octaspire_dern_value_t *dern_nuklear_edit_string(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName    = "nuklear-edit-string";
    char   const * const ctxName         = "ctx";
    size_t const         numExpectedArgs = 5;

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != numExpectedArgs)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects %zu arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numExpectedArgs,
            numArgs);
    }

    // ctx

    octaspire_dern_c_data_or_unpushed_error_t cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error(
            arguments,
            0,
            dernFuncName,
            ctxName,
            DERN_NUKLEAR_PLUGIN_NAME);

    if (cDataOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrError.unpushedError;
    }

    struct nk_context * const ctx = cDataOrError.cData;

    octaspire_helpers_verify_not_null(ctx);

    // edit-type

    char const * const altEditType[] =
    {
        "SIMPLE",
        "FIELD",
        "BOX",
        "EDITOR",
        0
    };

    octaspire_dern_one_of_texts_or_unpushed_error_const_t textOrErrorEditType =
        octaspire_dern_value_as_vector_get_element_at_as_one_of_texts_or_unpushed_error_const(
            arguments,
            1,
            dernFuncName,
            altEditType);

    if (textOrErrorEditType.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return textOrErrorEditType.unpushedError;
    }

    octaspire_helpers_verify_true(
        textOrErrorEditType.index >= 0 && textOrErrorEditType.index <= 3);

    nk_flags editType = NK_EDIT_SIMPLE;

    switch (textOrErrorEditType.index)
    {
        case 0: { editType = NK_EDIT_SIMPLE; } break;
        case 1: { editType = NK_EDIT_FIELD;  } break;
        case 2: { editType = NK_EDIT_BOX;    } break;
        case 3: { editType = NK_EDIT_EDITOR; } break;
    }

    // TODO This allows committing the changes in the text box
    // by using <ENTER>, but maybe this should not be hard-coded;
    // it might be better to allow it to be given as an argument
    // to this function somehow.
    editType |= NK_EDIT_SIG_ENTER;

    // str

    octaspire_dern_string_or_unpushed_error_t stringOrError =
        octaspire_dern_value_as_vector_get_element_at_as_string_or_unpushed_error(
            arguments,
            2,
            dernFuncName);

    if (stringOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return stringOrError.unpushedError;
    }

    // len

    octaspire_dern_number_or_unpushed_error_const_t numberOrError =
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

    int const len = numberOrError.number;

    if (len < 2)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects input length larger than 1. %zu was given."
            "%zu arguments were given.",
            dernFuncName,
            len);
    }

    // Remove characters until the string fits into the buffer.
    while (octaspire_dern_value_as_string_get_length_in_octets(
               stringOrError.value) >= (size_t)(len - 1))
    {
        octaspire_helpers_verify_true(
            octaspire_dern_value_as_string_pop_back_ucs_character(
                stringOrError.value));
    }

    // filter

    char const * const altFilter[] =
    {
        "DEFAULT",
        "ASCII",
        "FLOAT",
        "DECIMAL",
        "HEX",
        "OCT",
        "BINARY",
        0
    };

    octaspire_dern_one_of_texts_or_unpushed_error_const_t textOrErrorFilter =
        octaspire_dern_value_as_vector_get_element_at_as_one_of_texts_or_unpushed_error_const(
            arguments,
            4,
            dernFuncName,
            altFilter);

    if (textOrErrorFilter.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return textOrErrorFilter.unpushedError;
    }

    octaspire_helpers_verify_true(
        textOrErrorFilter.index >= 0 && textOrErrorFilter.index <= 6);

    nk_plugin_filter filter = nk_filter_default;

    switch (textOrErrorFilter.index)
    {
        case 0: { filter = nk_filter_default; } break;
        case 1: { filter = nk_filter_ascii;   } break;
        case 2: { filter = nk_filter_float;   } break;
        case 3: { filter = nk_filter_decimal; } break;
        case 4: { filter = nk_filter_hex;     } break;
        case 5: { filter = nk_filter_oct;     } break;
        case 6: { filter = nk_filter_binary;  } break;
    }

    char * memory = octaspire_allocator_malloc(
        octaspire_dern_vm_get_allocator(vm),
        len);

    octaspire_helpers_verify_not_null(memory);

    strncpy(
        memory,
        octaspire_dern_value_as_string_get_c_string(stringOrError.value),
        len);

    int memused = octaspire_dern_value_as_string_get_length_in_octets(
        stringOrError.value);

    // Show the widget.

    nk_flags const resultFlags = nk_edit_string(
        ctx,
        editType,
        memory,
        &memused,
        len,
        filter);

    if (memused < len)
    {
        memory[memused] = '\0';
    }

    octaspire_helpers_verify_true(
        octaspire_dern_value_as_text_set_c_string(
            stringOrError.value,
            memory));

    octaspire_allocator_free(octaspire_dern_vm_get_allocator(vm), memory);

    memory = 0;

    octaspire_string_t * const result = octaspire_string_new(
        "",
        octaspire_dern_vm_get_allocator(vm));

    octaspire_helpers_verify_not_null(result);

    if (resultFlags)
    {
        if (resultFlags & NK_EDIT_ACTIVE)
        {
            octaspire_string_concatenate_c_string(result, "ACTIVE ");
        }

        if (resultFlags & NK_EDIT_INACTIVE)
        {
            octaspire_string_concatenate_c_string(result, "INACTIVE ");
        }

        if (resultFlags & NK_EDIT_ACTIVATED)
        {
            octaspire_string_concatenate_c_string(result, "ACTIVATED ");
        }

        if (resultFlags & NK_EDIT_DEACTIVATED)
        {
            octaspire_string_concatenate_c_string(result, "DEACTIVATED ");
        }

        if (resultFlags & NK_EDIT_COMMITED)
        {
            octaspire_string_concatenate_c_string(result, "COMMITED ");
        }
    }

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_string(vm, result);
}

octaspire_dern_value_t *dern_nuklear_edit_focus(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName    = "nuklear-edit-focus";
    char   const * const ctxName         = "ctx";
    size_t const         numExpectedArgs = 2;

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != numExpectedArgs)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects %zu arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numExpectedArgs,
            numArgs);
    }

    // ctx

    octaspire_dern_c_data_or_unpushed_error_t cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error(
            arguments,
            0,
            dernFuncName,
            ctxName,
            DERN_NUKLEAR_PLUGIN_NAME);

    if (cDataOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrError.unpushedError;
    }

    struct nk_context * const ctx = cDataOrError.cData;

    octaspire_helpers_verify_not_null(ctx);

    // focus

    octaspire_dern_boolean_or_unpushed_error_const_t boolOrError =
        octaspire_dern_value_as_vector_get_element_at_as_boolean_or_unpushed_error_const(
            arguments,
            1,
            dernFuncName);

    if (boolOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return boolOrError.unpushedError;
    }

    bool const focus = boolOrError.boolean;

    // Apply the requested focusing.

    if (focus)
    {
        nk_edit_focus(ctx, NK_EDIT_ALWAYS_INSERT_MODE);
    }
    else
    {
        nk_edit_unfocus(ctx);
    }

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

octaspire_dern_value_t *dern_nuklear_progress(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName    = "nuklear-progress";
    char   const * const ctxName         = "ctx";
    size_t const         numExpectedArgs = 4;

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != numExpectedArgs)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects %zu arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numExpectedArgs,
            numArgs);
    }

    // ctx

    octaspire_dern_c_data_or_unpushed_error_t cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error(
            arguments,
            0,
            dernFuncName,
            ctxName,
            DERN_NUKLEAR_PLUGIN_NAME);

    if (cDataOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrError.unpushedError;
    }

    struct nk_context * const ctx = cDataOrError.cData;

    octaspire_helpers_verify_not_null(ctx);

    octaspire_dern_value_t const * const secondArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(secondArg);

    if (!octaspire_dern_value_is_number(secondArg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects number as the second "
            "argument. Type '%s' was given.",
            dernFuncName,
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }

    octaspire_dern_value_t const * const thirdArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 2);

    octaspire_helpers_verify_not_null(thirdArg);

    if (!octaspire_dern_value_is_number(thirdArg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects number as the third "
            "argument. Type '%s' was given.",
            dernFuncName,
            octaspire_dern_value_helper_get_type_as_c_string(thirdArg->typeTag));
    }

    octaspire_dern_value_t const * const fourthArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 3);

    octaspire_helpers_verify_not_null(fourthArg);

    if (!octaspire_dern_value_is_boolean(fourthArg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects boolean as the fourth "
            "argument. Type '%s' was given.",
            dernFuncName,
            octaspire_dern_value_helper_get_type_as_c_string(fourthArg->typeTag));
    }

    nk_size current = (nk_size)octaspire_dern_value_as_number_get_value(secondArg);

    nk_progress(
        ctx,
        &current,
        (nk_size)octaspire_dern_value_as_number_get_value(thirdArg),
        octaspire_dern_value_as_boolean_get_value(fourthArg) ? NK_MODIFIABLE : NK_FIXED);

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_integer(
        vm,
        (int32_t)current);
}

octaspire_dern_value_t *dern_nuklear_layout_row_dynamic(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName    = "nuklear-layout-row-dynamic";
    char   const * const ctxName         = "ctx";
    size_t const         numExpectedArgs = 3;

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != numExpectedArgs)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects %zu arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numExpectedArgs,
            numArgs);
    }

    // ctx

    octaspire_dern_c_data_or_unpushed_error_t cDataOrErrorCtx =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error(
            arguments,
            0,
            dernFuncName,
            ctxName,
            DERN_NUKLEAR_PLUGIN_NAME);

    if (cDataOrErrorCtx.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrErrorCtx.unpushedError;
    }

    struct nk_context * const ctx = cDataOrErrorCtx.cData;

    octaspire_helpers_verify_not_null(ctx);


    // row-height

    octaspire_dern_number_or_unpushed_error_const_t numberOrErrorRowHeight =
        octaspire_dern_value_as_vector_get_element_at_as_number_or_unpushed_error_const(
            arguments,
            1,
            dernFuncName);

    if (numberOrErrorRowHeight.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return numberOrErrorRowHeight.unpushedError;
    }

    double const height = numberOrErrorRowHeight.number;

    // num-columns

    octaspire_dern_number_or_unpushed_error_const_t numberOrErrorNumColumns =
        octaspire_dern_value_as_vector_get_element_at_as_number_or_unpushed_error_const(
            arguments,
            2,
            dernFuncName);

    if (numberOrErrorNumColumns.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return numberOrErrorNumColumns.unpushedError;
    }

    int const columns = numberOrErrorNumColumns.number;

    nk_layout_row_dynamic(ctx, height, columns);

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

octaspire_dern_value_t *dern_nuklear_layout_row_static(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName    = "nuklear-layout-row-static";
    char   const * const ctxName         = "ctx";
    size_t const         numExpectedArgs = 4;

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != numExpectedArgs)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects %zu arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numExpectedArgs,
            numArgs);
    }

    // ctx

    octaspire_dern_c_data_or_unpushed_error_t cDataOrErrorCtx =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error(
            arguments,
            0,
            dernFuncName,
            ctxName,
            DERN_NUKLEAR_PLUGIN_NAME);

    if (cDataOrErrorCtx.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrErrorCtx.unpushedError;
    }

    struct nk_context * const ctx = cDataOrErrorCtx.cData;

    octaspire_helpers_verify_not_null(ctx);


    // height

    octaspire_dern_number_or_unpushed_error_const_t numberOrErrorHeight =
        octaspire_dern_value_as_vector_get_element_at_as_number_or_unpushed_error_const(
            arguments,
            1,
            dernFuncName);

    if (numberOrErrorHeight.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return numberOrErrorHeight.unpushedError;
    }

    double const height = numberOrErrorHeight.number;

    // item-width

    octaspire_dern_number_or_unpushed_error_const_t numberOrErrorItemWidth =
        octaspire_dern_value_as_vector_get_element_at_as_number_or_unpushed_error_const(
            arguments,
            2,
            dernFuncName);

    if (numberOrErrorItemWidth.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return numberOrErrorItemWidth.unpushedError;
    }

    int const item_width = numberOrErrorItemWidth.number;

    // columns

    octaspire_dern_number_or_unpushed_error_const_t numberOrErrorColumns =
        octaspire_dern_value_as_vector_get_element_at_as_number_or_unpushed_error_const(
            arguments,
            3,
            dernFuncName);

    if (numberOrErrorColumns.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return numberOrErrorColumns.unpushedError;
    }

    int const columns = numberOrErrorColumns.number;

    nk_layout_row_static(ctx, height, item_width, columns);

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

octaspire_dern_value_t *dern_nuklear_layout_row(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName    = "nuklear-layout-row";
    char   const * const ctxName         = "ctx";
    size_t const         numExpectedArgs = 5;

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs < numExpectedArgs)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects %zu arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numExpectedArgs,
            numArgs);
    }

    // ctx

    octaspire_dern_c_data_or_unpushed_error_t cDataOrErrorCtx =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error(
            arguments,
            0,
            dernFuncName,
            ctxName,
            DERN_NUKLEAR_PLUGIN_NAME);

    if (cDataOrErrorCtx.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrErrorCtx.unpushedError;
    }

    struct nk_context * const ctx = cDataOrErrorCtx.cData;

    octaspire_helpers_verify_not_null(ctx);

    // format

    char const * const alternatives[] =
    {
        "DYNAMIC",
        "STATIC",
        0
    };

    octaspire_dern_one_of_texts_or_unpushed_error_const_t textOrError =
        octaspire_dern_value_as_vector_get_element_at_as_one_of_texts_or_unpushed_error_const(
            arguments,
            1,
            dernFuncName,
            alternatives);

    if (textOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));
        return textOrError.unpushedError;
    }

    octaspire_helpers_verify_true(
        textOrError.index == 0 || textOrError.index == 1);

    octaspire_string_t const * const format = textOrError.text;

    octaspire_helpers_verify_not_null(format);

    // height

    octaspire_dern_number_or_unpushed_error_const_t numberOrErrorHeight =
        octaspire_dern_value_as_vector_get_element_at_as_number_or_unpushed_error_const(
            arguments,
            2,
            dernFuncName);

    if (numberOrErrorHeight.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return numberOrErrorHeight.unpushedError;
    }

    double const height = numberOrErrorHeight.number;

    // columns

    octaspire_dern_number_or_unpushed_error_const_t numberOrErrorColumns =
        octaspire_dern_value_as_vector_get_element_at_as_number_or_unpushed_error_const(
            arguments,
            3,
            dernFuncName);

    if (numberOrErrorColumns.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return numberOrErrorColumns.unpushedError;
    }

    int const columns = numberOrErrorColumns.number;

    if (columns <= 0)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects non-negative number as the fourth argument. "
            "Number %d was given.",
            dernFuncName,
            columns);
    }

    float * ratios = octaspire_allocator_malloc(
        octaspire_dern_vm_get_allocator(vm),
        sizeof(float) * columns);

    octaspire_helpers_verify_not_null(ratios);

    double ratio          = 0;
    size_t numRatiosAdded = 0;

    for (size_t i = 4; (i < numArgs) && (numRatiosAdded < (size_t)columns); ++i)
    {
        octaspire_dern_number_or_unpushed_error_const_t numberOrErrorRatio =
            octaspire_dern_value_as_vector_get_element_at_as_number_or_unpushed_error_const(
                arguments,
                i,
                dernFuncName);

        if (numberOrErrorRatio.unpushedError)
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return numberOrErrorRatio.unpushedError;
        }

        ratio = numberOrErrorRatio.number;
        ratios[i-4] = ratio;
        ++numRatiosAdded;
    }

    // If not given all the required ratios, fill in the rest of
    // the ratios with the last given one.

    size_t const numRatiosToAdd = columns - numRatiosAdded;

    for (size_t i = 0; i < numRatiosToAdd; ++i)
    {
        ratios[numRatiosAdded + i] = ratio;
    }

    nk_layout_row(
        ctx,
        textOrError.index == 0 ? NK_DYNAMIC : NK_STATIC,
        height,
        columns,
        ratios);

    octaspire_allocator_free(
        octaspire_dern_vm_get_allocator(vm),
        ratios);

    ratios = 0;

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

octaspire_dern_value_t *dern_nuklear_end(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName    = "nuklear-end";
    char   const * const ctxName         = "ctx";
    size_t const         numExpectedArgs = 1;

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != numExpectedArgs)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects %zu arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numExpectedArgs,
            numArgs);
    }

    // ctx

    octaspire_dern_c_data_or_unpushed_error_t cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error(
            arguments,
            0,
            dernFuncName,
            ctxName,
            DERN_NUKLEAR_PLUGIN_NAME);

    if (cDataOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrError.unpushedError;
    }

    struct nk_context * const ctx = cDataOrError.cData;

    octaspire_helpers_verify_not_null(ctx);

    nk_end(ctx);

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

octaspire_dern_value_t *dern_nuklear_input_begin(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName    = "nuklear-input-begin";
    char   const * const ctxName         = "ctx";
    size_t const         numExpectedArgs = 1;

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != numExpectedArgs)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects %zu arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numExpectedArgs,
            numArgs);
    }

    // ctx

    octaspire_dern_c_data_or_unpushed_error_t cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error(
            arguments,
            0,
            dernFuncName,
            ctxName,
            DERN_NUKLEAR_PLUGIN_NAME);

    if (cDataOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrError.unpushedError;
    }

    struct nk_context * const ctx = cDataOrError.cData;

    octaspire_helpers_verify_not_null(ctx);

    nk_input_begin(ctx);

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

octaspire_dern_value_t *dern_nuklear_sdl_handle_event(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName    = "nuklear-sdl-handle-event";
    char   const * const ctxName         = "ctx";
    char   const * const eventName       = "event";
    char   const * const sdl2PluginName  = "dern_sdl2";
    size_t const         numExpectedArgs = 2;

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != numExpectedArgs)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects %zu arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numExpectedArgs,
            numArgs);
    }

    // ctx

    octaspire_dern_c_data_or_unpushed_error_t cDataOrErrorCtx =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error(
            arguments,
            0,
            dernFuncName,
            ctxName,
            DERN_NUKLEAR_PLUGIN_NAME);

    if (cDataOrErrorCtx.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrErrorCtx.unpushedError;
    }

    struct nk_context * const ctx = cDataOrErrorCtx.cData;

    octaspire_helpers_verify_not_null(ctx);

    // SDL2 event

    octaspire_dern_c_data_or_unpushed_error_const_t cDataOrErrorEvent =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
            arguments,
            1,
            dernFuncName,
            eventName,
            sdl2PluginName);

    if (cDataOrErrorEvent.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrErrorEvent.unpushedError;
    }

    // TODO actually sdl2_allocation_context_t
    dern_nuklear_allocation_context_t const * const context =
        cDataOrErrorEvent.cData;

    SDL_Event * const event = context->payload;

    nk_sdl_handle_event(event);

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

octaspire_dern_value_t *dern_nuklear_input_end(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName    = "nuklear-input-end";
    char   const * const ctxName         = "ctx";
    size_t const         numExpectedArgs = 1;

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != numExpectedArgs)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects %zu arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numExpectedArgs,
            numArgs);
    }

    // ctx

    octaspire_dern_c_data_or_unpushed_error_t cDataOrError =
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error(
            arguments,
            0,
            dernFuncName,
            ctxName,
            DERN_NUKLEAR_PLUGIN_NAME);

    if (cDataOrError.unpushedError)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return cDataOrError.unpushedError;
    }

    struct nk_context * const ctx = cDataOrError.cData;

    octaspire_helpers_verify_not_null(ctx);

    nk_input_end(ctx);

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

octaspire_dern_value_t *dern_nuklear_sdl_render(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName    = "nuklear-sdl-render";
    size_t const         numExpectedArgs = 0;

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != numExpectedArgs)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects %zu arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numExpectedArgs,
            numArgs);
    }

    nk_sdl_render(NK_ANTI_ALIASING_ON);

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

octaspire_dern_value_t *dern_nuklear_rgba(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const         stackLength  = octaspire_dern_vm_get_stack_length(vm);
    char   const * const dernFuncName = "nuklear-rgba";
    char   const * const nkColorName  = "nk_color";

    size_t const         numExpectedArgs = 4;

    size_t const numArgs =
        octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != numExpectedArgs)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' expects %zu arguments. "
            "%zu arguments were given.",
            dernFuncName,
            numExpectedArgs,
            numArgs);
    }

    int numbers[4] = {0};
    size_t const numNumbers = sizeof(numbers) / sizeof(numbers[0]);

    for (size_t i = 0; i < numNumbers; ++i)
    {
        octaspire_dern_number_or_unpushed_error_const_t const numberOrError =
            octaspire_dern_value_as_vector_get_element_at_as_number_or_unpushed_error_const(
                arguments,
                i,
                dernFuncName);

        if (numberOrError.unpushedError)
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return numberOrError.unpushedError;
        }

        numbers[i] = numberOrError.number;
    }

    struct nk_color *color = octaspire_allocator_malloc(
        octaspire_dern_vm_get_allocator(vm),
        sizeof(struct nk_color));

    if (!color)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' failed to allocate memory.",
            dernFuncName);
    }

    color->r = numbers[0];
    color->g = numbers[1];
    color->b = numbers[2];
    color->a = numbers[3];

    dern_nuklear_allocation_context_t * const context = octaspire_allocator_malloc(
        octaspire_dern_vm_get_allocator(vm),
        sizeof(dern_nuklear_allocation_context_t));

    if (!context)
    {
        octaspire_allocator_free(octaspire_dern_vm_get_allocator(vm), color);
        color = 0;

        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin '%s' failed to allocate memory for a nk_color context.",
            dernFuncName);
    }

    context->vm      = vm;
    context->payload = color;

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_NUKLEAR_PLUGIN_NAME,
        nkColorName,
        "dern_nuklear_nk_color_clean_up_callback",
        "",
        "",
        "",
        "", //"dern_nuklear_to_string",
        "", //"dern_nuklear_compare",
        false,
        context);
}

bool dern_nuklear_init(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_environment_t * const targetEnv,
    char const * const libName)
{
    octaspire_helpers_verify_true(vm && targetEnv && libName);

    dern_nuklear_private_lib_name = octaspire_string_new(
        libName,
        octaspire_dern_vm_get_allocator(vm));

    if (!dern_nuklear_private_lib_name)
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "nuklear-sdl-init",
            dern_nuklear_sdl_init,
            1,
            "NAME\n"
            "\tnuklear-sdl-init\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_nuklear)\n"
            "\n"
            "\t(nuklear-sdl-init window) -> ctx or error\n"
            "\n"
            "DESCRIPTION\n"
            "\tCreates a new nuklear context for the given SDL2 window\n"
            "\n"
            "ARGUMENTS\n"
            "\twindow           SDL2 window from dern_sdl2 library.\n"
            "\n"
            "RETURN VALUE\n"
            "\tNuklear context or error if something went wrong\n"
            "\n"
            "SEE ALSO\n"
            "\tnuklear-sdl-shutdown\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "nuklear-begin",
            dern_nuklear_begin,
            6,
            "NAME\n"
            "\tnuklear-begin\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_nuklear)\n"
            "\n"
            "\t(nuklear-begin ctx title x y w h flags...]) -> true or false\n"
            "\n"
            "DESCRIPTION\n"
            "\tStarts a new window. Must be called every frame unless\n"
            "\twindow is hidden.\n"
            "\n"
            "ARGUMENTS\n"
            "\tctx           nuklear context.\n"
            "\ttitle         window title and identifier.\n"
            "\tx             x component of initial position\n"
            "\ty             y component of initial position\n"
            "\tw             initial width\n"
            "\th             initial height\n"
            "\tflags...      optional window flags.\n"
            "\n"
            "RETURN VALUE\n"
            "\ttrue if window can be filled with widgets, false otherwise\n"
            "\n"
            "SEE ALSO\n"
            "\tnuklear-end\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "nuklear-label",
            dern_nuklear_label,
            4,
            "NAME\n"
            "\tnuklear-label\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_nuklear)\n"
            "\n"
            "\t(nuklear-label ctx text alignment <color>) -> true or error\n"
            "\n"
            "DESCRIPTION\n"
            "\tDisplay a text label\n"
            "\n"
            "ARGUMENTS\n"
            "\tctx           nuklear context.\n"
            "\ttext          text to be shown.\n"
            "\talignment     LEFT, CENTERED or RIGHT.\n"
            "\tcolor         optional color.\n"
            "\n"
            "RETURN VALUE\n"
            "\ttrue or error if something went wrong.\n"
            "\n"
            "SEE ALSO\n"
            "\tnuklear-begin\n"
            "\tnuklear-label-wrap\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "nuklear-label-wrap",
            dern_nuklear_label_wrap,
            3,
            "NAME\n"
            "\tnuklear-label-wrap\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_nuklear)\n"
            "\n"
            "\t(nuklear-label-wrap ctx text <color>) -> true or error\n"
            "\n"
            "DESCRIPTION\n"
            "\tDisplay a text label with wrapped text.\n"
            "\n"
            "ARGUMENTS\n"
            "\tctx           nuklear context.\n"
            "\ttext          text to be shown.\n"
            "\tcolor         optional color.\n"
            "\n"
            "RETURN VALUE\n"
            "\ttrue or error if something went wrong.\n"
            "\n"
            "SEE ALSO\n"
            "\tnuklear-begin\n"
            "\tnuklear-label\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "nuklear-button-label",
            dern_nuklear_button_label,
            2,
            "NAME\n"
            "\tnuklear-button-label\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_nuklear)\n"
            "\n"
            "\t(nuklear-button-label ctx text) -> true/false or error\n"
            "\n"
            "DESCRIPTION\n"
            "\tDisplay a button label\n"
            "\n"
            "ARGUMENTS\n"
            "\tctx           nuklear context.\n"
            "\ttext          text to be shown.\n"
            "\n"
            "RETURN VALUE\n"
            "\ttrue/false or error if something went wrong.\n"
            "\n"
            "SEE ALSO\n"
            "\tnuklear-begin\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "nuklear-checkbox-label",
            dern_nuklear_checkbox_label,
            3,
            "NAME\n"
            "\tnuklear-checkbox-label\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_nuklear)\n"
            "\n"
            "\t(nuklear-checkbox-label ctx text active) -> true/false or error\n"
            "\n"
            "DESCRIPTION\n"
            "\tDisplay a checkbox label\n"
            "\n"
            "ARGUMENTS\n"
            "\tctx           nuklear context.\n"
            "\ttext          text to be shown.\n"
            "\tactive        the state before and after.\n"
            "\n"
            "RETURN VALUE\n"
            "\ttrue if checkbox was toggled, false otherwise "
            "\t(or error if something went wrong).\n"
            "\n"
            "SEE ALSO\n"
            "\tnuklear-begin\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "nuklear-slider-int",
            dern_nuklear_slider_int,
            5,
            "NAME\n"
            "\tnuklear-slider-int\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_nuklear)\n"
            "\n"
            "\t(nuklear-slider-int ctx min val max step) -> true/false or error\n"
            "\n"
            "DESCRIPTION\n"
            "\tDisplay an integer slider.\n"
            "\n"
            "ARGUMENTS\n"
            "\tctx           nuklear context.\n"
            "\tmin           smallest possible value.\n"
            "\tval           selected value.\n"
            "\tmax           largest possible value.\n"
            "\tstep          amount of change per step.\n"
            "\n"
            "RETURN VALUE\n"
            "\ttrue if slider value was changed, false otherwise "
            "\t(or error if something went wrong).\n"
            "\n"
            "SEE ALSO\n"
            "\tnuklear-begin\n"
            "\tnuklear-slider-float\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "nuklear-slider-float",
            dern_nuklear_slider_float,
            5,
            "NAME\n"
            "\tnuklear-slider-float\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_nuklear)\n"
            "\n"
            "\t(nuklear-slider-float ctx min val max step) -> true/false or error\n"
            "\n"
            "DESCRIPTION\n"
            "\tDisplay a floating point slider.\n"
            "\n"
            "ARGUMENTS\n"
            "\tctx           nuklear context.\n"
            "\tmin           smallest possible value.\n"
            "\tval           selected value.\n"
            "\tmax           largest possible value.\n"
            "\tstep          amount of change per step.\n"
            "\n"
            "RETURN VALUE\n"
            "\ttrue if slider value was changed, false otherwise "
            "\t(or error if something went wrong).\n"
            "\n"
            "SEE ALSO\n"
            "\tnuklear-begin\n"
            "\tnuklear-slider-int\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "nuklear-option-label",
            dern_nuklear_option_label,
            4,
            "NAME\n"
            "\tnuklear-option-label\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_nuklear)\n"
            "\n"
            "\t(nuklear-option-label ctx label active) -> boolean or error\n"
            "\n"
            "DESCRIPTION\n"
            "\tDisplay a radio button\n"
            "\n"
            "ARGUMENTS\n"
            "\tctx           nuklear context.\n"
            "\tlabel         text label for the option.\n"
            "\tid            symbol or string identifying this option.\n"
            "\tactive-id     symbol identifying currently active option.\n"
            "\n"
            "RETURN VALUE\n"
            "\ttrue if this option was selected, false otherwise "
            "\t(or error if something went wrong).\n"
            "\n"
            "SEE ALSO\n"
            "\tnuklear-begin\n"
            "\tnuklear-checkbox-label\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "nuklear-progress",
            dern_nuklear_progress,
            4,
            "NAME\n"
            "\tnuklear-progress\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_nuklear)\n"
            "\n"
            "\t(nuklear-progress ctx current max modifyable) -> number or error\n"
            "\n"
            "DESCRIPTION\n"
            "\tDisplay a progress bar\n"
            "\n"
            "ARGUMENTS\n"
            "\tctx           nuklear context.\n"
            "\tcurrent       current value.\n"
            "\tmax           maximum value.\n"
            "\tmodifyable    can the value be changed by user.\n"
            "\n"
            "RETURN VALUE\n"
            "\tnumber of the current (possibly user modified) value\n"
            "\tor error if something went wrong.\n"
            "\n"
            "SEE ALSO\n"
            "\tnuklear-begin\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "nuklear-edit-string",
            dern_nuklear_edit_string,
            5,
            "NAME\n"
            "\tnuklear-edit-string\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_nuklear)\n"
            "\n"
            "\t(nuklear-edit-string ctx edit-type str len filter) -> string or error\n"
            "\n"
            "DESCRIPTION\n"
            "\tDisplay a simple text input with the given maximum length.\n"
            "\n"
            "ARGUMENTS\n"
            "\tctx           nuklear context.\n"
            "\tedit-type     One of symbols: SIMPLE, FIELD, BOX or EDITOR."
            "\tstr           edit result.\n"
            "\tlen           max allowed input length.\n"
            "\tfilter        text (string or symbol) describing allowed input.\n"
            "\t              Must be one of: DEFAULT, ASCII, FLOAT, DECIMAL,\n"
            "\t              HEX, OCT or BINARY.\n"
            "\n"
            "RETURN VALUE\n"
            "\tString that is empty or contains status information.\n"
            "\tPossible values are: ACTIVE, INACTIVE, ACTIVATED, DEACTIVATED\n"
            "\tand COMMITED. Error is returned if something went wrong.\n"
            "\n"
            "SEE ALSO\n"
            "\tnuklear-begin\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "nuklear-edit-focus",
            dern_nuklear_edit_focus,
            2,
            "NAME\n"
            "\tnuklear-edit-focus\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_nuklear)\n"
            "\n"
            "\t(nuklear-edit-focus ctx focus) -> true or error\n"
            "\n"
            "DESCRIPTION\n"
            "\tSet focus of the next text entry programmatically.\n"
            "\n"
            "ARGUMENTS\n"
            "\tctx           nuklear context.\n"
            "\tfocus         boolean value for the focus."
            "\n"
            "RETURN VALUE\n"
            "\ttrue or error if something went wrong.\n"
            "\n"
            "SEE ALSO\n"
            "\tnuklear-begin\n"
            "\tnuklear-edit-string\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "nuklear-layout-row-dynamic",
            dern_nuklear_layout_row_dynamic,
            3,
            "NAME\n"
            "\tnuklear-layout-row-dynamic\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_nuklear)\n"
            "\n"
            "\t(nuklear-row-dynamic ctx row-height num-columns) -> true or error\n"
            "\n"
            "DESCRIPTION\n"
            "\tDivide current layout into 'num-columns' of same sized\n"
            "\tgrowing columns.\n"
            "\n"
            "ARGUMENTS\n"
            "\tctx           nuklear context.\n"
            "\trow-height    height of the row.\n"
            "\tnum-columns   number of columns.\n"
            "\n"
            "RETURN VALUE\n"
            "\ttrue or error if something went wrong.\n"
            "\n"
            "SEE ALSO\n"
            "\tnuklear-begin\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "nuklear-layout-row-static",
            dern_nuklear_layout_row_static,
            4,
            "NAME\n"
            "\tnuklear-layout-row-static\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_nuklear)\n"
            "\n"
            "\t(nuklear-row-static ctx height item-width cols) -> true or error\n"
            "\n"
            "DESCRIPTION\n"
            "\tFill current row with 'cols' number of widgets.\n"
            "\n"
            "ARGUMENTS\n"
            "\tctx           nuklear context.\n"
            "\theight        height of the row.\n"
            "\titem-width    width of item.\n"
            "\tcols          number of columns.\n"
            "\n"
            "RETURN VALUE\n"
            "\ttrue or error if something went wrong.\n"
            "\n"
            "SEE ALSO\n"
            "\tnuklear-begin\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "nuklear-layout-row",
            dern_nuklear_layout_row,
            5,
            "NAME\n"
            "\tnuklear-layout-row\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_nuklear)\n"
            "\n"
            "\t(nuklear-layout-row ctx format height columns ratio...) -> true or error\n"
            "\n"
            "DESCRIPTION\n"
            "\tSpecify row columns.\n"
            "\n"
            "ARGUMENTS\n"
            "\tctx           nuklear context.\n"
            "\tformat        DYNAMIC or STATIC.\n"
            "\theight        height or zero for auto layout.\n"
            "\tcolumns       number of widgets on the row.\n"
            "\tratio...      'column' or less ratios, last repeated, if needed.\n"
            "\n"
            "RETURN VALUE\n"
            "\ttrue or error if something went wrong.\n"
            "\n"
            "SEE ALSO\n"
            "\tnuklear-begin\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "nuklear-end",
            dern_nuklear_end,
            1,
            "NAME\n"
            "\tnuklear-end\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_nuklear)\n"
            "\n"
            "\t(nuklear-end ctx) -> true or error message\n"
            "\n"
            "DESCRIPTION\n"
            "\tNeeds to be called in the end of window building process.\n"
            "\n"
            "ARGUMENTS\n"
            "\tctx           nuklear context.\n"
            "\n"
            "RETURN VALUE\n"
            "\ttrue or error if something went wrong.\n"
            "\n"
            "SEE ALSO\n"
            "\tnuklear-begin\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "nuklear-input-begin",
            dern_nuklear_input_begin,
            1,
            "NAME\n"
            "\tnuklear-input-begin\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_nuklear)\n"
            "\n"
            "\t(nuklear-input-begin ctx) -> true or error message\n"
            "\n"
            "DESCRIPTION\n"
            "\tBegin input mirroring process. Needs to be called before\n"
            "\tall the other nuklear-input-xxx calls.\n"
            "\n"
            "ARGUMENTS\n"
            "\tctx           nuklear context.\n"
            "\n"
            "RETURN VALUE\n"
            "\ttrue or error if something went wrong.\n"
            "\n"
            "SEE ALSO\n"
            "\tnuklear-input-end\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "nuklear-sdl-handle-event",
            dern_nuklear_sdl_handle_event,
            2,
            "NAME\n"
            "\tnuklear-sdl-handle-event\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_nuklear)\n"
            "\n"
            "\t(nuklear-sdl-handle-event ctx event) -> true or error\n"
            "\n"
            "DESCRIPTION\n"
            "\tHandle event returned by SDL2 library.\n"
            "\n"
            "ARGUMENTS\n"
            "\tctx           nuklear context.\n"
            "\tevent         SDL2 library event.\n"
            "\n"
            "RETURN VALUE\n"
            "\ttrue or error if something went wrong.\n"
            "\n"
            "SEE ALSO\n"
            "\tnuklear-input-begin\n"
            "\tnuklear-input-end\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "nuklear-input-end",
            dern_nuklear_input_end,
            1,
            "NAME\n"
            "\tnuklear-input-end\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_nuklear)\n"
            "\n"
            "\t(nuklear-input-end ctx) -> true or error message\n"
            "\n"
            "DESCRIPTION\n"
            "\tEnd input mirroring process.\n"
            "\n"
            "ARGUMENTS\n"
            "\tctx           nuklear context.\n"
            "\n"
            "RETURN VALUE\n"
            "\ttrue or error if something went wrong.\n"
            "\n"
            "SEE ALSO\n"
            "\tnuklear-input-begin\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "nuklear-sdl-render",
            dern_nuklear_sdl_render,
            0,
            "NAME\n"
            "\tnuklear-sdl-render\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_nuklear)\n"
            "\n"
            "\t(nuklear-sdl-render) -> true\n"
            "\n"
            "DESCRIPTION\n"
            "\tDoes the actual rendering.\n"
            "\n"
            "ARGUMENTS\n"
            "\n"
            "RETURN VALUE\n"
            "\ttrue.\n"
            "\n"
            "SEE ALSO\n"
            "\tnuklear-sdl-init\n",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "nuklear-rgba",
            dern_nuklear_rgba,
            4,
            "NAME\n"
            "\tnuklear-rgba\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_nuklear)\n"
            "\n"
            "\t(nuklear-rgba r g b a) -> true\n"
            "\n"
            "DESCRIPTION\n"
            "\tCreate new color value.\n"
            "\n"
            "ARGUMENTS\n"
            "\tr             Red   channel 0 - 0xFF.\n"
            "\tg             Green channel 0 - 0xFF.\n"
            "\tb             Blue  channel 0 - 0xFF.\n"
            "\ta             Alpha channel 0 - 0xFF.\n"
            "\n"
            "RETURN VALUE\n"
            "\tNuklear color value to be used with those functions.\n"
            "\tof this library that expect a Nuklear color as an argument.\n"
            "\n"
            "SEE ALSO\n"
            "\n",
            false,
            targetEnv))
    {
        return false;
    }

    return true;
}

