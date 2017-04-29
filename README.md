[![Build Status](https://travis-ci.org/octaspire/dern.svg?branch=master)](https://travis-ci.org/octaspire/dern) [![Coverage Status](https://codecov.io/gh/octaspire/dern/coverage.svg?branch=master)](https://codecov.io/gh/octaspire/dern/coverage.svg?branch=master) [![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)


Programming language written in standard C99

To build Dern from source in Raspberry Pi, Debian or Ubuntu (16.04 LTS) system:

```shell
sudo apt-get install cmake git
git clone https://github.com/octaspire/dern.git
cd dern/build
cmake ..
make
```

To build on Arch Linux (Arch Linux ARM) system:

```shell
sudo pacman -S cmake git gcc make
git clone https://github.com/octaspire/dern.git
cd dern/build
cmake ..
make
```

To build on Haiku (Version Walter (Revision hrev51127) x86_gcc2):

```shell
pkgman install gcc_x86 cmake_x86
git clone https://github.com/octaspire/dern.git
cd dern/build
CC=gcc-x86 cmake ..
make
```

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

For games (work in progress) being written with Dern and C99 see:
[http://www.octaspire.com/maze](http://www.octaspire.com/maze)
[http://www.octaspire.com/lightboard](http://www.octaspire.com/lightboard)

