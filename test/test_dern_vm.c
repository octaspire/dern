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

static octaspire_memory_allocator_t *allocator = 0;
static octaspire_stdio_t            *stdio     = 0;

TEST octaspire_dern_vm_new_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    ASSERT(vm->stack);
    ASSERT_EQ(allocator, vm->allocator);
    ASSERT(vm->all);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_create_new_value_boolean_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

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
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

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
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

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
    ASSERT(octaspire_dern_value_get_hash(pushedValue));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_vm_parse_and_eval_true_amid_whitespace_test(void)
{
    octaspire_input_t *input = octaspire_input_new_from_c_string("  \t \n  \t true  ", allocator);

    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_environment_t *wrappedEnv = octaspire_dern_environment_new(0, vm, allocator);

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
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

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
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

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
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

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
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

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
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

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
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

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

TEST octaspire_dern_vm_special_define_integer_value_test(void)
{
    octaspire_input_t *input = octaspire_input_new_from_c_string("(define x [test] 10)", allocator);

    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

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
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define (env-global) x [test] 10)");

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
    ASSERT_STR_EQ("test", octaspire_container_utf8_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_define_my_inc_function_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define my-inc [my own inc-function] '(value [the value to increase]) (fn (value) (++ value)))");

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
        octaspire_container_utf8_string_get_c_string(evaluatedValue->value.string));

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(my-inc 111)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(112,                              evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_special_define_factorial_function_with_integers_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define factorial [factorial function] '(n [calculate n!]) (fn (n) (if (== n 0) 1 (* n (factorial (- n 1))))))");

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
        octaspire_container_utf8_string_get_c_string(evaluatedValue->value.string));

    int32_t const expected[] = {1, 1, 2, 6, 24, 120, 720, 5040};

    char const * const input[] =
    {
        "(factorial 0)",
        "(factorial 1)",
        "(factorial 2)",
        "(factorial 3)",
        "(factorial 4)",
        "(factorial 5)",
        "(factorial 6)",
        "(factorial 7)",
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
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define factorial [factorial function] '(n [calculate n!]) (fn (n) (if (<= n 0.0) 1.0 (* n (factorial (- n 1))))))");

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
        octaspire_container_utf8_string_get_c_string(evaluatedValue->value.string));

    double const expected[] = {1, 1, 2, 6, 24, 120, 720, 5040};

    char const * const input[] =
    {
        "(factorial 0.0)",
        "(factorial 1.0)",
        "(factorial 2.0)",
        "(factorial 3.0)",
        "(factorial 4.0)",
        "(factorial 5.0)",
        "(factorial 6.0)",
        "(factorial 7.0)",
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

TEST octaspire_dern_vm_builtin_plus_plus_integer_value_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(vm, "(define x [test] 10)");

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
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define x [x-coordinate] 10)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(vm, "(doc x)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "x-coordinate",
        octaspire_container_utf8_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_read_and_eval_path_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    // TODO what about the path separator? Cross platform?
    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(read-and-eval-path [" OCTASPIRE_DERN_CONFIG_TEST_RES_PATH "test.dern])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(200,                              evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_read_and_eval_string_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(read-and-eval-string [(++ 10)])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(11,                               evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_1_2_3_4_minus_2_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+ 1 2 3 4 -2)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(8,                                evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_1_2_3_4_minus_2_2dot5_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+ 1 2 3 4 -2 2.5)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_REAL, evaluatedValue->typeTag);
    ASSERT_EQ(10.5,                          evaluatedValue->value.real);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_string_cat_dog_and_string_space_and_string_zebra_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+ [cat, dog] [ ] [and zebra])");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "cat, dog and zebra",
        octaspire_container_utf8_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_string_ca_and_character_t_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+ [ca] |t|)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "cat",
        octaspire_container_utf8_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_empty_string_and_characters_k_i_t_t_e_n_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+ [] |k| |i| |t| |t| |e| |n|)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "kitten",
        octaspire_container_utf8_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_minus_string_cat_dog_cat_and_string_cat_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(- [cat dog cat] [cat])");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ(" dog ", octaspire_container_utf8_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_minus_string_abcabcabc_and_character_a_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(- [abcabcabc] |a|)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ("bcbcbc", octaspire_container_utf8_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_minus_string_abcabcabc_and_character_a_and_character_b_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(- [abcabcabc] |a| |b|)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ("ccc", octaspire_container_utf8_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_minus_string_abcabcabc_and_character_a_and_character_b_and_caharacter_c_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(- [abcabcabc] |a| |b| |c|)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ("", octaspire_container_utf8_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_minus_1_2_3_4_minus_2_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(- 1 2 3 4 -2)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(-6,                               evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_minus_1_2_3_4_minus_2_2dot5_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(- 1 2 3 4 -2 2.5)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_REAL, evaluatedValue->typeTag);
    ASSERT_EQ(-8.5,                          evaluatedValue->value.real);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_string_abcdeaaba_character_a_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find [abcdeaaba] |a|)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(4, octaspire_container_vector_get_length(evaluatedValue->value.vector));

    int32_t const expected[] = {0, 5, 6, 8};

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        octaspire_dern_value_t const * const value =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                evaluatedValue,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                i);

        ASSERT(value);

        ASSERT_EQ(expected[i], value->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_string_abcdeaaba_character_y_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find [abcdeaaba] |y|)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_container_vector_get_length(evaluatedValue->value.vector));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_string_cat_dog_cat_zebra_cat_string_cat_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find [cat dog cat zebra cat] [cat])");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(3, octaspire_container_vector_get_length(evaluatedValue->value.vector));

    int32_t const expected[] = {0, 8, 18};

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        octaspire_dern_value_t const * const value =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                evaluatedValue,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                i);

        ASSERT(value);

        ASSERT_EQ(expected[i], value->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_string_cat_dog_cat_zebra_cat_string_parrot_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find [cat dog cat zebra cat] [parrot])");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_container_vector_get_length(evaluatedValue->value.vector));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_string_cat_dog_cat_zebra_cat_quoted_symbol_cat_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find [cat dog cat zebra cat] 'cat)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(3, octaspire_container_vector_get_length(evaluatedValue->value.vector));

    int32_t const expected[] = {0, 8, 18};

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        octaspire_dern_value_t const * const value =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                evaluatedValue,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                i);

        ASSERT(value);

        ASSERT_EQ(expected[i], value->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_string_cat_dog_cat_zebra_cat_quoted_symbol_parrot_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find [cat dog cat zebra cat] 'parrot)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_container_vector_get_length(evaluatedValue->value.vector));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_string_123_124_abc_123_99_123_integer_123_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find [123 124 abc 123 99 123] 123)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(3, octaspire_container_vector_get_length(evaluatedValue->value.vector));

    int32_t const expected[] = {0, 12, 19};

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        octaspire_dern_value_t const * const value =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                evaluatedValue,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                i);

        ASSERT(value);

        ASSERT_EQ(expected[i], value->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_string_123_124_abc_123_99_123_integer_777_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find [123 124 abc 123 99 123] 777)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_container_vector_get_length(evaluatedValue->value.vector));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_string_3dot14_abc_3dot13_3dot14_3dot146_3dot14_real_3dot14_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find [3.14 abc 3.13 3.14 3.146 3.14] 3.14)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(4, octaspire_container_vector_get_length(evaluatedValue->value.vector));

    int32_t const expected[] = {0, 14, 19, 25};

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        octaspire_dern_value_t const * const value =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                evaluatedValue,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                i);

        ASSERT(value);

        ASSERT_EQ(expected[i], value->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_string_3dot14_abc_3dot13_3dot14_3dot146_3dot14_real_7dot11_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find [3.14 abc 3.13 3.14 3.146 3.14] 7.11)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_container_vector_get_length(evaluatedValue->value.vector));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_symbol_abc_def_aaxa_char_a_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find 'abc_def_aaxa |a|)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(4, octaspire_container_vector_get_length(evaluatedValue->value.vector));

    int32_t const expected[] = {0, 8, 9, 11};

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        octaspire_dern_value_t const * const value =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                evaluatedValue,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                i);

        ASSERT(value);

        ASSERT_EQ(expected[i], value->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_symbol_abc_def_aaxa_char_y_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find 'abc_def_aaxa |y|)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_container_vector_get_length(evaluatedValue->value.vector));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_symbol_cat_dog_cat_cat_zebra_cat_string_cat_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find 'cat_dog_cat_cat_zebra_cat [cat])");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(4, octaspire_container_vector_get_length(evaluatedValue->value.vector));

    int32_t const expected[] = {0, 8, 12, 22};

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        octaspire_dern_value_t const * const value =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                evaluatedValue,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                i);

        ASSERT(value);

        ASSERT_EQ(expected[i], value->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_symbol_cat_dog_cat_cat_zebra_cat_string_parrot_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find 'cat_dog_cat_cat_zebra_cat [parrot])");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_container_vector_get_length(evaluatedValue->value.vector));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_symbol_cat_dog_cat_cat_zebra_cat_symbol_cat_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find 'cat_dog_cat_cat_zebra_cat 'cat)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(4, octaspire_container_vector_get_length(evaluatedValue->value.vector));

    int32_t const expected[] = {0, 8, 12, 22};

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        octaspire_dern_value_t const * const value =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                evaluatedValue,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                i);

        ASSERT(value);

        ASSERT_EQ(expected[i], value->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_symbol_cat_dog_cat_cat_zebra_cat_symbol_parrot_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find 'cat_dog_cat_cat_zebra_cat 'parrot)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_container_vector_get_length(evaluatedValue->value.vector));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_symbol_cat_123_dog_cat_123_123_cat_zebra_123_integer_123_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find 'cat_123_dog_cat_123_123_cat_zebra_123 123)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(4, octaspire_container_vector_get_length(evaluatedValue->value.vector));

    int32_t const expected[] = {4, 16, 20, 34};

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        octaspire_dern_value_t const * const value =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                evaluatedValue,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                i);

        ASSERT(value);

        ASSERT_EQ(expected[i], value->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_symbol_cat_321_dog_cat_321_321_cat_zebra_321_integer_123_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find 'cat_321_dog_cat_321_321_cat_zebra_321 123)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_container_vector_get_length(evaluatedValue->value.vector));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_symbol_cat_3dot14_dog_cat_3dot14_3dot14_cat_zebra_3dot14_real_3dot14_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find 'cat_3.14_dog_cat_3.14_3.14_cat_zebra_3.14 3.14)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(4, octaspire_container_vector_get_length(evaluatedValue->value.vector));

    int32_t const expected[] = {4, 17, 22, 37};

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        octaspire_dern_value_t const * const value =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                evaluatedValue,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                i);

        ASSERT(value);

        ASSERT_EQ(expected[i], value->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_symbol_cat_3dot14_dog_cat_3dot14_3dot14_cat_zebra_3dot14_real_4dot14_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find 'cat_3.14_dog_cat_3.14_3.14_cat_zebra_3.14 4.14)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_container_vector_get_length(evaluatedValue->value.vector));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_vector_of_chars_a_b_c_a_a_g_u_a_char_a_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find '(|a| |b| |c| |a| |a| |g| |u| |a|) |a|)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(4, octaspire_container_vector_get_length(evaluatedValue->value.vector));

    int32_t const expected[] = {0, 3, 4, 7};

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        octaspire_dern_value_t const * const value =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                evaluatedValue,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                i);

        ASSERT(value);

        ASSERT_EQ(expected[i], value->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_vector_of_chars_a_b_c_a_a_g_u_a_char_x_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find '(|a| |b| |c| |a| |a| |g| |u| |a|) |x|)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_container_vector_get_length(evaluatedValue->value.vector));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_vector_of_strings_cat_dog_cat_zebra_parrot_cat_string_cat_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find '([cat] [dog] [cat] [zebra] [parrot] [cat]) [cat])");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(3, octaspire_container_vector_get_length(evaluatedValue->value.vector));

    int32_t const expected[] = {0, 2, 5};

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        octaspire_dern_value_t const * const value =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                evaluatedValue,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                i);

        ASSERT(value);

        ASSERT_EQ(expected[i], value->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_vector_of_strings_cat_dog_cat_zebra_parrot_cat_string_kitten_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find '([cat] [dog] [cat] [zebra] [parrot] [cat]) [kitten])");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_container_vector_get_length(evaluatedValue->value.vector));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_vector_of_symbols_cat_dog_cat_zebra_parrot_cat_symbol_cat_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find '(cat dog cat zebra parrot cat) 'cat)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(3, octaspire_container_vector_get_length(evaluatedValue->value.vector));

    int32_t const expected[] = {0, 2, 5};

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        octaspire_dern_value_t const * const value =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                evaluatedValue,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                i);

        ASSERT(value);

        ASSERT_EQ(expected[i], value->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_vector_of_symbols_cat_dog_cat_zebra_parrot_cat_symbol_mouse_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find '(cat dog cat zebra parrot cat) 'mouse)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(0, octaspire_container_vector_get_length(evaluatedValue->value.vector));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_hash_map_of_chars_a_b_c_d_char_c_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find (hash-map |a| 1 |b| 2 |c| 3 |d| 4) |c|)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(3,                                evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_hash_map_of_chars_a_b_c_d_char_x_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find (hash-map |a| 1 |b| 2 |c| 3 |d| 4) |x|)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_NIL, evaluatedValue->typeTag);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_hash_map_of_chars_a_b_c_d_char_b_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find (hash-map |a| true |b| false |c| nil |d| 4) |b|)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(false,                            evaluatedValue->value.boolean);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_hash_map_of_strings_cat_dog_zebra_kitten_string_zebra_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find (hash-map [cat] |a| [dog] |b| [zebra] |c| [kitten] |d|) [zebra])");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, evaluatedValue->typeTag);
    ASSERT_STR_EQ("c", octaspire_container_utf8_string_get_c_string(evaluatedValue->value.character));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_hash_map_of_strings_cat_dog_zebra_kitten_string_parrot_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

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
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find (hash-map 1 [cat] 2 [dog] 3 [zebra] 4 [kitten]) 2)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ("dog", octaspire_container_utf8_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_hash_map_of_integers_1_2_3_4_integer_5_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find (hash-map 1 [cat] 2 [dog] 3 [zebra] 4 [kitten]) 5)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_NIL, evaluatedValue->typeTag);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_hash_map_of_reals_2dot1_2dot2_2dot3_2dot4_real_2dot2_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find (hash-map 2.1 1   2.2 2   2.3 3   2.4 4) 2.2)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(2,                                evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_hash_map_of_reals_2dot1_2dot2_2dot3_2dot4_real_2dot5_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(find (hash-map 2.1 1   2.2 2   2.3 3   2.4 4) 2.5)");

    ASSERT(evaluatedValue);

    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_NIL, evaluatedValue->typeTag);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_find_from_global_environment_symbol_find_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

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
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define xyz [xyz] '(1 2 3))");

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
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

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
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define i [i] 0)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(while (<= i 100) (++ i))");

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
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define i [i] 0)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define j [j] 1000)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(while (<= i 100) (++ i) (-- j))");

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

