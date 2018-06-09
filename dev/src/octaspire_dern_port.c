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

#ifndef OCTASPIRE_DERN_DO_NOT_USE_AMALGAMATED_CORE
    #include "octaspire-core-amalgamated.c"
#else
    #include "octaspire/core/octaspire_helpers.h"
#endif

struct octaspire_dern_port_t
{
    octaspire_allocator_t *allocator;
    octaspire_string_t    *name;
    ptrdiff_t              lengthInOctets;

    union
    {
        FILE *file;
    }
    value;

    octaspire_dern_port_tag_t typeTag;
    char                      padding[4];
};

octaspire_dern_port_t *octaspire_dern_port_new_copy(
    octaspire_dern_port_t * const other,
    octaspire_allocator_t *allocator)
{
    octaspire_dern_port_t *self =
        octaspire_allocator_malloc(allocator, sizeof(octaspire_dern_port_t));

    if (!self)
    {
        return self;
    }

    self->allocator      = allocator;
    self->name           = octaspire_string_new_copy(other->name, self->allocator);
    self->typeTag        = other->typeTag;
    self->lengthInOctets = other->lengthInOctets;

    switch (self->typeTag)
    {
        case OCTASPIRE_DERN_PORT_TAG_IO_FILE:
        {
            self->value.file =
                fopen(octaspire_string_get_c_string(self->name), "a+b");

            if (self->value.file == 0)
            {
                self->typeTag = OCTASPIRE_DERN_PORT_TAG_NOT_OPEN;
            }

            octaspire_helpers_verify_true(fseek(self->value.file, 0, SEEK_END) == 0);
            self->lengthInOctets = ftell(self->value.file);
            octaspire_helpers_verify_true(fseek(self->value.file, 0, SEEK_SET) == 0);
        }
        break;

        case OCTASPIRE_DERN_PORT_TAG_INPUT_FILE:
        {
            self->value.file =
                fopen(octaspire_string_get_c_string(self->name), "rb");

            if (self->value.file == 0)
            {
                self->typeTag = OCTASPIRE_DERN_PORT_TAG_NOT_OPEN;
            }

            octaspire_helpers_verify_true(fseek(self->value.file, 0, SEEK_END) == 0);
            self->lengthInOctets = ftell(self->value.file);
            octaspire_helpers_verify_true(fseek(self->value.file, 0, SEEK_SET) == 0);
        }
        break;

        case OCTASPIRE_DERN_PORT_TAG_OUTPUT_FILE:
        {
            self->value.file =
                fopen(octaspire_string_get_c_string(self->name), "a");

            if (self->value.file == 0)
            {
                self->typeTag = OCTASPIRE_DERN_PORT_TAG_NOT_OPEN;
            }

            self->lengthInOctets = ftell(self->value.file);
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

octaspire_dern_port_t *octaspire_dern_port_new_input_file(
    char const * const path,
    octaspire_allocator_t *allocator)
{
    octaspire_dern_port_t *self =
        octaspire_allocator_malloc(allocator, sizeof(octaspire_dern_port_t));

    if (!self)
    {
        return self;
    }

    self->allocator  = allocator;
    self->name       = octaspire_string_new(path, self->allocator);
    self->typeTag    = OCTASPIRE_DERN_PORT_TAG_INPUT_FILE;
    self->lengthInOctets = -1;
    self->value.file = fopen(path, "rb");

    if (self->value.file == 0)
    {
        self->typeTag = OCTASPIRE_DERN_PORT_TAG_NOT_OPEN;
        return self;
    }

    octaspire_helpers_verify_true(fseek(self->value.file, 0, SEEK_END) == 0);
    self->lengthInOctets = ftell(self->value.file);
    octaspire_helpers_verify_true(fseek(self->value.file, 0, SEEK_SET) == 0);

    return self;
}

octaspire_dern_port_t *octaspire_dern_port_new_output_file(
    char const * const path,
    octaspire_allocator_t *allocator)
{
    octaspire_dern_port_t *self =
        octaspire_allocator_malloc(allocator, sizeof(octaspire_dern_port_t));

    if (!self)
    {
        return self;
    }

    self->allocator  = allocator;
    self->name       = octaspire_string_new(path, self->allocator);
    self->typeTag    = OCTASPIRE_DERN_PORT_TAG_OUTPUT_FILE;
    self->lengthInOctets = -1;
    self->value.file = fopen(path, "a");

    if (self->value.file == 0)
    {
        self->typeTag = OCTASPIRE_DERN_PORT_TAG_NOT_OPEN;
        return self;
    }

    self->lengthInOctets = ftell(self->value.file);
    return self;
}

octaspire_dern_port_t *octaspire_dern_port_new_io_file(
    char const * const path,
    octaspire_allocator_t *allocator)
{
    octaspire_dern_port_t *self =
        octaspire_allocator_malloc(allocator, sizeof(octaspire_dern_port_t));

    if (!self)
    {
        return self;
    }

    self->allocator  = allocator;
    self->name       = octaspire_string_new(path, self->allocator);
    self->typeTag    = OCTASPIRE_DERN_PORT_TAG_IO_FILE;
    self->lengthInOctets = -1;
    self->value.file = fopen(path, "a+b");

    if (self->value.file == 0)
    {
        self->typeTag = OCTASPIRE_DERN_PORT_TAG_NOT_OPEN;
        return self;
    }

    octaspire_helpers_verify_true(fseek(self->value.file, 0, SEEK_END) == 0);
    self->lengthInOctets = ftell(self->value.file);
    octaspire_helpers_verify_true(fseek(self->value.file, 0, SEEK_SET) == 0);

    return self;
}

void octaspire_dern_port_release(octaspire_dern_port_t *self)
{
    if (!self)
    {
        return;
    }

    if (self->typeTag == OCTASPIRE_DERN_PORT_TAG_INPUT_FILE  ||
        self->typeTag == OCTASPIRE_DERN_PORT_TAG_OUTPUT_FILE ||
        self->typeTag == OCTASPIRE_DERN_PORT_TAG_IO_FILE)
    {
        int const res = fclose(self->value.file);
        self->value.file = 0;
        self->lengthInOctets = -1;
        octaspire_helpers_verify_true(res == 0);
    }

    octaspire_string_release(self->name);
    self->name = 0;

    octaspire_allocator_free(self->allocator, self);
}

ptrdiff_t octaspire_dern_port_write(
    octaspire_dern_port_t * const self,
    void const * const buffer,
    size_t const bufferSizeInOctets)
{
    octaspire_helpers_verify_not_null(self);

    if (self->typeTag == OCTASPIRE_DERN_PORT_TAG_IO_FILE ||
        self->typeTag == OCTASPIRE_DERN_PORT_TAG_OUTPUT_FILE)
    {
        octaspire_helpers_verify_not_null(self->value.file);

        size_t const numItemsWritten =
            fwrite(buffer, sizeof(char), bufferSizeInOctets, self->value.file);

        return (ptrdiff_t)numItemsWritten;
    }

    return -1;
}

ptrdiff_t octaspire_dern_port_read(
    octaspire_dern_port_t * const self,
    void * const buffer,
    size_t const bufferSizeInOctets)
{
    octaspire_helpers_verify_not_null(self);

    if (self->typeTag == OCTASPIRE_DERN_PORT_TAG_INPUT_FILE ||
        self->typeTag == OCTASPIRE_DERN_PORT_TAG_IO_FILE)
    {
        octaspire_helpers_verify_not_null(self->value.file);

        size_t const numItemsRead =
            fread(buffer, sizeof(char), bufferSizeInOctets, self->value.file);

        return (ptrdiff_t)numItemsRead;
    }

    return -1;
}

bool octaspire_dern_port_close(
    octaspire_dern_port_t * const self)
{
    switch (self->typeTag)
    {
        case OCTASPIRE_DERN_PORT_TAG_INPUT_FILE:
        case OCTASPIRE_DERN_PORT_TAG_OUTPUT_FILE:
        case OCTASPIRE_DERN_PORT_TAG_IO_FILE:
        {
            int const res = fclose(self->value.file);
            self->value.file = 0;
            self->lengthInOctets = -1;
            self->typeTag = OCTASPIRE_DERN_PORT_TAG_NOT_OPEN;
            return (res == 0);
        }

        case OCTASPIRE_DERN_PORT_TAG_NOT_OPEN:
        {
            return false;
        }
    }

    abort();
    return false;
}

ptrdiff_t octaspire_dern_port_get_length_in_octets(octaspire_dern_port_t const * const self)
{
    octaspire_helpers_verify_not_null(self);
    return self->lengthInOctets;
}

octaspire_string_t *octaspire_dern_port_to_string(
    octaspire_dern_port_t const * const self,
    octaspire_allocator_t * const allocator)
{
    switch (self->typeTag)
    {
        case OCTASPIRE_DERN_PORT_TAG_INPUT_FILE:
        {
            return octaspire_string_new_format(
                allocator,
#ifdef __AROS__
                "<input-port:%s (%ld octets)>",
#else
                "<input-port:%s (%td octets)>",
#endif
                octaspire_string_get_c_string(self->name),
                self->lengthInOctets);
        }

        case OCTASPIRE_DERN_PORT_TAG_OUTPUT_FILE:
        {
            return octaspire_string_new_format(
                allocator,
#ifdef __AROS__
                "<output-port:%s (%ld octets)>",
#else
                "<output-port:%s (%td octets)>",
#endif
                octaspire_string_get_c_string(self->name),
                self->lengthInOctets);
        }

        case OCTASPIRE_DERN_PORT_TAG_IO_FILE:
        {
            return octaspire_string_new_format(
                allocator,
#ifdef __AROS__
                "<input-output-port:%s (%ld octets)>",
#else
                "<input-output-port:%s (%td octets)>",
#endif
                octaspire_string_get_c_string(self->name),
                self->lengthInOctets);
        }

        case OCTASPIRE_DERN_PORT_TAG_NOT_OPEN:
        {
            return octaspire_string_new_format(
                allocator,
#ifdef __AROS__
                "<NOT-OPEN-port:%s (%ld octets)>",
#else
                "<NOT-OPEN-port:%s (%td octets)>",
#endif
                octaspire_string_get_c_string(self->name),
                self->lengthInOctets);
        }
    }

    return 0;
}

bool octaspire_dern_port_seek(
    octaspire_dern_port_t * const self,
    ptrdiff_t const amount,
    bool const fromCurrentPos)
{
    octaspire_helpers_verify_not_null(self);

    switch (self->typeTag)
    {
        case OCTASPIRE_DERN_PORT_TAG_INPUT_FILE:
        case OCTASPIRE_DERN_PORT_TAG_OUTPUT_FILE:
        case OCTASPIRE_DERN_PORT_TAG_IO_FILE:
        {
            octaspire_helpers_verify_not_null(self->value.file);

            if (amount < 0)
            {
                // Seek backwards from the end
                // TODO XXX check that amount fits into long and report error if it doesn't
                long const offset = fromCurrentPos ? amount : (amount + 1);
                return fseek(self->value.file, offset, fromCurrentPos ? SEEK_CUR : SEEK_END) == 0;
            }
            else
            {
                // Seek forward from the beginning
                // TODO XXX check that amount fits into long and report error if it doesn't
                long const offset = amount;
                return fseek(self->value.file, offset, fromCurrentPos ? SEEK_CUR : SEEK_SET) == 0;
            }
        }

        case OCTASPIRE_DERN_PORT_TAG_NOT_OPEN:
        {
            return false;
        }
    }

    return false;
}

bool octaspire_dern_port_flush(octaspire_dern_port_t * const self)
{
    octaspire_helpers_verify_not_null(self);

    switch (self->typeTag)
    {
        case OCTASPIRE_DERN_PORT_TAG_INPUT_FILE:
        case OCTASPIRE_DERN_PORT_TAG_OUTPUT_FILE:
        case OCTASPIRE_DERN_PORT_TAG_IO_FILE:
        {
            octaspire_helpers_verify_not_null(self->value.file);

            return fflush(self->value.file) == 0;
        }

        case OCTASPIRE_DERN_PORT_TAG_NOT_OPEN:
        {
            return false;
        }
    }

    return false;
}

ptrdiff_t octaspire_dern_port_distance(octaspire_dern_port_t const * const self)
{
    octaspire_helpers_verify_not_null(self);

    switch (self->typeTag)
    {
        case OCTASPIRE_DERN_PORT_TAG_INPUT_FILE:
        case OCTASPIRE_DERN_PORT_TAG_OUTPUT_FILE:
        case OCTASPIRE_DERN_PORT_TAG_IO_FILE:
        {
            octaspire_helpers_verify_not_null(self->value.file);

            return (ptrdiff_t)ftell(self->value.file);
        }

        case OCTASPIRE_DERN_PORT_TAG_NOT_OPEN:
        {
            return -1;
        }
    }

    return -1;
}

bool octaspire_dern_port_supports_output(
    octaspire_dern_port_t const * const self)
{
    octaspire_helpers_verify_not_null(self);

    switch (self->typeTag)
    {
        case OCTASPIRE_DERN_PORT_TAG_IO_FILE:
        case OCTASPIRE_DERN_PORT_TAG_OUTPUT_FILE:
        {
            octaspire_helpers_verify_not_null(self->value.file);
            return true;
        }

        case OCTASPIRE_DERN_PORT_TAG_INPUT_FILE:
        case OCTASPIRE_DERN_PORT_TAG_NOT_OPEN:
        {
            return false;
        }
    }

    return false;
}

bool octaspire_dern_port_supports_input(
    octaspire_dern_port_t const * const self)
{
    octaspire_helpers_verify_not_null(self);

    switch (self->typeTag)
    {
        case OCTASPIRE_DERN_PORT_TAG_IO_FILE:
        case OCTASPIRE_DERN_PORT_TAG_INPUT_FILE:
        {
            octaspire_helpers_verify_not_null(self->value.file);
            return true;
        }

        case OCTASPIRE_DERN_PORT_TAG_OUTPUT_FILE:
        case OCTASPIRE_DERN_PORT_TAG_NOT_OPEN:
        {
            return false;
        }
    }

    return false;
}

