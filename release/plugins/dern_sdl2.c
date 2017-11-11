#include "octaspire-dern-amalgamated.c"
#include "SDL.h"

static char const * const DERN_SDL2_PLUGIN_NAME = "dern_sdl2";

void dern_sdl2_window_clean_up_callback(void *payload)
{
    octaspire_helpers_verify_not_null(payload);
    SDL_DestroyWindow((SDL_Window*)payload);
    payload = 0;
}

octaspire_dern_value_t *dern_sdl2_Init(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs < 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-init' expects at least one argument. "
            "%zu arguments were given.",
            numArgs);
    }

    Uint32 flags = 0;

    for (size_t i = 0; i < numArgs; ++i)
    {
        octaspire_dern_value_t const * const flagArg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, i);

        octaspire_helpers_verify_not_null(flagArg);

        if (octaspire_dern_value_is_symbol(flagArg))
        {
            if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "TIMER"))
            {
                flags |= SDL_INIT_TIMER;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "AUDIO"))
            {
                flags |= SDL_INIT_AUDIO;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "VIDEO"))
            {
                flags |= SDL_INIT_VIDEO;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "JOYSTICK"))
            {
                flags |= SDL_INIT_JOYSTICK;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "HAPTIC"))
            {
                flags |= SDL_INIT_HAPTIC;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "GAMECONTROLLER"))
            {
                flags |= SDL_INIT_GAMECONTROLLER;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "EVENTS"))
            {
                flags |= SDL_INIT_EVENTS;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "EVERYTHING"))
            {
                flags |= SDL_INIT_EVERYTHING;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "NOPARACHUTE"))
            {
                flags |= SDL_INIT_NOPARACHUTE;
            }
            else
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'sdl2-init': unknown symbol '%s' as the %zu. argument. ",
                    octaspire_dern_value_as_symbol_get_c_string(flagArg),
                    i + 1);
            }
        }
        else
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-init' expects symbol as the %zu. argument. "
                "Type '%s' was given.",
                i + 1,
                octaspire_dern_value_helper_get_type_as_c_string(flagArg->typeTag));
        }
    }

    if (SDL_Init(flags) < 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-init' failed. Error message is '%s'.",
            SDL_GetError());
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(
        vm,
        true);
}

octaspire_dern_value_t *dern_sdl2_Delay(
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
            "Builtin 'sdl2-Delay' expects exactly one argument. "
            "%zu arguments were given.",
            numArgs);
    }

    Uint32 ms = 0;

    octaspire_dern_value_t const * const firstArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(firstArg);

    if (octaspire_dern_value_is_integer(firstArg))
    {
        ms = (Uint32)octaspire_dern_value_as_integer_get_value(firstArg);
    }
    else if (octaspire_dern_value_is_real(firstArg))
    {
        ms = (Uint32)(octaspire_dern_value_as_real_get_value(firstArg) * 1000.0);
    }
    else
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-Delay' expects integer (ms) or real (s) as the first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    SDL_Delay(ms);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(
        vm,
        true);
}

