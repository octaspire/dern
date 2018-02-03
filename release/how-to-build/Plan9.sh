#!/usr/local/plan9/bin/rc

cat << EnDoFmEsSaGe
1. Building the interactive Dern REPL
-------------------------------------------------------------------------------
EnDoFmEsSaGe
8c -DOCTASPIRE_DERN_AMALGAMATED_REPL_IMPLEMENTATION -DOCTASPIRE_PLAN9_IMPLEMENTATION octaspire-dern-amalgamated.c

8l -o octaspire-dern-repl octaspire-dern-amalgamated.8



cat << EnDoFmEsSaGe

==================================================================
Run programs and examples like this:
==================================================================
1) octaspire-dern-repl
==================================================================

PLEASE NOTE: Dern on Plan9 is currently EXPERIMENTAL.
EnDoFmEsSaGe
