Programming language written in standard C99

To build on Raspberry Pi:

```shell
sudo apt-get install cmake git
git clone https://gitlab.com/octaspire/dern.git
cd dern
mkdir build
cd build
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
[![asciicast](https://asciinema.org/a/109958.png)](https://asciinema.org/a/109958)

