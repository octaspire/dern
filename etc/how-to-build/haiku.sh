echoAndRun() { echo "$@" ; "$@" ; }
echoToDefs() { echo "$(tput setaf 9) $(tput sgr0)" ; }

CC=gcc-x86

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



echo " "
echo "$(tput bold)Done."
echo "$(tput setaf 2) $(tput bold)"
echo "============================================================================="
echo "Run programs and examples like this:"
echo "============================================================================="
echo "$(tput setaf 3)1)$(tput setaf 2) ./octaspire-dern-unit-test-runner"
echo "$(tput setaf 3)2)$(tput setaf 2) ./embedding-example"
echo "$(tput setaf 3)3)$(tput setaf 2) LIBRARY_PATH=\$LIBRARY_PATH:. ./octaspire-dern-repl examples/use-mylib.dern"
echo "$(tput setaf 3)4)$(tput setaf 2) ./octaspire-dern-repl -c"
echo "============================================================================="
echoToDefs

