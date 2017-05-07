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
#include "../src/octaspire_dern_lexer.c"
#include "external/greatest.h"
#include "octaspire/dern/octaspire_dern_lexer.h"
#include "octaspire/dern/octaspire_dern_config.h"

static octaspire_memory_allocator_t *allocator = 0;

TEST octaspire_dern_lexer_token_new_test(void)
{
    octaspire_dern_lexer_token_tag_t       const expectedTypeTag  = OCTASPIRE_DERN_LEXER_TOKEN_TAG_LPAREN;
    octaspire_dern_lexer_token_position_t  const expectedLine     = {987, 987};
    octaspire_dern_lexer_token_position_t  const expectedColumn   = {123, 123};
    octaspire_dern_lexer_token_position_t  const expectedUcsIndex = {300, 300};

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_token_new(
        expectedTypeTag,
        0,
        expectedLine,
        expectedColumn,
        expectedUcsIndex,
        allocator);

    ASSERT(token);

    ASSERT_EQ     (expectedTypeTag,  token->typeTag);

    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedLine,     token->line));
    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedColumn,   token->column));
    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedUcsIndex, token->ucsIndex));

    ASSERT_EQ     (allocator,        token->allocator);

    octaspire_dern_lexer_token_release(token);
    token = 0;

    PASS();
}

TEST octaspire_dern_lexer_token_new_allocation_failure_on_first_allocation_test(void)
{
    octaspire_dern_lexer_token_tag_t       const expectedTypeTag  = OCTASPIRE_DERN_LEXER_TOKEN_TAG_LPAREN;
    octaspire_dern_lexer_token_position_t  const expectedLine     = {987, 987};
    octaspire_dern_lexer_token_position_t  const expectedColumn   = {123, 123};
    octaspire_dern_lexer_token_position_t  const expectedUcsIndex = {300, 300};

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 1, 0);
    ASSERT_EQ(1, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_token_new(
        expectedTypeTag,
        0,
        expectedLine,
        expectedColumn,
        expectedUcsIndex,
        allocator);

    ASSERT_FALSE(token);

    ASSERT_EQ(0, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    PASS();
}

TEST octaspire_dern_lexer_token_new_allocation_failure_on_second_allocation_when_tag_is_string_test(void)
{
    octaspire_dern_lexer_token_tag_t       const expectedTypeTag  = OCTASPIRE_DERN_LEXER_TOKEN_TAG_STRING;
    octaspire_dern_lexer_token_position_t  const expectedLine     = {987, 987};
    octaspire_dern_lexer_token_position_t  const expectedColumn   = {123, 123};
    octaspire_dern_lexer_token_position_t  const expectedUcsIndex = {300, 300};

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 2, 0x01);
    ASSERT_EQ(2, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_token_new(
        expectedTypeTag,
        "some text",
        expectedLine,
        expectedColumn,
        expectedUcsIndex,
        allocator);

    ASSERT_FALSE(token);

    ASSERT_EQ(0, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    PASS();
}

TEST octaspire_dern_lexer_token_new_allocation_failure_on_second_allocation_when_tag_is_error_test(void)
{
    octaspire_dern_lexer_token_tag_t      const expectedTypeTag  = OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR;
    octaspire_dern_lexer_token_position_t const expectedLine     = {987, 987};
    octaspire_dern_lexer_token_position_t const expectedColumn   = {123, 123};
    octaspire_dern_lexer_token_position_t const expectedUcsIndex = {300, 300};

    octaspire_memory_allocator_set_number_and_type_of_future_allocations_to_be_rigged(allocator, 2, 0x01);
    ASSERT_EQ(2, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_token_new(
        expectedTypeTag,
        "some text",
        expectedLine,
        expectedColumn,
        expectedUcsIndex,
        allocator);

    ASSERT_FALSE(token);

    ASSERT_EQ(0, octaspire_memory_allocator_get_number_of_future_allocations_to_be_rigged(allocator));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    PASS();
}

TEST octaspire_dern_lexer_token_get_type_tag_test(void)
{
    octaspire_dern_lexer_token_tag_t      const expectedTypeTag  = OCTASPIRE_DERN_LEXER_TOKEN_TAG_LPAREN;

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_token_new(
        expectedTypeTag,
        0,
        octaspire_dern_lexer_token_position_init(987, 987),
        octaspire_dern_lexer_token_position_init(123, 123),
        octaspire_dern_lexer_token_position_init(300, 300),
        allocator);

    ASSERT(token);

    ASSERT_EQ(expectedTypeTag, octaspire_dern_lexer_token_get_type_tag(token));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    PASS();
}

TEST octaspire_dern_lexer_token_get_type_tag_as_c_string_test(void)
{
    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_LPAREN,
        0,
        octaspire_dern_lexer_token_position_init(987, 987),
        octaspire_dern_lexer_token_position_init(123, 123),
        octaspire_dern_lexer_token_position_init(300, 300),
        allocator);

    ASSERT(token);

    ASSERT_STR_EQ("OCTASPIRE_DERN_LEXER_TOKEN_TAG_LPAREN", octaspire_dern_lexer_token_get_type_tag_as_c_string(token));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    PASS();
}

TEST octaspire_dern_lexer_token_get_integer_value_test(void)
{
    int32_t const expected = 765;

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_INTEGER,
        &expected,
        octaspire_dern_lexer_token_position_init(987, 987),
        octaspire_dern_lexer_token_position_init(123, 123),
        octaspire_dern_lexer_token_position_init(300, 300),
        allocator);

    ASSERT(token);

    ASSERT_EQ(expected, octaspire_dern_lexer_token_get_integer_value(token));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    PASS();
}

TEST octaspire_dern_lexer_token_get_line_test(void)
{
    octaspire_dern_lexer_token_position_t  const expectedLine     = {32003, 32004};

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_LPAREN,
        0,
        expectedLine,
        octaspire_dern_lexer_token_position_init(123, 123),
        octaspire_dern_lexer_token_position_init(300, 300),
        allocator);

    ASSERT(token);

    ASSERT(octaspire_dern_lexer_token_position_is_equal(
        &expectedLine,
        octaspire_dern_lexer_token_get_position_line(token)));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    PASS();
}

TEST octaspire_dern_lexer_token_get_column_test(void)
{
    octaspire_dern_lexer_token_position_t  const expectedColumn     = {32003, 32004};

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_LPAREN,
        0,
        octaspire_dern_lexer_token_position_init(123, 123),
        expectedColumn,
        octaspire_dern_lexer_token_position_init(300, 300),
        allocator);

    ASSERT(token);

    ASSERT(octaspire_dern_lexer_token_position_is_equal(
        &expectedColumn,
        octaspire_dern_lexer_token_get_position_column(token)));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    PASS();
}

