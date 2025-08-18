#
# Source files
#
SRC =	src/chf_init.c \
	src/chf_gen.c \
	src/chf_sig.c \
	src/chf_abrt.c \
	src/chf_hdlr.c \
	src/chf_msgc.c \
	src/chf_st.c

#
# Target object files
#
OBJS =	$(SRC:.c=.o)

HEADERS = src/ChfPriv.h \
	src/Chf.h \

#
# Other definitions
#
OPTIM ?= 2

CFLAGS ?= -O$(OPTIM) -ansi -pedantic -D_POSIX_C_SOURCE=199506L -I.
# ifeq ($(MT), yes)
# 	override CFLAGS := -D_REENTRANT $(CFLAGS)
# endif

LFLAGS ?= $(CFLAGS)
LDFLAGS = -L.

#
# Library names
#
TARGET  = libChf.a

all: $(TARGET)

doc:
	make -C docs

clean:
	rm -f $(OBJS)
	make -C docs clean

mrproper: clean
	rm -f $(TARGET)
	make -C docs mrproper

$(TARGET): $(OBJS) $(HEADERS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

# Formatting
pretty-code:
	clang-format -i src/*.c src/*.h
