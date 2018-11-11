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
#include "../src/octaspire_dern_vm.c"
#include "external/greatest.h"
#include "octaspire/dern/octaspire_dern_vm.h"
#include "octaspire/dern/octaspire_dern_config.h"

static octaspire_allocator_t *octaspireDernVmTestAllocator = 0;
static octaspire_stdio_t            *octaspireDernVmTestStdio     = 0;

TEST octaspire_dern_vm_new_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    ASSERT(vm->stack);
    ASSERT_EQ(octaspireDernVmTestAllocator, vm->allocator);
    ASSERT(vm->all);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_create_new_value_boolean_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    for (size_t i = 0; i < 256; ++i)
    {
        octaspire_dern_value_t *value = octaspire_dern_vm_create_new_value_boolean(vm, i % 2);

        ASSERT(value);
        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, value->typeTag);
        ASSERT_EQ(i % 2, value->value.boolean);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_create_new_value_integer_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    for (int32_t i = 0; i < 256; ++i)
    {
        octaspire_dern_value_t *value = octaspire_dern_vm_create_new_value_integer(vm, i);

        ASSERT(value);
        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, value->typeTag);
        ASSERT_EQ(i, value->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_create_new_value_boolean_and_push_one_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *pushedValue = 0;

    for (size_t i = 0; i < 256; ++i)
    {
        octaspire_dern_value_t *value =
            octaspire_dern_vm_create_new_value_boolean(vm, i % 2);

        if (i == 10)
        {
            ASSERT(octaspire_dern_vm_push_value(vm, value));
            pushedValue = value;
        }
    }

    // Make sure we can use the pushed value (it is not freed)
    ASSERT(pushedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, pushedValue->typeTag);
    ASSERT_EQ(10 % 2, pushedValue->value.boolean);
    //ASSERT(octaspire_dern_value_get_hash(pushedValue));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_vm_parse_and_eval_true_amid_whitespace_test(void)
{
    octaspire_input_t *input = octaspire_input_new_from_c_string("  \t \n  \t true  ", octaspireDernVmTestAllocator);

    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_environment_t *wrappedEnv = octaspire_dern_environment_new(0, vm, octaspireDernVmTestAllocator);

    octaspire_dern_value_t *environment =
        octaspire_dern_vm_create_new_value_environment_from_environment(vm, wrappedEnv);

    ASSERT(environment);

    ASSERT(octaspire_dern_vm_push_value(vm, environment));

    octaspire_dern_value_t *parsedValue = octaspire_dern_vm_parse(vm, input);

    ASSERT(parsedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, parsedValue->typeTag);
    ASSERT_EQ(true,                             parsedValue->value.boolean);

    ASSERT(octaspire_dern_vm_push_value(vm, parsedValue));

    octaspire_dern_value_t *evaluatedValue = octaspire_dern_vm_eval(vm, parsedValue, environment);

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    ASSERT(octaspire_dern_vm_push_value(vm, evaluatedValue));

    ASSERT_EQ(evaluatedValue, octaspire_dern_vm_peek_value(vm));
    octaspire_dern_vm_pop_value(vm, evaluatedValue);

    ASSERT_EQ(parsedValue, octaspire_dern_vm_peek_value(vm));
    octaspire_dern_vm_pop_value(vm, parsedValue);

    ASSERT_EQ(environment, octaspire_dern_vm_peek_value(vm));
    octaspire_dern_vm_pop_value(vm, environment);

    octaspire_dern_vm_release(vm);
    vm = 0;

    octaspire_input_release(input);
    input = 0;

    PASS();
}

TEST octaspire_dern_vm_special_if_two_elements_false_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(if false true)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_NIL, evaluatedValue->typeTag);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_if_two_elements_true_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(if true |g|)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, evaluatedValue->typeTag);
    ASSERT_STR_EQ("g", octaspire_dern_value_as_character_get_c_string(evaluatedValue)); 

    octaspire_dern_vm_release(vm);
    vm = 0;

    
    PASS();
}

TEST octaspire_dern_vm_special_if_three_elements_false_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(if false |r| |g|)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, evaluatedValue->typeTag);
    ASSERT_STR_EQ("g", octaspire_dern_value_as_character_get_c_string(evaluatedValue)); 

    octaspire_dern_vm_release(vm);
    vm = 0;

    
    PASS();
}

TEST octaspire_dern_vm_special_if_three_elements_true_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(if true |r| |g|)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, evaluatedValue->typeTag);
    ASSERT_STR_EQ("r", octaspire_dern_value_as_character_get_c_string(evaluatedValue)); 

    octaspire_dern_vm_release(vm);
    vm = 0;

    
    PASS();
}

TEST octaspire_dern_vm_special_if_three_elements_with_function_resulting_true_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(if (fn () true) |r| |g|)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, evaluatedValue->typeTag);
    ASSERT_STR_EQ("r", octaspire_dern_value_as_character_get_c_string(evaluatedValue)); 

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_if_three_elements_with_function_call_resulting_true_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(if ((fn () true)) |r| |g|)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, evaluatedValue->typeTag);
    ASSERT_STR_EQ("r", octaspire_dern_value_as_character_get_c_string(evaluatedValue)); 

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_if_called_with_one_argument_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(if true)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Special 'if' expects two or three arguments. 1 arguments were given.\n"
        "\tAt form: >>>>>>>>>>(if true)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_if_called_with_integer_as_the_first_argument_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(if {D+10} true)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "First argument to special 'if' must evaluate into boolean value. Now it evaluated "
        "into type integer.\n"
        "\tAt form: >>>>>>>>>>(if {D+10} true)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_select_one_true_selector_to_string_a_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(select true [a])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ("a", octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_select_one_false_selector_to_string_a_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(select false [a])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_NIL, evaluatedValue->typeTag);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_select_one_default_selector_to_string_a_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(select default [a])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ("a", octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_select_one_false_and_one_default_selectors_to_string_a_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(select false [p] default [a])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ("a", octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_select_one_false_and_one_true_and_one_default_selectors_to_string_a_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(select false [p] true [a] default [b])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ("a", octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_select_one_false_and_one_true_selectors_to_string_a_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(select false [p] true [a])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ("a", octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_select_function_selectors_evaluating_into_false_and_true_to_string_a_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f1 as (fn () false) [f1] '() howto-ok)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define t1 as (fn () true) [t1] '() howto-ok)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(select (f1) [p] (t1) [a])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ("a", octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_select_function_selectors_failure_on_unknown_symbol_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f1 as (fn () false) [f1] '() howto-ok)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define t1 as (fn () true) [t1] '() howto-ok)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(select (f1) [p] (f2) [a])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Cannot evaluate operator of type 'error' (<error>: Unbound symbol 'f2')\n"
        "\tAt form: >>>>>>>>>>(select (f1) [p] (f2) [a])<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_select_called_with_zero_arguments_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(select)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Special 'select' expects at least two arguments and the number of arguments must be "
        "multiple of two. Now 0 arguments were given.\n"
        "\tAt form: >>>>>>>>>>(select)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_select_called_with_one_argument_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(select true)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Special 'select' expects at least two arguments and the number of arguments must be "
        "multiple of two. Now 1 arguments were given.\n"
        "\tAt form: >>>>>>>>>>(select true)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_select_called_with_three_arguments_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(select true [a] default)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Special 'select' expects at least two arguments and the number of arguments must be "
        "multiple of two. Now 3 arguments were given.\n"
        "\tAt form: >>>>>>>>>>(select true [a] default)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_select_called_non_boolean_selector_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(select {D+1} [a] {D+2} [b])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Selectors of special 'select' must evaluate into booleans. Type 'integer' was given.\n"
        "\tAt form: >>>>>>>>>>(select {D+1} [a] {D+2} [b])<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_select_called_with_default_as_first_selector_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(select default [a] {D+2} [b])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "'default' must be the last selector in special 'select'.\n"
        "\tAt form: >>>>>>>>>>(select default [a] {D+2} [b])<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_define_integer_value_test(void)
{
    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "(define x as {D+10} [test])",
        octaspireDernVmTestAllocator);

    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *parsedValue = octaspire_dern_vm_parse(vm, input);

    ASSERT(parsedValue);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_eval_in_global_environment(vm, parsedValue);

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(vm, "x");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(10,                               evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    octaspire_input_release(input);
    input = 0;

    PASS();
}