TEST octaspire_dern_lexer_token_to_string_with_lparen_token_test(void)
{
    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_LPAREN,
        0,
        octaspire_dern_lexer_token_position_init(987, 987),
        octaspire_dern_lexer_token_position_init(123, 123),
        octaspire_dern_lexer_token_position_init(300, 300),
        allocator);

    ASSERT(token);

    octaspire_container_utf8_string_t *str = octaspire_dern_lexer_token_to_string(token);

    ASSERT(str);

    char const * const expected =
        "token: line=987,987 column=123,123 ucsIndex=300,300 "
        "type=OCTASPIRE_DERN_LEXER_TOKEN_TAG_LPAREN value=left parenthesis";

    ASSERT_STR_EQ(expected, octaspire_container_utf8_string_get_c_string(str));

    octaspire_container_utf8_string_release(str);
    str = 0;

    octaspire_dern_lexer_token_release(token);
    token = 0;

    PASS();
}

TEST octaspire_dern_lexer_token_to_string_with_rparen_token_test(void)
{
    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_RPAREN,
        0,
        octaspire_dern_lexer_token_position_init(987, 987),
        octaspire_dern_lexer_token_position_init(123, 123),
        octaspire_dern_lexer_token_position_init(300, 300),
        allocator);

    ASSERT(token);

    octaspire_container_utf8_string_t *str = octaspire_dern_lexer_token_to_string(token);

    ASSERT(str);

    char const * const expected =
        "token: line=987,987 column=123,123 ucsIndex=300,300 "
        "type=OCTASPIRE_DERN_LEXER_TOKEN_TAG_RPAREN value=right parenthesis";

    ASSERT_STR_EQ(expected, octaspire_container_utf8_string_get_c_string(str));

    octaspire_container_utf8_string_release(str);
    str = 0;

    octaspire_dern_lexer_token_release(token);
    token = 0;

    PASS();
}

TEST octaspire_dern_lexer_token_to_string_with_integer_token_test(void)
{
    int32_t const expected = 16987;

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_INTEGER,
        &expected,
        octaspire_dern_lexer_token_position_init(987, 987),
        octaspire_dern_lexer_token_position_init(123, 123),
        octaspire_dern_lexer_token_position_init(300, 300),
        allocator);

    ASSERT(token);

    octaspire_container_utf8_string_t *str = octaspire_dern_lexer_token_to_string(token);

    ASSERT(str);

    ASSERT_STR_EQ(
        "token: line=987,987 column=123,123 ucsIndex=300,300 "
        "type=OCTASPIRE_DERN_LEXER_TOKEN_TAG_INTEGER value=integer 16987",
        octaspire_container_utf8_string_get_c_string(str));

    octaspire_container_utf8_string_release(str);
    str = 0;

    octaspire_dern_lexer_token_release(token);
    token = 0;

    PASS();
}

TEST octaspire_dern_lexer_token_to_string_with_error_token_test(void)
{
    char const * const value = "some error";
    octaspire_dern_lexer_token_t *expected = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
        value,
        octaspire_dern_lexer_token_position_init(987, 987),
        octaspire_dern_lexer_token_position_init(123, 123),
        octaspire_dern_lexer_token_position_init(300, 300),
        allocator);
    ASSERT(expected);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
        "some error",
        octaspire_dern_lexer_token_position_init(987, 987),
        octaspire_dern_lexer_token_position_init(123, 123),
        octaspire_dern_lexer_token_position_init(300, 300),
        allocator);

    ASSERT(token);

    ASSERT(octaspire_dern_lexer_token_is_equal(expected, token));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_dern_lexer_token_release(expected);
    expected = 0;

    PASS();
}

