pkgman install gcc_x86
git clone https://github.com/octaspire/dern.git
cd dern
make submodules-init
CC=gcc-x86 make
