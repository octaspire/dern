# Octaspire Dern

[http://www.octaspire.com/dern](http://www.octaspire.com/dern)

[![Build Status](https://travis-ci.org/octaspire/dern.svg?branch=master)](https://travis-ci.org/octaspire/dern) [![Coverage Status](https://codecov.io/gh/octaspire/dern/coverage.svg?branch=master)](https://codecov.io/gh/octaspire/dern) [![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)


Programming language written in standard C99

## To build Dern from source in Raspberry Pi, Debian or Ubuntu (16.04 LTS) system:

### From the amalgamated source distribution

```shell
wget https://github.com/octaspire/dern/releases/download/v0.73.0/octaspire_dern_amalgamated.c

gcc -O3 -std=c99 -Wall -Wextra -DOCTASPIRE_DERN_AMALGAMATED_UNIT_TEST_IMPLEMENTATION \
                               -DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS                \
                               -DGREATEST_ENABLE_ANSI_COLORS                         \
octaspire_dern_amalgamated.c -Wl,-export-dynamic -ldl -lm -o octaspire-dern-test-runner

gcc -O3 -std=c99 -Wall -Wextra -DOCTASPIRE_DERN_AMALGAMATED_REPL_IMPLEMENTATION \
                               -DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS           \
octaspire_dern_amalgamated.c -Wl,-export-dynamic -ldl -lm -o octaspire-dern-repl
```

### From the regular source distribution:

```shell
sudo apt-get install cmake git
git clone https://github.com/octaspire/dern.git
cd dern/build
cmake ..
make
```

## To build on Arch Linux (Arch Linux ARM) system:

```shell
sudo pacman -S cmake git gcc make
git clone https://github.com/octaspire/dern.git
cd dern/build
cmake ..
make
```

## To build on Haiku (Version Walter (Revision hrev51127) x86_gcc2):

```shell
pkgman install gcc_x86 cmake_x86
git clone https://github.com/octaspire/dern.git
cd dern/build
CC=gcc-x86 cmake ..
make
```

## To build on FreeBSD (FreeBSD-11.0-RELEASE-arm-armv6-RPI2) system:

```shell
sudo pkg install git cmake
git clone https://github.com/octaspire/dern.git
cd dern/build
cmake ..
make
```

## To build on MINIX 3 (minix_R3.3.0-588a35b) system:

```shell
su root
pkgin install cmake clang binutils git-base
exit
git clone git://github.com/octaspire/dern
cd dern
perl -pi -e 's/https/git/' .gitmodules
cd build
cmake ..
make
```

## To use Dern

To run the unit tests:

```shell
test/octaspire-dern-test-runner
```

To start the REPL with color diagnostics:

```shell
./octaspire-dern-repl -c
```

Language manual can be found from doc/book or from here:
[http://www.octaspire.com/dern](http://www.octaspire.com/dern)

Video of Dern in action:
[![asciicast](https://asciinema.org/a/112216.png)](https://asciinema.org/a/112216)

Downloading and building amalgamated source distribution of Octaspire Dern in Raspberry Pi 2.
Just one file. No build tools required. No dependencies, only C compiler is needed.
[![asciicast](https://asciinema.org/a/121995.png)](https://asciinema.org/a/121995)

For games (work in progress) being written with Dern and C99 see:
[http://www.octaspire.com/maze](http://www.octaspire.com/maze) and 
[http://www.octaspire.com/lightboard](http://www.octaspire.com/lightboard)