TEST octaspire_dern_vm_builtin_hash_map_empty_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

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
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(hash-map 'one 1)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_HASH_MAP, evaluatedValue->typeTag);
    ASSERT_EQ(1, octaspire_dern_value_as_hash_map_get_number_of_elements(evaluatedValue));

    octaspire_container_hash_map_element_t *element =
        octaspire_dern_value_as_hash_map_get_at_index(evaluatedValue, 0);

    ASSERT(element);

    evaluatedValue = octaspire_container_hash_map_element_get_key(element);
    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_SYMBOL, evaluatedValue->typeTag);
    ASSERT_STR_EQ("one", octaspire_container_utf8_string_get_c_string(evaluatedValue->value.symbol));

    evaluatedValue = octaspire_container_hash_map_element_get_value(element);
    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(1, evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_hash_map_one_element_1_symbol_one_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(hash-map 1 'one)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_HASH_MAP, evaluatedValue->typeTag);
    ASSERT_EQ(1, octaspire_dern_value_as_hash_map_get_number_of_elements(evaluatedValue));

    octaspire_container_hash_map_element_t *element =
        octaspire_dern_value_as_hash_map_get_at_index(evaluatedValue, 0);

    ASSERT(element);

    evaluatedValue = octaspire_container_hash_map_element_get_key(element);
    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(1, evaluatedValue->value.integer);

    evaluatedValue = octaspire_container_hash_map_element_get_value(element);
    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_SYMBOL, evaluatedValue->typeTag);
    ASSERT_STR_EQ("one", octaspire_container_utf8_string_get_c_string(evaluatedValue->value.symbol));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_hash_map_two_elements_strings_dog_barks_and_sun_shines_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

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

        octaspire_container_hash_map_element_t *element = octaspire_dern_value_as_hash_map_get(
            evaluatedValue,
            octaspire_dern_value_get_hash(keyValue),
            keyValue);

        ASSERT(element);

        octaspire_dern_value_t *valueValue = octaspire_container_hash_map_element_get_value(element);
        ASSERT(valueValue);
        ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, valueValue->typeTag);
        ASSERT_STR_EQ(expected[i], octaspire_container_utf8_string_get_c_string(valueValue->value.string));
    }

    ASSERT(octaspire_dern_vm_pop_value(vm, evaluatedValue));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_string_literal_with_embedded_characters_t_bar_newline_tab_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "[ca|t||bar|dog and parrot |newline||tab| 123]");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING,   evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "cat|dog and parrot \n\t 123",
        octaspire_container_utf8_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_string_literal_with_embedded_characters_c_a_t__a_n_d__d_o_g_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "[|c||a||t|| ||a||n||d|| ||d||o||g|]");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING,   evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "cat and dog",
        octaspire_container_utf8_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_string_literal_with_embedded_character_newline_failure_on_missing_end_delimiter_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "[|newline]");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR,   evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Problem with a character embedded in string: (character is not complete)",
        octaspire_container_utf8_string_get_c_string(evaluatedValue->value.error));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_string_literal_with_embedded_character_yy_failure_on_unknown_char_constant_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "[|yy|]");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR,   evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Problem with a character embedded in string: (Unknown character constant |yy|)",
        octaspire_container_utf8_string_get_c_string(evaluatedValue->value.error));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_string_literal_with_embedded_character_in_hex_A9_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "[|A9|]");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING,   evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "©",
        octaspire_container_utf8_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_string_literal_with_embedded_character_in_hex_a9_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "[|a9|]");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING,   evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "©",
        octaspire_container_utf8_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_string_literal_with_embedded_character_a_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "[|a|]");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING,   evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "a",
        octaspire_container_utf8_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_string_literal_with_embedded_character_in_hex_0x10000_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "[|10000|]");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING,   evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "𐀀",
        octaspire_container_utf8_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_string_literal_with_embedded_character_in_hex_failure_on_too_many_hex_digits_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "[|123456789|]");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR,   evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Problem with a character embedded in string: (Number of hex digits (9) in character definition may not be larger than eight)",
        octaspire_container_utf8_string_get_c_string(evaluatedValue->value.error));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_with_empty_vector_of_strings_and_integer_10_and_string_cat_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v [v] '())");

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
            "(= v 10 [cat])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(11,                              octaspire_dern_value_as_vector_get_length(evaluatedValue));

    for (size_t i = 0; i < octaspire_dern_value_as_vector_get_length(evaluatedValue); ++i)
    {
        octaspire_dern_value_t const * const element =
            octaspire_dern_value_as_vector_get_element_at_const(
                evaluatedValue,
                i);

        ASSERT(element);

        if (octaspire_dern_value_as_vector_is_valid_index(evaluatedValue, i + 1))
        {
            ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_NIL, element->typeTag);
        }
        else
        {
            ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, element->typeTag);

            ASSERT_STR_EQ(
                "cat",
                octaspire_container_utf8_string_get_c_string(element->value.string));
        }
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_with_vector_of_strings_and_integer_1_and_string_cat_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v [v] '([first] [second] [third]))");

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
            "(= v 1 [cat])");

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

