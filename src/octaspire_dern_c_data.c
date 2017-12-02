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
#include <octaspire/core/octaspire_helpers.h>
#include "octaspire/dern/octaspire_dern_config.h"
#include "octaspire/dern/octaspire_dern_lib.h"

#ifdef OCTASPIRE_DERN_CONFIG_BINARY_PLUGINS
#include <dlfcn.h>
#endif

struct octaspire_dern_c_data_t
{
    octaspire_container_utf8_string_t         *pluginName;
    octaspire_dern_lib_t                      *library;
    octaspire_container_utf8_string_t         *typeNameForPayload;
    void                                      *payload;
    octaspire_container_utf8_string_t         *cleanUpCallbackName;
    octaspire_container_utf8_string_t         *stdLibLenCallbackName;
    octaspire_container_utf8_string_t         *stdLibLinkAtCallbackName;
    octaspire_container_utf8_string_t         *stdLibCopyAtCallbackName;
    octaspire_memory_allocator_t              *allocator;
    bool                                       copyingAllowed;
    char                                       padding[7];
};

octaspire_dern_c_data_t *octaspire_dern_c_data_new(
    char const * const pluginName,
    octaspire_dern_lib_t * const library,
    char const * const typeNameForPayload,
    void * const payload,
    char const * const cleanUpCallbackName,
    char const * const stdLibLenCallbackName,
    char const * const stdLibLinkAtCallbackName,
    char const * const stdLibCopyAtCallbackName,
    bool const copyingAllowed,
    octaspire_memory_allocator_t *allocator)
{
    octaspire_dern_c_data_t *self =
        octaspire_memory_allocator_malloc(allocator, sizeof(octaspire_dern_c_data_t));

    if (!self)
    {
        return self;
    }

    self->allocator             = allocator;
    self->pluginName            = octaspire_container_utf8_string_new(pluginName,         self->allocator);
    self->library               = library;
    self->typeNameForPayload    = octaspire_container_utf8_string_new(typeNameForPayload, self->allocator);
    self->payload               = payload;
    self->cleanUpCallbackName   = octaspire_container_utf8_string_new(cleanUpCallbackName, self->allocator);
    self->stdLibLenCallbackName = octaspire_container_utf8_string_new(stdLibLenCallbackName, self->allocator);
    self->stdLibLinkAtCallbackName = octaspire_container_utf8_string_new(stdLibLinkAtCallbackName, self->allocator);
    self->stdLibCopyAtCallbackName = octaspire_container_utf8_string_new(stdLibCopyAtCallbackName, self->allocator);
    self->copyingAllowed        = copyingAllowed;

    return self;
}

octaspire_dern_c_data_t *octaspire_dern_c_data_new_copy(
    octaspire_dern_c_data_t * const other,
    octaspire_memory_allocator_t *allocator)
{
    return octaspire_dern_c_data_new(
        octaspire_container_utf8_string_get_c_string(other->pluginName),
        other->library,
        octaspire_container_utf8_string_get_c_string(other->typeNameForPayload),
        other->payload,
        octaspire_container_utf8_string_get_c_string(other->cleanUpCallbackName),
        octaspire_container_utf8_string_get_c_string(other->stdLibLenCallbackName),
        octaspire_container_utf8_string_get_c_string(other->stdLibLinkAtCallbackName),
        octaspire_container_utf8_string_get_c_string(other->stdLibCopyAtCallbackName),
        other->copyingAllowed,
        allocator);
}

void octaspire_dern_c_data_release(octaspire_dern_c_data_t *self)
{
    if (!self)
    {
        return;
    }

    octaspire_helpers_verify_not_null(self->cleanUpCallbackName);

#ifdef OCTASPIRE_DERN_CONFIG_BINARY_PLUGINS
    if (!octaspire_container_utf8_string_is_empty(self->cleanUpCallbackName) &&
         self->library)
    {
#ifdef _WIN32
        HINSTANCE * const handle = octaspire_dern_lib_get_handle(self->library);
        void (*func)(void * const payload);
        func = (void (*)(void * const))GetProcAddress(*handle, octaspire_container_utf8_string_get_c_string(self->cleanUpCallbackName));
        if (func)
        {
            func(self->payload);
        }
#else
        void * const handle = octaspire_dern_lib_get_handle(self->library);
        void (*func)(void * const payload);
        func = (void (*)(void * const))dlsym(handle, octaspire_container_utf8_string_get_c_string(self->cleanUpCallbackName));
        if (func)
        {
            func(self->payload);
        }
#endif
    }
#endif

    octaspire_container_utf8_string_release(self->cleanUpCallbackName);
    self->cleanUpCallbackName = 0;

    octaspire_container_utf8_string_release(self->stdLibLenCallbackName);
    self->stdLibLenCallbackName = 0;

    octaspire_container_utf8_string_release(self->stdLibLinkAtCallbackName);
    self->stdLibLinkAtCallbackName = 0;

    octaspire_container_utf8_string_release(self->stdLibCopyAtCallbackName);
    self->stdLibCopyAtCallbackName = 0;

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
        "C data (%s : %s) payload=%p cleanUpCallbackName=%s stdLibLenCallbackName=%s "
        "stdLibLinkAtCallbackName=%s stdLibCopyAtCallbackName=%s",
        octaspire_container_utf8_string_get_c_string(self->pluginName),
        octaspire_container_utf8_string_get_c_string(self->typeNameForPayload),
        (void*)self->payload,
        octaspire_container_utf8_string_get_c_string(self->cleanUpCallbackName),
        octaspire_container_utf8_string_get_c_string(self->stdLibLenCallbackName),
        octaspire_container_utf8_string_get_c_string(self->stdLibLinkAtCallbackName),
        octaspire_container_utf8_string_get_c_string(self->stdLibCopyAtCallbackName));
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

bool octaspire_dern_c_data_is_copying_allowed(
    octaspire_dern_c_data_t const * const self)
{
    return self->copyingAllowed;
}

