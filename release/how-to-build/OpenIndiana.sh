#!/usr/bin/env sh

YELLOW="$(tput setaf 3 ; tput bold)"
GREEN="$(tput setaf 2 ; tput bold)"
NOCOLOR="$(tput setaf 9 ; tput sgr0)"

echoAndRun() { echo "$@" ; "$@" ; }
echoToDefs() { printf "$NOCOLOR\n" ; }

CC=gcc

printf "$YELLOW\n"
cat << EnDoFmEsSaGe
1. Building stand alone unit test runner to test the release
-------------------------------------------------------------------------------
EnDoFmEsSaGe
echoToDefs
echoAndRun $CC -O2 -std=c99 -Wall -Wextra                 \
    -DOCTASPIRE_DERN_AMALGAMATED_UNIT_TEST_IMPLEMENTATION \
    -DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS                \
    -DGREATEST_ENABLE_ANSI_COLORS                         \
-I . octaspire-dern-amalgamated.c -lm                     \
-o octaspire-dern-unit-test-runner



printf "$YELLOW\n"
cat << EnDoFmEsSaGe
2. Building the embedding example
-------------------------------------------------------------------------------
EnDoFmEsSaGe
echoToDefs
echoAndRun $CC -O2 -std=c99 -Wall -Wextra                 \
    -DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS                \
-I . examples/embedding-example.c -lm                     \
-o embedding-example



printf "$YELLOW\n"
cat << EnDoFmEsSaGe
3. Building the binary library example
-------------------------------------------------------------------------------
EnDoFmEsSaGe
echoToDefs
echoAndRun $CC -O2 -std=c99 -Wall -Wextra -fPIC -I . -c examples/mylib.c
echoAndRun $CC -O2 -std=c99 -Wall -Wextra -shared -I . -o libmylib.so mylib.o



printf "$YELLOW\n"
cat << EnDoFmEsSaGe
4. Building the interactive Dern REPL
-------------------------------------------------------------------------------
EnDoFmEsSaGe
echoToDefs
echoAndRun $CC -O2 -std=c99 -Wall -Wextra                 \
    -DOCTASPIRE_DERN_AMALGAMATED_REPL_IMPLEMENTATION      \
    -DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS                \
-I . octaspire-dern-amalgamated.c -lm                     \
-o octaspire-dern-repl



printf "$YELLOW\n"
cat << EnDoFmEsSaGe
5. Building the 'dern_ncurses' (binary) plugin.  PLEASE NOTE: This plugin
   requires development version of 'ncurses' library (i.e. headers) to be
   installed on the system; otherwise compilation will fail. Failure will
   not affect other steps, so if this step fails and you don't want to use
   binary plugin 'dern_ncurses', you don't have to do anything. Otherwise,
   to install development version of library 'ncurses':

       - OpenIndiana: sudo pkg install ncurses
-------------------------------------------------------------------------------
EnDoFmEsSaGe
echoToDefs
echoAndRun $CC -O2 -std=c99 -Wall -Wextra -fPIC -I . -c plugins/dern_ncurses.c
echoAndRun $CC -O2 -std=c99 -Wall -Wextra -shared -I . -o libdern_ncurses.so dern_ncurses.o -lcurses



printf "\nDone.\n$GREEN"
echo   "=================================================================="
echo   "Run programs and examples like this:"
echo   "=================================================================="
printf "%b1)%b ./octaspire-dern-unit-test-runner\n" $YELLOW $GREEN
printf "%b2)%b ./embedding-example\n" $YELLOW $GREEN
printf "%b3)%b LD_LIBRARY_PATH=. ./octaspire-dern-repl examples/use-mylib.dern\n" $YELLOW $GREEN
printf "%b4)%b ./octaspire-dern-repl -c\n" $YELLOW $GREEN
printf "%b5)%b LD_LIBRARY_PATH=. ./octaspire-dern-repl examples/dern-ncurses-example.dern\n" $YELLOW $GREEN
echo "=================================================================="
echoToDefs

