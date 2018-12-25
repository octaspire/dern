#!/usr/bin/env sh

BLUE='\033[0;34m'
RED='\033[0;31m'
GREEN='\033[0;32m'
BOLD='\033[1m'
NOCOLOR='\033[0m'

CC=gcc
COVERAGE=""

EXAMPLE_NAME=""
EXAMPLE_ERROR_HINT=""
EXAMPLE_SUCCESS_RUN=""

echoAndRun() {
    printf "%bBuilding %b$EXAMPLE_NAME...%b" "$BLUE" "$BOLD" "$NOCOLOR"

    if ! "$@" > /dev/null; then
        printf "\n%bFAIL. POTENTIAL FIX: %b$EXAMPLE_ERROR_HINT\n"  "$RED" "$BOLD"
    else
        printf "%b\n" "$BLUE"
        for i in "$@"
        do
            printf "%s " "$i"
        done
        printf "\n%bRUN WITH %b$EXAMPLE_SUCCESS_RUN\n" "$GREEN" "$BOLD"
    fi
    printf "%b\n" "$NOCOLOR"
}

if [ "$#" -ge "1" ]; then CC=$1; fi
if [ "$2" = "--coverage" ]; then COVERAGE=$2; fi



EXAMPLE_NAME="stand alone unit test runner"
EXAMPLE_ERROR_HINT="Install $CC compiler?"
EXAMPLE_SUCCESS_RUN="./octaspire-dern-unit-test-runner"
echoAndRun "$CC" -O2 -std=c99 -Wall -Wextra -DOCTASPIRE_DERN_AMALGAMATED_UNIT_TEST_IMPLEMENTATION -DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS -DGREATEST_ENABLE_ANSI_COLORS $COVERAGE -I . octaspire-dern-amalgamated.c -Wl,-export-dynamic -lm -o octaspire-dern-unit-test-runner



EXAMPLE_NAME="embedding example"
EXAMPLE_ERROR_HINT="Install $CC compiler?"
EXAMPLE_SUCCESS_RUN="./embedding-example"
echoAndRun "$CC" -O2 -std=c99 -Wall -Wextra -DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS -I . examples/embedding-example.c -Wl,-export-dynamic -lm -o embedding-example



EXAMPLE_NAME="binary library example"
EXAMPLE_ERROR_HINT="Install $CC compiler?"
EXAMPLE_SUCCESS_RUN="LD_LIBRARY_PATH=. ./octaspire-dern-repl examples/use-mylib.dern"
echoAndRun "$CC" -O2 -std=c99 -Wall -Wextra -fPIC -shared -I . -o libmylib.so examples/mylib.c



EXAMPLE_NAME="interactive Dern REPL"
EXAMPLE_ERROR_HINT="Install $CC compiler?"
EXAMPLE_SUCCESS_RUN="./octaspire-dern-repl -c"
echoAndRun "$CC" -O2 -std=c99 -Wall -Wextra -DOCTASPIRE_DERN_AMALGAMATED_REPL_IMPLEMENTATION -DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS -I . octaspire-dern-amalgamated.c -Wl,-export-dynamic -lm -o octaspire-dern-repl



EXAMPLE_NAME="Dern socket plugin"
EXAMPLE_ERROR_HINT="Install $CC compiler?"
EXAMPLE_SUCCESS_RUN="LD_LIBRARY_PATH=. ./octaspire-dern-repl examples/irc-client.dern"
echoAndRun "$CC" -O2 -std=c99 -Wall -Wextra -fPIC -shared -I . -o libdern_socket.so plugins/dern_socket.c



EXAMPLE_NAME="Dern dir plugin"
EXAMPLE_ERROR_HINT="Install $CC compiler?"
EXAMPLE_SUCCESS_RUN="LD_LIBRARY_PATH=. ./octaspire-dern-repl examples/dern-dir-example.dern"
echoAndRun "$CC" -O2 -std=c99 -Wall -Wextra -fPIC -shared -I . -shared -o libdern_dir.so plugins/dern_dir.c



