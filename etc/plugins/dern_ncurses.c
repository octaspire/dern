#include "octaspire-dern-amalgamated.c"
#include <ncurses.h>

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
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'ncurses-getch' expects zero arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    int const ch = getch();

    if (ch < 0)
    {
        abort();
    }

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_character_from_uint32t(vm, (uint32_t)ch);
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
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'ncurses-set-raw' expects boolean argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }


    int const result = arg->value.boolean ? raw() : noraw();

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'ncurses-set-cbreak' expects boolean argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    int const result = arg->value.boolean ? cbreak() : nocbreak();

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'ncurses-set-echo' expects boolean argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    int const result = arg->value.boolean ? echo() : noecho();

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'ncurses-set-nl' expects boolean argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(arg->typeTag));
    }

    arg->value.boolean ? nl() : nonl();

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_nil(vm);
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
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'ncurses-set-keypad' expects two arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_dern_value_t const * const firstArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify(firstArg);

    if (!octaspire_dern_value_is_c_data(firstArg))
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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

    octaspire_helpers_verify(secondArg);

    if (!octaspire_dern_value_is_boolean(secondArg))
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'ncurses-set-keypad' expects boolean as second argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }

    int const result = keypad(octaspire_dern_c_data_get_payload(cData), secondArg->value.boolean);

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'ncurses-erase' expects one argument. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_dern_value_t const * const firstArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify(firstArg);

    if (!octaspire_dern_value_is_c_data(firstArg))
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'ncurses-erase' expects 'dern_ncurses' and 'window' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

    int const result = werase(octaspire_dern_c_data_get_payload(cData));

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'ncurses-clear' expects one argument. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_dern_value_t const * const firstArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify(firstArg);

    if (!octaspire_dern_value_is_c_data(firstArg))
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'ncurses-clear' expects 'dern_ncurses' and 'window' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

    int const result = wclear(octaspire_dern_c_data_get_payload(cData));

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, result != ERR);
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
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'ncurses-print' expects 2 - 4 arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_dern_value_t const * const firstArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify(firstArg);

    if (!octaspire_dern_value_is_c_data(firstArg))
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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

    octaspire_helpers_verify(secondArg);

    if (numArgs == 2)
    {
        if (!octaspire_dern_value_is_text(secondArg))
        {
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'ncurses-print' expects text (string, character or symbol) as "
                "second argument. Now second argument has type '%s'.",
                octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
        }

        int const result = wprintw(
            octaspire_dern_c_data_get_payload(cData),
            "%s",
            octaspire_dern_value_as_text_get_c_string(secondArg));

        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_boolean(vm, result != ERR);
    }
    else if (numArgs == 3)
    {
        if (!octaspire_dern_value_is_integer(secondArg))
        {
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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

        int const result = mvwprintw(
            payload,
            secondArg->value.integer,
            currentX,
            "%s",
            octaspire_dern_value_as_text_get_c_string(thirdArg));

        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_boolean(vm, result != ERR);
    }
    else if (numArgs == 4)
    {
        if (!octaspire_dern_value_is_integer(secondArg))
        {
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'ncurses-print' expects text (string, character or symbol) as fourth "
                "argument when four arguments are given. Now the fourth argument has type '%s'.",
                octaspire_dern_value_helper_get_type_as_c_string(fourthArg->typeTag));
        }

        int const result = mvwprintw(
            octaspire_dern_c_data_get_payload(cData),
            secondArg->value.integer,
            thirdArg->value.integer,
            "%s",
            octaspire_dern_value_as_text_get_c_string(fourthArg));

        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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

        octaspire_helpers_verify(arg);

        if (!octaspire_dern_value_is_symbol(arg) && !octaspire_dern_value_is_integer(arg))
        {
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
            else if (octaspire_container_utf8_string_is_equal_to_c_string(symAsStr, "ITALIC"))
            {
                attr = A_ITALIC;
            }
            else if (octaspire_container_utf8_string_is_equal_to_c_string(symAsStr, "CHARTEXT"))
            {
                attr = A_CHARTEXT;
            }
            else
            {
                octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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

        octaspire_helpers_verify(arg);

        if (!octaspire_dern_value_is_symbol(arg) && !octaspire_dern_value_is_integer(arg))
        {
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
            else if (octaspire_container_utf8_string_is_equal_to_c_string(symAsStr, "ITALIC"))
            {
                attr = A_ITALIC;
            }
            else if (octaspire_container_utf8_string_is_equal_to_c_string(symAsStr, "CHARTEXT"))
            {
                attr = A_CHARTEXT;
            }
            else
            {
                octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'ncurses-has-colors' expects zero arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    bool const result = has_colors();

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'ncurses-init-pair' expects three arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_dern_value_t const * arg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify(arg);

    if (!octaspire_dern_value_is_integer(arg))
    {
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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

        octaspire_helpers_verify(arg);

        if (!octaspire_dern_value_is_symbol(arg))
        {
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
            octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'ncurses-init-pair' expects color name (a symbol) as %zu. argument. "
                "Now symbol '%s' was given.",
                i + 1,
                octaspire_container_utf8_string_get_c_string(symStr));
        }
    }

    bool const result = init_pair(pairNum, colorNums[0], colorNums[1]);

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'ncurses-start-color' expects zero arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    bool const result = start_color();

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, result);
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
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'ncurses-refresh' expects zero arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    int const result = refresh();

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
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
        octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'ncurses-endwin' expects zero arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    int const result = endwin();

    octaspire_helpers_verify(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, result != ERR);
}

bool dern_ncurses_init(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_environment_t * const targetEnv)
{
    octaspire_helpers_verify(vm && targetEnv);

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "ncurses-getch",
            dern_ncurses_getch,
            0,
            "(getch) -> utf-8 character",
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
            "\tsymbol      |       \n"
            "\tstring      |       The text to print. To create formatted strings, use 'string-format'\n"
            "\tcharacter   |       \n"
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

