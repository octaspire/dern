AMALGA=sh dev/etc/amalgamate_file.sh
AMALGL=sh dev/etc/amalgamate_file_literally.sh
ETCDIR=dev/etc/
INCDIR=dev/include/octaspire/dern/
SRCDIR=dev/src/
TESTDR=dev/test/
EXTDIR=dev/external/
DEVDOCDIR=dev/doc/
DEVGAMEDIR=$(ETCDIR)games/
CORDIR=$(EXTDIR)octaspire_core/release/
RELDIR=release/
GAMESDIR=$(RELDIR)games/
PLUGINDIR=$(RELDIR)plugins/
RELDOCDIR=$(RELDIR)documentation/
AMALGAMATION=$(RELDIR)octaspire-dern-amalgamated.c
PLUGINS := $(wildcard $(PLUGINDIR)*.c)
UNAME=$(shell uname -s)
CFLAGS=-std=c99 -Wall -Wextra -g -O2 -DOCTASPIRE_DERN_CONFIG_BINARY_PLUGINS

TAGS_C_FILES := $(SRCDIR)*.c $(INCDIR)*.h $(CORDIR)octaspire-core-amalgamated.c

TAGS_DERN_FILES := $(GAMESDIR)octaspire-bounce.dern    \
                   $(GAMESDIR)octaspire-lightcube.dern \
                   $(GAMESDIR)octaspire-maze.dern