EXAMPLE_NAME="Dern easing plugin"
EXAMPLE_ERROR_HINT="Install $CC compiler?"
EXAMPLE_SUCCESS_RUN="LD_LIBRARY_PATH=. ./octaspire-dern-repl examples/dern-easing-example.dern"
echoAndRun "$CC" -O2 -std=c99 -Wall -Wextra -fPIC -shared -I . -shared -o libdern_easing.so plugins/dern_easing.c



EXAMPLE_NAME="Dern animation plugin"
EXAMPLE_ERROR_HINT="Install $CC compiler?"
EXAMPLE_SUCCESS_RUN="LD_LIBRARY_PATH=. ./octaspire-dern-repl examples/dern-animation-example.dern"
echoAndRun "$CC" -O2 -std=c99 -Wall -Wextra -fPIC -shared -I . -shared -o libdern_animation.so plugins/dern_animation.c



EXAMPLE_NAME="Dern ncurses plugin"
EXAMPLE_ERROR_HINT="OpenBSD: install ncurses library"
EXAMPLE_SUCCESS_RUN="LD_LIBRARY_PATH=. ./octaspire-dern-repl examples/dern-ncurses-example.dern"
echoAndRun "$CC" -O2 -std=c99 -Wall -Wextra -fPIC -I . -shared -o libdern_ncurses.so plugins/dern_ncurses.c -lncurses



EXAMPLE_NAME="Dern SDL2 plugin"
EXAMPLE_ERROR_HINT="OpenBSD: sudo pkg_add sdl2 sdl2-image sdl2-mixer sdl2-ttf"
EXAMPLE_SUCCESS_RUN="LD_LIBRARY_PATH=. ./octaspire-dern-repl examples/dern-sdl2-example.dern"
echoAndRun "$CC" -O2 -std=c99 -Wall -Wextra -fPIC -shared -DSDL_DISABLE_IMMINTRIN_H -DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_IMAGE_LIBRARY -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY $(sdl2-config --cflags) -I . -o libdern_sdl2.so plugins/dern_sdl2.c $(sdl2-config --libs) -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lGLU



EXAMPLE_NAME="Dern nuklear plugin"
EXAMPLE_ERROR_HINT="OpenBSD: sudo pkg_add sdl2 sdl2-image sdl2-mixer sdl2-ttf"
EXAMPLE_SUCCESS_RUN="LD_LIBRARY_PATH=. ./octaspire-dern-repl examples/dern-nuklear-example.dern"
echoAndRun "$CC" -O2 -std=c99 -Wall -Wextra -fPIC -shared -DSDL_DISABLE_IMMINTRIN_H -DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_IMAGE_LIBRARY -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY $(sdl2-config --cflags) -I . -I plugins/external/nuklear -o libdern_nuklear.so plugins/dern_nuklear.c $(sdl2-config --libs) -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lGLU



# Build Chipmunk library and plugin.

printf "${BLUE}Compiling ${BOLD}Chipmunk library${NOCOLOR}${BLUE}"
for srcFile in $(ls plugins/external/chipmunk/src/*.c)
do
    printf "."
    "$CC" -O2 -std=c99 -Wall -Wextra -fPIC -I plugins/external/chipmunk/include -I plugins/external/chipmunk/include/chipmunk -c "$srcFile" -o "${srcFile%.c}.o"
done

printf "\rLinking ${BOLD}Chipmunk library${NOCOLOR}${BLUE}...${NOCOLOR}                                           \n\n"
"$CC" -shared -o libchipmunk.so plugins/external/chipmunk/src/*.o -lm -lpthread

EXAMPLE_NAME="Dern Chipmunk plugin"
EXAMPLE_ERROR_HINT="Install $CC compiler?"
EXAMPLE_SUCCESS_RUN="LD_LIBRARY_PATH=. ./octaspire-dern-repl examples/dern-chipmunk-example.dern"
echoAndRun "$CC" -std=c99 -Wall -Wextra -g -O2 -DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS -I . -I plugins/external/chipmunk/include/ -I plugins/external/chipmunk/include/chipmunk -fPIC -shared -DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION -o libdern_chipmunk.so plugins/dern_chipmunk.c -lm -lpthread -L . -lchipmunk