TEST octaspire_dern_lexer_token_to_string_with_unknow_token_test(void)
{
    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_token_new(
        128,
        0,
        octaspire_dern_lexer_token_position_init(987, 987),
        octaspire_dern_lexer_token_position_init(123, 123),
        octaspire_dern_lexer_token_position_init(300, 300),
        allocator);

    ASSERT_FALSE(token);

    octaspire_dern_lexer_token_release(token);
    token = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_left_parenthesis_no_whitespace_test(void)
{
    octaspire_dern_lexer_token_t *expected = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_LPAREN,
        0,
        octaspire_dern_lexer_token_position_init(1, 1),
        octaspire_dern_lexer_token_position_init(1, 1),
        octaspire_dern_lexer_token_position_init(0, 0),
        allocator);
    ASSERT(expected);

    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "(",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    ASSERT(octaspire_dern_lexer_token_is_equal(expected, token));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    octaspire_dern_lexer_token_release(expected);
    expected = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_right_parenthesis_no_whitespace_test(void)
{
    octaspire_dern_lexer_token_t *expected = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_RPAREN,
        0,
        octaspire_dern_lexer_token_position_init(1, 1),
        octaspire_dern_lexer_token_position_init(1, 1),
        octaspire_dern_lexer_token_position_init(0, 0),
        allocator);
    ASSERT(expected);

    octaspire_input_t *input = octaspire_input_new_from_c_string(
        ")",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    ASSERT(octaspire_dern_lexer_token_is_equal(expected, token));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    octaspire_dern_lexer_token_release(expected);
    expected = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_left_parenthesis_amid_whitespace_test(void)
{
    octaspire_dern_lexer_token_t *expected = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_LPAREN,
        0,
        octaspire_dern_lexer_token_position_init(2, 2),
        octaspire_dern_lexer_token_position_init(5, 5),
        octaspire_dern_lexer_token_position_init(6, 6),
        allocator);
    ASSERT(expected);

    octaspire_input_t *input = octaspire_input_new_from_c_string(
        " \n \t  (   \n \t  ",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    ASSERT(octaspire_dern_lexer_token_is_equal(expected, token));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    octaspire_dern_lexer_token_release(expected);
    expected = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_right_parenthesis_amid_whitespace_test(void)
{
    octaspire_dern_lexer_token_t *expected = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_RPAREN,
        0,
        octaspire_dern_lexer_token_position_init(2, 2),
        octaspire_dern_lexer_token_position_init(5, 5),
        octaspire_dern_lexer_token_position_init(6, 6),
        allocator);
    ASSERT(expected);

    octaspire_input_t *input = octaspire_input_new_from_c_string(
        " \n \t  )   \n \t  ",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    ASSERT(octaspire_dern_lexer_token_is_equal(expected, token));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    octaspire_dern_lexer_token_release(expected);
    expected = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_quote_no_whitespace_test(void)
{
    octaspire_dern_lexer_token_t *expected = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_QUOTE,
        0,
        octaspire_dern_lexer_token_position_init(1, 1),
        octaspire_dern_lexer_token_position_init(1, 1),
        octaspire_dern_lexer_token_position_init(0, 0),
        allocator);
    ASSERT(expected);

    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "'",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    ASSERT(octaspire_dern_lexer_token_is_equal(expected, token));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    octaspire_dern_lexer_token_release(expected);
    expected = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_quote_amid_whitespace_test(void)
{
    octaspire_dern_lexer_token_t *expected = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_QUOTE,
        0,
        octaspire_dern_lexer_token_position_init(2, 2),
        octaspire_dern_lexer_token_position_init(5, 5),
        octaspire_dern_lexer_token_position_init(6, 6),
        allocator);
    ASSERT(expected);

    octaspire_input_t *input = octaspire_input_new_from_c_string(
        " \n \t  '   \n \t  ",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    ASSERT(octaspire_dern_lexer_token_is_equal(expected, token));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    octaspire_dern_lexer_token_release(expected);
    expected = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_true_no_whitespace_test(void)
{
    octaspire_dern_lexer_token_t *expected = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_TRUE,
        0,
        octaspire_dern_lexer_token_position_init(1, 1),
        octaspire_dern_lexer_token_position_init(1, 4),
        octaspire_dern_lexer_token_position_init(0, 3),
        allocator);
    ASSERT(expected);

    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "true",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    ASSERT(octaspire_dern_lexer_token_is_equal(expected, token));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    octaspire_dern_lexer_token_release(expected);
    expected = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_true_amid_whitespace_test(void)
{
    octaspire_dern_lexer_token_t *expected = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_TRUE,
        0,
        octaspire_dern_lexer_token_position_init(2, 2),
        octaspire_dern_lexer_token_position_init(5, 8),
        octaspire_dern_lexer_token_position_init(6, 9),
        allocator);
    ASSERT(expected);

    octaspire_input_t *input = octaspire_input_new_from_c_string(
        " \n \t  true   \n \t  ",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    ASSERT(octaspire_dern_lexer_token_is_equal(expected, token));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    octaspire_dern_lexer_token_release(expected);
    expected = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_nil_no_whitespace_test(void)
{
    octaspire_dern_lexer_token_t *expected = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_NIL,
        0,
        octaspire_dern_lexer_token_position_init(1, 1),
        octaspire_dern_lexer_token_position_init(1, 3),
        octaspire_dern_lexer_token_position_init(0, 2),
        allocator);
    ASSERT(expected);

    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "nil",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    ASSERT(octaspire_dern_lexer_token_is_equal(expected, token));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    octaspire_dern_lexer_token_release(expected);
    expected = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_nil_amid_whitespace_test(void)
{
    octaspire_dern_lexer_token_t *expected = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_NIL,
        0,
        octaspire_dern_lexer_token_position_init(2, 2),
        octaspire_dern_lexer_token_position_init(5, 7),
        octaspire_dern_lexer_token_position_init(6, 8),
        allocator);
    ASSERT(expected);

    octaspire_input_t *input = octaspire_input_new_from_c_string(
        " \n \t  nil   \n \t  ",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    ASSERT(octaspire_dern_lexer_token_is_equal(expected, token));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    octaspire_dern_lexer_token_release(expected);
    expected = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_symbol_length_no_whitespace_test(void)
{
    octaspire_dern_lexer_token_t *expected = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_SYMBOL,
        "length",
        octaspire_dern_lexer_token_position_init(1, 1),
        octaspire_dern_lexer_token_position_init(1, 6),
        octaspire_dern_lexer_token_position_init(0, 5),
        allocator);
    ASSERT(expected);

    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "length",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    ASSERT(octaspire_dern_lexer_token_is_equal(expected, token));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    octaspire_dern_lexer_token_release(expected);
    expected = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_symbol_length_amid_whitespace_test(void)
{
    octaspire_dern_lexer_token_t *expected = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_SYMBOL,
        "length",
        octaspire_dern_lexer_token_position_init(2, 2),
        octaspire_dern_lexer_token_position_init(5, 10),
        octaspire_dern_lexer_token_position_init(6, 11),
        allocator);
    ASSERT(expected);

    octaspire_input_t *input = octaspire_input_new_from_c_string(
        " \n \t  length   \n \t  ",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    ASSERT(octaspire_dern_lexer_token_is_equal(expected, token));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    octaspire_dern_lexer_token_release(expected);
    expected = 0;

    PASS();
}


TEST octaspire_dern_lexer_pop_next_token_integer_12_test(void)
{
    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "12",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    octaspire_dern_lexer_token_position_t  const expectedLine     = {1, 1};
    octaspire_dern_lexer_token_position_t  const expectedColumn   = {1, 2};
    octaspire_dern_lexer_token_position_t  const expectedUcsIndex = {0, 1};

    ASSERT_EQ(allocator,                              token->allocator);

    ASSERT   (octaspire_dern_lexer_token_position_is_equal(&expectedLine,     token->line));
    ASSERT   (octaspire_dern_lexer_token_position_is_equal(&expectedColumn,   token->column));
    ASSERT   (octaspire_dern_lexer_token_position_is_equal(&expectedUcsIndex, token->ucsIndex));

    ASSERT_EQ(OCTASPIRE_DERN_LEXER_TOKEN_TAG_INTEGER, token->typeTag);
    ASSERT_EQ(12,                                     token->value.integer);

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_real_12_dot_3_test(void)
{
    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "12.3",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    octaspire_dern_lexer_token_position_t  const expectedLine     = {1, 1};
    octaspire_dern_lexer_token_position_t  const expectedColumn   = {1, 4};
    octaspire_dern_lexer_token_position_t  const expectedUcsIndex = {0, 3};

    ASSERT_EQ(allocator,                              token->allocator);

    ASSERT   (octaspire_dern_lexer_token_position_is_equal(&expectedLine,     token->line));
    ASSERT   (octaspire_dern_lexer_token_position_is_equal(&expectedColumn,   token->column));
    ASSERT   (octaspire_dern_lexer_token_position_is_equal(&expectedUcsIndex, token->ucsIndex));

    ASSERT_EQ(OCTASPIRE_DERN_LEXER_TOKEN_TAG_REAL, token->typeTag);
    ASSERT_EQ(12.3,                                token->value.real);

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_integer_759_after_whitespace_test(void)
{
    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "   \t   759", // 3 spaces + 1 tab + 3 spaces
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    octaspire_dern_lexer_token_position_t  const expectedLine     = {1, 1};
    octaspire_dern_lexer_token_position_t  const expectedColumn   = {8, 10};
    octaspire_dern_lexer_token_position_t  const expectedUcsIndex = {7, 9};

    ASSERT_EQ(allocator,                              token->allocator);

    ASSERT   (octaspire_dern_lexer_token_position_is_equal(&expectedLine,     token->line));
    ASSERT   (octaspire_dern_lexer_token_position_is_equal(&expectedColumn,   token->column));
    ASSERT   (octaspire_dern_lexer_token_position_is_equal(&expectedUcsIndex, token->ucsIndex));

    ASSERT_EQ(OCTASPIRE_DERN_LEXER_TOKEN_TAG_INTEGER, token->typeTag);
    ASSERT_EQ(759,                                    token->value.integer);

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_real_759_dot_2_after_whitespace_test(void)
{
    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "   \t   759.2", // 3 spaces + 1 tab + 3 spaces
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    octaspire_dern_lexer_token_position_t  const expectedLine     = {1, 1};
    octaspire_dern_lexer_token_position_t  const expectedColumn   = {8, 12};
    octaspire_dern_lexer_token_position_t  const expectedUcsIndex = {7, 11};

    ASSERT_EQ(allocator,                              token->allocator);

    ASSERT   (octaspire_dern_lexer_token_position_is_equal(&expectedLine,     token->line));
    ASSERT   (octaspire_dern_lexer_token_position_is_equal(&expectedColumn,   token->column));
    ASSERT   (octaspire_dern_lexer_token_position_is_equal(&expectedUcsIndex, token->ucsIndex));

    ASSERT_EQ(OCTASPIRE_DERN_LEXER_TOKEN_TAG_REAL, token->typeTag);
    ASSERT_EQ(759.2,                               token->value.real);

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_integer_759_amid_whitespace_test(void)
{
    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "   \t   759   \t   ", // 3 spaces + 1 tab + 3 spaces
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    octaspire_dern_lexer_token_position_t  const expectedLine     = {1, 1};
    octaspire_dern_lexer_token_position_t  const expectedColumn   = {8, 10};
    octaspire_dern_lexer_token_position_t  const expectedUcsIndex = {7, 9};

    ASSERT_EQ(allocator,                              token->allocator);

    ASSERT   (octaspire_dern_lexer_token_position_is_equal(&expectedLine,     token->line));
    ASSERT   (octaspire_dern_lexer_token_position_is_equal(&expectedColumn,   token->column));
    ASSERT   (octaspire_dern_lexer_token_position_is_equal(&expectedUcsIndex, token->ucsIndex));

    ASSERT_EQ(OCTASPIRE_DERN_LEXER_TOKEN_TAG_INTEGER, token->typeTag);
    ASSERT_EQ(759,                                    token->value.integer);

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_real_759_dot_2_amid_whitespace_test(void)
{
    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "   \t   759.2   \t   ", // 3 spaces + 1 tab + 3 spaces
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    octaspire_dern_lexer_token_position_t  const expectedLine     = {1, 1};
    octaspire_dern_lexer_token_position_t  const expectedColumn   = {8, 12};
    octaspire_dern_lexer_token_position_t  const expectedUcsIndex = {7, 11};

    ASSERT_EQ(allocator,                              token->allocator);

    ASSERT   (octaspire_dern_lexer_token_position_is_equal(&expectedLine,     token->line));
    ASSERT   (octaspire_dern_lexer_token_position_is_equal(&expectedColumn,   token->column));
    ASSERT   (octaspire_dern_lexer_token_position_is_equal(&expectedUcsIndex, token->ucsIndex));

    ASSERT_EQ(OCTASPIRE_DERN_LEXER_TOKEN_TAG_REAL, token->typeTag);
    ASSERT_EQ(759.2,                               token->value.real);

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_multiline_comment_test(void)
{
    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "  \n  #! here is comment\n 1024!#  ",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    octaspire_dern_lexer_token_position_t  const expectedLine     = {2, 3};
    octaspire_dern_lexer_token_position_t  const expectedColumn   = {3, 7};
    octaspire_dern_lexer_token_position_t  const expectedUcsIndex = {5, 30};

    ASSERT_EQ(allocator,                              token->allocator);

    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedLine,     token->line));
    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedColumn,   token->column));
    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedUcsIndex, token->ucsIndex));

    ASSERT_EQ(OCTASPIRE_DERN_LEXER_TOKEN_TAG_MULTILINE_COMMENT, token->typeTag);
    ASSERT_STR_EQ(
        " here is comment\n 1024",
        octaspire_container_utf8_string_get_c_string(token->value.comment));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_multiline_comment_more_input_required_two_chars_missing_test(void)
{
    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "  \n  #! here is comment\n 1024",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    octaspire_dern_lexer_token_position_t  const expectedLine     = {2, 3};
    octaspire_dern_lexer_token_position_t  const expectedColumn   = {3, 5};
    octaspire_dern_lexer_token_position_t  const expectedUcsIndex = {5, 28};

    ASSERT_EQ(allocator,                              token->allocator);

    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedLine,     token->line));
    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedColumn,   token->column));
    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedUcsIndex, token->ucsIndex));

    ASSERT_EQ(OCTASPIRE_DERN_LEXER_TOKEN_TAG_MORE_INPUT_REQUIRED, token->typeTag);
    ASSERT_STR_EQ(
        "Multiline comment that is not closed with !#",
        octaspire_container_utf8_string_get_c_string(token->value.moreInputRequired));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_multiline_comment_more_input_required_one_char_missing_test(void)
{
    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "  \n  #! here is comment\n 1024!",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    octaspire_dern_lexer_token_position_t  const expectedLine     = {2, 3};
    octaspire_dern_lexer_token_position_t  const expectedColumn   = {3, 6};
    octaspire_dern_lexer_token_position_t  const expectedUcsIndex = {5, 29};

    ASSERT_EQ(allocator,                              token->allocator);

    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedLine,     token->line));
    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedColumn,   token->column));
    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedUcsIndex, token->ucsIndex));

    ASSERT_EQ(OCTASPIRE_DERN_LEXER_TOKEN_TAG_MORE_INPUT_REQUIRED, token->typeTag);
    ASSERT_STR_EQ(
        "Number sign '#' expected after '!' to close multiline comment",
        octaspire_container_utf8_string_get_c_string(token->value.moreInputRequired));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_integer_1024_after_whitespace_and_comment_test(void)
{
    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "  \n  ; here is comment\n1024",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    octaspire_dern_lexer_token_position_t  const expectedLine     = {3, 3};
    octaspire_dern_lexer_token_position_t  const expectedColumn   = {1, 4};
    octaspire_dern_lexer_token_position_t  const expectedUcsIndex = {23, 26};

    ASSERT_EQ(allocator,                              token->allocator);

    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedLine,     token->line));
    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedColumn,   token->column));
    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedUcsIndex, token->ucsIndex));

    ASSERT_EQ(OCTASPIRE_DERN_LEXER_TOKEN_TAG_INTEGER, token->typeTag);
    ASSERT_EQ(1024,                                   token->value.integer);

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_real_1024_dot_987_after_whitespace_and_comment_test(void)
{
    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "  \n  ; here is comment\n1024.987",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    octaspire_dern_lexer_token_position_t  const expectedLine     = {3, 3};
    octaspire_dern_lexer_token_position_t  const expectedColumn   = {1, 8};
    octaspire_dern_lexer_token_position_t  const expectedUcsIndex = {23, 30};

    ASSERT_EQ(allocator,                              token->allocator);

    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedLine,     token->line));
    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedColumn,   token->column));
    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedUcsIndex, token->ucsIndex));

    ASSERT_EQ(OCTASPIRE_DERN_LEXER_TOKEN_TAG_REAL, token->typeTag);
    ASSERT_IN_RANGE(1024.987, token->value.real, 0.0001);

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_integer_0_after_whitespace_and_comment_test(void)
{
    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "  \n  ; here is comment\n0",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    octaspire_dern_lexer_token_position_t  const expectedLine     = {3, 3};
    octaspire_dern_lexer_token_position_t  const expectedColumn   = {1, 1};
    octaspire_dern_lexer_token_position_t  const expectedUcsIndex = {23, 23};

    ASSERT_EQ(allocator,                              token->allocator);

    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedLine,     token->line));
    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedColumn,   token->column));
    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedUcsIndex, token->ucsIndex));

    ASSERT_EQ(OCTASPIRE_DERN_LEXER_TOKEN_TAG_INTEGER, token->typeTag);
    ASSERT_EQ(0,                                   token->value.integer);

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_real_0_dot_0_after_whitespace_and_comment_test(void)
{
    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "  \n  ; here is comment\n0.0",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    octaspire_dern_lexer_token_position_t  const expectedLine     = {3, 3};
    octaspire_dern_lexer_token_position_t  const expectedColumn   = {1, 3};
    octaspire_dern_lexer_token_position_t  const expectedUcsIndex = {23, 25};

    ASSERT_EQ(allocator,                              token->allocator);

    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedLine,     token->line));
    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedColumn,   token->column));
    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedUcsIndex, token->ucsIndex));

    ASSERT_EQ(OCTASPIRE_DERN_LEXER_TOKEN_TAG_REAL, token->typeTag);
    ASSERT_IN_RANGE(0.0, token->value.real, 0.0000001);

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_integer_minus_1024_after_whitespace_and_comment_test(void)
{
    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "  \n  ; here is comment\n-1024",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    octaspire_dern_lexer_token_position_t  const expectedLine     = {3, 3};
    octaspire_dern_lexer_token_position_t  const expectedColumn   = {1, 5};
    octaspire_dern_lexer_token_position_t  const expectedUcsIndex = {23, 27};

    ASSERT_EQ(allocator,                              token->allocator);

    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedLine,     token->line));
    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedColumn,   token->column));
    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedUcsIndex, token->ucsIndex));

    ASSERT_EQ(OCTASPIRE_DERN_LEXER_TOKEN_TAG_INTEGER, token->typeTag);
    ASSERT_EQ(-1024,                                  token->value.integer);

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_real_minus_1024_dot_987_after_whitespace_and_comment_test(void)
{
    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "  \n  ; here is comment\n-1024.987",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    octaspire_dern_lexer_token_position_t  const expectedLine     = {3,  3};
    octaspire_dern_lexer_token_position_t  const expectedColumn   = {1,  9};
    octaspire_dern_lexer_token_position_t  const expectedUcsIndex = {23, 31};

    ASSERT_EQ(allocator,                              token->allocator);

    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedLine,     token->line));
    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedColumn,   token->column));
    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedUcsIndex, token->ucsIndex));

    ASSERT_EQ(OCTASPIRE_DERN_LEXER_TOKEN_TAG_REAL, token->typeTag);
    ASSERT_IN_RANGE(-1024.987, token->value.real, 0.0001);

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_integer_1234567890_after_whitespace_and_comment_test(void)
{
    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "  \n  ; here is comment\n1234567890",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    octaspire_dern_lexer_token_position_t  const expectedLine     = {3,  3};
    octaspire_dern_lexer_token_position_t  const expectedColumn   = {1,  10};
    octaspire_dern_lexer_token_position_t  const expectedUcsIndex = {23, 32};

    ASSERT_EQ(allocator,                              token->allocator);

    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedLine,     token->line));
    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedColumn,   token->column));
    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedUcsIndex, token->ucsIndex));

    ASSERT_EQ(OCTASPIRE_DERN_LEXER_TOKEN_TAG_INTEGER, token->typeTag);
    ASSERT_EQ(1234567890,                            token->value.integer);

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_integer_minus_1234567890_after_whitespace_and_comment_test(void)
{
    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "  \n  ; here is comment\n-1234567890",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    octaspire_dern_lexer_token_position_t  const expectedLine     = {3,  3};
    octaspire_dern_lexer_token_position_t  const expectedColumn   = {1,  11};
    octaspire_dern_lexer_token_position_t  const expectedUcsIndex = {23, 33};

    ASSERT_EQ(allocator,                              token->allocator);

    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedLine,     token->line));
    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedColumn,   token->column));
    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedUcsIndex, token->ucsIndex));

    ASSERT_EQ(OCTASPIRE_DERN_LEXER_TOKEN_TAG_INTEGER, token->typeTag);
    ASSERT_EQ(-1234567890,                            token->value.integer);

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_real_12345_dot_67890_after_whitespace_and_comment_test(void)
{
    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "  \n  ; here is comment\n12345.67890",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    octaspire_dern_lexer_token_position_t  const expectedLine     = {3,  3};
    octaspire_dern_lexer_token_position_t  const expectedColumn   = {1,  11};
    octaspire_dern_lexer_token_position_t  const expectedUcsIndex = {23, 33};

    ASSERT_EQ(allocator,                              token->allocator);

    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedLine,     token->line));
    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedColumn,   token->column));
    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedUcsIndex, token->ucsIndex));

    ASSERT_EQ(OCTASPIRE_DERN_LEXER_TOKEN_TAG_REAL, token->typeTag);
    ASSERT_IN_RANGE(12345.67890, token->value.real, 0.000001);

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_real_minus_12345_dot_67890_after_whitespace_and_comment_test(void)
{
    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "  \n  ; here is comment\n-12345.67890",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    octaspire_dern_lexer_token_position_t  const expectedLine     = {3,  3};
    octaspire_dern_lexer_token_position_t  const expectedColumn   = {1,  12};
    octaspire_dern_lexer_token_position_t  const expectedUcsIndex = {23, 34};

    ASSERT_EQ(allocator,                              token->allocator);

    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedLine,     token->line));
    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedColumn,   token->column));
    ASSERT        (octaspire_dern_lexer_token_position_is_equal(&expectedUcsIndex, token->ucsIndex));

    ASSERT_EQ(OCTASPIRE_DERN_LEXER_TOKEN_TAG_REAL, token->typeTag);
    ASSERT_IN_RANGE(-12345.67890, token->value.real, 0.000001);

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_five_integers_11_22_33_44_55_test(void)
{
    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "11 22 33 44 55",
        allocator);

    ASSERT(input);

    for (size_t i = 0; i < 5; ++i)
    {
        octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
        ASSERT(token);

        octaspire_dern_lexer_token_position_t const expectedLine     = {1,           1};
        octaspire_dern_lexer_token_position_t const expectedColumn   = {1 + (i * 3), 2 + (i * 3)};
        octaspire_dern_lexer_token_position_t const expectedUcsIndex = {0 + (i * 3), 1 + (i * 3)};

        ASSERT_EQ(allocator,                              token->allocator);

        ASSERT   (octaspire_dern_lexer_token_position_is_equal(&expectedLine,     token->line));
        ASSERT   (octaspire_dern_lexer_token_position_is_equal(&expectedColumn,   token->column));
        ASSERT   (octaspire_dern_lexer_token_position_is_equal(&expectedUcsIndex, token->ucsIndex));

        ASSERT_EQ(OCTASPIRE_DERN_LEXER_TOKEN_TAG_INTEGER, token->typeTag);
        ASSERT_EQ(((int32_t)i + 1) * 11,                           token->value.integer);

        octaspire_dern_lexer_token_release(token);
        token = 0;
    }

    ASSERT_FALSE(octaspire_dern_lexer_pop_next_token(input, allocator));

    octaspire_input_release(input);
    input = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_five_reals_11_dot_1_22_dot_2_33_dot_3_44_dot_4_55_dot_5_test(void)
{
    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "11.1 22.2 33.3 44.4 55.5",
        allocator);

    ASSERT(input);

    for (size_t i = 0; i < 5; ++i)
    {
        octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
        ASSERT(token);

        octaspire_dern_lexer_token_position_t const expectedLine     = {1,           1};
        octaspire_dern_lexer_token_position_t const expectedColumn   = {1 + (i * 5), 4 + (i * 5)};
        octaspire_dern_lexer_token_position_t const expectedUcsIndex = {0 + (i * 5), 3 + (i * 5)};

        ASSERT_EQ(allocator,                              token->allocator);

        ASSERT   (octaspire_dern_lexer_token_position_is_equal(&expectedLine,     token->line));
        ASSERT   (octaspire_dern_lexer_token_position_is_equal(&expectedColumn,   token->column));
        ASSERT   (octaspire_dern_lexer_token_position_is_equal(&expectedUcsIndex, token->ucsIndex));

        ASSERT_EQ(OCTASPIRE_DERN_LEXER_TOKEN_TAG_REAL, token->typeTag);
        ASSERT_IN_RANGE(((double)i + 1.0) * 11.1,      token->value.real, 0.0000001);

        octaspire_dern_lexer_token_release(token);
        token = 0;
    }

    for (size_t i = 0; i < 5; ++i)
    {
        ASSERT_FALSE(octaspire_dern_lexer_pop_next_token(input, allocator));
    }

    octaspire_input_release(input);
    input = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_failure_on_integer_12_with_character_a_after_test(void)
{
    char const * const value = "Number cannot contain character 'a'";
    octaspire_dern_lexer_token_t *expected = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
        value,
        octaspire_dern_lexer_token_position_init(1, 1),
        octaspire_dern_lexer_token_position_init(1, 3),
        octaspire_dern_lexer_token_position_init(0, 2),
        allocator);
    ASSERT(expected);

    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "12a",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    ASSERT(octaspire_dern_lexer_token_is_equal(expected, token));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    octaspire_dern_lexer_token_release(expected);
    expected = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_failure_on_illegal_real_12_dot_2_dot_2_test(void)
{
    char const * const value = "Number can contain only one '.' character";
    octaspire_dern_lexer_token_t *expected = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
        value,
        octaspire_dern_lexer_token_position_init(1, 1),
        octaspire_dern_lexer_token_position_init(1, 5),
        octaspire_dern_lexer_token_position_init(0, 4),
        allocator);
    ASSERT(expected);

    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "12.2.2",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    ASSERT(octaspire_dern_lexer_token_is_equal(expected, token));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    octaspire_dern_lexer_token_release(expected);
    expected = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_failure_on_illegal_integer_minus_12_minus_22_test(void)
{
    char const * const value = "Number can contain only one '-' character";
    octaspire_dern_lexer_token_t *expected = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
        value,
        octaspire_dern_lexer_token_position_init(1, 1),
        octaspire_dern_lexer_token_position_init(1, 4),
        octaspire_dern_lexer_token_position_init(0, 3),
        allocator);
    ASSERT(expected);

    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "-12-22",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    ASSERT(octaspire_dern_lexer_token_is_equal(expected, token));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    octaspire_dern_lexer_token_release(expected);
    expected = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_failure_on_illegal_real_minus_12_dot_22_minus_33_test(void)
{
    char const * const value = "Number can contain only one '-' character";
    octaspire_dern_lexer_token_t *expected = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
        value,
        octaspire_dern_lexer_token_position_init(1, 1),
        octaspire_dern_lexer_token_position_init(1, 7),
        octaspire_dern_lexer_token_position_init(0, 6),
        allocator);
    ASSERT(expected);

    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "-12.22-33",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    ASSERT(octaspire_dern_lexer_token_is_equal(expected, token));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    octaspire_dern_lexer_token_release(expected);
    expected = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_failure_on_illegal_integer_12_minus_22_test(void)
{
    char const * const value = "Number can have '-' character only in the beginning";
    octaspire_dern_lexer_token_t *expected = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
        value,
        octaspire_dern_lexer_token_position_init(1, 1),
        octaspire_dern_lexer_token_position_init(1, 3),
        octaspire_dern_lexer_token_position_init(0, 2),
        allocator);
    ASSERT(expected);

    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "12-22",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    ASSERT(octaspire_dern_lexer_token_is_equal(expected, token));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    octaspire_dern_lexer_token_release(expected);
    expected = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_failure_on_illegal_integer_12_dot_22_minus_22_test(void)
{
    char const * const value = "Number can have '-' character only in the beginning";
    octaspire_dern_lexer_token_t *expected = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
        value,
        octaspire_dern_lexer_token_position_init(1, 1),
        octaspire_dern_lexer_token_position_init(1, 6),
        octaspire_dern_lexer_token_position_init(0, 5),
        allocator);
    ASSERT(expected);

    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "12.22-22",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    ASSERT(octaspire_dern_lexer_token_is_equal(expected, token));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    octaspire_dern_lexer_token_release(expected);
    expected = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_string_cat_and_dog_test(void)
{
    char const * const value = "Cat and dog.";
    octaspire_dern_lexer_token_t *expected = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_STRING,
        value,
        octaspire_dern_lexer_token_position_init(1, 1),
        octaspire_dern_lexer_token_position_init(1, 14),
        octaspire_dern_lexer_token_position_init(0, 13),
        allocator);
    ASSERT(expected);

    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "[Cat and dog.]",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    ASSERT(octaspire_dern_lexer_token_is_equal(expected, token));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    octaspire_dern_lexer_token_release(expected);
    expected = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_character_a_test(void)
{
    char const * const value = "a";
    octaspire_dern_lexer_token_t *expected = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_CHARACTER,
        value,
        octaspire_dern_lexer_token_position_init(1, 1),
        octaspire_dern_lexer_token_position_init(1, 3),
        octaspire_dern_lexer_token_position_init(0, 2),
        allocator);
    ASSERT(expected);

    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "|a|",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    ASSERT(octaspire_dern_lexer_token_is_equal(expected, token));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    octaspire_dern_lexer_token_release(expected);
    expected = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_character_vertical_line_test(void)
{
    char const * const value = "|";
    octaspire_dern_lexer_token_t *expected = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_CHARACTER,
        value,
        octaspire_dern_lexer_token_position_init(1, 1),
        octaspire_dern_lexer_token_position_init(1, 5),
        octaspire_dern_lexer_token_position_init(0, 4),
        allocator);
    ASSERT(expected);

    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "|bar|",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    ASSERT(octaspire_dern_lexer_token_is_equal(expected, token));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    octaspire_dern_lexer_token_release(expected);
    expected = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_character_newline_test(void)
{
    char const * const value = "\n";
    octaspire_dern_lexer_token_t *expected = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_CHARACTER,
        value,
        octaspire_dern_lexer_token_position_init(1, 1),
        octaspire_dern_lexer_token_position_init(1, 9),
        octaspire_dern_lexer_token_position_init(0, 8),
        allocator);
    ASSERT(expected);

    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "|newline|",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    ASSERT(octaspire_dern_lexer_token_is_equal(expected, token));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    octaspire_dern_lexer_token_release(expected);
    expected = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_character_tabulator_test(void)
{
    char const * const value = "\t";
    octaspire_dern_lexer_token_t *expected = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_CHARACTER,
        value,
        octaspire_dern_lexer_token_position_init(1, 1),
        octaspire_dern_lexer_token_position_init(1, 5),
        octaspire_dern_lexer_token_position_init(0, 4),
        allocator);
    ASSERT(expected);

    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "|tab|",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    ASSERT(octaspire_dern_lexer_token_is_equal(expected, token));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    octaspire_dern_lexer_token_release(expected);
    expected = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_character_a_amid_whitespace_test(void)
{
    char const * const value = "a";
    octaspire_dern_lexer_token_t *expected = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_CHARACTER,
        value,
        octaspire_dern_lexer_token_position_init(2, 2),
        octaspire_dern_lexer_token_position_init(6, 8),
        octaspire_dern_lexer_token_position_init(7, 9),
        allocator);
    ASSERT(expected);

    octaspire_input_t *input = octaspire_input_new_from_c_string(
        " \n \t \t |a| \n \t ",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    ASSERT(octaspire_dern_lexer_token_is_equal(expected, token));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    octaspire_dern_lexer_token_release(expected);
    expected = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_five_characters_a_b_c_d_e_amid_whitespace_test(void)
{
    octaspire_dern_lexer_token_t *expected[5];

    for (size_t i = 0; i < 5; ++i)
    {
        char const value[2] = {(char)((char)97 + (char)i) , '\0'};

        octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_token_new(
                OCTASPIRE_DERN_LEXER_TOKEN_TAG_CHARACTER,
                value,
                octaspire_dern_lexer_token_position_init(2, 2),
                octaspire_dern_lexer_token_position_init(1 + (i * 4), 3 + (i * 4)),
                octaspire_dern_lexer_token_position_init(1 + (i * 4), 3 + (i * 4)),
                allocator);

        expected[i] = token;

        ASSERT(expected[i]);
    }

    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "\n|a| |b| |c| |d| |e|  ",
        allocator);

    ASSERT(input);

    for (size_t i = 0; i < 5; ++i)
    {
        octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
        ASSERT(token);

        ASSERT(octaspire_dern_lexer_token_is_equal(expected[i], token));

        octaspire_dern_lexer_token_release(token);
        token = 0;
    }

    octaspire_input_release(input);
    input = 0;

    for (size_t i = 0; i < 5; ++i)
    {
        octaspire_dern_lexer_token_release(expected[i]);
        expected[i] = 0;
    }

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_five_characters_a_b_c_d_e_no_whitespace_test(void)
{
    octaspire_dern_lexer_token_t *expected[5];

    for (size_t i = 0; i < 5; ++i)
    {
        char const value[2] = {(char)((char)97 + (char)i) , '\0'};

        octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_token_new(
                OCTASPIRE_DERN_LEXER_TOKEN_TAG_CHARACTER,
                value,
                octaspire_dern_lexer_token_position_init(1, 1),
                octaspire_dern_lexer_token_position_init(1 + (i * 3), 3 + (i * 3)),
                octaspire_dern_lexer_token_position_init(0 + (i * 3), 2 + (i * 3)),
                allocator);

        expected[i] = token;

        ASSERT(expected[i]);
    }

    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "|a||b||c||d||e|",
        allocator);

    ASSERT(input);

    for (size_t i = 0; i < 5; ++i)
    {
        octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
        ASSERT(token);

        ASSERT(octaspire_dern_lexer_token_is_equal(expected[i], token));

        octaspire_dern_lexer_token_release(token);
        token = 0;
    }

    octaspire_input_release(input);
    input = 0;

    for (size_t i = 0; i < 5; ++i)
    {
        octaspire_dern_lexer_token_release(expected[i]);
        expected[i] = 0;
    }

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_character_a_end_of_input_before_end_delimiter_returns_null_token_test(void)
{
    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "|a",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT_FALSE(token);

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_illegal_character_constant_now_line_no_whitespace_test(void)
{
    char const * const value = "Unknown character constant |now line|";
    octaspire_dern_lexer_token_t *expected = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
        value,
        octaspire_dern_lexer_token_position_init(1, 1),
        octaspire_dern_lexer_token_position_init(1, 10),
        octaspire_dern_lexer_token_position_init(0, 9),
        allocator);
    ASSERT(expected);

    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "|now line|",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    ASSERT(octaspire_dern_lexer_token_is_equal(expected, token));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    octaspire_dern_lexer_token_release(expected);
    expected = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_illegal_character_empty_character_test(void)
{
    char const * const value = "Character cannot be empty: ||";
    octaspire_dern_lexer_token_t *expected = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
        value,
        octaspire_dern_lexer_token_position_init(1, 1),
        octaspire_dern_lexer_token_position_init(1, 2),
        octaspire_dern_lexer_token_position_init(0, 1),
        allocator);
    ASSERT(expected);

    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "||",
        allocator);

    ASSERT(input);

    octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
    ASSERT(token);

    ASSERT(octaspire_dern_lexer_token_is_equal(expected, token));

    octaspire_dern_lexer_token_release(token);
    token = 0;

    octaspire_input_release(input);
    input = 0;

    octaspire_dern_lexer_token_release(expected);
    expected = 0;

    PASS();
}

