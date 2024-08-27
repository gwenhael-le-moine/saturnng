# Simple Makefile to build saturn_bertolotti
#
# The cc-option function and the C{,PP}FLAGS logic were copied from the
# fsverity-utils project.
# https://git.kernel.org/pub/scm/fs/fsverity/fsverity-utils.git/
# The governing license can be found in the LICENSE file or at
# https://opensource.org/license/MIT.

PREFIX = /usr
DOCDIR = $(PREFIX)/doc/saturn

OPTIM ?= 2

CFLAGS ?= -g -O$(OPTIM) -I./src/ -D_GNU_SOURCE=1 -I./libChf -L./libChf/st_build -lutil
LIBS = -lm -lChf -lXm

X11CFLAGS = $(shell pkg-config --cflags x11 xext)
X11LIBS = $(shell pkg-config --libs x11 xext xt)

FULL_WARNINGS = no

DOTOS = src/cpu.o \
	src/debug.o \
	src/dis.o \
	src/disk_io.o \
	src/disk_io_obj.o \
	src/display.o \
	src/emulator.o \
	src/flash49.o \
	src/hdw.o \
	src/hw_config.o \
	src/keyb.o \
	src/main.o \
	src/modules.o \
	src/monitor.o \
	src/romram.o \
	src/romram49.o \
	src/serial.o \
	src/x11.o \
	src/x_func.o

MSFS =	src/MSFs/cpu.msf \
	src/MSFs/debug.msf \
	src/MSFs/disk_io.msf \
	src/MSFs/flash49.msf \
	src/MSFs/modules.msf \
	src/MSFs/saturn.msf \
	src/MSFs/serial.msf \
	src/MSFs/util.msf \
	src/MSFs/x11.msf \
	src/MSFs/x_func.msf \
	libChf/chf.msf

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
	$(X11CFLAGS) \
	$(CFLAGS)

override CPPFLAGS := -I./src/ -D_GNU_SOURCE=1 \
	$(CPPFLAGS)

.PHONY: all clean clean-all pretty-code install mrproper get-roms install

all: libChf/st_build/libChf.a dist/saturn dist/pack dist/saturn.cat manual

# Building
libChf/st_build/libChf.a:
	make -C libChf

dist/saturn: $(DOTOS) libChf/st_build/libChf.a
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS) $(X11LIBS)

dist/pack: src/pack.o src/disk_io.o src/debug.o libChf/st_build/libChf.a
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS)

dist/saturn.cat: $(MSFS)
	for msf in $? ;	\
	  do gencat $@ $$msf ; \
	done

manual:
	make -C manual

# Cleaning
clean:
	rm -f src/*.o
	make -C libChf clean
	make -C manual clean

mrproper: clean
	rm -f dist/pack dist/saturn dist/saturn.cat
	make -C dist/ROMs mrproper

clean-all: mrproper

# Formatting
pretty-code:
	clang-format -i src/*.c src/*.h libChf/*.c libChf/*.h

# Dependencies
get-roms:
	make -C dist/ROMs get-roms

# Installation
install: dist/saturn dist/pack dist/saturn.cat dist/Saturn.ad manual
	install -m 755 -d -- $(DESTDIR)$(PREFIX)/bin
	install -c -m 755 dist/saturn $(DESTDIR)$(PREFIX)/bin/saturn
	sed "s|@PREFIX@|$(PREFIX)|g" dist/saturn48gx > $(DESTDIR)$(PREFIX)/bin/saturn48gx
	chmod 755 $(DESTDIR)$(PREFIX)/bin/saturn48gx
	sed "s|@PREFIX@|$(PREFIX)|g" dist/saturn49g > $(DESTDIR)$(PREFIX)/bin/saturn49g
	chmod 755 $(DESTDIR)$(PREFIX)/bin/saturn49g
	sed "s|@PREFIX@|$(PREFIX)|g" dist/saturn40g > $(DESTDIR)$(PREFIX)/bin/saturn40g
	chmod 755 $(DESTDIR)$(PREFIX)/bin/saturn40g

	install -c -m 644 dist/saturn.cat $(DESTDIR)$(PREFIX)/bin/saturn.cat #FIXME
	install -m 755 -d -- $(DESTDIR)$(PREFIX)/share/locale/C/LC_MESSAGES
	install -c -m 644 dist/saturn.cat $(DESTDIR)$(PREFIX)/share/locale/C/LC_MESSAGES/saturn.cat

	install -m 755 -d -- $(DESTDIR)$(PREFIX)/share/saturn
	install -c -m 755 dist/pack $(DESTDIR)$(PREFIX)/share/saturn/pack
	install -c -m 644 dist/saturn.cat $(DESTDIR)$(PREFIX)/share/saturn.cat #FIXME
	install -c -m 644 dist/hplogo.png $(DESTDIR)$(PREFIX)/share/saturn/hplogo.png
	cp -R dist/ROMs/ $(DESTDIR)$(PREFIX)/share/saturn/

	install -m 755 -d -- $(DESTDIR)/etc/X11/app-defaults
	install -c -m 644 dist/Saturn.ad $(DESTDIR)/etc/X11/app-defaults/Saturn

	install -m 755 -d -- $(DESTDIR)$(DOCDIR)
	cp -R COPYING LICENSE README* docs* manual/ $(DESTDIR)$(DOCDIR)

	install -m 755 -d -- $(DESTDIR)$(PREFIX)/share/applications
	sed "s|@PREFIX@|$(PREFIX)|g" dist/saturn48gx.desktop > $(DESTDIR)$(PREFIX)/share/applications/saturn48gx.desktop
	sed "s|@PREFIX@|$(PREFIX)|g" dist/saturn49g.desktop > $(DESTDIR)$(PREFIX)/share/applications/saturn49g.desktop
	sed "s|@PREFIX@|$(PREFIX)|g" dist/saturn40g.desktop > $(DESTDIR)$(PREFIX)/share/applications/saturn40g.desktop
