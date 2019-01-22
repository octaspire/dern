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
@ECHO embedding example
@ECHO -----------------------------------------------------------------
cl /nologo /W2 /I. /DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS examples\embedding-example.c /link /out:embedding-example.exe
@ECHO.
@ECHO RUN WITH:
@ECHO embedding-example.exe
@ECHO.
@ECHO BUILD WITH:
@ECHO cl /W2 /I. /DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS examples\embedding-example.c /link /out:embedding-example.exe


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


@ECHO -----------------------------------------------------------------
@ECHO Dern socket plugin
@ECHO -----------------------------------------------------------------
cl /nologo /W2 /I. /DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION /LD plugins\dern_socket.c /link ws2_32.lib /out:libdern_socket.dll
@ECHO.
@ECHO RUN WITH:
@ECHO octaspire-dern-repl.exe examples\irc-client-ncurses.dern
@ECHO octaspire-dern-repl.exe examples\irc-client-nuklear.dern
@ECHO.
@ECHO BUILD WITH:
@ECHO cl /W2 /I. /DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION /LD plugins\dern_socket.c /link ws2_32.lib /out:libdern_socket.dll


@ECHO -----------------------------------------------------------------
@ECHO Dern ncurses plugin
@ECHO -----------------------------------------------------------------
cl /nologo /W2 /I. /DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION /LD plugins\dern_ncurses.c /link pdcurses.lib /out:libdern_ncurses.dll
@ECHO.
@ECHO RUN WITH:
@ECHO octaspire-dern-repl.exe examples\dern-ncurses-example.dern
@ECHO.
@ECHO BUILD WITH:
@ECHO cl /nologo /W2 /I. /DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION /LD plugins\dern_ncurses.c /link pdcurses.lib /out:libdern_ncurses.dll
