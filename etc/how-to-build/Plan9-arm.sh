#!/usr/local/plan9/bin/rc

cat << EnDoFmEsSaGe
1. Building the interactive Dern REPL
-------------------------------------------------------------------------------
EnDoFmEsSaGe
5c -DOCTASPIRE_DERN_AMALGAMATED_REPL_IMPLEMENTATION -DOCTASPIRE_PLAN9_IMPLEMENTATION octaspire-dern-amalgamated.c

5l -L /arm/lib -o octaspire-dern-repl octaspire-dern-amalgamated.5



cat << EnDoFmEsSaGe

==================================================================
Run programs and examples like this:
==================================================================
1) octaspire-dern-repl
==================================================================

PLEASE NOTE: Dern on Plan9 is currently EXPERIMENTAL.
EnDoFmEsSaGe
