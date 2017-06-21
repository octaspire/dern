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
#include <string.h>
#include <octaspire/core/octaspire_input.h>
#include <octaspire/dern/octaspire_dern_config.h>
#include "external/octaspire_dern_banner_color.h"
#include "external/octaspire_dern_banner_white.h"

#define ANSI_COLOR_RED    "\x1B[31m"
#define ANSI_COLOR_GREEN  "\x1B[32m"
#define ANSI_COLOR_YELLOW "\x1B[33m"
#define ANSI_COLOR_WHITE  "\x1B[37m"
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

void octaspire_dern_repl_print_message_c_str(
    char const * const message,
    octaspire_dern_repl_message_t const messageType,
    bool const useColors)
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

    printf("%s", message);

    if (useColors)
    {
        printf(ANSI_COLOR_RESET);
    }
}

void octaspire_dern_repl_print_message(
    octaspire_container_utf8_string_t const * const message,
    octaspire_dern_repl_message_t const messageType,
    bool const useColors)
{
    octaspire_dern_repl_print_message_c_str(
        octaspire_container_utf8_string_get_c_string(message),
        messageType,
        useColors);
}

void octaspire_dern_repl_print_version(bool const useColors)
{
    printf("  ");
    octaspire_dern_repl_print_message_c_str(
        OCTASPIRE_DERN_CONFIG_VERSION_STR,
        OCTASPIRE_DERN_REPL_MESSAGE_INFO,
        useColors);

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
        OCTASPIRE_DERN_REPL_MESSAGE_INFO, useColors);
}

void octaspire_dern_repl_print_usage(char const * const binaryName, bool const useColors)
{
    octaspire_dern_repl_print_banner(useColors);
    octaspire_dern_repl_print_version(useColors);
    printf("\nusage: %s [option] ... [file] ...\n", binaryName);
    printf("\nwhere [option] is one of the values listed below and every\n");
    printf("[file] is loaded and evaluated before the REPL is started or closed.\n");
    printf("If any of -e string or [file] is used, REPL is not started unless -i is used.\n\n");
    printf("-c        --color-diagnostics             : use colors on unix like systems\n");
    printf("-i        --interactive                   : start REPL after any -e string or [file]s are evaluated\n");
    printf("-e string --evaluate string               : evaluate a string without entering the REPL (unless -i is given)\n");
    printf("-f        --allow-file-system-access      : Allow code to access file system (read and write files)\n");
    printf("-v        --version                       : print version information and exit\n");
    printf("-h        --help                          : print this help message and exit\n");
}


// Globals for the REPL. ////////////////////////////
octaspire_container_vector_t      *stringsToBeEvaluated = 0;
octaspire_memory_allocator_t      *allocatorBootOnly    = 0;
octaspire_container_utf8_string_t *line                 = 0;
octaspire_stdio_t                 *stdio                = 0;
octaspire_input_t                 *input                = 0;
octaspire_dern_vm_t               *vm                   = 0;
octaspire_memory_allocator_t      *allocator            = 0;

static void octaspire_dern_repl_private_cleanup(void)
{
    octaspire_container_vector_release(stringsToBeEvaluated);
    stringsToBeEvaluated = 0;

    octaspire_memory_allocator_release(allocatorBootOnly);
    allocatorBootOnly = 0;

    octaspire_dern_vm_release(vm);
    vm = 0;

    octaspire_input_release(input);
    input = 0;

    octaspire_stdio_release(stdio);
    stdio = 0;

    octaspire_memory_allocator_release(allocator);
    allocator = 0;
}

