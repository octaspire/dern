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
#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#endif
#ifdef __FreeBSD__
#include <netinet/in.h>
#endif
#ifdef __DragonFly__
#include <netinet/in.h>
#endif
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>
#include <stdint.h>

static char const * const DERN_SOCKET_PLUGIN_NAME = "dern_socket";
static octaspire_string_t * dern_socket_plugin_private_lib_name = 0;

#ifdef _WIN32
static char const *dern_socket_private_format_win32_error_message(void)
{
    static char msgBuf[512];
    memset(msgBuf, 0, sizeof(msgBuf));

    int const errorCode = WSAGetLastError();

    FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        0,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        msgBuf,
        sizeof(msgBuf),
        0);

    if (strlen(msgBuf) == 0)
    {
        snprintf(msgBuf, sizeof(msgBuf), "%d", errorCode);
    }

    return msgBuf;
}
#endif

void dern_socket_socket_clean_up_callback(void *payload)
{
    octaspire_helpers_verify_not_null(payload);
    close((intptr_t)payload);
}

octaspire_dern_value_t *dern_socket_new_ipv4_stream_socket(
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
            "Builtin 'socket-new-ipv4-stream-socket' expects two arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_dern_value_t const * const firstArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    if (!octaspire_dern_value_is_nil(firstArg) && !octaspire_dern_value_is_text(firstArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'socket-new-ipv4-stream-socket' expects nil, string or symbol as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    char const * const address =
        octaspire_dern_value_is_nil(firstArg) ? 0 : octaspire_dern_value_as_text_get_c_string(firstArg);



    octaspire_dern_value_t const * const secondArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    if (!octaspire_dern_value_is_nil(secondArg) && !octaspire_dern_value_is_integer(secondArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'socket-new-ipv4-stream-socket' expects nil or integer as second argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }

    int const port = octaspire_dern_value_is_nil(secondArg)
        ? 0
        : (int)octaspire_dern_value_as_integer_get_value(secondArg);

#ifdef _WIN32
    WSADATA wsa;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_string_format(
            vm,
            "Builtin 'socket-new-ipv4-stream-socket' failed with error message: %s",
            dern_socket_private_format_win32_error_message());
    }

    SOCKET s;

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_string_format(
            vm,
            "Builtin 'socket-new-ipv4-stream-socket' failed to create a socket with error message: %s",
            dern_socket_private_format_win32_error_message());
    }

    if (address)
    {
        struct hostent *he = gethostbyname(address);

        if (!he)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_string_format(
                vm,
                "Builtin 'socket-new-ipv4-stream-socket' failed: gethostbyname(%s) failed.",
                address);
        }

        struct in_addr **addrs = (struct in_addr**)he->h_addr_list;

        if (addrs[0] == 0)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_string_format(
                vm,
                "Builtin 'socket-new-ipv4-stream-socket' failed: no addresses for %s",
                address);
        }

        struct sockaddr_in server;
        server.sin_addr.s_addr = addrs[0]->s_addr;
        server.sin_family      = AF_INET;
        server.sin_port        = htons(port);

        if (connect(s, (struct sockaddr*)&server, sizeof(server)) < 0)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_string_format(
                vm,
                "Builtin 'socket-new-ipv4-stream-socket' connect to %s failed.",
                address);
                //"Builtin 'socket-new-ipv4-stream-socket' connect failed: %s",
                //strerror(errno));
        }
    }
    else
    {
        struct sockaddr_in server;
        server.sin_family      = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port        = htons(port);

        if (bind(s, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_string_format(
                vm,
                "Builtin 'socket-new-ipv4-stream-socket' failed to bind a socket with error message: %s",
                dern_socket_private_format_win32_error_message());
        }

        if (listen(s, 5) == SOCKET_ERROR)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_string_format(
                vm,
                "Builtin 'socket-new-ipv4-stream-socket' failed to listen a socket with error message: %s",
                dern_socket_private_format_win32_error_message());
        }
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_SOCKET_PLUGIN_NAME,
        "socketFileDescriptor",
        "dern_socket_socket_clean_up_callback",
        "",
        "",
        "",
        "",
        "",
        false,
        (void*)s);

