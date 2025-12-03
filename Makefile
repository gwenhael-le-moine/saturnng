# Simple Makefile to build saturn
#
# The cc-option function and the C{,PP}FLAGS logic were copied from the
# fsverity-utils project.
# https://git.kernel.org/pub/scm/fs/fsverity/fsverity-utils.git/
# The governing license can be found in the LICENSE file or at
# https://opensource.org/license/MIT.

NAME = saturn

VERSION_MAJOR = 5
VERSION_MINOR = 6
PATCHLEVEL = 0

PREFIX ?= /usr
BINDIR ?= $(PREFIX)/bin
DATADIR ?= $(PREFIX)/share/$(NAME)
MENUDIR ?= $(PREFIX)/share/applications
MANDIR ?= $(PREFIX)/share/man/man1
DOCDIR ?= $(PREFIX)/doc/$(NAME)
INFODIR ?= $(PREFIX)/info

LUA_VERSION ?= lua
PKG_CONFIG ?= pkg-config

OPTIM ?= 2
FULL_WARNINGS ?= no
WITH_GTK ?= yes
WITH_SDL ?= yes
WITH_SDL2 ?= no

cc-option = $(shell if $(CC) $(1) -c -x c /dev/null -o /dev/null > /dev/null 2>&1; \
		  then echo $(1); fi)

### lua
LUA_CFLAGS = $(shell "$(PKG_CONFIG)" --cflags $(LUA_VERSION))
LUA_LIBS = $(shell "$(PKG_CONFIG)" --libs $(LUA_VERSION))

NCURSES_CFLAGS = $(shell "$(PKG_CONFIG)" --cflags ncursesw)
NCURSES_LIBS = $(shell "$(PKG_CONFIG)" --libs ncursesw)

ifeq ($(WITH_SDL), yes)
	SDL_CFLAGS = $(shell "$(PKG_CONFIG)" --cflags sdl3) -DHAS_SDL=1
	SDL_LIBS = $(shell "$(PKG_CONFIG)" --libs sdl3)
	SDL_SRC = src/ui4x/src/sdl.c
	SDL_HEADERS = src/ui4x/src/sdl.h
endif

ifeq ($(WITH_SDL2), yes)
	SDL_CFLAGS = $(shell "$(PKG_CONFIG)" --cflags sdl2) -DHAS_SDL=1 -DHAS_SDL2=1
	SDL_LIBS = $(shell "$(PKG_CONFIG)" --libs sdl2)
	SDL_SRC = src/ui4x/src/sdl.c
	SDL_HEADERS = src/ui4x/src/sdl.h
endif

ifeq ($(WITH_GTK), yes)
	GTK_CFLAGS = -DHAS_GTK=1 $(shell "$(PKG_CONFIG)" --cflags gtk4)
	GTK_LIBS = $(shell "$(PKG_CONFIG)" --libs gtk4)
	GTK_SRC = src/ui4x/src/gtk.c
	GTK_HEADERS = src/ui4x/src/gtk.h
endif

LIBS = -L./src/libChf -lChf $(NCURSES_LIBS) $(LUA_LIBS) $(SDL_LIBS) $(GTK_LIBS)

HEADERS = src/options.h \
	src/emulator_api.h \
	src/core/disk_io.h \
	src/core/flash49.h \
	src/core/romram49.h \
	src/core/romram48.h \
	src/core/keyboard.h \
	src/core/types.h \
	src/core/bus.h \
	src/core/monitor.h \
	src/core/serial.h \
	src/core/disassembler.h \
	src/core/hdw.h \
	src/core/emulator.h \
	src/core/cpu.h \
	src/core/chf_wrapper.h \
	src/ui4x/src/api.h \
	src/ui4x/src/bitmaps_misc.h \
	src/ui4x/src/ncurses.h \
	src/ui4x/src/inner.h \
	$(SDL_HEADERS) \
	$(GTK_HEADERS)

SRC = src/main.c \
	src/options.c \
	src/emulator_api.c \
	src/core/cpu.c \
	src/core/disassembler.c \
	src/core/disk_io.c \
	src/core/emulator.c \
	src/core/flash49.c \
	src/core/hdw.c \
	src/core/keyboard.c \
	src/core/bus.c \
	src/core/monitor.c \
	src/core/romram48.c \
	src/core/romram49.c \
	src/core/serial.c \
	src/core/chf_messages.c \
	src/ui4x/src/fonts.c \
	src/ui4x/src/bitmaps_misc.c \
	src/ui4x/src/48sx.c \
	src/ui4x/src/48gx.c \
	src/ui4x/src/49g.c \
	src/ui4x/src/40g.c \
	src/ui4x/src/50g.c \
	src/ui4x/src/api.c \
	src/ui4x/src/ncurses.c \
	$(SDL_SRC) \
	$(GTK_SRC)
OBJS = $(SRC:.c=.o)