#ifdef OCTASPIRE_PLAN9_IMPLEMENTATION
void main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    bool useColors               = false;
    int  userFilesStartIdx       = -1;
    bool enterReplAlways         = false;
    bool evaluate                = false;

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
            useColors);

        exit(EXIT_FAILURE);
    }

    allocatorBootOnly = octaspire_memory_allocator_new(0);

    if (!allocatorBootOnly)
    {
        octaspire_dern_repl_print_message_c_str(
            "Cannot create boot allocator",
            OCTASPIRE_DERN_REPL_MESSAGE_FATAL,
            useColors);

        exit(EXIT_FAILURE);
    }

    stringsToBeEvaluated = octaspire_container_vector_new(
        sizeof(octaspire_container_utf8_string_t*),
        true,
        (octaspire_container_vector_element_callback_t)octaspire_container_utf8_string_release,
        allocatorBootOnly);

    if (!stringsToBeEvaluated)
    {
        octaspire_dern_repl_print_message_c_str(
            "Cannot create evaluation vector",
            OCTASPIRE_DERN_REPL_MESSAGE_FATAL,
            useColors);

        exit(EXIT_FAILURE);
    }

    if (argc > 1)
    {
        for (int i = 1; i < argc; ++i)
        {
            if (evaluate)
            {
                evaluate = false;

                octaspire_container_utf8_string_t *tmp = octaspire_container_utf8_string_new(
                    argv[i],
                    allocatorBootOnly);

                if (!tmp)
                {
                    octaspire_dern_repl_print_message_c_str(
                        "Cannot create string to be evaluated",
                        OCTASPIRE_DERN_REPL_MESSAGE_FATAL,
                        useColors);

                    exit(EXIT_FAILURE);
                }

                octaspire_container_vector_push_back_element(stringsToBeEvaluated, &tmp);
            }
            else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--color-diagnostics") == 0)
            {
                useColors = true;
            }
            else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--interactive") == 0)
            {
                enterReplAlways = true;
            }
            else if (strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--evaluate") == 0)
            {
                evaluate = true;
            }
            else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--allow-file-system-access") == 0)
            {
                vmConfig.fileSystemAccessAllowed = true;
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
                    }
                }
            }
        }
    }

    allocator = octaspire_memory_allocator_new(0);

    if (!allocator)
    {
        octaspire_dern_repl_print_message_c_str(
            "Allocation failure\n",
            OCTASPIRE_DERN_REPL_MESSAGE_FATAL,
            useColors);

        exit(EXIT_FAILURE);
    }

    line  = 0;
    stdio = octaspire_stdio_new(allocator);
    input = octaspire_input_new_from_c_string("", allocator);
    vm    = octaspire_dern_vm_new_with_config(allocator, stdio, vmConfig);

    // Eval all files given as cmdline args
    for (size_t i = 0; i < octaspire_container_vector_get_length(stringsToBeEvaluated); ++i)
    {
        octaspire_container_utf8_string_t const * const str =
            octaspire_container_vector_get_element_at_const(stringsToBeEvaluated, i);

        octaspire_dern_value_t *value =
            octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
                vm,
                octaspire_container_utf8_string_get_c_string(str));

        assert(value);

        if (value->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
        {
            octaspire_container_utf8_string_t *str = octaspire_dern_value_to_string(value, allocator);

            octaspire_dern_repl_print_message(str, OCTASPIRE_DERN_REPL_MESSAGE_ERROR, useColors);

            printf("\n");

            octaspire_container_utf8_string_release(str);
            str = 0;

            exit(EXIT_FAILURE);
        }
    }

    if (userFilesStartIdx >= 0)
    {
        for (int i = userFilesStartIdx; i < argc; ++i)
        {
            octaspire_dern_value_t *value =
                octaspire_dern_vm_read_from_path_and_eval_in_global_environment(vm, argv[i]);

            assert(value);

            if (value->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
            {
                octaspire_container_utf8_string_t *str = octaspire_dern_value_to_string(value, allocator);

                octaspire_dern_repl_print_message(str, OCTASPIRE_DERN_REPL_MESSAGE_ERROR, useColors);

                printf("\n");

                octaspire_container_utf8_string_release(str);
                str = 0;

                exit(EXIT_FAILURE);
            }
        }

    }

    if (octaspire_container_vector_get_length(stringsToBeEvaluated) > 0 || userFilesStartIdx >= 0)
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
        OCTASPIRE_DERN_REPL_MESSAGE_INFO, useColors);

    do
    {
newInput:
        octaspire_dern_repl_print_message_c_str("> ", OCTASPIRE_DERN_REPL_MESSAGE_INFO, useColors);
moreInput:
        line = octaspire_stdio_read_line(stdio, stdin);

        if (!line)
        {
            break;
        }

        if (line && octaspire_container_utf8_string_get_length_in_ucs_characters(line) > 0)
        {
            octaspire_input_push_back_from_string(input, line);

            octaspire_dern_value_t *value = octaspire_dern_vm_parse(vm, input);

            if (!value)
            {
                octaspire_input_rewind(input);
                octaspire_dern_repl_print_message_c_str("| ", OCTASPIRE_DERN_REPL_MESSAGE_INFO, useColors);
                goto moreInput;
            }
            else if (value->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
            {
                octaspire_container_utf8_string_t *str = octaspire_dern_value_to_string(value, allocator);

                octaspire_dern_repl_print_message(str, OCTASPIRE_DERN_REPL_MESSAGE_ERROR, useColors);

                printf("\n");

                octaspire_container_utf8_string_release(str);
                str = 0;

                octaspire_input_clear(input);
                goto newInput;
            }
            else
            {
                octaspire_dern_vm_push_value(vm, value);

                octaspire_dern_value_t *evaluatedValue = octaspire_dern_vm_eval_in_global_environment(
                        vm,
                        value);

                if (!evaluatedValue)
                {
                    octaspire_dern_vm_pop_value(vm, evaluatedValue); // evaluatedValue
                    octaspire_input_rewind(input);
                    octaspire_dern_repl_print_message_c_str("+ ", OCTASPIRE_DERN_REPL_MESSAGE_INFO, useColors);
                    goto moreInput;
                    break;
                }
                else if (value->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                {
                    printf("--------------------\n");
                    octaspire_dern_value_print(evaluatedValue, octaspire_dern_vm_get_allocator(vm));
                    octaspire_input_clear(input);
                    goto newInput;
                }

                octaspire_dern_vm_push_value(vm, evaluatedValue);

                assert(evaluatedValue);

                octaspire_container_utf8_string_t *str =
                    octaspire_dern_value_to_string(evaluatedValue, allocator);

                assert(str);

                if (evaluatedValue->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
                {
                    octaspire_dern_repl_print_message(str, OCTASPIRE_DERN_REPL_MESSAGE_ERROR, useColors);
                }
                else
                {
                    octaspire_dern_repl_print_message(str, OCTASPIRE_DERN_REPL_MESSAGE_OUTPUT, useColors);
                }

                printf("\n");

                octaspire_container_utf8_string_release(str);
                str = 0;

                octaspire_dern_vm_pop_value(vm, evaluatedValue); // evaluatedValue
                octaspire_dern_vm_pop_value(vm, value); // value

                octaspire_input_clear(input);
            }
        }

        octaspire_container_utf8_string_release(line);
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