#else
    intptr_t const socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);

    if (socketFileDescriptor == -1)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_string_format(
            vm,
            "Builtin 'socket-new-ipv4-stream-socket' failed: %s",
            strerror(errno));
    }

    if (address)
    {
        struct hostent *he = gethostbyname(address);

        if (!he)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_string_format(
                vm,
                "Builtin 'socket-new-ipv4-stream-socket' failed: gethostbyname(%s) failed.",
                address);
        }

        struct in_addr **addrs = (struct in_addr**)he->h_addr_list;

        if (addrs[0] == 0)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_string_format(
                vm,
                "Builtin 'socket-new-ipv4-stream-socket' failed: no addresses for %s",
                address);
        }

        struct sockaddr_in server;
        server.sin_addr.s_addr = addrs[0]->s_addr;
        server.sin_family      = AF_INET;
        server.sin_port        = htons(port);

        if (connect(socketFileDescriptor, (struct sockaddr*)&server, sizeof(server)) < 0)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_string_format(
                vm,
                "Builtin 'socket-new-ipv4-stream-socket' connect failed: %s",
                strerror(errno));
        }
    }
    else
    {
        struct sockaddr_in server;
        server.sin_family      = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port        = htons(port);

        if (bind(socketFileDescriptor, (struct sockaddr*)&server, sizeof(server)) < 0)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_string_format(
                vm,
                "Builtin 'socket-new-ipv4-stream-socket' bind failed: %s",
                strerror(errno));
        }

        if (listen(socketFileDescriptor, 5) < 0)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_string_format(
                vm,
                "Builtin 'socket-new-ipv4-stream-socket' listen failed: %s",
                strerror(errno));
        }
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_SOCKET_PLUGIN_NAME,
        "socketFileDescriptor",
        "dern_socket_socket_clean_up_callback",
        "",
        "",
        "",
        "",
        "",
        false,
        (void*)socketFileDescriptor);
#endif
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

#ifdef _WIN32
    SOCKET s = (SOCKET)octaspire_dern_c_data_get_payload(cData);

    int result = closesocket(s);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    if (result != 0)
    {
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'socket-close' failed: %s.",
            dern_socket_private_format_win32_error_message());
    }

    return octaspire_dern_vm_create_new_value_boolean(vm, true);
#else
    intptr_t const socketFileDescriptor =
        (intptr_t)octaspire_dern_c_data_get_payload(cData);

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
#endif
}

octaspire_dern_value_t *dern_socket_accept(
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
            "Builtin 'socket-accept' expects one argument. "
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
            "Builtin 'socket-accept' expects socket as first argument. "
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
            "Builtin 'socket-accept' expects 'dern_socket' and 'socketFileDescriptor' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

#ifdef _WIN32
    SOCKET const s = (SOCKET const)octaspire_dern_c_data_get_payload(cData);

    struct sockaddr_in client;
    int clientLen = sizeof(struct sockaddr_in);

    SOCKET result =
        accept(s, (struct sockaddr*)&client, (int*)&clientLen);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    if (result == INVALID_SOCKET)
    {
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'socket-accept' failed: %s.",
            dern_socket_private_format_win32_error_message());
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_SOCKET_PLUGIN_NAME,
        "socketFileDescriptor",
        "dern_socket_socket_clean_up_callback",
        "",
        "",
        "",
        "",
        "",
        false,
        (void*)result);
#else
    intptr_t const socketFileDescriptor =
        (intptr_t)octaspire_dern_c_data_get_payload(cData);

    struct sockaddr_in client;
    int const clientLen = sizeof(struct sockaddr_in);

    intptr_t result =
        accept(socketFileDescriptor, (struct sockaddr*)&client, (socklen_t*)&clientLen);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    if (result < 0)
    {
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'socket-accept' failed: %s.",
            strerror(errno));
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    return octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_SOCKET_PLUGIN_NAME,
        "socketFileDescriptor",
        "dern_socket_socket_clean_up_callback",
        "",
        "",
        "",
        "",
        "",
        false,
        (void*)result);
#endif
}

