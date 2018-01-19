#!/usr/bin/tclsh
# Source files should refer the Apache License, Version 2.0

foreach file [getSourceFileNames] {
    set found false
    foreach comment [getTokens $file 1 0 -1 -1 {ccomment cppcomment}] {
        set value [lindex $comment 0]
        if {[string first "http://www.apache.org/licenses/LICENSE-2.0" $value] != -1} {
            set found true
            break
        }
    }
    if {$found == false} {
        report $file 1 "no reference to the Apache License, Version 2.0 found"
    }
}
