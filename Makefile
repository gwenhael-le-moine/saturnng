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
FULL_WARNINGS ?= no
WITH_SDL ?= yes
WITH_GTK ?= no

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
	SDL_SRC = src/ui4x/sdl.c
	SDL_HEADERS = src/ui4x/sdl.h
endif

ifeq ($(WITH_GTK), yes)
	GTK_CFLAGS = -DHAS_GTK=1 $(shell "$(PKG_CONFIG)" --cflags gtk4)
	GTK_LIBS = $(shell "$(PKG_CONFIG)" --libs gtk4)
	GTK_SRC = src/ui4x/gtk.c
	GTK_HEADERS = src/ui4x/gtk.h
endif

LIBS = -L./src/libChf -lChf $(NCURSES_LIBS) $(LUA_LIBS) $(SDL_LIBS) $(GTK_LIBS)

HEADERS = src/options.h \
	src/core/config.h \
	src/core/disk_io.h \
	src/core/flash49.h \
	src/core/keyb.h \
	src/core/machdep.h \
	src/core/modules.h \
	src/core/monitor.h \
	src/core/serial.h \
	src/core/x_func.h \
	src/core/dis.h \
	src/core/cpu.h \
	src/core/cpu_buscc.h \
	src/core/chf_wrapper.h \
	src/ui4x/api.h \
	src/ui4x/bitmaps_misc.h \
	src/ui4x/common.h \
	src/ui4x/ncurses.h \
	src/ui4x/inner.h \
	$(SDL_HEADERS) \
	$(GTK_HEADERS)

SRC = src/main.c \
	src/options.c \
	src/ui4x_api_impl.c \
	src/core/cpu.c \
	src/core/cpu_buscc.c \
	src/core/dis.c \
	src/core/disk_io.c \
	src/core/emulator.c \
	src/core/flash49.c \
	src/core/hdw.c \
	src/core/hw_config.c \
	src/core/keyb.c \
	src/core/modules.c \
	src/core/monitor.c \
	src/core/romram.c \
	src/core/romram49.c \
	src/core/serial.c \
	src/core/x_func.c \
	src/core/chf_messages.c \
	src/ui4x/fonts.c \
	src/ui4x/48sx.c \
	src/ui4x/48gx.c \
	src/ui4x/49g.c \
	src/ui4x/50g.c \
	src/ui4x/common.c \
	src/ui4x/ncurses.c \
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

override CFLAGS := -std=c23 \
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
	clang-format -i src/*.c src/*.h src/core/*.c src/core/*.h src/ui4x/*.c src/ui4x/*.h
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