octaspire_dern_value_t *dern_socket_receive(
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
            "Builtin 'socket-receive' expects two arguments. "
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
            "Builtin 'socket-receive' expects socket as first argument. "
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
            "Builtin 'socket-receive' expects 'dern_socket' and 'socketFileDescriptor' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

    octaspire_dern_value_t const * const secondArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    if (!octaspire_dern_value_is_boolean(secondArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'socket-receive' expects boolean as second argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }

    bool const waitForData = octaspire_dern_value_as_boolean_get_value(secondArg);

#ifdef _WIN32
    SOCKET const s = (SOCKET const)octaspire_dern_c_data_get_payload(cData);

    octaspire_string_t *str = 0;

    //while (true)
    {
        u_long mode = (waitForData ? 0 : 1);

        if (ioctlsocket(s, FIONBIO, &mode) != NO_ERROR)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_from_c_string(
                vm,
                "Builtin 'socket-receive' failed on ioctlsocket");
        }

        char buffer[1025] = {0};
        int const recvStatus =
            recv(s, buffer, 1024 * sizeof(char), 0);
            //recv(s, buffer, 1024 * sizeof(char), waitForData ? MSG_WAITALL : 0);

        if (recvStatus == SOCKET_ERROR)
        {
            if (waitForData)
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'socket-receive' failed to receive: %s.",
                    strerror(errno));
            }
            else
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_nil(vm);
            }
        }
        else if (recvStatus == 0)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_from_c_string(
                vm,
                "Builtin 'socket-receive' failed to receive: server closed connection.");
        }
        else
        {
            str = octaspire_string_new_from_buffer(
                buffer,
                recvStatus,
                octaspire_dern_vm_get_allocator(vm));
        }
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    octaspire_helpers_verify_not_null(str);
    return octaspire_dern_vm_create_new_value_string(vm, str);
#else
    intptr_t const socketFileDescriptor =
        (intptr_t)octaspire_dern_c_data_get_payload(cData);

    octaspire_string_t *str = 0;

    //while (true)
    {
        char buffer[1025] = {0};
        int const recvStatus =
            recv(socketFileDescriptor, buffer, 1024 * sizeof(char), waitForData ? 0 : MSG_DONTWAIT);

        if (recvStatus == -1)
        {
            if (waitForData)
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_error_format(
                    vm,
                    "Builtin 'socket-receive' failed to receive: %s.",
                    strerror(errno));
            }
            else
            {
                octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
                return octaspire_dern_vm_create_new_value_nil(vm);
            }
        }
        else if (recvStatus == 0)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_from_c_string(
                vm,
                "Builtin 'socket-receive' failed to receive: server closed connection.");
        }
        else
        {
            str = octaspire_string_new_from_buffer(
                buffer,
                recvStatus,
                octaspire_dern_vm_get_allocator(vm));
        }
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    octaspire_helpers_verify_not_null(str);
    return octaspire_dern_vm_create_new_value_string(vm, str);
#endif
}

octaspire_dern_value_t *dern_socket_send(
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
            "Builtin 'socket-send' expects two arguments. "
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
            "Builtin 'socket-send' expects socket as first argument. "
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
            "Builtin 'socket-send' expects 'dern_socket' and 'socketFileDescriptor' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

#ifdef _WIN32
    SOCKET const s = (SOCKET const)octaspire_dern_c_data_get_payload(cData);

    octaspire_dern_value_t const * const secondArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    if (!octaspire_dern_value_is_text(secondArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'socket-send' expects string or symbol as second argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }

    char const * ptr = octaspire_dern_value_as_text_get_c_string(secondArg);
    size_t       len = octaspire_dern_value_as_text_get_length_in_octets(secondArg);
    intptr_t     result = 0;

    while (true)
    {
        int const sendStatus = send(s, ptr, len, 0);

        if (sendStatus == SOCKET_ERROR)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'socket-send' failed to send: %s.",
                strerror(errno));
        }

        if (sendStatus > (int)len)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_from_c_string(
                vm,
                "Builtin 'socket-send' error: this should not happen.");
        }

        len    -= sendStatus;
        ptr    += sendStatus;
        result += sendStatus;

        if (len == 0)
        {
            break;
        }
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_integer(vm, result);
#else
    intptr_t const socketFileDescriptor =
        (intptr_t)octaspire_dern_c_data_get_payload(cData);

    octaspire_dern_value_t const * const secondArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    if (!octaspire_dern_value_is_text(secondArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'socket-send' expects string or symbol as second argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }

    char const * ptr = octaspire_dern_value_as_text_get_c_string(secondArg);
    size_t       len = octaspire_dern_value_as_text_get_length_in_octets(secondArg);
    intptr_t     result = 0;

    while (true)
    {
        int const sendStatus = send(socketFileDescriptor, ptr, len, 0);

        if (sendStatus == -1)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'socket-send' failed to send: %s.",
                strerror(errno));
        }

        if (sendStatus > (int)len)
        {
            octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
            return octaspire_dern_vm_create_new_value_error_from_c_string(
                vm,
                "Builtin 'socket-send' error: this should not happen.");
        }

        len    -= sendStatus;
        ptr    += sendStatus;
        result += sendStatus;

        if (len == 0)
        {
            break;
        }
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_integer(vm, result);
#endif
}

