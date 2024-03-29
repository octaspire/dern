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
#include "octaspire/dern/octaspire_dern_vm.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>

#ifndef OCTASPIRE_DERN_DO_NOT_USE_AMALGAMATED_CORE
    #include "octaspire-core-amalgamated.c"
#else
    #include <octaspire/core/octaspire_input.h>
    #include <octaspire/core/octaspire_helpers.h>
#endif

#include <octaspire/dern/octaspire_dern_config.h>
#include "external/octaspire_dern_banner_color.h"
#include "external/octaspire_dern_banner_white.h"
#include <octaspire/dern/octaspire_dern_lexer.h>

#define ANSI_COLOR_RED    "\x1B[31m"
#define ANSI_COLOR_GREEN  "\x1B[32m"
#define ANSI_COLOR_YELLOW "\x1B[33m"
#define ANSI_COLOR_DGRAY  "\x1B[1;30m"
#define ANSI_COLOR_RESET  "\x1B[0m"

typedef enum
{
    OCTASPIRE_DERN_REPL_MESSAGE_INFO,
    OCTASPIRE_DERN_REPL_MESSAGE_OUTPUT,
    OCTASPIRE_DERN_REPL_MESSAGE_ERROR,
    OCTASPIRE_DERN_REPL_MESSAGE_FATAL
}
octaspire_dern_repl_message_t;

static void octaspire_dern_repl_print_message_c_str(
    char const * const message,
    octaspire_dern_repl_message_t const messageType,
    bool const useColors,
    octaspire_input_t const * const input);

static void octaspire_dern_repl_print_message(
    octaspire_string_t const * const message,
    octaspire_dern_repl_message_t const messageType,
    bool const useColors,
    octaspire_input_t const * const input);

static void octaspire_dern_repl_print_version(bool const useColors);

static void octaspire_dern_repl_print_banner(bool const useColors);

static void octaspire_dern_repl_print_usage(
    char const * const binaryName,
    bool const useColors);



static void octaspire_dern_repl_print_message_c_str(
    char const * const message,
    octaspire_dern_repl_message_t const messageType,
    bool const useColors,
    octaspire_input_t const * const input)
{
    if (useColors)
    {
        switch (messageType)
        {
            case OCTASPIRE_DERN_REPL_MESSAGE_INFO:
            {
                printf(ANSI_COLOR_DGRAY);
            }
            break;

            case OCTASPIRE_DERN_REPL_MESSAGE_OUTPUT:
            {
                printf(ANSI_COLOR_GREEN);
            }
            break;

            case OCTASPIRE_DERN_REPL_MESSAGE_ERROR:
            {
                printf(ANSI_COLOR_RED);
            }
            break;

            case OCTASPIRE_DERN_REPL_MESSAGE_FATAL:
            {
                printf(ANSI_COLOR_YELLOW);
            }
            break;
        }
    }

    if (input && (messageType != OCTASPIRE_DERN_REPL_MESSAGE_OUTPUT))
    {
        printf(
            "At line %zu, column %zu (character index %zu from the start of "
            "the stream):\n",
            octaspire_input_get_line_number(input),
            octaspire_input_get_column_number(input),
            octaspire_input_get_ucs_character_index(input));
    }

    printf("%s", message);

    if (useColors)
    {
        printf(ANSI_COLOR_RESET);
    }
}

void octaspire_dern_repl_print_message(
    octaspire_string_t const * const message,
    octaspire_dern_repl_message_t const messageType,
    bool const useColors,
    octaspire_input_t const * const input)
{
    octaspire_dern_repl_print_message_c_str(
        octaspire_string_get_c_string(message),
        messageType,
        useColors,
        input);
}

void octaspire_dern_repl_print_version(bool const useColors)
{
    printf("  ");
    octaspire_dern_repl_print_message_c_str(
        OCTASPIRE_DERN_CONFIG_VERSION_STR,
        OCTASPIRE_DERN_REPL_MESSAGE_INFO,
        useColors,
        0);

    printf("\n");
}

