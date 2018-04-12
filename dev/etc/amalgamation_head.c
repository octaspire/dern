/******************************************************************************
Octaspire Dern - Programming language
Copyright 2017, 2018 www.octaspire.com

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*******************************************************************************

  This file is amalgamated version of the header files, implementation files
  and unit tests of Octaspire Dern. It is created automatically by a script.

                            DO NOT EDIT MANUALLY!

  Edit the separate .h and .c files in the source distribution and then let
  the script create this file again with the modifications.

******************************************************************************/
#ifndef OCTASPIRE_DERN_AMALGAMATED_H
#define OCTASPIRE_DERN_AMALGAMATED_H

#ifdef OCTASPIRE_DERN_CONFIG_BINARY_PLUGINS
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#endif


#ifdef OCTASPIRE_PLAN9_IMPLEMENTATION

#include <u.h>
#include <libc.h>
#include <mp.h>
#include <stdio.h>
#include <ctype.h>

#else

#ifndef OCTASPIRE_DERN_AMALGAMATED_NO_BOOL
#include <stdbool.h>
#endif
#include <stddef.h>
#include <stdint.h>

#include <string.h>
#include <inttypes.h>
#include <ctype.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <stdarg.h>
#include <limits.h>
#include <wchar.h>
#include <locale.h>

#endif

#ifdef OCTASPIRE_DERN_AMALGAMATED_UNIT_TEST_IMPLEMENTATION
#define OCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION 1
#endif

#ifdef OCTASPIRE_DERN_AMALGAMATED_REPL_IMPLEMENTATION
#define OCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION 1
#endif

#ifdef OCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION
#define OCTASPIRE_CORE_AMALGAMATED_IMPLEMENTATION 1
#endif

#ifdef __cplusplus
extern "C" {
#endif
