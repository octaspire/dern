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
#include "octaspire/dern/octaspire_dern_vm.h"

#ifndef OCTASPIRE_DERN_DO_NOT_USE_AMALGAMATED_CORE
    #include "octaspire-core-amalgamated.c"
#else
    #include <octaspire/core/octaspire_helpers.h>
    #include "octaspire/dern/octaspire_dern_config.h"
    #include "octaspire/dern/octaspire_dern_lib.h"
#endif

#ifdef OCTASPIRE_DERN_CONFIG_BINARY_PLUGINS
#include <dlfcn.h>
#endif

struct octaspire_dern_c_data_t
{
    octaspire_string_t    *pluginName;
    octaspire_string_t    *typeNameForPayload;
    void                  *payload;
    octaspire_string_t    *cleanUpCallbackName;
    octaspire_string_t    *stdLibLenCallbackName;
    octaspire_string_t    *stdLibLinkAtCallbackName;
    octaspire_string_t    *stdLibCopyAtCallbackName;
    octaspire_string_t    *stdLibToStringCallbackName;
    octaspire_dern_vm_t   *vm;
    octaspire_allocator_t *allocator;
    bool                   copyingAllowed;
    char                   padding[7];
};

octaspire_dern_c_data_t *octaspire_dern_c_data_new(
    char const * const pluginName,
    char const * const typeNameForPayload,
    void * const payload,
    char const * const cleanUpCallbackName,
    char const * const stdLibLenCallbackName,
    char const * const stdLibLinkAtCallbackName,
    char const * const stdLibCopyAtCallbackName,
    char const * const stdLibToStringCallbackName,
    bool const copyingAllowed,
    octaspire_dern_vm_t * const vm,
    octaspire_allocator_t *allocator)
{
    octaspire_dern_c_data_t *self =
        octaspire_allocator_malloc(allocator, sizeof(octaspire_dern_c_data_t));

    if (!self)
    {
        return self;
    }

    self->allocator = allocator;
    self->vm        = vm;

    self->pluginName = octaspire_string_new(
        pluginName,
        self->allocator);

    self->typeNameForPayload = octaspire_string_new(
        typeNameForPayload,
        self->allocator);

    self->payload = payload;

    self->cleanUpCallbackName = octaspire_string_new(
        cleanUpCallbackName,
        self->allocator);

    self->stdLibLenCallbackName = octaspire_string_new(
        stdLibLenCallbackName,
        self->allocator);

    self->stdLibLinkAtCallbackName = octaspire_string_new(
        stdLibLinkAtCallbackName,
        self->allocator);

    self->stdLibCopyAtCallbackName = octaspire_string_new(
        stdLibCopyAtCallbackName,
        self->allocator);

    self->stdLibToStringCallbackName = octaspire_string_new(
        stdLibToStringCallbackName,
        self->allocator);

    self->copyingAllowed = copyingAllowed;

    return self;
}

octaspire_dern_c_data_t *octaspire_dern_c_data_new_copy(
    octaspire_dern_c_data_t * const other,
    octaspire_allocator_t *allocator)
{
    return octaspire_dern_c_data_new(
        octaspire_string_get_c_string(other->pluginName),
        octaspire_string_get_c_string(other->typeNameForPayload),
        other->payload,
        octaspire_string_get_c_string(other->cleanUpCallbackName),
        octaspire_string_get_c_string(other->stdLibLenCallbackName),
        octaspire_string_get_c_string(other->stdLibLinkAtCallbackName),
        octaspire_string_get_c_string(other->stdLibCopyAtCallbackName),
        octaspire_string_get_c_string(other->stdLibToStringCallbackName),
        other->copyingAllowed,
        other->vm,
        allocator);
}

