gcc -g -Wall -Wextra -std=c99 -c -fPIC -I ../../release dern_ncurses.c
gcc -g -Wall -Wextra -std=c99 -shared -o libdern_ncurses.so dern_ncurses.o -lncurses

