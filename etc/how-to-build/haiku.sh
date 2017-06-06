echoAndRun() { echo "$@" ; "$@" ; }


echo " "
cat << EnDoFmEsSaGe
1. Building stand alone unit test runner to test the release
-------------------------------------------------------------------------------
EnDoFmEsSaGe
echoAndRun gcc-x86 -O2 -std=c99 -Wall -Wextra             \
    -DOCTASPIRE_DERN_AMALGAMATED_UNIT_TEST_IMPLEMENTATION \
    -DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS                \
    -DGREATEST_ENABLE_ANSI_COLORS                         \
-I . octaspire-dern-amalgamated.c -Wl,-export-dynamic -lm \
-o octaspire-dern-unit-test-runner



echo " "
cat << EnDoFmEsSaGe
2. Building the interactive Dern REPL
-------------------------------------------------------------------------------
EnDoFmEsSaGe
echoAndRun gcc-x86 -O2 -std=c99 -Wall -Wextra             \
    -DOCTASPIRE_DERN_AMALGAMATED_REPL_IMPLEMENTATION      \
    -DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS                \
-I . octaspire-dern-amalgamated.c -Wl,-export-dynamic -lm \
-o octaspire-dern-repl



echo " "
cat << EnDoFmEsSaGe
 3. Building the embedding example
-------------------------------------------------------------------------------
EnDoFmEsSaGe
echoAndRun gcc-x86 -O2 -std=c99 -Wall -Wextra             \
    -DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS                \
-I . examples/embedding-example.c -Wl,-export-dynamic -lm \
-o embedding-example



echo " "
cat << EnDoFmEsSaGe
 4. Building the binary library example
-------------------------------------------------------------------------------
EnDoFmEsSaGe
echoAndRun gcc-x86 -O2 -std=c99 -Wall -Wextra -fPIC -I . -c examples/mylib.c
echoAndRun gcc-x86 -O2 -std=c99 -Wall -Wextra -shared -I . -o libmylib.so mylib.o

echo "\nDone.\n"
echo "==============================================================="
echo "Run programs and examples like this:"
echo "===============================================================\n"
echo "./octaspire-dern-unit-test-runner"
echo "./embedding-example"
echo "LIBRARY_PATH=\$LIBRARY_PATH:. ./octaspire-dern-repl examples/use-mylib.dern"
echo "./octaspire-dern-repl -c"
echo "===============================================================\n"


