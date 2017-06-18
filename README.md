# Octaspire Dern

[http://www.octaspire.com/dern](http://www.octaspire.com/dern)

[![Build Status](https://travis-ci.org/octaspire/dern.svg?branch=master)](https://travis-ci.org/octaspire/dern) [![Coverage Status](https://codecov.io/gh/octaspire/dern/coverage.svg?branch=master)](https://codecov.io/gh/octaspire/dern) [![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)


Programming language written in standard C99

## To build Dern from the amalgamated one file source release:

The amalgamated source release is the recommended way of using Dern, if you don't need to
modify Dern itself. To use the amalgamated release, you will need only a C compiler and
C standard library supporting C99.

### Linux, FreeBSD, OpenBSD, NetBSD, OpenIndiana, DragonFly BSD, MINIX 3, Haiku

```shell
curl -O octaspire.com/dern/release.tar.bz2
tar jxf release.tar.bz2
cd release/*
curl -O https://octaspire.github.io/dern/checksums
sha512sum -c checksums
sh how-to-build/YOUR_PLATFORM_NAME_HERE.XX
```

replace **YOUR_PLATFORM_NAME_HERE.XX** with **FreeBSD.sh**, **NetBSD.sh**, **OpenBSD.sh**,
**OpenIndiana.sh**, **DragonFlyBSD.sh**, **linux.sh**, **minix3.sh** or **haiku.sh**.
More scripts for different platforms will be added later.



### Windows using MinGW

1. Download and install **MinGW** from
[www.mingw.org](http://www.mingw.org) into directory `C:\MinGW`. Please note, that you might need
to add `C:\MinGW` and `C:\MinGW\bin` into the `PATH`.
If you cannot install into `C:\MinGW`, you can install MinGW to some other place.
Remember the installation path, because later you can write it into the
`WindowsMinGW.bat` file, so that the script finds MinGW.

2. Download and install **7-Zip** from
[www.7-zip.org](http://www.7-zip.org).

3. Download
[www.octaspire.com/dern/release.tar.bz2](http://www.octaspire.com/dern/release.tar.bz2) and
extract it with 7-Zip. You might need to extract it twice; first into file `release.tar`
and then again to get the directory.

4. Start **shell** and change directory to the extracted release directory
   and then into directory `version-x.y.z`, where x, y and z are some version
   numbers.

5. When you are in the directory `version-x.y.z` run command `how-to-build\WindowsMinGW.bat`


More scripts for different tools might be added later.



Language manual can be found from doc/book or from here:
[http://www.octaspire.com/dern](http://www.octaspire.com/dern)

Video of Dern in action:
[![asciicast](https://asciinema.org/a/123972.png)](https://asciinema.org/a/123972)

For games (work in progress) being written with Dern and C99 see:
[http://www.octaspire.com/maze](http://www.octaspire.com/maze) and 
[http://www.octaspire.com/lightboard](http://www.octaspire.com/lightboard)

