#!/usr/bin/tclsh
# Source file should not be too long

set maxLines [getParameter "max-file-length" 10000]

foreach f [getSourceFileNames] {
    set length [getLineCount $f]
    if {$length > $maxLines} {
        report $f $length "source file is too long"
    }
}