void octaspire_dern_repl_print_banner(bool const useColors)
{
    printf("\n");
    if (useColors)
    {
        for (size_t i = 0; i < octaspire_dern_banner_color_len; ++i)
        {
            putchar(octaspire_dern_banner_color[i]);
        }
    }
    else
    {
        for (size_t i = 0; i < octaspire_dern_banner_white_len; ++i)
        {
            putchar(octaspire_dern_banner_white[i]);
        }
    }

    octaspire_dern_repl_print_message_c_str(
        "  \n"
        "  Licensed under the Apache License, Version 2.0. Distributed on\n"
        "  an \"AS IS\" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND.\n\n",
        OCTASPIRE_DERN_REPL_MESSAGE_INFO,
        useColors,
        0);
}

void octaspire_dern_repl_print_usage(char const * const binaryName, bool const useColors)
{
    octaspire_dern_repl_print_banner(useColors);
    octaspire_dern_repl_print_version(useColors);
    printf("\nusage: %s [option] ... [file] ...\n", binaryName);

    char const * const str =
        "\nwhere [option] is one of the values listed below and every\n"
        "[file] is loaded and evaluated before the REPL is started or closed.\n"
        "If any of -e string or [file] is used, REPL is not started unless -i is used.\n\n"
        "-c        --color-diagnostics : use colors on unix like systems\n"
        "-i        --interactive       : start REPL after any -e string or [file]s are evaluated\n"
        "-I dir    --include dir       : Search this directory for source (.dern) libraries\n"
        "-e string --evaluate string   : evaluate a string without entering the REPL (see -i)\n"
        "-v        --version           : print version information and exit\n"
        "-h        --help              : print this help message and exit\n"
        "-g        --debug             : print every form to stderr before it is evaluated\n"
        "-d        --no-dlclose        : do not close dynamic libraries;\n"
        "                                useful when searching memory leaks from plugins\n"
        "                                using Valgrind\n";

    printf("%s", str);
}


// Globals for the REPL. ////////////////////////////
static octaspire_vector_t    *stringsToBeEvaluated = 0;
static octaspire_vector_t    *includeDirectories   = 0;
static octaspire_allocator_t *allocatorBootOnly    = 0;
static octaspire_string_t    *line                 = 0;
static octaspire_stdio_t     *stdio                = 0;
static octaspire_input_t     *input                = 0;
static octaspire_dern_vm_t   *vm                   = 0;
static octaspire_allocator_t *allocator            = 0;

static void octaspire_dern_repl_private_cleanup(void)
{
    octaspire_vector_release(stringsToBeEvaluated);
    stringsToBeEvaluated = 0;

    octaspire_vector_release(includeDirectories);
    includeDirectories = 0;

    octaspire_allocator_release(allocatorBootOnly);
    allocatorBootOnly = 0;

    octaspire_dern_vm_release(vm);
    vm = 0;

    octaspire_input_release(input);
    input = 0;

    octaspire_stdio_release(stdio);
    stdio = 0;

    octaspire_allocator_release(allocator);
    allocator = 0;
}

#ifdef OCTASPIRE_PLAN9_IMPLEMENTATION
void main(int argc, char *argv[])
#else
    #ifdef _MSC_VER
    int main(int argc, char *argv[], char *envp[])
    #elif _WIN32
    int main(int argc, char *argv[], char *environ[])
    #elif __amigaos__
    int main(int argc, char *argv[], char *environ[])
    #else
    int main(int argc, char *argv[])
    #endif
