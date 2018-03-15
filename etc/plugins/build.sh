#!/usr/bin/env bash
cat << EOF
This script is intended to be used while developing plugins. For normal use,
it is recommended to use the files inside release-directory. The file for
your platform in directory 'how-to-build' builds also all the binary plugins.
This script supports only couple of platforms. Use the release to get support
for many more.
EOF

case "$(uname -s)" in
    Linux*)  platform=Linux;;
    Darwin*) platform=MacOS;;
    MINGW*)  platform=MinGW;;
    *)       platform=Unknown;;
esac

echo "--- Building plugins in $platform ---"

if [ "$platform" == "Linux" ]; then
    echo "--- plugin ncurses ---"
    gcc -g -Wall -Wextra -std=c99 -c -fPIC -I ../../release dern_ncurses.c
    gcc -g -Wall -Wextra -std=c99 -shared -o libdern_ncurses.so dern_ncurses.o -lncursesw

    echo "--- plugin socket  ---"
    gcc -g -Wall -Wextra -std=gnu99 -c -fPIC -I ../../release dern_socket.c
    gcc -g -Wall -Wextra -std=gnu99 -shared -o libdern_socket.so dern_socket.o

    echo "--- plugin openssl ---"
    gcc -g -Wall -Wextra -std=c99 -c -fPIC -I ../../release dern_openssl.c
    gcc -g -Wall -Wextra -std=c99 -shared -o libdern_openssl.so dern_openssl.o -lssl -lcrypto

    echo "--- plugin dir     ---"
    gcc -g -Wall -Wextra -std=c99 -DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION -c -fPIC -I ../../release dern_dir.c
    gcc -g -Wall -Wextra -std=c99 -shared -o libdern_dir.so dern_dir.o

    echo "--- plugin SDL2    ---"
    gcc -g -Wall -Wextra -pedantic -g -std=c99 -DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION \
        -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_IMAGE_LIBRARY \
        -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY \
        -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY   \
        -c -fPIC -I ../../release dern_sdl2.c `sdl2-config --cflags`
    gcc -g -Wall -Wextra -pedantic -std=c99 -shared -o libdern_sdl2.so dern_sdl2.o `sdl2-config --libs` -lSDL2_image -lSDL2_mixer -lSDL2_ttf

    echo "--- plugin easing    ---"
    gcc -g -Wall -Wextra -std=gnu99 -c -fPIC -I ../../release dern_easing.c
    gcc -g -Wall -Wextra -std=gnu99 -shared -o libdern_easing.so dern_easing.o

    echo "--- plugin animation    ---"
    gcc -g -Wall -Wextra -std=gnu99 -c -fPIC -I ../../release dern_animation.c
    gcc -g -Wall -Wextra -std=gnu99 -shared -o libdern_animation.so dern_animation.o
fi


if [ "$platform" == "MacOS" ]; then
    echo "--- plugin ncurses ---"
    clang -g -Wall -Wextra -std=c99 -DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION -c -fPIC -I ../../release dern_ncurses.c
    clang -g -Wall -Wextra -std=c99 -dynamiclib -o libdern_ncurses.dylib dern_ncurses.o -lncurses

    echo "--- plugin socket  ---"
    clang -g -Wall -Wextra -std=gnu99 -DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION -c -fPIC -I ../../release dern_socket.c
    clang -g -Wall -Wextra -std=gnu99 -dynamiclib -o libdern_socket.dylib dern_socket.o

    echo "--- plugin openssl ---"
    clang -g -Wall -Wextra -std=c99 -DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION -c -fPIC -I ../../release -I /usr/local/opt/openssl/include dern_openssl.c
    clang -g -Wall -Wextra -std=c99 -dynamiclib -o libdern_openssl.dylib dern_openssl.o -L /usr/local/opt/openssl/lib -lssl -lcrypto

    echo "--- plugin dir     ---"
    clang -g -Wall -Wextra -std=c99 -DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION -c -fPIC -I ../../release dern_dir.c
    clang -g -Wall -Wextra -std=c99 -dynamiclib -o libdern_dir.dylib dern_dir.o

    echo "--- plugin SDL2    ---"
    clang -g -O0 -Wall -Wextra -pedantic -g -std=c99 -DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION \
        -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_IMAGE_LIBRARY \
        -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY \
        -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY   \
        -c -fPIC -I ../../release dern_sdl2.c `sdl2-config --cflags`
    clang -g -Wall -Wextra -pedantic -std=c99 -dynamiclib -o libdern_sdl2.dylib dern_sdl2.o `sdl2-config --libs` -lSDL2_image -lSDL2_mixer -lSDL2_ttf

    echo "--- plugin easing     ---"
    clang -g -Wall -Wextra -std=c99 -DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION -c -fPIC -I ../../release dern_easing.c
    clang -g -Wall -Wextra -std=c99 -dynamiclib -o libdern_easing.dylib dern_easing.o

    echo "--- plugin animation     ---"
    clang -g -Wall -Wextra -std=c99 -DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION -c -fPIC -I ../../release dern_animation.c
    clang -g -Wall -Wextra -std=c99 -dynamiclib -o libdern_animation.dylib dern_animation.o
fi

if [ "$platform" == "MinGW" ]; then
    echo "--- implib         ---"
    gcc  -DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION ../../release/octaspire-dern-amalgamated.c -shared -Wl,--out-implib=imp.a

    #echo "--- plugin ncurses ---"
    #gcc -g -Wall -Wextra -std=c99 -c -fPIC -I ../../release dern_ncurses.c
    #gcc -g -Wall -Wextra -std=c99 -shared -o libdern_ncurses.dll dern_ncurses.o -lncursesw -L . imp.a

    echo "--- plugin socket  ---"
    gcc -g -Wall -Wextra -std=c99  -I ../../release  -c -fPIC dern_socket.c
    gcc -g -Wall -Wextra -std=c99 -shared -I . -o libdern_socket.dll dern_socket.o -lws2_32 -L . imp.a

    echo "--- plugin dir     ---"
    gcc -g -Wall -Wextra -std=c99 -DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION -c -fPIC -I ../../release dern_dir.c
    gcc -g -Wall -Wextra -std=c99 -shared -o libdern_dir.dll dern_dir.o
fi


