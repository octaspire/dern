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

    if ! "$@" > /dev/null 2>&1; then
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
EXAMPLE_SUCCESS_RUN="octaspire-dern-unit-test-runner.exe"
echoAndRun "$CC" -O2 -std=c99 -Wall -Wextra -DOCTASPIRE_DERN_AMALGAMATED_UNIT_TEST_IMPLEMENTATION -DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS $COVERAGE -I . octaspire-dern-amalgamated.c -lm -lKernel32 -o octaspire-dern-unit-test-runner



EXAMPLE_NAME="embedding example"
EXAMPLE_ERROR_HINT="Install $CC compiler?"
EXAMPLE_SUCCESS_RUN="embedding-example.exe"
echoAndRun "$CC" -O2 -std=c99 -Wall -Wextra -DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS -I . examples/embedding-example.c -lm -lKernel32 -o embedding-example


EXAMPLE_NAME="implementation library for binary library example"
EXAMPLE_ERROR_HINT="Install $CC compiler?"
EXAMPLE_SUCCESS_RUN=""
echoAndRun "$CC" -O2 -std=c99 -Wall -Wextra -DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION -DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS octaspire-dern-amalgamated.c -lKernel32 -shared -Wl,--out-implib=imp.a
EXAMPLE_NAME="binary library example"
EXAMPLE_ERROR_HINT="Install $CC compiler?"
EXAMPLE_SUCCESS_RUN="octaspire-dern-repl.exe examples\\use-mylib.dern"
echoAndRun "$CC" -O2 -std=c99 -Wall -Wextra -fPIC -shared -I . -o libmylib.dll examples/mylib.c -L . imp.a



EXAMPLE_NAME="interactive Dern REPL"
EXAMPLE_ERROR_HINT="Install $CC compiler?"
EXAMPLE_SUCCESS_RUN="octaspire-dern-repl.exe"
echoAndRun "$CC" -O2 -std=c99 -Wall -Wextra -DOCTASPIRE_DERN_AMALGAMATED_REPL_IMPLEMENTATION -DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS -I . octaspire-dern-amalgamated.c -lm -lKernel32 -o octaspire-dern-repl



EXAMPLE_NAME="Dern socket plugin"
EXAMPLE_ERROR_HINT="Install $CC compiler?"
EXAMPLE_SUCCESS_RUN="octaspire-dern-repl examples\\irc-client.dern"
echoAndRun "$CC" -O2 -std=c99 -Wall -Wextra -fPIC -shared -I . -o libdern_socket.dll plugins/dern_socket.c -lws2_32 -L . imp.a



EXAMPLE_NAME="Dern dir plugin"
EXAMPLE_ERROR_HINT="Install $CC compiler?"
EXAMPLE_SUCCESS_RUN="octaspire-dern-repl examples\\dern-dir-example.dern"
echoAndRun "$CC" -O2 -std=c99 -Wall -Wextra -fPIC -shared -I . -shared -o libdern_dir.dll plugins/dern_dir.c -lKernel32 -L . imp.a



EXAMPLE_NAME="Dern easing plugin"
EXAMPLE_ERROR_HINT="Install $CC compiler?"
EXAMPLE_SUCCESS_RUN="octaspire-dern-repl examples\\dern-easing-example.dern"
echoAndRun "$CC" -O2 -std=c99 -Wall -Wextra -fPIC -shared -I . -shared -o libdern_easing.dll plugins/dern_easing.c -L . imp.a



EXAMPLE_NAME="Dern animation plugin"
EXAMPLE_ERROR_HINT="Install $CC compiler?"
EXAMPLE_SUCCESS_RUN="octaspire-dern-repl examples\\dern-animation-example.dern"
echoAndRun "$CC" -O2 -std=c99 -Wall -Wextra -fPIC -shared -I . -shared -o libdern_animation.dll plugins/dern_animation.c -L . imp.a



EXAMPLE_NAME="Dern ncurses plugin"
read -rd '' EXAMPLE_ERROR_HINT << EOF
    a) Go to https://pdcurses.sourceforge.io
    b) Download file 'pdc34dllw.zip' into 'release'-directory
    c) Run command: unzip pdc34dllw.zip
    d) Run command: how-to-build/windows.sh
EOF
EXAMPLE_SUCCESS_RUN="octaspire-dern-repl examples\\dern-ncurses-example.dern"
echoAndRun "$CC" -O2 -std=c99 -Wall -Wextra -fPIC -I . -shared -o libdern_ncurses.dll plugins/dern_ncurses.c -L . -lpdcurses imp.a



EXAMPLE_NAME="Dern SDL2 plugin"
read -rd '' EXAMPLE_ERROR_HINT << EOF
    curl -O https://www.libsdl.org/release/SDL2-devel-2.0.7-mingw.tar.gz
    tar xzf SDL2-devel-2.0.7-mingw.tar.gz
    cp SDL2-2.0.7/i686-w64-mingw32/bin/SDL2.dll .
    curl -O https://www.libsdl.org/projects/SDL_image/release/SDL2_image-devel-2.0.2-mingw.tar.gz
    tar xzf SDL2_image-devel-2.0.2-mingw.tar.gz
    cp SDL2_image-2.0.2/i686-w64-mingw32/bin/*.dll .
    curl -O https://www.libsdl.org/projects/SDL_mixer/release/SDL2_mixer-devel-2.0.2-mingw.tar.gz
    tar xzf SDL2_mixer-devel-2.0.2-mingw.tar.gz
    cp SDL2_mixer-2.0.2/i686-w64-mingw32/bin/*.dll .
    curl -O https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-devel-2.0.14-mingw.tar.gz
    tar xzf SDL2_ttf-devel-2.0.14-mingw.tar.gz
    cp SDL2_ttf-2.0.14/i686-w64-mingw32/bin/*.dll .
EOF
EXAMPLE_SUCCESS_RUN="octaspire-dern-repl examples\\dern-sdl2-example.dern"
echoAndRun "$CC" -O2 -std=c99 -Wall -Wextra -fPIC -shared -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_IMAGE_LIBRARY -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY -Dmain=SDL_main -I . -I SDL2-2.0.7/i686-w64-mingw32/include/SDL2 -I SDL2_image-2.0.2/i686-w64-mingw32/include/SDL2 -I SDL2_mixer-2.0.2/i686-w64-mingw32/include/SDL2 -I SDL2_ttf-2.0.14/i686-w64-mingw32/include/SDL2 -o libdern_sdl2.dll plugins/dern_sdl2.c -L . -L SDL2-2.0.7/i686-w64-mingw32/lib -L SDL2_image-2.0.2/i686-w64-mingw32/lib -L SDL2_mixer-2.0.2/i686-w64-mingw32/lib -L SDL2_ttf-2.0.14/i686-w64-mingw32/lib imp.a -lSDL2main -lSDL2 libSDL2_image.a -lSDL2_mixer -lSDL2_ttf

