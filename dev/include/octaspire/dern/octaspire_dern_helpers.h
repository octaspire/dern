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
#ifndef OCTASPIRE_DERN_HELPERS_H
#define OCTASPIRE_DERN_HELPERS_H

#ifndef OCTASPIRE_DERN_DO_NOT_USE_AMALGAMATED_CORE
    #include "octaspire-core-amalgamated.c"
#else
    #include <octaspire/core/octaspire_map.h>
    #include <octaspire/core/octaspire_memory.h>
#endif

#include "octaspire/dern/octaspire_dern_value.h"

#ifdef __cplusplus
extern "C"       {
#endif

int octaspire_dern_helpers_compare_value_hash_maps(
    octaspire_map_t const * const firstValueHashMap,
    octaspire_map_t const * const otherValueHashMap);

double octaspire_dern_helpers_atof(
    char const * const str,
    octaspire_allocator_t * const allocator);

#ifdef __cplusplus
/* extern "C" */ }
#endif

#endif

