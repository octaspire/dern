#include "octaspire/core/octaspire_input.h"
#include <assert.h>
#include <string.h>
#include "octaspire/core/octaspire_container_utf8_string.h"
#include "octaspire/core/octaspire_helpers.h"

struct octaspire_input_t
{
    octaspire_container_utf8_string_t *text;
    size_t                             index;
    size_t                             line;
    size_t                             column;
    octaspire_memory_allocator_t      *allocator;
};

bool octaspire_input_private_is_ucs_character_index_valid(
    octaspire_input_t const * const self,
    size_t index);

octaspire_input_t *octaspire_input_new_from_c_string(
    char const * const str,
    octaspire_memory_allocator_t *allocator)
{
    return octaspire_input_new_from_buffer(str, str ? strlen(str) : 0, allocator);
}

octaspire_input_t *octaspire_input_new_from_buffer(
    char const * const buffer,
    size_t const lengthInOctets,
    octaspire_memory_allocator_t *allocator)
{
    octaspire_input_t *self =
        octaspire_memory_allocator_malloc(allocator, sizeof(octaspire_input_t));

    if (!self)
    {
        return self;
    }

    self->allocator = allocator;

    self->index  = 0;
    self->line   = 1;
    self->column = 1;

    self->text   = octaspire_container_utf8_string_new_from_buffer(buffer, lengthInOctets, self->allocator);

    if (!self->text)
    {
        octaspire_input_release(self);
        self = 0;
        return 0;
    }

    return self;
}

octaspire_input_t *octaspire_input_new_from_path(
    char const * const path,
    octaspire_memory_allocator_t *octaspireAllocator,
    octaspire_stdio_t *octaspireStdio)
{
    size_t octetsAllocated = 0;
    char *buffer = octaspire_helpers_path_to_buffer(path, &octetsAllocated, octaspireAllocator, octaspireStdio);

    if (!buffer)
    {
        return 0;
    }

    octaspire_input_t *self = octaspire_input_new_from_buffer(buffer, octetsAllocated, octaspireAllocator);

    octaspire_memory_allocator_free(octaspireAllocator, buffer);
    buffer = 0;

    return self;
}

void octaspire_input_release(octaspire_input_t *self)
{
    if (!self)
    {
        return;
    }

    octaspire_container_utf8_string_release(self->text);
    octaspire_memory_allocator_free(self->allocator, self);
}

size_t octaspire_input_get_length_in_ucs_characters(octaspire_input_t const * const self)
{
    return octaspire_container_utf8_string_get_length_in_ucs_characters(self->text);
}

void   octaspire_input_clear(octaspire_input_t *self)
{
    octaspire_container_utf8_string_clear(self->text);
    self->index  = 0;
    self->line   = 1;
    self->column = 1;
}

void   octaspire_input_rewind(octaspire_input_t *self)
{
    self->index  = 0;
    self->line   = 1;
    self->column = 1;
}

uint32_t octaspire_input_peek_next_ucs_character(octaspire_input_t *self)
{
    if (self->index >= octaspire_container_utf8_string_get_length_in_ucs_characters(self->text))
    {
        return 0;
    }

    return octaspire_container_utf8_string_get_ucs_character_at_index(self->text, self->index);
}

uint32_t octaspire_input_peek_next_next_ucs_character(octaspire_input_t *self)
{
    if ((self->index + 1) >= octaspire_container_utf8_string_get_length_in_ucs_characters(self->text))
    {
        return 0;
    }

    return octaspire_container_utf8_string_get_ucs_character_at_index(self->text, self->index + 1);
}

bool octaspire_input_pop_next_ucs_character(octaspire_input_t *self)
{
    if (!octaspire_input_private_is_ucs_character_index_valid(self, self->index))
    {
        return false;
    }

    uint32_t const result =
        octaspire_container_utf8_string_get_ucs_character_at_index(self->text, self->index);

    ++(self->index);

    if (octaspire_input_private_is_ucs_character_index_valid(self, self->index))
    {
        if (result == '\n')
        {
            self->column = 1;
            ++(self->line);
        }
        else
        {
            ++(self->column);
        }
    }

    return true;
}

bool octaspire_input_is_good(octaspire_input_t const * const self)
{
    return self->index < octaspire_container_utf8_string_get_length_in_ucs_characters(self->text);
}

bool octaspire_input_private_is_ucs_character_index_valid(
    octaspire_input_t const * const self,
    size_t index)
{
    return index < octaspire_container_utf8_string_get_length_in_ucs_characters(self->text);
}

bool octaspire_input_push_back_from_string(
    octaspire_input_t * const self,
    octaspire_container_utf8_string_t const * const str)
{
    return octaspire_input_push_back_from_c_string(
        self,
        octaspire_container_utf8_string_get_c_string(str));
}

bool octaspire_input_push_back_from_c_string(octaspire_input_t * const self, char const * const str)
{
    assert(self);
    return octaspire_container_utf8_string_concatenate_c_string(self->text, str);
}

size_t octaspire_input_get_line_number(octaspire_input_t const * const self)
{
    return self->line;
}

size_t octaspire_input_get_column_number(octaspire_input_t const * const self)
{
    return self->column;
}

size_t octaspire_input_get_ucs_character_index(octaspire_input_t const * const self)
{
    return self->index;
}

void octaspire_input_print(octaspire_input_t const * const self)
{
    printf("\n-------------------------- octaspire input --------------------------\n");
    printf("%s", octaspire_container_utf8_string_get_c_string(self->text));
    printf("---------------------------------------------------------------------\n");
}


