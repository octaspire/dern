[![Build Status](https://travis-ci.org/octaspire/dern.svg?branch=master)](https://travis-ci.org/octaspire/dern) [![Coverage Status](https://codecov.io/gh/octaspire/dern/coverage.svg?branch=master)](https://codecov.io/gh/octaspire/dern/coverage.svg?branch=master)

Programming language written in standard C99

To build Dern from source in Raspberry Pi or Debian system:

```shell
sudo apt-get install cmake git
git clone https://github.com/octaspire/dern.git
cd dern/build
cmake ..
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

For a game (work in progress) being written with Dern and C99 see:
[https://github.com/octaspire/maze](https://github.com/octaspire/maze)

