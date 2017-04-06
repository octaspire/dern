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
#include <octaspire/core/octaspire_region.h>
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
    octaspire_dern_repl_print_message_c_str(
        OCTASPIRE_DERN_CONFIG_VERSION_STR,
        OCTASPIRE_DERN_REPL_MESSAGE_INFO,
        useColors);

    printf("\n");
}

void octaspire_dern_repl_print_banner(bool const useColors)
{
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
}

void octaspire_dern_repl_print_usage(char const * const binaryName)
{
    octaspire_dern_repl_print_version(false);
    printf("\nusage: %s [option] ... [file] ...\n", binaryName);
    printf("\nwhere [option] is one of the values listen below\n");
    printf("and every [file] is loaded and evaluated on entering the REPL\n\n");
    printf("-n  --no-region-allocator : use regular malloc/free instead of region allocator\n");
    printf("-c  --color-diagnostics   : use colors on unix like systems\n");
    printf("-e  --enter-repl-always   : start REPL after any given files are evaluated\n");
    printf("-v  --version             : print version information and exit\n");
    printf("-h  --help                : print this help message and exit\n");
}


int main(int argc, char *argv[])
{
    bool useColors          = false;
    bool useRegionAllocator = true;
    int  userFilesStartIdx  = -1;
    bool enterReplAlways    = false;

    if (argc > 1)
    {
        for (int i = 1; i < argc; ++i)
        {
            if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--no-region-allocator") == 0)
            {
                useRegionAllocator = false;
            }
            else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--color-diagnostics") == 0)
            {
                useColors = true;
            }
            else if (strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--enter-repl-always") == 0)
            {
                enterReplAlways = true;
            }
            else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0)
            {
                octaspire_dern_repl_print_version(false);
                return EXIT_SUCCESS;
            }
            else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
            {
                octaspire_dern_repl_print_usage(argv[0]);
                return EXIT_SUCCESS;
            }
            else
            {
                if (argv[i][0] == '-')
                {
                    printf("Unknown argument %s\n\n", argv[i]);
                    octaspire_dern_repl_print_usage(argv[0]);
                    return EXIT_FAILURE;
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

    octaspire_memory_allocator_t *allocator = 0;

    if (useRegionAllocator)
    {
        octaspire_region_t *region = octaspire_region_new(
            OCTASPIRE_DERN_CONFIG_MEMORY_ALLOCATOR_REGION_MIN_BLOCK_SIZE_IN_OCTETS);

        if (!region)
        {
            octaspire_dern_repl_print_message_c_str(
                "Allocation failure\n",
                OCTASPIRE_DERN_REPL_MESSAGE_FATAL,
                useColors);

            exit(EXIT_FAILURE);
        }

        allocator = octaspire_memory_allocator_new(region);
    }
    else
    {
        allocator = octaspire_memory_allocator_new(0);
    }

    if (!allocator)
    {
        octaspire_dern_repl_print_message_c_str(
            "Allocation failure\n",
            OCTASPIRE_DERN_REPL_MESSAGE_FATAL,
            useColors);

        exit(EXIT_FAILURE);
    }

    octaspire_container_utf8_string_t *line = 0;
    octaspire_stdio_t *stdio = octaspire_stdio_new(allocator);
    octaspire_input_t *input = octaspire_input_new_from_c_string("", allocator);
    octaspire_dern_vm_t *vm  = octaspire_dern_vm_new(allocator, stdio);

    // Eval all files given as cmdline args
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

                return EXIT_FAILURE;
            }
        }

        if (!enterReplAlways)
        {
            goto octaspire_dern_repl_cleanup;
        }
    }

    octaspire_dern_repl_print_banner(useColors);
    octaspire_dern_repl_print_version(useColors);

    octaspire_dern_repl_print_message_c_str(
        "Licensed under the Apache License, Version 2.0\n"
        "Distributed on an \"AS IS\" BASIS, WITHOUT WARRANTIES\n"
        "OR CONDITIONS OF ANY KIND. Quit by pressing CTRL-d\n"
        "on empty line or by writing (exit) and then enter.\n\n",
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
    for (size_t i = 0; i < 1; ++i)
    {
    }

    int32_t const exitCode = octaspire_dern_vm_get_exit_code(vm);

    octaspire_dern_vm_release(vm);
    vm = 0;

    octaspire_input_release(input);
    input = 0;

    octaspire_stdio_release(stdio);
    stdio = 0;

    octaspire_memory_allocator_release(allocator);
    allocator = 0;

    return exitCode;
}

