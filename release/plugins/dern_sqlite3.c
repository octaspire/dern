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
#include "sqlite3.h"

static char const * const DERN_SQLITE3_PLUGIN_NAME = "dern_sqlite3";

typedef struct dern_sqlite3_callback_context_t
{
    octaspire_dern_vm_t       *vm;
    octaspire_dern_value_t    *environment;
    sqlite3                   *db;
    octaspire_dern_function_t *callback;
    octaspire_string_t        *error;
}
dern_sqlite3_callback_context_t;

static int dern_sqlite3_private_callback(
    void *userData,
    int argc,
    char **argv,
    char **colName)
{
    dern_sqlite3_callback_context_t * const context =
        (dern_sqlite3_callback_context_t*)userData;

    context->error = 0;

    octaspire_dern_value_t *arguments =
        octaspire_dern_vm_create_new_value_vector(context->vm);

    octaspire_helpers_verify_not_null(arguments);
    octaspire_dern_vm_push_value(context->vm, arguments);

    for (int i = 0; i < argc; ++i)
    {
        octaspire_dern_value_t *pairVal =
            octaspire_dern_vm_create_new_value_vector(context->vm);

        octaspire_dern_value_as_vector_push_back_element(arguments, &pairVal);

        octaspire_dern_value_t *value =
            octaspire_dern_vm_create_new_value_string_from_c_string(
                context->vm,
                colName[i]);

        octaspire_dern_value_as_vector_push_back_element(pairVal, &value);

        value = octaspire_dern_vm_create_new_value_string_from_c_string(
                context->vm,
                argv[i] ? argv[i] : "NULL");

        octaspire_dern_value_as_vector_push_back_element(pairVal, &value);
    }

    octaspire_dern_value_t const * const result = octaspire_dern_vm_call_lambda(
        context->vm,
        context->callback,
        arguments,
        context->environment);

    octaspire_dern_vm_pop_value(context->vm, arguments);

    if (octaspire_dern_value_is_error(result))
    {
        context->error = octaspire_string_new(
            octaspire_dern_value_as_error_get_c_string(result),
            octaspire_dern_vm_get_allocator(context->vm));

        return -1;
    }

    return 0;
}

void dern_sqlite3_db_clean_up_callback(void *payload)
{
    octaspire_helpers_verify_not_null(payload);
    sqlite3_close((sqlite3*)payload);
}

