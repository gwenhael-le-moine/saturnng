# Simple Makefile to build saturn
#
# The cc-option function and the C{,PP}FLAGS logic were copied from the
# fsverity-utils project.
# https://git.kernel.org/pub/scm/fs/fsverity/fsverity-utils.git/
# The governing license can be found in the LICENSE file or at
# https://opensource.org/license/MIT.

NAME = saturn

PREFIX ?= /usr
DOCDIR ?= $(PREFIX)/doc/$(NAME)

VERSION_MAJOR = 5
VERSION_MINOR = 1
PATCHLEVEL = 0

OPTIM ?= 2

override CFLAGS := -O$(OPTIM) \
	-D_GNU_SOURCE=1 \
	-DVERSION_MAJOR=$(VERSION_MAJOR) \
	-DVERSION_MINOR=$(VERSION_MINOR) \
	-DPATCHLEVEL=$(PATCHLEVEL) \
	-I./src/ \
	-I./src/libChf/src/ \
	$(CFLAGS)

LIBS = -L./src/libChf -lChf

SDLCFLAGS = $(shell pkg-config --cflags sdl2)
SDLLIBS = $(shell pkg-config --libs sdl2)

NCURSESCFLAGS = $(shell pkg-config --cflags ncursesw)
NCURSESLIBS = $(shell pkg-config --libs ncursesw)

FULL_WARNINGS = no

DOTOS = src/cpu.o \
	src/debug.o \
	src/dis.o \
	src/disk_io.o \
	src/disk_io_obj.o \
	src/emulator.o \
	src/flash49.o \
	src/hdw.o \
	src/hw_config.o \
	src/keyb.o \
	src/modules.o \
	src/monitor.o \
	src/romram.o \
	src/romram49.o \
	src/serial.o \
	src/x_func.o \
	src/chf_messages.o \
	src/main.o

DOTOS_UI4x = src/ui4x/config.o \
	src/ui4x/fonts.o \
	src/ui4x/48sx.o \
	src/ui4x/48gx.o \
	src/ui4x/49g.o \
	src/ui4x/common.o \
	src/ui4x/sdl2.o \
	src/ui4x/ncurses.o \
	src/ui4x/emulator.o

MAKEFLAGS +=-j$(NUM_CORES) -l$(NUM_CORES)

cc-option = $(shell if $(CC) $(1) -c -x c /dev/null -o /dev/null > /dev/null 2>&1; \
	      then echo $(1); fi)

ifeq ($(FULL_WARNINGS), no)
EXTRA_WARNING_FLAGS := -Wno-unused-function \
	-Wno-redundant-decls \
	$(call cc-option,-Wno-maybe-uninitialized) \
	$(call cc-option,-Wno-discarded-qualifiers) \
	$(call cc-option,-Wno-uninitialized) \
	$(call cc-option,-Wno-ignored-qualifiers)
endif

ifeq ($(FULL_WARNINGS), yes)
EXTRA_WARNING_FLAGS := -Wunused-function \
	-Wredundant-decls \
	-fsanitize-trap \
	$(call cc-option,-Wunused-variable)
endif

override CFLAGS := -std=c11 \
	-Wall -Wextra -Wpedantic \
	-Wformat=2 -Wshadow \
	-Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
	-Wnested-externs -Wmissing-include-dirs \
	-Wdouble-promotion \
	-Wno-sign-conversion \
	-Wno-unused-variable \
	-Wno-unused-parameter \
	-Wno-conversion \
	-Wno-format-nonliteral \
	$(call cc-option,-Wjump-misses-init) \
	$(call cc-option,-Wlogical-op) \
	$(call cc-option,-Wno-unknown-warning-option) \
	$(EXTRA_WARNING_FLAGS) \
	$(SDLCFLAGS) \
	$(NCURSESCFLAGS) \
	$(CFLAGS)

override CPPFLAGS := -I./src/ -D_GNU_SOURCE=1 \
	$(CPPFLAGS)

.PHONY: all clean clean-all pretty-code install mrproper get-roms install

