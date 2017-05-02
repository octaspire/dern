#!/bin/bash

function test-case {
    echo "$(tput setaf 5)[$1] $3 "$2" $(tput sgr 0)"
    RFILE=../../build/test/REPL/R$1
    script -q -c "$2" $RFILE > /dev/null

    if [ $? -ne 0 ]; then
        echo "$(tput setaf 1)----- [$1] FAILED TO EXECUTE OK. >>>$?<<<-----$(tput sgr 0)"
        exit 1
    fi

    DIFF=$(diff --strip-trailing-cr $RFILE E$1)

    if [ "$DIFF" != "" ]; then
        echo "$(tput setaf 1)----- [$1] FAILED TO PRODUCE CORRECT OUTPUT-----$(tput sgr 0)"
        colordiff --strip-trailing-cr $RFILE E$1
        exit 1
    fi

    echo "$(tput setaf 2)[$1] OK$(tput sgr 0)"
    echo ""
}

mkdir ../../build/test/REPL

test-case 1 '../../build/octaspire-dern-repl -h'    'Test help'
test-case 2 '../../build/octaspire-dern-repl -c -h' 'Test help with colors'

test-case 3 '../../build/octaspire-dern-repl -v'    'Test version'
test-case 4 '../../build/octaspire-dern-repl -c -v' 'Test version with colors'

test-case 5 '../../build/octaspire-dern-repl -e "(println [{}] (+ 3 3))"' 'Test -e string'

test-case 6 '../../build/octaspire-dern-repl test-a.dern'             'Test evaluating one file'
test-case 7 '../../build/octaspire-dern-repl test-a.dern test-b.dern' 'Test evaluating two files'
test-case 8 '../../build/octaspire-dern-repl -e "(println [{}] (+ 3 3))" -e "(println [{}] (* 5 5))" test-a.dern test-b.dern' 'Test -e string -e string file-a file-b'

