make -C ../doc/examples/plugin -f Makefile.Haiku
LIBRARY_PATH=$LIBRARY_PATH:../doc/examples/plugin \
    ./octaspire-dern-repl -c
