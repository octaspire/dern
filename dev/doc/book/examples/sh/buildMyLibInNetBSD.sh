make -C ../doc/examples/plugin
LD_LIBRARY_PATH=../doc/examples/plugin \
    ./octaspire-dern-repl -c
