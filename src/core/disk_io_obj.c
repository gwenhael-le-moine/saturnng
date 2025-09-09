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

/* +-+ */

/* .+

.identifier   : $Id: disk_io_obj.c,v 4.1 2000/12/11 09:54:19 cibrario Rel $
.context      : SATURN, Saturn CPU / HPxx emulator
.title        : $RCSfile: disk_io_obj.c,v $
.kind         : C source
.author       : Ivan Cibrario B.
.site         : IRITI-CNR
.creation     :	10-Nov-2000
.keywords     : *
.description  :
  This module implements the disk I/O functions used by the emulator to
  save and restore an object to/from disk files.

.include      : config.h, machdep.h, cpu.h, disk_io.h

.notes        :
  $Log: disk_io_obj.c,v $
  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.14  2000/11/13 11:24:20  cibrario
  *** empty log message ***


.- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "../libChf/src/Chf.h"

#include "config.h"
#include "cpu.h"
#include "modules.h"
#include "disk_io.h"
#include "debug.h"

/* .+

.title        : ReadObjectFromFile
.kind         : C function
.creation     : 10-Nov-2000
.description  :
  This function reads an object from file 'name' and stores it into
  the calculator's memory from nibble address 'start' inclusive to nibble
  address 'end' exclusive.

  The presence of header 'hdr' in the disk file is checked, then the
  header is stripped before starting the transfer.  In 'hdr', '?'
  is a wildcard character that matches any input character.

  Objects with an odd number of nibbles are padded to an even
  size adding a 0 nibble to their end; the trailing zero *is* stored
  in calculator's memory.

  When the object size exceeds the available space this function
  transfers its head anyway, thereby corrupting the calculator's
  memory between 'start' and 'end', except the first N_SAVE_AREA nibbles.

  This function returns to the caller a status code.

.call         :
                st = ReadObjectFromFile(name, hdr, start, end);

.input        :
                const char *name, input file name
                const char *hdr, file header
                Address start, start address (inclusive)
                Address end, end address (exclusive)
.output       :
                int st, status code
.status_codes :
                DISK_IO_I_CALLED	(signalled)
                DISK_IO_E_OPEN
                DISK_IO_E_GETC
                DISK_IO_E_BAD_HDR
                DISK_IO_E_SIZE
.notes        :
  3.14, 10-Nov-2000, creation

.- */
int ReadObjectFromFile( const char* name, const char* hdr, Address start, Address end )
{
    size_t hdr_len = strlen( hdr );
    FILE* f;
    int i;
    int by;
    Address cur;

#define N_SAVE_AREA 10
    Nibble save_area[ N_SAVE_AREA ];

    int st = DISK_IO_S_OK;

    debug1( DISK_IO_CHF_MODULE_ID, DEBUG_C_TRACE, DISK_IO_I_CALLED, "ReadObjectFromFile" );

    /* Save first nibbles of target space into save_area */
    for ( cur = start, i = 0; cur < end && i < N_SAVE_AREA; cur++, i++ )
        save_area[ i ] = ReadNibble( cur );

    if ( ( f = fopen( name, "rb" ) ) == ( FILE* )NULL ) {
        ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
        ChfGenerate( DISK_IO_CHF_MODULE_ID, __FILE__, __LINE__, st = DISK_IO_E_OPEN, CHF_ERROR, name );
    } else {
        /* Check and skip header */
        for ( i = 0; i < ( int )hdr_len; i++ ) {
            by = getc( f );

            if ( by == EOF ) {
                ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
                ChfGenerate( DISK_IO_CHF_MODULE_ID, __FILE__, __LINE__, st = DISK_IO_E_GETC, CHF_ERROR, name );
                break;
            } else if ( hdr[ i ] != '?' && by != hdr[ i ] ) {
                ChfGenerate( DISK_IO_CHF_MODULE_ID, __FILE__, __LINE__, st = DISK_IO_E_BAD_HDR, CHF_ERROR, name );
                break;
            }
        }

        if ( st == DISK_IO_S_OK ) {
            cur = start;

            /* Header check/skip OK; transfer */
            while ( ( by = getc( f ) ) != EOF ) {
                /* Next byte available in by; check available space */
                if ( cur >= end - 1 ) {
                    ChfGenerate( DISK_IO_CHF_MODULE_ID, __FILE__, __LINE__, st = DISK_IO_E_SIZE, CHF_ERROR, name );
                    break;
                }

                /* Store it */
                WriteNibble( cur++, ( Nibble )( by & 0x0F ) );
                WriteNibble( cur++, ( Nibble )( ( by & 0xF0 ) >> 4 ) );
            }

            /* Check why getc() failed */
            if ( ferror( f ) && !feof( f ) ) {
                ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
                ChfGenerate( DISK_IO_CHF_MODULE_ID, __FILE__, __LINE__, st = DISK_IO_E_GETC, CHF_ERROR, name );
            }

            /* Recover from save_area if transfer failed */
            if ( st )
                for ( cur = start, i = 0; cur < end && i < N_SAVE_AREA; cur++, i++ )
                    WriteNibble( cur, save_area[ i ] );
        }

        ( void )fclose( f );
    }

    return st;
}