CHIPMUNK_SRCS := $(wildcard release/plugins/external/chipmunk/src/*.c)
CHIPMUNK_OBJS := $(patsubst %.c, %.o, $(CHIPMUNK_SRCS))
CHIPMUNK_PATH := $(PLUGINDIR)external/chipmunk/src/

DOCEXAMPLES := $(wildcard $(DEVDOCDIR)book/examples/dern/*.dern)
DOCEXAMPLES += $(wildcard $(DEVDOCDIR)book/examples/sh/*.sh)
DOCEXAMPLES += $(wildcard $(DEVDOCDIR)book/examples/c/*.c)

TESTOBJS := $(SRCDIR)octaspire_dern_c_data.o      \
            $(SRCDIR)octaspire_dern_environment.o \
            $(SRCDIR)octaspire_dern_helpers.o     \
            $(SRCDIR)octaspire_dern_lib.o         \
            $(SRCDIR)octaspire_dern_port.o        \
            $(SRCDIR)octaspire_dern_stdlib.o      \
            $(SRCDIR)octaspire_dern_value.o       \

DEVOBJS := $(TESTOBJS)                           \
           $(SRCDIR)octaspire_dern_lexer.o       \
           $(SRCDIR)octaspire_dern_vm.o

UNAME := $(shell uname)
MACHINE := $(shell uname -m)
OS := "Unknown"

MAKE=make

# TODO Detect more platforms and show message about using the amalgamation on other platforms
ifeq ($(UNAME), Darwin)
    OS                 := "macOS"
    LDFLAGS            :=
    DLSUFFIX           := .dylib
    LIBCFLAGS          :=
    DLFLAGS            := -dynamiclib
    CURSESLDFLAGS      := -lncurses

    SDL2FLAGS          := -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_IMAGE_LIBRARY -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    SDL2CONFIG_CFLAGS  := $(shell sdl2-config --cflags) -framework OpenGL
    SDL2CONFIG_LDFLAGS := $(shell sdl2-config --libs) -lSDL2_image -lSDL2_mixer -lSDL2_ttf
    CHIPMUNK_LDFLAGS   := -lm
else ifeq ($(UNAME), OpenBSD)
    OS                 := "OpenBSD"
    LDFLAGS            := -lm
    DLSUFFIX           := .so
    LIBCFLAGS          := -fPIC
    DLFLAGS            := -shared
    CURSESLDFLAGS      := -lncurses
    SOCKETLDFLAGS      :=

    SDL2FLAGS          := -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_IMAGE_LIBRARY -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    SDL2CONFIG_CFLAGS  := $(shell sdl2-config --cflags)
    SDL2CONFIG_LDFLAGS := $(shell sdl2-config --libs) -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lGLU
    CHIPMUNK_LDFLAGS   := -lm -lpthread
    MAKE=gmake
else ifeq ($(UNAME), FreeBSD)
    OS                 := "FreeBSD"
    LDFLAGS            := -lm
    DLSUFFIX           := .so
    LIBCFLAGS          := -fPIC
    DLFLAGS            := -shared
    CURSESLDFLAGS      := -lncursesw
    SOCKETLDFLAGS      :=

    SDL2FLAGS          := -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_IMAGE_LIBRARY -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY
    SDL2CONFIG_CFLAGS  := $(shell sdl2-config --cflags)
    SDL2CONFIG_LDFLAGS := $(shell sdl2-config --libs) -lSDL2_image -lSDL2_mixer -lSDL2_ttf
    CHIPMUNK_LDFLAGS   := -lm
else ifeq ($(UNAME), NetBSD)
    OS                 := "NetBSD"
    LDFLAGS            := -lm
    DLSUFFIX           := .so
    LIBCFLAGS          := -fPIC
    DLFLAGS            := -shared
    CURSESLDFLAGS      := -lcurses -lterminfo
    SOCKETLDFLAGS      :=

    SDL2FLAGS          := -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY
    SDL2CONFIG_CFLAGS  := $(shell sdl2-config --cflags)
    SDL2CONFIG_LDFLAGS := $(shell sdl2-config --libs) -lSDL2_ttf
else ifeq ($(UNAME)$(MACHINE), Haikux86_64)
    OS                 := "X86_64 Haiku"
    LDFLAGS            := -lm -Wl,-export-dynamic
    DLSUFFIX           := .so
    LIBCFLAGS          := -fPIC
    DLFLAGS            := -shared
    CURSESLDFLAGS      := -lncursesw
    SOCKETLDFLAGS      := -L/system/lib -lnetwork

    SDL2FLAGS          := -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_IMAGE_LIBRARY -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    SDL2CONFIG_CFLAGS  := $(shell sdl2-config --cflags)
    SDL2CONFIG_LDFLAGS := $(shell sdl2-config --libs) -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lGLU
    CHIPMUNK_LDFLAGS   := -lm
else ifeq ($(UNAME)$(MACHINE), HaikuBePC)
    CC                 := gcc-x86
    OS                 := "Haiku"
    LDFLAGS            := -lm -Wl,-export-dynamic
    DLSUFFIX           := .so
    LIBCFLAGS          := -fPIC
    DLFLAGS            := -shared
    CURSESLDFLAGS      := -lncursesw
    SOCKETLDFLAGS      := -L/system/lib -lnet

    SDL2FLAGS          := -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_IMAGE_LIBRARY -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY
    SDL2CONFIG_CFLAGS  := $(shell sdl2-config --cflags)
    SDL2CONFIG_LDFLAGS := $(shell sdl2-config --libs) -lSDL2_image -lSDL2_mixer -lSDL2_ttf
    CHIPMUNK_LDFLAGS   := -lm
else ifeq ($(UNAME), Linux)
    OS                 := "GNU/Linux"
    LDFLAGS            := -lm -Wl,-export-dynamic -ldl
    DLSUFFIX           := .so
    LIBCFLAGS          := -fPIC
    DLFLAGS            := -shared
    CURSESLDFLAGS      := -lncursesw
    SOCKETLDFLAGS      :=
    SDL2FLAGS          := -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_IMAGE_LIBRARY -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_MIXER_LIBRARY -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_SDL_TTF_LIBRARY -DOCTASPIRE_DERN_SDL2_PLUGIN_USE_OPENGL2_LIBRARY
    SDL2CONFIG_CFLAGS  := $(shell sdl2-config --cflags)
    SDL2CONFIG_LDFLAGS := $(shell sdl2-config --libs) -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lGLU
    CHIPMUNK_LDFLAGS   := -lm
endif

.PHONY: oscheck development development-repl submodules-init submodules-pull clean codestyle cppcheck valgrind test coverage perf-linux major minor patch push tag

all: oscheck submodulecheck development

oscheck:
ifeq ($(OS), "Unknown")
    $(error "This platform is not supported for development builds. Please use the release.")
endif

define init_submodules
    @echo "--  Initializing submodules..."
    @git submodule init --quiet
    @git submodule update --quiet
    @echo "OK  Done."
endef

submodulecheck:
ifeq (,$(wildcard dev/external/octaspire_core/release/octaspire-core-amalgamated.c))
	$(call init_submodules)
endif

###############################################################################
####### Development part: build using separate implementation files ###########
###############################################################################

development: oscheck submodulecheck octaspire-dern-repl octaspire-dern-unit-test-runner libdern_socket$(DLSUFFIX) libdern_dir$(DLSUFFIX) libdern_easing$(DLSUFFIX) libdern_animation$(DLSUFFIX) libdern_ncurses$(DLSUFFIX) libdern_sdl2$(DLSUFFIX) libdern_sqlite3$(DLSUFFIX) libdern_chipmunk$(DLSUFFIX)

octaspire-dern-repl: $(SRCDIR)octaspire_dern_repl.o $(DEVOBJS)
	$(info LD  $@)
	@$(CC) $(CFLAGS) $(SRCDIR)octaspire_dern_repl.o $(DEVOBJS) -o $@ $(LDFLAGS)

$(SRCDIR)octaspire_dern_repl.o: $(SRCDIR)octaspire_dern_repl.c
	$(info CC  $<)
	@$(CC) $(CFLAGS) -c -I dev/include -I $(CORDIR) -I dev -DOCTASPIRE_CORE_AMALGAMATED_IMPLEMENTATION $< -o $@

octaspire-dern-unit-test-runner: $(TESTDR)test.o $(TESTDR)test_dern_lexer.o $(TESTDR)test_dern_vm.o $(DEVOBJS)
	$(info LD  $@)
	@$(CC) $(CFLAGS) $(TESTDR)test.o $(TESTDR)test_dern_lexer.o $(TESTDR)test_dern_vm.o $(TESTOBJS) -o $@ $(LDFLAGS)

$(TESTDR)test.o: $(TESTDR)test.c
	$(info CC  $<)
	@$(CC) $(CFLAGS) -c -I dev/include -I $(CORDIR) -I dev -DOCTASPIRE_CORE_AMALGAMATED_IMPLEMENTATION $< -o $@

$(TESTDR)test_dern_lexer.o: $(TESTDR)test_dern_lexer.c
	$(info CC  $<)
	@$(CC) $(CFLAGS) -c -I dev/include -I $(CORDIR) -I dev $< -o $@

$(TESTDR)test_dern_vm.o: $(TESTDR)test_dern_vm.c
	$(info CC  $<)
	@$(CC) $(CFLAGS) -c -I dev/include -I $(CORDIR) -I dev $< -o $@

libdern_socket$(DLSUFFIX): $(PLUGINDIR)dern_socket.c $(AMALGAMATION)
	$(info PC  $<)
	@$(CC) $(CFLAGS) $(LIBCFLAGS) $(DLFLAGS) $(LDFLAGS) $(SOCKETLDFLAGS) -DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION -I release -o $@ $<

libdern_dir$(DLSUFFIX): $(PLUGINDIR)dern_dir.c $(AMALGAMATION)
	$(info PC  $<)
	@$(CC) $(CFLAGS) $(LIBCFLAGS) $(DLFLAGS) -DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION -I release -o $@ $< $(LDFLAGS)

libdern_easing$(DLSUFFIX): $(PLUGINDIR)dern_easing.c $(AMALGAMATION)
	$(info PC  $<)
	@$(CC) $(CFLAGS) $(LIBCFLAGS) $(DLFLAGS) -DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION -I release -o $@ $< $(LDFLAGS)

libdern_animation$(DLSUFFIX): $(PLUGINDIR)dern_animation.c $(AMALGAMATION)
	$(info PC  $<)
	@$(CC) $(CFLAGS) $(LIBCFLAGS) $(DLFLAGS) -DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION -I release -o $@ $<

libdern_ncurses$(DLSUFFIX): $(PLUGINDIR)dern_ncurses.c $(AMALGAMATION)
	$(info PC  $<)
	@$(CC) $(CFLAGS) $(LIBCFLAGS) $(DLFLAGS) -DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION -I release -o $@ $< $(CURSESLDFLAGS) $(LDFLAGS)

libdern_sdl2$(DLSUFFIX): $(PLUGINDIR)dern_sdl2.c $(AMALGAMATION)
	$(info PC  $<)
	@$(CC) $(CFLAGS) -I release $(SDL2CONFIG_CFLAGS) $(SDL2FLAGS) $(LIBCFLAGS) $(DLFLAGS) -DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION -o $@ $< $(SDL2CONFIG_LDFLAGS)

libsqlite3$(DLSUFFIX): $(PLUGINDIR)external/sqlite3/sqlite3.c
	$(info EC  sqlite3)
	@$(CC) $(LIBCFLAGS) -c -o $@ $< $(SQLITE3_LDFLAGS)

libdern_sqlite3$(DLSUFFIX): $(PLUGINDIR)dern_sqlite3.c $(AMALGAMATION) libsqlite3$(DLSUFFIX)
	$(info PC  $<)
	@$(CC) $(CFLAGS) -I release -I $(PLUGINDIR)external/sqlite3 $(LIBCFLAGS) $(DLFLAGS) -DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION -o $@ $< $(SQLITE3_LDFLAGS)  -L . -lsqlite3

$(CHIPMUNK_PATH)%.o: $(CHIPMUNK_PATH)%.c
	$(info CC  $<)
	@$(CC) $(CFLAGS) $(LIBCFLAGS) -I $(PLUGINDIR)external/chipmunk/include -I $(PLUGINDIR)external/chipmunk/include/chipmunk -c $< -o $@

libchipmunk$(DLSUFFIX): $(CHIPMUNK_OBJS)
	$(info EC  chipmunk)
	@$(CC) $(DLFLAGS) -o $@ $^ $(CHIPMUNK_LDFLAGS)

libdern_chipmunk$(DLSUFFIX): $(PLUGINDIR)dern_chipmunk.c $(AMALGAMATION) libchipmunk$(DLSUFFIX)
	$(info PC  $<)
	@$(CC) $(CFLAGS) -I release -I $(PLUGINDIR)external/chipmunk/include/ -I $(PLUGINDIR)external/chipmunk/include/chipmunk $(LIBCFLAGS) $(DLFLAGS) -DOCTASPIRE_DERN_AMALGAMATED_IMPLEMENTATION -o $@ $< $(CHIPMUNK_LDFLAGS)  -L . -lchipmunk


perf-linux: octaspire-dern-unit-test-runner
	@echo "+---------------------------------------------------------------------+"
	@echo "| Going to measure perf. This works only in GNU/Linux. If permissions |"
	@echo "| are not OK, you might to need to run:                               |"
	@echo "|                                                                     |"
	@echo "| sudo sh -c 'echo 1 > /proc/sys/kernel/perf_event_paranoid'          |"
	@echo "| sudo sh -c 'echo 0 > /proc/sys/kernel/kptr_restrict'                |"
	@echo "|                                                                     |"
	@echo "| to set less paranoid settings. Then run make $@ again.      |"
	@echo "+---------------------------------------------------------------------+"
	@perf record ./octaspire-dern-unit-test-runner --write-test-files
	@perf report
%.o: %.c
	$(info CC  $<)
	@$(CC) $(CFLAGS) -c -I dev/include -I $(CORDIR) -I dev $*.c -o $*.o


###############################################################################
####### Release part: build using amalgamation ################################
###############################################################################

amalgamation: $(RELDIR)octaspire-dern-repl $(RELDIR)games/octaspire-lightcube.dern $(RELDIR)games/octaspire-maze.dern

$(RELDIR)octaspire-dern-repl: $(CORDIR)LICENSE $(AMALGAMATION) $(PLUGINS)
	@sh $(ETCDIR)build_amalgamation.sh

$(RELDIR)games/octaspire-lightcube.dern: $(DEVGAMEDIR)lightcube/entities.png
	 @$(ETCDIR)base64_replace_image.sh $< $@

$(RELDIR)games/octaspire-maze.dern: $(DEVGAMEDIR)maze/entities.png
	 @$(ETCDIR)base64_replace_image.sh $< $@

$(CORDIR)LICENSE:
	@$(MAKE) submodules-init --silent

submodules-init:
	$(call init_submodules)

submodules-pull:
	@echo "--  Pulling submodules..."
	@git submodule update --recursive --remote --quiet
	@$(MAKE) -s TAGS
	@echo "OK  Done."

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
	@echo "++  Creating amalgamation..."
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

$(RELDOCDIR)dern-manual.html: $(DEVDOCDIR)book/dern-manual.htm $(DOCEXAMPLES)
	$(info Generate $@)
	@cp $(RELDIR)tool-support/source-highlight/dern.lang .
	@cp $(DEVDOCDIR)book/lang.map .
	@python2 $(DEVDOCDIR)book/build_book.py $< > $@
	$(info Cleaning temp html files ...)
	@find $(DEVDOCDIR)book/examples/ -name '*.html' -delete

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
                $(PLUGINDIR)external/chipmunk/src/*.o                                 \
                coverage                                                              \
                $(SRCDIR)*.o                                                          \
                $(TESTDR)*.o                                                          \
                octaspire-dern-repl                                                   \
                octaspire-dern-unit-test-runner                                       \
                perf.data perf.data.old                                               \
                *.so *.dylib                                                          \
                dern-example-database.db                                              \
                $(DEVDOCDIR)book/examples/sh/*.html                                   \
                $(DEVDOCDIR)book/examples/dern/*.html                                 \
                $(DEVDOCDIR)book/examples/c/*.html                                    \
                dern.lang lang.map

codestyle:
	@vera++ --root dev/external/vera --profile octaspire-plugin --error $(wildcard $(SRCDIR)*.[ch])
	@vera++ --root dev/external/vera --profile octaspire-plugin --error $(filter-out $(PLUGINDIR)stb_image.h, $(wildcard $(PLUGINDIR)*.[ch]))

cppcheck:
	@cppcheck --std=c99 -I dev/include dev/external/ocraspire_core/include --enable=warning,performance,portability --verbose --quiet $(wildcard $(SRCDIR)*.[ch]) $(filter-out $(PLUGINDIR)stb_image.h, $(wildcard $(PLUGINDIR)*.[ch]))

# Test both amalgamation and development version with valgrind.
valgrind: $(RELDIR)octaspire-dern-repl octaspire-dern-unit-test-runner
	@valgrind --leak-check=full --track-origins=yes --error-exitcode=1 $(RELDIR)octaspire-dern-unit-test-runner --write-test-files
	@valgrind --leak-check=full --track-origins=yes --error-exitcode=1 ./octaspire-dern-unit-test-runner --write-test-files
	@valgrind --leak-check=full --track-origins=yes --error-exitcode=1 $(RELDIR)octaspire-dern-repl -e "(+ {D+1} {D+2} {D+3})(exit)"

# Test both amalgamation and development version.
test: $(RELDIR)octaspire-dern-repl octaspire-dern-unit-test-runner
	@$(RELDIR)octaspire-dern-unit-test-runner --write-test-files
	@./octaspire-dern-unit-test-runner --write-test-files
	@$(RELDIR)octaspire-dern-repl -e "(+ {D+1} {D+2} {D+3})(exit)"

coverage: $(AMALGAMATION)
	@sh $(ETCDIR)build_amalgamation.sh "gcc --coverage"
	@$(RELDIR)/octaspire-dern-unit-test-runner --write-test-files
	@lcov --no-external --capture --directory release --output-file $(RELDIR)coverage.info

coverage-show: coverage
	@genhtml $(RELDIR)coverage.info --output-directory coverage
	@xdg-open coverage/index.html &

# Create a TAGS file that allows the jumping between definitions and uses of
# things (for example functions) when editing C or Dern code. Dern files are
# indexed as scheme files; it seems to be working.
TAGS: $(TAGS_C_FILES) $(TAGS_DERN_FILES)
	@etags -o $@ $(TAGS_C_FILES)
	@etags -o $@ --append --language scheme $(TAGS_DERN_FILES)

major:
	@sh dev/etc/bump-version.sh major
	@rm -f release/octaspire-dern-amalgamated.c      # This ensures that the version number is updated
	@$(MAKE) -s release/octaspire-dern-amalgamated.c # also in the amalgamation.
	@$(MAKE) -s TAGS
	@echo "OK  Done."

minor:
	@sh dev/etc/bump-version.sh minor
	@rm -f release/octaspire-dern-amalgamated.c      # This ensures that the version number is updated
	@$(MAKE) -s release/octaspire-dern-amalgamated.c # also in the amalgamation.
	@$(MAKE) -s TAGS
	@echo "OK  Done."

patch:
	@sh dev/etc/bump-version.sh patch
	@rm -f release/octaspire-dern-amalgamated.c      # This ensures that the version number is updated
	@$(MAKE) -s release/octaspire-dern-amalgamated.c # also in the amalgamation.
	@$(MAKE) -s TAGS
	@echo "OK  Done."

tag:
	@sh dev/etc/tag-version.sh

push:
	@git push origin-gitlab
	@git push origin-gitlab --tags
	@git push origin-bitbucket
	@git push origin-bitbucket --tags
	@git push origin-sr
	@git push origin-sr --tags
	@git push origin-github --tags
	@git push origin-github