octaspire_dern_value_t *dern_sdl2_CreateWindow(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs < 5)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-create-window' expects at least five arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_dern_value_t const * const firstArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(firstArg);

    if (!octaspire_dern_value_is_text(firstArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-create-window' expects text for window title as the first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    int x, y, w, h;

    octaspire_dern_value_t const * const secondArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(secondArg);

    if (octaspire_dern_value_is_symbol(secondArg))
    {
        if (octaspire_dern_value_as_symbol_is_equal_to_c_string(secondArg, "CENTERED"))
        {
            x = SDL_WINDOWPOS_CENTERED;
        }
        else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(secondArg, "UNDEFINED"))
        {
            x = SDL_WINDOWPOS_UNDEFINED;
        }
        else
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-create-window' expects symbol "
                "CENTERED or UNDEFINED as the second argument. "
                "Symbol '%s' was given.",
                octaspire_dern_value_as_symbol_get_c_string(secondArg));
        }
    }
    else if (octaspire_dern_value_is_integer(secondArg))
    {
        x = octaspire_dern_value_as_integer_get_value(secondArg);
    }
    else
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-create-window' expects symbol or integer as the second argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }

    octaspire_dern_value_t const * const thirdArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 2);

    octaspire_helpers_verify_not_null(thirdArg);

    if (octaspire_dern_value_is_symbol(thirdArg))
    {
        if (octaspire_dern_value_as_symbol_is_equal_to_c_string(thirdArg, "CENTERED"))
        {
            y = SDL_WINDOWPOS_CENTERED;
        }
        else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(thirdArg, "UNDEFINED"))
        {
            y = SDL_WINDOWPOS_UNDEFINED;
        }
        else
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-create-window' expects symbol "
                "CENTERED or UNDEFINED as the third argument. "
                "Symbol '%s' was given.",
                octaspire_dern_value_as_symbol_get_c_string(thirdArg));
        }
    }
    else if (octaspire_dern_value_is_integer(thirdArg))
    {
        y = octaspire_dern_value_as_integer_get_value(thirdArg);
    }
    else
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-create-window' expects symbol or integer as the third argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(thirdArg->typeTag));
    }

    octaspire_dern_value_t const * const fourthArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 3);

    octaspire_helpers_verify_not_null(fourthArg);

    if (octaspire_dern_value_is_integer(fourthArg))
    {
        w = octaspire_dern_value_as_integer_get_value(fourthArg);
    }
    else
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-create-window' expects integer as the fourth argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(fourthArg->typeTag));
    }

    octaspire_dern_value_t const * const fifthArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 4);

    octaspire_helpers_verify_not_null(fifthArg);

    if (octaspire_dern_value_is_integer(fifthArg))
    {
        h = octaspire_dern_value_as_integer_get_value(fifthArg);
    }
    else
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-create-window' expects integer as the fifth argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(fifthArg->typeTag));
    }

    Uint32 flags = 0;

    for (size_t i = 5; i < numArgs; ++i)
    {
        octaspire_dern_value_t const * const flagArg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, i);

        octaspire_helpers_verify_not_null(flagArg);

        if (octaspire_dern_value_is_symbol(flagArg))
        {
            if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "FULLSCREEN"))
            {
                flags |= SDL_WINDOW_FULLSCREEN;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "FULLSCREEN_DESKTOP"))
            {
                flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "OPENGL"))
            {
                flags |= SDL_WINDOW_OPENGL;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "HIDDEN"))
            {
                flags |= SDL_WINDOW_HIDDEN;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "BORDERLESS"))
            {
                flags |= SDL_WINDOW_BORDERLESS;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "RESIZABLE"))
            {
                flags |= SDL_WINDOW_RESIZABLE;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "MINIMIZED"))
            {
                flags |= SDL_WINDOW_MINIMIZED;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "MAXIMIZED"))
            {
                flags |= SDL_WINDOW_MAXIMIZED;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "INPUT_GRABBED"))
            {
                flags |= SDL_WINDOW_INPUT_GRABBED;
            }
            else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(flagArg, "ALLOW_HIGHDPI"))
            {
                flags |= SDL_WINDOW_ALLOW_HIGHDPI;
            }
            else
            {
                octaspire_helpers_verify_true(
                    stackLength == octaspire_dern_vm_get_stack_length(vm));

                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'sdl2-create-window': unknown symbol '%s' as the %zu. argument. ",
                    octaspire_dern_value_as_symbol_get_c_string(flagArg),
                    i + 1);
            }
        }
        else
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-create-window' expects symbol as the %zu. argument. "
                "Type '%s' was given.",
                i + 1,
                octaspire_dern_value_helper_get_type_as_c_string(flagArg->typeTag));
        }
    }

    SDL_Window * const window = SDL_CreateWindow(
        octaspire_dern_value_as_text_get_c_string(firstArg),
        x,
        y,
        w,
        h,
        flags);

    if (!window)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-create-window' failed to create a window. "
            "Error message is: '%s'.",
            SDL_GetError());
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_SDL2_PLUGIN_NAME,
        "window",
        "dern_sdl2_window_clean_up_callback",
        "",
        "",
        "",
        true,
        window);
}

octaspire_dern_value_t *dern_sdl2_GetWindowSurface(
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
            "Builtin 'sdl2-GetWindowSurface' expects at exactly one argument. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_dern_value_t const * const firstArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(firstArg);

    if (!octaspire_dern_value_is_c_data(firstArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-GetWindowSurface' expects window as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    octaspire_dern_c_data_t * const cData = firstArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData,
            DERN_SDL2_PLUGIN_NAME,
            "window"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-GetWindowSurface' expects 'dern_sdl2' and 'window' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

    SDL_Window * const window = octaspire_dern_c_data_get_payload(cData);

    SDL_Surface * const surface = SDL_GetWindowSurface(window);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    if (!surface)
    {
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-GetWindowSurface' failed: %s",
            SDL_GetError());
    }

    return octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_SDL2_PLUGIN_NAME,
        "surface",
        "",
        "",
        "",
        "",
        true,
        surface);
}

