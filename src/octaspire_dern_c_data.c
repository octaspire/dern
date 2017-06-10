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
#include "octaspire/dern/octaspire_dern_c_data.h"

struct octaspire_dern_c_data_t
{
    octaspire_container_utf8_string_t *pluginName;
    octaspire_container_utf8_string_t *typeNameForPayload;
    void                              *payload;
    octaspire_memory_allocator_t      *allocator;
};

octaspire_dern_c_data_t *octaspire_dern_c_data_new(
    char const * const pluginName,
    char const * const typeNameForPayload,
    void * const payload,
    octaspire_memory_allocator_t *allocator)
{
    octaspire_dern_c_data_t *self =
        octaspire_memory_allocator_malloc(allocator, sizeof(octaspire_dern_c_data_t));

    if (!self)
    {
        return self;
    }

    self->allocator          = allocator;
    self->pluginName         = octaspire_container_utf8_string_new(pluginName,         self->allocator);
    self->typeNameForPayload = octaspire_container_utf8_string_new(typeNameForPayload, self->allocator);
    self->payload            = payload;

    return self;
}

octaspire_dern_c_data_t *octaspire_dern_c_data_new_copy(
    octaspire_dern_c_data_t * const other,
    octaspire_memory_allocator_t *allocator)
{
    return octaspire_dern_c_data_new(
        octaspire_container_utf8_string_get_c_string(other->pluginName),
        octaspire_container_utf8_string_get_c_string(other->typeNameForPayload),
        other->payload,
        allocator);
}

void octaspire_dern_c_data_release(octaspire_dern_c_data_t *self)
{
    if (!self)
    {
        return;
    }

    octaspire_container_utf8_string_release(self->pluginName);
    self->pluginName = 0;

    octaspire_container_utf8_string_release(self->typeNameForPayload);
    self->typeNameForPayload = 0;

    octaspire_memory_allocator_free(self->allocator, self);
}

octaspire_container_utf8_string_t *octaspire_dern_c_data_to_string(
    octaspire_dern_c_data_t const * const self,
    octaspire_memory_allocator_t * const allocator)
{
    return octaspire_container_utf8_string_new_format(
        allocator,
        "C data (%s : %s) %p",
        octaspire_container_utf8_string_get_c_string(self->pluginName),
        octaspire_container_utf8_string_get_c_string(self->typeNameForPayload),
        (void*)self->payload);
}

bool octaspire_dern_c_data_is_equal(
    octaspire_dern_c_data_t const * const self,
    octaspire_dern_c_data_t const * const other)
{
    if (self->payload != other->payload)
    {
        return false;
    }

    if (octaspire_container_utf8_string_is_equal(self->pluginName, other->pluginName))
    {
        return false;
    }

    if (octaspire_container_utf8_string_is_equal(self->typeNameForPayload, other->typeNameForPayload))
    {
        return false;
    }

    return true;
}

int octaspire_dern_c_data_compare(
    octaspire_dern_c_data_t const * const self,
    octaspire_dern_c_data_t const * const other)
{
    int tmp = octaspire_container_utf8_string_compare(self->pluginName, other->pluginName);

    if (tmp != 0)
    {
        return tmp;
    }

    tmp = octaspire_container_utf8_string_compare(
        self->typeNameForPayload,
        other->typeNameForPayload);

    if (tmp != 0)
    {
        return tmp;
    }

    ptrdiff_t const diff = self - other;

    if (diff == 0)
    {
        return 0;
    }

    if (diff < 0)
    {
        return -1;
    }

    return 1;
}

bool octaspire_dern_c_data_is_plugin_and_payload_type_name(
    octaspire_dern_c_data_t const * const self,
    char const * const pluginName,
    char const * const typeNameForPayload)
{
    if (!octaspire_container_utf8_string_is_equal_to_c_string(self->pluginName, pluginName))
    {
        return false;
    }

    if (!octaspire_container_utf8_string_is_equal_to_c_string(
            self->typeNameForPayload,
            typeNameForPayload))
    {
        return false;
    }

    return true;
}

char const *octaspire_dern_c_data_get_plugin_name(
    octaspire_dern_c_data_t const * const self)
{
    return octaspire_container_utf8_string_get_c_string(self->pluginName);
}

char const *octaspire_dern_c_data_get_payload_typename(
    octaspire_dern_c_data_t const * const self)
{
    return octaspire_container_utf8_string_get_c_string(self->typeNameForPayload);
}

void *octaspire_dern_c_data_get_payload(
    octaspire_dern_c_data_t const * const self)
{
    return self->payload;
}

