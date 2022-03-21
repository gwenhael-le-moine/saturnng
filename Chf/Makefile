# ICB, 2-Oct-2000
# Minor off-line changes to support stand-alone build in saturn
#
# ICB, 16-Nov-2000
# Off-line change: added default compile mode for unsupported platforms

# .+
#
# .identifier   : $Id: Makefile,v 2.1 2000/05/29 13:53:07 cibrario Rel $
# .context      : CHF, Condition Handling Facility
# .title        : $RCSfile: Makefile,v $, Makefile
# .kind	        : Makefile
# .author       : Ivan Cibrario B.
# .site	        : CSTV-CNR
# .creation     :  3-May-1996
# .keywords     : *
# .description  :
#   Makefile for the CHF library
# .notes        :
#   $Log: Makefile,v $
#   Revision 2.1  2000/05/29 13:53:07  cibrario
#   - Deep revision to support transparent, multi-platform builds
#   - Added multithreading support
#   - New/revised targets: test, doc, clean
#
#   Revision 1.2  1996/06/11  13:03:06  cibrario
#   Added new module chf_top.c
#
#   Revision 1.1  1996/05/29  09:12:10  cibrario
#   Initial revision
#
#
# .-

#
# Platform-dependent definitions.  Edit when required, for example to
# add a new platform.
#
# There is a set of definitions per platform; the prefix or the definition
# names is the value returned by a plain 'uname' on that platform.
#
# CC: cc compiler
# TS_CFLAGS: cc flags to enable multithreading
# TS_LOADLIBES: ld flags and additional libraries to enable multithreading
# F_CFLAGS: cc feature test macros
# O_CFLAGS: cc optimization control flags and other, additional flags
#
# Supported platforms:
# OSF1		Digital UNIX v4.0
# SunOS		Solaris v2.6
# Linux		Linux 2.1
#
# In addition, if the first build fails, a second attempt is made
# using the X_CC and X_CFLAGS fallback values.

OSF1_CC            = cc
OSF1_TS_CFLAGS     = -pthread
OSF1_TS_LOADLIBES  = -pthread
OSF1_F_CFLAGS      = "-D_POSIX_C_SOURCE=199506L -D_XOPEN_SOURCE"
OSF1_O_CFLAGS      = "-O3 -std1"

SunOS_CC           = gcc
SunOS_TS_CFLAGS    = -D_REENTRANT
SunOS_TS_LOADLIBES = -lpthread
SunOS_F_CFLAGS     = -D_POSIX_C_SOURCE=199506L
SunOS_O_CFLAGS     = "-O3 -ansi -pedantic"

Linux_CC           = cc
Linux_TS_CFLAGS    = -D_REENTRANT
Linux_TS_LOADLIBES = -lpthread
Linux_F_CFLAGS     = -D_POSIX_C_SOURCE=199506L
Linux_O_CFLAGS     = "-O3 -ansi -pedantic"

#
# Destination paths for library objects, C headers, binaries, message catalog
# sources and X resource files.  Edit as required.
#
DESTLIBDIR = $(HOME)/lib
DESTHDRDIR = $(HOME)/include
DESTBINDIR = $(HOME)/bin
DESTMSFDIR = $(HOME)/msf
DESTRESDIR = $(HOME)/res

#
# The default target recursively invokes make defining UNAME to the
# string returned by uname; this is used to make appropriate
# platform-dependent definitions above.
#
all:
	@$(MAKE) UNAME=`uname` build || \
	$(MAKE) UNAME=Unknown CC='"$(X_CC)"' O_CFLAGS='"$(X_CFLAGS)"' build

#
# Vectored definitions; see above and do not edit.
#
CC                 = $($(UNAME)_CC)
TS_CFLAGS          = $($(UNAME)_TS_CFLAGS)
TS_LOADLIBES       = $($(UNAME)_TS_LOADLIBES)
F_CFLAGS           = $($(UNAME)_F_CFLAGS)
O_CFLAGS           = $($(UNAME)_O_CFLAGS)

#
# Include common definitions
#
include Makefile_def

#
# Build directories for single-threaded and multi-threaded lib versions
#
ST_BUILD_DIR = ./st_build
MT_BUILD_DIR = ./mt_build