TEST octaspire_dern_vm_builtin_equals_with_hash_map_and_hash_map_with_elements_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define target [target] (hash-map))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define source [source] (hash-map 1 |a| 2 |b|))");

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

    octaspire_container_hash_map_t * const hashMap = evaluatedValue->value.hashMap;

    ASSERT_EQ(2, octaspire_container_hash_map_get_number_of_elements(hashMap));

    octaspire_container_hash_map_element_t *element =
        octaspire_container_hash_map_get_at_index(hashMap, 0);

    ASSERT(element);

    octaspire_dern_value_t *key = octaspire_container_hash_map_element_get_key(element);
    ASSERT(key);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, key->typeTag);
    ASSERT_EQ(1,                                key->value.integer);

    octaspire_dern_value_t *value = octaspire_container_hash_map_element_get_value(element);
    ASSERT(value);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, value->typeTag);
    ASSERT_STR_EQ("a", octaspire_container_utf8_string_get_c_string(value->value.character));

    element =
        octaspire_container_hash_map_get_at_index(hashMap, 1);

    ASSERT(element);

    key = octaspire_container_hash_map_element_get_key(element);
    ASSERT(key);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, key->typeTag);
    ASSERT_EQ(2,                                key->value.integer);

    value = octaspire_container_hash_map_element_get_value(element);
    ASSERT(value);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, value->typeTag);
    ASSERT_STR_EQ("b", octaspire_container_utf8_string_get_c_string(value->value.character));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_with_hash_map_and_empty_hash_map_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define target [target] (hash-map 1 |a|))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define source [source] (hash-map))");

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

    octaspire_container_hash_map_t * const hashMap = evaluatedValue->value.hashMap;

    ASSERT_EQ(0, octaspire_container_hash_map_get_number_of_elements(hashMap));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_with_vector_and_vector_with_elements_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define target [target] '())");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define source [source] '(1 |a|))");

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
    ASSERT_STR_EQ("a", octaspire_container_utf8_string_get_c_string(value->value.character));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_equals_with_vector_and_empty_vector_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define target [target] '(1 2 3))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define source [source] '())");

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

