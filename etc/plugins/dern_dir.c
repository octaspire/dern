#include "octaspire-dern-amalgamated.c"
#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <unistd.h>
#ifdef __linux__
#include <linux/limits.h>
#endif
#endif

octaspire_dern_value_t *dern_dir_get_listing(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    size_t const stackLength = octaspire_dern_vm_get_stack_length(vm);
    size_t const numArgs = octaspire_dern_value_as_vector_get_length(arguments);

    if (numArgs != 1)
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'dir-get-listing' expects one argument. "
            "%zu arguments were given.",
            numArgs);
    }

    octaspire_dern_value_t const * const firstArg =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    if (!octaspire_dern_value_is_text(firstArg))
    {
        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'dir-get-listing' expects string or symbol as "
            "first argument. Type '%s' was given.",
            octaspire_dern_value_helper_get_type_as_c_string(firstArg->typeTag));
    }

    char const * const path = octaspire_dern_value_as_text_get_c_string(firstArg);

    octaspire_dern_value_t * const result = octaspire_dern_vm_create_new_value_vector(vm);
    octaspire_helpers_verify_not_null(result);
    octaspire_dern_vm_push_value(vm, result);
#ifdef _WIN32
    WIN32_FIND_DATA info;

    octaspire_container_utf8_string_t *modifiedPath = octaspire_container_utf8_string_new_format(
        octaspire_dern_vm_get_allocator(vm),
        "%s\\*.*",
        path);

    HANDLE const h =
        FindFirstFile(octaspire_container_utf8_string_get_c_string(modifiedPath), &info);
    do
    {
        if (strcmp(info.cFileName, ".") != 0 && strcmp(info.cFileName, "..") != 0)
        {
            octaspire_container_utf8_string_t * const str =
                octaspire_container_utf8_string_new_format(
                    octaspire_dern_vm_get_allocator(vm),
                    "%s/%s",
                    path,
                    info.cFileName);

            octaspire_dern_value_t * const elem =
                octaspire_dern_vm_create_new_value_string(
                    vm,
                    str);

            octaspire_helpers_verify_not_null(elem);

            octaspire_helpers_verify_true(
                octaspire_dern_value_as_vector_push_back_element(result, &elem));
        }
    }
    while (FindNextFile(h, &info));

    FindClose(h);

    octaspire_container_utf8_string_release(modifiedPath);
    modifiedPath = 0;
#else
    struct dirent *entry = 0;
    DIR *dir = opendir(path);

    if (!dir)
    {
        octaspire_dern_vm_pop_value(vm, result);

        octaspire_helpers_verify_true(
            stackLength == octaspire_dern_vm_get_stack_length(vm));

        return octaspire_dern_vm_create_new_value_error_format(
            vm,
            "Builtin 'dir-get-listing' cannot open path '%s'.",
            path);
    }

    while ((entry = readdir(dir)))
    {
        if (strncmp(entry->d_name, ".", NAME_MAX) != 0 &&
            strncmp(entry->d_name, ".", NAME_MAX) != 0)
        {
            octaspire_container_utf8_string_t * const str =
                octaspire_container_utf8_string_new_format(
                    octaspire_dern_vm_get_allocator(vm),
                    "%s/%s",
                    path,
                    entry->d_name);

            octaspire_dern_value_t * const elem =
                octaspire_dern_vm_create_new_value_string(
                    vm,
                    str);

            octaspire_helpers_verify_not_null(elem);

            octaspire_helpers_verify_true(
                octaspire_dern_value_as_vector_push_back_element(result, &elem));
        }
    }
#endif

    octaspire_dern_vm_pop_value(vm, result);
    octaspire_helpers_verify_true(stackLength == octaspire_dern_vm_get_stack_length(vm));
    return result;
}

bool dern_dir_init(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_environment_t * const targetEnv)
{
    setlocale(LC_ALL, "");

    octaspire_helpers_verify_true(vm && targetEnv);

    if (!octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "dir-get-listing",
            dern_dir_get_listing,
            1,
            "NAME\n"
            "\tdir-get-listing\n"
            "\n"
            "SYNOPSIS\n"
            "\t(require 'dern_dir)\n"
            "\n"
            "\t(dir-get-listing path) -> vector\n"
            "\n"
            "DESCRIPTION\n"
            "\tCreates and returns a vector containing directory listing of a path.\n"
            "\n"
            "ARGUMENTS\n"
            "\tpath             String or symbol path of a directory to list\n"
            "\n"
            "RETURN VALUE\n"
            "\tVector containing names of the recursive contents of a directory.\n"
            "\n"
            "SEE ALSO\n",
            false,
            targetEnv))
    {
        return false;
    }

    return true;
}