bool dern_socket_init(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_environment_t * const targetEnv,
    char const * const libName)
{
    setlocale(LC_ALL, "");

    octaspire_helpers_verify_true(vm && targetEnv && libName);

    dern_socket_plugin_private_lib_name = octaspire_string_new(
        libName,
        octaspire_dern_vm_get_allocator(vm));

    octaspire_helpers_verify_not_null(dern_socket_plugin_private_lib_name);

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "socket-new-ipv4-stream-socket",
            dern_socket_new_ipv4_stream_socket,
            3,
            "NAME\n"
            "\tsocket-new-ipv4-stream-socket\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_socket)\n"
            "\n"
            "\t(socket-new-ipv4-stream-socket address port) -> socket\n"
            "\n"
            "DESCRIPTION\n"
            "\tCreates and returns a new IPv4 stream socket for communication.\n"
            "\n"
            "ARGUMENTS\n"
            "\taddress             String or symbol internet address or nil\n"
            "\tport                Integer port number or nil\n"
            "\n"
            "RETURN VALUE\n"
            "\tSocket to be used with those functions of this library that\n"
            "\texpect socket argument.\n"
            "\n"
            "SEE ALSO\n"
            "socket-close",
            false,
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
            "socket-new-ipv4-stream-socket",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "socket-accept",
            dern_socket_accept,
            1,
            "NAME\n"
            "\tsocket-accept\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_socket)\n"
            "\n"
            "\t(socket-accept socket) -> socket or <error message>\n"
            "\n"
            "DESCRIPTION\n"
            "\tAccept connection request.\n"
            "\n"
            "ARGUMENTS\n"
            "\tsocket              Socket created with 'socket-socket'\n"
            "\n"
            "RETURN VALUE\n"
            "\tReturns new socket on success. On error, returns error message.\n"
            "\n"
            "SEE ALSO\n"
            "socket-new-ipv4-stream-socket",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "socket-receive",
            dern_socket_receive,
            2,
            "NAME\n"
            "\tsocket-receive\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_socket)\n"
            "\n"
            "\t(socket-receive socket wait) -> string or <error message>\n"
            "\n"
            "DESCRIPTION\n"
            "\tReads a message from the given socket.\n"
            "\n"
            "ARGUMENTS\n"
            "\tsocket              Socket created with 'socket-socket'\n"
            "\twait                Boolean telling whether to wait for data or not\n"
            "\n"
            "RETURN VALUE\n"
            "\tOn success returns a string. On error, returns error message.\n"
            "\n"
            "SEE ALSO\n"
            "socket-close, socket-send",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "socket-send",
            dern_socket_send,
            2,
            "NAME\n"
            "\tsocket-send\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_socket)\n"
            "\n"
            "\t(socket-send socket text) -> integer or <error message>\n"
            "\n"
            "DESCRIPTION\n"
            "\tSends the given message through the given socket.\n"
            "\n"
            "ARGUMENTS\n"
            "\tsocket              Socket created with 'socket-socket'\n"
            "\ttext                String or symbol text to be sent\n"
            "\n"
            "RETURN VALUE\n"
            "\tOn success returns the number of octets send. On error, returns error message.\n"
            "\n"
            "SEE ALSO\n"
            "socket-close, socket-receive",
            false,
            targetEnv))
    {
        return false;
    }

    return true;
}

bool dern_socket_clean(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_environment_t * const targetEnv)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(vm);
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(targetEnv);

    octaspire_helpers_verify_not_null(dern_socket_plugin_private_lib_name);
    octaspire_string_release(dern_socket_plugin_private_lib_name);
    dern_socket_plugin_private_lib_name = 0;
    return true;
}