octaspire_dern_value_t *dern_sdl2_GetPixelFormat(
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
            "Builtin 'sdl2-GetPixelFormat' expects exactly one argument. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_dern_value_t const * const firstArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(firstArg);

    if (!octaspire_dern_value_is_c_data(firstArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-GetPixelFormat' expects surface as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    octaspire_dern_c_data_t * const cData = firstArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData,
            DERN_SDL2_PLUGIN_NAME,
            "surface"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-GetPixelFormat' expects 'dern_sdl2' and 'surface' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

    SDL_Surface * const surface = octaspire_dern_c_data_get_payload(cData);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_SDL2_PLUGIN_NAME,
        "pixelFormat",
        "",
        "",
        "",
        "",
        true,
        surface->format);
}

octaspire_dern_value_t *dern_sdl2_FillRect(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 3)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-FillRect' expects three arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_dern_value_t const * const firstArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(firstArg);

    if (!octaspire_dern_value_is_c_data(firstArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-FillRect' expects surface as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    octaspire_dern_c_data_t * const cData = firstArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData,
            DERN_SDL2_PLUGIN_NAME,
            "surface"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-FillRect' expects 'dern_sdl2' and 'surface' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

    SDL_Surface * const surface = octaspire_dern_c_data_get_payload(cData);

    octaspire_dern_value_t const * const thirdArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 2);

    octaspire_helpers_verify_not_null(thirdArg);

    if (!octaspire_dern_value_is_integer(thirdArg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-FillRect' expects integer as third argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(thirdArg->typeTag));
    }

    Uint32 const colorValue = octaspire_dern_value_as_integer_get_value(thirdArg);

    octaspire_dern_value_t const * const secondArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(secondArg);

    int result = 0;

    if (octaspire_dern_value_is_nil(secondArg))
    {
        result = SDL_FillRect(surface, 0, colorValue);
    }
    else if (octaspire_dern_value_is_vector(secondArg))
    {
        SDL_Rect rect;

        octaspire_dern_value_t const * coordinateVal =
            octaspire_dern_value_as_vector_get_element_at_const(secondArg, 0);

        if (!octaspire_dern_value_is_integer(coordinateVal))
        {
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-FillRect' expects integer in coordinate vector at index zero. "
                "Type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(coordinateVal->typeTag));
        }

        rect.x = octaspire_dern_value_as_integer_get_value(coordinateVal);



        coordinateVal =
            octaspire_dern_value_as_vector_get_element_at_const(secondArg, 1);

        if (!octaspire_dern_value_is_integer(coordinateVal))
        {
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-FillRect' expects integer in coordinate vector at index one. "
                "Type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(coordinateVal->typeTag));
        }

        rect.y = octaspire_dern_value_as_integer_get_value(coordinateVal);



        coordinateVal =
            octaspire_dern_value_as_vector_get_element_at_const(secondArg, 2);

        if (!octaspire_dern_value_is_integer(coordinateVal))
        {
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-FillRect' expects integer in coordinate vector at index two. "
                "Type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(coordinateVal->typeTag));
        }

        rect.w = octaspire_dern_value_as_integer_get_value(coordinateVal);



        coordinateVal =
            octaspire_dern_value_as_vector_get_element_at_const(secondArg, 3);

        if (!octaspire_dern_value_is_integer(coordinateVal))
        {
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-FillRect' expects integer in coordinate vector at index three. "
                "Type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(coordinateVal->typeTag));
        }

        rect.h = octaspire_dern_value_as_integer_get_value(coordinateVal);



        result = SDL_FillRect(surface, &rect, colorValue);
    }
    else
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-FillRect' expects nil or vector as second argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    if (result)
    {
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-FillRect' failed: %s",
            SDL_GetError());
    }

    return octaspire_dern_vm_create_new_value_boolean(
        vm,
        true);
}

octaspire_dern_value_t *dern_sdl2_UpdateWindowSurface(
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
            "Builtin 'sdl2-UpdateWindowSurface' expects at exactly one argument. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_dern_value_t const * const firstArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(firstArg);

    if (!octaspire_dern_value_is_c_data(firstArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-UpdateWindowSurface' expects window as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    octaspire_dern_c_data_t * const cData = firstArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData,
            DERN_SDL2_PLUGIN_NAME,
            "window"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-GetWindowSurface' expects 'dern_sdl2' and 'window' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

    SDL_Window * const window = octaspire_dern_c_data_get_payload(cData);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    if (SDL_UpdateWindowSurface(window) < 0)
    {
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-UpdateWindowSurface' failed: %s",
            SDL_GetError());
    }

    return octaspire_dern_vm_create_new_value_boolean(
        vm,
        true);
}

octaspire_dern_value_t *dern_sdl2_MapRGB(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 4)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-MapRGB' expects four arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_dern_value_t const * const firstArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(firstArg);

    if (!octaspire_dern_value_is_c_data(firstArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-MapRGB' expects pixelFormat as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    octaspire_dern_c_data_t * const cData = firstArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData,
            DERN_SDL2_PLUGIN_NAME,
            "pixelFormat"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-MapRGB' expects 'dern_sdl2' and 'pixelFormat' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

    SDL_PixelFormat const * const pixelFormat = octaspire_dern_c_data_get_payload(cData);

    Uint8 colorComponents[3];

    for (size_t i = 1; i < 4; ++i)
    {
        octaspire_dern_value_t const * const arg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, i);

        octaspire_helpers_verify_not_null(arg);

        if (!octaspire_dern_value_is_integer(arg))
        {
            octaspire_helpers_verify_true(
                stackLength == octaspire_dern_vm_get_stack_length(vm));

            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sdl2-MapRGB' expects integer as the %zu. argument. "
                "Type '%s' was given.",
                i + 1,
                octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
        }

        colorComponents[i - 1] = (Uint8)octaspire_dern_value_as_integer_get_value(arg);
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_integer(
        vm,
        SDL_MapRGB(pixelFormat, colorComponents[0], colorComponents[1], colorComponents[2]));
}

octaspire_dern_value_t *dern_sdl2_NumJoysticks(
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
            "Builtin 'sdl2-NumJoysticks' expects no arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    int const result = SDL_NumJoysticks();

    if (result < 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'sdl2-NumJoysticks' failed. "
            "Error message is: '%s'.",
            SDL_GetError());
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_integer(vm, result);
}

octaspire_dern_value_t *dern_sdl2_Quit(
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
            "Builtin 'sdl2-Quit' expects no arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(
        vm,
        true);
}

bool dern_sdl2_init(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_environment_t * const targetEnv)
{
    octaspire_helpers_verify_true(vm && targetEnv);

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-Init",
            dern_sdl2_Init,
            1,
            "(sdl2-Init flags...) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-Delay",
            dern_sdl2_Delay,
            1,
            "(sdl2-Delay msOrSecs) -> true",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-CreateWindow",
            dern_sdl2_CreateWindow,
            5,
            "(sdl2-CreateWindow title, x, y, w, h, optional-flags...) -> <window or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-GetWindowSurface",
            dern_sdl2_GetWindowSurface,
            1,
            "(sdl2-GetWindowSurface window) -> <surface or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-GetPixelFormat",
            dern_sdl2_GetPixelFormat,
            1,
            "(sdl2-GetPixelFormat surface) -> <pixelFormat or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-FillRect",
            dern_sdl2_FillRect,
            1,
            "(sdl2-FillRect surface, <(x, y, w, h) or nil> color) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-UpdateWindowSurface",
            dern_sdl2_UpdateWindowSurface,
            1,
            "(sdl2-UpdateWindowSurface surface) -> <true or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-MapRGB",
            dern_sdl2_MapRGB,
            4,
            "(sdl2-MapRGB pixelFormat, r, g, b) -> integer",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-NumJoysticks",
            dern_sdl2_NumJoysticks,
            0,
            "(sdl2-NumJoysticks) -> integer or error message",
            true,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-Quit",
            dern_sdl2_Quit,
            0,
            "(sdl2-Quit) -> true",
            true,
            targetEnv))
    {
        return false;
    }

    return true;
}