TEST octaspire_dern_vm_builtin_minus_equals_with_character_x_and_integer_2_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define c [c] |x|)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(-= c 2)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, evaluatedValue->typeTag);
    ASSERT_STR_EQ("v", octaspire_container_utf8_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_minus_equals_with_character_x_and_character_exclamation_mark_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define c [c] |x|)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(-= c |!|)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, evaluatedValue->typeTag);
    ASSERT_STR_EQ("W", octaspire_container_utf8_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_minus_equals_with_real_3_dot_14_and_reals_1_dot_0_and_zero_dot_14_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define r [r] 3.14)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(-= r 1.0 0.14)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_REAL, evaluatedValue->typeTag);
    ASSERT_IN_RANGE(2.0, evaluatedValue->value.real, 0.000001);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_minus_equals_with_integer_10_and_integers_1_and_2_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define i [i] 10)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(-= i 1 2)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(7, evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_minus_equals_with_vector_1_1_2_2_2_3_and_values_1_and_2_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v [v] '(1 1 2 2 2 3))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(-= v 1 2)");

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
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s [s] [abcd])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(-= s |a| |b|)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);
    ASSERT_STR_EQ("cd", octaspire_container_utf8_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_minus_equals_with_hash_map_1_a_2_b_3_c_and_value_1_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define h [h] (hash-map 1 |a| 2 |b| 3 |c|))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(-= h 2 3)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_HASH_MAP, evaluatedValue->typeTag);
    ASSERT_EQ(1, octaspire_dern_value_as_hash_map_get_number_of_elements(evaluatedValue));

    octaspire_container_hash_map_element_t const * const element =
        octaspire_dern_value_as_hash_map_get_at_index(evaluatedValue, 0);

    ASSERT(element);

    octaspire_dern_value_t const * const key = octaspire_container_hash_map_element_get_key(element);
    ASSERT(key);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER,  key->typeTag);
    ASSERT_EQ(1,                                 key->value.integer);

    octaspire_dern_value_t const * const value = octaspire_container_hash_map_element_get_value(element);
    ASSERT(value);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, value->typeTag);
    ASSERT_STR_EQ("a", octaspire_container_utf8_string_get_c_string(value->value.character));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_equals_with_hash_map_and_hash_map_1_a_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define h [h] (hash-map))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+= h (hash-map 1 |a|))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_HASH_MAP, evaluatedValue->typeTag);
    ASSERT_EQ(1, octaspire_dern_value_as_hash_map_get_number_of_elements(evaluatedValue));

    octaspire_container_hash_map_element_t const * const element =
        octaspire_dern_value_as_hash_map_get_at_index(evaluatedValue, 0);

    ASSERT(element);

    octaspire_dern_value_t const * const key = octaspire_container_hash_map_element_get_key(element);
    ASSERT(key);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER,  key->typeTag);
    ASSERT_EQ(1,                                 key->value.integer);

    octaspire_dern_value_t const * const value = octaspire_container_hash_map_element_get_value(element);
    ASSERT(value);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, value->typeTag);
    ASSERT_STR_EQ("a", octaspire_container_utf8_string_get_c_string(value->value.character));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_equals_with_hash_map_and_1_a_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define h [h] (hash-map))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+= h 1 |a|)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_HASH_MAP, evaluatedValue->typeTag);
    ASSERT_EQ(1, octaspire_dern_value_as_hash_map_get_number_of_elements(evaluatedValue));

    octaspire_container_hash_map_element_t const * const element =
        octaspire_dern_value_as_hash_map_get_at_index(evaluatedValue, 0);

    ASSERT(element);

    octaspire_dern_value_t const * const key = octaspire_container_hash_map_element_get_key(element);
    ASSERT(key);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER,  key->typeTag);
    ASSERT_EQ(1,                                 key->value.integer);

    octaspire_dern_value_t const * const value = octaspire_container_hash_map_element_get_value(element);
    ASSERT(value);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, value->typeTag);
    ASSERT_STR_EQ("a", octaspire_container_utf8_string_get_c_string(value->value.character));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_equals_with_hash_map_and_list_1_a_2_b_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define h [h] (hash-map))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+= h '(1 |a| 2 |b|))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_HASH_MAP, evaluatedValue->typeTag);
    ASSERT_EQ(2, octaspire_dern_value_as_hash_map_get_number_of_elements(evaluatedValue));

    octaspire_container_hash_map_element_t const * element =
        octaspire_dern_value_as_hash_map_get_at_index(evaluatedValue, 0);

    ASSERT(element);

    octaspire_dern_value_t const * key = octaspire_container_hash_map_element_get_key(element);
    ASSERT(key);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER,  key->typeTag);
    ASSERT_EQ(1,                                 key->value.integer);

    octaspire_dern_value_t const * value = octaspire_container_hash_map_element_get_value(element);
    ASSERT(value);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, value->typeTag);
    ASSERT_STR_EQ("a", octaspire_container_utf8_string_get_c_string(value->value.character));

    element =
        octaspire_dern_value_as_hash_map_get_at_index(evaluatedValue, 1);

    ASSERT(element);

    key = octaspire_container_hash_map_element_get_key(element);
    ASSERT(key);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER,  key->typeTag);
    ASSERT_EQ(2,                                 key->value.integer);

    value = octaspire_container_hash_map_element_get_value(element);
    ASSERT(value);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, value->typeTag);
    ASSERT_STR_EQ("b", octaspire_container_utf8_string_get_c_string(value->value.character));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_equals_with_vector_1_2_3_and_4_5_6_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v [v] '(1 2 3))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN,  evaluatedValue->typeTag);
    ASSERT_EQ(true,                              evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+= v 4 5 6)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_VECTOR, evaluatedValue->typeTag);
    ASSERT_EQ(6,                               octaspire_dern_value_as_vector_get_length(evaluatedValue));

    for (size_t i = 0; i < octaspire_dern_value_as_vector_get_length(evaluatedValue); ++i)
    {
        octaspire_dern_value_t const * const element =
            octaspire_dern_value_as_vector_get_element_of_type_at_const(
                evaluatedValue,
                OCTASPIRE_DERN_VALUE_TAG_INTEGER,
                i);

        ASSERT   (element);
        ASSERT_EQ((int32_t)(i+1), element->value.integer);
    }

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_plus_equals_with_bad_input_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(+= (hash-map 1 ¦a|) (hash-map 2 |b|))");
    // bad input here      ^^^

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR,  evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Unknown character constant |) (hash-map 2 |",
        octaspire_container_utf8_string_get_c_string(evaluatedValue->value.error));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_run_user_factorial_function_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

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
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v [v] '())");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(for i from 0 to 10 (+= v i))");

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

