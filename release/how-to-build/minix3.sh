#!/usr/bin/env sh

echoAndRun() { echo "$@" ; "$@" ; }
echoToDefs() { echo "$(tput setaf 9) $(tput sgr0)" ; }

CC=clang

echo "$(tput setaf 3) $(tput bold)"
cat << EnDoFmEsSaGe
1. Building stand alone unit test runner to test the release
-------------------------------------------------------------------------------
EnDoFmEsSaGe
echoToDefs
echoAndRun $CC -O2 -std=c99 -Wall -Wextra                 \
    -DOCTASPIRE_DERN_AMALGAMATED_UNIT_TEST_IMPLEMENTATION \
    -DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS                \
    -DGREATEST_ENABLE_ANSI_COLORS                         \
-I . octaspire-dern-amalgamated.c -Wl,-export-dynamic -lm \
-o octaspire-dern-unit-test-runner



echo "$(tput setaf 3) $(tput bold)"
cat << EnDoFmEsSaGe
2. Building the embedding example
-------------------------------------------------------------------------------
EnDoFmEsSaGe
echoToDefs
echoAndRun $CC -O2 -std=c99 -Wall -Wextra                 \
    -DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS                \
-I . examples/embedding-example.c -Wl,-export-dynamic -lm \
-o embedding-example



echo "$(tput setaf 3) $(tput bold)"
cat << EnDoFmEsSaGe
3. Building the binary library example
-------------------------------------------------------------------------------
EnDoFmEsSaGe
echoToDefs
echoAndRun $CC -O2 -std=c99 -Wall -Wextra -fPIC -I . -c examples/mylib.c
echoAndRun $CC -O2 -std=c99 -Wall -Wextra -shared -I . -o libmylib.so mylib.o



echo "$(tput setaf 3) $(tput bold)"
cat << EnDoFmEsSaGe
4. Building the interactive Dern REPL
-------------------------------------------------------------------------------
EnDoFmEsSaGe
echoToDefs
echoAndRun $CC -O2 -std=c99 -Wall -Wextra                 \
    -DOCTASPIRE_DERN_AMALGAMATED_REPL_IMPLEMENTATION      \
    -DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS                \
-I . octaspire-dern-amalgamated.c -Wl,-export-dynamic -lm \
-o octaspire-dern-repl



echo "$(tput setaf 3) $(tput bold)"
cat << EnDoFmEsSaGe
5. Building the 'dern_ncurses' (binary) plugin.  PLEASE NOTE: This plugin
   requires development version of 'ncurses' library (i.e. headers) to be
   installed on the system; otherwise compilation will fail. Failure will
   not affect other steps, so if this step fails and you don't want to use
   binary plugin 'dern_ncurses', you don't have to do anything. Otherwise,
   to install development version of library 'ncurses':

       - MINIX 3: su root
                  pkgin install ncurses
                  exit
-------------------------------------------------------------------------------
EnDoFmEsSaGe
echoToDefs
echoAndRun $CC -O2 -std=c99 -Wall -Wextra -fPIC -I . -c plugins/dern_ncurses.c
echoAndRun $CC -O2 -std=c99 -Wall -Wextra -shared -I . -o libdern_ncurses.so dern_ncurses.o -lncurses



echo " "
echo "$(tput bold)Done."
echo "$(tput setaf 2) $(tput bold)"
echo "=================================================================="
echo "Run programs and examples like this:"
echo "=================================================================="
echo "$(tput setaf 3)1)$(tput setaf 2) ./octaspire-dern-unit-test-runner"
echo "$(tput setaf 3)2)$(tput setaf 2) ./embedding-example"
echo "$(tput setaf 3)3)$(tput setaf 2) LD_LIBRARY_PATH=. ./octaspire-dern-repl examples/use-mylib.dern"
echo "$(tput setaf 3)4)$(tput setaf 2) ./octaspire-dern-repl -c"
echo "$(tput setaf 3)5)$(tput setaf 2) LD_LIBRARY_PATH=. ./octaspire-dern-repl examples/dern-ncurses-example.dern"
echo "=================================================================="
echoToDefs

