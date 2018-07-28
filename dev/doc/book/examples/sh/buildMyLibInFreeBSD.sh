make -C ../doc/examples/plugin -f Makefile.FreeBSD
LD_LIBRARY_PATH=../doc/examples/plugin \
    ./octaspire-dern-repl -c
