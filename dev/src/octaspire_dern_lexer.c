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
#include "octaspire/dern/octaspire_dern_lexer.h"
#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>

#ifndef OCTASPIRE_DERN_DO_NOT_USE_AMALGAMATED_CORE
    #include "octaspire-core-amalgamated.c"
#else
    #include <octaspire/core/octaspire_string.h>
    #include <octaspire/core/octaspire_helpers.h>
#endif

struct octaspire_dern_lexer_token_t
{
    octaspire_allocator_t                 *allocator;
    octaspire_dern_lexer_token_position_t *line;
    octaspire_dern_lexer_token_position_t *column;
    octaspire_dern_lexer_token_position_t *ucsIndex;

    union
    {
        octaspire_string_t *string;
        octaspire_string_t *character;
        octaspire_string_t *comment;
        octaspire_string_t *symbol;
        octaspire_string_t *error;
        octaspire_string_t *moreInputRequired;
        octaspire_semver_t *semver;
        int32_t             integer;
        double              real;
    }
    value;

    octaspire_dern_lexer_token_tag_t typeTag;
    char                             padding[4];
};

static char const * const octaspire_dern_lexer_private_token_tag_types_as_c_strings[] =
{
    "OCTASPIRE_DERN_LEXER_TOKEN_TAG_LPAREN",
    "OCTASPIRE_DERN_LEXER_TOKEN_TAG_RPAREN",
    "OCTASPIRE_DERN_LEXER_TOKEN_TAG_QUOTE",
    "OCTASPIRE_DERN_LEXER_TOKEN_TAG_BACK_QUOTE",
    "OCTASPIRE_DERN_LEXER_TOKEN_TAG_TRUE",
    "OCTASPIRE_DERN_LEXER_TOKEN_TAG_FALSE",
    "OCTASPIRE_DERN_LEXER_TOKEN_TAG_NIL",
    "OCTASPIRE_DERN_LEXER_TOKEN_TAG_INTEGER",
    "OCTASPIRE_DERN_LEXER_TOKEN_TAG_REAL",
    "OCTASPIRE_DERN_LEXER_TOKEN_TAG_STRING",
    "OCTASPIRE_DERN_LEXER_TOKEN_TAG_CHARACTER",
    "OCTASPIRE_DERN_LEXER_TOKEN_TAG_SYMBOL",
    "OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR",
    "OCTASPIRE_DERN_LEXER_TOKEN_TAG_MORE_INPUT_REQUIRED",
    "OCTASPIRE_DERN_LEXER_TOKEN_TAG_MULTILINE_COMMENT",
    "OCTASPIRE_DERN_LEXER_TOKEN_TAG_SEMVER",
};

void octaspire_dern_lexer_private_pop_whitespace(
    octaspire_input_t *input);

void octaspire_dern_lexer_private_pop_rest_of_line(
    octaspire_input_t *input);

octaspire_dern_lexer_token_t *octaspire_dern_lexer_private_pop_multiline_comment(
    octaspire_input_t *input,
    octaspire_allocator_t *allocator,
    size_t const startLine,
    size_t const startColumn,
    size_t const startIndexInInput);

octaspire_dern_lexer_token_t *octaspire_dern_lexer_private_pop_left_parenthesis(
    octaspire_input_t *input,
    octaspire_allocator_t *allocator,
    size_t const startLine,
    size_t const startColumn,
    size_t const startIndexInInput);

octaspire_dern_lexer_token_t *octaspire_dern_lexer_private_pop_right_parenthesis(
    octaspire_input_t *input,
    octaspire_allocator_t *allocator,
    size_t const startLine,
    size_t const startColumn,
    size_t const startIndexInInput);

octaspire_dern_lexer_token_t *octaspire_dern_lexer_private_pop_quote(
    octaspire_input_t *input,
    octaspire_allocator_t *allocator,
    size_t const startLine,
    size_t const startColumn,
    size_t const startIndexInInput);

octaspire_dern_lexer_token_t *octaspire_dern_lexer_private_pop_back_quote(
    octaspire_input_t *input,
    octaspire_allocator_t *allocator,
    size_t const startLine,
    size_t const startColumn,
    size_t const startIndexInInput);

octaspire_dern_lexer_token_t *octaspire_dern_lexer_private_pop_integer_or_real_number(
    octaspire_input_t *input,
    octaspire_allocator_t *allocator,
    size_t const startLine,
    size_t const startColumn,
    size_t const startIndexInInput);

octaspire_dern_lexer_token_t *octaspire_dern_lexer_private_pop_string(
    octaspire_input_t *input,
    octaspire_allocator_t *allocator,
    size_t const startLine,
    size_t const startColumn,
    size_t const startIndexInInput);

octaspire_dern_lexer_token_t *octaspire_dern_lexer_private_pop_character(
    octaspire_input_t *input,
    octaspire_allocator_t *allocator,
    size_t const startLine,
    size_t const startColumn,
    size_t const startIndexInInput);

octaspire_dern_lexer_token_t *octaspire_dern_lexer_private_pop_true_or_false_or_nil_or_symbol(
    octaspire_input_t *input,
    octaspire_allocator_t *allocator,
    size_t const startLine,
    size_t const startColumn,
    size_t const startIndexInInput);

bool octaspire_dern_lexer_private_is_delimeter(uint32_t const c);



octaspire_dern_lexer_token_position_t octaspire_dern_lexer_token_position_init(
    size_t const start, size_t const end)
{
    octaspire_dern_lexer_token_position_t result;
    result.start = start;
    result.end   = end;
    return result;
}

bool octaspire_dern_lexer_token_position_is_equal(
    octaspire_dern_lexer_token_position_t const * const self,
    octaspire_dern_lexer_token_position_t const * const other)
{
    return self->start == other->start &&
           self->end   == other->end;
}

octaspire_dern_lexer_token_t *octaspire_dern_lexer_token_new(
    octaspire_dern_lexer_token_tag_t const typeTag,
    void const * const value,
    octaspire_dern_lexer_token_position_t const line,
    octaspire_dern_lexer_token_position_t const column,
    octaspire_dern_lexer_token_position_t const ucsIndex,
    octaspire_allocator_t *allocator)
{
    octaspire_dern_lexer_token_t *self = octaspire_allocator_malloc(
        allocator,
        sizeof(octaspire_dern_lexer_token_t));

    if (!self)
    {
        return self;
    }

    self->allocator = allocator;

    self->line =
        octaspire_allocator_malloc(allocator, sizeof(octaspire_dern_lexer_token_position_t));

    self->column =
        octaspire_allocator_malloc(allocator, sizeof(octaspire_dern_lexer_token_position_t));

    self->ucsIndex =
        octaspire_allocator_malloc(allocator, sizeof(octaspire_dern_lexer_token_position_t));

    if (!self->line || !self->column || !self->ucsIndex)
    {
        octaspire_dern_lexer_token_release(self);
        self = 0;
        return 0;
    }

    self->line->start = line.start;
    self->line->end   = line.end;

    self->column->start = column.start;
    self->column->end   = column.end;

    self->ucsIndex->start = ucsIndex.start;
    self->ucsIndex->end   = ucsIndex.end;

    self->typeTag   = typeTag;

    switch (self->typeTag)
    {
        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_LPAREN:
        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_RPAREN:
        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_QUOTE:
        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_BACK_QUOTE:
        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_TRUE:
        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_FALSE:
        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_NIL:
        {
        }
        break;

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_INTEGER:
        {
            self->value.integer = *(int32_t const * const)value;
        }
        break;

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_REAL:
        {
            self->value.real = *(double const * const)value;
        }
        break;

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_STRING:
        {
            self->value.string = octaspire_string_new(
                (char const * const)value,
                allocator);

            if (!self->value.string)
            {
                octaspire_dern_lexer_token_release(self);
                self = 0;
                return 0;
            }

            octaspire_helpers_verify_true(
                octaspire_string_get_error_status(self->value.string) ==
                OCTASPIRE_STRING_ERROR_STATUS_OK);
        }
        break;

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_MULTILINE_COMMENT:
        {
            self->value.comment = octaspire_string_new(
                (char const * const)value,
                allocator);

            if (!self->value.comment)
            {
                octaspire_dern_lexer_token_release(self);
                self = 0;
                return 0;
            }

            octaspire_helpers_verify_true(
                octaspire_string_get_error_status(self->value.comment) ==
                OCTASPIRE_STRING_ERROR_STATUS_OK);
        }
        break;

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_SEMVER:
        {
            self->value.semver = octaspire_semver_new_copy(value, allocator);

            if (!self->value.semver)
            {
                octaspire_dern_lexer_token_release(self);
                self = 0;
                return 0;
            }
        }
        break;

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_MORE_INPUT_REQUIRED:
        {
            self->value.moreInputRequired = octaspire_string_new(
                (char const * const)value,
                allocator);

            if (!self->value.moreInputRequired)
            {
                octaspire_dern_lexer_token_release(self);
                self = 0;
                return 0;
            }

            octaspire_helpers_verify_true(
                octaspire_string_get_error_status(self->value.moreInputRequired) ==
                OCTASPIRE_STRING_ERROR_STATUS_OK);
        }
        break;

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_CHARACTER:
        {
            self->value.character = octaspire_string_new(
                (char const * const)value,
                allocator);

            if (!self->value.character)
            {
                octaspire_dern_lexer_token_release(self);
                self = 0;
                return 0;
            }

            octaspire_helpers_verify_true(
                octaspire_string_get_error_status(self->value.character) ==
                OCTASPIRE_STRING_ERROR_STATUS_OK);
        }
        break;

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_SYMBOL:
        {
            self->value.symbol = octaspire_string_new(
                (char const * const)value,
                allocator);

            if (!self->value.symbol)
            {
                octaspire_dern_lexer_token_release(self);
                self = 0;
                return 0;
            }

            octaspire_helpers_verify_true(
                octaspire_string_get_error_status(self->value.symbol) ==
                OCTASPIRE_STRING_ERROR_STATUS_OK);
        }
        break;

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR:
        {
            self->value.error = octaspire_string_new(
                (char const * const)value,
                allocator);

            if (!self->value.error)
            {
                octaspire_dern_lexer_token_release(self);
                self = 0;
                return 0;
            }

            octaspire_helpers_verify_true(
                octaspire_string_get_error_status(self->value.error) ==
                OCTASPIRE_STRING_ERROR_STATUS_OK);
        }
        break;
    }

    return self;
}

