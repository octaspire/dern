#!/usr/bin/env sh

YELLOW="$(tput setaf 3 ; tput bold)"
GREEN="$(tput setaf 2 ; tput bold)"
NOCOLOR="$(tput setaf 9 ; tput sgr0)"

CC=clang
COVERAGE=""

echoAndRun() { echo "$@" ; "$@" ; }
echoToDefs() { printf "$NOCOLOR\n" ; }

if [ "$#" -ge "1" ]; then CC=$1; fi
if [ "$2" = "--coverage" ]; then COVERAGE=$2; fi



printf "$YELLOW\n"
cat << EnDoFmEsSaGe
1. Building stand alone unit test runner to test the release
-------------------------------------------------------------------------------
EnDoFmEsSaGe
echoToDefs
echoAndRun $CC -O2 -std=c99 -Wall -Wextra                      \
    -DOCTASPIRE_DERN_AMALGAMATED_UNIT_TEST_IMPLEMENTATION      \
    -DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS                     \
    -DGREATEST_ENABLE_ANSI_COLORS                              \
    $COVERAGE                                                  \
-I . octaspire-dern-amalgamated.c -lm                          \
-o octaspire-dern-unit-test-runner



printf "$YELLOW\n"
cat << EnDoFmEsSaGe
2. Building the embedding example
-------------------------------------------------------------------------------
EnDoFmEsSaGe
echoToDefs
echoAndRun $CC -O2 -std=c99 -Wall -Wextra                      \
    -DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS                     \
-I . examples/embedding-example.c -lm                          \
-o embedding-example



printf "$YELLOW\n"
cat << EnDoFmEsSaGe
3. Building the binary library example
-------------------------------------------------------------------------------
EnDoFmEsSaGe
echoToDefs
echoAndRun $CC -O2 -std=c99 -Wall -Wextra -dynamiclib          \
    -DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION                \
-I . -o libmylib.dylib examples/mylib.c



printf "$YELLOW\n"
cat << EnDoFmEsSaGe
4. Building the interactive Dern REPL
-------------------------------------------------------------------------------
EnDoFmEsSaGe
echoToDefs
echoAndRun $CC -O2 -std=c99 -Wall -Wextra                      \
    -DOCTASPIRE_DERN_AMALGAMATED_REPL_IMPLEMENTATION           \
    -DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS                     \
-I . octaspire-dern-amalgamated.c -lm                          \
-o octaspire-dern-repl



printf "$YELLOW\n"
cat << EnDoFmEsSaGe
5. Building the 'dern_socket' (binary) plugin.
-------------------------------------------------------------------------------
EnDoFmEsSaGe
echoToDefs
echoAndRun $CC -O2 -std=c99 -Wall -Wextra -dynamiclib          \
    -DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION                \
-I . -o libdern_socket.dylib plugins/dern_socket.c



printf "$YELLOW\n"
cat << EnDoFmEsSaGe
6. Building the 'dern_dir' (binary) plugin.
-------------------------------------------------------------------------------
EnDoFmEsSaGe
echoToDefs
echoAndRun $CC -O2 -std=c99 -Wall -Wextra -dynamiclib          \
    -DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION                \
-I . -o libdern_dir.dylib plugins/dern_dir.c



printf "$YELLOW\n"
cat << EnDoFmEsSaGe
7. Building the 'dern_ncurses' (binary) plugin.  PLEASE NOTE: This plugin
   requires development version of 'ncurses' library (i.e. headers) to be
   installed on the system; otherwise compilation will fail. Failure will
   not affect other steps, so if this step fails and you don't want to use
   binary plugin 'dern_ncurses', you don't have to do anything. Otherwise,
   to install development version of library 'ncurses':

       - macOS: should be already installed. Can also be installed, for
                example, with homebrew: brew install ncurses
-------------------------------------------------------------------------------
EnDoFmEsSaGe
echoToDefs
echoAndRun $CC -O2 -std=c99 -Wall -Wextra -dynamiclib          \
    -DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION                \
-I . -o libdern_ncurses.dylib -lncurses plugins/dern_ncurses.c



printf "\nDone.\n$GREEN"
echo   "=================================================================="
echo   "Run programs and examples like this:"
echo   "=================================================================="
printf "%b1)%b ./octaspire-dern-unit-test-runner\n" $YELLOW $GREEN
printf "%b2)%b ./embedding-example\n" $YELLOW $GREEN
printf "%b3)%b DYLD_LIBRARY_PATH=. ./octaspire-dern-repl examples/use-mylib.dern\n" $YELLOW $GREEN
printf "%b4)%b ./octaspire-dern-repl -c\n" $YELLOW $GREEN
printf "%b5)%b DYLD_LIBRARY_PATH=. ./octaspire-dern-repl examples/dern-sockets-echo-server.dern\n" $YELLOW $GREEN
printf "%b+)%b DYLD_LIBRARY_PATH=. ./octaspire-dern-repl examples/dern-sockets-echo-client.dern\n" $YELLOW $GREEN
printf "%b6)%b DYLD_LIBRARY_PATH=. ./octaspire-dern-repl examples/dern-dir-example.dern\n" $YELLOW $GREEN
printf "%b7)%b DYLD_LIBRARY_PATH=. ./octaspire-dern-repl examples/dern-ncurses-example.dern\n" $YELLOW $GREEN
printf "%b8)%b DYLD_LIBRARY_PATH=. ./octaspire-dern-repl examples/irc-client.dern\n" $YELLOW $GREEN
echo "=================================================================="
echoToDefs

