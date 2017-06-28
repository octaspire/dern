echo "--- plugin ncurses ---"
gcc -g -Wall -Wextra -std=c99 -c -fPIC -I ../../release dern_ncurses.c
gcc -g -Wall -Wextra -std=c99 -shared -o libdern_ncurses.so dern_ncurses.o -lncursesw

echo "--- plugin socket  ---"
gcc -g -Wall -Wextra -std=gnu99 -c -fPIC -I ../../release dern_socket.c
gcc -g -Wall -Wextra -std=gnu99 -shared -o libdern_socket.so dern_socket.o