TEST octaspire_dern_lexer_pop_next_token_all_token_types_amid_whitespace_test(void)
{
    octaspire_dern_lexer_token_t *expected[12];

    uint32_t const     integerVal = 123;
    double const       realVal    = 987.456;
    char const * const strVal     = "here is a string";
    char const * const charVal    = "+";
    char const * const symbolVal  = "here_is_a_symbol";
    char const * const errorVal   = "Number cannot contain character 'a'";

    void const * const values[12] =
    {
        0,
        0,
        0,
        0,
        0,
        0,
        &integerVal,
        &realVal,
        strVal,
        charVal,
        symbolVal,
        errorVal
    };

    octaspire_dern_lexer_token_position_t linePositions[12] =
    {
        {1, 1},
        {1, 1},
        {1, 1},
        {1, 1},
        {1, 1},
        {1, 1},
        {1, 1},
        {1, 1},
        {1, 1},
        {1, 1},
        {1, 1},
        {1, 1}
    };

    octaspire_dern_lexer_token_position_t columnPositions[12] =
    {
        {1, 1},
        {3, 3},
        {5, 5},
        {7, 10},
        {12, 16},
        {18, 20},
        {22, 24},
        {26, 32},
        {34, 51},
        {53, 55},
        {57, 72},
        {74, 76}
    };

    // + 6
    octaspire_dern_lexer_token_position_t ucsIndexPositions[12] =
    {
        {0, 0},
        {2, 2},
        {4, 4},
        {6, 9},
        {11, 15},
        {17, 19},
        {21, 23},
        {25, 31},
        {33, 50},
        {52, 54},
        {56, 71},
        {73, 75}
    };

    ASSERT((sizeof(expected) / sizeof(expected[0])) == (sizeof(values) / sizeof(values[0])));
    ASSERT((sizeof(expected) / sizeof(expected[0])) == (sizeof(linePositions) / sizeof(linePositions[0])));
    ASSERT((sizeof(expected) / sizeof(expected[0])) == (sizeof(columnPositions) / sizeof(columnPositions[0])));
    ASSERT((sizeof(expected) / sizeof(expected[0])) == (sizeof(ucsIndexPositions) / sizeof(ucsIndexPositions[0])));

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_token_new(
            (octaspire_dern_lexer_token_tag_t)i,
            values[i],
            linePositions[i],
            columnPositions[i],
            ucsIndexPositions[i],
            allocator);

        expected[i] = token;

        ASSERT(expected[i]);
    }

    octaspire_input_t *input = octaspire_input_new_from_c_string(
        "( ) ' true false nil 123 987.456 [here is a string] |+| here_is_a_symbol 12a",
        allocator);

    ASSERT(input);

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        octaspire_dern_lexer_token_t *token = octaspire_dern_lexer_pop_next_token(input, allocator);
        ASSERT(token);

        ASSERT(octaspire_dern_lexer_token_is_equal(expected[i], token));

        octaspire_dern_lexer_token_release(token);
        token = 0;
    }

    octaspire_input_release(input);
    input = 0;

    for (size_t i = 0; i < (sizeof(expected) / sizeof(expected[0])); ++i)
    {
        octaspire_dern_lexer_token_release(expected[i]);
        expected[i] = 0;
    }

    PASS();
}

