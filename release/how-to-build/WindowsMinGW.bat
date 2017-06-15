echo off
set MINGW_PATH=C:\MinGW
echo =                                                             
echo ===============================================================
echo = This script expects that you have MinGW installed in %MINGW_PATH%
echo = and that %MINGW_PATH% and %MINGW_PATH%\bin are in the PATH.
echo = If you don't have MinGW installed, you can download it from:
echo =                                                             
echo =                     http://www.mingw.org/                   
echo =                                                             
echo = There are also installation instructions available for it.
echo ===============================================================
echo =
echo =
echo = 1. Building stand alone unit test runner to test the release 
echo ===============================================================

echo on
%MINGW_PATH%\bin\gcc.exe -O2 -std=c99 -Wall -Wextra -DOCTASPIRE_DERN_AMALGAMATED_UNIT_TEST_IMPLEMENTATION octaspire-dern-amalgamated.c -lm -o octaspire-dern-unit-test-runner

echo off
echo =
echo =
echo = 2. Building the embedding example                          
echo ===============================================================

echo on
%MINGW_PATH%\bin\gcc.exe -O2 -std=c99 -Wall -Wextra -I . examples/embedding-example.c -lm -o embedding-example

echo off
echo =
echo =
echo = 3. Building the interactive REPL                             
echo ===============================================================

echo on
%MINGW_PATH%\bin\gcc.exe -O2 -std=c99 -Wall -Wextra -DOCTASPIRE_DERN_AMALGAMATED_REPL_IMPLEMENTATION octaspire-dern-amalgamated.c -lm -o octaspire-dern-repl

echo off
echo =
echo =
echo ======================================
echo = Run programs and examples like this:
echo ======================================
echo = 1) octaspire-dern-unit-test-runner  
echo = 2) embedding-example                
echo = 3) octaspire-dern-repl              
echo ======================================
