#!/usr/bin/env sh

YELLOW='\033[1;33m'
GREEN='\033[1;32m'
NOCOLOR='\033[0m'

CC=gcc
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
    $COVERAGE                                                  \
    -I . octaspire-dern-amalgamated.c -lm                      \
    -lKernel32                                                 \
    -o octaspire-dern-unit-test-runner



printf "$YELLOW\n"
cat << EnDoFmEsSaGe
2. Building the embedding example
-------------------------------------------------------------------------------
EnDoFmEsSaGe
echoToDefs
echoAndRun $CC -O2 -std=c99 -Wall -Wextra                      \
    -DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS                     \
    -I . examples/embedding-example.c -lm                      \
    -lKernel32                                                 \
    -o embedding-example



printf "$YELLOW\n"
cat << EnDoFmEsSaGe
3. Building the binary library example
-------------------------------------------------------------------------------
EnDoFmEsSaGe
echoToDefs
echoAndRun $CC                                                 \
    -DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION                \
    -DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS                     \
    octaspire-dern-amalgamated.c                               \
    -lKernel32                                                 \
    -shared -Wl,--out-implib=imp.a

echoAndRun $CC -O2 -std=c99 -Wall -Wextra -fPIC -I . -c examples/mylib.c
echoAndRun $CC -O2 -std=c99 -Wall -Wextra -shared -I .         \
    -o libmylib.dll mylib.o -lKernel32 -L . imp.a



printf "$YELLOW\n"
cat << EnDoFmEsSaGe
4. Building the interactive Dern REPL
-------------------------------------------------------------------------------
EnDoFmEsSaGe
echoToDefs
echoAndRun $CC -O2 -std=c99 -Wall -Wextra                      \
    -DOCTASPIRE_DERN_AMALGAMATED_REPL_IMPLEMENTATION           \
    -DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS                     \
    -I . octaspire-dern-amalgamated.c -lm                      \
    -lKernel32                                                 \
    -o octaspire-dern-repl



printf "$YELLOW\n"
cat << EnDoFmEsSaGe
5. Building the 'dern_socket' (binary) plugin.
-------------------------------------------------------------------------------
EnDoFmEsSaGe
echoToDefs
echoAndRun $CC -O2 -std=c99 -Wall -Wextra -fPIC -I . -c plugins/dern_socket.c
echoAndRun $CC -O2 -std=c99 -Wall -Wextra -shared -I .         \
    -o libdern_socket.dll dern_socket.o -lws2_32 -L . imp.a



printf "$YELLOW\n"
cat << EnDoFmEsSaGe
6. Building the 'dern_dir' (binary) plugin.
-------------------------------------------------------------------------------
EnDoFmEsSaGe
echoToDefs
echoAndRun $CC -O2 -std=c99 -Wall -Wextra -fPIC -I . -c plugins/dern_dir.c
echoAndRun $CC -O2 -std=c99 -Wall -Wextra -shared -I .         \
    -o libdern_dir.dll dern_dir.o -lKernel32 -L . imp.a



printf "$YELLOW\n"
cat << EnDoFmEsSaGe
7. Building the 'dern_ncurses' (binary) plugin.  PLEASE NOTE: This plugin
   requires 'ncurses' library and headers to be installed on the system;
   otherwise compilation will fail. Failure will not affect other steps,
   so if this step fails and you don't want to use  binary plugin
   'dern_ncurses', you don't have to do anything. Otherwise, download
   'PDCurses' library for windows from:

        a) Go to https://pdcurses.sourceforge.io
        b) Download file 'pdc34dllw.zip' into 'release'-directory
        c) Run command: unzip pdc34dllw.zip
        d) Run command: how-to-build/windows.sh
-------------------------------------------------------------------------------
EnDoFmEsSaGe
echoToDefs
echoAndRun $CC -O2 -std=c99 -Wall -Wextra -fPIC -I . -c plugins/dern_ncurses.c
echoAndRun $CC -O2 -std=c99 -Wall -Wextra -shared -I .         \
    -o libdern_ncurses.dll dern_ncurses.o -lKernel32 -L . -lpdcurses imp.a



printf "$YELLOW\n"
cat << EnDoFmEsSaGe
8. Building the 'dern_sdl2' (binary) plugin.  PLEASE NOTE: This plugin
   requires 'sdl2' library and headers to be installed on the system;
   otherwise compilation will fail. Failure will not affect other steps,
   so if this step fails and you don't want to use  binary plugin
   'dern_sdl2', you don't have to do anything. Otherwise, download and
   use 'SDL2' library for windows by giving the following commands:

        a) curl -O https://www.libsdl.org/release/SDL2-devel-2.0.7-mingw.tar.gz
        b) tar xzf SDL2-devel-2.0.7-mingw.tar.gz
        c) cp SDL2-2.0.7/i686-w64-mingw32/bin/SDL2.dll .
        d) how-to-build/windows.sh
-------------------------------------------------------------------------------
EnDoFmEsSaGe
echoToDefs
echoAndRun $CC -O2 -std=c99 -Wall -Wextra -fPIC -I . -I SDL2-2.0.7/i686-w64-mingw32/include/SDL2 -Dmain=SDL_main -c plugins/dern_sdl2.c
echoAndRun $CC -O2 -std=c99 -Wall -Wextra -shared -I .         \
    -o libdern_sdl2.dll dern_sdl2.o -L . imp.a -L SDL2-2.0.7/i686-w64-mingw32/lib -lSDL2



printf "\nDone.\n$GREEN"
echo   "====================================================================="
echo   "Run programs and examples like this using __Windows Command Prompt__:"
echo   "====================================================================="
printf "%b1)%b octaspire-dern-unit-test-runner.exe\n" $YELLOW $GREEN
printf "%b2)%b embedding-example.exe\n" $YELLOW $GREEN
printf '%b3)%b octaspire-dern-repl.exe examples\\use-mylib.dern\n' $YELLOW $GREEN
printf "%b4)%b octaspire-dern-repl.exe\n" $YELLOW $GREEN
printf "%b5)%b octaspire-dern-repl examples\\dern-sockets-echo-server.dern\n" $YELLOW $GREEN
printf "%b+)%b octaspire-dern-repl examples\\dern-sockets-echo-client.dern\n" $YELLOW $GREEN
printf "%b6)%b octaspire-dern-repl examples\\dern-dir-example.dern\n" $YELLOW $GREEN
printf "%b7)%b octaspire-dern-repl examples\\dern-ncurses-example.dern\n" $YELLOW $GREEN
printf "%b8)%b octaspire-dern-repl examples\\dern-sdl2-example.dern\n" $YELLOW $GREEN
printf "%b9)%b octaspire-dern-repl examples\\irc-client.dern\n" $YELLOW $GREEN
echo "======================================================================="
echoToDefs