octaspire_dern_value_t *dern_sqlite3_open(
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
        return octaspire_dern_vm_create_new_value_string_format(
            vm,
            "Builtin 'sqlite3-open' expects one argument. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_dern_value_t const * const firstArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    if (!octaspire_dern_value_is_text(firstArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_string_format(
            vm,
            "Builtin 'sqlite3-open' expects string or symbol as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    char const * const name =
        octaspire_dern_value_as_text_get_c_string(firstArg);

    sqlite3 *db = 0;

    if (sqlite3_open(name, &db) != SQLITE_OK)
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

        octaspire_dern_value_t * const result =
            octaspire_dern_vm_create_new_value_string_format(
                vm,
                "Builtin 'sqlite3-open' failed: %s",
                sqlite3_errmsg(db));

        sqlite3_close(db);
        return result;
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    octaspire_dern_value_t * const result =
        octaspire_dern_vm_create_new_value_c_data(
        vm,
        DERN_SQLITE3_PLUGIN_NAME,
        "sqlite3",
        "dern_sqlite3_db_clean_up_callback",
        "",
        "",
        "",
        "",
        false,
        db);

    return result;
}

octaspire_dern_value_t *dern_sqlite3_close(
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
        return octaspire_dern_vm_create_new_value_string_format(
            vm,
            "Builtin 'sqlite3-close' expects one argument. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_dern_value_t const * const firstArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(firstArg);

    if (!octaspire_dern_value_is_c_data(firstArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_string_format(
            vm,
            "Builtin 'sqlite3-close' expects socket as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    octaspire_dern_c_data_t * const cData = firstArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData,
            DERN_SQLITE3_PLUGIN_NAME,
            "sqlite3"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_string_format(
            vm,
            "Builtin 'sqlite3-close' expects 'dern_sqlite3' and 'sqlite3' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

    sqlite3 * const db =
        (sqlite3 * const)octaspire_dern_c_data_get_payload(cData);

    int result = sqlite3_close(db);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    if (result != SQLITE_OK)
    {
        return octaspire_dern_vm_create_new_value_string_format(
            vm,
            "Builtin 'sqlite3-close' failed: %i.",
            result);
    }

    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

octaspire_dern_value_t *dern_sqlite3_exec(
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
        return octaspire_dern_vm_create_new_value_string_format(
            vm,
            "Builtin 'sqlite3-exec' expects three arguments. "
            "%zu arguments were given.",
            numArgs);
    }

    // 1. argument; sqlite3 database in user data.

    octaspire_dern_value_t const * const firstArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    octaspire_helpers_verify_not_null(firstArg);

    if (!octaspire_dern_value_is_c_data(firstArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_string_format(
            vm,
            "Builtin 'sqlite3-exec' expects sqlite3 database as first argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    octaspire_dern_c_data_t * const cData = firstArg->value.cData;

    if (!octaspire_dern_c_data_is_plugin_and_payload_type_name(
            cData,
            DERN_SQLITE3_PLUGIN_NAME,
            "sqlite3"))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_string_format(
            vm,
            "Builtin 'sqlite3-exec' expects 'dern_sqlite3' and 'sqlite3' as "
            "plugin name and payload type name for the C data of the first argument. "
            "Names '%s' and '%s' were given.",
            octaspire_dern_c_data_get_plugin_name(cData),
            octaspire_dern_c_data_get_payload_typename(cData));
    }

    // 2. argument; sql

    octaspire_dern_value_t const * const secondArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 1);

    if (!octaspire_dern_value_is_text(secondArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_string_format(
            vm,
            "Builtin 'sqlite3-exec' expects string or symbol as second argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(secondArg->typeTag));
    }

    char const * const sql =
        octaspire_dern_value_as_text_get_c_string(secondArg);

    // 3. argument; callback

    octaspire_dern_value_t * const thirdArg =
        octaspire_dern_value_as_vector_get_element_at(arguments, 2);

    if (!octaspire_dern_value_is_function(thirdArg))
    {
        octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
        return octaspire_dern_vm_create_new_value_string_format(
            vm,
            "Builtin 'sqlite3-exec' expects function as third argument. "
            "Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(thirdArg->typeTag));
    }

    octaspire_dern_function_t * const callback =
        octaspire_dern_value_as_function(thirdArg);

    sqlite3 *db =
        (sqlite3 * const)octaspire_dern_c_data_get_payload(cData);

    char * errorMessage = 0;

    dern_sqlite3_callback_context_t context;
    context.vm          = vm;
    context.environment = environment;
    context.db          = db;
    context.callback    = callback;
    context.error       = 0;

    int const execResult = sqlite3_exec(
        db,
        sql,
        dern_sqlite3_private_callback,
        &context,
        &errorMessage);

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));

    if (execResult != SQLITE_OK)
    {
        octaspire_dern_value_t *result = 0;

        if (context.error)
        {
            result = octaspire_dern_vm_create_new_value_error_format(
                vm,
                "Builtin 'sqlite3-exec' failed: %s (%s).",
                errorMessage,
                octaspire_string_get_c_string(context.error));

            octaspire_string_release(context.error);
            context.error = 0;
        }
        else
        {
            result = octaspire_dern_vm_create_new_value_string_format(
                vm,
                "Builtin 'sqlite3-exec' failed: %s (%s).",
                errorMessage,
                context.error ? octaspire_string_get_c_string(context.error)
                              : "");
        }

        sqlite3_free(errorMessage);
        return result;
    }

    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

bool dern_sqlite3_init(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_environment_t * const targetEnv,
    char const * const libName)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(libname);

    octaspire_helpers_verify_true(vm && targetEnv);

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sqlite3-open",
            dern_sqlite3_open,
            1,
            "NAME\n"
            "\tsqlite3-open\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_sqlite3)\n"
            "\n"
            "\t(sqlite3-open path) -> database\n"
            "\n"
            "DESCRIPTION\n"
            "\tCreates and returns a new sqlite3 database.\n"
            "\n"
            "ARGUMENTS\n"
            "\tpath                String or symbol path of the new database\n"
            "\n"
            "RETURN VALUE\n"
            "\tDatabase to be used with those functions of this library that\n"
            "\texpect database argument.\n"
            "\n"
            "SEE ALSO\n"
            "sqlite3-close",
            false,
            targetEnv))
    {
        return false;
    }

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "sqlite3-close",
            dern_sqlite3_close,
            1,
            "NAME\n"
            "\tsqlite3-close\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_sqlite3)\n"
            "\n"
            "\t(sqlite3-close database) -> true or <error message>\n"
            "\n"
            "DESCRIPTION\n"
            "\tCloses sqlite3 database.\n"
            "\n"
            "ARGUMENTS\n"
            "\tdatabase              Database created with 'sqlite3-open'\n"
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
            "sqlite3-exec",
            dern_sqlite3_exec,
            3,
            "NAME\n"
            "\tsqlite3-exec\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_sqlite3)\n"
            "\n"
            "\t(sqlite3-exec database sql callback) -> true or <error message>\n"
            "\n"
            "DESCRIPTION\n"
            "\tExecute sql statement and call callback with the results.\n"
            "\n"
            "ARGUMENTS\n"
            "\tdatabase              Database created with 'sqlite3-open'\n"
            "\tsql                   Sql statement to be executed'\n"
            "\tcallback              Dern callback function to be called with results'\n"
            "\n"
            "RETURN VALUE\n"
            "\tReturns true on success. On error, returns error message.\n"
            "\n"
            "SEE ALSO\n"
            "sqlite3-open",
            false,
            targetEnv))
    {
        return false;
    }

    return true;
}

