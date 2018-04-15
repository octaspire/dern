AMALGA=sh dev/etc/amalgamate_file.sh
AMALGL=sh dev/etc/amalgamate_file_literally.sh
ETCDIR=dev/etc/
INCDIR=dev/include/octaspire/dern/
SRCDIR=dev/src/
TESTDR=dev/test/
EXTDIR=dev/external/
DEVDOCDIR=dev/doc/
CORDIR=$(EXTDIR)octaspire_core/release/
RELDIR=release/
PLUGINDIR=$(RELDIR)plugins/
RELDOCDIR=$(RELDIR)documentation/
AMALGAMATION=$(RELDIR)octaspire-dern-amalgamated.c
PLUGINS := $(wildcard $(PLUGINDIR)*.c)
UNAME=$(shell uname -s)

# In batch mode Emacs doesn't load the usual initialization file. To get the correct
# settings and styles in the batch mode, the initialization file must be loaded manually.
# However, there are still some small problems with the Org export when running in batch
# mode using the default version of Org mode, so the export is run without batch mode at
# the moment.
#EMACSFLAGS=--load dev/external/octaspire_dotfiles/emacs/.emacs.d/init.el --batch
EMACSFLAGS=

.PHONY: submodules clean codestyle cppcheck valgrind test coverage

$(RELDIR)octaspire-dern-repl: $(AMALGAMATION) $(PLUGINS)
	@echo "Building for $(UNAME)..."
	@if [ "$(UNAME)" = "Linux" ]; then\
            cd release && sh how-to-build/linux.sh > /dev/null 2>&1 && echo "Done.";\
        elif [ "$(UNAME)" = "Darwin" ]; then\
            cd release && sh how-to-build/macOS.sh > /dev/null 2>&1 && echo "Done.";\
        elif [ "$(UNAME)" = "OpenBSD" ]; then\
            cd release && sh how-to-build/OpenBSD.sh > /dev/null 2>&1 && echo "Done.";\
        elif [ "$(UNAME)" = "FreeBSD" ]; then\
            cd release && sh how-to-build/FreeBSD.sh > /dev/null 2>&1 && echo "Done.";\
        elif [ "$(UNAME)" = "NetBSD" ]; then\
            cd release && sh how-to-build/NetBSD.sh > /dev/null 2>&1 && echo "Done.";\
        elif [ "$(UNAME)" = "Minix" ]; then\
            cd release && sh how-to-build/minix3.sh > /dev/null 2>&1 && echo "Done.";\
        elif [ "$(UNAME)" = "DragonFly" ]; then\
            cd release && sh how-to-build/DragonFlyBSD.sh > /dev/null 2>&1 && echo "Done.";\
        elif [ "$(UNAME)" = "Haiku" ]; then\
            cd release && sh how-to-build/haiku.sh > /dev/null 2>&1 && echo "Done.";\
        else\
            echo "This platform is not handled by Makefile at the moment. Please build using a script from 'release/how-to-build'.";\
        fi;

submodules:
	@echo "Initializing submodules..."
	@git submodule init
	@git submodule update
	@echo "Done."

