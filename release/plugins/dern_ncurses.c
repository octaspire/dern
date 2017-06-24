#include "octaspire-dern-amalgamated.c"
#include <ctype.h>
#include <locale.h>
#include <wctype.h>

#define NCURSES_WIDECHAR 1
#define _XOPEN_SOURCE_EXTENDED 1

#if defined(__NetBSD__) || defined(__OpenBSD__) || defined(__minix)
#include <curses.h>
#elif defined(__DragonFly__)
#include <ncurses/curses.h>
#elif defined(__sun) && defined(__SVR4)
// Solaris, OpenIndiana
#include <ncurses/curses.h>
#elif defined(__FreeBSD__) || defined(__HAIKU__) || defined(__MidnightBSD__) || defined(__SYLLABLE__)
#include <ncurses.h>
#else
#include <ncursesw/ncurses.h>
#endif

static char const * const DERN_NCURSES_PLUGIN_NAME = "dern_ncurses";

octaspire_dern_value_t *dern_ncurses_initscr(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(arguments);
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    WINDOW * const window = initscr();
    return octaspire_dern_vm_create_new_value_c_data(vm, DERN_NCURSES_PLUGIN_NAME, "window", window);
}


octaspire_dern_value_t *dern_ncurses_getmaxyx(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    WINDOW *window = stdscr;

    if (numArgs > 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'ncurses-getmaxyx' expects zero or one argument. "
            "%zu arguments were given.",
            numArgs);
    }

    if (numArgs == 1)
    {
        octaspire_dern_value_t const * const firstArg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

        octaspire_helpers_verify_not_null(firstArg);

        if (!octaspire_dern_value_is_c_data(firstArg))
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'ncurses-getmaxyx' expects window as first argument. "
                "Type '%s' was given.",
                octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
        }

        octaspire_dern_c_data_t * const cData = firstArg->value.cData;

        if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
                cData,
                DERN_NCURSES_PLUGIN_NAME,
                "window"))
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'ncurses-getmaxyx' expects 'dern_ncurses' and 'window' as "
                "plugin name and payload type name for the C data of the first argument. "
                "Names '%s' and '%s' were given.",
                octaspire_dern_c_data_get_plugin_name(cData),
                octaspire_dern_c_data_get_payload_typename(cData));
        }

        window = octaspire_dern_c_data_get_payload(cData);
    }

    int y = 0;
    int x = 0;

    getmaxyx(window, y, x);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_vector_from_values(
        vm,
        2,
        octaspire_dern_vm_create_new_value_integer(vm, (int32_t)y),
        octaspire_dern_vm_create_new_value_integer(vm, (int32_t)x));
}

octaspire_dern_value_t *dern_ncurses_getch(
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
            "Builtin 'ncurses-getch' expects zero arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    wint_t wint;
    /*int const status =*/get_wch(&wint);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    switch (wint)
    {
        case KEY_BREAK: return octaspire_dern_vm_create_new_value_symbol_from_c_string(vm, "KEY_BREAK");
        case KEY_DOWN:  return octaspire_dern_vm_create_new_value_symbol_from_c_string(vm, "KEY_DOWN");
        case KEY_UP:    return octaspire_dern_vm_create_new_value_symbol_from_c_string(vm, "KEY_UP");
        case KEY_LEFT:  return octaspire_dern_vm_create_new_value_symbol_from_c_string(vm, "KEY_LEFT");
        case KEY_RIGHT: return octaspire_dern_vm_create_new_value_symbol_from_c_string(vm, "KEY_RIGHT");

        // TODO add more special keys

        default:
        {
            return octaspire_dern_vm_create_new_value_character_from_uint32t(vm, (uint32_t)wint);
        }
        break;
    }
}

