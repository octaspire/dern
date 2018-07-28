make -C ../doc/examples/plugin -f Makefile.MINIX3
LD_LIBRARY_PATH=../doc/examples/plugin \
    ./octaspire-dern-repl -c