TEST octaspire_dern_vm_special_define_integer_value_with_explicit_target_global_env_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+10} [test] in (env-global))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(vm, "x");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(10,                               evaluatedValue->value.integer);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(vm, "(doc x)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ("test", octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_define_my_inc_function_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define my-inc as (fn (value) (++ value)) [my own inc-function] '(value [the value to increase]) howto-no)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(doc my-inc)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ(
        "my own inc-function\n"
        "Arguments are:\n"
        "value -> the value to increase",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(my-inc {D+111})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(112,                              evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_define_factorial_function_with_integers_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define factorial as (fn (n) (if (== n {D+0}) {D+1} (* n (factorial (- n {D+1}))))) [factorial function] '(n [calculate n!]) howto-ok)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(doc factorial)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ(
        "factorial function\n"
        "Arguments are:\n"
        "n -> calculate n!",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    int32_t const expected[] = {1, 1, 2, 6, 24, 120, 720, 5040};

    char const * const input[] =
    {
        "(factorial {D+0})",
        "(factorial {D+1})",
        "(factorial {D+2})",
        "(factorial {D+3})",
        "(factorial {D+4})",
        "(factorial {D+5})",
        "(factorial {D+6})",
        "(factorial {D+7})",
    };

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        evaluatedValue =
            octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(vm, input[i]);

        ASSERT(evaluatedValue);
        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
        ASSERT_EQ(expected[i],                      evaluatedValue->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_define_factorial_function_with_reals_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define factorial as (fn (n) (if (<= n {D+0.0}) {D+1.0} (* n (factorial (- n {D+1}))))) [factorial function] '(n [calculate n!]) howto-ok)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(doc factorial)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ(
        "factorial function\n"
        "Arguments are:\n"
        "n -> calculate n!",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    double const expected[] = {1, 1, 2, 6, 24, 120, 720, 5040};

    char const * const input[] =
    {
        "(factorial {D+0.0})",
        "(factorial {D+1.0})",
        "(factorial {D+2.0})",
        "(factorial {D+3.0})",
        "(factorial {D+4.0})",
        "(factorial {D+5.0})",
        "(factorial {D+6.0})",
        "(factorial {D+7.0})",
    };

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        evaluatedValue =
            octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(vm, input[i]);

        ASSERT(evaluatedValue);
        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_REAL, evaluatedValue->typeTag);
        ASSERT_IN_RANGE(expected[i],             evaluatedValue->value.real, 0.000001);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_define_called_with_three_arguments_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+10})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Special 'define' expects four, six or eight arguments. "
        "3 arguments were given.\n"
        "\tAt form: >>>>>>>>>>(define x as {D+10})<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_define_called_with_eight_arguments_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define myEnv as (env-new) [myEnv])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn () {D+128}) [f] '() in myEnv howto-ok)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    // Make sure f is NOT defined in global environment
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "f");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Unbound symbol 'f'",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    // Make sure f IS defined in myEnv-environment
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(eval f myEnv)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_FUNCTION, evaluatedValue->typeTag);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(eval (f) myEnv)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(128,                              evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_define_called_with_four_arguments_first_being_value_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define {D+10} as x [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "The first argument of special 'define' is a vector but doesn't evaluate into "
        "a symbol. It evaluates into '{D+10}'.\n"
        "\tAt form: >>>>>>>>>>(define {D+10} as x [x])<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_define_called_with_eight_arguments_first_being_integer_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define myEnv as (env-new) [myEnv])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define {D+10} as (fn () {D+128}) [f] '() in myEnv howto-ok)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "The first argument of special 'define' is a vector but doesn't evaluate into "
        "a symbol. It evaluates into '{D+10}'.\n"
        "\tAt form: >>>>>>>>>>(define {D+10} as (fn () {D+128}) [f] (quote ()) in myEnv howto-ok)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_define_called_with_four_arguments_error_at_first_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as (noSuchFuNcTion) [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Cannot evaluate operator of type 'error' (<error>: Unbound symbol 'noSuchFuNcTion')\n"
        "\tAt form: >>>>>>>>>>(define x as (noSuchFuNcTion) [x])<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_define_called_with_four_arguments_with_docstring_being_integer_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+10} {D+20})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Special 'define' expects documentation string as the fourth argument "
        "in this context. Value '{D+20}' was given.\n"
        "\tAt form: >>>>>>>>>>(define x as {D+10} {D+20})<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_define_called_with_four_arguments_name_evaluates_into_integer_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define {D+20} as (fn () {D+10}) [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "The first argument of special 'define' is a vector but doesn't evaluate into "
        "a symbol. It evaluates into '{D+20}'.\n"
        "\tAt form: >>>>>>>>>>(define {D+20} as (fn () {D+10}) [x])<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_define_called_with_six_arguments_docstring_is_integer_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn () 'x) {D+20} '() howto-ok)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Special 'define' expects documentation string as the fourth argument in this "
        "context. Value '{D+20}' was given.\n"
        "\tAt form: >>>>>>>>>>(define f as (fn () (quote x)) {D+20} (quote ()) howto-ok)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_define_called_with_eight_arguments_error_in_environment_argument_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn () 'x) [f] '() in (noSuchFuNcTion) howto-ok)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Special 'define' expects environment as the seventh argument in this "
        "context. Value '<error>: Cannot evaluate operator of type 'error' (<error>: "
        "Unbound symbol 'noSuchFuNcTion')' was given.\n"
        "\tAt form: >>>>>>>>>>(define f as (fn () (quote x)) [f] (quote ()) in (noSuchFuNcTion) howto-ok)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_define_called_with_four_arguments_integer_as_docstring_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+20} {D+10})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Special 'define' expects documentation string as the fourth argument "
        "in this context. Value '{D+10}' was given.\n"
        "\tAt form: >>>>>>>>>>(define x as {D+20} {D+10})<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_quote_called_without_arguments_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(quote)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Special 'quote' expects one argument. 0 arguments were given.\n"
        "\tAt form: >>>>>>>>>>(quote)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_max_integers_0_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(max {D+0})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'max' expects at least 2 arguments. "
        "1 arguments were given.\n"
        "\tAt form: >>>>>>>>>>(max {D+0})<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_max_integers_0_1_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(max {D+0} {D+1})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(1,                                evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_max_integers_10_11_12_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(max {D+10} {D+11} {D+12})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(12,                               evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_max_integers_12_11_10_minus_100_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(max {D+12} {D+10} {D+11} {D-100})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(12,                               evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_max_strings_abc_abd_abe_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(max [abc] [abd] [abe])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "abe",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_max_strings_abe_abd_abc_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(max [abe] [abd] [abc])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "abe",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_min_integers_0_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(min {D+0})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'min' expects at least 2 arguments. "
        "1 arguments were given.\n"
        "\tAt form: >>>>>>>>>>(min {D+0})<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_min_integers_0_1_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(min {D+0} {D+1})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(0,                                evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_min_integers_10_11_12_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(min {D+10} {D+11} {D+12})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(10,                               evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_min_integers_12_11_10_minus_100_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(min {D+12} {D+10} {D+11} {D-100})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(-100,                             evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_min_strings_abc_abd_abe_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(min [abc] [abd] [abe])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "abc",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_min_strings_abe_abd_abc_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(min [abe] [abd] [abc])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "abc",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_distance_integers_0_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(distance {D+0})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'distance' expects two arguments. "
        "1 arguments were given.\n"
        "\tAt form: >>>>>>>>>>(distance {D+0})<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_distance_integers_0_1_2_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(distance {D+0} {D+1} {D+2})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'distance' expects two arguments. "
        "3 arguments were given.\n"
        "\tAt form: >>>>>>>>>>(distance {D+0} {D+1} {D+2})<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_distance_integers_0_1_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(distance {D+0} {D+1})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_REAL, evaluatedValue->typeTag);
    ASSERT_IN_RANGE(1, evaluatedValue->value.real, 0.000001);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_distance_reals_0dot1_3dot14_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(distance {D+0.1} {D+3.14})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_REAL, evaluatedValue->typeTag);
    ASSERT_IN_RANGE(3.04, evaluatedValue->value.real, 0.000001);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_distance_10_0dot1_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(distance {D+10} {D+0.1})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_REAL, evaluatedValue->typeTag);
    ASSERT_IN_RANGE(9.9, evaluatedValue->value.real, 0.000001);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_plus_integer_value_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+10} [test])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    for (int32_t i = 0; i < 100; ++i)
    {
        evaluatedValue =
            octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(vm, "(++ x)");

        ASSERT(evaluatedValue);
        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
        ASSERT_EQ(10 + 1 + i,                       evaluatedValue->value.integer);
    }

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(vm, "x");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(110,                              evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_doc_for_integer_value_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+10} [x-coordinate])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(vm, "(doc x)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "x-coordinate",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_read_and_eval_path_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    // TODO what about the path separator? Cross platform?
    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(read-and-eval-path [" OCTASPIRE_DERN_CONFIG_TEST_RES_PATH "octaspire_read_and_eval_path_test.dern])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(200,                              evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_read_and_eval_string_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(read-and-eval-string [(++ {D+10})])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(11,                               evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_slash_1_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(/ {D+1})");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_REAL, evaluatedValue->typeTag);
    ASSERT_IN_RANGE(1, evaluatedValue->value.real, 0.000001);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_slash_10_2_2_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(/ {D+10} {D+2} {D+2})");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_REAL, evaluatedValue->typeTag);
    ASSERT_IN_RANGE(2.5, evaluatedValue->value.real, 0.000001);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_slash_0_2_2_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(/ {D+0} {D+2} {D+2})");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_REAL, evaluatedValue->typeTag);
    ASSERT_IN_RANGE(0, evaluatedValue->value.real, 0.000001);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_slash_100_10_0dot5_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(/ {D+100} {D+10} {D+0.5})");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_REAL, evaluatedValue->typeTag);
    ASSERT_IN_RANGE(20, evaluatedValue->value.real, 0.000001);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_slash_100_minus10_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(/ {D+100} {D-10})");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_REAL, evaluatedValue->typeTag);
    ASSERT_IN_RANGE(-10, evaluatedValue->value.real, 0.000001);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_slash_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(/)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin '/' expects at least one numeric argument (integer or real).\n"
        "\tAt form: >>>>>>>>>>(/)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_slash_0_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(/ {D+0})");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "First argument to builtin '/' cannot be zero. It would cause division by zero.\n"
        "\tAt form: >>>>>>>>>>(/ {D+0})<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_slash_10_2_0_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(/ {D+10} {D+2} {D+0})");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Argument number 3 to builtin '/' cannot be zero. It would cause division by zero.\n"
        "\tAt form: >>>>>>>>>>(/ {D+10} {D+2} {D+0})<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_slash_10_2_character_a_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(/ {D+10} {D+2} |a|)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin '/' expects numeric arguments (integer or real). 3th argument has type character.\n"
        "\tAt form: >>>>>>>>>>(/ {D+10} {D+2} |a|)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_mod_5_mod_3_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(mod {D+5} {D+3})");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(2,                                evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_mod_0_mod_3_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(mod {D+0} {D+3})");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(0,                                evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_mod_3_mod_3_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(mod {D+3} {D+3})");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(0,                                evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_mod_4_mod_3_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(mod {D+4} {D+3})");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(1,                                evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_mod_4_mod_0_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(mod {D+4} {D+0})");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "The second argument to builtin 'mod' cannot be zero. "
        "It would cause division by zero.\n"
        "\tAt form: >>>>>>>>>>(mod {D+4} {D+0})<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_1_2_3_4_minus_2_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+ {D+1} {D+2} {D+3} {D+4} {D-2})");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(8,                                evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_1_2_3_4_minus_2_2dot5_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+ {D+1} {D+2} {D+3} {D+4} {D-2} {D+2.5})");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_REAL, evaluatedValue->typeTag);
    ASSERT_EQ(10.5,                          evaluatedValue->value.real);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_string_cat_dog_and_string_space_and_string_zebra_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+ [cat, dog] [ ] [and zebra])");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "cat, dog and zebra",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_string_ca_and_character_t_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+ [ca] |t|)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "cat",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_empty_string_and_characters_k_i_t_t_e_n_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+ [] |k| |i| |t| |t| |e| |n|)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "kitten",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_vector_1_2_3_and_4_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+ '({D+1} {D+2} {D+3}) {D+4})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);

    octaspire_string_t *tmpStr = octaspire_dern_value_to_string(
        evaluatedValue,
        octaspire_dern_vm_get_allocator(vm));

    ASSERT_STR_EQ(
        "({D+1} {D+2} {D+3} {D+4})",
        octaspire_string_get_c_string(tmpStr));

    octaspire_string_release(tmpStr);
    tmpStr = 0;

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_vector_1_2_3_and_4_and_a_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+ '({D+1} {D+2} {D+3}) {D+4} |a|)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);

    octaspire_string_t *tmpStr = octaspire_dern_value_to_string(
        evaluatedValue,
        octaspire_dern_vm_get_allocator(vm));

    ASSERT_STR_EQ(
        "({D+1} {D+2} {D+3} {D+4} |a|)",
        octaspire_string_get_c_string(tmpStr));

    octaspire_string_release(tmpStr);
    tmpStr = 0;

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_empty_vector_and_1_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+ '() {D+1})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);

    octaspire_string_t *tmpStr = octaspire_dern_value_to_string(
        evaluatedValue,
        octaspire_dern_vm_get_allocator(vm));

    ASSERT_STR_EQ(
        "({D+1})",
        octaspire_string_get_c_string(tmpStr));

    octaspire_string_release(tmpStr);
    tmpStr = 0;

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_empty_vector_and_empty_vector_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+ '() '())");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);

    octaspire_string_t *tmpStr = octaspire_dern_value_to_string(
        evaluatedValue,
        octaspire_dern_vm_get_allocator(vm));

    ASSERT_STR_EQ(
        "(())",
        octaspire_string_get_c_string(tmpStr));

    octaspire_string_release(tmpStr);
    tmpStr = 0;

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_empty_vector_and_empty_vector_and_empty_vector_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+ '() '() '())");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);

    octaspire_string_t *tmpStr = octaspire_dern_value_to_string(
        evaluatedValue,
        octaspire_dern_vm_get_allocator(vm));

    ASSERT_STR_EQ(
        "(() ())",
        octaspire_string_get_c_string(tmpStr));

    octaspire_string_release(tmpStr);
    tmpStr = 0;

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_empty_hash_map_and_1_and_a_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+ (hash-map) {D+1} |a|)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_HASH_MAP, evaluatedValue->typeTag);

    octaspire_string_t *tmpStr = octaspire_dern_value_to_string(
        evaluatedValue,
        octaspire_dern_vm_get_allocator(vm));

    ASSERT_STR_EQ(
        "(hash-map {D+1} |a|)",
        octaspire_string_get_c_string(tmpStr));

    octaspire_string_release(tmpStr);
    tmpStr = 0;

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_hash_map_1_a_and_2_and_b_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+ (hash-map {D+1} |a|) {D+2} |b|)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_HASH_MAP, evaluatedValue->typeTag);

    octaspire_string_t *tmpStr = octaspire_dern_value_to_string(
        evaluatedValue,
        octaspire_dern_vm_get_allocator(vm));

    ASSERT_STR_EQ(
        "(hash-map {D+2} |b|\n          {D+1} |a|)",
        octaspire_string_get_c_string(tmpStr));

    octaspire_string_release(tmpStr);
    tmpStr = 0;

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_hash_map_1_a_and_2_and_b_and_3_and_c_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+ (hash-map {D+1} |a|)   {D+2} |b|   {D+3} |c|)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_HASH_MAP, evaluatedValue->typeTag);

    octaspire_string_t *tmpStr = octaspire_dern_value_to_string(
        evaluatedValue,
        octaspire_dern_vm_get_allocator(vm));

    ASSERT_STR_EQ(
        "(hash-map {D+2} |b|\n          {D+3} |c|\n          {D+1} |a|)",
        octaspire_string_get_c_string(tmpStr));

    octaspire_string_release(tmpStr);
    tmpStr = 0;

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_semver_1_1_1_and_1_0_0_and_0_2_0_and_0_0_3_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+ 1.1.1 1.0.0 0.2.0 0.0.3)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_SEMVER, evaluatedValue->typeTag);

    octaspire_string_t *tmpStr = octaspire_dern_value_to_string(
        evaluatedValue,
        octaspire_dern_vm_get_allocator(vm));

    // Not 2.3.4, because adding major number makes minor and patch zero,
    // and adding minor number makes patch zero.
    ASSERT_STR_EQ(
        "2.2.3",
        octaspire_string_get_c_string(tmpStr));

    octaspire_string_release(tmpStr);
    tmpStr = 0;

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_minus_semver_3_4_5_and_1_0_0_and_0_2_0_and_0_0_3_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(- 3.5.7 1.0.0 0.2.0 0.0.3)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_SEMVER, evaluatedValue->typeTag);

    octaspire_string_t *tmpStr = octaspire_dern_value_to_string(
        evaluatedValue,
        octaspire_dern_vm_get_allocator(vm));

    ASSERT_STR_EQ(
        "2.3.4",
        octaspire_string_get_c_string(tmpStr));

    octaspire_string_release(tmpStr);
    tmpStr = 0;

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_minus_string_cat_dog_cat_and_string_cat_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(- [cat dog cat] [cat])");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ(" dog ", octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_minus_string_abcabcabc_and_character_a_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(- [abcabcabc] |a|)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ("bcbcbc", octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_minus_string_abcabcabc_and_character_a_and_character_b_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(- [abcabcabc] |a| |b|)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ("ccc", octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_minus_string_abcabcabc_and_character_a_and_character_b_and_caharacter_c_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(- [abcabcabc] |a| |b| |c|)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ("", octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_minus_1_2_3_4_minus_2_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(- {D+1} {D+2} {D+3} {D+4} {D-2})");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(-6,                               evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_minus_1_2_3_4_minus_2_2dot5_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(- {D+1} {D+2} {D+3} {D+4} {D-2} {D+2.5})");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_REAL, evaluatedValue->typeTag);
    ASSERT_IN_RANGE(-8.5, evaluatedValue->value.real, 0.000001);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_string_abcdeaaba_character_a_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find [abcdeaaba] |a|)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(4, octaspire_vector_get_length(evaluatedValue->value.vector));

    int32_t const expected[] = {0, 5, 6, 8};

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        octaspire_dern_value_t const * const value =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                evaluatedValue,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                (ptrdiff_t)i);

        ASSERT(value);

        ASSERT_EQ(expected[i], value->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_string_abcdeaaba_character_y_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find [abcdeaaba] |y|)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_vector_get_length(evaluatedValue->value.vector));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_string_cat_dog_cat_zebra_cat_string_cat_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find [cat dog cat zebra cat] [cat])");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(3, octaspire_vector_get_length(evaluatedValue->value.vector));

    int32_t const expected[] = {0, 8, 18};

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        octaspire_dern_value_t const * const value =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                evaluatedValue,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                (ptrdiff_t)i);

        ASSERT(value);

        ASSERT_EQ(expected[i], value->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_string_cat_dog_cat_zebra_cat_string_parrot_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find [cat dog cat zebra cat] [parrot])");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_vector_get_length(evaluatedValue->value.vector));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_string_cat_dog_cat_zebra_cat_quoted_symbol_cat_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find [cat dog cat zebra cat] 'cat)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(3, octaspire_vector_get_length(evaluatedValue->value.vector));

    int32_t const expected[] = {0, 8, 18};

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        octaspire_dern_value_t const * const value =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                evaluatedValue,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                (ptrdiff_t)i);

        ASSERT(value);

        ASSERT_EQ(expected[i], value->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_string_cat_dog_cat_zebra_cat_quoted_symbol_parrot_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find [cat dog cat zebra cat] 'parrot)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_vector_get_length(evaluatedValue->value.vector));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_string_123_124_abc_123_99_123_integer_123_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find [123 124 abc 123 99 123] {D+123})");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(3, octaspire_vector_get_length(evaluatedValue->value.vector));

    int32_t const expected[] = {0, 12, 19};

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        octaspire_dern_value_t const * const value =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                evaluatedValue,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                (ptrdiff_t)i);

        ASSERT(value);

        ASSERT_EQ(expected[i], value->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_string_123_124_abc_123_99_123_integer_777_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find [123 124 abc 123 99 123] {D+777})");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_vector_get_length(evaluatedValue->value.vector));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_string_3dot14_abc_3dot13_3dot14_3dot146_3dot14_real_3dot14_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find [3.14 abc 3.13 3.14 3.146 3.14] {D+3.14})");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(4, octaspire_vector_get_length(evaluatedValue->value.vector));

    int32_t const expected[] = {0, 14, 19, 25};

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        octaspire_dern_value_t const * const value =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                evaluatedValue,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                (ptrdiff_t)i);

        ASSERT(value);

        ASSERT_EQ(expected[i], value->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_string_3dot14_abc_3dot13_3dot14_3dot146_3dot14_real_7dot11_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find [3.14 abc 3.13 3.14 3.146 3.14] {D+7.11})");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_vector_get_length(evaluatedValue->value.vector));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_symbol_abc_def_aaxa_char_a_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find 'abc_def_aaxa |a|)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(4, octaspire_vector_get_length(evaluatedValue->value.vector));

    int32_t const expected[] = {0, 8, 9, 11};

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        octaspire_dern_value_t const * const value =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                evaluatedValue,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                (ptrdiff_t)i);

        ASSERT(value);

        ASSERT_EQ(expected[i], value->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_symbol_abc_def_aaxa_char_y_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find 'abc_def_aaxa |y|)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_vector_get_length(evaluatedValue->value.vector));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_symbol_cat_dog_cat_cat_zebra_cat_string_cat_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find 'cat_dog_cat_cat_zebra_cat [cat])");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(4, octaspire_vector_get_length(evaluatedValue->value.vector));

    int32_t const expected[] = {0, 8, 12, 22};

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        octaspire_dern_value_t const * const value =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                evaluatedValue,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                (ptrdiff_t)i);

        ASSERT(value);

        ASSERT_EQ(expected[i], value->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_symbol_cat_dog_cat_cat_zebra_cat_string_parrot_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find 'cat_dog_cat_cat_zebra_cat [parrot])");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_vector_get_length(evaluatedValue->value.vector));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_symbol_cat_dog_cat_cat_zebra_cat_symbol_cat_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find 'cat_dog_cat_cat_zebra_cat 'cat)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(4, octaspire_vector_get_length(evaluatedValue->value.vector));

    int32_t const expected[] = {0, 8, 12, 22};

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        octaspire_dern_value_t const * const value =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                evaluatedValue,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                (ptrdiff_t)i);

        ASSERT(value);

        ASSERT_EQ(expected[i], value->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_symbol_cat_dog_cat_cat_zebra_cat_symbol_parrot_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find 'cat_dog_cat_cat_zebra_cat 'parrot)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_vector_get_length(evaluatedValue->value.vector));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_symbol_cat_123_dog_cat_123_123_cat_zebra_123_integer_123_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find 'cat_123_dog_cat_123_123_cat_zebra_123 {D+123})");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(4, octaspire_vector_get_length(evaluatedValue->value.vector));

    int32_t const expected[] = {4, 16, 20, 34};

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        octaspire_dern_value_t const * const value =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                evaluatedValue,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                (ptrdiff_t)i);

        ASSERT(value);

        ASSERT_EQ(expected[i], value->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_symbol_cat_321_dog_cat_321_321_cat_zebra_321_integer_123_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find 'cat_321_dog_cat_321_321_cat_zebra_321 {D+123})");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_vector_get_length(evaluatedValue->value.vector));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_symbol_cat_3dot14_dog_cat_3dot14_3dot14_cat_zebra_3dot14_real_3dot14_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find 'cat_3.14_dog_cat_3.14_3.14_cat_zebra_3.14 {D+3.14})");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(4, octaspire_vector_get_length(evaluatedValue->value.vector));

    int32_t const expected[] = {4, 17, 22, 37};

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        octaspire_dern_value_t const * const value =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                evaluatedValue,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                (ptrdiff_t)i);

        ASSERT(value);

        ASSERT_EQ(expected[i], value->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_symbol_cat_3dot14_dog_cat_3dot14_3dot14_cat_zebra_3dot14_real_4dot14_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find 'cat_3.14_dog_cat_3.14_3.14_cat_zebra_3.14 {D+4.14})");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_vector_get_length(evaluatedValue->value.vector));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_vector_of_chars_a_b_c_a_a_g_u_a_char_a_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find '(|a| |b| |c| |a| |a| |g| |u| |a|) |a|)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(4, octaspire_vector_get_length(evaluatedValue->value.vector));

    int32_t const expected[] = {0, 3, 4, 7};

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        octaspire_dern_value_t const * const value =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                evaluatedValue,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                (ptrdiff_t)i);

        ASSERT(value);

        ASSERT_EQ(expected[i], value->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_vector_of_chars_a_b_c_a_a_g_u_a_char_x_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find '(|a| |b| |c| |a| |a| |g| |u| |a|) |x|)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_vector_get_length(evaluatedValue->value.vector));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_vector_of_strings_cat_dog_cat_zebra_parrot_cat_string_cat_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find '([cat] [dog] [cat] [zebra] [parrot] [cat]) [cat])");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(3, octaspire_vector_get_length(evaluatedValue->value.vector));

    int32_t const expected[] = {0, 2, 5};

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        octaspire_dern_value_t const * const value =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                evaluatedValue,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                (ptrdiff_t)i);

        ASSERT(value);

        ASSERT_EQ(expected[i], value->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_vector_of_strings_cat_dog_cat_zebra_parrot_cat_string_kitten_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find '([cat] [dog] [cat] [zebra] [parrot] [cat]) [kitten])");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_vector_get_length(evaluatedValue->value.vector));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_vector_of_symbols_cat_dog_cat_zebra_parrot_cat_symbol_cat_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find '(cat dog cat zebra parrot cat) 'cat)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(3, octaspire_vector_get_length(evaluatedValue->value.vector));

    int32_t const expected[] = {0, 2, 5};

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        octaspire_dern_value_t const * const value =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                evaluatedValue,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                (ptrdiff_t)i);

        ASSERT(value);

        ASSERT_EQ(expected[i], value->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_vector_of_symbols_cat_dog_cat_zebra_parrot_cat_symbol_mouse_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find '(cat dog cat zebra parrot cat) 'mouse)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_vector_get_length(evaluatedValue->value.vector));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_hash_map_of_chars_a_b_c_d_char_c_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find (hash-map |a| {D+1} |b| {D+2} |c| {D+3} |d| {D+4}) |c|)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(3,                                evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_hash_map_of_chars_a_b_c_d_char_x_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find (hash-map |a| {D+1} |b| {D+2} |c| {D+3} |d| {D+4}) |x|)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_NIL, evaluatedValue->typeTag);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_hash_map_of_chars_a_b_c_d_char_b_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find (hash-map |a| true |b| false |c| nil |d| {D+4}) |b|)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_hash_map_of_strings_cat_dog_zebra_kitten_string_zebra_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find (hash-map [cat] |a| [dog] |b| [zebra] |c| [kitten] |d|) [zebra])");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, evaluatedValue->typeTag);
    ASSERT_STR_EQ("c", octaspire_string_get_c_string(evaluatedValue->value.character));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_hash_map_of_strings_cat_dog_zebra_kitten_string_parrot_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find (hash-map [cat] |a| [dog] |b| [zebra] |c| [kitten] |d|) [parrot])");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_NIL, evaluatedValue->typeTag);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_hash_map_of_integers_1_2_3_4_integer_2_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find (hash-map {D+1} [cat] {D+2} [dog] {D+3} [zebra] {D+4} [kitten]) {D+2})");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ("dog", octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_hash_map_of_integers_1_2_3_4_integer_5_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find (hash-map {D+1} [cat] {D+2} [dog] {D+3} [zebra] {D+4} [kitten]) {D+5})");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_NIL, evaluatedValue->typeTag);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_hash_map_of_reals_2dot1_2dot2_2dot3_2dot4_real_2dot2_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find (hash-map {D+2.1} {D+1}   {D+2.2} {D+2}   {D+2.3} {D+3}   {D+2.4} {D+4}) {D+2.2})");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(2,                                evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_hash_map_of_reals_2dot1_2dot2_2dot3_2dot4_real_2dot5_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find (hash-map {D+2.1} {D+1}   {D+2.2} {D+2}   {D+2.3} {D+3}   {D+2.4} {D+4}) {D+2.5})");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_NIL, evaluatedValue->typeTag);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_global_environment_symbol_find_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find (env-global) 'find)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BUILTIN, evaluatedValue->typeTag);
    ASSERT_EQ(octaspire_dern_vm_builtin_find,   evaluatedValue->value.builtin->cFunction);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_global_environment_defined_symbol_xyz_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define xyz as '({D+1} {D+2} {D+3}) [xyz] )");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find (env-global) 'xyz)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(3, octaspire_dern_value_as_vector_get_length(evaluatedValue));

    for (int32_t i = 0; i < 3; ++i)
    {
        octaspire_dern_value_t const * const tmp =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                evaluatedValue,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                i);

        ASSERT(tmp);
        ASSERT_EQ(i + 1, tmp->value.integer);
    }


    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_global_environment_symbol_notfound_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find (env-global) 'notfound)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_NIL, evaluatedValue->typeTag);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_while_with_one_value_to_repeat_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define i as {D+0} [i])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(while (<= i {D+100}) (++ i))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(101,                              evaluatedValue->value.integer);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "i");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(101,                              evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_while_with_two_values_to_repeat_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define i as {D+0} [i])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define j as {D+1000} [j])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(while (<= i {D+100}) (++ i) (-- j))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(101,                              evaluatedValue->value.integer);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "i");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(101,                              evaluatedValue->value.integer);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "j");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(899,                              evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_while_called_with_one_argument_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(while true)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Special 'while' expects at least two arguments. 1 arguments were given.\n"
        "\tAt form: >>>>>>>>>>(while true)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message)); 

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_while_called_with_integer_as_first_argument_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+0} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(while {D+1} (++ x))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "First argument to special 'while' must evaluate into boolean value. Now it evaluated "
        "into type integer.\n"
        "\tAt form: >>>>>>>>>>(while {D+1} (++ x))<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message)); 

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_hash_map_empty_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(hash-map)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_HASH_MAP, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_dern_value_as_hash_map_get_number_of_elements(evaluatedValue));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_hash_map_one_element_symbol_one_1_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(hash-map 'one {D+1})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_HASH_MAP, evaluatedValue->typeTag);
    ASSERT_EQ(1, octaspire_dern_value_as_hash_map_get_number_of_elements(evaluatedValue));

    octaspire_map_element_t *element =
        octaspire_dern_value_as_hash_map_get_at_index(evaluatedValue, 0);

    ASSERT(element);

    evaluatedValue = octaspire_map_element_get_key(element);
    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_SYMBOL, evaluatedValue->typeTag);
    ASSERT_STR_EQ("one", octaspire_string_get_c_string(evaluatedValue->value.symbol));

    evaluatedValue = octaspire_map_element_get_value(element);
    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(1, evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_hash_map_one_element_1_symbol_one_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(hash-map {D+1} 'one)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_HASH_MAP, evaluatedValue->typeTag);
    ASSERT_EQ(1, octaspire_dern_value_as_hash_map_get_number_of_elements(evaluatedValue));

    octaspire_map_element_t *element =
        octaspire_dern_value_as_hash_map_get_at_index(evaluatedValue, 0);

    ASSERT(element);

    evaluatedValue = octaspire_map_element_get_key(element);
    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(1, evaluatedValue->value.integer);

    evaluatedValue = octaspire_map_element_get_value(element);
    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_SYMBOL, evaluatedValue->typeTag);
    ASSERT_STR_EQ("one", octaspire_string_get_c_string(evaluatedValue->value.symbol));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_hash_map_two_elements_strings_dog_barks_and_sun_shines_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(hash-map [dog] [barks] [sun] [shines])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_HASH_MAP, evaluatedValue->typeTag);
    ASSERT_EQ(2, octaspire_dern_value_as_hash_map_get_number_of_elements(evaluatedValue));

    ASSERT(octaspire_dern_vm_push_value(vm, evaluatedValue));

    char const * const keys[] =
    {
        "dog",
        "sun"
    };

    char const * const expected[] =
    {
        "barks",
        "shines"
    };

    ASSERT_EQ(sizeof(keys) / sizeof(keys[0]), sizeof(expected) / sizeof(expected[0]));

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        octaspire_dern_value_t *keyValue = octaspire_dern_vm_create_new_value_string_from_c_string(
            vm,
            keys[i]);

        ASSERT(keyValue);

        octaspire_map_element_t *element = octaspire_dern_value_as_hash_map_get(
            evaluatedValue,
            octaspire_dern_value_get_hash(keyValue),
            keyValue);

        ASSERT(element);

        octaspire_dern_value_t *valueValue = octaspire_map_element_get_value(element);
        ASSERT(valueValue);
        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, valueValue->typeTag);
        ASSERT_STR_EQ(expected[i], octaspire_string_get_c_string(valueValue->value.string));
    }

    ASSERT(octaspire_dern_vm_pop_value(vm, evaluatedValue));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_string_literal_with_embedded_characters_t_bar_newline_tab_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "[ca|t||bar|dog and parrot |newline||tab| 123]");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING,   evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "cat|dog and parrot \n\t 123",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_string_literal_with_embedded_characters_c_a_t__a_n_d__d_o_g_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "[|c||a||t|| ||a||n||d|| ||d||o||g|]");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING,   evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "cat and dog",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_string_literal_with_embedded_character_newline_failure_on_missing_end_delimiter_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "[|newline]");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR,   evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Problem with a character embedded in string: (character is not complete)",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_string_literal_with_embedded_character_yy_failure_on_unknown_char_constant_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "[|yy|]");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR,   evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Problem with a character embedded in string: (Unknown character constant |yy|)",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_string_literal_with_embedded_character_in_hex_A9_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "[|A9|]");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING,   evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "©",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_string_literal_with_embedded_character_in_hex_a9_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "[|a9|]");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING,   evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "©",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_string_literal_with_embedded_character_a_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "[|a|]");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING,   evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "a",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_string_literal_with_embedded_character_in_hex_0x10000_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "[|10000|]");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING,   evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "𐀀",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_string_literal_with_embedded_character_in_hex_failure_on_too_many_hex_digits_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "[|123456789|]");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR,   evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Problem with a character embedded in string: (Number of hex digits (9) in character definition may not be larger than eight)",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_with_empty_vector_of_strings_and_integer_10_and_string_cat_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v as '() [v])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "v");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(0,                               octaspire_dern_value_as_vector_get_length(evaluatedValue));

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(= v {D+10} [cat])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(11,                              octaspire_dern_value_as_vector_get_length(evaluatedValue));

    for (size_t i = 0; i < octaspire_dern_value_as_vector_get_length(evaluatedValue); ++i)
    {
        octaspire_dern_value_t const * const element =
            octaspire_dern_value_as_vector_get_element_at_const(
                evaluatedValue,
                (ptrdiff_t)i);

        ASSERT(element);

        if (octaspire_dern_value_as_vector_is_valid_index(
                evaluatedValue,
                (ptrdiff_t)(i + 1)))
        {
            ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_NIL, element->typeTag);
        }
        else
        {
            ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, element->typeTag);

            ASSERT_STR_EQ(
                "cat",
                octaspire_string_get_c_string(element->value.string));
        }
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_with_vector_of_strings_and_integer_1_and_string_cat_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v as '([first] [second] [third]) [v])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "v");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(3,                               octaspire_dern_value_as_vector_get_length(evaluatedValue));

    octaspire_dern_value_t const *vecElemVal =
        octaspire_dern_value_as_vector_get_element_at_const(evaluatedValue, 0);

    ASSERT(vecElemVal);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, vecElemVal->typeTag);
    ASSERT_STR_EQ("first", octaspire_dern_value_as_string_get_c_string(vecElemVal));

    vecElemVal = octaspire_dern_value_as_vector_get_element_at_const(evaluatedValue, 1);
    ASSERT(vecElemVal);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, vecElemVal->typeTag);
    ASSERT_STR_EQ("second", octaspire_dern_value_as_string_get_c_string(vecElemVal));

    vecElemVal = octaspire_dern_value_as_vector_get_element_at_const(evaluatedValue, 2);
    ASSERT(vecElemVal);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, vecElemVal->typeTag);
    ASSERT_STR_EQ("third", octaspire_dern_value_as_string_get_c_string(vecElemVal));

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(= v {D+1} [cat])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(3,                               octaspire_dern_value_as_vector_get_length(evaluatedValue));

    vecElemVal = octaspire_dern_value_as_vector_get_element_at_const(evaluatedValue, 0);
    ASSERT(vecElemVal);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, vecElemVal->typeTag);
    ASSERT_STR_EQ("first", octaspire_dern_value_as_string_get_c_string(vecElemVal));

    vecElemVal = octaspire_dern_value_as_vector_get_element_at_const(evaluatedValue, 1);
    ASSERT(vecElemVal);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, vecElemVal->typeTag);
    ASSERT_STR_EQ("cat", octaspire_dern_value_as_string_get_c_string(vecElemVal));

    vecElemVal = octaspire_dern_value_as_vector_get_element_at_const(evaluatedValue, 2);
    ASSERT(vecElemVal);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, vecElemVal->typeTag);
    ASSERT_STR_EQ("third", octaspire_dern_value_as_string_get_c_string(vecElemVal));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_with_hash_map_and_same_key_inserted_multiple_times_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define target as (hash-map |a| {D+1}) [target])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);



    // First addition
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(= target |a| {D+2})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_HASH_MAP, evaluatedValue->typeTag);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "target");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_HASH_MAP, evaluatedValue->typeTag);

    octaspire_map_t * hashMap = evaluatedValue->value.hashMap;

    ASSERT_EQ(1, octaspire_map_get_number_of_elements(hashMap));

    octaspire_map_element_t *element =
        octaspire_map_get_at_index(hashMap, 0);

    ASSERT(element);

    octaspire_dern_value_t *key = octaspire_map_element_get_key(element);
    ASSERT(key);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, key->typeTag);
    ASSERT_STR_EQ("a", octaspire_string_get_c_string(key->value.string));

    octaspire_dern_value_t *value = octaspire_map_element_get_value(element);
    ASSERT(value);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, value->typeTag);
    ASSERT_EQ(2,                                 value->value.integer);



    // Second addition
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(= target |a| {D+3})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_HASH_MAP, evaluatedValue->typeTag);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "target");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_HASH_MAP, evaluatedValue->typeTag);

    hashMap = evaluatedValue->value.hashMap;

    ASSERT_EQ(1, octaspire_map_get_number_of_elements(hashMap));

    element =
        octaspire_map_get_at_index(hashMap, 0);

    ASSERT(element);

    key = octaspire_map_element_get_key(element);
    ASSERT(key);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, key->typeTag);
    ASSERT_STR_EQ("a", octaspire_string_get_c_string(key->value.string));

    value = octaspire_map_element_get_value(element);
    ASSERT(value);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, value->typeTag);
    ASSERT_EQ(3,                                 value->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_with_hash_map_and_hash_map_with_elements_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define target as (hash-map) [target])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define source as (hash-map {D+1} |a| {D+2} |b|) [source])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(= target source)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_HASH_MAP, evaluatedValue->typeTag);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "target");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_HASH_MAP, evaluatedValue->typeTag);

    octaspire_map_t * const hashMap = evaluatedValue->value.hashMap;

    ASSERT_EQ(2, octaspire_map_get_number_of_elements(hashMap));

    octaspire_map_element_t *element =
        octaspire_map_get_at_index(hashMap, 1);

    ASSERT(element);

    octaspire_dern_value_t *key = octaspire_map_element_get_key(element);
    ASSERT(key);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, key->typeTag);
    ASSERT_EQ(1,                                key->value.integer);

    octaspire_dern_value_t *value = octaspire_map_element_get_value(element);
    ASSERT(value);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, value->typeTag);
    ASSERT_STR_EQ("a", octaspire_string_get_c_string(value->value.character));

    element =
        octaspire_map_get_at_index(hashMap, 0);

    ASSERT(element);

    key = octaspire_map_element_get_key(element);
    ASSERT(key);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, key->typeTag);
    ASSERT_EQ(2,                                key->value.integer);

    value = octaspire_map_element_get_value(element);
    ASSERT(value);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, value->typeTag);
    ASSERT_STR_EQ("b", octaspire_string_get_c_string(value->value.character));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_with_hash_map_and_empty_hash_map_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define target as (hash-map {D+1} |a|) [target])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define source as (hash-map) [source])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(= target source)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_HASH_MAP, evaluatedValue->typeTag);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "target");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_HASH_MAP, evaluatedValue->typeTag);

    octaspire_map_t * const hashMap = evaluatedValue->value.hashMap;

    ASSERT_EQ(0, octaspire_map_get_number_of_elements(hashMap));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_with_vector_and_vector_with_elements_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define target as '() [target])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define source as '({D+1} |a|) [source])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(= target source)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "target");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);

    ASSERT_EQ(2, octaspire_dern_value_as_vector_get_length(evaluatedValue));

    octaspire_dern_value_t const * value =
        octaspire_dern_value_as_vector_get_element_at_const(evaluatedValue, 0);

    ASSERT(value);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, value->typeTag);
    ASSERT_EQ(1,                                value->value.integer);

    value =
        octaspire_dern_value_as_vector_get_element_at_const(evaluatedValue, 1);

    ASSERT(value);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, value->typeTag);
    ASSERT_STR_EQ("a", octaspire_string_get_c_string(value->value.character));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_with_vector_and_empty_vector_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define target as '({D+1} {D+2} {D+3}) [target])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define source as '() [source])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(= target source)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "target");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);

    ASSERT_EQ(0, octaspire_dern_value_as_vector_get_length(evaluatedValue));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_with_string_and_index_and_character_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s as [Pong] [s])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(= s {D+1} |ö|)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ("Pöng", octaspire_dern_value_as_string_get_c_string(evaluatedValue));

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "s");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ("Pöng", octaspire_dern_value_as_string_get_c_string(evaluatedValue));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_minus_equals_with_character_x_and_integer_2_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define c as |x| [c])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(-= c {D+2})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, evaluatedValue->typeTag);
    ASSERT_STR_EQ("v", octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_minus_equals_with_character_x_and_character_exclamation_mark_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define c as |x| [c])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(-= c |!|)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, evaluatedValue->typeTag);
    ASSERT_STR_EQ("W", octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_minus_equals_with_real_3_dot_14_and_reals_1_dot_0_and_zero_dot_14_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define r as {D+3.14} [r])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(-= r {D+1.0} {D+0.14})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_REAL, evaluatedValue->typeTag);
    ASSERT_IN_RANGE(2.0, evaluatedValue->value.real, 0.000001);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_minus_equals_with_integer_10_and_integers_1_and_2_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define i as {D+10} [i])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(-= i {D+1} {D+2})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(7, evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_minus_equals_with_vector_1_1_2_2_2_3_and_values_1_and_2_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v as '({D+1} {D+1} {D+2} {D+2} {D+2} {D+3}) [v])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(-= v {D+1} {D+2})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(1, octaspire_dern_value_as_vector_get_length(evaluatedValue));

    octaspire_dern_value_t const * const value = octaspire_dern_value_as_vector_get_element_at_const(evaluatedValue, 0);
    ASSERT(value);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER,  value->typeTag);
    ASSERT_EQ(3,                                 value->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_minus_equals_with_string_abcd_and_characters_a_and_b_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s as [abcd] [s])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(-= s |a| |b|)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ("cd", octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_minus_equals_with_hash_map_1_a_2_b_3_c_and_value_1_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define h as (hash-map {D+1} |a| {D+2} |b| {D+3} |c|) [h])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(-= h {D+2} {D+3})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_HASH_MAP, evaluatedValue->typeTag);
    ASSERT_EQ(1, octaspire_dern_value_as_hash_map_get_number_of_elements(evaluatedValue));

    octaspire_map_element_t const * const element =
        octaspire_dern_value_as_hash_map_get_at_index(evaluatedValue, 0);

    ASSERT(element);

    octaspire_dern_value_t const * const key = octaspire_map_element_get_key(element);
    ASSERT(key);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER,  key->typeTag);
    ASSERT_EQ(1,                                 key->value.integer);

    octaspire_dern_value_t const * const value = octaspire_map_element_get_value(element);
    ASSERT(value);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, value->typeTag);
    ASSERT_STR_EQ("a", octaspire_string_get_c_string(value->value.character));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_minus_equals_with_vector_1_2_3_3_and_ln_at_v_minus_1_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v as '({D+1} {D+2} {D+3} {D+3}) [v])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(-= v (ln@ v {D-1}))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);

    octaspire_string_t *tmpStr = octaspire_dern_value_to_string(
        evaluatedValue,
        octaspire_dern_vm_get_allocator(vm));

    ASSERT_STR_EQ(
        "({D+1} {D+2})",
        octaspire_string_get_c_string(tmpStr));

    octaspire_string_release(tmpStr);
    tmpStr = 0;

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_minus_equals_equals_with_vector_1_2_3_3_and_ln_at_v_minus_1_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v as '({D+1} {D+2} {D+3} {D+3}) [v])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(-== v (ln@ v {D-1}))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);

    octaspire_string_t *tmpStr = octaspire_dern_value_to_string(
        evaluatedValue,
        octaspire_dern_vm_get_allocator(vm));

    ASSERT_STR_EQ(
        "({D+1} {D+2} {D+3})",
        octaspire_string_get_c_string(tmpStr));

    octaspire_string_release(tmpStr);
    tmpStr = 0;

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}



