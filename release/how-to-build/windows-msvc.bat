@ECHO OFF

@ECHO -----------------------------------------------------------------
@ECHO stand alone unit test runner:
@ECHO -----------------------------------------------------------------
cl /nologo /W3 /DOCTASPIRE_DERN_AMALGAMATED_UNIT_TEST_IMPLEMENTATION octaspire-dern-amalgamated.c /link /out:octaspire-dern-unit-test-runner.exe
@ECHO.
@ECHO RUN WITH:
@ECHO octaspire-dern-unit-test-runner.exe
@ECHO.
@ECHO BUILD WITH:
@ECHO cl /W3 /DOCTASPIRE_DERN_AMALGAMATED_UNIT_TEST_IMPLEMENTATION octaspire-dern-amalgamated.c /link /out:octaspire-dern-unit-test-runner.exe


@ECHO -----------------------------------------------------------------
@ECHO interactive Dern REPL:
@ECHO -----------------------------------------------------------------
cl /nologo /W2 /DOCTASPIRE_DERN_AMALGAMATED_REPL_IMPLEMENTATION octaspire-dern-amalgamated.c /link /out:octaspire-dern-repl.exe
@ECHO.
@ECHO RUN WITH:
@ECHO octaspire-dern-repl.exe
@ECHO.
@ECHO BUILD WITH:
@ECHO cl /W2 /DOCTASPIRE_DERN_AMALGAMATED_REPL_IMPLEMENTATION octaspire-dern-amalgamated.c /link /out:octaspire-dern-repl.exe