$(AMALGAMATION): $(ETCDIR)amalgamation_head.c                \
                 $(CORDIR)octaspire-core-amalgamated.c       \
                 $(INCDIR)octaspire_dern_config.h            \
                 $(INCDIR)octaspire_dern_lexer.h             \
                 $(INCDIR)octaspire_dern_c_data.h            \
                 $(INCDIR)octaspire_dern_port.h              \
                 $(INCDIR)octaspire_dern_value.h             \
                 $(INCDIR)octaspire_dern_helpers.h           \
                 $(INCDIR)octaspire_dern_environment.h       \
                 $(INCDIR)octaspire_dern_lib.h               \
                 $(INCDIR)octaspire_dern_vm.h                \
                 $(INCDIR)octaspire_dern_stdlib.h            \
                 $(ETCDIR)amalgamation_impl_head.c           \
                 $(SRCDIR)octaspire_dern_environment.c       \
                 $(SRCDIR)octaspire_dern_lexer.c             \
                 $(SRCDIR)octaspire_dern_lib.c               \
                 $(SRCDIR)octaspire_dern_c_data.c            \
                 $(SRCDIR)octaspire_dern_port.c              \
                 $(SRCDIR)octaspire_dern_helpers.c           \
                 $(SRCDIR)octaspire_dern_stdlib.c            \
                 $(SRCDIR)octaspire_dern_value.c             \
                 $(SRCDIR)octaspire_dern_vm.c                \
                 $(ETCDIR)amalgamation_impl_tail.c           \
                 $(EXTDIR)octaspire_dern_banner_color.h      \
                 $(EXTDIR)octaspire_dern_banner_white.h      \
                 $(SRCDIR)octaspire_dern_repl.c              \
                 $(ETCDIR)amalgamation_impl_unit_test_head.c \
                 $(EXTDIR)greatest.h                         \
                 $(TESTDR)test_dern_lexer.c                  \
                 $(TESTDR)test_dern_vm.c                     \
                 $(ETCDIR)amalgamation_impl_unit_test_tail.c
	@echo "Creating amalgamation..."
	@rm -rf $(AMALGAMATION)
	@$(AMALGL) $(ETCDIR)amalgamation_head.c                $(AMALGAMATION)
	@$(AMALGA) $(CORDIR)octaspire-core-amalgamated.c       $(AMALGAMATION)
	@$(AMALGA) $(INCDIR)octaspire_dern_config.h            $(AMALGAMATION)
	@$(AMALGA) $(INCDIR)octaspire_dern_lexer.h             $(AMALGAMATION)
	@$(AMALGA) $(INCDIR)octaspire_dern_c_data.h            $(AMALGAMATION)
	@$(AMALGA) $(INCDIR)octaspire_dern_port.h              $(AMALGAMATION)
	@$(AMALGA) $(INCDIR)octaspire_dern_value.h             $(AMALGAMATION)
	@$(AMALGA) $(INCDIR)octaspire_dern_helpers.h           $(AMALGAMATION)
	@$(AMALGA) $(INCDIR)octaspire_dern_environment.h       $(AMALGAMATION)
	@$(AMALGA) $(INCDIR)octaspire_dern_lib.h               $(AMALGAMATION)
	@$(AMALGA) $(INCDIR)octaspire_dern_vm.h                $(AMALGAMATION)
	@$(AMALGA) $(INCDIR)octaspire_dern_stdlib.h            $(AMALGAMATION)
	@$(AMALGL) $(ETCDIR)amalgamation_impl_head.c           $(AMALGAMATION)
	@$(AMALGA) $(SRCDIR)octaspire_dern_environment.c       $(AMALGAMATION)
	@$(AMALGA) $(SRCDIR)octaspire_dern_lexer.c             $(AMALGAMATION)
	@$(AMALGA) $(SRCDIR)octaspire_dern_lib.c               $(AMALGAMATION)
	@$(AMALGA) $(SRCDIR)octaspire_dern_c_data.c            $(AMALGAMATION)
	@$(AMALGA) $(SRCDIR)octaspire_dern_port.c              $(AMALGAMATION)
	@$(AMALGA) $(SRCDIR)octaspire_dern_helpers.c           $(AMALGAMATION)
	@$(AMALGA) $(SRCDIR)octaspire_dern_stdlib.c            $(AMALGAMATION)
	@$(AMALGA) $(SRCDIR)octaspire_dern_value.c             $(AMALGAMATION)
	@$(AMALGA) $(SRCDIR)octaspire_dern_vm.c                $(AMALGAMATION)
	@$(AMALGL) $(ETCDIR)amalgamation_impl_tail.c           $(AMALGAMATION)
	@$(AMALGA) $(EXTDIR)octaspire_dern_banner_color.h      $(AMALGAMATION)
	@$(AMALGA) $(EXTDIR)octaspire_dern_banner_white.h      $(AMALGAMATION)
	@$(AMALGA) $(SRCDIR)octaspire_dern_repl.c              $(AMALGAMATION)
	@$(AMALGA) $(ETCDIR)amalgamation_impl_unit_test_head.c $(AMALGAMATION)
	@$(AMALGL) $(EXTDIR)greatest.h                         $(AMALGAMATION)
	@$(AMALGA) $(TESTDR)test_dern_lexer.c                  $(AMALGAMATION)
	@$(AMALGA) $(TESTDR)test_dern_vm.c                     $(AMALGAMATION)
	@$(AMALGA) $(ETCDIR)amalgamation_impl_unit_test_tail.c $(AMALGAMATION)

