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
#ifndef OCTASPIRE_DERN_LEXER_H
#define OCTASPIRE_DERN_LEXER_H

#include <stddef.h>

#ifndef OCTASPIRE_DERN_DO_NOT_USE_AMALGAMATED_CORE
    #include "octaspire-core-amalgamated.c"
#else
    #include <octaspire/core/octaspire_memory.h>
    #include <octaspire/core/octaspire_string.h>
    #include <octaspire/core/octaspire_input.h>
#endif

#ifdef __cplusplus
extern "C"       {
#endif

typedef enum
{
    OCTASPIRE_DERN_LEXER_TOKEN_TAG_LPAREN,
    OCTASPIRE_DERN_LEXER_TOKEN_TAG_RPAREN,
    OCTASPIRE_DERN_LEXER_TOKEN_TAG_QUOTE,
    OCTASPIRE_DERN_LEXER_TOKEN_TAG_BACK_QUOTE,
    OCTASPIRE_DERN_LEXER_TOKEN_TAG_TRUE,
    OCTASPIRE_DERN_LEXER_TOKEN_TAG_FALSE,
    OCTASPIRE_DERN_LEXER_TOKEN_TAG_NIL,
    OCTASPIRE_DERN_LEXER_TOKEN_TAG_INTEGER,
    OCTASPIRE_DERN_LEXER_TOKEN_TAG_REAL,
    OCTASPIRE_DERN_LEXER_TOKEN_TAG_STRING,
    OCTASPIRE_DERN_LEXER_TOKEN_TAG_CHARACTER,
    OCTASPIRE_DERN_LEXER_TOKEN_TAG_SYMBOL,
    OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
    OCTASPIRE_DERN_LEXER_TOKEN_TAG_MORE_INPUT_REQUIRED,
    OCTASPIRE_DERN_LEXER_TOKEN_TAG_MULTILINE_COMMENT,
    OCTASPIRE_DERN_LEXER_TOKEN_TAG_SEMVER
}
octaspire_dern_lexer_token_tag_t;

typedef struct octaspire_dern_lexer_token_t octaspire_dern_lexer_token_t;

typedef struct octaspire_dern_lexer_token_position_t
{
    size_t start;
    size_t end;
}
octaspire_dern_lexer_token_position_t;

octaspire_dern_lexer_token_position_t octaspire_dern_lexer_token_position_init(
    size_t const start, size_t const end);

bool octaspire_dern_lexer_token_position_is_equal(
    octaspire_dern_lexer_token_position_t const * const self,
    octaspire_dern_lexer_token_position_t const * const other);

octaspire_dern_lexer_token_t *octaspire_dern_lexer_token_new(
    octaspire_dern_lexer_token_tag_t const typeTag,
    void const * const value,
    octaspire_dern_lexer_token_position_t const line,
    octaspire_dern_lexer_token_position_t const column,
    octaspire_dern_lexer_token_position_t const ucsIndex,
    octaspire_allocator_t *allocator);

octaspire_dern_lexer_token_t *octaspire_dern_lexer_token_new_format(
    octaspire_dern_lexer_token_tag_t const typeTag,
    octaspire_dern_lexer_token_position_t const line,
    octaspire_dern_lexer_token_position_t const column,
    octaspire_dern_lexer_token_position_t const ucsIndex,
    octaspire_allocator_t *allocator,
    void const * const format,
    ...);

void octaspire_dern_lexer_token_release(
    octaspire_dern_lexer_token_t *self);

octaspire_dern_lexer_token_tag_t octaspire_dern_lexer_token_get_type_tag(
    octaspire_dern_lexer_token_t const * const self);

char const *octaspire_dern_lexer_token_get_type_tag_as_c_string(
    octaspire_dern_lexer_token_t const * const self);

int32_t octaspire_dern_lexer_token_get_integer_value(
    octaspire_dern_lexer_token_t const * const self);

double octaspire_dern_lexer_token_get_real_value(
    octaspire_dern_lexer_token_t const * const self);

char const *octaspire_dern_lexer_token_get_string_value_as_c_string(
    octaspire_dern_lexer_token_t const * const self);

char const *octaspire_dern_lexer_token_get_character_value_as_c_string(
    octaspire_dern_lexer_token_t const * const self);

char const *octaspire_dern_lexer_token_get_symbol_value_as_c_string(
    octaspire_dern_lexer_token_t const * const self);

char const *octaspire_dern_lexer_token_get_error_value_as_c_string(
    octaspire_dern_lexer_token_t const * const self);

octaspire_semver_t const *octaspire_dern_lexer_token_get_semver_value(
    octaspire_dern_lexer_token_t const * const self);

char const *octaspire_dern_lexer_token_get_multiline_comment_value_as_c_string(
    octaspire_dern_lexer_token_t const * const self);



octaspire_dern_lexer_token_position_t *octaspire_dern_lexer_token_get_position_line(
    octaspire_dern_lexer_token_t const * const self);

octaspire_dern_lexer_token_position_t *octaspire_dern_lexer_token_get_position_column(
    octaspire_dern_lexer_token_t const * const self);

octaspire_dern_lexer_token_position_t *octaspire_dern_lexer_token_get_position_ucs_index(
    octaspire_dern_lexer_token_t const * const self);



bool octaspire_dern_lexer_token_is_equal(
    octaspire_dern_lexer_token_t const * const self,
    octaspire_dern_lexer_token_t const * const other);


octaspire_string_t *octaspire_dern_lexer_token_to_string(
    octaspire_dern_lexer_token_t const * const self);

void octaspire_dern_lexer_token_print(
    octaspire_dern_lexer_token_t const * const self);

octaspire_dern_lexer_token_t *octaspire_dern_lexer_pop_next_token(
    octaspire_input_t *input,
    octaspire_allocator_t *allocator);

#ifdef __cplusplus
/* extern "C" */ }
#endif

#endif