octaspire_dern_value_t *dern_ncurses_getstr(
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
            "Builtin 'ncurses-getstr' expects zero arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_container_utf8_string_t *resultStr = octaspire_container_utf8_string_new(
        "",
        octaspire_dern_vm_get_allocator(vm));

    octaspire_helpers_verify_not_null(resultStr);

    noecho();

    int y = 0;
    int x = 0;
    getyx(stdscr, y, x);

    while (true)
    {
        wint_t wint;
        /*int const status = */get_wch(&wint);

        if (wint == KEY_ENTER || wint == 10)
        {
            break;
        }
        else if (wint == KEY_BACKSPACE)
        {
            if (octaspire_container_utf8_string_pop_back_ucs_character(resultStr))
            {
                mvwaddstr(
                    stdscr,
                    y,
                    x,
                    octaspire_container_utf8_string_get_c_string(resultStr));

                mvdelch(y, x + octaspire_container_utf8_string_get_length_in_ucs_characters(resultStr));
            }
        }
        else if (iswprint(wint))
        {
            octaspire_container_utf8_string_push_back_ucs_character(resultStr, (uint32_t)wint);

            mvwaddstr(
                stdscr,
                y,
                x,
                octaspire_container_utf8_string_get_c_string(resultStr));

            //move(y, x + octaspire_container_utf8_string_get_length_in_ucs_characters(resultStr));
        }
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_string(vm, resultStr);
}

octaspire_dern_value_t *dern_ncurses_set_raw(
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
            "Builtin 'ncurses-set-raw' expects one argument. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_dern_value_t const * const arg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    if (!octaspire_dern_value_is_boolean(arg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'ncurses-set-raw' expects boolean argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }


    int const result = arg->value.boolean ? raw() : noraw();

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, result != ERR);
}

octaspire_dern_value_t *dern_ncurses_set_cbreak(
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
            "Builtin 'ncurses-set-cbreak' expects one argument. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_dern_value_t const * const arg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    if (!octaspire_dern_value_is_boolean(arg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'ncurses-set-cbreak' expects boolean argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    int const result = arg->value.boolean ? cbreak() : nocbreak();

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, result != ERR);
}

octaspire_dern_value_t *dern_ncurses_set_echo(
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
            "Builtin 'ncurses-set-echo' expects one argument. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_dern_value_t const * const arg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    if (!octaspire_dern_value_is_boolean(arg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'ncurses-set-echo' expects boolean argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    int const result = arg->value.boolean ? echo() : noecho();

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, result != ERR);
}

octaspire_dern_value_t *dern_ncurses_set_nl(
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
            "Builtin 'ncurses-set-nl' expects one argument. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_dern_value_t const * const arg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    if (!octaspire_dern_value_is_boolean(arg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'ncurses-set-nl' expects boolean argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    arg->value.boolean ? nl() : nonl();

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_nil(vm);
}

octaspire_dern_value_t *dern_ncurses_set_cursor(
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
            "Builtin 'ncurses-set-cursor' expects one argument. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_dern_value_t const * const arg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    if (!octaspire_dern_value_is_symbol(arg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'ncurses-set-cursor' expects symbol argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    int result = ERR;

    if (octaspire_dern_value_as_symbol_is_equal_to_c_string(arg, "invisible"))
    {
        result = curs_set(0);

        if (result == ERR)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_string_from_c_string(
                vm,
                "Builtin 'ncurses-set-cursor' failed to make cursor 'invisible'.");
        }
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(arg, "normal"))
    {
        result = curs_set(1);

        if (result == ERR)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_string_from_c_string(
                vm,
                "Builtin 'ncurses-set-cursor' failed to make cursor 'normal'.");
        }
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(arg, "very-visible"))
    {
        result = curs_set(2);

        if (result == ERR)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_string_from_c_string(
                vm,
                "Builtin 'ncurses-set-cursor' failed to make cursor 'very-visible'. ");
        }
    }
    else
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'ncurses-set-cursor' expects symbol argument 'invisible', 'normal' or "
            "'very-visible'. Symbol '%s' was given.",
            octaspire_dern_value_as_symbol_get_c_string(arg));
    }

    octaspire_helpers_verify_true(result == 0 || result == 1 || result == 2);

    if (result == 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_symbol_from_c_string(vm, "invisible");
    }

    if (result == 1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_symbol_from_c_string(vm, "normal");
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_symbol_from_c_string(vm, "very-visible");
}

octaspire_dern_value_t *dern_ncurses_set_keypad(
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
            "Builtin 'ncurses-set-keypad' expects two arguments. "
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
            "Builtin 'ncurses-set-keypad' expects window as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    octaspire_dern_c_data_t * const cData = firstArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData,
            DERN_NCURSES_PLUGIN_NAME,
            "window"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'ncurses-set-keypad' expects 'dern_ncurses' and 'window' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

    octaspire_dern_value_t const * const secondArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(secondArg);

    if (!octaspire_dern_value_is_boolean(secondArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'ncurses-set-keypad' expects boolean as second argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }

    int const result = keypad(octaspire_dern_c_data_get_payload(cData), secondArg->value.boolean);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, result != ERR);
}