$(RELDOCDIR)dern-manual.html: $(DEVDOCDIR)book/dern-manual.org
	@LANG=eng_US.utf8 emacs $(EMACSFLAGS) $< --funcall org-reload --funcall org-html-export-to-html --kill > /dev/null 2>&1
	@mv dev/doc/book/dern-manual.html release/documentation/

clean:
	@rm -rf $(AMALGAMATION) $(RELDIR)embedding-example $(RELDIR)*.so              \
                $(RELDIR)octaspire-dern-repl $(RELDIR)octaspire-dern-unit-test-runner \
                $(RELDIR)octaspire_dern_vm_run_user_factorial_function_test.dern      \
                $(RELDIR)octaspire_io_file_open_test.txt                              \
                $(RELDIR)octaspire_read_and_eval_path_test.dern                       \
                octaspire_dern_vm_run_user_factorial_function_test.dern               \
                octaspire_io_file_open_test.txt                                       \
                octaspire_read_and_eval_path_test.dern                                \
                $(RELDIR)coverage.info                                                \
                $(RELDIR)octaspire-dern-amalgamated.gcda                              \
                $(RELDIR)octaspire-dern-amalgamated.gcno                              \
                coverage

codestyle:
	@vera++ --root dev/external/vera --profile octaspire-plugin --error $(wildcard $(SRCDIR)*.[ch])
	@vera++ --root dev/external/vera --profile octaspire-plugin --error $(wildcard $(PLUGINDIR)*.[ch])

cppcheck:
	@cppcheck --std=c99 -I dev/include dev/external/ocraspire_core/include --enable=warning,performance,portability --verbose --quiet $(wildcard $(SRCDIR)*.[ch]) $(wildcard $(PLUGINDIR)*.[ch])

valgrind: $(RELDIR)octaspire-dern-repl
	@valgrind --leak-check=full --track-origins=yes --error-exitcode=1 $(RELDIR)octaspire-dern-unit-test-runner --write-test-files
	@valgrind --leak-check=full --track-origins=yes --error-exitcode=1 $(RELDIR)octaspire-dern-repl -e "(+ 1 2 3)(exit)"

test: $(RELDIR)octaspire-dern-repl
	@$(RELDIR)octaspire-dern-unit-test-runner --write-test-files

coverage: $(AMALGAMATION)
	@echo "Building for $(UNAME) with coverage enabled..."
	@if [ "$(UNAME)" = "Linux" ]; then\
            cd release && sh how-to-build/linux.sh gcc --coverage > /dev/null 2>&1 && echo "Done.";\
        elif [ "$(UNAME)" = "Darwin" ]; then\
            cd release && sh how-to-build/macOS.sh gcc --coverage > /dev/null 2>&1 && echo "Done.";\
        elif [ "$(UNAME)" = "OpenBSD" ]; then\
            cd release && sh how-to-build/OpenBSD.sh gcc --coverage > /dev/null 2>&1 && echo "Done.";\
        elif [ "$(UNAME)" = "FreeBSD" ]; then\
            cd release && sh how-to-build/FreeBSD.sh gcc --coverage > /dev/null 2>&1 && echo "Done.";\
        elif [ "$(UNAME)" = "NetBSD" ]; then\
            cd release && sh how-to-build/NetBSD.sh gcc --coverage > /dev/null 2>&1 && echo "Done.";\
        elif [ "$(UNAME)" = "Minix" ]; then\
            cd release && sh how-to-build/minix3.sh gcc --coverage > /dev/null 2>&1 && echo "Done.";\
        elif [ "$(UNAME)" = "DragonFly" ]; then\
            cd release && sh how-to-build/DragonFlyBSD.sh gcc --coverage > /dev/null 2>&1 && echo "Done.";\
        elif [ "$(UNAME)" = "Haiku" ]; then\
            cd release && sh how-to-build/haiku.sh gcc --coverage > /dev/null 2>&1 && echo "Done.";\
        else\
            echo "This platform is not handled by Makefile at the moment. Please build using a script from 'release/how-to-build'.";\
        fi;
	@$(RELDIR)/octaspire-dern-unit-test-runner --write-test-files
	@lcov --no-external --capture --directory release --output-file $(RELDIR)coverage.info
	@genhtml $(RELDIR)coverage.info --output-directory coverage
	@xdg-open coverage/index.html &