TEST octaspire_dern_vm_builtin_pop_front_with_vector_1_2_3_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v as '({D+1} {D+2} {D+3}) [v])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    char const * const expected[] =
    {
        "({D+2} {D+3})",
        "({D+3})",
        "()",
        "()",
        "()"
    };

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        evaluatedValue =
            octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
                vm,
                "(pop-front v)");

        ASSERT(evaluatedValue);
        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);

        octaspire_string_t *tmpStr = octaspire_dern_value_to_string(
            evaluatedValue,
            octaspire_dern_vm_get_allocator(vm));

        ASSERT_STR_EQ(
            expected[i],
            octaspire_string_get_c_string(tmpStr));

        octaspire_string_release(tmpStr);
        tmpStr = 0;
    }

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(len v)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_dern_value_as_integer_get_value(evaluatedValue));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_pop_front_with_string_abc_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s as [abc] [s])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    char const * const expected[] =
    {
        "[bc]",
        "[c]",
        "[]",
        "[]",
        "[]"
    };

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        evaluatedValue =
            octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
                vm,
                "(pop-front s)");

        ASSERT(evaluatedValue);
        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

        octaspire_string_t *tmpStr = octaspire_dern_value_to_string(
            evaluatedValue,
            octaspire_dern_vm_get_allocator(vm));

        ASSERT_STR_EQ(
            expected[i],
            octaspire_string_get_c_string(tmpStr));

        octaspire_string_release(tmpStr);
        tmpStr = 0;
    }

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(len s)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_dern_value_as_integer_get_value(evaluatedValue));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_pop_front_with_symbol_abc_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s as 'abc [s])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    char const * const expected[] =
    {
        "bc",
        "c",
        "",
        "",
        ""
    };

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        evaluatedValue =
            octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
                vm,
                "(pop-front s)");

        ASSERT(evaluatedValue);
        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_SYMBOL, evaluatedValue->typeTag);

        octaspire_string_t *tmpStr = octaspire_dern_value_to_string(
            evaluatedValue,
            octaspire_dern_vm_get_allocator(vm));

        ASSERT_STR_EQ(
            expected[i],
            octaspire_string_get_c_string(tmpStr));

        octaspire_string_release(tmpStr);
        tmpStr = 0;
    }

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(len s)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_dern_value_as_integer_get_value(evaluatedValue));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_pop_front_with_list_1_2_3_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define l as (list {D+1} {D+2} {D+3}) [l])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    char const * const expected[] =
    {
        "(list {D+2} {D+3})",
        "(list {D+3})",
        "(list )",
        "(list )",
        "(list )"
    };

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        evaluatedValue =
            octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
                vm,
                "(pop-front l)");

        ASSERT(evaluatedValue);
        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_LIST, evaluatedValue->typeTag);

        octaspire_string_t *tmpStr = octaspire_dern_value_to_string(
            evaluatedValue,
            octaspire_dern_vm_get_allocator(vm));

        ASSERT_STR_EQ(
            expected[i],
            octaspire_string_get_c_string(tmpStr));

        octaspire_string_release(tmpStr);
        tmpStr = 0;
    }

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(len l)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_dern_value_as_integer_get_value(evaluatedValue));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_pop_front_with_integer_10_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(pop-front {D+10})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR,  evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "First argument to builtin 'pop-front' cannot be of type 'integer'.\n"
        "\tAt form: >>>>>>>>>>(pop-front {D+10})<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}




TEST octaspire_dern_vm_builtin_pop_back_with_vector_1_2_3_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v as '({D+1} {D+2} {D+3}) [v])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    char const * const expected[] =
    {
        "({D+1} {D+2})",
        "({D+1})",
        "()",
        "()",
        "()"
    };

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        evaluatedValue =
            octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
                vm,
                "(pop-back v)");

        ASSERT(evaluatedValue);
        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);

        octaspire_string_t *tmpStr = octaspire_dern_value_to_string(
            evaluatedValue,
            octaspire_dern_vm_get_allocator(vm));

        ASSERT_STR_EQ(
            expected[i],
            octaspire_string_get_c_string(tmpStr));

        octaspire_string_release(tmpStr);
        tmpStr = 0;
    }

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(len v)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_dern_value_as_integer_get_value(evaluatedValue));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_pop_back_with_string_abc_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s as [abc] [s])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    char const * const expected[] =
    {
        "[ab]",
        "[a]",
        "[]",
        "[]",
        "[]"
    };

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        evaluatedValue =
            octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
                vm,
                "(pop-back s)");

        ASSERT(evaluatedValue);
        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

        octaspire_string_t *tmpStr = octaspire_dern_value_to_string(
            evaluatedValue,
            octaspire_dern_vm_get_allocator(vm));

        ASSERT_STR_EQ(
            expected[i],
            octaspire_string_get_c_string(tmpStr));

        octaspire_string_release(tmpStr);
        tmpStr = 0;
    }

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(len s)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_dern_value_as_integer_get_value(evaluatedValue));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_pop_back_with_symbol_abc_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s as 'abc [s])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    char const * const expected[] =
    {
        "ab",
        "a",
        "",
        "",
        ""
    };

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        evaluatedValue =
            octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
                vm,
                "(pop-back s)");

        ASSERT(evaluatedValue);
        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_SYMBOL, evaluatedValue->typeTag);

        octaspire_string_t *tmpStr = octaspire_dern_value_to_string(
            evaluatedValue,
            octaspire_dern_vm_get_allocator(vm));

        ASSERT_STR_EQ(
            expected[i],
            octaspire_string_get_c_string(tmpStr));

        octaspire_string_release(tmpStr);
        tmpStr = 0;
    }

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(len s)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_dern_value_as_integer_get_value(evaluatedValue));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_pop_back_with_list_1_2_3_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define l as (list {D+1} {D+2} {D+3}) [l])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    char const * const expected[] =
    {
        "(list {D+1} {D+2})",
        "(list {D+1})",
        "(list )",
        "(list )",
        "(list )"
    };

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        evaluatedValue =
            octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
                vm,
                "(pop-back l)");

        ASSERT(evaluatedValue);
        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_LIST, evaluatedValue->typeTag);

        octaspire_string_t *tmpStr = octaspire_dern_value_to_string(
            evaluatedValue,
            octaspire_dern_vm_get_allocator(vm));

        ASSERT_STR_EQ(
            expected[i],
            octaspire_string_get_c_string(tmpStr));

        octaspire_string_release(tmpStr);
        tmpStr = 0;
    }

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(len l)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_dern_value_as_integer_get_value(evaluatedValue));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_pop_back_with_queue_1_2_3_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define q as (queue {D+1} {D+2} {D+3}) [q])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    char const * const expected[] =
    {
        "(queue {D+2} {D+3})",
        "(queue {D+3})",
        "(queue )",
        "(queue )",
        "(queue )"
    };

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        evaluatedValue =
            octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
                vm,
                "(pop-back q)");

        ASSERT(evaluatedValue);
        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_QUEUE, evaluatedValue->typeTag);

        octaspire_string_t *tmpStr = octaspire_dern_value_to_string(
            evaluatedValue,
            octaspire_dern_vm_get_allocator(vm));

        ASSERT_STR_EQ(
            expected[i],
            octaspire_string_get_c_string(tmpStr));

        octaspire_string_release(tmpStr);
        tmpStr = 0;
    }

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(len q)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_dern_value_as_integer_get_value(evaluatedValue));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_pop_back_with_integer_10_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(pop-back {D+10})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR,  evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "First argument to builtin 'pop-back' cannot be of type 'integer'.\n"
        "\tAt form: >>>>>>>>>>(pop-back {D+10})<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_equals_with_hash_map_and_hash_map_1_a_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define h as (hash-map)[h])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+= h (hash-map {D+1} |a|))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_HASH_MAP, evaluatedValue->typeTag);
    ASSERT_EQ(1, octaspire_dern_value_as_hash_map_get_number_of_elements(evaluatedValue));

    octaspire_map_element_t const * const element =
        octaspire_dern_value_as_hash_map_get_at_index(evaluatedValue, 0);

    ASSERT(element);

    octaspire_dern_value_t const * const key = octaspire_map_element_get_key(element);
    ASSERT(key);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER,  key->typeTag);
    ASSERT_EQ(1,                                 key->value.integer);

    octaspire_dern_value_t const * const value = octaspire_map_element_get_value(element);
    ASSERT(value);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, value->typeTag);
    ASSERT_STR_EQ("a", octaspire_string_get_c_string(value->value.character));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_equals_with_hash_map_and_1_a_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define h as (hash-map) [h])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+= h {D+1} |a|)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_HASH_MAP, evaluatedValue->typeTag);
    ASSERT_EQ(1, octaspire_dern_value_as_hash_map_get_number_of_elements(evaluatedValue));

    octaspire_map_element_t const * const element =
        octaspire_dern_value_as_hash_map_get_at_index(evaluatedValue, 0);

    ASSERT(element);

    octaspire_dern_value_t const * const key = octaspire_map_element_get_key(element);
    ASSERT(key);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER,  key->typeTag);
    ASSERT_EQ(1,                                 key->value.integer);

    octaspire_dern_value_t const * const value = octaspire_map_element_get_value(element);
    ASSERT(value);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, value->typeTag);
    ASSERT_STR_EQ("a", octaspire_string_get_c_string(value->value.character));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_equals_with_hash_map_and_list_1_a_2_b_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define h as (hash-map) [h])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+= h '({D+1} |a| {D+2} |b|))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_HASH_MAP, evaluatedValue->typeTag);
    ASSERT_EQ(2, octaspire_dern_value_as_hash_map_get_number_of_elements(evaluatedValue));

    octaspire_map_element_t const * element =
        octaspire_dern_value_as_hash_map_get_at_index(evaluatedValue, 1);

    ASSERT(element);

    octaspire_dern_value_t const * key = octaspire_map_element_get_key(element);
    ASSERT(key);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER,  key->typeTag);
    ASSERT_EQ(1,                                 key->value.integer);

    octaspire_dern_value_t const * value = octaspire_map_element_get_value(element);
    ASSERT(value);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, value->typeTag);
    ASSERT_STR_EQ("a", octaspire_string_get_c_string(value->value.character));

    element =
        octaspire_dern_value_as_hash_map_get_at_index(evaluatedValue, 0);

    ASSERT(element);

    key = octaspire_map_element_get_key(element);
    ASSERT(key);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER,  key->typeTag);
    ASSERT_EQ(2,                                 key->value.integer);

    value = octaspire_map_element_get_value(element);
    ASSERT(value);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, value->typeTag);
    ASSERT_STR_EQ("b", octaspire_string_get_c_string(value->value.character));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_equals_with_vector_1_2_3_and_4_5_6_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v as '({D+1} {D+2} {D+3}) [v])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+= v {D+4} {D+5} {D+6})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(6,                               octaspire_dern_value_as_vector_get_length(evaluatedValue));

    for (size_t i = 0; i < octaspire_dern_value_as_vector_get_length(evaluatedValue); ++i)
    {
        octaspire_dern_value_t const * const element =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                evaluatedValue,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                (ptrdiff_t)i);

        ASSERT   (element);
        ASSERT_EQ((int32_t)(i+1), element->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_equals_with_string_abc_and_string_def_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s as [abc] [target string])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+= s [def])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    char const * const expected = "abcdef";

    ASSERT_STR_EQ(
        expected,
        octaspire_string_get_c_string(evaluatedValue->value.string));

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "s");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        expected,
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_equals_with_string_abc_and_symbol_def_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s as [abc] [target string])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+= s 'def)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    char const * const expected = "abcdef";

    ASSERT_STR_EQ(
        expected,
        octaspire_string_get_c_string(evaluatedValue->value.string));

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "s");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        expected,
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_equals_with_string_abc_and_character_d_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s as [abc] [target string])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+= s |d|)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    char const * const expected = "abcd";

    ASSERT_STR_EQ(
        expected,
        octaspire_string_get_c_string(evaluatedValue->value.string));

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "s");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        expected,
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_equals_with_string_abc_and_vector_of_strings_def_ghi_jkl_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s as [abc] [target string])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+= s '([def] [ghi] [jkl]))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    char const * const expected = "abcdefghijkl";

    ASSERT_STR_EQ(
        expected,
        octaspire_string_get_c_string(evaluatedValue->value.string));

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "s");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        expected,
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_equals_with_string_abc_and_nil_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s as [abc] [target string])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+= s nil)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    char const * const expected = "abcnil";

    ASSERT_STR_EQ(
        expected,
        octaspire_string_get_c_string(evaluatedValue->value.string));

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "s");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        expected,
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_equals_with_string_abc_and_booleans_true_and_false_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s as [abc] [target string])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+= s true false)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    char const * const expected = "abctruefalse";

    ASSERT_STR_EQ(
        expected,
        octaspire_string_get_c_string(evaluatedValue->value.string));

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "s");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        expected,
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_equals_with_string_abc_and_integer_128_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s as [abc] [target string])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+= s {D+128})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    char const * const expected = "abc{D+128}";

    ASSERT_STR_EQ(
        expected,
        octaspire_string_get_c_string(evaluatedValue->value.string));

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "s");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        expected,
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_equals_with_string_abc_and_real_3dot14_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s as [abc] [target string])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+= s {D+3.14})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    char const * const expected = "abc{D+3.14}";

    ASSERT_STR_EQ(
        expected,
        octaspire_string_get_c_string(evaluatedValue->value.string));

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "s");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        expected,
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}





TEST octaspire_dern_vm_builtin_plus_equals_with_symbol_abc_and_string_def_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s as 'abc [target symbol])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+= s [def])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_SYMBOL, evaluatedValue->typeTag);

    char const * const expected = "abcdef";

    ASSERT_STR_EQ(
        expected,
        octaspire_string_get_c_string(evaluatedValue->value.symbol));

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "s");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_SYMBOL, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        expected,
        octaspire_string_get_c_string(evaluatedValue->value.symbol));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_equals_with_symbol_abc_and_symbol_def_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s as 'abc [target symbol])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+= s 'def)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_SYMBOL, evaluatedValue->typeTag);

    char const * const expected = "abcdef";

    ASSERT_STR_EQ(
        expected,
        octaspire_string_get_c_string(evaluatedValue->value.symbol));

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "s");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_SYMBOL, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        expected,
        octaspire_string_get_c_string(evaluatedValue->value.symbol));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_equals_with_symbol_abc_and_character_d_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s as 'abc [target symbol])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+= s |d|)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_SYMBOL, evaluatedValue->typeTag);

    char const * const expected = "abcd";

    ASSERT_STR_EQ(
        expected,
        octaspire_string_get_c_string(evaluatedValue->value.symbol));

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "s");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_SYMBOL, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        expected,
        octaspire_string_get_c_string(evaluatedValue->value.symbol));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_equals_with_symbol_abc_and_vector_of_strings_def_ghi_jkl_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s as 'abc [target symbol])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+= s '([def] [ghi] [jkl]))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_SYMBOL, evaluatedValue->typeTag);

    char const * const expected = "abcdefghijkl";

    ASSERT_STR_EQ(
        expected,
        octaspire_string_get_c_string(evaluatedValue->value.symbol));

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "s");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_SYMBOL, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        expected,
        octaspire_string_get_c_string(evaluatedValue->value.symbol));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_equals_with_symbol_abc_and_nil_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s as 'abc [target symbol])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+= s nil)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_SYMBOL, evaluatedValue->typeTag);

    char const * const expected = "abcnil";

    ASSERT_STR_EQ(
        expected,
        octaspire_string_get_c_string(evaluatedValue->value.symbol));

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "s");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_SYMBOL, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        expected,
        octaspire_string_get_c_string(evaluatedValue->value.symbol));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_equals_with_symbol_abc_and_booleans_true_and_false_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s as 'abc [target symbol])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+= s true false)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_SYMBOL, evaluatedValue->typeTag);

    char const * const expected = "abctruefalse";

    ASSERT_STR_EQ(
        expected,
        octaspire_string_get_c_string(evaluatedValue->value.symbol));

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "s");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_SYMBOL, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        expected,
        octaspire_string_get_c_string(evaluatedValue->value.symbol));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_equals_with_symbol_abc_and_integer_128_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s as 'abc [target symbol])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+= s {D+128})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_SYMBOL, evaluatedValue->typeTag);

    char const * const expected = "abc{D+128}";

    ASSERT_STR_EQ(
        expected,
        octaspire_string_get_c_string(evaluatedValue->value.symbol));

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "s");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_SYMBOL, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        expected,
        octaspire_string_get_c_string(evaluatedValue->value.symbol));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_equals_with_symbol_abc_and_real_3dot14_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s as 'abc [target symbol])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+= s {D+3.14})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_SYMBOL, evaluatedValue->typeTag);

    char const * const expected = "abc{D+3.14}";

    ASSERT_STR_EQ(
        expected,
        octaspire_string_get_c_string(evaluatedValue->value.symbol));

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "s");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_SYMBOL, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        expected,
        octaspire_string_get_c_string(evaluatedValue->value.symbol));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}










