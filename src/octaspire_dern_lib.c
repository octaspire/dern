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
#include "octaspire/dern/octaspire_dern_config.h"

#ifdef OCTASPIRE_DERN_CONFIG_BINARY_PLUGINS
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#endif

struct octaspire_dern_lib_t
{
    octaspire_memory_allocator_t      *allocator;
    octaspire_dern_vm_t               *vm;
    octaspire_container_utf8_string_t *name;
    octaspire_container_utf8_string_t *errorMessage;
#ifdef OCTASPIRE_DERN_CONFIG_BINARY_PLUGINS
#ifdef _WIN32
    HINSTANCE                          binaryLibHandle;
#else
    void                              *binaryLibHandle;
#endif
#endif
    octaspire_dern_lib_tag_t           typeTag;
    char                               padding[4];
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

#ifdef OCTASPIRE_DERN_CONFIG_BINARY_PLUGINS
#ifdef _WIN32
    /* NOP */
#else
    self->binaryLibHandle = 0;
#endif
#endif

    octaspire_dern_value_t *value =
        octaspire_dern_vm_read_from_octaspire_input_and_eval_in_global_environment(vm, input);

    octaspire_helpers_verify_not_null(value);

    if (value->typeTag == OCTASPIRE_DERN_VALUE_TAG_ERROR)
    {
        self->errorMessage =
            octaspire_container_utf8_string_new_copy(value->value.error, self->allocator);

        octaspire_helpers_verify_not_null(self->errorMessage);
    }

    return self;
}

#ifdef _WIN32
static char const *octaspire_dern_lib_private_format_win32_error_message(void)
{
    static char msgBuf[512];
    memset(msgBuf, 0, sizeof(msgBuf));

    int const errorCode = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        0,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        msgBuf,
        sizeof(msgBuf),
        0);

    if (strlen(msgBuf) == 0)
    {
        snprintf(msgBuf, sizeof(msgBuf), "%d", errorCode);
    }

    return msgBuf;
}
#endif

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

#ifdef OCTASPIRE_DERN_CONFIG_BINARY_PLUGINS
#ifdef _WIN32
    self->binaryLibHandle = LoadLibrary(fileName);

    if (!self->binaryLibHandle)
    {
        self->errorMessage =
            octaspire_container_utf8_string_new_format(
                self->allocator,
                "Binary library (name='%s' fileName='%s')\n"
                "cannot be loaded/opened with LoadLibrary: %s",
                name,
                fileName,
                octaspire_dern_lib_private_format_win32_error_message());

        octaspire_helpers_verify_not_null(self->errorMessage);
    }
    else
    {
        bool (*libInitFunc)(octaspire_dern_vm_t * const, octaspire_dern_environment_t * const);

        octaspire_container_utf8_string_t *libInitFuncName =
            octaspire_container_utf8_string_new_format(self->allocator, "%s_init", name);

        octaspire_helpers_verify_not_null(libInitFuncName);

        libInitFunc =
            (bool (*)(octaspire_dern_vm_t * const, octaspire_dern_environment_t * const))GetProcAddress(
                self->binaryLibHandle,
                octaspire_container_utf8_string_get_c_string(libInitFuncName));

        octaspire_container_utf8_string_release(libInitFuncName);
        libInitFuncName = 0;

        if (!libInitFunc)
        {
            self->errorMessage =
                octaspire_container_utf8_string_new_format(
                    self->allocator,
                    "Binary library (name='%s' fileName='%s'):\n"
                    "GetProcAddress failed on the init function for the library.",
                    name,
                    fileName);

            octaspire_helpers_verify_not_null(self->errorMessage);
        }
        else
        {
            if (!(*libInitFunc)(
                    vm,
                    octaspire_dern_vm_get_global_environment(vm)->value.environment))
            {
                self->errorMessage =
                    octaspire_container_utf8_string_new_format(
                        self->allocator,
                        "Binary library (name='%s' fileName='%s'):\n"
                        "init function failed.",
                        name,
                        fileName);

                octaspire_helpers_verify_not_null(self->errorMessage);
            }
        }
    }
#else
        // Clear any old errors
        dlerror();

        self->binaryLibHandle = dlopen(fileName, RTLD_LAZY);

        if (!self->binaryLibHandle)
        {
            self->errorMessage =
                octaspire_container_utf8_string_new_format(
                    self->allocator,
                    "Binary library (name='%s' fileName='%s')\n"
                    "cannot be loaded/opened with dlopen. dlerror is: %s",
                    name,
                    fileName,
                    dlerror());

            octaspire_helpers_verify_not_null(self->errorMessage);
        }
        else
        {
            bool (*libInitFunc)(octaspire_dern_vm_t * const, octaspire_dern_environment_t * const);

            octaspire_container_utf8_string_t *libInitFuncName =
                octaspire_container_utf8_string_new_format(self->allocator, "%s_init", name);

            octaspire_helpers_verify_not_null(libInitFuncName);

            libInitFunc =
                (bool (*)(octaspire_dern_vm_t * const, octaspire_dern_environment_t * const))dlsym(
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
                        "dlsym failed on the init function for the library. dlerror is: %s",
                        name,
                        fileName,
                        error);

                octaspire_helpers_verify_not_null(self->errorMessage);

                //dlclose(self->binaryLibHandle);
                //self->binaryLibHandle = 0;
            }
            else
            {
                if (!(*libInitFunc)(
                        vm,
                        octaspire_dern_vm_get_global_environment(vm)->value.environment))
                {
                    self->errorMessage =
                        octaspire_container_utf8_string_new_format(
                            self->allocator,
                            "Binary library (name='%s' fileName='%s'):\n"
                            "init function failed.",
                            name,
                            fileName);

                    octaspire_helpers_verify_not_null(self->errorMessage);

                    //dlclose(self->binaryLibHandle);
                    //self->binaryLibHandle = 0;
                }
            }
        }
