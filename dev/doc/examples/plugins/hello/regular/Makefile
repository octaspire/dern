CC         = gcc
CFLAGS     = -fPIC -Wall -Wextra -g -I ../../../include -I ../../../external/octaspire_core/include
LDFLAGS    = -shared

.PHONY: all
all: libmylib.so

libmylib.so: mylib.o
	$(CC) $(LDFLAGS) mylib.o -o libmylib.so

mylib.o: mylib.c
	$(CC) -c $(CFLAGS) mylib.c

.PHONY: clean
clean:
	rm -f libmylib.so mylib.o
