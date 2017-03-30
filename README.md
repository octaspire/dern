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
[![asciicast](https://asciinema.org/a/2ejot73uc3o6m045jpqvjf8av.png)](https://asciinema.org/a/2ejot73uc3o6m045jpqvjf8av)

