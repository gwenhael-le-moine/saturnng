# Simple Makefile to build saturn
#
# The cc-option function and the C{,PP}FLAGS logic were copied from the
# fsverity-utils project.
# https://git.kernel.org/pub/scm/fs/fsverity/fsverity-utils.git/
# The governing license can be found in the LICENSE file or at
# https://opensource.org/license/MIT.

NAME = saturn

VERSION_MAJOR = 5
VERSION_MINOR = 5
PATCHLEVEL = 2

PREFIX ?= /usr
DOCDIR ?= $(PREFIX)/doc/$(NAME)
INFODIR ?= $(PREFIX)/info

LUA_VERSION ?= lua
PKG_CONFIG ?= pkg-config

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

SDLCFLAGS = $(shell "$(PKG_CONFIG)" --cflags sdl3)
SDLLIBS = $(shell "$(PKG_CONFIG)" --libs sdl3)

NCURSESCFLAGS = $(shell "$(PKG_CONFIG)" --cflags ncursesw)
NCURSESLIBS = $(shell "$(PKG_CONFIG)" --libs ncursesw)

### lua
LUACFLAGS = $(shell "$(PKG_CONFIG)" --cflags $(LUA_VERSION))
LUALIBS = $(shell "$(PKG_CONFIG)" --libs $(LUA_VERSION))

FULL_WARNINGS = no

DOTOS = src/cpu.o \
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
	src/options.o \
	src/main.o

DOTOS_UI4x = src/ui4x/fonts.o \
	src/ui4x/48sx.o \
	src/ui4x/48gx.o \
	src/ui4x/49g.o \
	src/ui4x/50g.o \
	src/ui4x/common.o \
	src/ui4x/sdl.o \
	src/ui4x/ncurses.o \
	src/ui4x/emulator.o

HEADERS = src/ui4x/bitmaps_misc.h \
	src/ui4x/common.h \
	src/options.h \
	src/ui4x/ncurses.h \
	src/ui4x/sdl.h \
	src/ui4x/inner.h \
	src/ui4x/emulator.h \
	src/disk_io.h \
	src/flash49.h \
	src/keyb.h \
	src/machdep.h \
	src/modules.h \
	src/monitor.h \
	src/serial.h \
	src/x_func.h \
	src/chf_messages.h \
	src/config.h \
	src/cpu.h \
	src/debug.h

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
	$(LUACFLAGS) \
	$(CFLAGS)

override CPPFLAGS := -I./src/ -D_GNU_SOURCE=1 \
	$(CPPFLAGS)

.PHONY: all clean clean-all pretty-code install mrproper get-roms install

all: src/libChf/libChf.a dist/$(NAME) docs

# Building
src/libChf/libChf.a:
	make -C src/libChf #MT=yes

dist/$(NAME): $(DOTOS) $(DOTOS_UI4x) $(HEADERS) src/libChf/libChf.a
	$(CC) $(DOTOS) $(DOTOS_UI4x) src/libChf/libChf.a -o $@ $(CFLAGS) $(LIBS) $(SDLLIBS) $(NCURSESLIBS) $(LUALIBS)

# UNUSED
dist/pack: src/pack.o src/disk_io.o src/debug.o src/libChf/libChf.a
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS)

doc:
	make -C docs

# Cleaning
clean:
	rm -f src/*.o src/ui4x/*.o
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
	clang-format -i src/*.c src/*.h src/ui4x/*.c src/ui4x/*.h
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
	cp -R COPYING LICENSE README* ./*.png docs-4.1.1.1 docs/*.{dvi,ps,pdf} $(DESTDIR)$(DOCDIR)

	install -m 755 -d -- $(DESTDIR)$(INFODIR)
	cp docs/*.info $(DESTDIR)$(INFODIR)

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