TEST octaspire_dern_vm_special_for_from_10_to_0_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define v [v] '())");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(for i from 10 to 0 (+= v i))");

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

TEST octaspire_dern_vm_special_for_in_with_vector_of_strings_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define s [s] [])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define n [n] '([John] [Mike] [Ellie]))");

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
        octaspire_container_utf8_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_error_in_function_body_is_reported_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f [f] '(x [x]) (fn (x) (NoSuchFunction x)))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(f 1)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);
    // TODO type of 'error' or 'vector'?
    ASSERT_STR_EQ(
        "Cannot evaluate operator of type 'error' (<error>: Unbound symbol 'NoSuchFunction')",
        octaspire_container_utf8_string_get_c_string(evaluatedValue->value.error));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_nth_called_with_0_and_string_abc_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(nth 0 [abc])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "a",
        octaspire_container_utf8_string_get_c_string(evaluatedValue->value.character));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_nth_called_with_1_and_string_abc_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(nth 1 [abc])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "b",
        octaspire_container_utf8_string_get_c_string(evaluatedValue->value.character));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_nth_called_with_2_and_string_abc_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(nth 2 [abc])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "c",
        octaspire_container_utf8_string_get_c_string(evaluatedValue->value.character));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_nth_called_with_3_and_string_abc_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(nth 3 [abc])");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'nth' cannot index string of length 3 from index 3.",
        octaspire_container_utf8_string_get_c_string(evaluatedValue->value.error));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_nth_called_with_0_and_vector_1_2_3_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(nth 0 '(1 2 3))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(1,                                evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_nth_called_with_1_and_vector_1_2_3_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(nth 1 '(1 2 3))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(2,                                evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_nth_called_with_2_and_vector_1_2_3_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(nth 2 '(1 2 3))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_INTEGER, evaluatedValue->typeTag);
    ASSERT_EQ(3,                                evaluatedValue->value.integer);

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_nth_called_with_3_and_vector_1_2_3_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(nth 3 '(1 2 3))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'nth' cannot index vector of length 3 from index 3.",
        octaspire_container_utf8_string_get_c_string(evaluatedValue->value.error));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_nth_called_with_0_and_hash_map_1a_2b_3c_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(nth 0 (hash-map 1 |a| 2 |b| 3 |c|))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "a",
        octaspire_container_utf8_string_get_c_string(evaluatedValue->value.character));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_nth_called_with_1_and_hash_map_1a_2b_3c_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(nth 1 (hash-map 1 |a| 2 |b| 3 |c|))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "b",
        octaspire_container_utf8_string_get_c_string(evaluatedValue->value.character));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_nth_called_with_2_and_hash_map_1a_2b_3c_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(nth 2 (hash-map 1 |a| 2 |b| 3 |c|))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_CHARACTER, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "c",
        octaspire_container_utf8_string_get_c_string(evaluatedValue->value.character));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_builtin_nth_called_with_3_and_hash_map_1a_2b_3c_failure_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(nth 3 (hash-map 1 |a| 2 |b| 3 |c|))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_ERROR, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "Builtin 'nth' cannot index hash map of length 3 from index 3.",
        octaspire_container_utf8_string_get_c_string(evaluatedValue->value.error));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_changing_atom_doesnt_change_another_defined_from_it_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define original [original] 10)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define copied [copied] original)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(= original 999)");

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
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define counter [counter] 0)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f [f] '() (fn () (do (++ counter) (return 99) (++ counter))))");

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