octaspire_dern_lexer_token_t *octaspire_dern_lexer_token_new_format(
    octaspire_dern_lexer_token_tag_t const typeTag,
    octaspire_dern_lexer_token_position_t const line,
    octaspire_dern_lexer_token_position_t const column,
    octaspire_dern_lexer_token_position_t const ucsIndex,
    octaspire_allocator_t *allocator,
    void const * const format,
    ...)
{
    va_list arguments;
    va_start(arguments, format);
    octaspire_string_t *str = octaspire_string_new_vformat(
        allocator,
        format,
        arguments);
    va_end(arguments);

    assert(
        typeTag == OCTASPIRE_DERN_LEXER_TOKEN_TAG_STRING ||
        typeTag == OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR);

    octaspire_dern_lexer_token_t *result = octaspire_dern_lexer_token_new(
        typeTag,
        octaspire_string_get_c_string(str),
        line,
        column,
        ucsIndex,
        allocator);

    octaspire_string_release(str);
    str = 0;

    return result;
}


void octaspire_dern_lexer_token_release(
    octaspire_dern_lexer_token_t *self)
{
    if (!self)
    {
        return;
    }

    switch (self->typeTag)
    {
        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_STRING:
        {
            octaspire_string_release(self->value.string);
        }
        break;

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_CHARACTER:
        {
            octaspire_string_release(self->value.character);
        }
        break;

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_SYMBOL:
        {
            octaspire_string_release(self->value.symbol);
        }
        break;

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR:
        {
            octaspire_string_release(self->value.error);
        }
        break;

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_MORE_INPUT_REQUIRED:
        {
            octaspire_string_release(self->value.moreInputRequired);
        }
        break;

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_MULTILINE_COMMENT:
        {
            octaspire_string_release(self->value.comment);
        }
        break;

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_SEMVER:
        {
            octaspire_semver_release(self->value.semver);
        }
        break;

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_LPAREN:
        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_RPAREN:
        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_QUOTE:
        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_BACK_QUOTE:
        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_TRUE:
        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_FALSE:
        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_NIL:
        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_INTEGER:
        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_REAL:
        {
            // Nothing to be done here
        }
        break;
    }

    octaspire_allocator_free(self->allocator, self->line);
    self->line = 0;

    octaspire_allocator_free(self->allocator, self->column);
    self->column = 0;

    octaspire_allocator_free(self->allocator, self->ucsIndex);
    self->ucsIndex = 0;

    octaspire_allocator_free(self->allocator, self);
}

octaspire_dern_lexer_token_tag_t octaspire_dern_lexer_token_get_type_tag(
    octaspire_dern_lexer_token_t const * const self)
{
    assert(self);
    return self->typeTag;
}

char const *octaspire_dern_lexer_token_get_type_tag_as_c_string(
    octaspire_dern_lexer_token_t const * const self)
{
    octaspire_dern_lexer_token_tag_t const tag = octaspire_dern_lexer_token_get_type_tag(self);

    assert(
        (size_t)tag <
            (sizeof(octaspire_dern_lexer_private_token_tag_types_as_c_strings) /
                sizeof(octaspire_dern_lexer_private_token_tag_types_as_c_strings[0])));

    return octaspire_dern_lexer_private_token_tag_types_as_c_strings[tag];
}

char const *octaspire_dern_lexer_token_get_string_value_as_c_string(
    octaspire_dern_lexer_token_t const * const self)
{
    return octaspire_string_get_c_string(self->value.string);
}

char const *octaspire_dern_lexer_token_get_character_value_as_c_string(
    octaspire_dern_lexer_token_t const * const self)
{
    return octaspire_string_get_c_string(self->value.character);
}

char const *octaspire_dern_lexer_token_get_symbol_value_as_c_string(
    octaspire_dern_lexer_token_t const * const self)
{
    return octaspire_string_get_c_string(self->value.symbol);
}

int32_t octaspire_dern_lexer_token_get_integer_value(
    octaspire_dern_lexer_token_t const * const self)
{
    return self->value.integer;
}

double octaspire_dern_lexer_token_get_real_value(
    octaspire_dern_lexer_token_t const * const self)
{
    return self->value.real;
}

char const *octaspire_dern_lexer_token_get_error_value_as_c_string(
    octaspire_dern_lexer_token_t const * const self)
{
    return octaspire_string_get_c_string(self->value.error);
}

octaspire_semver_t const *octaspire_dern_lexer_token_get_semver_value(
    octaspire_dern_lexer_token_t const * const self)
{
    octaspire_helpers_verify_true(self->typeTag == OCTASPIRE_DERN_LEXER_TOKEN_TAG_SEMVER);
    return self->value.semver;
}

char const *octaspire_dern_lexer_token_get_multiline_comment_value_as_c_string(
    octaspire_dern_lexer_token_t const * const self)
{
    return octaspire_string_get_c_string(self->value.comment);
}

octaspire_dern_lexer_token_position_t *octaspire_dern_lexer_token_get_position_line(
    octaspire_dern_lexer_token_t const * const self)
{
    return self->line;
}

octaspire_dern_lexer_token_position_t *octaspire_dern_lexer_token_get_position_column(
    octaspire_dern_lexer_token_t const * const self)
{
    return self->column;
}

octaspire_dern_lexer_token_position_t *octaspire_dern_lexer_token_get_position_ucs_index(
    octaspire_dern_lexer_token_t const * const self)
{
    return self->ucsIndex;
}

bool octaspire_dern_lexer_token_is_equal(
    octaspire_dern_lexer_token_t const * const self,
    octaspire_dern_lexer_token_t const * const other)
{
    if (self->allocator != other->allocator)
    {
        return false;
    }

    if (!octaspire_dern_lexer_token_position_is_equal(self->line, other->line))
    {
        return false;
    }

    if (!octaspire_dern_lexer_token_position_is_equal(self->column, other->column))
    {
        return false;
    }

    if (!octaspire_dern_lexer_token_position_is_equal(self->ucsIndex, other->ucsIndex))
    {
        return false;
    }

    if (self->typeTag != other->typeTag)
    {
        return false;
    }

    switch (self->typeTag)
    {
        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_STRING:
        {
            return octaspire_string_is_equal(
                self->value.string,
                other->value.string);
        }

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_CHARACTER:
        {
            return octaspire_string_is_equal(
                self->value.character,
                other->value.character);
        }

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_SYMBOL:
        {
            return octaspire_string_is_equal(
                self->value.symbol,
                other->value.symbol);
        }

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR:
        {
            return octaspire_string_is_equal(self->value.error, other->value.error);
        }

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_MORE_INPUT_REQUIRED:
        {
            return octaspire_string_is_equal(
                self->value.moreInputRequired,
                other->value.moreInputRequired);
        }

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_INTEGER:
        {
            return self->value.integer == other->value.integer;
        }

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_REAL:
        {
            // To prevent clang compiler warning on level
            // -Weverything without using #pragma
            return (self->value.real >= other->value.real) &&
                   (self->value.real <= other->value.real);
        }

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_MULTILINE_COMMENT:
        {
            return octaspire_string_is_equal(
                self->value.comment,
                other->value.comment);
        }

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_SEMVER:
        {
            return octaspire_semver_is_equal_to(
                self->value.semver,
                other->value.semver);
        }

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_LPAREN:
        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_RPAREN:
        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_QUOTE:
        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_BACK_QUOTE:
        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_TRUE:
        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_FALSE:
        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_NIL:
        {
            return true;
        }
    }

    abort();
    return false;
}

octaspire_string_t *octaspire_dern_lexer_token_to_string(
    octaspire_dern_lexer_token_t const * const self)
{
    assert(self);

    octaspire_string_t *result = octaspire_string_new_format(
        (void*)self->allocator,
        "token: line=%zu,%zu column=%zu,%zu ucsIndex=%zu,%zu type=%s value=",
        self->line->start, self->line->end,
        self->column->start, self->column->end,
        self->ucsIndex->start, self->ucsIndex->end,
        octaspire_dern_lexer_token_get_type_tag_as_c_string(self));

    switch (self->typeTag)
    {
        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_LPAREN:
        {
            if (!octaspire_string_concatenate_c_string(result, "left parenthesis"))
            {
                return 0;
            }

            return result;
        }

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_RPAREN:
        {
            if (!octaspire_string_concatenate_c_string(result, "right parenthesis"))
            {
                return 0;
            }

            return result;
        }

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_QUOTE:
        {
            if (!octaspire_string_concatenate_c_string(result, "quote"))
            {
                return 0;
            }

            return result;
        }

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_BACK_QUOTE:
        {
            if (!octaspire_string_concatenate_c_string(result, "back quote"))
            {
                return 0;
            }

            return result;
        }

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_TRUE:
        {
            if (!octaspire_string_concatenate_c_string(result, "true"))
            {
                return 0;
            }

            return result;
        }

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_FALSE:
        {
            if (!octaspire_string_concatenate_c_string(result, "false"))
            {
                return 0;
            }

            return result;
        }

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_NIL:
        {
            if (!octaspire_string_concatenate_c_string(result, "nil"))
            {
                return 0;
            }

            return result;
        }

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_INTEGER:
        {
            if (!octaspire_string_concatenate_format(
                result,
                "integer %" PRId32 "",
                self->value.integer))
            {
                return 0;
            }

            return result;
        }

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_REAL:
        {
            if (!octaspire_string_concatenate_format(
                result,
                "real %g",
                self->value.real))
            {
                return 0;
            }

            return result;
        }

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_STRING:
        {
            if (!octaspire_string_concatenate_format(
                result,
                "[%s]",
                octaspire_string_get_c_string(self->value.string)))
            {
                return 0;
            }

            return result;
        }

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_CHARACTER:
        {
            if (octaspire_string_is_equal_to_c_string(self->value.character, "|"))
            {
                if (!octaspire_string_concatenate_c_string(result, "|bar|"))
                {
                    return 0;
                }
            }
            else if (octaspire_string_is_equal_to_c_string(
                self->value.character,
                "\n"))
            {
                if (!octaspire_string_concatenate_c_string(result, "|newline|"))
                {
                    return 0;
                }
            }
            else if (octaspire_string_is_equal_to_c_string(
                self->value.character,
                "\t"))
            {
                if (!octaspire_string_concatenate_c_string(result, "|tab|"))
                {
                    return 0;
                }
            }
            else if (octaspire_string_is_equal_to_c_string(
                self->value.character,
                "["))
            {
                if (!octaspire_string_concatenate_c_string(result, "|string-start|"))
                {
                    return 0;
                }
            }
            else if (octaspire_string_is_equal_to_c_string(
                self->value.character,
                "]"))
            {
                if (!octaspire_string_concatenate_c_string(result, "|string-end|"))
                {
                    return 0;
                }
            }
            else
            {
                if (!octaspire_string_concatenate_format(
                            result,
                            "|%s|",
                            octaspire_string_get_c_string(self->value.character)))
                {
                    return 0;
                }
            }

            return result;
        }

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_SYMBOL:
        {
            if (!octaspire_string_concatenate_format(
                result,
                "%s",
                octaspire_string_get_c_string(self->value.symbol)))
            {
                return 0;
            }

            return result;
        }

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR:
        {
            if (!octaspire_string_concatenate_format(
                result,
                "error: %s",
                octaspire_string_get_c_string(self->value.error)))
            {
                return 0;
            }

            return result;
        }

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_MULTILINE_COMMENT:
        {
            if (!octaspire_string_concatenate_format(
                result,
                "%s",
                octaspire_string_get_c_string(self->value.comment)))
            {
                return 0;
            }

            return result;
        }

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_SEMVER:
        {
            octaspire_string_t * str =
                octaspire_semver_to_string(self->value.semver);

            if (!octaspire_string_concatenate_format(
                result,
                "%s",
                octaspire_string_get_c_string(str)))
            {
                octaspire_string_release(str);
                str = 0;
                return 0;
            }

            octaspire_string_release(str);
            str = 0;
            return result;
        }

        case OCTASPIRE_DERN_LEXER_TOKEN_TAG_MORE_INPUT_REQUIRED:
        {
            if (!octaspire_string_concatenate_format(
                result,
                "more input required: %s",
                octaspire_string_get_c_string(self->value.moreInputRequired)))
            {
                return 0;
            }

            return result;
        }
    }

    if (!octaspire_string_concatenate_format(
        result,
        "unknown token type %i",
        (int)self->typeTag))
    {
        return 0;
    }

    return result;
}