ifeq ($(FULL_WARNINGS), no)
EXTRA_WARNING_FLAGS := -Wno-unused-function \
	-Wno-redundant-decls \
	$(call cc-option,-Wno-maybe-uninitialized) \
	$(call cc-option,-Wno-discarded-qualifiers) \
	$(call cc-option,-Wno-uninitialized) \
	$(call cc-option,-Wno-ignored-qualifiers)
else
EXTRA_WARNING_FLAGS := -Wunused-function \
	-Wredundant-decls \
	-fsanitize=thread \
	$(call cc-option,-Wunused-variable)
endif

override CFLAGS := -std=gnu2x \
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
	$(SDL_CFLAGS) \
	$(GTK_CFLAGS) \
	$(NCURSES_CFLAGS) \
	$(LUA_CFLAGS) \
	-O$(OPTIM) \
	-D_GNU_SOURCE=1 \
	-DVERSION_MAJOR=$(VERSION_MAJOR) \
	-DVERSION_MINOR=$(VERSION_MINOR) \
	-DPATCHLEVEL=$(PATCHLEVEL) \
	-DDATADIR=\"$(DATADIR)\" \
	-I./src/ \
	-I./src/libChf/src/ \
	$(CFLAGS)

.PHONY: all clean clean-all pretty-code install mrproper get-roms install

all: src/libChf/libChf.a dist/$(NAME) docs

# Building
src/libChf/libChf.a:
	make -C src/libChf #MT=yes

dist/$(NAME): $(OBJS) $(HEADERS) src/libChf/libChf.a
	$(CC) $(OBJS) src/libChf/libChf.a -o $@ $(CFLAGS) $(LIBS)

doc:
	make -C docs

# Cleaning
clean:
	rm -f $(OBJS)
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
	clang-format -i src/*.c src/*.h src/core/*.c src/core/*.h src/ui4x/src/*.c src/ui4x/src/*.h
	make -C src/libChf pretty-code

# Dependencies
get-roms:
	make -C dist/ROMs get-roms

# Installation
install: dist/$(NAME) doc
	install -m 755 -d -- $(DESTDIR)$(BINDIR)
	install -c -m 755 dist/$(NAME) $(DESTDIR)$(BINDIR)/$(NAME)
	install -c -m 755 dist/saturn48gx $(DESTDIR)$(BINDIR)/saturn48gx
	install -c -m 755 dist/saturn48sx $(DESTDIR)$(BINDIR)/saturn48sx
	install -c -m 755 dist/saturn40g $(DESTDIR)$(BINDIR)/saturn40g
	install -c -m 755 dist/saturn49g $(DESTDIR)$(BINDIR)/saturn49g

	install -m 755 -d -- $(DESTDIR)$(DATADIR)
	install -c -m 644 dist/hplogo.png $(DESTDIR)$(DATADIR)/hplogo.png
	cp -R dist/ROMs/ $(DESTDIR)$(DATADIR)/
	cp src/ui4x/*.css "$(DESTDIR)$(DATADIR)/"

	install -m 755 -d -- $(DESTDIR)$(DOCDIR)
	cp -R COPYING LICENSE README* ./*.png docs-4.1.1.1 docs/*.{dvi,ps,pdf} $(DESTDIR)$(DOCDIR)

	install -m 755 -d -- $(DESTDIR)$(INFODIR)
	cp docs/*.info $(DESTDIR)$(INFODIR)

	install -m 755 -d -- $(DESTDIR$(MENUDIR)
	sed "s|@PREFIX@|$(PREFIX)|g" dist/saturn48gx.desktop > $(DESTDIR$(MENUDIR)/saturn48gx.desktop
	sed "s|@PREFIX@|$(PREFIX)|g" dist/saturn48sx.desktop > $(DESTDIR$(MENUDIR)/saturn48sx.desktop
	sed "s|@PREFIX@|$(PREFIX)|g" dist/saturn49g.desktop > $(DESTDIR$(MENUDIR)/saturn49g.desktop
	sed "s|@PREFIX@|$(PREFIX)|g" dist/saturn40g.desktop > $(DESTDIR$(MENUDIR)/saturn40g.desktop

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(NAME)
	rm -f $(DESTDIR)$(BINDIR)/saturn48gx
	rm -f $(DESTDIR)$(BINDIR)/saturn48sx
	rm -f $(DESTDIR)$(BINDIR)/saturn40g
	rm -f $(DESTDIR)$(BINDIR)/saturn49g
	rm -fr $(DESTDIR)$(DATADIR)
	rm -fr $(DESTDIR)$(DOCDIR)
	rm -f $(DESTDIR$(MENUDIR)/saturn48gx.desktop
	rm -f $(DESTDIR$(MENUDIR)/saturn48sx.desktop
	rm -f $(DESTDIR$(MENUDIR)/saturn49g.desktop
	rm -f $(DESTDIR$(MENUDIR)/saturn40g.desktop
