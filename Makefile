# Simple Makefile to build saturn_bertolotti

MAKEFLAGS +=-j$(NUM_CORES) -l$(NUM_CORES)

CC ?= gcc

OPTIM ?= 2

CFLAGS = -g -O$(OPTIM) -I./src/ -D_GNU_SOURCE=1 -I./src/libChf -L./src/libChf/st_build -lutil
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

MSFS=	src/debug.msf \
	src/cpu.msf \
	src/modules.msf \
	src/disk_io.msf \
	src/x11.msf \
	src/serial.msf \
	src/flash49.msf \
	src/x_func.msf \
	src/saturn.msf \
	src/util.msf \
	src/libChf/chf.msf

.PHONY: all clean clean-all pretty-code install mrproper get-roms

all: src/libChf/st_build/libChf.a dist/saturn dist/pack dist/saturn.cat manual

# Building
src/libChf/st_build/libChf.a:
	make -C src/libChf

dist/saturn: $(DOTOS) src/libChf/st_build/libChf.a
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS)

dist/pack: src/pack.o src/disk_io.o src/debug.o src/libChf/st_build/libChf.a
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
	make -C src/libChf clean
	make -C manual clean

mrproper: clean
	rm -f dist/pack dist/saturn dist/saturn.cat

clean-all: mrproper

# Formatting
pretty-code:
	clang-format -i src/*.c src/*.h src/libChf/*.c src/libChf/*.h

# Dependencies
get-roms:
	make -C ROMs get-roms
