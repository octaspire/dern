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


------------------------------------------------------------------
!!!!!! PLEASE NOTE: Dern on Plan9 is currently EXPERIMENTAL. !!!!!
------------------------------------------------------------------
It can CRASH and contains currently only the REPL, not unit test
runner and no examples or binary plugins. It is tested only a bit.
Most tested examples worked fine, but some caused crash: for example
simple function call:

(env-global)

caused crash (when tested on virtualbox) with error message:

corrupt tail magic0
pool sbrkmem block ...
...
panic: pool panic
octaspire-dern-repl 1047: suicide: sys: trap: fault read addr=0x0 pc=0x00026ebd
------------------------------------------------------------------
EnDoFmEsSaGe
