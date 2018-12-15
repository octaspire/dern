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
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
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
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
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

    bool const result = nk_begin(ctx, title, nk_rect(10,450,100,50),
                                 NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|
                                 NK_WINDOW_SCALABLE|NK_WINDOW_MINIMIZABLE|
                                 NK_WINDOW_TITLE);

    octaspire_helpers_verify_true(
        stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_boolean(vm, result);
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
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
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
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
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
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
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

    octaspire_dern_c_data_or_unpushed_error_t cDataOrErrorEvent =
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
    dern_nuklear_allocation_context_t * context = cDataOrErrorEvent.cData;

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
        octaspire_dern_value_as_vector_get_element_at_as_c_data_or_unpushed_error_const(
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
            2,
            "NAME\n"
            "\tnuklear-begin\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_nuklear)\n"
            "\n"
            "\t(nuklear-begin ctx title bounds flags) -> true or false\n"
            "\n"
            "DESCRIPTION\n"
            "\tStarts a new window. Must be called every frame unless\n"
            "\twindow is hidden.\n"
            "\n"
            "ARGUMENTS\n"
            "\tctx           nuklear context.\n"
            "\ttitle         window title and identifier.\n"
            "\tbounds        initial position and size.\n"
            "\tflags         window flags defined in nk_panel_flags.\n"
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

    return true;
}