TEST octaspire_dern_vm_builtin_plus_equals_with_bad_input_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+= (hash-map {D+1} ¦a|) (hash-map {D+2} |b|))");
    // bad input here      ^^^

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR,  evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Unknown character constant |) (hash-map {D+2} |",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_run_user_factorial_function_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_path_and_eval_in_global_environment(
            vm,
            OCTASPIRE_DERN_CONFIG_TEST_RES_PATH "octaspire_dern_vm_run_user_factorial_function_test.dern");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(101,                              evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_for_from_0_to_10_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v as '() [v])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(for i from {D+0} to {D+10} (+= v i))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(11,                               evaluatedValue->value.integer);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "v");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(11, octaspire_dern_value_as_vector_get_length(evaluatedValue));

    for (int32_t i = 0; i < 11; ++i)
    {
        octaspire_dern_value_t const * const tmp =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                evaluatedValue,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                i);

        ASSERT(tmp);
        ASSERT_EQ(i, tmp->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_for_from_0_to_10_with_step_2_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v as '() [v])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(for i from {D+0} to {D+10} step {D+2} (+= v i))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(6,                                evaluatedValue->value.integer);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "v");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(6, octaspire_dern_value_as_vector_get_length(evaluatedValue));

    for (int32_t i = 0, cmp = 0; i < 6; ++i, cmp += 2)
    {
        octaspire_dern_value_t const * const tmp =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                evaluatedValue,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                i);

        ASSERT(tmp);
        ASSERT_EQ(cmp, tmp->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_for_from_0_to_10_with_step_minus_2_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v as '() [v])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(for i from {D+0} to {D+10} step {D-2} (+= v i))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "The 'step' of special 'for' must be larger than zero. Now it is {D-2}.\n"
        "\tAt form: >>>>>>>>>>(for i from {D+0} to {D+10} step {D-2} (+= v i))<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_for_from_10_to_0_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v as '() [v])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(for i from {D+10} to {D+0} (+= v i))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(11,                               evaluatedValue->value.integer);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "v");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(11, octaspire_dern_value_as_vector_get_length(evaluatedValue));

    for (int32_t i = 0; i < 11; ++i)
    {
        octaspire_dern_value_t const * const tmp =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                evaluatedValue,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                i);

        ASSERT(tmp);
        ASSERT_EQ(10 - i, tmp->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_for_from_10_to_0_with_step_2_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v as '() [v])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(for i from {D+10} to {D+0} step {D+2} (+= v i))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(6,                               evaluatedValue->value.integer);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "v");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(6, octaspire_dern_value_as_vector_get_length(evaluatedValue));

    for (int32_t i = 0, cmp = 0; i < 6; ++i, cmp += 2)
    {
        octaspire_dern_value_t const * const tmp =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                evaluatedValue,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                i);

        ASSERT(tmp);
        ASSERT_EQ(10 - cmp, tmp->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_for_from_10_to_0_with_step_minus_2_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v as '() [v])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(for i from {D+10} to {D+0} step {D-2} (+= v i))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);
    ASSERT_STR_EQ(
        "The 'step' of special 'for' must be larger than zero. Now it is {D-2}.\n"
        "\tAt form: >>>>>>>>>>(for i from {D+10} to {D+0} step {D-2} (+= v i))<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}


TEST octaspire_dern_vm_special_for_in_with_string_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s as [] [s])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define n as [abc] [n])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(for i in n (+= s i))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(3,                                evaluatedValue->value.integer);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "s");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "abc",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_for_in_with_string_step_2_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s as [] [s])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define n as [abc] [n])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(for i in n step {D+2} (+= s i))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(2,                                evaluatedValue->value.integer);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "s");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "ac",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_for_in_with_string_step_minus_2_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s as [] [s])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define n as [abc] [n])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(for i in n step {D-2} (+= s i))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "The 'step' of special 'for' must be larger than zero. Now it is {D-2}.\n"
        "\tAt form: >>>>>>>>>>(for i in n step {D-2} (+= s i))<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_for_in_with_vector_of_strings_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s as [] [s])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define n as '([John] [Mike] [Ellie]) [n])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(for i in n (+= s i [ ]))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(3,                                evaluatedValue->value.integer);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "s");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "John Mike Ellie ",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_for_in_with_vector_of_strings_step_2_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s as [] [s])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define n as '([John] [Mike] [Ellie]) [n])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(for i in n step {D+2} (+= s i [ ]))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(2,                                evaluatedValue->value.integer);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "s");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "John Ellie ",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_for_in_with_vector_of_strings_step_minus_2_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s as [] [s])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define n as '([John] [Mike] [Ellie]) [n])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(for i in n step {D-2} (+= s i [ ]))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);
    ASSERT_STR_EQ(
        "The 'step' of special 'for' must be larger than zero. Now it is {D-2}.\n"
        "\tAt form: >>>>>>>>>>(for i in n step {D-2} (+= s i [ ]))<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_for_in_with_environment_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define e as (env-new) [e])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define e2 as (env-new) [e2])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);



    // a is 1
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define a as {D+1} [a] in e)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    // b is 2
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define b as {D+2} [b] in e)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    // c is 3
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define c as {D+3} [c] in e)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);



    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(for i in e (define (ln@ i {D+0}) as (ln@ i {D+1}) [-] in e2))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(3,                                evaluatedValue->value.integer);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(to-string e2)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "---------- environment ----------\n"
        "a -> {D+1}\n"
        "b -> {D+2}\n"
        "c -> {D+3}\n"
        "---------------------------------\n",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_for_in_with_environment_step_2_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define e as (env-new) [e])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define e2 as (env-new) [e2])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);



    // a is 1
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define a as {D+1} [a] in e)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    // b is 2
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define b as {D+2} [b] in e)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    // c is 3
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define c as {D+3} [c] in e)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);



    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(for i in e step {D+2} (define (ln@ i {D+0}) as (ln@ i {D+1}) [-] in e2))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(2,                                evaluatedValue->value.integer);

    // octaspire_map_t is not ordered map, so we cannot know for sure which
    // two values are the first ones in the map.

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_for_in_with_environment_step_minus_2_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define e as (env-new) [e])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define e2 as (env-new) [e2])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);



    // a is 1
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define a as {D+1} [a] in e)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    // b is 2
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define b as {D+2} [b] in e)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    // c is 3
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define c as {D+3} [c] in e)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);



    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(for i in e step {D-2} (define (ln@ i {D+0}) as (ln@ i {D+1}) [-] in e2))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);
    ASSERT_STR_EQ(
        "The 'step' of special 'for' must be larger than zero. Now it is {D-2}.\n"
        "\tAt form: >>>>>>>>>>(for i in e step {D-2} (define (ln@ i {D+0}) as (ln@ i {D+1}) [-] in e2))<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_for_in_with_hash_map_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define h as (hash-map 'a {D+1} 'b {D+2} 'c {D+3}) [h])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define e as (env-new) [e])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(for i in h (define (ln@ i {D+0}) as (ln@ i {D+1}) [-] in e))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(3,                                evaluatedValue->value.integer);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(to-string e)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "---------- environment ----------\n"
        "a -> {D+1}\n"
        "b -> {D+2}\n"
        "c -> {D+3}\n"
        "---------------------------------\n",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_for_in_with_hash_map_step_2_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define h as (hash-map 'a {D+1} 'b {D+2} 'c {D+3}) [h])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define e as (env-new) [e])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(for i in h step {D+2} (define (ln@ i {D+0}) as (ln@ i {D+1}) [-] in e))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(2,                                evaluatedValue->value.integer);

    // octaspire_map_t is not ordered map, so we cannot know for sure which
    // two values are the first ones in the map.

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_for_in_with_hash_map_step_minus_2_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define h as (hash-map 'a {D+1} 'b {D+2} 'c {D+3}) [h])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define e as (env-new) [e])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(for i in h step {D-2} (define (ln@ i {D+0}) as (ln@ i {D+1}) [-] in e))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "The 'step' of special 'for' must be larger than zero. Now it is {D-2}.\n"
        "\tAt form: >>>>>>>>>>(for i in h step {D-2} (define (ln@ i {D+0}) as (ln@ i {D+1}) [-] in e))<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_for_called_without_arguments_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(for)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Special 'for' expects at least four (for iterating container or port) or five (for "
        "iterating numeric range) arguments. 0 arguments were given.\n"
        "\tAt form: >>>>>>>>>>(for)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message)); 

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_for_called_with_one_argument_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(for i)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Special 'for' expects at least four (for iterating container or port) or five (for "
        "iterating numeric range) arguments. 1 arguments were given.\n"
        "\tAt form: >>>>>>>>>>(for i)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message)); 

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_for_second_argument_not_symbol_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(for i {D+10} {D+0} to {D+20} true)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Second argument to special 'for' must be symbol 'in' or 'from'. Now it has type integer.\n"
        "\tAt form: >>>>>>>>>>(for i {D+10} {D+0} to {D+20} true)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message)); 

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_for_called_with_integer_as_first_argument_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(for {D+1} from {D+0} to {D+10} true)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "First argument to special 'for' must be symbol value. Now it has type integer.\n"
        "\tAt form: >>>>>>>>>>(for {D+1} from {D+0} to {D+10} true)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message)); 

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_error_in_function_body_is_reported_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn (x) (NoSuchFunction x)) [f] '(x [x]) howto-ok)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(f {D+1})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);
    // TODO type of 'error' or 'vector'?
    ASSERT_STR_EQ(
        "Cannot evaluate operator of type 'error' (<error>: Unbound symbol 'NoSuchFunction')\n"
        "\tAt form: >>>>>>>>>>(f {D+1})<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_cp_at_sign_called_with_1_2_3_and_integers_0_and_1_and_2_and_3_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    // 0
    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(cp@ 1.2.3 {D+0})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);

    ASSERT_EQ(
        1,
        octaspire_dern_value_as_integer_get_value(evaluatedValue));

    // 1
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(cp@ 1.2.3 {D+1})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);

    ASSERT_EQ(
        2,
        octaspire_dern_value_as_integer_get_value(evaluatedValue));

    // 2
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(cp@ 1.2.3 {D+2})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);

    ASSERT_EQ(
        3,
        octaspire_dern_value_as_integer_get_value(evaluatedValue));

    // 3
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(cp@ 1.2.3 {D+3})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Index 3 is outside the SemVer length 3.\n"
        "\tAt form: >>>>>>>>>>(cp@ 1.2.3 {D+3})<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));


    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_cp_at_sign_called_with_0_and_string_abc_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(cp@ [abc] {D+0})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "a",
        octaspire_string_get_c_string(evaluatedValue->value.character));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_cp_at_sign_called_with_1_and_string_abc_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(cp@ [abc] {D+1})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "b",
        octaspire_string_get_c_string(evaluatedValue->value.character));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_cp_at_sign_called_with_2_and_string_abc_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(cp@ [abc] {D+2})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "c",
        octaspire_string_get_c_string(evaluatedValue->value.character));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_cp_at_sign_called_with_3_and_string_abc_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(cp@ [abc] {D+3})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Index to builtin 'cp@' is not valid for the given string. "
        "Index '3' was given.\n"
        "\tAt form: >>>>>>>>>>(cp@ [abc] {D+3})<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_ln_at_sign_called_with_0_and_vector_1_2_3_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(ln@ '({D+1} {D+2} {D+3}) {D+0})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(1,                                evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_ln_at_sign_called_with_1_and_vector_1_2_3_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(ln@ '({D+1} {D+2} {D+3}) {D+1})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(2,                                evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_ln_at_sign_called_with_2_and_vector_1_2_3_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(ln@ '({D+1} {D+2} {D+3}) {D+2})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(3,                                evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_ln_at_sign_called_with_3_and_vector_1_2_3_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(ln@ '({D+1} {D+2} {D+3}) {D+3})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Index to builtin 'ln@' is not valid for the given vector. "
        "Index '3' was given.\n"
        "\tAt form: >>>>>>>>>>(ln@ (quote ({D+1} {D+2} {D+3})) {D+3})<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_ln_at_sign_called_with_0_and_index_and_hash_map_1a_2b_3c_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(ln@ (hash-map {D+1} |a| {D+2} |b| {D+3} |c|) {D+0} 'index)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "b",
        octaspire_string_get_c_string(evaluatedValue->value.character));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_ln_at_sign_called_with_1_and_index_and_hash_map_1a_2b_3c_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(ln@ (hash-map {D+1} |a| {D+2} |b| {D+3} |c|) {D+1} 'index)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "c",
        octaspire_string_get_c_string(evaluatedValue->value.character));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_ln_at_sign_called_with_2_and_index_and_hash_map_1a_2b_3c_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(ln@ (hash-map {D+1} |a| {D+2} |b| {D+3} |c|) {D+2} 'index)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "a",
        octaspire_string_get_c_string(evaluatedValue->value.character));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_ln_at_sign_called_with_3_and_index_and_hash_map_1a_2b_3c_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(ln@ (hash-map {D+1} |a| {D+2} |b| {D+3} |c|) {D+3} 'index)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'ln@' could not find the requested element from hash map.\n"
        "\tAt form: >>>>>>>>>>(ln@ (hash-map {D+1} |a| {D+2} |b| {D+3} |c|) {D+3} (quote index))<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_ln_at_sign_called_with_2_and_hash_and_hash_map_1a_2b_3c_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(ln@ (hash-map {D+1} |a| {D+2} |b| {D+3} |c|) {D+2} 'hash)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "b",
        octaspire_string_get_c_string(evaluatedValue->value.character));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_changing_atom_doesnt_change_another_defined_from_it_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define original as {D+10} [original])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define copied as original [copied])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(= original {D+999})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(999,                              evaluatedValue->value.integer);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "copied");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(10,                               evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_return_in_special_do_inside_function_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define counter as {D+0} [counter])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn () (do (++ counter) (return {D+99}) (++ counter))) [f] '() howto-no)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(f)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(99,                               evaluatedValue->value.integer);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "counter");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(1,                                evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_do_error_stops_evaluation_and_is_reported_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define counter as {D+0} [counter])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(do (++ counter))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(1,                                evaluatedValue->value.integer);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(do (++ counter) (NoSuchFunction) (++ counter))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Cannot evaluate operator of type 'error' (<error>: Unbound symbol 'NoSuchFunction')\n"
        "\tAt form: >>>>>>>>>>(NoSuchFunction)<<<<<<<<<<\n\n"
        "\tAt form: >>>>>>>>>>(do (++ counter) (NoSuchFunction) (++ counter))<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "counter");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(2,                                evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_fn_in_env_howto_no_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn (x) (+ x x)) [f] '(x [x]) in (env-global) howto-no)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "f");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_FUNCTION, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->howtoAllowed);
    ASSERT_EQ(false, evaluatedValue->value.function->howtoAllowed);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_fn_in_env_howto_ok_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn (x) (+ x x)) [f] '(x [x]) in (env-global) howto-ok)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "f");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_FUNCTION, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->howtoAllowed);
    ASSERT_EQ(true, evaluatedValue->value.function->howtoAllowed);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_return_inside_function_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define counter as {D+0} [counter])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn () (++ counter) (return {D+99}) (++ counter)) [f] '() howto-no)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(f)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(99,                               evaluatedValue->value.integer);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "counter");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(1,                                evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_return_in_special_for_with_numeric_range_inside_function_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define counter as {D+0} [counter])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn () (for i from {D+0} to {D+10} (++ counter) (if (== i {D+5}) (return {D+99})))) [f] '() howto-no)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(f)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(99,                               evaluatedValue->value.integer);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "counter");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(6,                                evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_return_in_special_for_with_collection_inside_function_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define counter as {D+0} [counter])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn () (for i in '({D+1} {D+2} {D+3} {D+4} {D+5} {D+6}) (+= counter i) (if (== i {D+4}) (return {D+99})))) [f] '() howto-no)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(f)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(99,                               evaluatedValue->value.integer);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "counter");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(10,                               evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_return_in_special_for_with_string_inside_function_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define counter as {D+0} [counter])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn () (for i in [abcdef] (+= counter {D+1}) (if (== i |d|) (return {D+99})))) [f] '() howto-no)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(f)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(99,                               evaluatedValue->value.integer);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "counter");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(4,                               evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_return_in_special_for_with_environment_inside_function_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define counter as {D+0} [counter])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn () (define a as {D+1} [a]) (define b as {D+2} [b]) (define c as {D+3} [c]) (for i in (env-current) (+= counter {D+1}) (if (== counter {D+1}) (return {D+99})))) [f] '() howto-no)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(f)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(99,                               evaluatedValue->value.integer);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "counter");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(1,                               evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_return_in_special_for_with_hash_map_inside_function_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define counter as {D+0} [counter])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn () (for i in (hash-map |a| {D+1} |b| {D+2} |c| {D+3}) (+= counter {D+1}) (if (== counter {D+1}) (return {D+99})))) [f] '() howto-no)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(f)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(99,                               evaluatedValue->value.integer);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "counter");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(1,                               evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

// TODO XXX add new test that is similar to above, but uses (for i in container ...
// TODO XXX other loops and other functions need to handle return?

TEST octaspire_dern_vm_builtin_return_in_special_while_inside_function_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define counter as {D+0} [counter])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn () (while (< counter {D+100}) (++ counter) (return {D+99}))) [f] '() howto-no)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(f)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(99,                               evaluatedValue->value.integer);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "counter");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(1,                                evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_return_called_without_argument_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define counter as {D+0} [counter])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn () (while (< counter {D+100}) (++ counter) (return))) [f] '() howto-no)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(f)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_NIL, evaluatedValue->typeTag);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "counter");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(1,                                evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_return_called_with_two_arguments_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define counter as {D+0} [counter])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn () (while (< counter {D+100}) (++ counter) (return {D+1} {D+2}))) [f] '() howto-no)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(f)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'return' expects zero or one arguments. 2 arguments were given.\n"
        "\tAt form: >>>>>>>>>>(return {D+1} {D+2})<<<<<<<<<<\n"
        "\n"
        "\tAt form: >>>>>>>>>>(while (< counter {D+100}) (++ counter) (return {D+1} {D+2}))<<<<<<<<<<\n"
        "\n"
        "\tAt form: >>>>>>>>>>(f)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "counter");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(1,                                evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_function_taking_one_regular_and_varargs_called_with_four_arguments_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn (x y ...)\n"
            "(define result as [] [result])\n"
            "(+= result (string-format x)) (+= result (string-format y))\n"
            "result) "
            "[f] '(x [x] y [rest of the args] ... [varargs]) howto-no)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(f {D+1} {D+2} {D+3} {D+4})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "{D+1}({D+2} {D+3} {D+4})",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_function_taking_one_regular_and_varargs_called_with_one_argument_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn (x y ...)\n"
            "(define result as [] [result])\n"
            "(+= result (string-format x)) (+= result (string-format y))\n"
            "result) "
            "[f] '(x [x] y [rest of the args] ... [varargs]) howto-no)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(f {D+1})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "{D+1}()",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_function_taking_one_regular_and_varargs_called_with_zero_arguments_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn (x y ...)\n"
            "(define result [result] [])\n"
            "(+= result (string-format x)) (+= result (string-format y))\n"
            "result) "
            "[f] '(x [x] y [rest of the args] ... [varargs]) howto-no)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(f)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Function expects 1 arguments. Now 0 arguments were given.",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_newly_created_function_returned_from_another_function_and_used_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define creator as (fn () (fn (val) (+ {D+11} val))) [creator] '() howto-ok)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "((creator) {D+7})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(18,                               evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_recursive_function_used_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define a as (fn (x) (if (< x {D+10}) (a (+ x {D+1})) x)) [a] '(x [x]) howto-ok)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(a {D+1})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(10,                               evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_mutually_recursive_functions_used_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define a as (fn (val) (b (+ val {D+1}))) [a] '(val [val]) howto-ok)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define b as (fn (x) (if (< x {D+10}) (a x) x)) [b] '(x [x]) howto-ok)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(b {D+1})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(10,                               evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_and_called_without_arguments_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(vm, "(and)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_or_called_without_arguments_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(vm, "(or)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_not_called_without_arguments_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(vm, "(not)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);
    ASSERT_STR_EQ(
        "Builtin 'not' expects one argument.\n"
        "\tAt form: >>>>>>>>>>(not)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_and_called_with_one_argument_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+9} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(vm, "(and x)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(9,                                evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_or_called_with_one_argument_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+9} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(vm, "(and x)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(9,                                evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_and_called_with_one_false_argument_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as false [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(vm, "(and x)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_or_called_with_one_false_argument_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as false [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(vm, "(or x)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_not_called_with_one_false_argument_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as false [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(vm, "(not x)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                            evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_not_called_with_one_true_argument_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as true [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(vm, "(not x)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_not_called_with_one_integer_argument_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+0} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(vm, "(not x)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'not' expects boolean argument.\n"
        "\tAt form: >>>>>>>>>>(not x)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_not_called_with_two_boolean_arguments_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as false [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define y as true [y])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(vm, "(not x y)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'not' expects one argument.\n"
        "\tAt form: >>>>>>>>>>(not x y)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_and_called_with_three_arguments_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s as [] [s])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn (x) (+= s x) x) [f] '(x [x]) howto-no)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(and (f [a]) (f [b]) (f [c]))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ("c", octaspire_string_get_c_string(evaluatedValue->value.string));

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "s");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ("abc", octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_or_called_with_three_arguments_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s as [] [s])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn (x) (+= s x) (if (== x [c]) x false)) [f] '(x [x]) howto-no)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(or (f [a]) (f [b]) (f [c]))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ("c", octaspire_string_get_c_string(evaluatedValue->value.string));

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "s");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ("abc", octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_and_called_with_three_arguments_with_false_as_second_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s as [] [s])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn (x) (+= s x) x) [f] '(x [x]) howto-no)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(and (f [a]) false (f [c]))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "s");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ("a", octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_or_called_with_three_arguments_with_true_as_second_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s as [] [s])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn (x) (+= s x) false) [f] '(x [x]) howto-no)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(or (f [a]) true (f [c]))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "s");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ("a", octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_greater_than_called_with_two_reals_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+10.02} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(> x {D+10.01})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_greater_than_called_with_two_integers_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+10} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(> x {D+9})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_greater_than_called_with_integer_10_and_real_9dot9_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+10} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(> x {D+9.9})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_greater_than_called_with_integer_9_and_real_9dot9_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+9} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(> x {D+9.9})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_greater_than_called_with_real_9dot9_and_integer_10_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+9.9} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(> x {D+10})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_greater_than_called_with_real_10dot1_and_integer_10_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+10.1} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(> x {D+10})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}






TEST octaspire_dern_vm_special_greater_than_or_equal_called_with_two_reals_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+10.02} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(>= x {D+10.01})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(>= x {D+10.02})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(>= x {D+10.03})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_greater_than_or_equal_called_with_two_integers_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+10} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(>= x {D+9})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(>= x {D+10})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(>= x {D+11})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_greater_than_or_equal_called_with_integer_10_and_real_9dot9_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+10} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(>= x {D+9.9})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_greater_than_or_equal_called_with_integer_9_and_real_9dot9_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+9} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(>= x {D+9.9})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_greater_than_or_equal_called_with_real_9dot9_and_integer_10_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+9.9} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(>= x {D+10})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_greater_than_or_equal_called_with_real_10dot1_and_integer_10_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+10.1} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(>= x {D+10})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

















TEST octaspire_dern_vm_special_less_than_called_with_two_reals_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+10.01} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(< x {D+10.02})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_less_than_called_with_two_integers_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+9} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(< x {D+10})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_less_than_called_with_integer_9_and_real_9dot9_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+9} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(< x {D+9.9})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_less_than_called_with_integer_10_and_real_9dot9_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+10} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(< x {D+9.9})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_less_than_called_with_real_10dot1_and_integer_10_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+10.1} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(< x {D+10})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_less_than_called_with_real_9dot9_and_integer_10_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+9.9} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(< x {D+10})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}












TEST octaspire_dern_vm_special_less_than_or_equal_called_with_two_reals_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+10.01} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= x {D+10.02})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= x {D+10.01})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= x {D+10.0})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_less_than_or_equal_called_with_two_integers_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+9} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= x {D+10})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= x {D+9})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= x {D+8})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_less_than_or_equal_called_with_integer_9_and_real_9dot9_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+9} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= x {D+9.9})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_less_than_or_equal_called_with_integer_10_and_real_9dot9_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+10} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= x {D+9.9})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_less_than_or_equal_called_with_real_10dot1_and_integer_10_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+10.1} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= x {D+10})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_less_than_or_equal_called_with_real_9dot9_and_integer_10_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+9.9} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= x {D+10})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_equals_called_with_two_equal_queues_of_integers_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define q1 as (queue {D+1} {D+2} {D+3} {D+4} {D+5}) [q1])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define q2 as (queue {D+1} {D+2} {D+3} {D+4} {D+5}) [q2])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== q1 q2)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== q2 q1)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_equals_called_with_two_equal_queues_of_vectors_of_integers_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define q1 as (queue '({D+1} {D+2} {D+3}) '({D+4} {D+5} {D+6})) [q1])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define q2 as (queue '({D+1} {D+2} {D+3}) '({D+4} {D+5} {D+6})) [q2])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== q1 q2)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== q2 q1)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_equals_called_with_two_unequal_queues_of_vectors_of_integers_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define q1 as (queue '({D+1} {D+2} {D+3}) '({D+4} {D+5} {D+6})) [q1])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define q2 as (queue '({D+1} {D+2} {D+3}) '({D+4} {D+5} {D+7})) [q2])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== q1 q2)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== q2 q1)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_equals_called_with_two_unequal_queues_of_integers_with_different_lengths_of_5_and_6_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define q1 as (queue {D+1} {D+2} {D+3} {D+4} {D+5}) [q1])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define q2 as (queue {D+1} {D+2} {D+3} {D+4} {D+5} {D+6}) [q2])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== q1 q2)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== q2 q1)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_equals_called_with_two_unequal_queues_of_integers_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define q1 as (queue {D+1} {D+2} {D+3} {D+4} {D+5}) [q1])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define q2 as (queue {D+1} {D+2} {D+3} {D+4} {D+6}) [q2])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== q1 q2)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== q2 q1)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_equals_called_with_two_equal_lists_of_integers_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define l1 as (list {D+1} {D+2} {D+3} {D+4} {D+5}) [l1])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define l2 as (list {D+1} {D+2} {D+3} {D+4} {D+5}) [l2])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== l1 l2)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== l2 l1)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_equals_called_with_two_unequal_lists_of_integers_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define l1 as (list {D+1} {D+2} {D+3} {D+4} {D+5}) [l1])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define l2 as (list {D+1} {D+2} {D+3} {D+4} {D+6}) [l2])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== l1 l2)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== l2 l1)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_equals_called_with_two_equal_vectors_of_integers_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v1 as '({D+1} {D+2} {D+3} {D+4} {D+5}) [v1])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v2 as '({D+1} {D+2} {D+3} {D+4} {D+5}) [v2])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== v1 v2)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== v2 v1)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_equals_called_with_two_equal_vectors_of_vectors_of_integers_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v1 as '(({D+1} {D+2} {D+3}) ({D+4} {D+5} {D+6})) [v1])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v2 as '(({D+1} {D+2} {D+3}) ({D+4} {D+5} {D+6})) [v2])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== v1 v2)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== v2 v1)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_equals_called_with_two_equal_hash_maps_of_vectors_of_integers_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v1 as (hash-map |a| '({D+1} {D+2} {D+3}) |b| '({D+4} {D+5} {D+6})) [v1])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v2 as (hash-map |a| '({D+1} {D+2} {D+3}) |b| '({D+4} {D+5} {D+6})) [v2])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== v1 v2)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== v2 v1)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_equals_called_with_two_unequal_hash_maps_of_vectors_of_integers_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v1 as (hash-map |a| '({D+1} {D+2} {D+3}) |b| '({D+4} {D+5} {D+6})) [v1])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v2 as (hash-map |a| '({D+1} {D+2} {D+3}) |b| '({D+4} {D+5} {D+7})) [v2])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== v1 v2)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== v2 v1)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_equals_called_with_two_equal_environments_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define e1 as (env-new) [e1])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define e2 as (env-new) [e2])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    // Compare empty environments
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== e1 e2)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== e2 e1)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    // Add some content to environments
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define a as {D+1} [a] in e1)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define a as {D+1} [a] in e2)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    // Compare environments with one value
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== e1 e2)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== e2 e1)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_equals_called_with_two_unequal_environments_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define e1 as (env-new) [e1])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define e2 as (env-new) [e2])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    // Add some differing content to environments
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define a as {D+1} [a] in e1)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define a as {D+2} [a] in e2)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    // Compare environments with one value
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== e1 e2)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== e2 e1)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_equals_called_with_two_equal_functions_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define e1 as (env-new) [e1])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define e2 as (env-new) [e2])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    // First function
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn (x) (+ x x)) [f] '(x [x]) in e1 howto-ok)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    // Second function
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn (x) (+ x x)) [f] '(x [x]) in e2 howto-ok)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    // Compare functions
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== (find e1 'f) (find e2 'f))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== (find e2 'f) (find e1 'f))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_equals_called_with_two_unequal_functions_formal_name_differs_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define e1 as (env-new) [e1])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define e2 as (env-new) [e2])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    // First function
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn (x) (+ x x)) [f] '(x [x]) in e1 howto-ok)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    // Second function
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn (y) (+ y y)) [f] '(y [x]) in e2 howto-ok)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    // Compare functions
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== (find e1 'f) (find e2 'f))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== (find e2 'f) (find e1 'f))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_equals_called_with_two_unequal_functions_name_differs_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define e1 as (env-new) [e1])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define e2 as (env-new) [e2])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    // First function
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn (x) (+ x x)) [f] '(x [x]) in e1 howto-ok)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    // Second function
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f2 as (fn (x) (+ x x)) [f] '(x [x]) in e2 howto-ok)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    // Compare functions
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== (find e1 'f) (find e2 'f2))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== (find e2 'f2) (find e1 'f))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_equals_called_with_two_unequal_functions_docstr_differs_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define e1 as (env-new) [e1])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define e2 as (env-new) [e2])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    // First function
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn (x) (+ x x)) [f] '(x [x]) in e1 howto-ok)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    // Second function
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn (x) (+ x x)) [f2] '(x [x]) in e2 howto-ok)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    // Compare functions
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== (find e1 'f) (find e2 'f))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== (find e2 'f) (find e1 'f))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_equals_called_with_two_unequal_functions_howto_differs_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define e1 as (env-new) [e1])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define e2 as (env-new) [e2])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    // First function
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn (x) (+ x x)) [f] '(x [x]) in e1 howto-ok)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    // Second function
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn (x) (+ x x)) [f] '(x [x]) in e2 howto-no)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    // Compare functions
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== (find e1 'f) (find e2 'f))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== (find e2 'f) (find e1 'f))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_equals_called_with_two_unequal_vectors_of_vectors_of_integers_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v1 as '(({D+1} {D+2} {D+3}) ({D+4} {D+5} {D+6})) [v1])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v2 as '(({D+1} {D+2} {D+3}) ({D+4} {D+5} {D+7})) [v2])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== v1 v2)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== v2 v1)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_equals_called_with_two_unequal_vectors_of_integers_with_different_lengths_5_and_6_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v1 as '({D+1} {D+2} {D+3} {D+4} {D+5}) [v1])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v2 as '({D+1} {D+2} {D+3} {D+4} {D+5} {D+6}) [v2])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== v1 v2)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== v2 v1)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_equals_called_with_two_unequal_vectors_of_integers_with_different_elements_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v1 as '({D+1} {D+2} {D+3} {D+4} {D+5}) [v1])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v2 as '({D+1} {D+2} {D+3} {D+4} {D+6}) [v2])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== v1 v2)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== v2 v1)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_equals_called_with_two_reals_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+10.01} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== x {D+10.01})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== x {D+10.02})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_equals_called_with_two_integers_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+9} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== x {D+9})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== x {D+10})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_buildin_equals_equals_called_with_integer_9_and_reals_9dot1_and_9dot0_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+9} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== x {D+9.1})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== x {D+9.0})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                            evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_equals__called_with_integer_10_and_real_9dot9_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+10} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== x {D+9.9})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_equals_called_with_real_10dot1_and_integer_10_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+10.1} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== x {D+10})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_equals_called_with_real_9dot9_and_integer_10_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+9.9} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== x {D+10})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}