void octaspire_dern_lexer_token_print(
    octaspire_dern_lexer_token_t const * const self)
{
    octaspire_string_t *str = octaspire_dern_lexer_token_to_string(self);
    printf("%s\n", octaspire_string_get_c_string(str));
    octaspire_string_release(str);
    str = 0;
}

void octaspire_dern_lexer_private_pop_whitespace(
    octaspire_input_t *input)
{
    while (octaspire_input_is_good(input))
    {
        uint32_t const c = octaspire_input_peek_next_ucs_character(input);

        if (!isspace((int const)c))
        {
            break;
        }

        if (!octaspire_input_pop_next_ucs_character(input))
        {
            abort();
        }
    }
}

void octaspire_dern_lexer_private_pop_rest_of_line(
    octaspire_input_t *input)
{
    while (octaspire_input_is_good(input))
    {
        if (octaspire_input_peek_next_ucs_character(input) == '\n')
        {
            if (!octaspire_input_pop_next_ucs_character(input))
            {
                abort();
            }

            return;
        }

        if (!octaspire_input_pop_next_ucs_character(input))
        {
            abort();
        }
    }
}

octaspire_dern_lexer_token_t *octaspire_dern_lexer_private_pop_multiline_comment(
    octaspire_input_t *input,
    octaspire_allocator_t *allocator,
    size_t const startLine,
    size_t const startColumn,
    size_t const startIndexInInput)
{
    if (octaspire_input_peek_next_ucs_character(input) != '#')
    {
        size_t const endIndexInInput  = octaspire_input_get_ucs_character_index(input);
        return octaspire_dern_lexer_token_new(
            OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
            "Number sign '#' expected to start multiline comment",
            octaspire_dern_lexer_token_position_init(
                startLine,
                octaspire_input_get_line_number(input)),
            octaspire_dern_lexer_token_position_init(
                startColumn,
                octaspire_input_get_column_number(input)),
            octaspire_dern_lexer_token_position_init(
                startIndexInInput,
                endIndexInInput),
            allocator);
    }

    if (!octaspire_input_pop_next_ucs_character(input))
    {
        abort();
    }

    if (octaspire_input_peek_next_ucs_character(input) != '!')
    {
        size_t const endIndexInInput  = octaspire_input_get_ucs_character_index(input);
        return octaspire_dern_lexer_token_new(
            OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
            "Exclamation mark '!' expected after '#' to start multiline comment",
            octaspire_dern_lexer_token_position_init(
                startLine,
                octaspire_input_get_line_number(input)),
            octaspire_dern_lexer_token_position_init(
                startColumn,
                octaspire_input_get_column_number(input)),
            octaspire_dern_lexer_token_position_init(
                startIndexInInput,
                endIndexInInput),
            allocator);
    }

    if (!octaspire_input_pop_next_ucs_character(input))
    {
        abort();
    }

    octaspire_string_t *commentStr =
        octaspire_string_new("", allocator);

    while (octaspire_input_is_good(input))
    {
        uint32_t currentChar = octaspire_input_peek_next_ucs_character(input);

        if (!octaspire_input_pop_next_ucs_character(input))
        {
            abort();
        }

        if (currentChar == '!')
        {
            if (!octaspire_input_is_good(input))
            {
                octaspire_string_release(commentStr);
                commentStr = 0;

                size_t const endIndexInInput  = octaspire_input_get_ucs_character_index(input) - 1;
                return octaspire_dern_lexer_token_new(
                    OCTASPIRE_DERN_LEXER_TOKEN_TAG_MORE_INPUT_REQUIRED,
                    "Number sign '#' expected after '!' to close multiline comment",
                    octaspire_dern_lexer_token_position_init(
                        startLine,
                        octaspire_input_get_line_number(input)),
                    octaspire_dern_lexer_token_position_init(
                        startColumn,
                        octaspire_input_get_column_number(input)),
                    octaspire_dern_lexer_token_position_init(
                        startIndexInInput,
                        endIndexInInput),
                    allocator);
            }

            currentChar = octaspire_input_peek_next_ucs_character(input);

            if (currentChar == '#')
            {
                if (!octaspire_input_pop_next_ucs_character(input))
                {
                    abort();
                }

                octaspire_string_release(commentStr);
                commentStr = 0;

                // Multiline comment was lexed OK. Return 0 instead of a new
                // lexer token, because comments should not leave the lexer.
                return 0;
            }
            else
            {
                octaspire_string_push_back_ucs_character(commentStr, currentChar);
            }
        }
        else
        {
            octaspire_string_push_back_ucs_character(commentStr, currentChar);
        }
    }

    octaspire_string_release(commentStr);
    commentStr = 0;

    size_t const endIndexInInput  = octaspire_input_get_ucs_character_index(input) - 1;
    return octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_MORE_INPUT_REQUIRED,
        "Multiline comment that is not closed with !#",
        octaspire_dern_lexer_token_position_init(
            startLine,
            octaspire_input_get_line_number(input)),
        octaspire_dern_lexer_token_position_init(
            startColumn,
            octaspire_input_get_column_number(input)),
        octaspire_dern_lexer_token_position_init(
            startIndexInInput,
            endIndexInInput),
        allocator);
}

bool octaspire_dern_lexer_private_is_delimeter(uint32_t const c)
{
    return (
        isspace(c) ||
        c == '|'   ||
        c == '['   ||
        c == ']'   ||
        c == '('   ||
        c == ')'   ||
        c == '\''  ||
        c == '\n'
    );
}

octaspire_dern_lexer_token_t *octaspire_dern_lexer_private_pop_left_parenthesis(
    octaspire_input_t *input,
    octaspire_allocator_t *allocator,
    size_t const startLine,
    size_t const startColumn,
    size_t const startIndexInInput)
{
    size_t   endIndexInInput = startIndexInInput;

    if (octaspire_input_is_good(input))
    {
        endIndexInInput  = octaspire_input_get_ucs_character_index(input);
        uint32_t const c = octaspire_input_peek_next_ucs_character(input);

        if (c == '(')
        {
            octaspire_dern_lexer_token_t * result = octaspire_dern_lexer_token_new(
                OCTASPIRE_DERN_LEXER_TOKEN_TAG_LPAREN,
                0,
                octaspire_dern_lexer_token_position_init(
                    startLine,
                    octaspire_input_get_line_number(input)),
                octaspire_dern_lexer_token_position_init(
                    startColumn,
                    octaspire_input_get_column_number(input)),
                octaspire_dern_lexer_token_position_init(
                    startIndexInInput,
                    endIndexInInput),
                allocator);

            if (!octaspire_input_pop_next_ucs_character(input))
            {
                abort();
            }

            return result;
        }
        else
        {
            return octaspire_dern_lexer_token_new(
                OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
                "Left parenthesis '(' expected",
                octaspire_dern_lexer_token_position_init(
                    startLine,
                    octaspire_input_get_line_number(input)),
                octaspire_dern_lexer_token_position_init(
                    startColumn,
                    octaspire_input_get_column_number(input)),
                octaspire_dern_lexer_token_position_init(
                    startIndexInInput,
                    endIndexInInput),
                allocator);
        }
    }
    else
    {
        return octaspire_dern_lexer_token_new(
            OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
            "Left parenthesis '(' expected",
            octaspire_dern_lexer_token_position_init(
                startLine,
                octaspire_input_get_line_number(input)),
            octaspire_dern_lexer_token_position_init(
                startColumn,
                octaspire_input_get_column_number(input)),
            octaspire_dern_lexer_token_position_init(
                startIndexInInput,
                endIndexInInput),
            allocator);
    }
}