static size_t octaspireDernLexerSuiteNumTimesRun = 0;

GREATEST_SUITE(octaspire_dern_lexer_suite)
{
    octaspireDernLexerSuiteNumTimesRun = 0;

    allocator = octaspire_memory_allocator_new_create_region(
        OCTASPIRE_DERN_CONFIG_MEMORY_ALLOCATOR_REGION_MIN_BLOCK_SIZE_IN_OCTETS);

second_run:

    assert(allocator);

    RUN_TEST(octaspire_dern_lexer_token_new_test);
    RUN_TEST(octaspire_dern_lexer_token_new_allocation_failure_on_first_allocation_test);
    RUN_TEST(octaspire_dern_lexer_token_new_allocation_failure_on_second_allocation_when_tag_is_string_test);
    RUN_TEST(octaspire_dern_lexer_token_new_allocation_failure_on_second_allocation_when_tag_is_error_test);
    RUN_TEST(octaspire_dern_lexer_token_get_type_tag_test);
    RUN_TEST(octaspire_dern_lexer_token_get_type_tag_as_c_string_test);
    RUN_TEST(octaspire_dern_lexer_token_get_integer_value_test);
    RUN_TEST(octaspire_dern_lexer_token_get_line_test);
    RUN_TEST(octaspire_dern_lexer_token_get_column_test);
    RUN_TEST(octaspire_dern_lexer_token_to_string_with_lparen_token_test);
    RUN_TEST(octaspire_dern_lexer_token_to_string_with_rparen_token_test);
    RUN_TEST(octaspire_dern_lexer_token_to_string_with_integer_token_test);
    RUN_TEST(octaspire_dern_lexer_token_to_string_with_error_token_test);
    RUN_TEST(octaspire_dern_lexer_token_to_string_with_unknow_token_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_left_parenthesis_no_whitespace_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_right_parenthesis_no_whitespace_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_left_parenthesis_amid_whitespace_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_right_parenthesis_amid_whitespace_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_quote_no_whitespace_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_quote_amid_whitespace_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_true_no_whitespace_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_true_amid_whitespace_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_nil_no_whitespace_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_nil_amid_whitespace_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_symbol_length_no_whitespace_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_symbol_length_amid_whitespace_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_integer_12_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_real_12_dot_3_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_integer_759_after_whitespace_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_real_759_dot_2_after_whitespace_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_integer_759_amid_whitespace_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_real_759_dot_2_amid_whitespace_test);

    RUN_TEST(octaspire_dern_lexer_pop_next_token_multiline_comment_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_multiline_comment_more_input_required_two_chars_missing_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_multiline_comment_more_input_required_one_char_missing_test);

    RUN_TEST(octaspire_dern_lexer_pop_next_token_integer_1024_after_whitespace_and_comment_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_real_1024_dot_987_after_whitespace_and_comment_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_integer_0_after_whitespace_and_comment_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_real_0_dot_0_after_whitespace_and_comment_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_integer_minus_1024_after_whitespace_and_comment_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_real_minus_1024_dot_987_after_whitespace_and_comment_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_integer_minus_1234567890_after_whitespace_and_comment_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_integer_1234567890_after_whitespace_and_comment_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_real_12345_dot_67890_after_whitespace_and_comment_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_real_minus_12345_dot_67890_after_whitespace_and_comment_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_five_integers_11_22_33_44_55_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_five_reals_11_dot_1_22_dot_2_33_dot_3_44_dot_4_55_dot_5_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_failure_on_integer_12_with_character_a_after_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_failure_on_illegal_real_12_dot_2_dot_2_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_failure_on_illegal_integer_minus_12_minus_22_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_failure_on_illegal_real_minus_12_dot_22_minus_33_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_failure_on_illegal_integer_12_minus_22_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_failure_on_illegal_integer_12_dot_22_minus_22_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_string_cat_and_dog_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_character_a_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_character_vertical_line_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_character_newline_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_character_tabulator_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_character_a_amid_whitespace_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_five_characters_a_b_c_d_e_amid_whitespace_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_five_characters_a_b_c_d_e_no_whitespace_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_character_a_end_of_input_before_end_delimiter_returns_null_token_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_illegal_character_constant_now_line_no_whitespace_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_illegal_character_empty_character_test);
    RUN_TEST(octaspire_dern_lexer_pop_next_token_all_token_types_amid_whitespace_test);

    octaspire_memory_allocator_release(allocator);
    allocator = 0;

    ++octaspireDernLexerSuiteNumTimesRun;

    if (octaspireDernLexerSuiteNumTimesRun < 2)
    {
        // Second run without region allocator

        allocator = octaspire_memory_allocator_new(0);

        goto second_run;
    }
}