octaspire_dern_value_t *dern_ncurses_erase(
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
            "Builtin 'ncurses-erase' expects one argument. "
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
            "Builtin 'ncurses-erase' expects window as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    octaspire_dern_c_data_t * const cData = firstArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData,
            DERN_NCURSES_PLUGIN_NAME,
            "window"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'ncurses-erase' expects 'dern_ncurses' and 'window' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

    int const result = werase(octaspire_dern_c_data_get_payload(cData));

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, result != ERR);
}

octaspire_dern_value_t *dern_ncurses_clear(
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
            "Builtin 'ncurses-clear' expects one argument. "
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
            "Builtin 'ncurses-clear' expects window as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    octaspire_dern_c_data_t * const cData = firstArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData,
            DERN_NCURSES_PLUGIN_NAME,
            "window"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'ncurses-clear' expects 'dern_ncurses' and 'window' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

    int const result = wclear(octaspire_dern_c_data_get_payload(cData));

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, result != ERR);
}

static int dern_ncurses_private_print_symbol(
    octaspire_dern_value_t const * const value,
    WINDOW * const window)
{
    int result = ERR;

    if (octaspire_dern_value_as_symbol_is_equal_to_c_string(value, "ACS_BLOCK"))
    {
        result = waddch(window, ACS_BLOCK);
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(value, "ACS_BOARD"))
    {
        result = waddch(window, ACS_BOARD);
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(value, "ACS_BTEE"))
    {
        result = waddch(window, ACS_BTEE);
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(value, "ACS_BULLET"))
    {
        result = waddch(window, ACS_BULLET);
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(value, "ACS_CKBOARD"))
    {
        result = waddch(window, ACS_CKBOARD);
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(value, "ACS_DARROW"))
    {
        result = waddch(window, ACS_DARROW);
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(value, "ACS_DEGREE"))
    {
        result = waddch(window, ACS_DEGREE);
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(value, "ACS_DIAMOND"))
    {
        result = waddch(window, ACS_DIAMOND);
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(value, "ACS_HLINE"))
    {
        result = waddch(window, ACS_HLINE);
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(value, "ACS_LANTERN"))
    {
        result = waddch(window, ACS_LANTERN);
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(value, "ACS_LARROW"))
    {
        result = waddch(window, ACS_LARROW);
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(value, "ACS_LLCORNER"))
    {
        result = waddch(window, ACS_LLCORNER);
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(value, "ACS_LRCORNER"))
    {
        result = waddch(window, ACS_LRCORNER);
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(value, "ACS_LTEE"))
    {
        result = waddch(window, ACS_LTEE);
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(value, "ACS_PLMINUS"))
    {
        result = waddch(window, ACS_PLMINUS);
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(value, "ACS_PLUS"))
    {
        result = waddch(window, ACS_PLUS);
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(value, "ACS_RARROW"))
    {
        result = waddch(window, ACS_RARROW);
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(value, "ACS_RTEE"))
    {
        result = waddch(window, ACS_RTEE);
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(value, "ACS_S1"))
    {
        result = waddch(window, ACS_S1);
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(value, "ACS_S9"))
    {
        result = waddch(window, ACS_S9);
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(value, "ACS_TTEE"))
    {
        result = waddch(window, ACS_TTEE);
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(value, "ACS_UARROW"))
    {
        result = waddch(window, ACS_UARROW);
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(value, "ACS_ULCORNER"))
    {
        result = waddch(window, ACS_ULCORNER);
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(value, "ACS_URCORNER"))
    {
        result = waddch(window, ACS_URCORNER);
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(value, "ACS_VLINE"))
    {
        result = waddch(window, ACS_VLINE);
    }
    else
    {
        result = wprintw(
            window,
            "%s",
            octaspire_dern_value_as_text_get_c_string(value));
    }

    return result;
}