octaspire_dern_lexer_token_t *octaspire_dern_lexer_private_pop_right_parenthesis(
    octaspire_input_t *input,
    octaspire_allocator_t *allocator,
    size_t const startLine,
    size_t const startColumn,
    size_t const startIndexInInput)
{
    size_t   endIndexInInput = startIndexInInput;

    if (octaspire_input_is_good(input))
    {
        endIndexInInput  = octaspire_input_get_ucs_character_index(input);
        uint32_t const c = octaspire_input_peek_next_ucs_character(input);

        if (c == ')')
        {
            octaspire_dern_lexer_token_t *result = octaspire_dern_lexer_token_new(
                OCTASPIRE_DERN_LEXER_TOKEN_TAG_RPAREN,
                0,
                octaspire_dern_lexer_token_position_init(
                    startLine,
                    octaspire_input_get_line_number(input)),
                octaspire_dern_lexer_token_position_init(
                    startColumn,
                    octaspire_input_get_column_number(input)),
                octaspire_dern_lexer_token_position_init(
                    startIndexInInput,
                    endIndexInInput),
                allocator);

            if (!octaspire_input_pop_next_ucs_character(input))
            {
                abort();
            }

            return result;
        }
        else
        {
            return octaspire_dern_lexer_token_new(
                OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
                "Right parenthesis ')' expected",
                octaspire_dern_lexer_token_position_init(
                    startLine,
                    octaspire_input_get_line_number(input)),
                octaspire_dern_lexer_token_position_init(
                    startColumn,
                    octaspire_input_get_column_number(input)),
                octaspire_dern_lexer_token_position_init(
                    startIndexInInput,
                    endIndexInInput),
                allocator);
        }
    }
    else
    {
        return octaspire_dern_lexer_token_new(
            OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
            "Right parenthesis ')' expected",
            octaspire_dern_lexer_token_position_init(
                startLine,
                octaspire_input_get_line_number(input)),
            octaspire_dern_lexer_token_position_init(
                startColumn,
                octaspire_input_get_column_number(input)),
            octaspire_dern_lexer_token_position_init(
                startIndexInInput,
                endIndexInInput),
            allocator);
    }
}

octaspire_dern_lexer_token_t *octaspire_dern_lexer_private_pop_quote(
    octaspire_input_t *input,
    octaspire_allocator_t *allocator,
    size_t const startLine,
    size_t const startColumn,
    size_t const startIndexInInput)
{
    size_t   endIndexInInput = startIndexInInput;

    if (octaspire_input_is_good(input))
    {
        endIndexInInput  = octaspire_input_get_ucs_character_index(input);
        uint32_t const c = octaspire_input_peek_next_ucs_character(input);

        if (c == '\'')
        {
            octaspire_dern_lexer_token_t *result = octaspire_dern_lexer_token_new(
                OCTASPIRE_DERN_LEXER_TOKEN_TAG_QUOTE,
                0,
                octaspire_dern_lexer_token_position_init(
                    startLine,
                    octaspire_input_get_line_number(input)),
                octaspire_dern_lexer_token_position_init(
                    startColumn,
                    octaspire_input_get_column_number(input)),
                octaspire_dern_lexer_token_position_init(
                    startIndexInInput,
                    endIndexInInput),
                allocator);

            if (!octaspire_input_pop_next_ucs_character(input))
            {
                abort();
            }

            return result;
        }
        else
        {
            return octaspire_dern_lexer_token_new(
                OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
                "Quote ' expected",
                octaspire_dern_lexer_token_position_init(
                    startLine,
                    octaspire_input_get_line_number(input)),
                octaspire_dern_lexer_token_position_init(
                    startColumn,
                    octaspire_input_get_column_number(input)),
                octaspire_dern_lexer_token_position_init(
                    startIndexInInput,
                    endIndexInInput),
                allocator);
        }
    }
    else
    {
        return octaspire_dern_lexer_token_new(
            OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
            "Quote ' expected",
            octaspire_dern_lexer_token_position_init(
                startLine,
                octaspire_input_get_line_number(input)),
            octaspire_dern_lexer_token_position_init(
                startColumn,
                octaspire_input_get_column_number(input)),
            octaspire_dern_lexer_token_position_init(
                startIndexInInput,
                endIndexInInput),
            allocator);
    }
}

octaspire_dern_lexer_token_t *octaspire_dern_lexer_private_pop_back_quote(
    octaspire_input_t *input,
    octaspire_allocator_t *allocator,
    size_t const startLine,
    size_t const startColumn,
    size_t const startIndexInInput)
{
    size_t endIndexInInput = startIndexInInput;

    if (octaspire_input_is_good(input))
    {
        endIndexInInput  = octaspire_input_get_ucs_character_index(input);
        uint32_t const c = octaspire_input_peek_next_ucs_character(input);

        if (c == '`')
        {
            octaspire_dern_lexer_token_t *result = octaspire_dern_lexer_token_new(
                OCTASPIRE_DERN_LEXER_TOKEN_TAG_BACK_QUOTE,
                0,
                octaspire_dern_lexer_token_position_init(
                    startLine,
                    octaspire_input_get_line_number(input)),
                octaspire_dern_lexer_token_position_init(
                    startColumn,
                    octaspire_input_get_column_number(input)),
                octaspire_dern_lexer_token_position_init(
                    startIndexInInput,
                    endIndexInInput),
                allocator);

            if (!octaspire_input_pop_next_ucs_character(input))
            {
                abort();
            }

            return result;
        }
        else
        {
            return octaspire_dern_lexer_token_new(
                OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
                "Back quote ` expected",
                octaspire_dern_lexer_token_position_init(
                    startLine,
                    octaspire_input_get_line_number(input)),
                octaspire_dern_lexer_token_position_init(
                    startColumn,
                    octaspire_input_get_column_number(input)),
                octaspire_dern_lexer_token_position_init(
                    startIndexInInput,
                    endIndexInInput),
                allocator);
        }
    }
    else
    {
        return octaspire_dern_lexer_token_new(
            OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
            "Back quote ` expected",
            octaspire_dern_lexer_token_position_init(
                startLine,
                octaspire_input_get_line_number(input)),
            octaspire_dern_lexer_token_position_init(
                startColumn,
                octaspire_input_get_column_number(input)),
            octaspire_dern_lexer_token_position_init(
                startIndexInInput,
                endIndexInInput),
            allocator);
    }
}

static octaspire_dern_lexer_token_t *octaspire_dern_lexer_private_expect_octet(
    octaspire_input_t *input,
    octaspire_allocator_t *allocator,
    size_t const startLine,
    size_t const startColumn,
    size_t const startIndexInInput,
    char const * const sourceName,
    char const * const expected,
    uint32_t * octetRead)
{
    size_t   endIndexInInput = startIndexInInput;

    // Read one octet.
    if (octaspire_input_is_good(input))
    {
        endIndexInInput  = octaspire_input_get_ucs_character_index(input);
        uint32_t const c = octaspire_input_peek_next_ucs_character(input);

        if (!strchr(expected, c))
        {
            return octaspire_dern_lexer_token_new_format(
                OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
                octaspire_dern_lexer_token_position_init(
                    startLine,
                    octaspire_input_get_line_number(input)),
                octaspire_dern_lexer_token_position_init(
                    startColumn,
                    octaspire_input_get_column_number(input)),
                octaspire_dern_lexer_token_position_init(startIndexInInput, endIndexInInput),
                allocator,
                "%s: one of octets '%s' expected. Instead '%c' was found.",
                sourceName,
                expected,
                (char)c);
        }

        *octetRead = c;

        if (!octaspire_input_pop_next_ucs_character(input))
        {
            abort();
        }

        return 0;
    }
    else
    {
        return octaspire_dern_lexer_token_new_format(
            OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
            octaspire_dern_lexer_token_position_init(
                startLine,
                octaspire_input_get_line_number(input)),
            octaspire_dern_lexer_token_position_init(
                startColumn,
                octaspire_input_get_column_number(input)),
            octaspire_dern_lexer_token_position_init(startIndexInInput, endIndexInInput),
            allocator,
            "%s: one of octets '%s' expected",
            sourceName,
            expected);
    }
}

static size_t octaspire_dern_lexer_private_expect_semver_number(
    octaspire_input_t *input,
    octaspire_allocator_t *allocator,
    size_t const startLine,
    size_t const startColumn,
    size_t const startIndexInInput,
    char const * const numberName,
    size_t * result)
{
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(allocator);
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(startLine);
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(startColumn);
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(startIndexInInput);
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(numberName);

    char digits[256]       = {'\0'};
    size_t nextDigitIndex  = 0;

    while (octaspire_input_is_good(input))
    {
        uint32_t const c = octaspire_input_peek_next_ucs_character(input);

        if (isdigit((int const)c))
        {
            if (nextDigitIndex >= 256)
            {
                abort();
            }

            digits[nextDigitIndex] = c;
            ++nextDigitIndex;
        }
        else
        {
            *result = 0;
            for (size_t i = 0; i < nextDigitIndex; ++i)
            {
                char const c = digits[nextDigitIndex - 1 - i];
                *result += (size_t)(pow(10, i) * (c - '0'));
            }

            return nextDigitIndex;
        }

        if (!octaspire_input_pop_next_ucs_character(input))
        {
            abort();
        }
    }

    *result = 0;
    for (size_t i = 0; i < nextDigitIndex; ++i)
    {
        char const c = digits[nextDigitIndex - 1 - i];
        *result += (size_t)(pow(10, i) * (c - '0'));
    }

    return nextDigitIndex;
}

