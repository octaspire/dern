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
cl /nologo /W2 /DOCTASPIRE_DERN_AMALGAMATED_REPL_IMPLEMENTATION /DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS octaspire-dern-amalgamated.c /link /out:octaspire-dern-repl.exe
@ECHO.
@ECHO RUN WITH:
@ECHO octaspire-dern-repl.exe
@ECHO.
@ECHO BUILD WITH:
@ECHO cl /W2 /DOCTASPIRE_DERN_AMALGAMATED_REPL_IMPLEMENTATION /DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS octaspire-dern-amalgamated.c /link /out:octaspire-dern-repl.exe


@ECHO -----------------------------------------------------------------
@ECHO binary library example
@ECHO -----------------------------------------------------------------
cl /nologo /W2 /I. /DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION /LD examples\mylib.c /link /out:libmylib.dll
@ECHO.
@ECHO RUN WITH:
@ECHO octaspire-dern-repl.exe examples\use-mylib.dern
@ECHO.
@ECHO BUILD WITH:
@ECHO cl /W2 /I. /DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION /LD examples\mylib.c /link /out:libmylib.dll