#endif
#else

        self->errorMessage =
            octaspire_container_utf8_string_new_format(
                self->allocator,
                "Your version of Octaspire Dern is compiled\n"
                "WITHOUT support for binary libraries and this binary library\n"
                "(name='%s' fileName='%s')\n"
                "cannot thus be loaded. Use source-code libraries instead\n"
                "or compile Dern again with binary library support.\n",
                name,
                fileName);

        octaspire_helpers_verify_not_null(self->errorMessage);

#endif

        return self;
}

void octaspire_dern_lib_release(octaspire_dern_lib_t *self)
{
    if (!self)
    {
        return;
    }

    if (self->typeTag == OCTASPIRE_DERN_LIB_TAG_BINARY)
    {
#ifdef OCTASPIRE_DERN_CONFIG_BINARY_PLUGINS
#ifdef _WIN32
        if (self->binaryLibHandle)
        {
            bool (*libCleanFunc)(octaspire_dern_vm_t * const, octaspire_dern_environment_t * const);

            octaspire_container_utf8_string_t *libCleanFuncName =
                octaspire_container_utf8_string_new_format(self->allocator, "%s_clean", name);

            octaspire_helpers_verify_not_null(libCleanFuncName);

            libCleanFunc =
                (bool (*)(octaspire_dern_vm_t * const, octaspire_dern_environment_t * const))GetProcAddress(
                    self->binaryLibHandle,
                    octaspire_container_utf8_string_get_c_string(libCleanFuncName));

            octaspire_container_utf8_string_release(libCleanFuncName);
            libCleanFuncName = 0;

            if (!libCleanFunc)
            {
                self->errorMessage =
                    octaspire_container_utf8_string_new_format(
                        self->allocator,
                        "Binary library (name='%s' fileName='%s'):\n"
                        "GetProcAddress failed on the clean function for the library.",
                        name,
                        fileName);

                octaspire_helpers_verify_not_null(self->errorMessage);
            }
            else
            {
                if (!(*libCleanFunc)(
                        vm,
                        octaspire_dern_vm_get_global_environment(vm)->value.environment))
                {
                    self->errorMessage =
                        octaspire_container_utf8_string_new_format(
                            self->allocator,
                            "Binary library (name='%s' fileName='%s'):\n"
                            "clean function failed.",
                            name,
                            fileName);

                    octaspire_helpers_verify_not_null(self->errorMessage);
                }
            }

            FreeLibrary(self->binaryLibHandle);
            self->binaryLibHandle = 0;
        }
#else
        if (self->binaryLibHandle)
        {
            bool (*libCleanFunc)(octaspire_dern_vm_t * const, octaspire_dern_environment_t * const);

            octaspire_container_utf8_string_t *libCleanFuncName =
                octaspire_container_utf8_string_new_format(
                    self->allocator,
                    "%s_clean",
                    octaspire_container_utf8_string_get_c_string(self->name));

            octaspire_helpers_verify_not_null(libCleanFuncName);

            libCleanFunc =
                (bool (*)(octaspire_dern_vm_t * const, octaspire_dern_environment_t * const))dlsym(
                    self->binaryLibHandle,
                    octaspire_container_utf8_string_get_c_string(libCleanFuncName));

            octaspire_container_utf8_string_release(libCleanFuncName);
            libCleanFuncName = 0;

            char *error = dlerror();

            if (error)
            {
                self->errorMessage =
                    octaspire_container_utf8_string_new_format(
                        self->allocator,
                        "Binary library (name='%s'):\n"
                        "dlsym failed on the clean function for the library. dlerror is: %s",
                        octaspire_container_utf8_string_get_c_string(self->name),
                        error);

                octaspire_helpers_verify_not_null(self->errorMessage);

                //dlclose(self->binaryLibHandle);
                //self->binaryLibHandle = 0;
            }
            else
            {
                if (!(*libCleanFunc)(
                        self->vm,
                        octaspire_dern_vm_get_global_environment(self->vm)->value.environment))
                {
                    self->errorMessage =
                        octaspire_container_utf8_string_new_format(
                            self->allocator,
                            "Binary library (name='%s'):\n"
                            "clean function failed.",
                            octaspire_container_utf8_string_get_c_string(self->name));

                    octaspire_helpers_verify_not_null(self->errorMessage);

                    //dlclose(self->binaryLibHandle);
                    //self->binaryLibHandle = 0;
                }
            }

            dlclose(self->binaryLibHandle);
            self->binaryLibHandle = 0;
        }
#endif
#endif
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

void *octaspire_dern_lib_get_handle(octaspire_dern_lib_t * const self)
{
#ifdef OCTASPIRE_DERN_CONFIG_BINARY_PLUGINS
#ifdef _WIN32
    return &(self->binaryLibHandle);
#else
    return self->binaryLibHandle;
#endif
#else
    OCTASPIRE_HELPERS_UNUSED_PARAMETER(self);
    return 0;
#endif
}

