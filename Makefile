# Simple Makefile to build saturn_bertolotti

MAKEFLAGS +=-j$(NUM_CORES) -l$(NUM_CORES)

CC ?= gcc

OPTIM ?= 2

CFLAGS = -g -O$(OPTIM) -I./src/ -D_GNU_SOURCE=1 -I./libChf -L./libChf/st_build -lutil
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
	src/main.o \
	src/serial.o \
	src/x11.o \
	src/x_func.o

MSFS=	src/MSFs/debug.msf \
	src/MSFs/cpu.msf \
	src/MSFs/modules.msf \
	src/MSFs/disk_io.msf \
	src/MSFs/x11.msf \
	src/MSFs/serial.msf \
	src/MSFs/flash49.msf \
	src/MSFs/x_func.msf \
	src/MSFs/saturn.msf \
	src/MSFs/util.msf \
	libChf/chf.msf

.PHONY: all clean clean-all pretty-code install mrproper get-roms

all: libChf/st_build/libChf.a dist/saturn dist/pack dist/saturn.cat manual

# Building
libChf/st_build/libChf.a:
	make -C libChf

dist/saturn: $(DOTOS) libChf/st_build/libChf.a
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS)

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

clean-all: mrproper

# Formatting
pretty-code:
	clang-format -i src/*.c src/*.h libChf/*.c libChf/*.h

# Dependencies
get-roms:
	make -C ROMs get-roms