octaspire_dern_value_t *dern_ncurses_print(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs < 2 || numArgs > 4)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'ncurses-print' expects 2 - 4 arguments. "
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
            "Builtin 'ncurses-print' expects window as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    octaspire_dern_c_data_t * const cData = firstArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData,
            DERN_NCURSES_PLUGIN_NAME,
            "window"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'ncurses-print' expects 'dern_ncurses' and 'window' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

    octaspire_dern_value_t const * const secondArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(secondArg);

    if (numArgs == 2)
    {
        if (!octaspire_dern_value_is_text(secondArg))
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'ncurses-print' expects text (string, character or symbol) as "
                "second argument. Now second argument has type '%s'.",
                octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
        }

        int result = ERR;

        if (octaspire_dern_value_is_symbol(secondArg))
        {
            result = dern_ncurses_private_print_symbol(
                secondArg,
                octaspire_dern_c_data_get_payload(cData));
        }
        else
        {
            result = wprintw(
                octaspire_dern_c_data_get_payload(cData),
                "%s",
                octaspire_dern_value_as_text_get_c_string(secondArg));
        }

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_boolean(vm, result != ERR);
    }
    else if (numArgs == 3)
    {
        if (!octaspire_dern_value_is_integer(secondArg))
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'ncurses-print' expects integer as second argument, when three "
                "arguments are given. Now the second argument has type '%s'.",
                octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
        }

        octaspire_dern_value_t const * const thirdArg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, 2);

        if (!octaspire_dern_value_is_text(thirdArg))
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'ncurses-print' expects text (string, character or symbol) as third "
                "argument when three arguments are given. Now the third argument has type '%s'.",
                octaspire_dern_value_helper_get_type_as_c_string(thirdArg->typeTag));
        }

        int currentY = 0;
        int currentX = 0;

        void * const payload = octaspire_dern_c_data_get_payload(cData);

        getyx((WINDOW * const)payload, currentY, currentX);
        OCTASPIRE_HELPERS_UNUSED_PARAMETER(currentY);

        int result = ERR;

        if (octaspire_dern_value_is_symbol(thirdArg))
        {
            result = wmove(payload, secondArg->value.integer, currentX);

            int const tmpResult = dern_ncurses_private_print_symbol(
                thirdArg,
                payload);

            if (result != ERR)
            {
                result = tmpResult;
            }
        }
        else
        {
            result = mvwprintw(
                payload,
                secondArg->value.integer,
                currentX,
                "%s",
                octaspire_dern_value_as_text_get_c_string(thirdArg));
        }

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_boolean(vm, result != ERR);
    }
    else if (numArgs == 4)
    {
        if (!octaspire_dern_value_is_integer(secondArg))
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'ncurses-print' expects integer as second argument, when four "
                "arguments are given. Now the second argument has type '%s'.",
                octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
        }

        octaspire_dern_value_t const * const thirdArg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, 2);

        if (!octaspire_dern_value_is_integer(thirdArg))
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'ncurses-print' expects integer as third argument, when four "
                "arguments are given. Now the third argument has type '%s'.",
                octaspire_dern_value_helper_get_type_as_c_string(thirdArg->typeTag));
        }

        octaspire_dern_value_t const * const fourthArg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, 3);

        if (!octaspire_dern_value_is_text(fourthArg))
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'ncurses-print' expects text (string, character or symbol) as fourth "
                "argument when four arguments are given. Now the fourth argument has type '%s'.",
                octaspire_dern_value_helper_get_type_as_c_string(fourthArg->typeTag));
        }

        void * const payload = octaspire_dern_c_data_get_payload(cData);
        int result = ERR;

        if (octaspire_dern_value_is_symbol(fourthArg))
        {
            result = wmove(payload, secondArg->value.integer, thirdArg->value.integer);

            int const tmpResult = dern_ncurses_private_print_symbol(
                fourthArg,
                payload);

            if (result != ERR)
            {
                result = tmpResult;
            }
        }
        else
        {
/*
            wint_t wint = (wint_t)
                cchar_t outchar;
                setcchar(&outchar, &wint, 0, 0, 0);
                add_wch(&outchar);
            }

#if 0
            result = mvwins_wstr(
                payload,
                secondArg->value.integer,
                thirdArg->value.integer,
                octaspire_dern_value_as_text_get_c_string(fourthArg));
#endif

            for (size_t i = 0;
                i < octaspire_container_utf8_string_get_length_in_ucs_characters(fourthArg->value.string); ++i)
            {
                uint32_t const ch =
                    octaspire_container_utf8_string_get_ucs_character_at_index(fourthArg->value.string, i);

                int tmpRes = mvwaddch(
                    payload,
                    secondArg->value.integer,
                    thirdArg->value.integer,
                    ch);

                if (tmpRes == ERR)
                {
                    result = tmpRes;
                }
            }
            */

            result = mvwaddstr(
                payload,
                (int)secondArg->value.integer,
                (int)thirdArg->value.integer,
                octaspire_container_utf8_string_get_c_string(fourthArg->value.string));
        }

        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_boolean(vm, result != ERR);
    }

    abort();
}

