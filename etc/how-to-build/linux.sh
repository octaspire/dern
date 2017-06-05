echoAndRun() { echo "$@" ; "$@" ; }


echo "\n"
cat << EnDoFmEsSaGe
1. Building stand alone unit test runner to test the release
---------------------------------------------------------------------------------------------------
EnDoFmEsSaGe
echoAndRun gcc -O3 -std=c99 -Wall -Wextra -DOCTASPIRE_DERN_AMALGAMATED_UNIT_TEST_IMPLEMENTATION \
                               -DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS \
-I . octaspire-dern-amalgamated.c -Wl,-export-dynamic -ldl -lm -o octaspire-dern-unit-test-runner



echo "\n"
cat << EnDoFmEsSaGe
2. Building the interactive Dern REPL
---------------------------------------------------------------------------------------------------
EnDoFmEsSaGe
echoAndRun gcc -O3 -std=c99 -Wall -Wextra -DOCTASPIRE_DERN_AMALGAMATED_REPL_IMPLEMENTATION \
                               -DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS \
-I . octaspire-dern-amalgamated.c -Wl,-export-dynamic -ldl -lm -o octaspire-dern-repl



echo "\n"
cat << EnDoFmEsSaGe
 3. Building the embedding example
---------------------------------------------------------------------------------------------------
EnDoFmEsSaGe
echoAndRun gcc -O3 -std=c99 -Wall -Wextra -DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS \
-I . examples/embedding-example.c -Wl,-export-dynamic -ldl -lm -o embedding-example



echo "\n"
cat << EnDoFmEsSaGe
 4. Building the binary library example
---------------------------------------------------------------------------------------------------
EnDoFmEsSaGe
echoAndRun gcc -O3 -std=c99 -Wall -Wextra -fPIC -I . -c examples/mylib.c
echoAndRun gcc -O3 -std=c99 -Wall -Wextra -shared -I . -o libmylib.so mylib.o

echo "\nDone."

