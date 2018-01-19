#!/usr/bin/tclsh
# No leading empty lines and a trailing empty line

foreach f [getSourceFileNames] {
    set lineCount [getLineCount $f]
    if {$lineCount > 0} {
        set firstLine [getLine $f 1]
        if {[string trim $firstLine] == ""} {
            report $f 1 "leading empty line(s)"
        }

        set lastLine [getLine $f $lineCount]
        if {[string trim $lastLine] != ""} {
            report $f $lineCount "no trailing empty line"
        }
    }
}
