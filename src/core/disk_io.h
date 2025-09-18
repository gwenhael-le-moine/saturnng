#ifndef _DISK_IO_H
#  define _DISK_IO_H 1

/* -------------------------------------------------------------------------
   saturn - A poor-man's emulator of some HP calculators
   Copyright (C) 1998-2000 Ivan Cibrario Bertolotti

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the documentation of this program; if not, write to
   the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

   For more information, please contact the author, preferably by email,
   at the following address:

   Ivan Cibrario Bertolotti
   IRITI - National Research Council
   c/o IEN "Galileo Ferraris"
   Strada delle Cacce, 91
   10135 - Torino (ITALY)

   email: cibrario@iriti.cnr.it
   ------------------------------------------------------------------------- */

/* .+
.author       : Ivan Cibrario B.

.creation     :	28-Jan-1998

.description  :
  This header declares the disk I/O functions used by the emulator to
  save/restore the machine stats to/from disk files.

.notes        :
  $Log: disk_io.h,v $
  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.14  2000/11/13 10:31:16  cibrario
  Implemented fast load/save; improved keyboard interface emulation at
  high emulated CPU speed:

  - New condition codes: DISK_IO_E_BAD_HDR, DISK_IO_E_SIZE
  - New prototypes: ReadObjectFromFile(), WriteObjectToFile()

  Revision 3.10  2000/10/24 16:14:36  cibrario
  Added/Replaced GPL header

  Revision 1.1  1998/02/17 11:55:02  cibrario
  Initial revision
.- */

#  include <sys/types.h>

#  include "types.h"

/*---------------------------------------------------------------------------
        Chf condition codes
  ---------------------------------------------------------------------------*/
typedef enum {
    DISK_IO_S_OK = 0,        /* Function completed succesfully */
    DISK_IO_I_CALLED = 101,  /* Function %s called */
    DISK_IO_E_OPEN = 401,    /* Open file %s failed */
    DISK_IO_E_GETC = 402,    /* getc() from file %s failed */
    DISK_IO_E_PUTC = 403,    /* putc() to file %s failed */
    DISK_IO_E_READ = 404,    /* fread() from file %s failed */
    DISK_IO_E_WRITE = 405,   /* fwrite() to file %s failed */
    DISK_IO_E_CLOSE = 406,   /* Close file %s failed */
    DISK_IO_E_BAD_HDR = 407, /* File %s has a bad header */
    DISK_IO_E_SIZE = 408,    /* File %s too large */
} disk_io_chf_message_id_t;

/*---------------------------------------------------------------------------
        Function prototypes
  ---------------------------------------------------------------------------*/

int ReadNibblesFromFile( const char* name, int size, Nibble* dest );
int WriteNibblesToFile( const Nibble* src, int size, const char* name );
int ReadStructFromFile( const char* name, size_t s_size, void* s );
int WriteStructToFile( const void* s, size_t s_size, const char* name );

int ReadObjectFromFile( const char* name, const char* hdr, Address start, Address end );
int WriteObjectToFile( Address start, Address end, const char* hdr, const char* name );

void import_file( char* filename );
void export_file( char* filename );

#endif /*!_DISK_IO_H*/
