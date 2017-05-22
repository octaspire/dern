#!/usr/bin/env sh

FILE=octaspire_dern_amalgamated.c

# Remove old file, if created
rm -rf $FILE

# Write license and the top-part of the include guards
cat >> $FILE << EnDoFmEsSaGe
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
*******************************************************************************

  This file is amalgamated version of the header files of Octaspire Dern.
  It is created automatically by a script.

                            DO NOT EDIT MANUALLY!

  Edit the separate header files and then let the script create this file.

******************************************************************************/
#ifndef OCTASPIRE_DERN_AMALGAMATED_H
#define OCTASPIRE_DERN_AMALGAMATED_H

EnDoFmEsSaGe


# Write system includes
cat >> $FILE << EnDoFmEsSaGe
#include <dlfcn.h>
#include <stdbool.h>
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


EnDoFmEsSaGe





concat_file() {
INFILE=$1
cat >> $FILE << EnDoFmEsSaGe
//////////////////////////////////////////////////////////////////////////////////////////////////
// START OF        $INFILE
//////////////////////////////////////////////////////////////////////////////////////////////////
EnDoFmEsSaGe
sed -n '/#include/!p' $INFILE >> $FILE
cat >> $FILE << EnDoFmEsSaGe
//////////////////////////////////////////////////////////////////////////////////////////////////
// END OF          $INFILE
//////////////////////////////////////////////////////////////////////////////////////////////////



EnDoFmEsSaGe
}


concat_file_literally() {
INFILE=$1
cat >> $FILE << EnDoFmEsSaGe
//////////////////////////////////////////////////////////////////////////////////////////////////
// START OF        $INFILE
//////////////////////////////////////////////////////////////////////////////////////////////////
EnDoFmEsSaGe
cat $INFILE >> $FILE
cat >> $FILE << EnDoFmEsSaGe
//////////////////////////////////////////////////////////////////////////////////////////////////
// END OF          $INFILE
//////////////////////////////////////////////////////////////////////////////////////////////////



EnDoFmEsSaGe
}




cat >> $FILE << EnDoFmEsSaGe
#ifdef OCTASPIRE_DERN_AMALGAMATED_UNIT_TEST_IMPLEMENTATION
#define OCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION 1
#endif

#ifdef OCTASPIRE_DERN_AMALGAMATED_REPL_IMPLEMENTATION
#define OCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION 1
#endif

#ifdef OCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION
#define OCTASPIRE_CORE_AMALGAMATED_IMPLEMENTATION 1
#endif


EnDoFmEsSaGe




concat_file "../external/octaspire_core/etc/octaspire_core_amalgamated.c"

concat_file "../build/include/octaspire/dern/octaspire_dern_config.h"
concat_file "../include/octaspire/dern/octaspire_dern_lexer.h"
concat_file "../include/octaspire/dern/octaspire_dern_port.h"
concat_file "../include/octaspire/dern/octaspire_dern_value.h"
concat_file "../include/octaspire/dern/octaspire_dern_environment.h"
concat_file "../include/octaspire/dern/octaspire_dern_lib.h"
concat_file "../include/octaspire/dern/octaspire_dern_vm.h"
concat_file "../include/octaspire/dern/octaspire_dern_stdlib.h"









cat >> $FILE << EnDoFmEsSaGe
#ifdef __cplusplus
extern "C" {
#endif

EnDoFmEsSaGe



# Write the bottom-part of the include guards
cat >> $FILE << EnDoFmEsSaGe
#ifdef __cplusplus
}
#endif

#endif

EnDoFmEsSaGe













# Write implementations
cat >> $FILE << EnDoFmEsSaGe







#ifdef OCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION

#define OCTASPIRE_CORE_AMALGAMATED_IMPLEMENTATION 1

EnDoFmEsSaGe




concat_file "../src/octaspire_dern_environment.c"
concat_file "../src/octaspire_dern_lexer.c"
concat_file "../src/octaspire_dern_lib.c"
concat_file "../src/octaspire_dern_port.c"
concat_file "../src/octaspire_dern_stdlib.c"
concat_file "../src/octaspire_dern_value.c"
concat_file "../src/octaspire_dern_vm.c"

cat >> $FILE << EnDoFmEsSaGe

#endif // OCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION

EnDoFmEsSaGe




# Write implementations
cat >> $FILE << EnDoFmEsSaGe

#ifdef OCTASPIRE_DERN_AMALGAMATED_REPL_IMPLEMENTATION


#define OCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION 1

EnDoFmEsSaGe

concat_file "../external/octaspire_dern_banner_color.h"
concat_file "../external/octaspire_dern_banner_white.h"
concat_file "../src/octaspire_dern_repl.c"


# Write implementations
cat >> $FILE << EnDoFmEsSaGe
#endif // OCTASPIRE_DERN_AMALGAMATED_REPL_IMPLEMENTATION

EnDoFmEsSaGe














# Write unit tests
cat >> $FILE << EnDoFmEsSaGe







#ifdef OCTASPIRE_DERN_AMALGAMATED_UNIT_TEST_IMPLEMENTATION

EnDoFmEsSaGe



concat_file_literally "../external/greatest.h"

concat_file "../test/test_dern_lexer.c"
concat_file "../test/test_dern_vm.c"


cat >> $FILE << EnDoFmEsSaGe

GREATEST_MAIN_DEFS();

int main(int argc, char **argv)
{
    GREATEST_MAIN_BEGIN();
    RUN_SUITE(octaspire_dern_lexer_suite);
    RUN_SUITE(octaspire_dern_vm_suite);
    GREATEST_MAIN_END();
}


#endif // OCTASPIRE_DERN_AMALGAMATED_UNIT_TEST_IMPLEMENTATION

EnDoFmEsSaGe



