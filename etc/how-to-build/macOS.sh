#!/usr/bin/env sh

YELLOW="$(tput setaf 3 ; tput bold)"
GREEN="$(tput setaf 2 ; tput bold)"
NOCOLOR="$(tput setaf 9 ; tput sgr0)"

CC=clang

echoAndRun() { echo "$@" ; "$@" ; }
echoToDefs() { printf "$NOCOLOR\n" ; }

if [ "$#" -ge "1" ]; then CC=$1; fi



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
4. Building the interactive Dern REPL
-------------------------------------------------------------------------------
EnDoFmEsSaGe
echoToDefs
echoAndRun $CC -O2 -std=c99 -Wall -Wextra                 \
    -DOCTASPIRE_DERN_AMALGAMATED_REPL_IMPLEMENTATION      \
    -DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS                \
-I . octaspire-dern-amalgamated.c -lm                     \
-o octaspire-dern-repl



printf "\nDone.\n$GREEN"
echo   "=================================================================="
echo   "Run programs and examples like this:"
echo   "=================================================================="
printf "%b1)%b ./octaspire-dern-unit-test-runner\n" $YELLOW $GREEN
printf "%b2)%b ./embedding-example\n" $YELLOW $GREEN
printf "%b4)%b ./octaspire-dern-repl -c\n" $YELLOW $GREEN
echo "=================================================================="
echoToDefs

