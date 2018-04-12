/***
  To build this file into a shared library in Linux system:

  gcc -c -fPIC mylib.c -I ../../../include -I ../../../external/octaspire_core/include
  gcc -shared -o libmylib.so mylib.o
***/
#include <stdio.h>
#include <octaspire/core/octaspire_helpers.h>
#include "octaspire/dern/octaspire_dern_vm.h"
#include "octaspire/dern/octaspire_dern_environment.h"

octaspire_dern_value_t *mylib_say(
    octaspire_dern_vm_t *vm,
    octaspire_dern_value_t *arguments,
    octaspire_dern_value_t *environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    if (octaspire_dern_value_as_vector_get_length(arguments) != 1)
    {
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "mylib-say expects one argument");
    }

    octaspire_dern_value_t const * const messageVal =
        octaspire_dern_value_as_vector_get_element_at_const(arguments, 0);

    if (messageVal->typeTag != OCTASPIRE_DERN_VALUE_TAG_STRING)
    {
        return octaspire_dern_vm_create_new_value_error_from_c_string(
            vm,
            "mylib-say expects string argument");
    }

    printf("%s\n", octaspire_dern_value_as_string_get_c_string(messageVal));

    return octaspire_dern_vm_create_new_value_boolean(vm, true);
}

bool mylib_init(octaspire_dern_vm_t * const vm, octaspire_dern_environment_t * const targetEnv)
{
    octaspire_helpers_verify_true(vm && targetEnv);

    if (!octaspire_dern_vm_create_and_register_new_builtin(
        vm,
        "mylib-say",
        mylib_say,
        1,
        "mylib says something",
        false,
        targetEnv))
    {
        return false;
    }

    return true;
}