TEST octaspire_dern_vm_builtin_return_inside_function_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define counter [counter] 0)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f [f] '() (fn () (++ counter) (return 99) (++ counter)))");

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
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define counter [counter] 0)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f [f] '() (fn () (for i from 0 to 10 (++ counter) (if (== i 5) (return 99)))))");

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
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define counter [counter] 0)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f [f] '() (fn () (for i in '(1 2 3 4 5 6) (+= counter i) (if (== i 4) (return 99)))))");

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
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define counter [counter] 0)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f [f] '() (fn () (for i in [abcdef] (+= counter 1) (if (== i |d|) (return 99)))))");

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
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define counter [counter] 0)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f [f] '() (fn () (define a [a] 1) (define b [b] 2) (define c [c] 3) (for i in (env-current) (+= counter 1) (if (== counter 1) (return 99)))))");

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
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define counter [counter] 0)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f [f] '() (fn () (for i in (hash-map |a| 1 |b| 2 |c| 3) (+= counter 1) (if (== counter 1) (return 99)))))");

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
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define counter [counter] 0)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f [f] '() (fn () (while (< counter 100) (++ counter) (return 99))))");

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

TEST octaspire_dern_vm_function_taking_one_regular_and_varargs_called_with_four_arguments_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f [f] '(x [x] y [rest of the args] ... [varargs]) (fn (x y ...)\n"
            "(define result [result] [])\n"
            "(+= result (string-format x)) (+= result (string-format y))\n"
            "result))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(f 1 2 3 4)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "1(2 3 4)",
        octaspire_container_utf8_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_function_taking_one_regular_and_varargs_called_with_one_argument_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f [f] '(x [x] y [rest of the args] ... [varargs]) (fn (x y ...)\n"
            "(define result [result] [])\n"
            "(+= result (string-format x)) (+= result (string-format y))\n"
            "result))");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_BOOLEAN, evaluatedValue->typeTag);
    ASSERT_EQ(true,                             evaluatedValue->value.boolean);

    evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(f 1)");

    ASSERT(evaluatedValue);
    ASSERT_EQ(OCTASPIRE_DERN_VALUE_TAG_STRING, evaluatedValue->typeTag);

    ASSERT_STR_EQ(
        "1()",
        octaspire_container_utf8_string_get_c_string(evaluatedValue->value.string));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}