octaspire_dern_value_t *dern_ncurses_attron(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs == 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'ncurses-attron' expects at least one argument. "
            "%zu arguments were given.",
            numArgs);
    }

    int result = OK;

    for (size_t i = 0; i < numArgs; ++i)
    {
        octaspire_dern_value_t const * const arg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, i);

        octaspire_helpers_verify_not_null(arg);

        if (!octaspire_dern_value_is_symbol(arg) && !octaspire_dern_value_is_integer(arg))
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'ncurses-attron' expects symbol or integer arguments. %zu. argument "
                "has type '%s'.",
                i,
                octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
        }

        int attr = 0;

        if (octaspire_dern_value_is_symbol(arg))
        {
            octaspire_container_utf8_string_t * const symAsStr = arg->value.symbol;

            if (octaspire_container_utf8_string_is_equal_to_c_string(symAsStr, "NORMAL"))
            {
                attr = A_NORMAL;
            }
            else if (octaspire_container_utf8_string_is_equal_to_c_string(symAsStr, "STANDOUT"))
            {
                attr = A_STANDOUT;
            }
            else if (octaspire_container_utf8_string_is_equal_to_c_string(symAsStr, "UNDERLINE"))
            {
                attr = A_UNDERLINE;
            }
            else if (octaspire_container_utf8_string_is_equal_to_c_string(symAsStr, "REVERSE"))
            {
                attr = A_REVERSE;
            }
            else if (octaspire_container_utf8_string_is_equal_to_c_string(symAsStr, "BLINK"))
            {
                attr = A_BLINK;
            }
            else if (octaspire_container_utf8_string_is_equal_to_c_string(symAsStr, "DIM"))
            {
                attr = A_DIM;
            }
            else if (octaspire_container_utf8_string_is_equal_to_c_string(symAsStr, "BOLD"))
            {
                attr = A_BOLD;
            }
            else if (octaspire_container_utf8_string_is_equal_to_c_string(symAsStr, "PROTECT"))
            {
                attr = A_PROTECT;
            }
            else if (octaspire_container_utf8_string_is_equal_to_c_string(symAsStr, "INVISIBLE"))
            {
                attr = A_INVIS;
            }
            else if (octaspire_container_utf8_string_is_equal_to_c_string(symAsStr, "ALTCHARSET"))
            {
                attr = A_ALTCHARSET;
            }
            else if (octaspire_container_utf8_string_is_equal_to_c_string(symAsStr, "CHARTEXT"))
            {
                attr = A_CHARTEXT;
            }
            else
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'ncurses-attron': %zu. argument '%s' is not known attribute.",
                    i,
                    octaspire_container_utf8_string_get_c_string(symAsStr));
            }
        }
        else
        {
            attr = COLOR_PAIR(arg->value.integer);
        }

        int const tmpResult = attron(attr);

        if (tmpResult == ERR)
        {
            result = tmpResult;
        }
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, result != ERR);
}

