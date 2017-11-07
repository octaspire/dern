#include "octaspire-dern-amalgamated.c"
#include "SDL.h"

static char const * const DERN_SDL2_PLUGIN_NAME = "dern_sdl2";

void dern_sdl2_window_clean_up_callback(void *payload)
{
    octaspire_helpers_verify_not_null(payload);
    SDL_DestroyWindow((SDL_Window*)payload);
    payload = 0;
}

octaspire_dern_value_t *dern_sdl2_create_window(
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

bool dern_sdl2_init(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_environment_t * const targetEnv)
{
    octaspire_helpers_verify_true(vm && targetEnv);

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sdl2-create-window",
            dern_sdl2_create_window,
            0,
            "(sdl2-create-window title, x, y, w, h, optional-flags...) -> <window or error message>",
            true,
            targetEnv))
    {
        return false;
    }

    return true;
}