TEST octaspire_dern_vm_function_taking_one_regular_and_varargs_called_with_zero_arguments_test(void)
{
    octaspire_dern_vm_t *vm = octaspire_dern_vm_new(allocator, stdio);

    octaspire_dern_value_t *evaluatedValue =
        octaspire_dern_vm_read_from_c_string_and_eval_in_global_environment(
            vm,
            "(define f [f] '(x [x] y [rest of the args] ... [varargs]) (fn (x y ...)\n"
            "(define result [result] [])\n"
            "(+= result (string-format x)) (+= result (string-format y))\n"
            "result))");

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
        octaspire_container_utf8_string_get_c_string(evaluatedValue->value.error));

    octaspire_dern_vm_release(vm);
    vm = 0;

    PASS();
}


GREATEST_SUITE(octaspire_dern_vm_suite)
{
    size_t numTimesRun = 0;

    allocator = octaspire_memory_allocator_new_create_region(
        OCTASPIRE_DERN_CONFIG_MEMORY_ALLOCATOR_REGION_MIN_BLOCK_SIZE_IN_OCTETS);

    stdio     = octaspire_stdio_new(allocator);

second_run:

    assert(allocator);
    assert(stdio);

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
    RUN_TEST(octaspire_dern_vm_special_define_integer_value_test);
    RUN_TEST(octaspire_dern_vm_special_define_integer_value_with_explicit_target_global_env_test);
    RUN_TEST(octaspire_dern_vm_special_define_my_inc_function_test);
    RUN_TEST(octaspire_dern_vm_special_define_factorial_function_with_integers_test);
    RUN_TEST(octaspire_dern_vm_special_define_factorial_function_with_reals_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_plus_integer_value_test);
    RUN_TEST(octaspire_dern_vm_builtin_doc_for_integer_value_test);
    RUN_TEST(octaspire_dern_vm_builtin_read_and_eval_path_test);
    RUN_TEST(octaspire_dern_vm_builtin_read_and_eval_string_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_1_2_3_4_minus_2_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_1_2_3_4_minus_2_2dot5_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_string_cat_dog_and_string_space_and_string_zebra_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_string_ca_and_character_t_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_empty_string_and_characters_k_i_t_t_e_n_test);
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
    RUN_TEST(octaspire_dern_vm_builtin_equals_with_hash_map_and_hash_map_with_elements_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_with_hash_map_and_empty_hash_map_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_with_vector_and_vector_with_elements_test);
    RUN_TEST(octaspire_dern_vm_builtin_equals_with_vector_and_empty_vector_test);
    RUN_TEST(octaspire_dern_vm_builtin_minus_equals_with_character_x_and_integer_2_test);
    RUN_TEST(octaspire_dern_vm_builtin_minus_equals_with_character_x_and_character_exclamation_mark_test);
    RUN_TEST(octaspire_dern_vm_builtin_minus_equals_with_real_3_dot_14_and_reals_1_dot_0_and_zero_dot_14_test);
    RUN_TEST(octaspire_dern_vm_builtin_minus_equals_with_integer_10_and_integers_1_and_2_test);
    RUN_TEST(octaspire_dern_vm_builtin_minus_equals_with_vector_1_1_2_2_2_3_and_values_1_and_2_test);
    RUN_TEST(octaspire_dern_vm_builtin_minus_equals_with_string_abcd_and_characters_a_and_b_test);
    RUN_TEST(octaspire_dern_vm_builtin_minus_equals_with_hash_map_1_a_2_b_3_c_and_value_1_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_equals_with_hash_map_and_hash_map_1_a_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_equals_with_hash_map_and_1_a_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_equals_with_hash_map_and_list_1_a_2_b_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_equals_with_vector_1_2_3_and_4_5_6_test);
    RUN_TEST(octaspire_dern_vm_builtin_plus_equals_with_bad_input_test);
    RUN_TEST(octaspire_dern_vm_run_user_factorial_function_test);
    RUN_TEST(octaspire_dern_vm_special_for_from_0_to_10_test);
    RUN_TEST(octaspire_dern_vm_special_for_from_10_to_0_test);
    RUN_TEST(octaspire_dern_vm_special_for_in_with_vector_of_strings_test);
    RUN_TEST(octaspire_dern_vm_error_in_function_body_is_reported_test);
    RUN_TEST(octaspire_dern_vm_builtin_nth_called_with_0_and_string_abc_test);
    RUN_TEST(octaspire_dern_vm_builtin_nth_called_with_1_and_string_abc_test);
    RUN_TEST(octaspire_dern_vm_builtin_nth_called_with_2_and_string_abc_test);
    RUN_TEST(octaspire_dern_vm_builtin_nth_called_with_3_and_string_abc_failure_test);
    RUN_TEST(octaspire_dern_vm_builtin_nth_called_with_0_and_vector_1_2_3_test);
    RUN_TEST(octaspire_dern_vm_builtin_nth_called_with_1_and_vector_1_2_3_test);
    RUN_TEST(octaspire_dern_vm_builtin_nth_called_with_2_and_vector_1_2_3_test);
    RUN_TEST(octaspire_dern_vm_builtin_nth_called_with_3_and_vector_1_2_3_failure_test);
    RUN_TEST(octaspire_dern_vm_builtin_nth_called_with_0_and_hash_map_1a_2b_3c_test);
    RUN_TEST(octaspire_dern_vm_builtin_nth_called_with_1_and_hash_map_1a_2b_3c_test);
    RUN_TEST(octaspire_dern_vm_builtin_nth_called_with_2_and_hash_map_1a_2b_3c_test);
    RUN_TEST(octaspire_dern_vm_builtin_nth_called_with_3_and_hash_map_1a_2b_3c_failure_test);
    RUN_TEST(octaspire_dern_vm_changing_atom_doesnt_change_another_defined_from_it_test);
    RUN_TEST(octaspire_dern_vm_builtin_return_in_special_do_inside_function_test);
    RUN_TEST(octaspire_dern_vm_builtin_return_inside_function_test);
    RUN_TEST(octaspire_dern_vm_builtin_return_in_special_for_with_numeric_range_inside_function_test);
    RUN_TEST(octaspire_dern_vm_builtin_return_in_special_for_with_collection_inside_function_test);
    RUN_TEST(octaspire_dern_vm_builtin_return_in_special_for_with_string_inside_function_test);
    RUN_TEST(octaspire_dern_vm_builtin_return_in_special_for_with_environment_inside_function_test);
    RUN_TEST(octaspire_dern_vm_builtin_return_in_special_for_with_hash_map_inside_function_test);
    RUN_TEST(octaspire_dern_vm_builtin_return_in_special_while_inside_function_test);
    RUN_TEST(octaspire_dern_vm_function_taking_one_regular_and_varargs_called_with_four_arguments_test);
    RUN_TEST(octaspire_dern_vm_function_taking_one_regular_and_varargs_called_with_one_argument_test);
    RUN_TEST(octaspire_dern_vm_function_taking_one_regular_and_varargs_called_with_zero_arguments_test);

    octaspire_stdio_release(stdio);
    stdio = 0;

    octaspire_memory_allocator_release(allocator);
    allocator = 0;

    ++numTimesRun;

    if (numTimesRun < 2)
    {
        // Second run without region allocator

        allocator = octaspire_memory_allocator_new(0);
        stdio     = octaspire_stdio_new(allocator);

        goto second_run;
    }
}