octaspire_dern_value_t *dern_ncurses_attroff(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);

    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs == 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'ncurses-attroff' expects at least one argument. "
            "%zu arguments were given.",
            numArgs);
    }

    int result = OK;

    for (size_t i = 0; i < numArgs; ++i)
    {
        octaspire_dern_value_t const * const arg =
            octaspire_dern_value_as_vector_get_element_at_const(arguments, i);

        octaspire_helpers_verify_not_null(arg);

        if (!octaspire_dern_value_is_symbol(arg) && !octaspire_dern_value_is_integer(arg))
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'ncurses-attroff' expects symbol or integer arguments. %zu. argument "
                "has type '%s'.",
                i,
                octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
        }

        int attr = 0;

        if (octaspire_dern_value_is_symbol(arg))
        {
            octaspire_container_utf8_string_t * const symAsStr = arg->value.symbol;

            if (octaspire_container_utf8_string_is_equal_to_c_string(symAsStr, "NORMAL"))
            {
                attr = A_NORMAL;
            }
            else if (octaspire_container_utf8_string_is_equal_to_c_string(symAsStr, "STANDOUT"))
            {
                attr = A_STANDOUT;
            }
            else if (octaspire_container_utf8_string_is_equal_to_c_string(symAsStr, "UNDERLINE"))
            {
                attr = A_UNDERLINE;
            }
            else if (octaspire_container_utf8_string_is_equal_to_c_string(symAsStr, "REVERSE"))
            {
                attr = A_REVERSE;
            }
            else if (octaspire_container_utf8_string_is_equal_to_c_string(symAsStr, "BLINK"))
            {
                attr = A_BLINK;
            }
            else if (octaspire_container_utf8_string_is_equal_to_c_string(symAsStr, "DIM"))
            {
                attr = A_DIM;
            }
            else if (octaspire_container_utf8_string_is_equal_to_c_string(symAsStr, "BOLD"))
            {
                attr = A_BOLD;
            }
            else if (octaspire_container_utf8_string_is_equal_to_c_string(symAsStr, "PROTECT"))
            {
                attr = A_PROTECT;
            }
            else if (octaspire_container_utf8_string_is_equal_to_c_string(symAsStr, "INVISIBLE"))
            {
                attr = A_INVIS;
            }
            else if (octaspire_container_utf8_string_is_equal_to_c_string(symAsStr, "ALTCHARSET"))
            {
                attr = A_ALTCHARSET;
            }
            else if (octaspire_container_utf8_string_is_equal_to_c_string(symAsStr, "CHARTEXT"))
            {
                attr = A_CHARTEXT;
            }
            else
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'ncurses-attroff': %zu. argument '%s' is not known attribute.",
                    i,
                    octaspire_container_utf8_string_get_c_string(symAsStr));
            }
        }
        else
        {
            attr = COLOR_PAIR(arg->value.integer);
        }

        int const tmpResult = attroff(attr);

        if (tmpResult == ERR)
        {
            result = tmpResult;
        }
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, result != ERR);
}

octaspire_dern_value_t *dern_ncurses_has_colors(
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
            "Builtin 'ncurses-has-colors' expects zero arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    bool const result = has_colors();

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, result);
}

octaspire_dern_value_t *dern_ncurses_init_pair(
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
            "Builtin 'ncurses-init-pair' expects three arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_dern_value_t const * arg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(arg);

    if (!octaspire_dern_value_is_integer(arg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'ncurses-init-pair' expects integer as first argument. "
            "Now type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    short const pairNum = arg->value.integer;

    short colorNums[2] = {0};

    for (size_t i = 1; i < 3; ++i)
    {
        arg = octaspire_dern_value_as_vector_get_element_at_const(arguments, i);

        octaspire_helpers_verify_not_null(arg);

        if (!octaspire_dern_value_is_symbol(arg))
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'ncurses-init-pair' expects symbol as %zu. argument. "
                "Now type '%s' was given.",
                i + 1,
                octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
        }

        octaspire_container_utf8_string_t const * const symStr = arg->value.symbol;

        if (octaspire_container_utf8_string_is_equal_to_c_string(symStr, "BLACK"))
        {
            colorNums[i-1] = COLOR_BLACK;
        }
        else if (octaspire_container_utf8_string_is_equal_to_c_string(symStr, "RED"))
        {
            colorNums[i-1] = COLOR_RED;
        }
        else if (octaspire_container_utf8_string_is_equal_to_c_string(symStr, "GREEN"))
        {
            colorNums[i-1] = COLOR_GREEN;
        }
        else if (octaspire_container_utf8_string_is_equal_to_c_string(symStr, "YELLOW"))
        {
            colorNums[i-1] = COLOR_YELLOW;
        }
        else if (octaspire_container_utf8_string_is_equal_to_c_string(symStr, "BLUE"))
        {
            colorNums[i-1] = COLOR_BLUE;
        }
        else if (octaspire_container_utf8_string_is_equal_to_c_string(symStr, "MAGENTA"))
        {
            colorNums[i-1] = COLOR_MAGENTA;
        }
        else if (octaspire_container_utf8_string_is_equal_to_c_string(symStr, "CYAN"))
        {
            colorNums[i-1] = COLOR_CYAN;
        }
        else if (octaspire_container_utf8_string_is_equal_to_c_string(symStr, "WHITE"))
        {
            colorNums[i-1] = COLOR_WHITE;
        }
        else
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'ncurses-init-pair' expects color name (a symbol) as %zu. argument. "
                "Now symbol '%s' was given.",
                i + 1,
                octaspire_container_utf8_string_get_c_string(symStr));
        }
    }

    int const result = init_pair(pairNum, colorNums[0], colorNums[1]);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, result != ERR);
}