/* .+

.title        : WriteObjectToFile
.kind         : C function
.creation     : 10-Nov-2000
.description  :
  This function writes an object located in calculator's memory,
  from nibble address 'start' inclusive to nibble address 'end' exclusive,
  into the file 'name'.  The header 'hdr' is prepended to the actual
  object and objects with an odd number of nibbles are padded to an even
  size adding a 0 nibble to their end.  In 'hdr', the wildcard character
  '?' is replaced by 'S' when the header is written on disk.

  This function returns to the caller a status code.

.call         :
                st = WriteObjectToFile(start, end, hdr, name);
.input        :
                Address start, start address (inclusive)
                Address end, end address (exclusive)
                const char *hdr, file header
                const char *name, output file name
.output       :
                int st, status code
.status_codes :
                DISK_IO_I_CALLED	(signalled)
                DISK_IO_E_OPEN
                DISK_IO_E_PUTC
                DISK_IO_E_CLOSE
.notes        :
  3.14, 10-Nov-2000, creation

.- */
int WriteObjectToFile( Address start, Address end, const char* hdr, const char* name )
{
    size_t hdr_len = strlen( hdr );
    FILE* f;
    int i;
    int by;
    Address cur;

    int st = DISK_IO_S_OK;

    debug1( DISK_IO_CHF_MODULE_ID, DEBUG_C_TRACE, DISK_IO_I_CALLED, "WriteObjectFromFile" );

    if ( ( f = fopen( name, "wb" ) ) == ( FILE* )NULL ) {
        ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
        ChfGenerate( DISK_IO_CHF_MODULE_ID, __FILE__, __LINE__, st = DISK_IO_E_OPEN, CHF_ERROR, name );
    } else {
        /* Write header; replace wildcard character '?' with 'S' */
        for ( i = 0; i < ( int )hdr_len; i++ ) {
            if ( putc( hdr[ i ] == '?' ? 'S' : hdr[ i ], f ) == EOF ) {
                ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
                ChfGenerate( DISK_IO_CHF_MODULE_ID, __FILE__, __LINE__, st = DISK_IO_E_PUTC, CHF_ERROR, name );
                break;
            }
        }

        if ( st == DISK_IO_S_OK ) {
            cur = start;

            while ( cur < end - 1 ) {
                /* Make a byte with two nibbles */
                by = ( int )ReadNibble( cur++ );
                by |= ( int )ReadNibble( cur++ ) << 4;

                if ( putc( by, f ) == EOF ) {
                    ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
                    ChfGenerate( DISK_IO_CHF_MODULE_ID, __FILE__, __LINE__, st = DISK_IO_E_PUTC, CHF_ERROR, name );
                    break;
                }
            }

            /* Write the last odd nibble, if necessary */
            if ( st == DISK_IO_S_OK && cur == end - 1 ) {
                by = ( int )ReadNibble( cur++ );

                if ( putc( by, f ) == EOF ) {
                    ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
                    ChfGenerate( DISK_IO_CHF_MODULE_ID, __FILE__, __LINE__, st = DISK_IO_E_PUTC, CHF_ERROR, name );
                }
            }
        }

        /* Close the output file anyway */
        if ( fclose( f ) == EOF ) {
            ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
            ChfGenerate( DISK_IO_CHF_MODULE_ID, __FILE__, __LINE__, st = DISK_IO_E_CLOSE, CHF_ERROR, name );
        }
    }

    return st;
}
