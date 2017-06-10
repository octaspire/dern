gcc -g -std=c99 -c -fPIC dern_ncurses.c
gcc -g -std=c99 -shared -o libdern_ncurses.so dern_ncurses.o -lncurses