octaspire_dern_lexer_token_t *octaspire_dern_lexer_private_pop_integer_or_real_number(
    octaspire_input_t *input,
    octaspire_allocator_t *allocator,
    size_t const startLine,
    size_t const startColumn,
    size_t const startIndexInInput)
{
    size_t   charsRead       = 0;
    bool     dotRead         = false;

    size_t   endIndexInInput = startIndexInInput;
    size_t   endColumn       = startColumn;

    double   factor          = 1;
    size_t   base            = 10;
    uint32_t prevChar        = 0;

    char digits[256]         = {'\0'};
    size_t nextDigitIndex    = 0;

    uint32_t octetRead       = 0;

    // Read '{'
    octaspire_dern_lexer_token_t * potentialError =
        octaspire_dern_lexer_private_expect_octet(
            input,
            allocator,
            startLine,
            startColumn,
            startIndexInInput,
            "Number",
            "{",
            &octetRead);

    if (potentialError)
    {
        return potentialError;
    }

    // Read 'X', 'D','O' or 'B'
    potentialError =
        octaspire_dern_lexer_private_expect_octet(
            input,
            allocator,
            startLine,
            startColumn,
            startIndexInInput,
            "Number",
            "XDOB",
            &octetRead);

    if (potentialError)
    {
        return potentialError;
    }

    switch (octetRead)
    {
        case 'X': { base = 16; } break;
        case 'D': { base = 10; } break;
        case 'O': { base =  8; } break;
        case 'B': { base =  2; } break;
    }

    // Read '+' or '-'
    potentialError =
        octaspire_dern_lexer_private_expect_octet(
            input,
            allocator,
            startLine,
            startColumn,
            startIndexInInput,
            "Number",
            "+-",
            &octetRead);

    if (potentialError)
    {
        return potentialError;
    }

    factor = (octetRead == '-') ? -1 : 1;

    while (octaspire_input_is_good(input))
    {
        // Spaces can be used to make the number more readable,
        // for example {D+100 000 000}.
        octaspire_dern_lexer_private_pop_whitespace(input);

        endIndexInInput  = octaspire_input_get_ucs_character_index(input);
        uint32_t const c = octaspire_input_peek_next_ucs_character(input);

        if (c == '.')
        {
            if (dotRead)
            {
                return octaspire_dern_lexer_token_new(
                    OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
                    "Number can contain only one '.' character",
                    octaspire_dern_lexer_token_position_init(
                        startLine,
                        octaspire_input_get_line_number(input)),
                    octaspire_dern_lexer_token_position_init(
                        startColumn,
                        octaspire_input_get_column_number(input)),
                    octaspire_dern_lexer_token_position_init(
                        startIndexInInput,
                        endIndexInInput),
                    allocator);
            }

            if (charsRead == 0)
            {
                return octaspire_dern_lexer_token_new(
                    OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
                    "Character '.' cannot start a number",
                    octaspire_dern_lexer_token_position_init(
                        startLine,
                        octaspire_input_get_line_number(input)),
                    octaspire_dern_lexer_token_position_init(
                        startColumn,
                        octaspire_input_get_column_number(input)),
                    octaspire_dern_lexer_token_position_init(
                        startIndexInInput,
                        endIndexInInput),
                    allocator);
            }

            dotRead = true;
        }
        else if (isxdigit((int const)c))
        {
            if (dotRead)
            {
                factor /= base;
            }

            if (nextDigitIndex >= 256)
            {
                abort();
            }

            if (base == 2)
            {
                if (c != '0' && c != '1')
                {
                    return octaspire_dern_lexer_token_new(
                        OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
                        "Binary number can contain only '0' and '1' digits.",
                        octaspire_dern_lexer_token_position_init(
                            startLine,
                            octaspire_input_get_line_number(input)),
                        octaspire_dern_lexer_token_position_init(
                            startColumn,
                            octaspire_input_get_column_number(input)),
                        octaspire_dern_lexer_token_position_init(
                            startIndexInInput,
                            endIndexInInput),
                        allocator);
                }
            }
            else if (base == 8)
            {
                if (c != '0' && c != '1' && c != '2' && c != '3' && c != '4' &&
                    c != '5' && c != '6' && c != '7')
                {
                    return octaspire_dern_lexer_token_new(
                        OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
                        "Octal number can contain only digits '0' - '7'.",
                        octaspire_dern_lexer_token_position_init(
                            startLine,
                            octaspire_input_get_line_number(input)),
                        octaspire_dern_lexer_token_position_init(
                            startColumn,
                            octaspire_input_get_column_number(input)),
                        octaspire_dern_lexer_token_position_init(
                            startIndexInInput,
                            endIndexInInput),
                        allocator);
                }
            }
            else if (base == 10)
            {
                if (c != '0' && c != '1' && c != '2' && c != '3' && c != '4' &&
                    c != '5' && c != '6' && c != '7' && c != '8' && c != '9')
                {
                    return octaspire_dern_lexer_token_new(
                        OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
                        "Decimal number can contain only digits '0' - '9'.",
                        octaspire_dern_lexer_token_position_init(
                            startLine,
                            octaspire_input_get_line_number(input)),
                        octaspire_dern_lexer_token_position_init(
                            startColumn,
                            octaspire_input_get_column_number(input)),
                        octaspire_dern_lexer_token_position_init(
                            startIndexInInput,
                            endIndexInInput),
                        allocator);
                }
            }

            digits[nextDigitIndex] = c;
            ++nextDigitIndex;
        }
        else if (c == '}')
        {
            endColumn = octaspire_input_get_column_number(input);

            if (!octaspire_input_pop_next_ucs_character(input))
            {
                abort();
            }

            ++charsRead;
            break;
        }
        else
        {
            return octaspire_dern_lexer_token_new_format(
                OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
                octaspire_dern_lexer_token_position_init(
                    startLine,
                    octaspire_input_get_line_number(input)),
                octaspire_dern_lexer_token_position_init(
                    startColumn,
                    octaspire_input_get_column_number(input)),
                octaspire_dern_lexer_token_position_init(
                    startIndexInInput,
                    endIndexInInput),
                allocator,
                "Number cannot contain character '%c'",
                c);
        }

        if (!octaspire_input_pop_next_ucs_character(input))
        {
            abort();
        }

        ++charsRead;

        if (c != ' ')
        {
            // ' ' could hide case where number has . or - as
            // last non-whitespace octet.
            prevChar = c;
        }
    }

    if (prevChar == '.')
    {
        return octaspire_dern_lexer_token_new(
                OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
                "Character '.' cannot end a number",
                octaspire_dern_lexer_token_position_init(
                    startLine,
                    octaspire_input_get_line_number(input)),
                octaspire_dern_lexer_token_position_init(
                    startColumn,
                    endColumn),
                octaspire_dern_lexer_token_position_init(
                    startIndexInInput,
                    endIndexInInput),
                allocator);
    }

    if (prevChar == '-')
    {
        return octaspire_dern_lexer_token_new(
                OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
                "Character '-' cannot end a number",
                octaspire_dern_lexer_token_position_init(
                    startLine,
                    octaspire_input_get_line_number(input)),
                octaspire_dern_lexer_token_position_init(
                    startColumn,
                    endColumn),
                octaspire_dern_lexer_token_position_init(
                    startIndexInInput,
                    endIndexInInput),
                allocator);
    }

    size_t value = 0;

    for (size_t i = 0; i < nextDigitIndex; ++i)
    {
        char const c = digits[nextDigitIndex - 1 - i];

        if (c >= '0' && c <= '9')
        {
            value += (size_t)(pow(base, i) * (c - '0'));
        }
        else
        {
            value += (size_t)(pow(base, i) * (10 + (tolower(c) - 'a')));
        }
    }

    if (dotRead)
    {
        double const resultValue = (double)value * factor;

        return octaspire_dern_lexer_token_new(
            OCTASPIRE_DERN_LEXER_TOKEN_TAG_REAL,
            &resultValue,
            octaspire_dern_lexer_token_position_init(
                startLine,
                octaspire_input_get_line_number(input)),
            octaspire_dern_lexer_token_position_init(
                startColumn,
                endColumn),
            octaspire_dern_lexer_token_position_init(
                startIndexInInput,
                endIndexInInput),
            allocator);
    }

    int32_t const resultValue = (int32_t)((int32_t)value * factor);

    return octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_INTEGER,
        &resultValue,
        octaspire_dern_lexer_token_position_init(
            startLine,
            octaspire_input_get_line_number(input)),
        octaspire_dern_lexer_token_position_init(
            startColumn,
            endColumn),
        octaspire_dern_lexer_token_position_init(
            startIndexInInput,
            endIndexInInput),
        allocator);
}