#endif
{
#ifndef OCTASPIRE_PLAN9_IMPLEMENTATION
    setlocale(LC_ALL, "");
#endif
    bool useColors               = false;
    int  userFilesStartIdx       = -1;
    bool enterReplAlways         = false;
    bool evaluate                = false;
    bool include                 = false;

    octaspire_dern_vm_config_t vmConfig = octaspire_dern_vm_config_default();

#ifdef OCTASPIRE_PLAN9_IMPLEMENTATION
    if (atexit(octaspire_dern_repl_private_cleanup) == 0)
#else
    if (atexit(octaspire_dern_repl_private_cleanup) != 0)
#endif
    {
        octaspire_dern_repl_print_message_c_str(
            "Cannot register the 'atexit' function",
            OCTASPIRE_DERN_REPL_MESSAGE_FATAL,
            useColors,
            0);

        exit(EXIT_FAILURE);
    }

    allocatorBootOnly = octaspire_allocator_new(0);

    if (!allocatorBootOnly)
    {
        octaspire_dern_repl_print_message_c_str(
            "Cannot create boot allocator",
            OCTASPIRE_DERN_REPL_MESSAGE_FATAL,
            useColors,
            0);

        exit(EXIT_FAILURE);
    }

    stringsToBeEvaluated = octaspire_vector_new(
        sizeof(octaspire_string_t*),
        true,
        (octaspire_vector_element_callback_t)octaspire_string_release,
        allocatorBootOnly);

    if (!stringsToBeEvaluated)
    {
        octaspire_dern_repl_print_message_c_str(
            "Cannot create evaluation vector",
            OCTASPIRE_DERN_REPL_MESSAGE_FATAL,
            useColors,
            0);

        exit(EXIT_FAILURE);
    }

    includeDirectories = octaspire_vector_new(
        sizeof(octaspire_string_t*),
        true,
        (octaspire_vector_element_callback_t)octaspire_string_release,
        allocatorBootOnly);

    if (!includeDirectories)
    {
        octaspire_dern_repl_print_message_c_str(
            "Cannot create include directory vector",
            OCTASPIRE_DERN_REPL_MESSAGE_FATAL,
            useColors,
            0);

        exit(EXIT_FAILURE);
    }

    if (argc > 1)
    {
        for (int i = 1; i < argc; ++i)
        {
            if (evaluate)
            {
                evaluate = false;

                octaspire_string_t *tmp = octaspire_string_new(
                    argv[i],
                    allocatorBootOnly);

                if (!tmp)
                {
                    octaspire_dern_repl_print_message_c_str(
                        "Cannot create string to be evaluated",
                        OCTASPIRE_DERN_REPL_MESSAGE_FATAL,
                        useColors,
                        0);

                    exit(EXIT_FAILURE);
                }

                octaspire_vector_push_back_element(stringsToBeEvaluated, &tmp);
            }
            else if (include)
            {
                include = false;

                octaspire_string_t *tmp = octaspire_string_new(
                    argv[i],
                    allocatorBootOnly);

                if (!tmp)
                {
                    octaspire_dern_repl_print_message_c_str(
                        "Cannot create string for include path",
                        OCTASPIRE_DERN_REPL_MESSAGE_FATAL,
                        useColors,
                        0);

                    exit(EXIT_FAILURE);
                }

                octaspire_vector_push_back_element(includeDirectories, &tmp);
                vmConfig.includeDirectories = includeDirectories;
            }
            else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--color-diagnostics") == 0)
            {
                useColors = true;
            }
            else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--interactive") == 0)
            {
                enterReplAlways = true;
            }
            else if (strcmp(argv[i], "-I") == 0 || strcmp(argv[i], "--include") == 0)
            {
                include = true;
            }
            else if (strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--evaluate") == 0)
            {
                evaluate = true;
            }
            else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0)
            {
                octaspire_dern_repl_print_version(useColors);
                exit(EXIT_SUCCESS);
            }
            else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
            {
                octaspire_dern_repl_print_usage(argv[0], useColors);
                exit(EXIT_SUCCESS);
            }
            else if (strcmp(argv[i], "-g") == 0 || strcmp(argv[i], "--debug") == 0)
            {
                vmConfig.debugModeOn = true;
            }
            else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--no-dlclose") == 0)
            {
                vmConfig.noDlClose = true;
            }
            else
            {
                if (argv[i][0] == '-')
                {
                    printf("Unknown argument %s\n\n", argv[i]);
                    octaspire_dern_repl_print_usage(argv[0], useColors);
                    exit(EXIT_FAILURE);
                }
                else
                {
                    if (userFilesStartIdx < 0)
                    {
                        userFilesStartIdx = i;
                        break;
                    }
                }
            }
        }
    }

    allocator = octaspire_allocator_new(0);

    if (!allocator)
    {
        octaspire_dern_repl_print_message_c_str(
            "Allocation failure\n",
            OCTASPIRE_DERN_REPL_MESSAGE_FATAL,
            useColors,
            0);

        exit(EXIT_FAILURE);
    }

    line  = 0;
    stdio = octaspire_stdio_new(allocator);
    input = octaspire_input_new_from_c_string("", allocator);
    vm    = octaspire_dern_vm_new_with_config(allocator, stdio, vmConfig);

#ifndef OCTASPIRE_PLAN9_IMPLEMENTATION
#ifndef _WIN32
    #ifndef __amigaos__
    extern char **environ;
    #endif
#endif

#ifdef _MSC_VER
    for (char **var = envp; (var && *var); ++var)
#else
    for (char **var = environ; (var && *var); ++var)
#endif
    {
        octaspire_dern_vm_add_environment_variable(vm, *var);
    }
#endif

    // Eval all files given as cmdline args
    for (size_t i = 0; i < octaspire_vector_get_length(stringsToBeEvaluated); ++i)
    {
        octaspire_string_t const * const str =
            octaspire_vector_get_element_at_const(
                stringsToBeEvaluated,
                (ptrdiff_t)i);

        octaspire_dern_value_t *value =
            octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
                vm,
                octaspire_string_get_c_string(str));

        if (!value)
        {
            octaspire_string_t *tmpStr =
                octaspire_string_new("Incomplete input", allocator);

            octaspire_dern_repl_print_message(
                tmpStr,
                OCTASPIRE_DERN_REPL_MESSAGE_ERROR,
                useColors,
                input);

            printf("\n");

            octaspire_string_release(tmpStr);
            tmpStr = 0;

            exit(EXIT_FAILURE);
        }

        if (value->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
        {
            octaspire_string_t *tmpStr =
                octaspire_dern_value_to_string(value, allocator);

            octaspire_dern_repl_print_message(
                tmpStr,
                OCTASPIRE_DERN_REPL_MESSAGE_ERROR,
                useColors,
                input);

            printf("\n");

            octaspire_string_release(tmpStr);
            tmpStr = 0;

            exit(EXIT_FAILURE);
        }
    }

    if (userFilesStartIdx >= 0)
    {
        for (int i = userFilesStartIdx + 1; i < argc; ++i)
        {
            octaspire_dern_vm_add_command_line_argument(vm, argv[i]);
        }

        octaspire_input_release(input);
        input = 0;

        input = octaspire_input_new_from_path(
            argv[userFilesStartIdx],
            allocator,
            stdio);

        if (!input)
        {
            octaspire_string_t *tmpStr =
                octaspire_string_new_format(allocator, "Path '%s' cannot be read", argv[userFilesStartIdx]);

            octaspire_dern_repl_print_message(
                tmpStr,
                OCTASPIRE_DERN_REPL_MESSAGE_ERROR,
                useColors,
                input);

            printf("\n");

            octaspire_string_release(tmpStr);
            tmpStr = 0;

            exit(EXIT_FAILURE);
        }

        octaspire_dern_value_t *value = 0;

        if (!input)
        {
            value = octaspire_dern_vm_create_new_value_error_from_c_string(
                vm,
                "Allocation failure of input");
        }
        else
        {
            value = octaspire_dern_vm_read_from_octaspire_input_and_eval_in_global_environment(
                vm,
                input);
        }

        octaspire_helpers_verify_not_null(value);

        if (value->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
        {
            octaspire_string_t *str =
                octaspire_dern_value_to_string(value, allocator);

            octaspire_dern_repl_print_message(
                str,
                OCTASPIRE_DERN_REPL_MESSAGE_ERROR,
                useColors,
                input);

            printf("\n");

            octaspire_string_release(str);
            str = 0;

            exit(EXIT_FAILURE);
        }
    }

    octaspire_input_clear(input);

    if (octaspire_vector_get_length(stringsToBeEvaluated) > 0 || userFilesStartIdx >= 0)
    {
        if (!enterReplAlways)
        {
            goto octaspire_dern_repl_cleanup;
        }
    }

    octaspire_dern_repl_print_banner(useColors);
    octaspire_dern_repl_print_version(useColors);

    octaspire_dern_repl_print_message_c_str(
        "\n  Quit by pressing CTRL-d on empty line\n"
        "  or by writing (exit) and then enter.\n\n",
        OCTASPIRE_DERN_REPL_MESSAGE_INFO,
        useColors,
        0);

    do
    {
        octaspire_dern_repl_print_message_c_str(
            "> ",
            OCTASPIRE_DERN_REPL_MESSAGE_INFO,
            useColors,
            0);
moreInput:
        line = octaspire_stdio_read_line(stdio, stdin);

        if (!line)
        {
            break;
        }

        if (line && octaspire_string_get_length_in_ucs_characters(line) > 0)
        {
            octaspire_input_push_back_from_string(input, line);

            octaspire_dern_lexer_token_t *token =
                octaspire_dern_lexer_pop_next_token(input, allocator);

            if (!token)
            {
                goto noToken;
            }

            octaspire_dern_value_t *parsedValue =
                octaspire_dern_vm_parse_token(vm, token, input);

            octaspire_dern_lexer_token_release(token);
            token = 0;

            if (!parsedValue)
            {
                octaspire_input_rewind(input);

                octaspire_dern_repl_print_message_c_str(
                    "| ",
                    OCTASPIRE_DERN_REPL_MESSAGE_INFO,
                    useColors,
                    0);

                goto moreInput;
            }

            octaspire_dern_vm_push_value(vm, parsedValue);

            octaspire_dern_value_t *evaluatedValue =
                octaspire_dern_vm_eval_in_global_environment(vm, parsedValue);

            if (!evaluatedValue)
            {
                octaspire_input_rewind(input);

                octaspire_dern_repl_print_message_c_str(
                    "| ",
                    OCTASPIRE_DERN_REPL_MESSAGE_INFO,
                    useColors,
                    0);

                octaspire_dern_vm_pop_value(vm, parsedValue);
                goto moreInput;
            }

            octaspire_string_t *str =
                octaspire_dern_value_to_string(evaluatedValue, allocator);

            octaspire_dern_repl_print_message(
                str,
                (parsedValue->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                    ? OCTASPIRE_DERN_REPL_MESSAGE_ERROR
                    : OCTASPIRE_DERN_REPL_MESSAGE_OUTPUT,
                useColors,
                input);

            octaspire_dern_vm_pop_value(vm, parsedValue);

        noToken:
            printf("\n");

            octaspire_string_release(str);
            str = 0;

            octaspire_input_clear(input);
        }

        octaspire_string_release(line);
        line = 0;
    }
    while (!octaspire_dern_vm_is_quit(vm));


octaspire_dern_repl_cleanup:
    // A label can only be part of a statement
    for (size_t i = 0; i < 1; ++i)
    {
    }

    int32_t exitCode = EXIT_FAILURE;

    if (vm)
    {
        exitCode = octaspire_dern_vm_get_exit_code(vm);
    }

    octaspire_dern_repl_private_cleanup();

#ifdef OCTASPIRE_PLAN9_IMPLEMENTATION
    exits(exitCode == EXIT_SUCCESS ? "" : "error");
#else
    return exitCode;
#endif
}