octaspire_dern_value_t *dern_ncurses_start_color(
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
            "Builtin 'ncurses-start-color' expects zero arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    int const result = start_color();

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, result != ERR);
}


octaspire_dern_value_t *dern_ncurses_refresh(
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
            "Builtin 'ncurses-refresh' expects zero arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    int const result = refresh();

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, result != ERR);
}

octaspire_dern_value_t *dern_ncurses_endwin(
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
            "Builtin 'ncurses-endwin' expects zero arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    int const result = endwin();

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, result != ERR);
}

bool dern_ncurses_init(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_environment_t * const targetEnv)
{
    setlocale(LC_ALL, "");

    octaspire_helpers_verify_true(vm && targetEnv);

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "ncurses-getmaxyx",
            dern_ncurses_getmaxyx,
            0,
            "(ncurses-getmaxyx <optional window>) -> '(rows, cols)",
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "ncurses-getch",
            dern_ncurses_getch,
            0,
            "(getch) -> <utf-8 character> or <symbol for a special key>",
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "ncurses-getstr",
            dern_ncurses_getstr,
            0,
            "(getsr) -> utf-8 string",
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "ncurses-initscr",
            dern_ncurses_initscr,
            0,
            "(initscr) -> C data window",
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "ncurses-set-raw",
            dern_ncurses_set_raw,
            0,
            "(ncurses-set-raw boolean) -> boolean",
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "ncurses-set-cbreak",
            dern_ncurses_set_cbreak,
            0,
            "(ncurses-set-cbreak boolean) -> boolean",
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "ncurses-set-echo",
            dern_ncurses_set_echo,
            0,
            "(ncurses-set-echo boolean) -> boolean",
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "ncurses-set-nl",
            dern_ncurses_set_nl,
            1,
            "(ncurses-set-nl boolean) -> nil",
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "ncurses-set-cursor",
            dern_ncurses_set_cursor,
            1,
            "(ncurses-set-cursor boolean) -> symbol of previous mode or error string",
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "ncurses-set-keypad",
            dern_ncurses_set_keypad,
            0,
            "(ncurses-set-keypad window boolean) -> boolean",
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "ncurses-erase",
            dern_ncurses_erase,
            0,
            "(ncurses-erase window) -> boolean",
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "ncurses-clear",
            dern_ncurses_clear,
            0,
            "(ncurses-clear window) -> boolean",
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "ncurses-print",
            dern_ncurses_print,
            2,
            "NAME\n"
            "\tncurses-print\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_ncurses)\n"
            "\n"
            "\t(ncurses-print window <optional y> <optional x> symbol)    -> boolean\n"
            "\t(ncurses-print window <optional y> <optional x> string)    -> boolean\n"
            "\t(ncurses-print window <optional y> <optional x> character) -> boolean\n"
            "\n"
            "DESCRIPTION\n"
            "\tPrint the given text to the given window. Components of the current cursor position\n"
            "\tare used for any missing coordinate component.\n"
            "\n"
            "ARGUMENTS\n"
            "\twindow              Target window, a value returned by some call to ncurses-initscr\n"
            "\ty        optional   Y-coordinate (line)   to print to\n"
            "\tx        optional   X-coordinate (column) to print to\n"
            "\tsymbol              The symbol's text value to print, or   ACS_BLOCK,  ACS_BOARD,\n"
            "\t                    ACS_BTEE,     ACS_BULLET,   ACS_CKBOARD, ACS_DARROW, ACS_DEGREE,\n"
            "\t                    ACS_DIAMOND,  ACS_HLINE,    ACS_LANTERN, ACS_LARROW, ACS_LLCORNER,\n"
            "\t                    ACS_LRCORNER, ACS_LTEE,     ACS_PLMINUS, ACS_PLUS,   ACS_RARROW,\n"
            "\t                    ACS_RTEE,     ACS_S1,       ACS_S9,      ACS_TTEE,   ACS_UARROW,\n"
            "\t                    ACS_ULCORNER, ACS_URCORNER, ACS_VLINE\n"
            "\tstring              The text to print. To create formatted strings, use 'string-format'\n"
            "\tcharacter           The character to print.\n"
            "\n"
            "RETURN VALUE\n"
            "\tBoolean: 'false' on failure and 'true' otherwise.\n"
            "\n"
            "SEE ALSO\n"
            "string-format",
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "ncurses-attron",
            dern_ncurses_attron,
            2,
            "NAME\n"
            "\tncurses-attron\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_ncurses)\n"
            "\n"
            "\t(ncurses-attron attribute ...) -> boolean\n"
            "\n"
            "DESCRIPTION\n"
            "\tManipulate one or more attributes\n"
            "\n"
            "ARGUMENTS\n"
            "\tattribute           Symbol for the attribute\n"
            "\n"
            "RETURN VALUE\n"
            "\tBoolean: 'false' on failure and 'true' otherwise.\n"
            "\n"
            "SEE ALSO\n"
            "",
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "ncurses-attroff",
            dern_ncurses_attroff,
            2,
            "NAME\n"
            "\tncurses-attroff\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_ncurses)\n"
            "\n"
            "\t(ncurses-attroff attribute ...) -> boolean\n"
            "\n"
            "DESCRIPTION\n"
            "\tManipulate one or more attributes\n"
            "\n"
            "ARGUMENTS\n"
            "\tattribute           Symbol for the attribute\n"
            "\n"
            "RETURN VALUE\n"
            "\tBoolean: 'false' on failure and 'true' otherwise.\n"
            "\n"
            "SEE ALSO\n"
            "",
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "ncurses-has-colors",
            dern_ncurses_has_colors,
            0,
            "NAME\n"
            "\tncurses-has-colors\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_ncurses)\n"
            "\n"
            "\t(ncurses-has-colors) -> boolean\n"
            "\n"
            "DESCRIPTION\n"
            "\tTell whether the terminal can manipulate colors\n"
            "\n"
            "ARGUMENTS\n"
            "\tnone.\n"
            "\n"
            "RETURN VALUE\n"
            "\tBoolean: 'true' if colors are supported, 'false' otherwise.\n"
            "\n"
            "SEE ALSO\n"
            "",
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "ncurses-init-pair",
            dern_ncurses_init_pair,
            0,
            "NAME\n"
            "\tncurses-init-pair\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_ncurses)\n"
            "\n"
            "\t(ncurses-init-pair colorPairNum fgColorNum bgColorNum) -> boolean\n"
            "\n"
            "DESCRIPTION\n"
            "\tChange definition of a color pair\n"
            "\n"
            "ARGUMENTS\n"
            "\tnone.\n"
            "\n"
            "RETURN VALUE\n"
            "\tBoolean: 'false' on failure and 'true' otherwise.\n"
            "\n"
            "SEE ALSO\n"
            "",
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "ncurses-start-color",
            dern_ncurses_start_color,
            0,
            "NAME\n"
            "\tncurses-start-color\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_ncurses)\n"
            "\n"
            "\t(ncurses-start-color) -> boolean\n"
            "\n"
            "DESCRIPTION\n"
            "\tEnable colors\n"
            "\n"
            "ARGUMENTS\n"
            "\tnone.\n"
            "\n"
            "RETURN VALUE\n"
            "\tBoolean: 'false' on failure and 'true' otherwise.\n"
            "\n"
            "SEE ALSO\n"
            "",
            targetEnv))
    {
        return false;
    }


    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "ncurses-refresh",
            dern_ncurses_refresh,
            0,
            "refresh",
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "ncurses-endwin",
            dern_ncurses_endwin,
            0,
            "endwin",
            targetEnv))
    {
        return false;
    }

    return true;
}

