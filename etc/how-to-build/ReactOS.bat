@ECHO ---- stand alone unit test runner: octaspire-dern-unit-test-runner.exe
gcc -O2 -std=c99 -Wall -Wextra -DOCTASPIRE_DERN_AMALGAMATED_UNIT_TEST_IMPLEMENTATION -DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS -I . octaspire-dern-amalgamated.c -lm -lKernel32 -o octaspire-dern-unit-test-runner
@ECHO --------------------------------------------------------------------------

@ECHO ---- embedding example: embedding-example.exe
gcc -O2 -std=c99 -Wall -Wextra -DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS -I . examples\embedding-example.c -lm -lKernel32 -o embedding-example
@ECHO --------------------------------------------------------------------------

@ECHO ---- implementation library for binary library example
gcc -O2 -std=c99 -Wall -Wextra -DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION -DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS octaspire-dern-amalgamated.c -lKernel32 -shared -Wl,--out-implib=imp.a
@ECHO --------------------------------------------------------------------------
@ECHO ---- binary library example: octaspire-dern-repl.exe examples\use-mylib.dern
gcc -O2 -std=c99 -Wall -Wextra -fPIC -shared -I . -o libmylib.dll examples\mylib.c -L . imp.a
@ECHO --------------------------------------------------------------------------

@ECHO ---- interactive Dern REPL: octaspire-dern-repl.exe
gcc -O2 -std=c99 -Wall -Wextra -DOCTASPIRE_DERN_AMALGAMATED_REPL_IMPLEMENTATION -DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS -I . octaspire-dern-amalgamated.c -lm -lKernel32 -o octaspire-dern-repl
@ECHO --------------------------------------------------------------------------

@ECHO ---- Dern socket plugin: octaspire-dern-repl examples\irc-client.dern
gcc -O2 -std=c99 -Wall -Wextra -fPIC -shared -I . -o libdern_socket.dll plugins\dern_socket.c -lws2_32 -L . imp.a
@ECHO --------------------------------------------------------------------------

@ECHO ---- Dern dir plugin: octaspire-dern-repl examples\dern-dir-example.dern
gcc -O2 -std=c99 -Wall -Wextra -fPIC -shared -I . -shared -o libdern_dir.dll plugins\dern_dir.c -lKernel32 -L . imp.a
@ECHO --------------------------------------------------------------------------

@ECHO ---- Dern easing plugin: octaspire-dern-repl examples\dern-easing-example.dern
gcc -O2 -std=c99 -Wall -Wextra -fPIC -shared -I . -shared -o libdern_easing.dll plugins\dern_easing.c -L . imp.a
@ECHO --------------------------------------------------------------------------

@ECHO ---- Dern animation plugin: octaspire-dern-repl examples\dern-animation-example.dern
gcc -O2 -std=c99 -Wall -Wextra -fPIC -shared -I . -shared -o libdern_animation.dll plugins\dern_animation.c -L . imp.a
@ECHO --------------------------------------------------------------------------

@ECHO ---- Dern ncurses plugin:  octaspire-dern-repl examples\dern-ncurses-example.dern
gcc -O2 -std=c99 -Wall -Wextra -fPIC -I . -shared -o libdern_ncurses.dll plugins\dern_ncurses.c -L . -lpdcurses imp.a
@ECHO --------------------------------------------------------------------------

@ECHO ---- Dern SDL2 plugin: octaspire-dern-repl examples\dern-sdl2-example.dern
gcc -O2 -std=c99 -Wall -Wextra -fPIC -shared -Dmain=SDL_main -I . -I SDL2 -o libdern_sdl2.dll plugins\dern_sdl2.c -L . -lSDL2main -lSDL2 imp.a
@ECHO --------------------------------------------------------------------------