octaspire_dern_lexer_token_t *octaspire_dern_lexer_private_pop_semver(
    octaspire_input_t *input,
    octaspire_allocator_t *allocator,
    size_t const startLine,
    size_t const startColumn,
    size_t const startIndexInInput)
{
    size_t   endIndexInInput = startIndexInInput;
    size_t   endColumn       = startColumn;

    uint32_t octetRead       = 0;

    size_t   major           = 0;
    size_t   minor           = 0;
    size_t   patch           = 0;

    octaspire_vector_t * preRelease = octaspire_vector_new(
        sizeof(octaspire_semver_pre_release_elem_t*),
        true,
        (octaspire_vector_element_callback_t)octaspire_semver_pre_release_elem_release,
        allocator);

    if (!preRelease)
    {
        abort();
    }

    octaspire_vector_t * buildMetadata = octaspire_vector_new(
        sizeof(octaspire_string_t*),
        true,
        (octaspire_vector_element_callback_t)octaspire_string_release,
        allocator);

    if (!buildMetadata)
    {
        abort();
    }

    // Read major version number.
    if (!octaspire_dern_lexer_private_expect_semver_number(
        input,
        allocator,
        startLine,
        startColumn,
        startIndexInInput,
        "Major",
        &major))
    {
        octaspire_vector_release(preRelease);
        preRelease = 0;

        octaspire_vector_release(buildMetadata);
        buildMetadata= 0;

        return octaspire_dern_lexer_token_new(
            OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
            "Major component of semantic version number cannot be empty",
            octaspire_dern_lexer_token_position_init(
                startLine,
                octaspire_input_get_line_number(input)),
            octaspire_dern_lexer_token_position_init(
                startColumn,
                octaspire_input_get_column_number(input)),
            octaspire_dern_lexer_token_position_init(startIndexInInput, endIndexInInput),
            allocator);
    }

    // Read '.'
    octaspire_dern_lexer_token_t * potentialError =
        octaspire_dern_lexer_private_expect_octet(
            input,
            allocator,
            startLine,
            startColumn,
            startIndexInInput,
            "SemVer: after major component",
            ".",
            &octetRead);

    if (potentialError)
    {
        octaspire_vector_release(preRelease);
        preRelease = 0;

        octaspire_vector_release(buildMetadata);
        buildMetadata= 0;

        return potentialError;
    }

    // Read minor version number.
    if (!octaspire_dern_lexer_private_expect_semver_number(
        input,
        allocator,
        startLine,
        startColumn,
        startIndexInInput,
        "Minor",
        &minor))
    {
        octaspire_vector_release(preRelease);
        preRelease = 0;

        octaspire_vector_release(buildMetadata);
        buildMetadata= 0;

        return octaspire_dern_lexer_token_new(
            OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
            "Minor component of semantic version number cannot be empty",
            octaspire_dern_lexer_token_position_init(
                startLine,
                octaspire_input_get_line_number(input)),
            octaspire_dern_lexer_token_position_init(
                startColumn,
                octaspire_input_get_column_number(input)),
            octaspire_dern_lexer_token_position_init(startIndexInInput, endIndexInInput),
            allocator);
    }

    // Read '.'
    potentialError =
        octaspire_dern_lexer_private_expect_octet(
            input,
            allocator,
            startLine,
            startColumn,
            startIndexInInput,
            "SemVer: after minor component",
            ".",
            &octetRead);

    if (potentialError)
    {
        octaspire_vector_release(preRelease);
        preRelease = 0;

        octaspire_vector_release(buildMetadata);
        buildMetadata= 0;

        return potentialError;
    }

    // Read patch version number.
    if (!octaspire_dern_lexer_private_expect_semver_number(
        input,
        allocator,
        startLine,
        startColumn,
        startIndexInInput,
        "Patch",
        &patch))
    {
        octaspire_vector_release(preRelease);
        preRelease = 0;

        octaspire_vector_release(buildMetadata);
        buildMetadata= 0;

        return octaspire_dern_lexer_token_new(
            OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
            "Patch component of semantic version number cannot be empty",
            octaspire_dern_lexer_token_position_init(
                startLine,
                octaspire_input_get_line_number(input)),
            octaspire_dern_lexer_token_position_init(
                startColumn,
                octaspire_input_get_column_number(input)),
            octaspire_dern_lexer_token_position_init(startIndexInInput, endIndexInInput),
            allocator);
    }

    octaspire_semver_t * semver =
        octaspire_semver_new(
            major,
            minor,
            patch,
            preRelease,
            buildMetadata,
            allocator);

    octaspire_helpers_verify_not_null(semver);

    octaspire_vector_release(preRelease);
    preRelease = 0;

    octaspire_vector_release(buildMetadata);
    buildMetadata= 0;

    octaspire_dern_lexer_token_t * const result =
        octaspire_dern_lexer_token_new(
            OCTASPIRE_DERN_LEXER_TOKEN_TAG_SEMVER,
            semver,
            octaspire_dern_lexer_token_position_init(
                startLine,
                octaspire_input_get_line_number(input)),
            octaspire_dern_lexer_token_position_init(
                startColumn,
                endColumn),
            octaspire_dern_lexer_token_position_init(
                startIndexInInput,
                endIndexInInput),
            allocator);

    octaspire_semver_release(semver);
    semver = 0;
    return result;
}

octaspire_dern_lexer_token_t *octaspire_dern_lexer_private_pop_string(
    octaspire_input_t *input,
    octaspire_allocator_t *allocator,
    size_t const startLine,
    size_t const startColumn,
    size_t const startIndexInInput)
{
    size_t   charsRead       = 0;
    bool     lastDelimiterRead = false;
    bool     endsInDelimiter = false;

    size_t   endIndexInInput = startIndexInInput;

    octaspire_string_t *tmpStr =
        octaspire_string_new("", allocator);

    if (!tmpStr)
    {
        return octaspire_dern_lexer_token_new(
            OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
            "Memory allocation failed",
            octaspire_dern_lexer_token_position_init(
                startLine,
                octaspire_input_get_line_number(input)),
            octaspire_dern_lexer_token_position_init(
                startColumn,
                octaspire_input_get_column_number(input)),
            octaspire_dern_lexer_token_position_init(startIndexInInput, endIndexInInput),
            allocator);
    }

    while (octaspire_input_is_good(input))
    {
        endIndexInInput  = octaspire_input_get_ucs_character_index(input);
        uint32_t c = octaspire_input_peek_next_ucs_character(input);

        if (lastDelimiterRead)
        {
            if (octaspire_dern_lexer_private_is_delimeter(c))
            {
                --endIndexInInput;
                endsInDelimiter = true;
                break;
            }
            else
            {
                octaspire_string_release(tmpStr);
                tmpStr = 0;

                return octaspire_dern_lexer_token_new(
                    OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
                    "After last ']' of string there must be dern delimiter",
                    octaspire_dern_lexer_token_position_init(
                        startLine,
                        octaspire_input_get_line_number(input)),
                    octaspire_dern_lexer_token_position_init(
                        startColumn,
                        octaspire_input_get_column_number(input)),
                    octaspire_dern_lexer_token_position_init(startIndexInInput, endIndexInInput),
                    allocator);
            }
        }

        if (charsRead == 0)
        {
            if (c == '[')
            {
            }
            else
            {
                octaspire_string_release(tmpStr);
                tmpStr = 0;

                return octaspire_dern_lexer_token_new(
                    OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
                    "String must begin with character '['",
                    octaspire_dern_lexer_token_position_init(
                        startLine,
                        octaspire_input_get_line_number(input)),
                    octaspire_dern_lexer_token_position_init(
                        startColumn,
                        octaspire_input_get_column_number(input)),
                    octaspire_dern_lexer_token_position_init(startIndexInInput, endIndexInInput),
                    allocator);
            }
        }
        else
        {
            if (c == ']')
            {
                lastDelimiterRead = true;
            }
            else if (c == '|')
            {
                octaspire_dern_lexer_token_t *charToken =
                    octaspire_dern_lexer_private_pop_character(
                        input,
                        allocator,
                        startLine,
                        startColumn,
                        startIndexInInput);

                if (!charToken)
                {
                    octaspire_string_release(tmpStr);
                    tmpStr = 0;

                    return octaspire_dern_lexer_token_new(
                        OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
                        "Problem with a character embedded in string: (character is not complete)",
                        octaspire_dern_lexer_token_position_init(
                            startLine,
                            octaspire_input_get_line_number(input)),
                        octaspire_dern_lexer_token_position_init(
                            startColumn, octaspire_input_get_column_number(input)),
                        octaspire_dern_lexer_token_position_init(
                            startIndexInInput,
                            endIndexInInput),
                        allocator);
                }

                if (octaspire_dern_lexer_token_get_type_tag(charToken) ==
                    OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR)
                {
                    octaspire_string_release(tmpStr);
                    tmpStr = 0;

                    octaspire_dern_lexer_token_t *result = octaspire_dern_lexer_token_new_format(
                        OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
                        octaspire_dern_lexer_token_position_init(
                            startLine,
                            octaspire_input_get_line_number(input)),
                        octaspire_dern_lexer_token_position_init(
                            startColumn,
                            octaspire_input_get_column_number(input)),
                        octaspire_dern_lexer_token_position_init(
                            startIndexInInput,
                            endIndexInInput),
                        allocator,
                        "Problem with a character embedded in string: (%s)",
                        octaspire_dern_lexer_token_get_error_value_as_c_string(charToken));

                    octaspire_dern_lexer_token_release(charToken);
                    charToken = 0;

                    return result;
                }

                // TODO check that char available
                c = octaspire_string_get_ucs_character_at_index(
                    charToken->value.character,
                    0);

                if (!octaspire_string_push_back_ucs_character(tmpStr, c))
                {
                    octaspire_string_release(tmpStr);
                    tmpStr = 0;

                    octaspire_dern_lexer_token_release(charToken);
                    charToken = 0;

                    return octaspire_dern_lexer_token_new(
                        OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
                        "Memory allocation failed",
                        octaspire_dern_lexer_token_position_init(
                            startLine,
                            octaspire_input_get_line_number(input)),
                        octaspire_dern_lexer_token_position_init(
                            startColumn,
                            octaspire_input_get_column_number(input)),
                        octaspire_dern_lexer_token_position_init(
                            startIndexInInput,
                            endIndexInInput),
                        allocator);
                }

                octaspire_dern_lexer_token_release(charToken);
                charToken = 0;
                // TODO should charsRead be incremented by some amount? (length of embedded string,
                // including the ||, etc.)
                goto loopEnd;
            }
            else
            {
                if (!octaspire_string_push_back_ucs_character(tmpStr, c))
                {
                    octaspire_string_release(tmpStr);
                    tmpStr = 0;

                    return octaspire_dern_lexer_token_new(
                        OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
                        "Memory allocation failed",
                        octaspire_dern_lexer_token_position_init(
                            startLine,
                            octaspire_input_get_line_number(input)),
                        octaspire_dern_lexer_token_position_init(
                            startColumn,
                            octaspire_input_get_column_number(input)),
                        octaspire_dern_lexer_token_position_init(
                            startIndexInInput,
                            endIndexInInput),
                        allocator);
                }
            }
        }

        if (!octaspire_input_pop_next_ucs_character(input))
        {
            abort();
        }

loopEnd:
        ++charsRead;
    }

    if (!lastDelimiterRead)
    {
        octaspire_string_release(tmpStr);
        tmpStr = 0;

        if (!octaspire_input_is_good(input))
        {
            return 0; // Out of input
        }

        return octaspire_dern_lexer_token_new(
            OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
            "String must end with character ']'",
            octaspire_dern_lexer_token_position_init(
                startLine,
                octaspire_input_get_line_number(input)),
            octaspire_dern_lexer_token_position_init(
                startColumn,
                octaspire_input_get_column_number(input)),
            octaspire_dern_lexer_token_position_init(startIndexInInput, endIndexInInput),
            allocator);
    }

    octaspire_dern_lexer_token_t *result = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_STRING,
        octaspire_string_get_c_string(tmpStr),
        octaspire_dern_lexer_token_position_init(
            startLine,
            octaspire_input_get_line_number(input)),
        octaspire_dern_lexer_token_position_init(
            startColumn,
            octaspire_input_get_column_number(input) - (endsInDelimiter ? 1 : 0)),
        octaspire_dern_lexer_token_position_init(
            startIndexInInput,
            endIndexInInput),
        allocator);

    octaspire_string_release(tmpStr);
    tmpStr = 0;

    return result;
}

