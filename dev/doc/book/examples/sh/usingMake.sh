git clone https://github.com/octaspire/dern.git
cd dern
make submodules-init
make
make test
make codestyle
make cppcheck
make valgrind
make coverage
make coverage-show
make perf-linux

./octaspire-dern-repl

make amalgamation
release/octaspire-dern-repl

make clean

rm release/octaspire-dern-amalgamated.c
make release/octaspire-dern-amalgamated.c

rm release/documentation/dern-manual.html
make release/documentation/dern-manual.html
