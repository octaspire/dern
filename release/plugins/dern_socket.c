#include "octaspire-dern-amalgamated.c"
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <locale.h>
#include <unistd.h>

static char const * const DERN_SOCKET_PLUGIN_NAME = "dern_socket";

void dern_socket_clean_up_callback(void * const payload)
{
    octaspire_helpers_verify_not_null(payload);
    close((int)payload);
}

octaspire_dern_value_t *dern_socket_socket(
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
            "Builtin 'socket-socket' expects three arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_dern_value_t const * const firstArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(firstArg);

    if (!octaspire_dern_value_is_symbol(firstArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'socket-socket' expects symbol for domain as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    int domain, type, protocol;

    if (octaspire_dern_value_as_symbol_is_equal_to_c_string(firstArg, "AF_INET"))
    {
        domain = AF_INET;
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(firstArg, "AF_INET6"))
    {
        domain = AF_INET6;
    }
    else
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'socket-socket' expects symbol AF_INET or AF_INET6 as first argument. "
            "Symbol '%s' was given.",
            octaspire_dern_value_as_symbol_get_c_string(firstArg));
    }



    octaspire_dern_value_t const * const secondArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    octaspire_helpers_verify_not_null(secondArg);

    if (!octaspire_dern_value_is_symbol(secondArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'socket-socket' expects symbol for socket type as second argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }

    if (octaspire_dern_value_as_symbol_is_equal_to_c_string(secondArg, "SOCK_STREAM"))
    {
        type = SOCK_STREAM;
    }
    else if (octaspire_dern_value_as_symbol_is_equal_to_c_string(firstArg, "SOCK_DGRAM"))
    {
        type = SOCK_DGRAM;
    }
    else
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'socket-socket' expects symbol SOCK_STREAM or SOCK_DGRAM as second argument. "
            "Symbol '%s' was given.",
            octaspire_dern_value_as_symbol_get_c_string(secondArg));
    }





    octaspire_dern_value_t const * const thirdArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 2);

    octaspire_helpers_verify_not_null(thirdArg);

    if (!octaspire_dern_value_is_integer(thirdArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'socket-socket' expects integer for socket protocol as third argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(thirdArg->typeTag));
    }

    protocol = (int)thirdArg->value.integer;






    int const socketFileDescriptor = socket(domain, type, protocol);

    if (socketFileDescriptor < 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'socket-socket' failed to create a socket: %s.",
            strerror(errno));
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_SOCKET_PLUGIN_NAME,
        "socketFileDescriptor",
        "dern_socket_clean_up_callback",
        (void*)socketFileDescriptor);
}

octaspire_dern_value_t *dern_socket_close(
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
            "Builtin 'socket-close' expects one argument. "
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
            "Builtin 'socket-close' expects socket as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    octaspire_dern_c_data_t * const cData = firstArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData,
            DERN_SOCKET_PLUGIN_NAME,
            "socketFileDescriptor"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'socket-close' expects 'dern_socket' and 'socketFileDescriptor' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

    int const socketFileDescriptor = (int)octaspire_dern_c_data_get_payload(cData);

    int result = close(socketFileDescriptor);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    if (result < 0)
    {
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'socket-close' failed: %s.",
            strerror(errno));
    }

    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

bool dern_socket_init(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_environment_t * const targetEnv)
{
    setlocale(LC_ALL, "");

    octaspire_helpers_verify_true(vm && targetEnv);

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "socket-socket",
            dern_socket_socket,
            2,
            "NAME\n"
            "\tsocket-socket\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_socket)\n"
            "\n"
            "\t(socket-socket domain type protocol) -> socket\n"
            "\n"
            "DESCRIPTION\n"
            "\tCreates and returns a new socket for communication.\n"
            "\n"
            "ARGUMENTS\n"
            "\tdomain              Symbol AF_INET or AF_INET6\n"
            "\ttype                Symbol SOCK_STREAM or SOCK_DGRAM\n"
            "\tprotocal            Integer specifying a protocol\n"
            "\n"
            "RETURN VALUE\n"
            "\tSocket to be used with those functions of this library that\n"
            "\texpect socket argument.\n"
            "\n"
            "SEE ALSO\n"
            "socket-close",
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "socket-close",
            dern_socket_close,
            2,
            "NAME\n"
            "\tsocket-close\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_socket)\n"
            "\n"
            "\t(socket-close socket) -> true or <error message>\n"
            "\n"
            "DESCRIPTION\n"
            "\tCloses communication socket.\n"
            "\n"
            "ARGUMENTS\n"
            "\tsocket              Socket created with 'socket-socket'\n"
            "\n"
            "RETURN VALUE\n"
            "\tReturns 'true' on success. On error, returns error message.\n"
            "\n"
            "SEE ALSO\n"
            "socket-socket",
            targetEnv))
    {
        return false;
    }


    return true;
}