#
# Library names
#
ST_LIB_NAME  = libChf.a
MT_LIB_NAME  = libChf_r.a

#
# Library paths
#
ST_LIB_PATH  = $(ST_BUILD_DIR)/$(ST_LIB_NAME)
MT_LIB_PATH  = $(MT_BUILD_DIR)/$(MT_LIB_NAME)

#
# build: Create and populate build directories; for each build directory,
# make dependencies and rebuild
#
build: $(ST_LIB_PATH) $(MT_LIB_PATH)

#
# test: Build and executes test programs
#
test: 
	@$(MAKE) UNAME=`uname` do_test
	@echo "All tests completed successfully"

do_test: $(ST_LIB_PATH) $(MT_LIB_PATH)
	@cd $(ST_BUILD_DIR) && \
	$(MAKE) test CC=$(CC) LIB=$(ST_LIB_NAME) \
	F_CFLAGS=$(F_CFLAGS) O_CFLAGS=$(O_CFLAGS) > test.log 2>&1
	@cd $(MT_BUILD_DIR) && \
	$(MAKE) test CC=$(CC) LIB=$(MT_LIB_NAME) \
	F_CFLAGS=$(F_CFLAGS) O_CFLAGS=$(O_CFLAGS) \
	TS_CFLAGS=$(TS_CFLAGS) TS_LOADLIBES=$(TS_LOADLIBES) >> test.log 2>&1

#
# doc: Make documentation files
#
doc:
	makeinfo chf.texi
	texi2dvi chf.texi; dvips -o chf.ps chf.dvi

#
# clean: Remove build directories
#
clean:
	rm -rf $(ST_BUILD_DIR) $(MT_BUILD_DIR)

#
# install: Move all files to their directories
# ICB, 23-Oct-2000: Do nothing
#
install: $(ST_LIB_PATH) $(MT_LIB_PATH) $(HDR) $(MSF) $(RES)

#
# Prepare the build directories for use
#
# ICB, 16-Nov-2000
# Off-line change: Force the build to fail with exit 1 if CC is undefined
#
$(ST_BUILD_DIR) $(MT_BUILD_DIR):
	@[ "$(CC)" = "" ] && \
	  { echo "Warning: using X_ defaults"; exit 1; } ; \
	mkdir $@ && \
	cd $@ && \
	ln -s ../Makefile_sub Makefile && \
	for s in $(SRC) $(TSRC) $(HDR) $(MSF) $(TMSF) $(RES) ; do \
	  ln -s ../$$s $$s ; \
	done

#
# Make the libraries; the rebuild is forced here because only the
# sub-makes have module dependencies.
#
# ICB, 16-Nov-2000
# Off-line change: Force the build to fail with exit 1 if CC is undefined
#
$(ST_LIB_PATH): $(ST_BUILD_DIR) force
	@[ "$(CC)" = "" ] && \
	  { echo "Warning: using X_ defaults"; exit 1; } ; \
	cd $(ST_BUILD_DIR) && \
	$(MAKE) depend CC=$(CC) LIB=$(ST_LIB_NAME) \
	F_CFLAGS=$(F_CFLAGS) O_CFLAGS=$(O_CFLAGS) && \
	$(MAKE) all CC=$(CC) LIB=$(ST_LIB_NAME) \
	F_CFLAGS=$(F_CFLAGS) O_CFLAGS=$(O_CFLAGS)

$(MT_LIB_PATH): $(MT_BUILD_DIR) force
	@[ "$(CC)" = "" ] && \
	  { echo "Warning: using X_ defaults"; exit 1; } ; \
	cd $(MT_BUILD_DIR) && \
	$(MAKE) depend CC=$(CC) LIB=$(MT_LIB_NAME) \
	F_CFLAGS=$(F_CFLAGS) O_CFLAGS=$(O_CFLAGS) \
	TS_CFLAGS=$(TS_CFLAGS) TS_LOADLIBES=$(TS_LOADLIBES) && \
	$(MAKE) all CC=$(CC) LIB=$(MT_LIB_NAME) \
	F_CFLAGS=$(F_CFLAGS) O_CFLAGS=$(O_CFLAGS) \
	TS_CFLAGS=$(TS_CFLAGS) TS_LOADLIBES=$(TS_LOADLIBES)

force:
