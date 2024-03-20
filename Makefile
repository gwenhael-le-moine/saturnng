# Simple Makefile to build saturn_bertolotti

MAKEFLAGS +=-j$(NUM_CORES) -l$(NUM_CORES)

CC ?= gcc

OPTIM ?= 2

CFLAGS = -g -O$(OPTIM) -I./src/ -D_GNU_SOURCE=1 -I./Chf -L./Chf/st_build -lutil
LIBS = -lm -lChf -lXm

X11CFLAGS = $(shell pkg-config --cflags x11 xext) -D_GNU_SOURCE=1
X11LIBS = $(shell pkg-config --libs x11 xext xt)

CFLAGS += $(X11CFLAGS)
LIBS += $(X11LIBS)

FULL_WARNINGS = no
ifeq ($(FULL_WARNINGS), yes)
	CFLAGS += -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-function -Wconversion -Wdouble-promotion -Wno-sign-conversion -fsanitize=undefined -fsanitize-trap
endif

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
	src/modules.o \
	src/monitor.o \
	src/romram.o \
	src/romram49.o \
	src/saturn.o \
	src/serial.o \
	src/x11.o \
	src/x_func.o

MSFS=	Msfs/debug.msf \
	Msfs/cpu.msf \
	Msfs/modules.msf \
	Msfs/disk_io.msf \
	Msfs/x11.msf \
	Msfs/serial.msf Msfs/flash49.msf Msfs/x_func.msf\
	Msfs/saturn.msf Msfs/util.msf \
	Chf/chf.msf

.PHONY: all clean clean-all pretty-code install mrproper

all: Chf/st_build/libChf.a dist/saturn dist/pack dist/saturn.cat docs

# Binaries
Chf/st_build/libChf.a:
	make -C Chf

dist/saturn: $(DOTOS) Chf/st_build/libChf.a
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS)

dist/pack: src/pack.o src/disk_io.o src/debug.o Chf/st_build/libChf.a
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS)

dist/saturn.cat: $(MSFS)
	for msf in $? ;	\
	  do gencat $@ $$msf ; \
	done

docs:
	make -C docs

# Cleaning
clean:
	rm -f src/*.o
	make -C Chf clean
	make -C docs clean

mrproper: clean
	rm -f dist/pack dist/saturn dist/saturn.cat

clean-all: mrproper

# Formatting
pretty-code:
	clang-format -i src/*.c src/*.h

# # Installing
# ROMs/sxrom-a:
#	curl "https://www.hpcalc.org/hp48/pc/emulators/sxrom-a.zip" --output - | funzip > "ROMs/sxrom-a"
# ROMs/sxrom-b:
#	curl "https://www.hpcalc.org/hp48/pc/emulators/sxrom-b.zip" --output - | funzip > "ROMs/sxrom-b"
# ROMs/sxrom-c:
#	curl "https://www.hpcalc.org/hp48/pc/emulators/sxrom-c.zip" --output - | funzip > "ROMs/sxrom-c"
# ROMs/sxrom-d:
#	curl "https://www.hpcalc.org/hp48/pc/emulators/sxrom-d.zip" --output - | funzip > "ROMs/sxrom-d"
# ROMs/sxrom-e:
#	curl "https://www.hpcalc.org/hp48/pc/emulators/sxrom-e.zip" --output - | funzip > "ROMs/sxrom-e"
# ROMs/sxrom-j:
#	curl "https://www.hpcalc.org/hp48/pc/emulators/sxrom-j.zip" --output - | funzip > "ROMs/sxrom-j"
# ROMs/gxrom-l:
#	curl "https://www.hpcalc.org/hp48/pc/emulators/gxrom-l.zip" --output - | funzip > "ROMs/gxrom-l"
# ROMs/gxrom-m:
#	curl "https://www.hpcalc.org/hp48/pc/emulators/gxrom-m.zip" --output - | funzip > "ROMs/gxrom-m"
# ROMs/gxrom-p:
#	curl "https://www.hpcalc.org/hp48/pc/emulators/gxrom-p.zip" --output - | funzip > "ROMs/gxrom-p"
# ROMs/gxrom-r:
#	curl "https://www.hpcalc.org/hp48/pc/emulators/gxrom-r.zip" --output - | funzip > "ROMs/gxrom-r"

# get-roms: ROMs/sxrom-a ROMs/sxrom-b ROMs/sxrom-c ROMs/sxrom-d ROMs/sxrom-e ROMs/sxrom-j ROMs/gxrom-l ROMs/gxrom-m ROMs/gxrom-p ROMs/gxrom-r

# install: all get-roms
#	install -m 755 -d -- $(DESTDIR)$(PREFIX)/bin
#	install -c -m 755 x48ng $(DESTDIR)$(PREFIX)/bin/x48ng

#	install -m 755 -d -- $(DESTDIR)$(PREFIX)/share/x48ng
#	install -c -m 755 mkcard $(DESTDIR)$(PREFIX)/share/x48ng/mkcard
#	install -c -m 755 dump2rom $(DESTDIR)$(PREFIX)/share/x48ng/dump2rom
#	install -c -m 755 checkrom $(DESTDIR)$(PREFIX)/share/x48ng/checkrom
#	install -c -m 644 hplogo.png $(DESTDIR)$(PREFIX)/share/x48ng/hplogo.png
#	cp -R ROMs/ $(DESTDIR)$(PREFIX)/share/x48ng/
#	sed "s|@PREFIX@|$(PREFIX)|g" setup-x48ng-home.sh > $(DESTDIR)$(PREFIX)/share/x48ng/setup-x48ng-home.sh
#	chmod 755 $(DESTDIR)$(PREFIX)/share/x48ng/setup-x48ng-home.sh

#	install -m 755 -d -- $(DESTDIR)$(MANDIR)/man1
#	sed "s|@VERSION@|$(VERSION_MAJOR).$(VERSION_MINOR).$(PATCHLEVEL)|g" x48ng.man.1 > $(DESTDIR)$(MANDIR)/man1/x48ng.1
#	gzip -9  $(DESTDIR)$(MANDIR)/man1/x48ng.1

#	install -m 755 -d -- $(DESTDIR)$(DOCDIR)
#	cp -R AUTHORS LICENSE README* doc* romdump/ $(DESTDIR)$(DOCDIR)
#	x48ng --print-config > config.lua
#	install -c -m 644 config.lua $(DESTDIR)$(DOCDIR)/config.lua

#	install -m 755 -d -- $(DESTDIR)$(PREFIX)/share/applications
#	sed "s|@PREFIX@|$(PREFIX)|g" x48ng.desktop > $(DESTDIR)$(PREFIX)/share/applications/x48ng.desktop
