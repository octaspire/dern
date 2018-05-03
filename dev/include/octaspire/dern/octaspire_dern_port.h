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
#ifndef OCTASPIRE_DERN_PORT_H
#define OCTASPIRE_DERN_PORT_H

#include <stdint.h>

#ifndef OCTASPIRE_DERN_DO_NOT_USE_AMALGAMATED_CORE
    #include "octaspire-core-amalgamated.c"
#else
    #include <octaspire/core/octaspire_memory.h>
    #include <octaspire/core/octaspire_string.h>
#endif

#ifdef __cplusplus
extern "C"       {
#endif

typedef enum
{
    OCTASPIRE_DERN_PORT_TAG_NOT_OPEN,
    OCTASPIRE_DERN_PORT_TAG_INPUT_FILE,
    OCTASPIRE_DERN_PORT_TAG_OUTPUT_FILE,
    OCTASPIRE_DERN_PORT_TAG_IO_FILE
}
octaspire_dern_port_tag_t;

typedef struct octaspire_dern_port_t octaspire_dern_port_t;

octaspire_dern_port_t *octaspire_dern_port_new_copy(
    octaspire_dern_port_t * const other,
    octaspire_allocator_t *allocator);

octaspire_dern_port_t *octaspire_dern_port_new_input_file(
    char const * const path,
    octaspire_allocator_t *allocator);

octaspire_dern_port_t *octaspire_dern_port_new_output_file(
    char const * const path,
    octaspire_allocator_t *allocator);

octaspire_dern_port_t *octaspire_dern_port_new_io_file(
    char const * const path,
    octaspire_allocator_t *allocator);

void octaspire_dern_port_release(octaspire_dern_port_t *self);

ptrdiff_t octaspire_dern_port_write(
    octaspire_dern_port_t * const self,
    void const * const buffer,
    size_t const bufferSizeInOctets);

ptrdiff_t octaspire_dern_port_read(
    octaspire_dern_port_t * const self,
    void * const buffer,
    size_t const bufferSizeInOctets);

bool octaspire_dern_port_close(
    octaspire_dern_port_t * const self);

ptrdiff_t octaspire_dern_port_get_length_in_octets(octaspire_dern_port_t const * const self);

octaspire_string_t *octaspire_dern_port_to_string(
    octaspire_dern_port_t const * const self,
    octaspire_allocator_t * const allocator);

bool octaspire_dern_port_seek(
    octaspire_dern_port_t * const self,
    ptrdiff_t const amount,
    bool const fromCurrentPos);

bool octaspire_dern_port_flush(octaspire_dern_port_t * const self);
ptrdiff_t octaspire_dern_port_distance(octaspire_dern_port_t const * const self);

bool octaspire_dern_port_supports_output(
    octaspire_dern_port_t const * const self);

bool octaspire_dern_port_supports_input(
    octaspire_dern_port_t const * const self);

#ifdef __cplusplus
/* extern "C" */ }
#endif

#endif

