# Octaspire Dern

[http://www.octaspire.com/dern](http://www.octaspire.com/dern)

[![Build Status](https://travis-ci.org/octaspire/dern.svg?branch=master)](https://travis-ci.org/octaspire/dern) [![Coverage Status](https://codecov.io/gh/octaspire/dern/coverage.svg?branch=master)](https://codecov.io/gh/octaspire/dern) [![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)


Programming language written in standard C99

## To build Dern from the amalgamated one file source release:

The amalgamated source release is the recommended way of using Dern, if you don't need to
modify Dern itself. To use the amalgamated release, you will need only a C compiler and
C standard library supporting C99.

```shell
curl -O www.octaspire.com/dern/release.tar.bz2
tar jxf release.tar.bz2
cd release/*
curl -O https://octaspire.github.io/dern/checksums
sha512sum -c checksums
sh how-to-build/YOUR_PLATFORM_NAME_HERE.XX
```

replace **YOUR_PLATFORM_NAME_HERE.XX** with **FreeBSD.sh**, **NetBSD.sh**, **linux.sh**,
**minix3.sh** or **haiku.sh**. More scripts for different platforms will be added later.

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