all: src/libChf/libChf.a dist/$(NAME) docs

# Building
src/libChf/libChf.a:
	make -C src/libChf

dist/$(NAME): $(DOTOS) $(DOTOS_UI4x) src/libChf/libChf.a
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS) $(SDLLIBS) $(NCURSESLIBS)

dist/pack: src/pack.o src/disk_io.o src/debug.o src/libChf/libChf.a
	# UNUSED
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS)

doc:
	make -C docs
	make -C src/libChf doc

# Cleaning
clean:
	rm -f src/*.o
	make -C src/libChf clean
	make -C docs clean

mrproper: clean
	rm -f dist/$(NAME) dist/pack
	make -C dist/ROMs mrproper
	make -C src/libChf mrproper
	make -C docs mrproper

clean-all: mrproper

# Formatting
pretty-code:
	clang-format -i src/*.c src/*.h
	make -C src/libChf pretty-code

# Dependencies
get-roms:
	make -C dist/ROMs get-roms

# Installation
install: dist/$(NAME) doc
	install -m 755 -d -- $(DESTDIR)$(PREFIX)/bin
	install -c -m 755 dist/$(NAME) $(DESTDIR)$(PREFIX)/bin/$(NAME)
	install -c -m 755 dist/saturn48gx $(DESTDIR)$(PREFIX)/bin/saturn48gx
	install -c -m 755 dist/saturn48sx $(DESTDIR)$(PREFIX)/bin/saturn48sx
	install -c -m 755 dist/saturn40g $(DESTDIR)$(PREFIX)/bin/saturn40g
	install -c -m 755 dist/saturn49g $(DESTDIR)$(PREFIX)/bin/saturn49g

	install -m 755 -d -- $(DESTDIR)$(PREFIX)/share/$(NAME)
	install -c -m 644 dist/hplogo.png $(DESTDIR)$(PREFIX)/share/$(NAME)/hplogo.png
	cp -R dist/ROMs/ $(DESTDIR)$(PREFIX)/share/$(NAME)/

	install -m 755 -d -- $(DESTDIR)$(DOCDIR)
	cp -R COPYING LICENSE README* docs-4.1.1.1 docs/*.{info,dvi,ps,pdf} src/libChf/docs/*.{info,dvi,ps,pdf} $(DESTDIR)$(DOCDIR)

	install -m 755 -d -- $(DESTDIR)$(PREFIX)/share/applications
	sed "s|@PREFIX@|$(PREFIX)|g" dist/saturn48gx.desktop > $(DESTDIR)$(PREFIX)/share/applications/saturn48gx.desktop
	sed "s|@PREFIX@|$(PREFIX)|g" dist/saturn48sx.desktop > $(DESTDIR)$(PREFIX)/share/applications/saturn48sx.desktop
	sed "s|@PREFIX@|$(PREFIX)|g" dist/saturn49g.desktop > $(DESTDIR)$(PREFIX)/share/applications/saturn49g.desktop
	sed "s|@PREFIX@|$(PREFIX)|g" dist/saturn40g.desktop > $(DESTDIR)$(PREFIX)/share/applications/saturn40g.desktop

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(NAME)
	rm -f $(DESTDIR)$(PREFIX)/bin/saturn48gx
	rm -f $(DESTDIR)$(PREFIX)/bin/saturn48sx
	rm -f $(DESTDIR)$(PREFIX)/bin/saturn40g
	rm -f $(DESTDIR)$(PREFIX)/bin/saturn49g
	rm -fr $(DESTDIR)$(PREFIX)/share/$(NAME)
	rm -fr $(DESTDIR)$(DOCDIR)
	rm -f $(DESTDIR)$(PREFIX)/share/applications/saturn48gx.desktop
	rm -f $(DESTDIR)$(PREFIX)/share/applications/saturn48sx.desktop
	rm -f $(DESTDIR)$(PREFIX)/share/applications/saturn49g.desktop
	rm -f $(DESTDIR)$(PREFIX)/share/applications/saturn40g.desktop
