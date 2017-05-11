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
#include "octaspire/dern/octaspire_dern_port.h"
#include <stdio.h>
#include "octaspire/core/octaspire_helpers.h"

struct octaspire_dern_port_t
{
    octaspire_memory_allocator_t      *allocator;
    octaspire_container_utf8_string_t *name;
    size_t                             lengthInOctets;
    octaspire_dern_port_tag_t          typeTag;

    union
    {
        FILE *file;
    }
    value;
};

octaspire_dern_port_t *octaspire_dern_port_new_copy(
    octaspire_dern_port_t * const other,
    octaspire_memory_allocator_t *allocator)
{
    octaspire_dern_port_t *self =
        octaspire_memory_allocator_malloc(allocator, sizeof(octaspire_dern_port_t));

    if (!self)
    {
        return self;
    }

    self->allocator      = allocator;
    self->name           = octaspire_container_utf8_string_new_copy(other->name, self->allocator);
    self->typeTag        = other->typeTag;
    self->lengthInOctets = other->lengthInOctets;

    switch (self->typeTag)
    {
        case OCTASPIRE_DERN_PORT_TAG_FILE:
        {
            self->value.file =
                fopen(octaspire_container_utf8_string_get_c_string(self->name), "r+b");

            if (self->value.file == 0)
            {
                self->typeTag = OCTASPIRE_DERN_PORT_TAG_NOT_OPEN;
            }

            octaspire_helpers_verify(fseek(self->value.file, 0, SEEK_END) == 0);
            self->lengthInOctets = ftell(self->value.file);
            octaspire_helpers_verify(fseek(self->value.file, 0, SEEK_SET) == 0);
        }
        break;

        case OCTASPIRE_DERN_PORT_TAG_NOT_OPEN:
        {
            // NOP
        }
        break;
    }

    return self;
}

octaspire_dern_port_t *octaspire_dern_port_new_io_file(
    char const * const path,
    octaspire_memory_allocator_t *allocator)
{
    octaspire_dern_port_t *self =
        octaspire_memory_allocator_malloc(allocator, sizeof(octaspire_dern_port_t));

    if (!self)
    {
        return self;
    }

    self->allocator  = allocator;
    self->name       = octaspire_container_utf8_string_new(path, self->allocator);
    self->typeTag    = OCTASPIRE_DERN_PORT_TAG_FILE;
    self->lengthInOctets = -1;
    self->value.file = fopen(path, "r+b");

    if (self->value.file == 0)
    {
        self->typeTag = OCTASPIRE_DERN_PORT_TAG_NOT_OPEN;
    }

    octaspire_helpers_verify(fseek(self->value.file, 0, SEEK_END) == 0);
    self->lengthInOctets = ftell(self->value.file);
    octaspire_helpers_verify(fseek(self->value.file, 0, SEEK_SET) == 0);

    return self;
}

void octaspire_dern_port_release(octaspire_dern_port_t *self)
{
    if (!self)
    {
        return;
    }

    if (self->typeTag == OCTASPIRE_DERN_PORT_TAG_FILE)
    {
        int const res = fclose(self->value.file);
        self->value.file = 0;
        self->lengthInOctets = -1;
        octaspire_helpers_verify(res == 0);
    }

    octaspire_container_utf8_string_release(self->name);
    self->name = 0;

    octaspire_memory_allocator_free(self->allocator, self);
}

ptrdiff_t octaspire_dern_port_write(
    octaspire_dern_port_t * const self,
    void const * const buffer,
    size_t const bufferSizeInOctets)
{
    octaspire_helpers_verify(self);

    if (self->typeTag == OCTASPIRE_DERN_PORT_TAG_FILE)
    {
        octaspire_helpers_verify(self->value.file);
        size_t const numItemsWritten = fwrite(buffer, sizeof(char), bufferSizeInOctets, self->value.file);
        return (ptrdiff_t)numItemsWritten;
    }

    return -1;
}

ptrdiff_t octaspire_dern_port_read(
    octaspire_dern_port_t * const self,
    void * const buffer,
    size_t const bufferSizeInOctets)
{
    octaspire_helpers_verify(self);

    if (self->typeTag == OCTASPIRE_DERN_PORT_TAG_FILE)
    {
        octaspire_helpers_verify(self->value.file);
        size_t const numItemsRead = fread(buffer, sizeof(char), bufferSizeInOctets, self->value.file);
        return (ptrdiff_t)numItemsRead;
    }

    return -1;
}

bool octaspire_dern_port_close(
    octaspire_dern_port_t * const self)
{
    switch (self->typeTag)
    {
        case OCTASPIRE_DERN_PORT_TAG_FILE:
        {
            int const res = fclose(self->value.file);
            self->value.file = 0;
            self->lengthInOctets = -1;
            self->typeTag = OCTASPIRE_DERN_PORT_TAG_NOT_OPEN;
            return (res == 0);
        }
        break;

        case OCTASPIRE_DERN_PORT_TAG_NOT_OPEN:
        {
            return false;
        }
        break;
    }

    abort();
}

ptrdiff_t octaspire_dern_port_get_length_in_octets(octaspire_dern_port_t const * const self)
{
    octaspire_helpers_verify(self);
    return self->lengthInOctets;
}

octaspire_container_utf8_string_t *octaspire_dern_port_to_string(
    octaspire_dern_port_t const * const self,
    octaspire_memory_allocator_t * const allocator)
{
    switch (self->typeTag)
    {
        case OCTASPIRE_DERN_PORT_TAG_FILE:
        {
            return octaspire_container_utf8_string_new_format(
                allocator,
                "<input-output-port:%s (%td octets)>",
                octaspire_container_utf8_string_get_c_string(self->name),
                self->lengthInOctets);
        }
        break;

        case OCTASPIRE_DERN_PORT_TAG_NOT_OPEN:
        {
            return octaspire_container_utf8_string_new_format(
                allocator,
                "<NOT-OPEN-port:%s (%td octets)>",
                octaspire_container_utf8_string_get_c_string(self->name),
                self->lengthInOctets);
        }
        break;
    }

    abort();
}

