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
#include "octaspire/dern/octaspire_dern_lib.h"
#include <octaspire/core/octaspire_helpers.h>
#include "octaspire/dern/octaspire_dern_vm.h"

#ifdef OCTASPIRE_DERN_CONFIG_PLUGINS
#include <dlfcn.h>
#endif

struct octaspire_dern_lib_t
{
    octaspire_memory_allocator_t      *allocator;
    octaspire_dern_vm_t               *vm;
    octaspire_container_utf8_string_t *name;
    octaspire_dern_lib_tag_t           typeTag;
    octaspire_container_utf8_string_t *errorMessage;
    void                              *binaryLibHandle;
};

octaspire_dern_lib_t *octaspire_dern_lib_new_source(
    char const * const name,
    octaspire_input_t * const input,
    octaspire_dern_vm_t *vm,
    octaspire_memory_allocator_t *allocator)
{
    octaspire_dern_lib_t *self =
        octaspire_memory_allocator_malloc(allocator, sizeof(octaspire_dern_lib_t));

    if (!self)
    {
        return self;
    }

    self->allocator       = allocator;
    self->vm              = vm;
    self->name            = octaspire_container_utf8_string_new(name, self->allocator);
    self->typeTag         = OCTASPIRE_DERN_LIB_TAG_SOURCE;
    self->binaryLibHandle = 0;

    octaspire_dern_value_t *value =
        octaspire_dern_vm_read_from_octaspire_input_and_eval_in_global_environment(vm, input);

    octaspire_helpers_verify(value);

    if (value->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
    {
        self->errorMessage =
            octaspire_container_utf8_string_new_copy(value->value.error, self->allocator);

        octaspire_helpers_verify(self->errorMessage);
    }

    return self;
}

octaspire_dern_lib_t *octaspire_dern_lib_new_binary(
    char const * const name,
    char const * const fileName,
    octaspire_dern_vm_t *vm,
    octaspire_memory_allocator_t *allocator)
{
    octaspire_dern_lib_t *self =
        octaspire_memory_allocator_malloc(allocator, sizeof(octaspire_dern_lib_t));

    if (!self)
    {
        return self;
    }

    self->allocator       = allocator;
    self->vm              = vm;
    self->name            = octaspire_container_utf8_string_new(name, self->allocator);
    self->typeTag         = OCTASPIRE_DERN_LIB_TAG_BINARY;
    self->binaryLibHandle = 0;


#ifdef OCTASPIRE_DERN_CONFIG_PLUGINS

        // Clear any old errors
        dlerror();

        self->binaryLibHandle = dlopen(fileName, RTLD_LAZY);

        if (!self->binaryLibHandle)
        {
            self->errorMessage =
                octaspire_container_utf8_string_new_format(
                    self->allocator,
                    "Binary library (name='%s' fileName='%s')\n"
                    "cannot be loaded/opened with dlopen. dlerror is: %s"
                    name,
                    fileName,
                    dlerror());

            octaspire_helpers_verify(self->errorMessage);
        }
        else
        {
            bool (*libInitFunc)(octaspire_dern_vm_t * const, octaspire_dern_environment_t * const);

            octaspire_container_utf8_string_t *libInitFuncName =
                octaspire_container_utf8_string_new_format(self->allocator, "%s_init", name);

            octaspire_helpers_verify(libInitFuncName);

            libInitFunc =
                (bool (*)(octaspire_dern_vm_t * const, octaspire_dern_value_t * const))dlsym(
                    self->binaryLibHandle,
                    octaspire_container_utf8_string_get_c_string(libInitFuncName));

            octaspire_container_utf8_string_release(libInitFuncName);
            libInitFuncName = 0;

            char *error = dlerror();

            if (error)
            {
                self->errorMessage =
                    octaspire_container_utf8_string_new_format(
                        self->allocator,
                        "Binary library (name='%s' fileName='%s'):\n"
                        "dlsym failed on the init function for the library. dlerror is: %s"
                        name,
                        fileName,
                        error);

                octaspire_helpers_verify(self->errorMessage);

                dlclose(self->binaryLibHandle);
                self->binaryLibHandle = 0;
            }
            else
            {
                if (!(*libInitFunc)(vm, octaspire_dern_vm_get_global_environment(vm)))
                {
                    self->errorMessage =
                        octaspire_container_utf8_string_new_format(
                            self->allocator,
                            "Binary library (name='%s' fileName='%s'):\n"
                            "init function failed."
                            name,
                            fileName);

                    octaspire_helpers_verify(self->errorMessage);

                    dlclose(self->binaryLibHandle);
                    self->binaryLibHandle = 0;
                }
            }
        }

#else

        self->errorMessage =
            octaspire_container_utf8_string_new_format(
                self->allocator,
                "Your version of Octaspire Dern is compiled WITHOUT support for binary libraries\n"
                "and this binary library (name='%s' fileName='%s')\n"
                "cannot thus be loaded. Use source-code libraries\n"
                "instead or compile Dern again with binary library support.",
                name,
                fileName);

        octaspire_helpers_verify(self->errorMessage);

#endif

        return self;
}

void octaspire_dern_lib_release(octaspire_dern_lib_t *self)
{
    if (!self)
    {
        return;
    }

    if (self->typeTag == OCTASPIRE_DERN_LIB_TAG_BINARY && self->binaryLibHandle)
    {
#ifdef OCTASPIRE_DERN_CONFIG_PLUGINS
        dlclose(self->binaryLibHandle);
#endif
        self->binaryLibHandle = 0;
    }

    octaspire_container_utf8_string_release(self->name);
    self->name = 0;

    octaspire_container_utf8_string_release(self->errorMessage);
    self->errorMessage = 0;

    octaspire_memory_allocator_free(self->allocator, self);
}

bool octaspire_dern_lib_is_good(octaspire_dern_lib_t const * const self)
{
    return (self->errorMessage == 0);
}

char const *octaspire_dern_lib_get_error_message(octaspire_dern_lib_t const * const self)
{
    if (!self->errorMessage)
    {
        return 0;
    }

    return octaspire_container_utf8_string_get_c_string(self->errorMessage);
}