TEST octaspire_dern_vm_builtin_exclamation_equals_called_with_two_reals_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+10.01} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= x {D+10.02})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= x {D+10.01})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_exclamation_equals_called_with_two_integers_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+9} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= x {D+10})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= x {D+9})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_buildin_exclamation_equals_called_with_integer_9_and_reals_9dot1_and_9dot0_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+9} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= x {D+9.1})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= x {D+9.0})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_exclamation_equals_called_with_integer_10_and_real_9dot9_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+10} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= x {D+9.9})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_exclamation_equals_called_with_real_10dot1_and_integer_10_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+10.1} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= x {D+10})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_exclamation_equals_called_with_real_9dot9_and_integer_10_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x as {D+9.9} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= x {D+10})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_error_during_user_function_call_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn () (NoSuchFunction)) [my function] '() howto-ok)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(f)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);
    ASSERT_STR_EQ(
        "Cannot evaluate operator of type 'error' (<error>: Unbound symbol 'NoSuchFunction')\n"
        "\tAt form: >>>>>>>>>>(f)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_error_during_builtin_call_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(++)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);
    ASSERT_STR_EQ(
        "Builtin '++' expects at least one argument.\n"
        "\tAt form: >>>>>>>>>>(++)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_error_during_special_call_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);
    ASSERT_STR_EQ(
        "Special '<' expects at least two arguments.\n"
        "\tAt form: >>>>>>>>>>(<)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_error_during_special_call_during_user_function_call_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn () (<)) [f] '() howto-ok)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(f)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);
    ASSERT_STR_EQ(
        "Special '<' expects at least two arguments.\n"
        "\tAt form: >>>>>>>>>>(<)<<<<<<<<<<\n"
        "\n"
        "\tAt form: >>>>>>>>>>(f)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_eval_plus_1_2_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(eval (+ {D+1} {D+2}))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(3,                                evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_eval_plus_1_2_in_given_global_env_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(eval (+ {D+1} {D+2}) (env-global))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(3,                                evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_eval_value_from_given_local_env_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define myEnv as (env-new) [myEnv])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define myVal as {D+128} [myVal] in myEnv)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(eval myVal myEnv)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(128,                              evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_eval_eval_eval_f_1_2_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define myFn as + [myFn])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(eval ((eval myFn) {D+1} {D+2}))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(3,                              evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_eval_failure_on_integer_on_second_argument_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(eval (+ {D+1} {D+1}) {D+10})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Second argument to special 'eval' must evaluate into environment value.\n"
        "Now it evaluated into type integer.\n"
        "\tAt form: >>>>>>>>>>(eval (+ {D+1} {D+1}) {D+10})<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_eval_failure_on_unbound_symbol_on_second_argument_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(eval (+ {D+1} {D+1}) pi)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Unbound symbol 'pi'\n"
        "\tAt form: >>>>>>>>>>(eval (+ {D+1} {D+1}) pi)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_eval_called_with_three_arguments_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(eval (+ {D+1} {D+1}) (env-global) {D+10})");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Special 'eval' expects one or two arguments. 3 arguments were given.\n"
        "\tAt form: >>>>>>>>>>(eval (+ {D+1} {D+1}) (env-global) {D+10})<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_create_new_value_copy_called_with_vector_value_of_int_values_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t * const originalVal = octaspire_dern_vm_create_new_value_vector(vm);

    octaspire_dern_vm_push_value(vm, originalVal);

    for (int32_t i = 0; i < 10; ++i)
    {
        octaspire_dern_value_t *tmpVal = octaspire_dern_vm_create_new_value_integer(vm, i);

        ASSERT(octaspire_dern_value_as_vector_push_back_element(originalVal, &tmpVal));
    }

    octaspire_dern_value_t * const copiedVal =
        octaspire_dern_vm_create_new_value_copy(vm, originalVal);

    octaspire_dern_vm_push_value(vm, copiedVal);

    ASSERT(copiedVal);
    ASSERT(originalVal != copiedVal);

    ASSERT_EQ(
        octaspire_dern_value_as_vector_get_length(originalVal),
        octaspire_dern_value_as_vector_get_length(copiedVal));

    for (size_t i = 0; i < octaspire_dern_value_as_vector_get_length(copiedVal); ++i)
    {
        octaspire_dern_value_t const * const original =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                originalVal,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                0);

        octaspire_dern_value_t const * const copied =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                copiedVal,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                0);

        ASSERT(original && copied);
        ASSERT(original != copied);
        ASSERT_EQ(original->value.integer, copied->value.integer);
    }

    octaspire_dern_vm_pop_value(vm, copiedVal);
    octaspire_dern_vm_pop_value(vm, originalVal);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_comment_alone_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "; comment");

    ASSERT_EQ(0, evaluatedValue);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_comment_after_integer_1_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "{D+1} ;comment");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(1, evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_comment_before_integer_123_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            ";comment\n{D+123}");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(123, evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_multiline_comment_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "#!/bin/octaspire-dern-repl\n!#");

    ASSERT_EQ(0, evaluatedValue);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_multiline_comment_in_vector_1_2_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(len (vector {D+1} {D+2} #! comment !# {D+3} #!abc!# {D+4} #!!# {D+5}))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(5, evaluatedValue->value.integer);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(to-string (vector {D+1} {D+2} #! comment !# {D+3} #!abc!# {D+4} #!!# {D+5}))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ(
        "({D+1} {D+2} {D+3} {D+4} {D+5})",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_multiline_comment_before_integer_123_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(
        octaspireDernVmTestAllocator,
        octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "#!comment!#\n{D+123}");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(123, evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_multiline_comment_missing_chars_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "#!/bin/octaspire-dern-repl\n!");

    ASSERT_EQ(0, evaluatedValue);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "#!/bin/octaspire-dern-repl\n");

    ASSERT_EQ(0, evaluatedValue);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_io_file_open_success_test(void)
{
    octaspire_dern_vm_config_t config = octaspire_dern_vm_config_default();

    octaspire_dern_vm_t *vm = octaspire_dern_vm_new_with_config(octaspireDernVmTestAllocator, octaspireDernVmTestStdio, config);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (io-file-open [" OCTASPIRE_DERN_CONFIG_TEST_RES_PATH "octaspire_io_file_open_test.txt]) [f])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    for (size_t i = 0; i < 3; ++i)
    {
        evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(port-read f)");

        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
        ASSERT_EQ((int32_t)(65 + i),                evaluatedValue->value.integer);
    }

    for (size_t i = 0; i < 3; ++i)
    {
        evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(port-read f)");

        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
        ASSERT_EQ((int32_t)(65 + i),                evaluatedValue->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_input_file_open_success_test(void)
{
    octaspire_dern_vm_config_t config = octaspire_dern_vm_config_default();

    octaspire_dern_vm_t *vm = octaspire_dern_vm_new_with_config(octaspireDernVmTestAllocator, octaspireDernVmTestStdio, config);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (input-file-open [" OCTASPIRE_DERN_CONFIG_TEST_RES_PATH "octaspire_io_file_open_test.txt]) [f])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    for (size_t i = 0; i < 3; ++i)
    {
        evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(port-read f)");

        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
        ASSERT_EQ((int32_t)(65 + i),                evaluatedValue->value.integer);
    }

    for (size_t i = 0; i < 3; ++i)
    {
        evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(port-read f)");

        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
        ASSERT_EQ((int32_t)(65 + i),                evaluatedValue->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_output_file_open_success_test(void)
{
    octaspire_dern_vm_config_t config = octaspire_dern_vm_config_default();

    octaspire_dern_vm_t *vm = octaspire_dern_vm_new_with_config(octaspireDernVmTestAllocator, octaspireDernVmTestStdio, config);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (output-file-open [" OCTASPIRE_DERN_CONFIG_TEST_RES_PATH "octaspire_io_file_open_test.txt]) [f])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_port_supports_input_question_mark_called_with_output_file_test(void)
{
    octaspire_dern_vm_config_t config = octaspire_dern_vm_config_default();

    octaspire_dern_vm_t *vm = octaspire_dern_vm_new_with_config(octaspireDernVmTestAllocator, octaspireDernVmTestStdio, config);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (output-file-open [" OCTASPIRE_DERN_CONFIG_TEST_RES_PATH "octaspire_io_file_open_test.txt]) [f])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(port-supports-input? f)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_port_supports_input_question_mark_called_with_input_file_test(void)
{
    octaspire_dern_vm_config_t config = octaspire_dern_vm_config_default();

    octaspire_dern_vm_t *vm = octaspire_dern_vm_new_with_config(octaspireDernVmTestAllocator, octaspireDernVmTestStdio, config);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (input-file-open [" OCTASPIRE_DERN_CONFIG_TEST_RES_PATH "octaspire_io_file_open_test.txt]) [f])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(port-supports-input? f)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_port_supports_input_question_mark_called_with_io_file_test(void)
{
    octaspire_dern_vm_config_t config = octaspire_dern_vm_config_default();

    octaspire_dern_vm_t *vm = octaspire_dern_vm_new_with_config(octaspireDernVmTestAllocator, octaspireDernVmTestStdio, config);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (io-file-open [" OCTASPIRE_DERN_CONFIG_TEST_RES_PATH "octaspire_io_file_open_test.txt]) [f])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(port-supports-input? f)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_port_supports_input_question_mark_called_with_integer_failure_test(void)
{
    octaspire_dern_vm_config_t config = octaspire_dern_vm_config_default();

    octaspire_dern_vm_t *vm = octaspire_dern_vm_new_with_config(octaspireDernVmTestAllocator, octaspireDernVmTestStdio, config);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as {D+10} [f])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(port-supports-input? f)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'port-supports-input?' expects port argument.\n"
        "\tAt form: >>>>>>>>>>(port-supports-input? f)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_port_supports_input_question_mark_called_without_arguments_failure_test(void)
{
    octaspire_dern_vm_config_t config = octaspire_dern_vm_config_default();

    octaspire_dern_vm_t *vm = octaspire_dern_vm_new_with_config(octaspireDernVmTestAllocator, octaspireDernVmTestStdio, config);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(port-supports-input?)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'port-supports-input?' expects one argument.\n"
        "\tAt form: >>>>>>>>>>(port-supports-input?)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_port_supports_output_question_mark_called_with_input_file_test(void)
{
    octaspire_dern_vm_config_t config = octaspire_dern_vm_config_default();

    octaspire_dern_vm_t *vm = octaspire_dern_vm_new_with_config(octaspireDernVmTestAllocator, octaspireDernVmTestStdio, config);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (input-file-open [" OCTASPIRE_DERN_CONFIG_TEST_RES_PATH "octaspire_io_file_open_test.txt]) [f])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(port-supports-output? f)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_port_supports_output_question_mark_called_with_output_file_test(void)
{
    octaspire_dern_vm_config_t config = octaspire_dern_vm_config_default();

    octaspire_dern_vm_t *vm = octaspire_dern_vm_new_with_config(octaspireDernVmTestAllocator, octaspireDernVmTestStdio, config);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (output-file-open [" OCTASPIRE_DERN_CONFIG_TEST_RES_PATH "octaspire_io_file_open_test.txt]) [f])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(port-supports-output? f)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_port_supports_output_question_mark_called_with_io_file_test(void)
{
    octaspire_dern_vm_config_t config = octaspire_dern_vm_config_default();

    octaspire_dern_vm_t *vm = octaspire_dern_vm_new_with_config(octaspireDernVmTestAllocator, octaspireDernVmTestStdio, config);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (io-file-open [" OCTASPIRE_DERN_CONFIG_TEST_RES_PATH "octaspire_io_file_open_test.txt]) [f])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(port-supports-output? f)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_port_supports_output_question_mark_called_with_integer_failure_test(void)
{
    octaspire_dern_vm_config_t config = octaspire_dern_vm_config_default();

    octaspire_dern_vm_t *vm = octaspire_dern_vm_new_with_config(octaspireDernVmTestAllocator, octaspireDernVmTestStdio, config);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as {D+10} [f])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(port-supports-output? f)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'port-supports-output?' expects port argument.\n"
        "\tAt form: >>>>>>>>>>(port-supports-output? f)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_port_supports_output_question_mark_called_without_arguments_failure_test(void)
{
    octaspire_dern_vm_config_t config = octaspire_dern_vm_config_default();

    octaspire_dern_vm_t *vm = octaspire_dern_vm_new_with_config(octaspireDernVmTestAllocator, octaspireDernVmTestStdio, config);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(port-supports-output?)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'port-supports-output?' expects one argument.\n"
        "\tAt form: >>>>>>>>>>(port-supports-output?)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_port_close_called_with_io_file_port_test(void)
{
    octaspire_dern_vm_config_t config = octaspire_dern_vm_config_default();

    octaspire_dern_vm_t *vm = octaspire_dern_vm_new_with_config(octaspireDernVmTestAllocator, octaspireDernVmTestStdio, config);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (io-file-open [" OCTASPIRE_DERN_CONFIG_TEST_RES_PATH "octaspire_io_file_open_test.txt]) [f])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    for (size_t i = 0; i < 3; ++i)
    {
        evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(port-read f)");

        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
        ASSERT_EQ((int32_t)(65 + i),                evaluatedValue->value.integer);
    }

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(to-string f)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "<input-output-port:" OCTASPIRE_DERN_CONFIG_TEST_RES_PATH "octaspire_io_file_open_test.txt (7 octets)>",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    // Close port
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(port-close f)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    // Check the closed port
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(to-string f)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "<NOT-OPEN-port:" OCTASPIRE_DERN_CONFIG_TEST_RES_PATH "octaspire_io_file_open_test.txt (-1 octets)>",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    // Check that reading from closed file port fails
    for (size_t i = 0; i < 3; ++i)
    {
        evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(port-read f)");

        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);
        ASSERT_STR_EQ(
            "Builtin 'port-read' failed to read the requested one octet.\n"
            "\tAt form: >>>>>>>>>>(port-read f)<<<<<<<<<<\n",
            octaspire_string_get_c_string(evaluatedValue->value.error->message));
    }

    // Check that closing again fails
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(port-close f)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_port_dist_called_with_a_file_test(void)
{
    octaspire_dern_vm_config_t config = octaspire_dern_vm_config_default();

    octaspire_dern_vm_t *vm = octaspire_dern_vm_new_with_config(octaspireDernVmTestAllocator, octaspireDernVmTestStdio, config);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (io-file-open [" OCTASPIRE_DERN_CONFIG_TEST_RES_PATH "octaspire_io_file_open_test.txt]) [f])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    int32_t dist = 0;

    for (size_t i = 0; i < 3; ++i)
    {
        evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(port-dist f)");

        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
        ASSERT_EQ(dist,                             evaluatedValue->value.integer);

        evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(port-read f)");

        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
        ASSERT_EQ((int32_t)(65 + i),                evaluatedValue->value.integer);

        ++dist;
    }

    for (size_t i = 0; i < 3; ++i)
    {
        evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(port-dist f)");

        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
        ASSERT_EQ(dist,                             evaluatedValue->value.integer);

        evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(port-read f)");

        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
        ASSERT_EQ((int32_t)(65 + i),                evaluatedValue->value.integer);

        ++dist;
    }

    // Close port
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(port-close f)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(port-dist f)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ((int32_t)-1,                      evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_port_seek_called_with_a_file_test(void)
{
    octaspire_dern_vm_config_t config = octaspire_dern_vm_config_default();

    octaspire_dern_vm_t *vm = octaspire_dern_vm_new_with_config(octaspireDernVmTestAllocator, octaspireDernVmTestStdio, config);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (io-file-open [" OCTASPIRE_DERN_CONFIG_TEST_RES_PATH "octaspire_io_file_open_test.txt]) [f])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);



    // From beginning
    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(port-seek f {D+0})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(port-dist f)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(0,                                evaluatedValue->value.integer);



    // From end
    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(port-seek f {D-1})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(port-dist f)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(7,                                evaluatedValue->value.integer);



    // Backward from current pos
    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(port-seek f {D-1} 'from-current)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(port-dist f)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(6,                                evaluatedValue->value.integer);


    // Forward from current pos
    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(port-seek f {D+1} 'from-current)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(port-dist f)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(7,                                evaluatedValue->value.integer);

    // Close port
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(port-close f)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(port-seek f {D+0})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_input_file_open_with_file_system_access_allowed_failure_on_missing_file_test(void)
{
    octaspire_dern_vm_config_t config = octaspire_dern_vm_config_default();

    octaspire_dern_vm_t *vm = octaspire_dern_vm_new_with_config(octaspireDernVmTestAllocator, octaspireDernVmTestStdio, config);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (input-file-open [" OCTASPIRE_DERN_CONFIG_TEST_RES_PATH "no-such-file.nono]) [f])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(to-string f)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "<NOT-OPEN-port:" OCTASPIRE_DERN_CONFIG_TEST_RES_PATH "no-such-file.nono (-1 octets)>",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_port_write_failure_on_input_file_test(void)
{
    octaspire_dern_vm_config_t config = octaspire_dern_vm_config_default();

    octaspire_dern_vm_t *vm = octaspire_dern_vm_new_with_config(octaspireDernVmTestAllocator, octaspireDernVmTestStdio, config);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (input-file-open [" OCTASPIRE_DERN_CONFIG_TEST_RES_PATH "octaspire_io_file_open_test.txt]) [f])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    for (size_t i = 0; i < 3; ++i)
    {
        evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(port-read f)");

        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
        ASSERT_EQ((int32_t)(65 + i),                evaluatedValue->value.integer);
    }

    for (size_t i = 0; i < 3; ++i)
    {
        evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(port-read f)");

        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
        ASSERT_EQ((int32_t)(65 + i),                evaluatedValue->value.integer);
    }

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "f");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_PORT, evaluatedValue->typeTag);

    int32_t const fileLengthInOctets =
        (int32_t)octaspire_dern_port_get_length_in_octets(evaluatedValue->value.port);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(port-write f {D+70})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "The first argument to builtin 'port-write' must be a port supporting writing.\n"
        "\tAt form: >>>>>>>>>>(port-write f {D+70})<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    // Make sure that the file has still the same size as before the write attempt
    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "f");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_PORT, evaluatedValue->typeTag);

    ASSERT_EQ(
        fileLengthInOctets,
        octaspire_dern_port_get_length_in_octets(evaluatedValue->value.port));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_port_length_test(void)
{
    octaspire_dern_vm_config_t config = octaspire_dern_vm_config_default();

    octaspire_dern_vm_t *vm = octaspire_dern_vm_new_with_config(octaspireDernVmTestAllocator, octaspireDernVmTestStdio, config);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (io-file-open [" OCTASPIRE_DERN_CONFIG_TEST_RES_PATH "octaspire_io_file_open_test.txt]) [f])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(port-length f)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(7,                                evaluatedValue->value.integer);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(port-close f)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(port-length f)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(-1,                               evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_integer_question_mark_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(integer? {D+0})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(integer? {D+10})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(integer? {D-1})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(integer? {D-1.0})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(integer? {D+0.0})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(integer? {D+10.0})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(integer? |0|)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(integer? [0])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(integer? '())");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(integer?)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'integer?' expects one argument.\n"
        "\tAt form: >>>>>>>>>>(integer?)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(integer? {D+1} {D+2})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'integer?' expects one argument.\n"
        "\tAt form: >>>>>>>>>>(integer? {D+1} {D+2})<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_real_question_mark_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(real? {D+0.0})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(real? {D+10.0})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(real? {D-1.1})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(real? {D-1})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(real? {D+0})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(real? {D+10})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(real? |0|)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(real? [0.0])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(real? '())");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(real?)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'real?' expects one argument.\n"
        "\tAt form: >>>>>>>>>>(real?)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(real? {D+1.1} {D+1.2})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'real?' expects one argument.\n"
        "\tAt form: >>>>>>>>>>(real? {D+1.1} {D+1.2})<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_number_question_mark_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(number? {D+0})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(number? {D+0.0})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(number? {D-1.1})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(number? {D-1})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(number? {D+2048})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(number? {D-100})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(number? |0|)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(number? [0.0])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(number? '())");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(number?)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'number?' expects one argument.\n"
        "\tAt form: >>>>>>>>>>(number?)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(number? {D+1.1} {D+1.2})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'number?' expects one argument.\n"
        "\tAt form: >>>>>>>>>>(number? {D+1.1} {D+1.2})<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_nil_question_mark_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(nil? nil)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(nil? {D+0})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(nil? [nil])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(nil? {D-1})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(nil? {D+2048})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(nil? |a|)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(define myNil as nil [myNil])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(nil? myNil)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(nil? '())");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(nil?)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'nil?' expects one argument.\n"
        "\tAt form: >>>>>>>>>>(nil?)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(nil? {D+1.1} {D+1.2})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'nil?' expects one argument.\n"
        "\tAt form: >>>>>>>>>>(nil? {D+1.1} {D+1.2})<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_boolean_question_mark_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(boolean? true)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(boolean? false)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                            evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(boolean? [true])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(boolean? {D-1})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(boolean? {D+2048})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(boolean? |a|)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(define myBool as true [myBool])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(boolean? myBool)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(boolean? '())");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(boolean?)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'boolean?' expects one argument.\n"
        "\tAt form: >>>>>>>>>>(boolean?)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(boolean? true true)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'boolean?' expects one argument.\n"
        "\tAt form: >>>>>>>>>>(boolean? true true)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_character_question_mark_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(character? |a|)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(character? |0|)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                            evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(character? [true])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(character? |newline|)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(character? {D+2048})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(character? {D+0})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(define myChar as |a| [myChar])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(character? myChar)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(character? '())");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(character?)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'character?' expects one argument.\n"
        "\tAt form: >>>>>>>>>>(character?)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(character? |a| |b|)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'character?' expects one argument.\n"
        "\tAt form: >>>>>>>>>>(character? |a| |b|)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_string_question_mark_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(string? [])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(string? [0])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(string? |a|)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(string? |newline|)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(string? {D+2048})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(string? {D+0})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(define myStr as [abc] [myStr])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(string? myStr)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(string? '())");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(string?)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'string?' expects one argument.\n"
        "\tAt form: >>>>>>>>>>(string?)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(string? [a] [b])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'string?' expects one argument.\n"
        "\tAt form: >>>>>>>>>>(string? [a] [b])<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_symbol_question_mark_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(symbol? 'a)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(symbol? 'abc123)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(symbol? |a|)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(symbol? |newline|)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(symbol? {D+2048})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(symbol? {D+0})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(define mySym as {D+10} [mySym])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(symbol? 'mySym)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(symbol? '())");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(symbol?)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'symbol?' expects one argument.\n"
        "\tAt form: >>>>>>>>>>(symbol?)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(symbol? 'a 'b)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'symbol?' expects one argument.\n"
        "\tAt form: >>>>>>>>>>(symbol? (quote a) (quote b))<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_vector_question_mark_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(vector? '())");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(vector? '({D+1}))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(vector? '({D+1} {D+2} {D+3}))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(vector? |newline|)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(vector? {D+2048})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(vector? {D+0})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(define mySym as '({D+99} {D+98} {D+97} {D+96}) [mySym])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(vector? mySym)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(vector? (hash-map))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(vector?)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'vector?' expects one argument.\n"
        "\tAt form: >>>>>>>>>>(vector?)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(vector? '() '())");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'vector?' expects one argument.\n"
        "\tAt form: >>>>>>>>>>(vector? (quote ()) (quote ()))<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_hash_map_question_mark_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(hash-map? (hash-map))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(hash-map? (hash-map {D+0} |a|))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(hash-map? (hash-map {D+0} |a| {D+1} [abc]))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(hash-map? |newline|)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(hash-map? {D+2048})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(hash-map? {D+0})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(define myHashMap as (hash-map |a| {D+0}) [myHashMap])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(hash-map? myHashMap)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(hash-map? '())");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(hash-map?)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'hash-map?' expects one argument.\n"
        "\tAt form: >>>>>>>>>>(hash-map?)<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(hash-map? (hash-map) (hash-map))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'hash-map?' expects one argument.\n"
        "\tAt form: >>>>>>>>>>(hash-map? (hash-map) (hash-map))<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_queue_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define q as (queue) [queue])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(len q)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(0,                                evaluatedValue->value.integer);



    // 1. line
    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(+= q [line 1])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_QUEUE, evaluatedValue->typeTag);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(to-string q)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ(
        "(queue [line 1])",
        octaspire_string_get_c_string(evaluatedValue->value.string));



    // 2. line
    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(+= q [line 2])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_QUEUE, evaluatedValue->typeTag);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(to-string q)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ(
        "(queue [line 1] [line 2])",
        octaspire_string_get_c_string(evaluatedValue->value.string));



    // 3. line
    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(+= q [line 3])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_QUEUE, evaluatedValue->typeTag);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(to-string q)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ(
        "(queue [line 1] [line 2] [line 3])",
        octaspire_string_get_c_string(evaluatedValue->value.string));



    // 4. line
    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(+= q [line 4])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_QUEUE, evaluatedValue->typeTag);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(to-string q)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ(
        "(queue [line 1] [line 2] [line 3] [line 4])",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_queue_with_max_length_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define q as (queue-with-max-length {D+3}) [queue])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(len q)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(0,                                evaluatedValue->value.integer);



    // 1. line
    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(+= q [line 1])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_QUEUE, evaluatedValue->typeTag);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(to-string q)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ(
        "(queue [line 1])",
        octaspire_string_get_c_string(evaluatedValue->value.string));



    // 2. line
    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(+= q [line 2])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_QUEUE, evaluatedValue->typeTag);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(to-string q)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ(
        "(queue [line 1] [line 2])",
        octaspire_string_get_c_string(evaluatedValue->value.string));



    // 3. line
    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(+= q [line 3])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_QUEUE, evaluatedValue->typeTag);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(to-string q)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ(
        "(queue [line 1] [line 2] [line 3])",
        octaspire_string_get_c_string(evaluatedValue->value.string));



    // 4. line
    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(+= q [line 4])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_QUEUE, evaluatedValue->typeTag);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(to-string q)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ(
        "(queue [line 2] [line 3] [line 4])",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_list_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define l as (list) [list])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(len l)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(0,                                evaluatedValue->value.integer);



    // 1. line
    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(+= l [line 1])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_LIST, evaluatedValue->typeTag);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(to-string l)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ(
        "(list [line 1])",
        octaspire_string_get_c_string(evaluatedValue->value.string));



    // 2. line
    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(+= l [line 2])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_LIST, evaluatedValue->typeTag);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(to-string l)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ(
        "(list [line 1] [line 2])",
        octaspire_string_get_c_string(evaluatedValue->value.string));



    // 3. line
    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(+= l [line 3])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_LIST, evaluatedValue->typeTag);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(to-string l)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ(
        "(list [line 1] [line 2] [line 3])",
        octaspire_string_get_c_string(evaluatedValue->value.string));



    // 4. line
    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(+= l [line 4])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_LIST, evaluatedValue->typeTag);

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(to-string l)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ(
        "(list [line 1] [line 2] [line 3] [line 4])",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_copy_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(to-string (copy '({D+1} {D+2} {D+3})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ("({D+1} {D+2} {D+3})", octaspire_dern_value_as_string_get_c_string(evaluatedValue));

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(to-string (copy '({D+1} {D+2} {D+3}) (fn (v i) (< i {D+2}))))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ("({D+1} {D+2})", octaspire_dern_value_as_string_get_c_string(evaluatedValue));

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(to-string (copy '({D+1} {D+2} {D+3}) (fn (v i) (== v {D+2}))))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ("({D+2})", octaspire_dern_value_as_string_get_c_string(evaluatedValue));

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(to-string (copy [abc] (fn (v i) (< i {D+2}))))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ("[ab]", octaspire_dern_value_as_string_get_c_string(evaluatedValue));

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(to-string (copy [abc] (fn (v i) (== v |b|))))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ("[b]", octaspire_dern_value_as_string_get_c_string(evaluatedValue));

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(copy {D+123})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(123, octaspire_dern_value_as_integer_get_value(evaluatedValue));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}


TEST octaspire_dern_vm_split_called_with_string_and_char_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(split [here is some text] | |)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);

    size_t const expectedNumOfElems = 4;

    ASSERT_EQ(expectedNumOfElems, octaspire_dern_value_as_vector_get_length(evaluatedValue));

    char const * const expected[] = {"here", "is", "some", "text" };

    for (size_t i = 0; i < expectedNumOfElems; ++i)
    {
        ASSERT_STR_EQ(
            expected[i],
            octaspire_dern_value_as_string_get_c_string(
                octaspire_dern_value_as_vector_get_element_at_const(
                    evaluatedValue,
                    (ptrdiff_t)i)));
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}




TEST octaspire_dern_vm_cp_at_sign_with_vector_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v as '({D+1} {D+2} {D+3}) [v])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    for (int32_t i = 0; i < 10; ++i)
    {
        evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(++ (cp@ v {D+1}))");

        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
        ASSERT_EQ(3,                               evaluatedValue->value.integer);
    }

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(to-string v)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "({D+1} {D+2} {D+3})",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_cp_at_sign_with_string_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s as [abc] [s])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    for (int32_t i = 0; i < 10; ++i)
    {
        evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+= (cp@ s {D+1}) {D+2})");

        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, evaluatedValue->typeTag);

        ASSERT_STR_EQ(
            "d",
            octaspire_string_get_c_string(
                evaluatedValue->value.character));
    }

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "s");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "abc",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_cp_at_sign_with_hash_map_of_size_one_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define h as (hash-map [a] {D+2}) [h])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    for (int32_t i = 0; i < 5; ++i)
    {
        evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(++ (cp@ h {D+0} 'index))");

        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
        ASSERT_EQ(3,                                evaluatedValue->value.integer);
    }

    for (int32_t i = 0; i < 5; ++i)
    {
        evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(++ (cp@ h [a] 'hash))");

        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
        ASSERT_EQ(3,                                evaluatedValue->value.integer);
    }

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(to-string h)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "(hash-map [a] {D+2})",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_cp_at_sign_with_hash_map_of_size_three_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define h as (hash-map [a] {D+1}   [b] {D+2}   [c] {D+3}) [h])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    for (int32_t i = 0; i < 10; ++i)
    {
        evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(++ (cp@ h [b] 'hash))");

        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
        ASSERT_EQ(3,                                evaluatedValue->value.integer);
    }

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(find h [a])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(1, octaspire_dern_value_as_integer_get_value(evaluatedValue));

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(find h [b])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(2, octaspire_dern_value_as_integer_get_value(evaluatedValue));

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(find h [c])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(3, octaspire_dern_value_as_integer_get_value(evaluatedValue));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}






TEST octaspire_dern_vm_ln_at_sign_with_vector_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v as '({D+1} {D+2} {D+3}) [v])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    for (int32_t i = 0; i < 10; ++i)
    {
        evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(++ (ln@ v {D+1}))");

        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
        ASSERT_EQ(3 + i,                            evaluatedValue->value.integer);
    }

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(to-string v)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "({D+1} {D+12} {D+3})",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_ln_at_sign_with_string_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s as [abc] [s])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(ln@ [abc] {D+1})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'ln@' cannot be used with strings. Use 'cp@' instead.\n"
        "\tAt form: >>>>>>>>>>(ln@ [abc] {D+1})<<<<<<<<<<\n",
        octaspire_string_get_c_string(evaluatedValue->value.error->message)); 

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_ln_at_sign_with_hash_map_of_size_one_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define h as (hash-map [a] {D+2}) [h])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    for (int32_t i = 0; i < 5; ++i)
    {
        evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(++ (ln@ h {D+0} 'index))");

        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
        ASSERT_EQ(3 + i,                            evaluatedValue->value.integer);
    }

    for (int32_t i = 0; i < 5; ++i)
    {
        evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(++ (ln@ h [a] 'hash))");

        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
        ASSERT_EQ(3 + 5 + i,                        evaluatedValue->value.integer);
    }

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(to-string h)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "(hash-map [a] {D+12})",
        octaspire_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_ln_at_sign_with_hash_map_of_size_three_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define h as (hash-map [a] {D+1}   [b] {D+2}   [c] {D+3}) [h])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    for (int32_t i = 0; i < 10; ++i)
    {
        evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(++ (ln@ h [b] 'hash))");

        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
        ASSERT_EQ(3 + i,                            evaluatedValue->value.integer);
    }

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(find h [a])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(1, octaspire_dern_value_as_integer_get_value(evaluatedValue));

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(find h [b])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(12, octaspire_dern_value_as_integer_get_value(evaluatedValue));

    evaluatedValue = octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
        vm,
        "(find h [c])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(3, octaspire_dern_value_as_integer_get_value(evaluatedValue));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_host_command_line_arguments_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    // Test with missing command line arguments
    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(to-string (host-get-command-line-arguments))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "()",
        octaspire_dern_value_as_string_get_c_string(evaluatedValue));

    // Test with existing command line arguments
    char const * const expected[] =
    {
        "a",
        "\"this is 2. argument\"",
        "--do-something",
        "1234",
        "-c"
    };

    for (size_t i = 0; i < 5; ++i)
    {
        ASSERT(octaspire_dern_vm_add_command_line_argument(vm, expected[i]));
    }

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(to-string (host-get-command-line-arguments))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "([a] [\"this is 2. argument\"] [--do-something] [1234] [-c])",
        octaspire_dern_value_as_string_get_c_string(evaluatedValue));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_host_environment_variables_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    // Test with missing environment variables
    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(to-string (host-get-environment-variables))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "()",
        octaspire_dern_value_as_string_get_c_string(evaluatedValue));

    // Test with existing environment variables
    char const * const expected[] =
    {
        "a=0",
        "LD_LIBRARY_PATH=/some_path/xyz",
        "b=1",
        "LANGUAGE=en_US:en",
        "_=/usr/bin/env"
    };

    for (size_t i = 0; i < 5; ++i)
    {
        ASSERT(octaspire_dern_vm_add_environment_variable(vm, expected[i]));
    }

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(to-string (host-get-environment-variables))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "([a=0] [LD_LIBRARY_PATH=/some_path/xyz] [b=1] "
        "[LANGUAGE=en_US:en] [_=/usr/bin/env])",
        octaspire_dern_value_as_string_get_c_string(evaluatedValue));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

static char const * const octaspireDernVmTestCreateNewUserDataTestPluginName =
    "octaspireDernUnitTestPluginName";

static char const * const octaspireDernVmTestCreateNewUserDataTestPayloadTypeName =
    "payloadTypeName";

static char const * const octaspireDernVmTestCreateNewUserDataTestPayload =
    "This is the payload...";

octaspire_dern_value_t *octaspire_dern_test_dern_vm_create_new_user_data(
    octaspire_dern_vm_t * const vm,
    octaspire_dern_value_t * const arguments,
    octaspire_dern_value_t * const environment)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(arguments);
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(environment);

    return octaspire_dern_vm_create_new_value_c_data(
            vm,
            octaspireDernVmTestCreateNewUserDataTestPluginName,
            octaspireDernVmTestCreateNewUserDataTestPayloadTypeName,
            "",
            "",
            "",
            "",
            "",
            "",
            true,
            (void*)octaspireDernVmTestCreateNewUserDataTestPayload);
}

TEST octaspire_dern_vm_create_user_data_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    ASSERT(octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "octaspire-dern-test-dern-vm-create-new-user-data",
            octaspire_dern_test_dern_vm_create_new_user_data,
            0,
            "...",
            false,
            octaspire_dern_value_as_environment_get_value(
                octaspire_dern_vm_get_global_environment(vm))));

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(octaspire-dern-test-dern-vm-create-new-user-data)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_C_DATA, evaluatedValue->typeTag);

    octaspire_dern_c_data_t const * const cData =
        octaspire_dern_value_as_c_data_get_value(evaluatedValue);

    ASSERT_STR_EQ(
        octaspireDernVmTestCreateNewUserDataTestPluginName,
        octaspire_dern_c_data_get_plugin_name(cData));

    ASSERT_STR_EQ(
        octaspireDernVmTestCreateNewUserDataTestPayloadTypeName,
        octaspire_dern_c_data_get_payload_typename(cData));

    ASSERT_EQ(
        octaspireDernVmTestCreateNewUserDataTestPayload,
        octaspire_dern_c_data_get_payload(cData));

    ASSERT_STR_EQ(
        (char const * const)octaspireDernVmTestCreateNewUserDataTestPayload,
        (char const * const)octaspire_dern_c_data_get_payload(cData));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_copy_user_data_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    ASSERT(octaspire_dern_vm_create_and_register_new_builtin(
            vm,
            "octaspire-dern-test-dern-vm-create-new-user-data",
            octaspire_dern_test_dern_vm_create_new_user_data,
            0,
            "...",
            false,
            octaspire_dern_value_as_environment_get_value(
                octaspire_dern_vm_get_global_environment(vm))));

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define a as (octaspire-dern-test-dern-vm-create-new-user-data) [a])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, octaspire_dern_value_as_boolean_get_value(evaluatedValue));

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define b as (copy a) [b])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, octaspire_dern_value_as_boolean_get_value(evaluatedValue));

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(= a {D+1})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(1, octaspire_dern_value_as_integer_get_value(evaluatedValue));

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "b");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_C_DATA, evaluatedValue->typeTag);

    octaspire_dern_c_data_t const * const cData =
        octaspire_dern_value_as_c_data_get_value(evaluatedValue);

    ASSERT_STR_EQ(
        octaspireDernVmTestCreateNewUserDataTestPluginName,
        octaspire_dern_c_data_get_plugin_name(cData));

    ASSERT_STR_EQ(
        octaspireDernVmTestCreateNewUserDataTestPayloadTypeName,
        octaspire_dern_c_data_get_payload_typename(cData));

    ASSERT_EQ(
        octaspireDernVmTestCreateNewUserDataTestPayload,
        octaspire_dern_c_data_get_payload(cData));

    ASSERT_STR_EQ(
        (char const * const)octaspireDernVmTestCreateNewUserDataTestPayload,
        (char const * const)octaspire_dern_c_data_get_payload(cData));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