void octaspire_dern_c_data_release(octaspire_dern_c_data_t *self)
{
    if (!self)
    {
        return;
    }

    octaspire_string_release(self->cleanUpCallbackName);
    self->cleanUpCallbackName = 0;

    octaspire_string_release(self->stdLibLenCallbackName);
    self->stdLibLenCallbackName = 0;

    octaspire_string_release(self->stdLibLinkAtCallbackName);
    self->stdLibLinkAtCallbackName = 0;

    octaspire_string_release(self->stdLibCopyAtCallbackName);
    self->stdLibCopyAtCallbackName = 0;

    octaspire_string_release(self->stdLibToStringCallbackName);
    self->stdLibToStringCallbackName = 0;

    octaspire_string_release(self->pluginName);
    self->pluginName = 0;

    octaspire_string_release(self->typeNameForPayload);
    self->typeNameForPayload = 0;

    octaspire_allocator_free(self->allocator, self);
}

octaspire_string_t *octaspire_dern_c_data_to_string(
    octaspire_dern_c_data_t const * const self,
    octaspire_allocator_t * const allocator)
{
    if (octaspire_string_is_empty(self->stdLibToStringCallbackName))
    {
        return octaspire_string_new_format(
            allocator,
            "C data (%s : %s) payload=%p cleanUpCallbackName=%s stdLibLenCallbackName=%s "
            "stdLibLinkAtCallbackName=%s stdLibCopyAtCallbackName=%s "
            "stdLibCopyAtCallbackName=%s",
            octaspire_string_get_c_string(self->pluginName),
            octaspire_string_get_c_string(self->typeNameForPayload),
            (void*)self->payload,
            octaspire_string_get_c_string(self->cleanUpCallbackName),
            octaspire_string_get_c_string(self->stdLibLenCallbackName),
            octaspire_string_get_c_string(self->stdLibLinkAtCallbackName),
            octaspire_string_get_c_string(self->stdLibCopyAtCallbackName),
            octaspire_string_get_c_string(self->stdLibToStringCallbackName));
    }

    octaspire_dern_lib_t const * const lib = octaspire_dern_vm_get_library_const(
        octaspire_dern_c_data_get_vm_const(self),
        octaspire_dern_c_data_get_plugin_name(self));

    if (!lib)
    {
        return 0;
    }

    return octaspire_dern_lib_dycall(
        (octaspire_dern_lib_t * const)lib,
        octaspire_string_get_c_string(self->stdLibToStringCallbackName),
        (octaspire_dern_c_data_t * const)self);
}

bool octaspire_dern_c_data_is_equal(
    octaspire_dern_c_data_t const * const self,
    octaspire_dern_c_data_t const * const other)
{
    if (self->payload != other->payload)
    {
        return false;
    }

    if (octaspire_string_is_equal(self->pluginName, other->pluginName))
    {
        return false;
    }

    if (octaspire_string_is_equal(
        self->typeNameForPayload,
        other->typeNameForPayload))
    {
        return false;
    }

    return true;
}

int octaspire_dern_c_data_compare(
    octaspire_dern_c_data_t const * const self,
    octaspire_dern_c_data_t const * const other)
{
    int tmp = octaspire_string_compare(self->pluginName, other->pluginName);

    if (tmp != 0)
    {
        return tmp;
    }

    tmp = octaspire_string_compare(
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
    if (!octaspire_string_is_equal_to_c_string(self->pluginName, pluginName))
    {
        return false;
    }

    if (!octaspire_string_is_equal_to_c_string(
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
    return octaspire_string_get_c_string(self->pluginName);
}

char const *octaspire_dern_c_data_get_payload_typename(
    octaspire_dern_c_data_t const * const self)
{
    return octaspire_string_get_c_string(self->typeNameForPayload);
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

octaspire_dern_vm_t * octaspire_dern_c_data_get_vm(
    octaspire_dern_c_data_t * const self)
{
    return self->vm;
}

octaspire_dern_vm_t const * octaspire_dern_c_data_get_vm_const(
    octaspire_dern_c_data_t const * const self)
{
    return self->vm;
}