octaspire_dern_lexer_token_t *octaspire_dern_lexer_private_pop_character(
    octaspire_input_t *input,
    octaspire_allocator_t *allocator,
    size_t const startLine,
    size_t const startColumn,
    size_t const startIndexInInput)
{
    size_t   charsRead       = 0;
    bool     lastDelimiterRead = false;
    bool     endsInDelimiter = false;

    size_t   endIndexInInput = startIndexInInput;

    octaspire_string_t *tmpStr =
        octaspire_string_new("", allocator);

    if (!tmpStr)
    {
        return octaspire_dern_lexer_token_new(
            OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
            "Memory allocation failed",
            octaspire_dern_lexer_token_position_init(
                startLine,
                octaspire_input_get_line_number(input)),
            octaspire_dern_lexer_token_position_init(
                startColumn,
                octaspire_input_get_column_number(input)),
            octaspire_dern_lexer_token_position_init(startIndexInInput, endIndexInInput),
            allocator);
    }

    while (octaspire_input_is_good(input))
    {
        endIndexInInput  = octaspire_input_get_ucs_character_index(input);
        uint32_t const c = octaspire_input_peek_next_ucs_character(input);

        if (lastDelimiterRead)
        {
            if (octaspire_dern_lexer_private_is_delimeter(c))
            {
                --endIndexInInput;
                endsInDelimiter = true;
                break;
            }
            else
            {
                // NOP
                break;
            }
        }

        if (charsRead == 0)
        {
            if (c == '|')
            {
            }
            else
            {
                octaspire_string_release(tmpStr);
                tmpStr = 0;

                return octaspire_dern_lexer_token_new(
                    OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
                    "Character must begin with character '|'",
                    octaspire_dern_lexer_token_position_init(
                        startLine,
                        octaspire_input_get_line_number(input)),
                    octaspire_dern_lexer_token_position_init(
                        startColumn,
                        octaspire_input_get_column_number(input)),
                    octaspire_dern_lexer_token_position_init(startIndexInInput, endIndexInInput),
                    allocator);
            }
        }
        else
        {
            if (c == '|')
            {
                lastDelimiterRead = true;
            }
            else
            {
                if (!octaspire_string_push_back_ucs_character(tmpStr, c))
                {
                    octaspire_string_release(tmpStr);
                    tmpStr = 0;

                    return octaspire_dern_lexer_token_new(
                        OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
                        "Memory allocation failed",
                        octaspire_dern_lexer_token_position_init(
                            startLine,
                            octaspire_input_get_line_number(input)),
                        octaspire_dern_lexer_token_position_init(
                            startColumn,
                            octaspire_input_get_column_number(input)),
                        octaspire_dern_lexer_token_position_init(
                            startIndexInInput,
                            endIndexInInput),
                        allocator);
                }
            }
        }

        if (!octaspire_input_pop_next_ucs_character(input))
        {
            abort();
        }

        ++charsRead;
    }

    if (!lastDelimiterRead)
    {
        octaspire_string_release(tmpStr);
        tmpStr = 0;

        if (!octaspire_input_is_good(input))
        {
            return 0; // Out of input
        }

        return octaspire_dern_lexer_token_new(
            OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
            "Character must end with character '|'",
            octaspire_dern_lexer_token_position_init(
                startLine,
                octaspire_input_get_line_number(input)),
            octaspire_dern_lexer_token_position_init(
                startColumn,
                octaspire_input_get_column_number(input)),
            octaspire_dern_lexer_token_position_init(startIndexInInput, endIndexInInput),
            allocator);
    }

    if (octaspire_string_get_length_in_ucs_characters(tmpStr) > 1)
    {
        if (octaspire_string_is_equal_to_c_string(tmpStr, "bar"))
        {
            octaspire_string_release(tmpStr);
            tmpStr = 0;

            return octaspire_dern_lexer_token_new(
                OCTASPIRE_DERN_LEXER_TOKEN_TAG_CHARACTER,
                "|",
                octaspire_dern_lexer_token_position_init(
                    startLine,
                    octaspire_input_get_line_number(input)),
                octaspire_dern_lexer_token_position_init(
                    startColumn,
                    octaspire_input_get_column_number(input) - (endsInDelimiter ? 1 : 0)),
                octaspire_dern_lexer_token_position_init(
                    startIndexInInput,
                    endIndexInInput),
                allocator);
        }
        else if (octaspire_string_is_equal_to_c_string(tmpStr, "newline"))
        {
            octaspire_string_release(tmpStr);
            tmpStr = 0;

            return octaspire_dern_lexer_token_new(
                OCTASPIRE_DERN_LEXER_TOKEN_TAG_CHARACTER,
                "\n",
                octaspire_dern_lexer_token_position_init(
                    startLine,
                    octaspire_input_get_line_number(input)),
                octaspire_dern_lexer_token_position_init(
                    startColumn,
                    octaspire_input_get_column_number(input) - (endsInDelimiter ? 1 : 0)),
                octaspire_dern_lexer_token_position_init(
                    startIndexInInput,
                    endIndexInInput),
                allocator);
        }
        else if (octaspire_string_is_equal_to_c_string(tmpStr, "tab"))
        {
            octaspire_string_release(tmpStr);
            tmpStr = 0;

            return octaspire_dern_lexer_token_new(
                OCTASPIRE_DERN_LEXER_TOKEN_TAG_CHARACTER,
                "\t",
                octaspire_dern_lexer_token_position_init(
                    startLine,
                    octaspire_input_get_line_number(input)),
                octaspire_dern_lexer_token_position_init(
                    startColumn,
                    octaspire_input_get_column_number(input) - (endsInDelimiter ? 1 : 0)),
                octaspire_dern_lexer_token_position_init(
                    startIndexInInput,
                    endIndexInInput),
                allocator);
        }
        else if (octaspire_string_is_equal_to_c_string(tmpStr, "string-start"))
        {
            octaspire_string_release(tmpStr);
            tmpStr = 0;

            return octaspire_dern_lexer_token_new(
                OCTASPIRE_DERN_LEXER_TOKEN_TAG_CHARACTER,
                "[",
                octaspire_dern_lexer_token_position_init(
                    startLine,
                    octaspire_input_get_line_number(input)),
                octaspire_dern_lexer_token_position_init(
                    startColumn,
                    octaspire_input_get_column_number(input) - (endsInDelimiter ? 1 : 0)),
                octaspire_dern_lexer_token_position_init(
                    startIndexInInput,
                    endIndexInInput),
                allocator);
        }
        else if (octaspire_string_is_equal_to_c_string(tmpStr, "string-end"))
        {
            octaspire_string_release(tmpStr);
            tmpStr = 0;

            return octaspire_dern_lexer_token_new(
                OCTASPIRE_DERN_LEXER_TOKEN_TAG_CHARACTER,
                "]",
                octaspire_dern_lexer_token_position_init(
                    startLine,
                    octaspire_input_get_line_number(input)),
                octaspire_dern_lexer_token_position_init(
                    startColumn,
                    octaspire_input_get_column_number(input) - (endsInDelimiter ? 1 : 0)),
                octaspire_dern_lexer_token_position_init(
                    startIndexInInput,
                    endIndexInInput),
                allocator);
        }
        else if (
            strspn(
                octaspire_string_get_c_string(tmpStr),
                "0123456789abcdefABCDEF") ==
                    octaspire_string_get_length_in_ucs_characters(tmpStr))
        {
            if (octaspire_string_get_length_in_ucs_characters(tmpStr) > 8)
            {
                octaspire_dern_lexer_token_t *result = octaspire_dern_lexer_token_new_format(
                    OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
                    octaspire_dern_lexer_token_position_init(
                        startLine,
                        octaspire_input_get_line_number(input)),
                    octaspire_dern_lexer_token_position_init(
                        startColumn,
                        octaspire_input_get_column_number(input)),
                    octaspire_dern_lexer_token_position_init(startIndexInInput, endIndexInInput),
                    allocator,
                    "Number of hex digits (%zu) in character definition may not be larger "
                    "than eight",
                    octaspire_string_get_length_in_ucs_characters(tmpStr));

                octaspire_string_release(tmpStr);
                tmpStr = 0;

                return result;
            }

            octaspire_string_t *unicodeChar =
                octaspire_string_new("", allocator);

            if (!octaspire_string_push_back_ucs_character(
                unicodeChar,
                (uint32_t)strtol(
                    octaspire_string_get_c_string(tmpStr),
                    0,
                    16)))
            {
                abort();
            }

            if (octaspire_string_get_error_status(unicodeChar) !=
                OCTASPIRE_STRING_ERROR_STATUS_OK)
            {
                abort();
            }

            assert(unicodeChar);

            octaspire_dern_lexer_token_t * result = octaspire_dern_lexer_token_new(
                OCTASPIRE_DERN_LEXER_TOKEN_TAG_CHARACTER,
                octaspire_string_get_c_string(unicodeChar),
                octaspire_dern_lexer_token_position_init(
                    startLine,
                    octaspire_input_get_line_number(input)),
                octaspire_dern_lexer_token_position_init(
                    startColumn,
                    octaspire_input_get_column_number(input) - (endsInDelimiter ? 1 : 0)),
                octaspire_dern_lexer_token_position_init(
                    startIndexInInput,
                    endIndexInInput),
                allocator);

            octaspire_string_release(unicodeChar);
            unicodeChar = 0;

            octaspire_string_release(tmpStr);
            tmpStr = 0;

            return result;
        }
        else
        {
            octaspire_dern_lexer_token_t *result = octaspire_dern_lexer_token_new_format(
                OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
                octaspire_dern_lexer_token_position_init(
                    startLine,
                    octaspire_input_get_line_number(input)),
                octaspire_dern_lexer_token_position_init(
                    startColumn,
                    octaspire_input_get_column_number(input)),
                octaspire_dern_lexer_token_position_init(startIndexInInput, endIndexInInput),
                allocator,
                "Unknown character constant |%s|",
                octaspire_string_get_c_string(tmpStr));

            octaspire_string_release(tmpStr);
            tmpStr = 0;

            return result;
        }
    }

    if (octaspire_string_is_empty(tmpStr))
    {
        octaspire_dern_lexer_token_t *result = octaspire_dern_lexer_token_new(
            OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
            "Character cannot be empty: ||",
            octaspire_dern_lexer_token_position_init(
                startLine,
                octaspire_input_get_line_number(input)),
            octaspire_dern_lexer_token_position_init(
                startColumn,
                octaspire_input_get_column_number(input)),
            octaspire_dern_lexer_token_position_init(startIndexInInput, endIndexInInput),
            allocator);

        octaspire_string_release(tmpStr);
        tmpStr = 0;

        return result;
    }

    octaspire_dern_lexer_token_t *result = octaspire_dern_lexer_token_new(
        OCTASPIRE_DERN_LEXER_TOKEN_TAG_CHARACTER,
        octaspire_string_get_c_string(tmpStr),
        octaspire_dern_lexer_token_position_init(
            startLine,
            octaspire_input_get_line_number(input)),
        octaspire_dern_lexer_token_position_init(
            startColumn,
            octaspire_input_get_column_number(input) - (endsInDelimiter ? 1 : 0)),
        octaspire_dern_lexer_token_position_init(
            startIndexInInput,
            endIndexInInput),
        allocator);

    octaspire_string_release(tmpStr);
    tmpStr = 0;

    return result;
}