octaspire_input_t *octaspire_test_dern_vm_custom_require_file_loader(
    char const * const name,
    octaspire_allocator_t * const allocator)
{
    if (strcmp("test1.dern", name) == 0)
    {
        return octaspire_input_new_from_c_string(
            "(define f1 as (fn (a b) (+ a b)) [f1] '(a [a] b [b]) howto-ok)",
            allocator);
    }
    else if (strcmp("test2.dern", name) == 0)
    {
        return octaspire_input_new_from_c_string(
            "(define f2 as (fn (a b) (* a b)) [f2] '(a [a] b [b]) howto-ok)",
            allocator);
    }

    return 0;
}

TEST octaspire_dern_vm_require_a_source_library_test(void)
{
    octaspire_dern_vm_config_t config = octaspire_dern_vm_config_default();
    config.preLoaderForRequireSrc = octaspire_test_dern_vm_custom_require_file_loader;

    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new_with_config(
            octaspireDernVmTestAllocator,
            octaspireDernVmTestStdio,
            config);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(require 'test1)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "test1.dern",
        octaspire_dern_value_as_string_get_c_string(evaluatedValue));



    // Test function from the 'test1' library
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(f1 {D+2} {D+10})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(12, octaspire_dern_value_as_integer_get_value(evaluatedValue));



    // Load 'test2' library
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(require 'test2)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "test2.dern",
        octaspire_dern_value_as_string_get_c_string(evaluatedValue));



    // Test function from the 'test2' library
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(f2 {D+2} {D+10})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(20, octaspire_dern_value_as_integer_get_value(evaluatedValue));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_howto_1_2_3_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(howto {D+1} {D+2} {D+3})");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);

    octaspire_string_t *tmpStr = octaspire_dern_value_to_string(
        evaluatedValue,
        octaspire_dern_vm_get_allocator(vm));

    ASSERT_STR_EQ(
        "((+ {D+1} {D+2}) (+ {D+2} {D+1}))",
        octaspire_string_get_c_string(tmpStr));

    octaspire_string_release(tmpStr);
    tmpStr = 0;

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_howto_strings_a_b_ab_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(howto [a] [b] [ab])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);

    octaspire_string_t *tmpStr = octaspire_dern_value_to_string(
        evaluatedValue,
        octaspire_dern_vm_get_allocator(vm));

    ASSERT_STR_EQ(
        "((+ [a] [b]))",
        octaspire_string_get_c_string(tmpStr));

    octaspire_string_release(tmpStr);
    tmpStr = 0;

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_howto_chars_a_b_string_ab_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(howto |a| |b| [ab])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);

    octaspire_string_t *tmpStr = octaspire_dern_value_to_string(
        evaluatedValue,
        octaspire_dern_vm_get_allocator(vm));

    ASSERT_STR_EQ(
        "((+ |a| |b|))",
        octaspire_string_get_c_string(tmpStr));

    octaspire_string_release(tmpStr);
    tmpStr = 0;

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_howto_symbols_a_b_ab_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(howto 'a 'b 'ab)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);

    octaspire_string_t *tmpStr = octaspire_dern_value_to_string(
        evaluatedValue,
        octaspire_dern_vm_get_allocator(vm));

    ASSERT_STR_EQ(
        "((+ (quote a) (quote b)))",
        octaspire_string_get_c_string(tmpStr));

    octaspire_string_release(tmpStr);
    tmpStr = 0;

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_howto_hash_map_1_a_and_2_b_and_hash_map_1_a_2_b_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(howto (hash-map {D+1} |a|) {D+2} |b| (hash-map {D+1} |a| {D+2} |b|))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);

    octaspire_string_t *tmpStr = octaspire_dern_value_to_string(
        evaluatedValue,
        octaspire_dern_vm_get_allocator(vm));

    ASSERT_STR_EQ(
        "((+ (hash-map {D+1} |a|) {D+2} |b|))",
        octaspire_string_get_c_string(tmpStr));

    octaspire_string_release(tmpStr);
    tmpStr = 0;

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_compare_vectors_of_integers_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(< '({D+1} {D+2} {D+3}) '({D+1} {D+2} {D+4}))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= '({D+1} {D+2} {D+3}) '({D+1} {D+2} {D+4}))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= '({D+1} {D+2} {D+3}) '({D+1} {D+2} {D+3}))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(< '({D+1} {D+2} {D+3}) '({D+1} {D+2} {D+3}))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== '({D+1} {D+2} {D+3}) '({D+1} {D+2} {D+3}))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= '({D+1} {D+2} {D+3}) '({D+1} {D+2} {D+3}))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= '({D+1} {D+2} {D+3}) '({D+1} {D+2} {D+4}))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(> '({D+1} {D+2} {D+3}) '({D+1} {D+2} {D+3}))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(>= '({D+1} {D+2} {D+3}) '({D+1} {D+2} {D+3}))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_compare_vectors_of_vectors_of_integers_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(< '(({D+1} {D+2}) ({D+3} {D+4})) '(({D+1} {D+2}) ({D+3} {D+5})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= '(({D+1} {D+2}) ({D+3} {D+4})) '(({D+1} {D+2}) ({D+3} {D+5})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= '(({D+1} {D+2}) ({D+3} {D+4})) '(({D+1} {D+2}) ({D+3} {D+4})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(< '(({D+1} {D+2}) ({D+3} {D+4})) '(({D+1} {D+2}) ({D+3} {D+4})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== '(({D+1} {D+2}) ({D+3} {D+4})) '(({D+1} {D+2}) ({D+3} {D+4})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= '(({D+1} {D+2}) ({D+3} {D+4})) '(({D+1} {D+2}) ({D+3} {D+4})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= '(({D+1} {D+2}) ({D+3} {D+4})) '(({D+1} {D+2}) ({D+3} {D+5})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(> '(({D+1} {D+2}) ({D+3} {D+4})) '(({D+1} {D+2}) ({D+3} {D+4})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(>= '(({D+1} {D+2}) ({D+3} {D+4})) '(({D+1} {D+2}) ({D+3} {D+4})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_compare_vectors_of_vectors_of_reals_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(< '(({D+1.0} {D+2.0}) ({D+3.0} {D+4.0})) '(({D+1.0} {D+2.0}) ({D+3.0} {D+4.0001})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= '(({D+1.0} {D+2.0}) ({D+3.0} {D+4.0})) '(({D+1.0} {D+2.0}) ({D+3.0} {D+4.0001})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= '(({D+1.0} {D+2.0}) ({D+3.0} {D+4.0})) '(({D+1.0} {D+2.0}) ({D+3.0} {D+4.0})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(< '(({D+1.0} {D+2.0}) ({D+3.0} {D+4.0})) '(({D+1.0} {D+2.0}) ({D+3.0} {D+4.0})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== '(({D+1.0} {D+2.0}) ({D+3.0} {D+4.0})) '(({D+1.0} {D+2.0}) ({D+3.0} {D+4.0})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= '(({D+1.0} {D+2.0}) ({D+3.0} {D+4.0})) '(({D+1.0} {D+2.0}) ({D+3.0} {D+4.0})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= '(({D+1.0} {D+2.0}) ({D+3.0} {D+4.0})) '(({D+1.0} {D+2.0}) ({D+3.0} {D+4.0001})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(> '(({D+1.0} {D+2.0}) ({D+3.0} {D+4.0})) '(({D+1.0} {D+2.0}) ({D+3.0} {D+4.0})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(>= '(({D+1.0} {D+2.0}) ({D+3.0} {D+4.0})) '(({D+1.0} {D+2.0}) ({D+3.0} {D+4.0})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_compare_vectors_of_vectors_of_integers_and_reals_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(< '(({D+1.0} {D+2.0}) ({D+3.0} {D+4})) '(({D+1.0} {D+2.0}) ({D+3.0} {D+4.0001})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= '(({D+1.0} {D+2.0}) ({D+3.0} {D+4})) '(({D+1.0} {D+2.0}) ({D+3.0} {D+4.0001})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= '(({D+1.0} {D+2.0}) ({D+3.0} {D+4})) '(({D+1.0} {D+2.0}) ({D+3.0} {D+4.0})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(< '(({D+1.0} {D+2.0}) ({D+3.0} {D+4})) '(({D+1.0} {D+2.0}) ({D+3.0} {D+4.0})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== '(({D+1.0} {D+2.0}) ({D+3.0} {D+4})) '(({D+1.0} {D+2.0}) ({D+3.0} {D+4.0})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= '(({D+1.0} {D+2.0}) ({D+3.0} {D+4})) '(({D+1.0} {D+2.0}) ({D+3.0} {D+4.0})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= '(({D+1.0} {D+2.0}) ({D+3.0} {D+4})) '(({D+1.0} {D+2.0}) ({D+3.0} {D+4.0001})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(> '(({D+1.0} {D+2.0}) ({D+3.0} {D+4})) '(({D+1.0} {D+2.0}) ({D+3.0} {D+4.0})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(>= '(({D+1.0} {D+2.0}) ({D+3.0} {D+4})) '(({D+1.0} {D+2.0}) ({D+3.0} {D+4.0})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_compare_vectors_of_vectors_of_strings_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(< '(([a] [b]) ([c] [d])) '(([a] [b]) ([c] [e])))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= '(([a] [b]) ([c] [d])) '(([a] [b]) ([c] [e])))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= '(([a] [b]) ([c] [d])) '(([a] [b]) ([c] [d])))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(< '(([a] [b]) ([c] [d])) '(([a] [b]) ([c] [d])))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== '(([a] [b]) ([c] [d])) '(([a] [b]) ([c] [d])))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= '(([a] [b]) ([c] [d])) '(([a] [b]) ([c] [d])))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= '(([a] [b]) ([c] [d])) '(([a] [b]) ([c] [e])))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(> '(([a] [b]) ([c] [d])) '(([a] [b]) ([c] [d])))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(>= '(([a] [b]) ([c] [d])) '(([a] [b]) ([c] [d])))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_compare_vectors_of_vectors_of_chars_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(< '((|a| |b|) (|c| |d|)) '((|a| |b|) (|c| |e|)))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= '((|a| |b|) (|c| |d|)) '((|a| |b|) (|c| |e|)))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= '((|a| |b|) (|c| |d|)) '((|a| |b|) (|c| |d|)))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(< '((|a| |b|) (|c| |d|)) '((|a| |b|) (|c| |d|)))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== '((|a| |b|) (|c| |d|)) '((|a| |b|) (|c| |d|)))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= '((|a| |b|) (|c| |d|)) '((|a| |b|) (|c| |d|)))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= '((|a| |b|) (|c| |d|)) '((|a| |b|) (|c| |e|)))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(> '((|a| |b|) (|c| |d|)) '((|a| |b|) (|c| |d|)))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(>= '((|a| |b|) (|c| |d|)) '((|a| |b|) (|c| |d|)))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_compare_vectors_of_vectors_of_misc_types_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(< '(({D+1} |a|) ([b] c)) '(({D+1} |a|) ([b] d)))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= '(({D+1} |a|) ([b] c)) '(({D+1} |a|) ([b] d)))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= '(({D+1} |a|) ([b] c)) '(({D+1} |a|) ([b] c)))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(< '(({D+1} |a|) ([b] c)) '(({D+1} |a|) ([b] c)))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== '(({D+1} |a|) ([b] c)) '(({D+1} |a|) ([b] c)))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= '(({D+1} |a|) ([b] c)) '(({D+1} |a|) ([b] c)))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= '(({D+1} |a|) ([b] c)) '(({D+1} |a|) ([b] d)))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(> '(({D+1} |a|) ([b] c)) '(({D+1} |a|) ([b] c)))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(>= '(({D+1} |a|) ([b] c)) '(({D+1} |a|) ([b] c)))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_compare_functions_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(< (fn (a) (+ a a)) (fn (b) (+ b b)))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= (fn (a) (+ a a)) (fn (b) (+ b b)))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= (fn (a) (+ a a)) (fn (a) (+ a a)))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(< (fn (a) (+ a a)) (fn (a) (+ a a)))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== (fn (a) (+ a a)) (fn (a) (+ a a)))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= (fn (a) (+ a a)) (fn (a) (+ a a)))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= (fn (a) (+ a a)) (fn (b) (+ b b)))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(> (fn (a) (+ a a)) (fn (a) (+ a a)))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(>= (fn (a) (+ a a)) (fn (a) (+ a a)))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_compare_hash_maps_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(< (hash-map {D+1} |a|) (hash-map {D+1} |b|))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= (hash-map {D+1} |a|) (hash-map {D+1} |b|))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= (hash-map {D+1} |a|) (hash-map {D+1} |a|))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(< (hash-map {D+1} |a|) (hash-map {D+1} |a|))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== (hash-map {D+1} |a|) (hash-map {D+1} |a|))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= (hash-map {D+1} |a|) (hash-map {D+1} |a|))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= (hash-map {D+1} |a|) (hash-map {D+1} |b|))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(> (hash-map {D+1} |a|) (hash-map {D+1} |a|))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(>= (hash-map {D+1} |a|) (hash-map {D+1} |a|))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_compare_hash_maps_of_vectors_of_integers_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(< (hash-map {D+1} '({D+1} {D+2} {D+3})) (hash-map {D+1} '({D+1} {D+2} {D+4})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= (hash-map {D+1} '({D+1} {D+2} {D+3})) (hash-map {D+1} '({D+1} {D+2} {D+4})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= (hash-map {D+1} '({D+1} {D+2} {D+3})) (hash-map {D+1} '({D+1} {D+2} {D+3})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(< (hash-map {D+1} '({D+1} {D+2} {D+3})) (hash-map {D+1} '({D+1} {D+2} {D+3})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== (hash-map {D+1} '({D+1} {D+2} {D+3})) (hash-map {D+1} '({D+1} {D+2} {D+3})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= (hash-map {D+1} '({D+1} {D+2} {D+3})) (hash-map {D+1} '({D+1} {D+2} {D+3})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= (hash-map {D+1} '({D+1} {D+2} {D+3})) (hash-map {D+1} '({D+1} {D+2} {D+4})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(> (hash-map {D+1} '({D+1} {D+2} {D+3})) (hash-map {D+1} '({D+1} {D+2} {D+3})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(>= (hash-map {D+1} '({D+1} {D+2} {D+3})) (hash-map {D+1} '({D+1} {D+2} {D+3})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_compare_hash_maps_of_lists_of_integers_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(< (hash-map {D+1} (list {D+1} {D+2} {D+3})) (hash-map {D+1} (list {D+1} {D+2} {D+4})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= (hash-map {D+1} (list {D+1} {D+2} {D+3})) (hash-map {D+1} (list {D+1} {D+2} {D+4})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= (hash-map {D+1} (list {D+1} {D+2} {D+3})) (hash-map {D+1} (list {D+1} {D+2} {D+3})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(< (hash-map {D+1} (list {D+1} {D+2} {D+3})) (hash-map {D+1} (list {D+1} {D+2} {D+3})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== (hash-map {D+1} (list {D+1} {D+2} {D+3})) (hash-map {D+1} (list {D+1} {D+2} {D+3})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= (hash-map {D+1} (list {D+1} {D+2} {D+3})) (hash-map {D+1} (list {D+1} {D+2} {D+3})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= (hash-map {D+1} (list {D+1} {D+2} {D+3})) (hash-map {D+1} (list {D+1} {D+2} {D+4})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(> (hash-map {D+1} (list {D+1} {D+2} {D+3})) (hash-map {D+1} (list {D+1} {D+2} {D+3})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(>= (hash-map {D+1} (list {D+1} {D+2} {D+3})) (hash-map {D+1} (list {D+1} {D+2} {D+3})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_compare_hash_maps_of_queues_of_integers_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(< (hash-map {D+1} (queue {D+1} {D+2} {D+3})) (hash-map {D+1} (queue {D+1} {D+2} {D+4})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= (hash-map {D+1} (queue {D+1} {D+2} {D+3})) (hash-map {D+1} (queue {D+1} {D+2} {D+4})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= (hash-map {D+1} (queue {D+1} {D+2} {D+3})) (hash-map {D+1} (queue {D+1} {D+2} {D+3})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(< (hash-map {D+1} (queue {D+1} {D+2} {D+3})) (hash-map {D+1} (queue {D+1} {D+2} {D+3})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== (hash-map {D+1} (queue {D+1} {D+2} {D+3})) (hash-map {D+1} (queue {D+1} {D+2} {D+3})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= (hash-map {D+1} (queue {D+1} {D+2} {D+3})) (hash-map {D+1} (queue {D+1} {D+2} {D+3})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= (hash-map {D+1} (queue {D+1} {D+2} {D+3})) (hash-map {D+1} (queue {D+1} {D+2} {D+4})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(> (hash-map {D+1} (queue {D+1} {D+2} {D+3})) (hash-map {D+1} (queue {D+1} {D+2} {D+3})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(>= (hash-map {D+1} (queue {D+1} {D+2} {D+3})) (hash-map {D+1} (queue {D+1} {D+2} {D+3})))");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_compare_environments_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    // Create environments
    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define e1 as (env-new) [e1])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define e2 as (env-new) [e2])");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    // Add values to environments
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define a as {D+1} [a] in e1)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define a as {D+2} [a] in e2)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    // Compare environments
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(< e1 e2)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= e1 e2)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    // Change environment e2
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define a as {D+1} [a] in e2)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(<= e1 e2)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(< e1 e2)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(== e1 e2)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= e1 e2)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    // Change environment e1
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define a as {D+0} [a] in e1)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(!= e1 e2)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(> e1 e2)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false, evaluatedValue->value.boolean);

    // Change environment e2
    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define a as {D-1000} [a] in e2)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(>= e1 e2)");

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true, evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_eval_empty_vector_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "()");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Cannot evaluate empty vector '()'",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_eval_vector_containing_empty_vector_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(())");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Cannot evaluate operator of type 'error' (<error>: Cannot evaluate "
        "empty vector '()')",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_eval_right_parenthesis_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            ")");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "unexpected token: line=1,1 column=1,1 ucsIndex=0,0 type=OCTASPIRE_DERN_LEXER_TOKEN_TAG_RPAREN value=right parenthesis",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_eval_unbalanced_parenthesis_1_test(void)
{
    octaspire_dern_vm_t *vm =
        octaspire_dern_vm_new(octaspireDernVmTestAllocator, octaspireDernVmTestStdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f as (fn () [f] '() howto-no)(define x as {D+10} [x])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Balancing right parenthesis ')' missing for left parenthesis "
        "given at column 1 of line 1",
        octaspire_string_get_c_string(evaluatedValue->value.error->message));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

GREATEST_SUITE(octaspire_dern_vm_suite)
{
    octaspireDernVmTestAllocator = octaspire_allocator_new(0);
    octaspireDernVmTestStdio     = octaspire_stdio_new(octaspireDernVmTestAllocator);

    assert(octaspireDernVmTestAllocator);
    assert(octaspireDernVmTestStdio);

    RUN_TEST(octaspire_dern_vm_new_test);
    RUN_TEST(octaspire_dern_vm_create_new_value_boolean_test);
    RUN_TEST(octaspire_dern_vm_create_new_value_integer_test);
    RUN_TEST(octaspire_dern_vm_create_new_value_boolean_and_push_one_test);
    RUN_TEST(octaspire_dern_vm_vm_parse_and_eval_true_amid_whitespace_test);

    RUN_TEST(octaspire_dern_vm_special_if_two_elements_false_test);
    RUN_TEST(octaspire_dern_vm_special_if_two_elements_true_test);
    RUN_TEST(octaspire_dern_vm_special_if_three_elements_false_test);
    RUN_TEST(octaspire_dern_vm_special_if_three_elements_true_test);
    RUN_TEST(octaspire_dern_vm_special_if_three_elements_with_function_resulting_true_test);
    RUN_TEST(octaspire_dern_vm_special_if_three_elements_with_function_call_resulting_true_test);
    RUN_TEST(octaspire_dern_vm_special_if_called_with_one_argument_failure_test);
    RUN_TEST(octaspire_dern_vm_special_if_called_with_integer_as_the_first_argument_failure_test);

    RUN_TEST(octaspire_dern_vm_special_select_one_true_selector_to_string_a_test);
    RUN_TEST(octaspire_dern_vm_special_select_one_false_selector_to_string_a_test);
    RUN_TEST(octaspire_dern_vm_special_select_one_default_selector_to_string_a_test);
    RUN_TEST(octaspire_dern_vm_special_select_one_false_and_one_default_selectors_to_string_a_test);
    RUN_TEST(octaspire_dern_vm_special_select_one_false_and_one_true_and_one_default_selectors_to_string_a_test);
    RUN_TEST(octaspire_dern_vm_special_select_one_false_and_one_true_selectors_to_string_a_test);
    RUN_TEST(octaspire_dern_vm_special_select_function_selectors_evaluating_into_false_and_true_to_string_a_test);
    RUN_TEST(octaspire_dern_vm_special_select_function_selectors_failure_on_unknown_symbol_test);
    RUN_TEST(octaspire_dern_vm_special_select_called_with_zero_arguments_failure_test);
    RUN_TEST(octaspire_dern_vm_special_select_called_with_one_argument_failure_test);
    RUN_TEST(octaspire_dern_vm_special_select_called_with_three_arguments_failure_test);
    RUN_TEST(octaspire_dern_vm_special_select_called_non_boolean_selector_failure_test);
    RUN_TEST(octaspire_dern_vm_special_select_called_with_default_as_first_selector_failure_test);

    RUN_TEST(octaspire_dern_vm_special_define_integer_value_test);
    RUN_TEST(octaspire_dern_vm_special_define_integer_value_with_explicit_target_global_env_test);
    RUN_TEST(octaspire_dern_vm_special_define_my_inc_function_test);
    RUN_TEST(octaspire_dern_vm_special_define_factorial_function_with_integers_test);
    RUN_TEST(octaspire_dern_vm_special_define_factorial_function_with_reals_test);
    RUN_TEST(octaspire_dern_vm_special_define_called_with_three_arguments_failure_test);
    RUN_TEST(octaspire_dern_vm_special_define_called_with_eight_arguments_test);
    RUN_TEST(octaspire_dern_vm_special_define_called_with_four_arguments_first_being_value_failure_test);
    RUN_TEST(octaspire_dern_vm_special_define_called_with_eight_arguments_first_being_integer_failure_test);
    RUN_TEST(octaspire_dern_vm_special_define_called_with_four_arguments_error_at_first_failure_test);
    RUN_TEST(octaspire_dern_vm_special_define_called_with_four_arguments_with_docstring_being_integer_failure_test);
    RUN_TEST(octaspire_dern_vm_special_define_called_with_four_arguments_name_evaluates_into_integer_failure_test);
    RUN_TEST(octaspire_dern_vm_special_define_called_with_six_arguments_docstring_is_integer_failure_test);
    RUN_TEST(octaspire_dern_vm_special_define_called_with_eight_arguments_error_in_environment_argument_failure_test);
    RUN_TEST(octaspire_dern_vm_special_define_called_with_four_arguments_integer_as_docstring_failure_test);

    RUN_TEST(octaspire_dern_vm_special_quote_called_without_arguments_failure_test);

    RUN_TEST(octaspire_dern_vm_builtin_max_integers_0_failure_test);
    RUN_TEST(octaspire_dern_vm_builtin_max_integers_0_1_test);
    RUN_TEST(octaspire_dern_vm_builtin_max_integers_10_11_12_test);
    RUN_TEST(octaspire_dern_vm_builtin_max_integers_12_11_10_minus_100_test);
    RUN_TEST(octaspire_dern_vm_builtin_max_strings_abc_abd_abe_test);
    RUN_TEST(octaspire_dern_vm_builtin_max_strings_abe_abd_abc_test);

    RUN_TEST(octaspire_dern_vm_builtin_min_integers_0_failure_test);
    RUN_TEST(octaspire_dern_vm_builtin_min_integers_0_1_test);
    RUN_TEST(octaspire_dern_vm_builtin_min_integers_10_11_12_test);
    RUN_TEST(octaspire_dern_vm_builtin_min_integers_12_11_10_minus_100_test);
    RUN_TEST(octaspire_dern_vm_builtin_min_strings_abc_abd_abe_test);
    RUN_TEST(octaspire_dern_vm_builtin_min_strings_abe_abd_abc_test);

    RUN_TEST(octaspire_dern_vm_builtin_distance_integers_0_failure_test);
    RUN_TEST(octaspire_dern_vm_builtin_distance_integers_0_1_2_failure_test);
    RUN_TEST(octaspire_dern_vm_builtin_distance_integers_0_1_test);
    RUN_TEST(octaspire_dern_vm_builtin_distance_reals_0dot1_3dot14_test);
    RUN_TEST(octaspire_dern_vm_builtin_distance_10_0dot1_test);

    RUN_TEST(octaspire_dern_vm_builtin_plus_plus_integer_value_test);
    RUN_TEST(octaspire_dern_vm_builtin_doc_for_integer_value_test);
    RUN_TEST(octaspire_dern_vm_builtin_read_and_eval_path_test);
    RUN_TEST(octaspire_dern_vm_builtin_read_and_eval_string_test);
    RUN_TEST(octaspire_dern_vm_builtin_slash_1_test);
    RUN_TEST(octaspire_dern_vm_builtin_slash_10_2_2_test);
    RUN_TEST(octaspire_dern_vm_builtin_slash_0_2_2_test);
    RUN_TEST(octaspire_dern_vm_builtin_slash_100_10_0dot5_test);
    RUN_TEST(octaspire_dern_vm_builtin_slash_100_minus10_test);
    RUN_TEST(octaspire_dern_vm_builtin_slash_failure_test);
    RUN_TEST(octaspire_dern_vm_builtin_slash_0_failure_test);
    RUN_TEST(octaspire_dern_vm_builtin_slash_10_2_0_failure_test);
    RUN_TEST(octaspire_dern_vm_builtin_slash_10_2_character_a_failure_test);
    RUN_TEST(octaspire_dern_vm_builtin_mod_5_mod_3_test);
    RUN_TEST(octaspire_dern_vm_builtin_mod_0_mod_3_test);
    RUN_TEST(octaspire_dern_vm_builtin_mod_3_mod_3_test);
    RUN_TEST(octaspire_dern_vm_builtin_mod_4_mod_3_test);
    RUN_TEST(octaspire_dern_vm_builtin_mod_4_mod_0_failure_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_1_2_3_4_minus_2_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_1_2_3_4_minus_2_2dot5_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_string_cat_dog_and_string_space_and_string_zebra_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_string_ca_and_character_t_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_empty_string_and_characters_k_i_t_t_e_n_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_vector_1_2_3_and_4_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_vector_1_2_3_and_4_and_a_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_empty_vector_and_1_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_empty_vector_and_empty_vector_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_empty_vector_and_empty_vector_and_empty_vector_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_empty_hash_map_and_1_and_a_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_hash_map_1_a_and_2_and_b_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_hash_map_1_a_and_2_and_b_and_3_and_c_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_semver_1_1_1_and_1_0_0_and_0_2_0_and_0_0_3_test);
    RUN_TEST(octaspire_dern_vm_builtin_minus_semver_3_4_5_and_1_0_0_and_0_2_0_and_0_0_3_test);
    RUN_TEST(octaspire_dern_vm_builtin_minus_string_cat_dog_cat_and_string_cat_test);
    RUN_TEST(octaspire_dern_vm_builtin_minus_string_abcabcabc_and_character_a_test);
    RUN_TEST(octaspire_dern_vm_builtin_minus_string_abcabcabc_and_character_a_and_character_b_test);
    RUN_TEST(octaspire_dern_vm_builtin_minus_string_abcabcabc_and_character_a_and_character_b_and_caharacter_c_test);
    RUN_TEST(octaspire_dern_vm_builtin_minus_1_2_3_4_minus_2_test);
    RUN_TEST(octaspire_dern_vm_builtin_minus_1_2_3_4_minus_2_2dot5_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_string_abcdeaaba_character_a_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_string_abcdeaaba_character_y_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_string_cat_dog_cat_zebra_cat_string_cat_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_string_cat_dog_cat_zebra_cat_string_parrot_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_string_cat_dog_cat_zebra_cat_quoted_symbol_cat_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_string_cat_dog_cat_zebra_cat_quoted_symbol_parrot_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_string_123_124_abc_123_99_123_integer_123_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_string_123_124_abc_123_99_123_integer_777_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_string_3dot14_abc_3dot13_3dot14_3dot146_3dot14_real_3dot14_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_string_3dot14_abc_3dot13_3dot14_3dot146_3dot14_real_7dot11_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_symbol_abc_def_aaxa_char_a_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_symbol_abc_def_aaxa_char_y_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_symbol_cat_dog_cat_cat_zebra_cat_string_cat_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_symbol_cat_dog_cat_cat_zebra_cat_string_parrot_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_symbol_cat_dog_cat_cat_zebra_cat_symbol_cat_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_symbol_cat_dog_cat_cat_zebra_cat_symbol_parrot_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_symbol_cat_123_dog_cat_123_123_cat_zebra_123_integer_123_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_symbol_cat_321_dog_cat_321_321_cat_zebra_321_integer_123_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_symbol_cat_3dot14_dog_cat_3dot14_3dot14_cat_zebra_3dot14_real_3dot14_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_symbol_cat_3dot14_dog_cat_3dot14_3dot14_cat_zebra_3dot14_real_4dot14_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_vector_of_chars_a_b_c_a_a_g_u_a_char_a_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_vector_of_chars_a_b_c_a_a_g_u_a_char_x_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_vector_of_strings_cat_dog_cat_zebra_parrot_cat_string_cat_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_vector_of_strings_cat_dog_cat_zebra_parrot_cat_string_kitten_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_vector_of_symbols_cat_dog_cat_zebra_parrot_cat_symbol_cat_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_vector_of_symbols_cat_dog_cat_zebra_parrot_cat_symbol_mouse_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_hash_map_of_chars_a_b_c_d_char_c_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_hash_map_of_chars_a_b_c_d_char_x_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_hash_map_of_chars_a_b_c_d_char_b_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_hash_map_of_strings_cat_dog_zebra_kitten_string_zebra_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_hash_map_of_strings_cat_dog_zebra_kitten_string_parrot_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_hash_map_of_integers_1_2_3_4_integer_2_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_hash_map_of_integers_1_2_3_4_integer_5_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_hash_map_of_reals_2dot1_2dot2_2dot3_2dot4_real_2dot2_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_hash_map_of_reals_2dot1_2dot2_2dot3_2dot4_real_2dot5_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_global_environment_symbol_find_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_global_environment_defined_symbol_xyz_test);
    RUN_TEST(octaspire_dern_vm_builtin_find_from_global_environment_symbol_notfound_test);

    RUN_TEST(octaspire_dern_vm_special_while_with_one_value_to_repeat_test);
    RUN_TEST(octaspire_dern_vm_special_while_with_two_values_to_repeat_test);
    RUN_TEST(octaspire_dern_vm_special_while_called_with_one_argument_failure_test);
    RUN_TEST(octaspire_dern_vm_special_while_called_with_integer_as_first_argument_failure_test);

    RUN_TEST(octaspire_dern_vm_builtin_hash_map_empty_test);
    RUN_TEST(octaspire_dern_vm_builtin_hash_map_one_element_symbol_one_1_test);
    RUN_TEST(octaspire_dern_vm_builtin_hash_map_one_element_1_symbol_one_test);
    RUN_TEST(octaspire_dern_vm_builtin_hash_map_two_elements_strings_dog_barks_and_sun_shines_test);
    RUN_TEST(octaspire_dern_vm_string_literal_with_embedded_characters_t_bar_newline_tab_test);
    RUN_TEST(octaspire_dern_vm_string_literal_with_embedded_characters_c_a_t__a_n_d__d_o_g_test);
    RUN_TEST(octaspire_dern_vm_string_literal_with_embedded_character_newline_failure_on_missing_end_delimiter_test);
    RUN_TEST(octaspire_dern_vm_string_literal_with_embedded_character_yy_failure_on_unknown_char_constant_test);
    RUN_TEST(octaspire_dern_vm_string_literal_with_embedded_character_in_hex_A9_test);
    RUN_TEST(octaspire_dern_vm_string_literal_with_embedded_character_in_hex_a9_test);
    RUN_TEST(octaspire_dern_vm_string_literal_with_embedded_character_a_test);
    RUN_TEST(octaspire_dern_vm_string_literal_with_embedded_character_in_hex_0x10000_test);
    RUN_TEST(octaspire_dern_vm_string_literal_with_embedded_character_in_hex_failure_on_too_many_hex_digits_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_with_empty_vector_of_strings_and_integer_10_and_string_cat_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_with_vector_of_strings_and_integer_1_and_string_cat_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_with_hash_map_and_same_key_inserted_multiple_times_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_with_hash_map_and_hash_map_with_elements_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_with_hash_map_and_empty_hash_map_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_with_vector_and_vector_with_elements_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_with_vector_and_empty_vector_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_with_string_and_index_and_character_test);
    RUN_TEST(octaspire_dern_vm_builtin_minus_equals_with_character_x_and_integer_2_test);
    RUN_TEST(octaspire_dern_vm_builtin_minus_equals_with_character_x_and_character_exclamation_mark_test);
    RUN_TEST(octaspire_dern_vm_builtin_minus_equals_with_real_3_dot_14_and_reals_1_dot_0_and_zero_dot_14_test);
    RUN_TEST(octaspire_dern_vm_builtin_minus_equals_with_integer_10_and_integers_1_and_2_test);
    RUN_TEST(octaspire_dern_vm_builtin_minus_equals_with_vector_1_1_2_2_2_3_and_values_1_and_2_test);
    RUN_TEST(octaspire_dern_vm_builtin_minus_equals_with_string_abcd_and_characters_a_and_b_test);
    RUN_TEST(octaspire_dern_vm_builtin_minus_equals_with_hash_map_1_a_2_b_3_c_and_value_1_test);
    RUN_TEST(octaspire_dern_vm_builtin_minus_equals_with_vector_1_2_3_3_and_ln_at_v_minus_1_test);
    RUN_TEST(octaspire_dern_vm_builtin_minus_equals_equals_with_vector_1_2_3_3_and_ln_at_v_minus_1_test);

    RUN_TEST(octaspire_dern_vm_builtin_pop_front_with_vector_1_2_3_test);
    RUN_TEST(octaspire_dern_vm_builtin_pop_front_with_string_abc_test);
    RUN_TEST(octaspire_dern_vm_builtin_pop_front_with_symbol_abc_test);
    RUN_TEST(octaspire_dern_vm_builtin_pop_front_with_list_1_2_3_test);
    // No pop front for queue
    RUN_TEST(octaspire_dern_vm_builtin_pop_front_with_integer_10_failure_test);

    RUN_TEST(octaspire_dern_vm_builtin_pop_back_with_vector_1_2_3_test);
    RUN_TEST(octaspire_dern_vm_builtin_pop_back_with_string_abc_test);
    RUN_TEST(octaspire_dern_vm_builtin_pop_back_with_symbol_abc_test);
    RUN_TEST(octaspire_dern_vm_builtin_pop_back_with_list_1_2_3_test);
    RUN_TEST(octaspire_dern_vm_builtin_pop_back_with_queue_1_2_3_test);
    RUN_TEST(octaspire_dern_vm_builtin_pop_back_with_integer_10_failure_test);

    RUN_TEST(octaspire_dern_vm_builtin_plus_equals_with_hash_map_and_hash_map_1_a_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_equals_with_hash_map_and_1_a_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_equals_with_hash_map_and_list_1_a_2_b_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_equals_with_vector_1_2_3_and_4_5_6_test);

    RUN_TEST(octaspire_dern_vm_builtin_plus_equals_with_string_abc_and_string_def_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_equals_with_string_abc_and_symbol_def_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_equals_with_string_abc_and_character_d_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_equals_with_string_abc_and_vector_of_strings_def_ghi_jkl_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_equals_with_string_abc_and_nil_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_equals_with_string_abc_and_booleans_true_and_false_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_equals_with_string_abc_and_integer_128_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_equals_with_string_abc_and_real_3dot14_test);

    RUN_TEST(octaspire_dern_vm_builtin_plus_equals_with_symbol_abc_and_string_def_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_equals_with_symbol_abc_and_symbol_def_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_equals_with_symbol_abc_and_character_d_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_equals_with_symbol_abc_and_vector_of_strings_def_ghi_jkl_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_equals_with_symbol_abc_and_nil_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_equals_with_symbol_abc_and_booleans_true_and_false_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_equals_with_symbol_abc_and_integer_128_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_equals_with_symbol_abc_and_real_3dot14_test);


    RUN_TEST(octaspire_dern_vm_builtin_plus_equals_with_bad_input_test);
    RUN_TEST(octaspire_dern_vm_run_user_factorial_function_test);

    RUN_TEST(octaspire_dern_vm_special_for_from_0_to_10_test);
    RUN_TEST(octaspire_dern_vm_special_for_from_0_to_10_with_step_2_test);
    RUN_TEST(octaspire_dern_vm_special_for_from_0_to_10_with_step_minus_2_failure_test);
    RUN_TEST(octaspire_dern_vm_special_for_from_10_to_0_test);
    RUN_TEST(octaspire_dern_vm_special_for_from_10_to_0_with_step_2_test);
    RUN_TEST(octaspire_dern_vm_special_for_from_10_to_0_with_step_minus_2_failure_test);
    RUN_TEST(octaspire_dern_vm_special_for_in_with_string_test);
    RUN_TEST(octaspire_dern_vm_special_for_in_with_string_step_2_test);
    RUN_TEST(octaspire_dern_vm_special_for_in_with_string_step_minus_2_failure_test);
    RUN_TEST(octaspire_dern_vm_special_for_in_with_vector_of_strings_test);
    RUN_TEST(octaspire_dern_vm_special_for_in_with_vector_of_strings_step_2_test);
    RUN_TEST(octaspire_dern_vm_special_for_in_with_vector_of_strings_step_minus_2_failure_test);
    RUN_TEST(octaspire_dern_vm_special_for_in_with_environment_test);
    RUN_TEST(octaspire_dern_vm_special_for_in_with_environment_step_2_test);
    RUN_TEST(octaspire_dern_vm_special_for_in_with_environment_step_minus_2_failure_test);
    RUN_TEST(octaspire_dern_vm_special_for_in_with_hash_map_test);
    RUN_TEST(octaspire_dern_vm_special_for_in_with_hash_map_step_2_test);
    RUN_TEST(octaspire_dern_vm_special_for_in_with_hash_map_step_minus_2_failure_test);
    RUN_TEST(octaspire_dern_vm_special_for_called_without_arguments_failure_test);
    RUN_TEST(octaspire_dern_vm_special_for_called_with_one_argument_failure_test);
    RUN_TEST(octaspire_dern_vm_special_for_second_argument_not_symbol_failure_test);
    RUN_TEST(octaspire_dern_vm_special_for_called_with_integer_as_first_argument_failure_test);


    RUN_TEST(octaspire_dern_vm_error_in_function_body_is_reported_test);
    RUN_TEST(octaspire_dern_vm_builtin_cp_at_sign_called_with_1_2_3_and_integers_0_and_1_and_2_and_3_test);
    RUN_TEST(octaspire_dern_vm_builtin_cp_at_sign_called_with_0_and_string_abc_test);
    RUN_TEST(octaspire_dern_vm_builtin_cp_at_sign_called_with_1_and_string_abc_test);
    RUN_TEST(octaspire_dern_vm_builtin_cp_at_sign_called_with_2_and_string_abc_test);
    RUN_TEST(octaspire_dern_vm_builtin_cp_at_sign_called_with_3_and_string_abc_failure_test);
    RUN_TEST(octaspire_dern_vm_builtin_ln_at_sign_called_with_0_and_vector_1_2_3_test);
    RUN_TEST(octaspire_dern_vm_builtin_ln_at_sign_called_with_1_and_vector_1_2_3_test);
    RUN_TEST(octaspire_dern_vm_builtin_ln_at_sign_called_with_2_and_vector_1_2_3_test);
    RUN_TEST(octaspire_dern_vm_builtin_ln_at_sign_called_with_3_and_vector_1_2_3_failure_test);
    RUN_TEST(octaspire_dern_vm_builtin_ln_at_sign_called_with_0_and_index_and_hash_map_1a_2b_3c_test);
    RUN_TEST(octaspire_dern_vm_builtin_ln_at_sign_called_with_1_and_index_and_hash_map_1a_2b_3c_test);
    RUN_TEST(octaspire_dern_vm_builtin_ln_at_sign_called_with_2_and_index_and_hash_map_1a_2b_3c_test);
    RUN_TEST(octaspire_dern_vm_builtin_ln_at_sign_called_with_3_and_index_and_hash_map_1a_2b_3c_failure_test);
    RUN_TEST(octaspire_dern_vm_builtin_ln_at_sign_called_with_2_and_hash_and_hash_map_1a_2b_3c_test);
    RUN_TEST(octaspire_dern_vm_changing_atom_doesnt_change_another_defined_from_it_test);
    RUN_TEST(octaspire_dern_vm_builtin_return_in_special_do_inside_function_test);
    RUN_TEST(octaspire_dern_vm_special_do_error_stops_evaluation_and_is_reported_test);
    RUN_TEST(octaspire_dern_vm_special_fn_in_env_howto_no_test);
    RUN_TEST(octaspire_dern_vm_special_fn_in_env_howto_ok_test);
    RUN_TEST(octaspire_dern_vm_builtin_return_inside_function_test);
    RUN_TEST(octaspire_dern_vm_builtin_return_in_special_for_with_numeric_range_inside_function_test);
    RUN_TEST(octaspire_dern_vm_builtin_return_in_special_for_with_collection_inside_function_test);
    RUN_TEST(octaspire_dern_vm_builtin_return_in_special_for_with_string_inside_function_test);
    RUN_TEST(octaspire_dern_vm_builtin_return_in_special_for_with_environment_inside_function_test);
    RUN_TEST(octaspire_dern_vm_builtin_return_in_special_for_with_hash_map_inside_function_test);
    RUN_TEST(octaspire_dern_vm_builtin_return_in_special_while_inside_function_test);
    RUN_TEST(octaspire_dern_vm_builtin_return_called_without_argument_test);
    RUN_TEST(octaspire_dern_vm_builtin_return_called_with_two_arguments_failure_test);
    RUN_TEST(octaspire_dern_vm_function_taking_one_regular_and_varargs_called_with_four_arguments_test);
    RUN_TEST(octaspire_dern_vm_function_taking_one_regular_and_varargs_called_with_one_argument_test);
    RUN_TEST(octaspire_dern_vm_function_taking_one_regular_and_varargs_called_with_zero_arguments_test);

    RUN_TEST(octaspire_dern_vm_newly_created_function_returned_from_another_function_and_used_test);
    RUN_TEST(octaspire_dern_vm_recursive_function_used_test);
    RUN_TEST(octaspire_dern_vm_mutually_recursive_functions_used_test);

    RUN_TEST(octaspire_dern_vm_special_and_called_without_arguments_test);
    RUN_TEST(octaspire_dern_vm_special_or_called_without_arguments_test);
    RUN_TEST(octaspire_dern_vm_builtin_not_called_without_arguments_failure_test);

    RUN_TEST(octaspire_dern_vm_special_and_called_with_one_argument_test);
    RUN_TEST(octaspire_dern_vm_special_or_called_with_one_argument_test);

    RUN_TEST(octaspire_dern_vm_special_and_called_with_one_false_argument_test);
    RUN_TEST(octaspire_dern_vm_special_or_called_with_one_false_argument_test);
    RUN_TEST(octaspire_dern_vm_builtin_not_called_with_one_false_argument_test);
    RUN_TEST(octaspire_dern_vm_builtin_not_called_with_one_true_argument_test);
    RUN_TEST(octaspire_dern_vm_builtin_not_called_with_one_integer_argument_failure_test);
    RUN_TEST(octaspire_dern_vm_builtin_not_called_with_two_boolean_arguments_failure_test);

    RUN_TEST(octaspire_dern_vm_special_and_called_with_three_arguments_test);
    RUN_TEST(octaspire_dern_vm_special_or_called_with_three_arguments_test);

    RUN_TEST(octaspire_dern_vm_special_and_called_with_three_arguments_with_false_as_second_test);
    RUN_TEST(octaspire_dern_vm_special_or_called_with_three_arguments_with_true_as_second_test);

    RUN_TEST(octaspire_dern_vm_special_greater_than_called_with_two_reals_test);
    RUN_TEST(octaspire_dern_vm_special_greater_than_called_with_two_integers_test);
    RUN_TEST(octaspire_dern_vm_special_greater_than_called_with_integer_10_and_real_9dot9_test);
    RUN_TEST(octaspire_dern_vm_special_greater_than_called_with_integer_9_and_real_9dot9_test);
    RUN_TEST(octaspire_dern_vm_special_greater_than_called_with_real_9dot9_and_integer_10_test);
    RUN_TEST(octaspire_dern_vm_special_greater_than_called_with_real_10dot1_and_integer_10_test);

    RUN_TEST(octaspire_dern_vm_special_greater_than_or_equal_called_with_two_reals_test);
    RUN_TEST(octaspire_dern_vm_special_greater_than_or_equal_called_with_two_integers_test);
    RUN_TEST(octaspire_dern_vm_special_greater_than_or_equal_called_with_integer_10_and_real_9dot9_test);
    RUN_TEST(octaspire_dern_vm_special_greater_than_or_equal_called_with_integer_9_and_real_9dot9_test);
    RUN_TEST(octaspire_dern_vm_special_greater_than_or_equal_called_with_real_9dot9_and_integer_10_test);
    RUN_TEST(octaspire_dern_vm_special_greater_than_or_equal_called_with_real_10dot1_and_integer_10_test);

    RUN_TEST(octaspire_dern_vm_special_less_than_called_with_two_reals_test);
    RUN_TEST(octaspire_dern_vm_special_less_than_called_with_two_integers_test);
    RUN_TEST(octaspire_dern_vm_special_less_than_called_with_integer_9_and_real_9dot9_test);
    RUN_TEST(octaspire_dern_vm_special_less_than_called_with_integer_10_and_real_9dot9_test);
    RUN_TEST(octaspire_dern_vm_special_less_than_called_with_real_10dot1_and_integer_10_test);
    RUN_TEST(octaspire_dern_vm_special_less_than_called_with_real_9dot9_and_integer_10_test);

    RUN_TEST(octaspire_dern_vm_special_less_than_or_equal_called_with_two_reals_test);
    RUN_TEST(octaspire_dern_vm_special_less_than_or_equal_called_with_two_integers_test);
    RUN_TEST(octaspire_dern_vm_special_less_than_or_equal_called_with_integer_9_and_real_9dot9_test);
    RUN_TEST(octaspire_dern_vm_special_less_than_or_equal_called_with_integer_10_and_real_9dot9_test);
    RUN_TEST(octaspire_dern_vm_special_less_than_or_equal_called_with_real_10dot1_and_integer_10_test);
    RUN_TEST(octaspire_dern_vm_special_less_than_or_equal_called_with_real_9dot9_and_integer_10_test);

    RUN_TEST(octaspire_dern_vm_builtin_equals_equals_called_with_two_equal_lists_of_integers_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_equals_called_with_two_unequal_lists_of_integers_test);

    RUN_TEST(octaspire_dern_vm_builtin_equals_equals_called_with_two_equal_vectors_of_integers_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_equals_called_with_two_equal_queues_of_integers_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_equals_called_with_two_equal_queues_of_vectors_of_integers_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_equals_called_with_two_unequal_queues_of_vectors_of_integers_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_equals_called_with_two_unequal_queues_of_integers_with_different_lengths_of_5_and_6_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_equals_called_with_two_unequal_queues_of_integers_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_equals_called_with_two_equal_vectors_of_vectors_of_integers_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_equals_called_with_two_equal_hash_maps_of_vectors_of_integers_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_equals_called_with_two_unequal_hash_maps_of_vectors_of_integers_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_equals_called_with_two_equal_environments_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_equals_called_with_two_unequal_environments_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_equals_called_with_two_equal_functions_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_equals_called_with_two_unequal_functions_formal_name_differs_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_equals_called_with_two_unequal_functions_name_differs_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_equals_called_with_two_unequal_functions_docstr_differs_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_equals_called_with_two_unequal_functions_howto_differs_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_equals_called_with_two_unequal_vectors_of_vectors_of_integers_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_equals_called_with_two_unequal_vectors_of_integers_with_different_lengths_5_and_6_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_equals_called_with_two_unequal_vectors_of_integers_with_different_elements_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_equals_called_with_two_reals_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_equals_called_with_two_integers_test);
    RUN_TEST(octaspire_dern_vm_buildin_equals_equals_called_with_integer_9_and_reals_9dot1_and_9dot0_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_equals__called_with_integer_10_and_real_9dot9_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_equals_called_with_real_10dot1_and_integer_10_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_equals_called_with_real_9dot9_and_integer_10_test);

    RUN_TEST(octaspire_dern_vm_builtin_exclamation_equals_called_with_two_reals_test);
    RUN_TEST(octaspire_dern_vm_builtin_exclamation_equals_called_with_two_integers_test);
    RUN_TEST(octaspire_dern_vm_buildin_exclamation_equals_called_with_integer_9_and_reals_9dot1_and_9dot0_test);
    RUN_TEST(octaspire_dern_vm_builtin_exclamation_equals_called_with_integer_10_and_real_9dot9_test);
    RUN_TEST(octaspire_dern_vm_builtin_exclamation_equals_called_with_real_10dot1_and_integer_10_test);
    RUN_TEST(octaspire_dern_vm_builtin_exclamation_equals_called_with_real_9dot9_and_integer_10_test);
    RUN_TEST(octaspire_dern_vm_error_during_user_function_call_test);
    RUN_TEST(octaspire_dern_vm_error_during_builtin_call_test);
    RUN_TEST(octaspire_dern_vm_error_during_special_call_test);
    RUN_TEST(octaspire_dern_vm_error_during_special_call_during_user_function_call_test);
    RUN_TEST(octaspire_dern_vm_special_eval_plus_1_2_test);
    RUN_TEST(octaspire_dern_vm_special_eval_plus_1_2_in_given_global_env_test);
    RUN_TEST(octaspire_dern_vm_special_eval_value_from_given_local_env_test);
    RUN_TEST(octaspire_dern_vm_special_eval_eval_eval_f_1_2_test);
    RUN_TEST(octaspire_dern_vm_special_eval_failure_on_integer_on_second_argument_test);
    RUN_TEST(octaspire_dern_vm_special_eval_failure_on_unbound_symbol_on_second_argument_test);
    RUN_TEST(octaspire_dern_vm_special_eval_called_with_three_arguments_failure_test);

    RUN_TEST(octaspire_dern_vm_create_new_value_copy_called_with_vector_value_of_int_values_test);

    RUN_TEST(octaspire_dern_vm_comment_alone_test);
    RUN_TEST(octaspire_dern_vm_comment_after_integer_1_test);
    RUN_TEST(octaspire_dern_vm_comment_before_integer_123_test);

    RUN_TEST(octaspire_dern_vm_multiline_comment_test);
    RUN_TEST(octaspire_dern_vm_multiline_comment_in_vector_1_2_test);
    RUN_TEST(octaspire_dern_vm_multiline_comment_before_integer_123_test);
    RUN_TEST(octaspire_dern_vm_multiline_comment_missing_chars_test);

    RUN_TEST(octaspire_dern_vm_io_file_open_success_test);
    RUN_TEST(octaspire_dern_vm_input_file_open_success_test);
    RUN_TEST(octaspire_dern_vm_output_file_open_success_test);

    RUN_TEST(octaspire_dern_vm_port_supports_input_question_mark_called_with_output_file_test);
    RUN_TEST(octaspire_dern_vm_port_supports_input_question_mark_called_with_input_file_test);
    RUN_TEST(octaspire_dern_vm_port_supports_input_question_mark_called_with_io_file_test);
    RUN_TEST(octaspire_dern_vm_port_supports_input_question_mark_called_with_integer_failure_test);
    RUN_TEST(octaspire_dern_vm_port_supports_input_question_mark_called_without_arguments_failure_test);

    RUN_TEST(octaspire_dern_vm_port_supports_output_question_mark_called_with_input_file_test);
    RUN_TEST(octaspire_dern_vm_port_supports_output_question_mark_called_with_output_file_test);
    RUN_TEST(octaspire_dern_vm_port_supports_output_question_mark_called_with_io_file_test);
    RUN_TEST(octaspire_dern_vm_port_supports_output_question_mark_called_with_integer_failure_test);
    RUN_TEST(octaspire_dern_vm_port_supports_output_question_mark_called_without_arguments_failure_test);

    RUN_TEST(octaspire_dern_vm_port_close_called_with_io_file_port_test);
    RUN_TEST(octaspire_dern_vm_port_dist_called_with_a_file_test);
    RUN_TEST(octaspire_dern_vm_port_seek_called_with_a_file_test);

    RUN_TEST(octaspire_dern_vm_input_file_open_with_file_system_access_allowed_failure_on_missing_file_test);
    RUN_TEST(octaspire_dern_vm_port_write_failure_on_input_file_test);
    RUN_TEST(octaspire_dern_vm_port_length_test);

    RUN_TEST(octaspire_dern_vm_integer_question_mark_test);
    RUN_TEST(octaspire_dern_vm_real_question_mark_test);
    RUN_TEST(octaspire_dern_vm_number_question_mark_test);
    RUN_TEST(octaspire_dern_vm_nil_question_mark_test);
    RUN_TEST(octaspire_dern_vm_boolean_question_mark_test);
    RUN_TEST(octaspire_dern_vm_character_question_mark_test);
    RUN_TEST(octaspire_dern_vm_string_question_mark_test);
    RUN_TEST(octaspire_dern_vm_symbol_question_mark_test);
    RUN_TEST(octaspire_dern_vm_vector_question_mark_test);
    RUN_TEST(octaspire_dern_vm_hash_map_question_mark_test);

    RUN_TEST(octaspire_dern_vm_queue_test);
    RUN_TEST(octaspire_dern_vm_queue_with_max_length_test);
    RUN_TEST(octaspire_dern_vm_list_test);

    RUN_TEST(octaspire_dern_vm_copy_test);

    RUN_TEST(octaspire_dern_vm_split_called_with_string_and_char_test);

    RUN_TEST(octaspire_dern_vm_cp_at_sign_with_vector_test);
    RUN_TEST(octaspire_dern_vm_cp_at_sign_with_string_test);
    RUN_TEST(octaspire_dern_vm_cp_at_sign_with_hash_map_of_size_one_test);
    RUN_TEST(octaspire_dern_vm_cp_at_sign_with_hash_map_of_size_three_test);

    RUN_TEST(octaspire_dern_vm_ln_at_sign_with_vector_test);
    RUN_TEST(octaspire_dern_vm_ln_at_sign_with_string_test);
    RUN_TEST(octaspire_dern_vm_ln_at_sign_with_hash_map_of_size_one_test);
    RUN_TEST(octaspire_dern_vm_ln_at_sign_with_hash_map_of_size_three_test);

    RUN_TEST(octaspire_dern_vm_host_command_line_arguments_test);
    RUN_TEST(octaspire_dern_vm_host_environment_variables_test);

    RUN_TEST(octaspire_dern_vm_create_user_data_test);
    RUN_TEST(octaspire_dern_vm_copy_user_data_test);

    RUN_TEST(octaspire_dern_vm_require_a_source_library_test);

    RUN_TEST(octaspire_dern_vm_special_howto_1_2_3_test);
    RUN_TEST(octaspire_dern_vm_special_howto_strings_a_b_ab_test);
    RUN_TEST(octaspire_dern_vm_special_howto_chars_a_b_string_ab_test);
    RUN_TEST(octaspire_dern_vm_special_howto_symbols_a_b_ab_test);
    RUN_TEST(octaspire_dern_vm_special_howto_hash_map_1_a_and_2_b_and_hash_map_1_a_2_b_test);

    RUN_TEST(octaspire_dern_vm_compare_vectors_of_integers_test);
    RUN_TEST(octaspire_dern_vm_compare_vectors_of_vectors_of_integers_test);
    RUN_TEST(octaspire_dern_vm_compare_vectors_of_vectors_of_reals_test);
    RUN_TEST(octaspire_dern_vm_compare_vectors_of_vectors_of_integers_and_reals_test);
    RUN_TEST(octaspire_dern_vm_compare_vectors_of_vectors_of_strings_test);
    RUN_TEST(octaspire_dern_vm_compare_vectors_of_vectors_of_chars_test);
    RUN_TEST(octaspire_dern_vm_compare_vectors_of_vectors_of_misc_types_test);
    RUN_TEST(octaspire_dern_vm_compare_functions_test);
    RUN_TEST(octaspire_dern_vm_compare_hash_maps_test);
    RUN_TEST(octaspire_dern_vm_compare_hash_maps_of_vectors_of_integers_test);
    RUN_TEST(octaspire_dern_vm_compare_hash_maps_of_lists_of_integers_test);
    RUN_TEST(octaspire_dern_vm_compare_hash_maps_of_queues_of_integers_test);
    RUN_TEST(octaspire_dern_vm_compare_environments_test);

    RUN_TEST(octaspire_dern_vm_eval_empty_vector_test);
    RUN_TEST(octaspire_dern_vm_eval_vector_containing_empty_vector_test);
    RUN_TEST(octaspire_dern_vm_eval_right_parenthesis_test);

    RUN_TEST(octaspire_dern_vm_eval_unbalanced_parenthesis_1_test);

    octaspire_stdio_release(octaspireDernVmTestStdio);
    octaspireDernVmTestStdio = 0;

    octaspire_allocator_release(octaspireDernVmTestAllocator);
    octaspireDernVmTestAllocator = 0;
}