octaspire_dern_lexer_token_t *octaspire_dern_lexer_private_pop_true_or_false_or_nil_or_symbol(
    octaspire_input_t *input,
    octaspire_allocator_t *allocator,
    size_t const startLine,
    size_t const startColumn,
    size_t const startIndexInInput)
{
    size_t   charsRead       = 0;
    bool     endsInDelimiter = false;

    size_t   endIndexInInput = startIndexInInput;

    octaspire_string_t *tmpStr =
        octaspire_string_new("", allocator);

    if (!tmpStr)
    {
        return octaspire_dern_lexer_token_new(
            OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
            "Memory allocation failed",
            octaspire_dern_lexer_token_position_init(
                startLine,
                octaspire_input_get_line_number(input)),
            octaspire_dern_lexer_token_position_init(
                startColumn,
                octaspire_input_get_column_number(input)),
            octaspire_dern_lexer_token_position_init(startIndexInInput, endIndexInInput),
            allocator);
    }

    while (octaspire_input_is_good(input))
    {
        endIndexInInput  = octaspire_input_get_ucs_character_index(input);
        uint32_t const c = octaspire_input_peek_next_ucs_character(input);

        if (octaspire_dern_lexer_private_is_delimeter(c))
        {
            --endIndexInInput;
            endsInDelimiter = true;
            break;
        }

        if (!octaspire_string_push_back_ucs_character(tmpStr, c))
        {
            octaspire_string_release(tmpStr);
            tmpStr = 0;

            return octaspire_dern_lexer_token_new(
                OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
                "Memory allocation failed",
                octaspire_dern_lexer_token_position_init(
                    startLine,
                    octaspire_input_get_line_number(input)),
                octaspire_dern_lexer_token_position_init(
                    startColumn,
                    octaspire_input_get_column_number(input)),
                octaspire_dern_lexer_token_position_init(startIndexInInput, endIndexInInput),
                allocator);
        }

        if (!octaspire_input_pop_next_ucs_character(input))
        {
            abort();
        }

        ++charsRead;
    }

    if (octaspire_string_is_empty(tmpStr))
    {
            octaspire_string_release(tmpStr);
            tmpStr = 0;

            return octaspire_dern_lexer_token_new(
                OCTASPIRE_DERN_LEXER_TOKEN_TAG_ERROR,
                "Symbol cannot be empty",
                octaspire_dern_lexer_token_position_init(
                    startLine,
                    octaspire_input_get_line_number(input)),
                octaspire_dern_lexer_token_position_init(
                    startColumn,
                    octaspire_input_get_column_number(input)),
                octaspire_dern_lexer_token_position_init(startIndexInInput, endIndexInInput),
                allocator);
    }

    octaspire_dern_lexer_token_t *result = 0;

    if (octaspire_string_is_equal_to_c_string(tmpStr, "true"))
    {
        result = octaspire_dern_lexer_token_new(
            OCTASPIRE_DERN_LEXER_TOKEN_TAG_TRUE,
            0,
            octaspire_dern_lexer_token_position_init(
                startLine,
                octaspire_input_get_line_number(input)),
            octaspire_dern_lexer_token_position_init(
                startColumn,
                octaspire_input_get_column_number(input) - (endsInDelimiter ? 1 : 0)),
            octaspire_dern_lexer_token_position_init(
                startIndexInInput,
                endIndexInInput),
            allocator);
    }
    else if (octaspire_string_is_equal_to_c_string(tmpStr, "false"))
    {
        result = octaspire_dern_lexer_token_new(
            OCTASPIRE_DERN_LEXER_TOKEN_TAG_FALSE,
            0,
            octaspire_dern_lexer_token_position_init(
                startLine,
                octaspire_input_get_line_number(input)),
            octaspire_dern_lexer_token_position_init(
                startColumn,
                octaspire_input_get_column_number(input) - (endsInDelimiter ? 1 : 0)),
            octaspire_dern_lexer_token_position_init(
                startIndexInInput,
                endIndexInInput),
            allocator);
    }
    else if (octaspire_string_is_equal_to_c_string(tmpStr, "nil"))
    {
        result = octaspire_dern_lexer_token_new(
            OCTASPIRE_DERN_LEXER_TOKEN_TAG_NIL,
            0,
            octaspire_dern_lexer_token_position_init(
                startLine,
                octaspire_input_get_line_number(input)),
            octaspire_dern_lexer_token_position_init(
                startColumn,
                octaspire_input_get_column_number(input) - (endsInDelimiter ? 1 : 0)),
            octaspire_dern_lexer_token_position_init(
                startIndexInInput,
                endIndexInInput),
            allocator);
    }
    else
    {
        result = octaspire_dern_lexer_token_new(
            OCTASPIRE_DERN_LEXER_TOKEN_TAG_SYMBOL,
            octaspire_string_get_c_string(tmpStr),
            octaspire_dern_lexer_token_position_init(
                startLine,
                octaspire_input_get_line_number(input)),
            octaspire_dern_lexer_token_position_init(
                startColumn,
                octaspire_input_get_column_number(input) - (endsInDelimiter ? 1 : 0)),
            octaspire_dern_lexer_token_position_init(
                startIndexInInput,
                endIndexInInput),
            allocator);
    }

    octaspire_string_release(tmpStr);
    tmpStr = 0;

    return result;
}


octaspire_dern_lexer_token_t *octaspire_dern_lexer_pop_next_token(
    octaspire_input_t *input,
    octaspire_allocator_t *allocator)
{
    while (octaspire_input_is_good(input))
    {
        octaspire_dern_lexer_private_pop_whitespace(input);

        if (!octaspire_input_is_good(input))
        {
            return 0;
        }

        size_t const startLine         = octaspire_input_get_line_number(input);
        size_t const startColumn       = octaspire_input_get_column_number(input);
        size_t const startIndexInInput = octaspire_input_get_ucs_character_index(input);

        switch (octaspire_input_peek_next_ucs_character(input))
        {
            case ';':
            {
                octaspire_dern_lexer_private_pop_rest_of_line(input);
                return octaspire_dern_lexer_pop_next_token(input, allocator);
            }
            break;

            case '#':
            {
                switch (octaspire_input_peek_next_next_ucs_character(input))
                {
                    case '!':
                    {
                        octaspire_dern_lexer_token_t * const errorOrNull =
                            octaspire_dern_lexer_private_pop_multiline_comment(
                                input,
                                allocator,
                                startLine,
                                startColumn,
                                startIndexInInput);

                        if (errorOrNull)
                        {
                            return errorOrNull;
                        }

                        return octaspire_dern_lexer_pop_next_token(input, allocator);
                    }

                    default:
                    {
                        return octaspire_dern_lexer_private_pop_true_or_false_or_nil_or_symbol(
                            input,
                            allocator,
                            startLine,
                            startColumn,
                            startIndexInInput);
                    }
                }
            }

            case '(':
            {
                return octaspire_dern_lexer_private_pop_left_parenthesis(
                    input,
                    allocator,
                    startLine,
                    startColumn,
                    startIndexInInput);
            }

            case ')':
            {
                return octaspire_dern_lexer_private_pop_right_parenthesis(
                    input,
                    allocator,
                    startLine,
                    startColumn,
                    startIndexInInput);
            }

            case '\'':
            {
                return octaspire_dern_lexer_private_pop_quote(
                    input,
                    allocator,
                    startLine,
                    startColumn,
                    startIndexInInput);
            }

            case '`':
            {
                return octaspire_dern_lexer_private_pop_back_quote(
                    input,
                    allocator,
                    startLine,
                    startColumn,
                    startIndexInInput);
            }

            case '-':
            {
                return octaspire_dern_lexer_private_pop_true_or_false_or_nil_or_symbol(
                    input,
                    allocator,
                    startLine,
                    startColumn,
                    startIndexInInput);
            }

            case '{':
            {
                return octaspire_dern_lexer_private_pop_integer_or_real_number(
                    input,
                    allocator,
                    startLine,
                    startColumn,
                    startIndexInInput);
            }

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            {
                return octaspire_dern_lexer_private_pop_semver(
                    input,
                    allocator,
                    startLine,
                    startColumn,
                    startIndexInInput);
            }

            /*
            case '|':
            {
                return octaspire_dern_lexer_private_pop_string_or_character(
                    input,
                    allocator,
                    startLine,
                    startColumn,
                    startIndexInInput);
            }
            break;
            */

            case '[':
            {
                return octaspire_dern_lexer_private_pop_string(
                    input,
                    allocator,
                    startLine,
                    startColumn,
                    startIndexInInput);
            }

            case '|':
            {
                return octaspire_dern_lexer_private_pop_character(
                    input,
                    allocator,
                    startLine,
                    startColumn,
                    startIndexInInput);
            }

            case 't':
            case 'f':
            case 'n':
            default:
            {
                return octaspire_dern_lexer_private_pop_true_or_false_or_nil_or_symbol(
                    input,
                    allocator,
                    startLine,
                    startColumn,
                    startIndexInInput);
            }
        }
    }

    return 0;
}

