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

.creation     :	23-Jan-1998

.description  :
This module implements the disk I/O functions used by the emulator to
save and restore the machine status to/from disk files.

.notes        :
$Log: disk_io.c,v $
Revision 4.1  2000/12/11 09:54:19  cibrario
Public release.

  Revision 3.10  2000/10/24 16:14:36  cibrario
  Added/Replaced GPL header

  Revision 1.1  1998/02/17 11:54:38  cibrario
  Initial revision
  .- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../libChf/src/Chf.h"

#include "disk_io.h"
#include "chf_wrapper.h"
#include "modules.h"

/* .+

.creation     : 11-Feb-1998
.description  :
This function reads 'size' nibbles from the disk file named 'name',
and stores them into main memory starting from 'dest'. It returns to the
caller a status code.

.call         :
st = ReadNibbledFromFile(name, size, dest);
.input        :
const char *name, file name
int size, size of the file (nibbles, NOT bytes)
.output       :
Nibble *dest, pointer to the destination memory area
int st, status code
.status_codes :
DISK_IO_I_CALLED	(signalled)
DISK_IO_E_OPEN
DISK_IO_E_GETC
.notes        :
1.1, 11-Feb-1998, creation

.- */
int ReadNibblesFromFile( const char* name, int size, Nibble* dest )
{
    FILE* f = fopen( name, "rb" );

    if ( f == ( FILE* )NULL ) {
        SIGNAL_ERRNO
        ERROR( DISK_IO_CHF_MODULE_ID, DISK_IO_E_OPEN, name )

        return DISK_IO_E_OPEN;
    }

    int by;
    int st = DISK_IO_S_OK;
    for ( int i = 0; i < size; ) {
        by = getc( f );

        if ( by == -1 ) {
            st = DISK_IO_E_GETC;

            SIGNAL_ERRNO
            ERROR( DISK_IO_CHF_MODULE_ID, st, name )

            break;
        }

        dest[ i++ ] = ( Nibble )( by & 0x0F );
        dest[ i++ ] = ( Nibble )( ( by & 0xF0 ) >> 4 );
    }

    ( void )fclose( f );

    return st;
}

/* .+

.creation     : 11-Feb-1998
.description  :
This function writes 'size' nibbles taken from 'src' into the file 'name'.
It returns to the caller a status code

.call         :
st = WriteNibblesToFile(src, size, name);
.input        :
const Nibble *src, pointer to data to be written
int size, # of nibble to write
const char *name, file name
.output       :
int st, status code
.status_codes :
DISK_IO_I_CALLED	(signalled)
DISK_IO_E_OPEN
DISK_IO_E_PUTC
DISK_IO_E_CLOSE
.notes        :
1.1, 11-Feb-1998, creation

.- */
int WriteNibblesToFile( const Nibble* src, int size, const char* name )
{
    FILE* f = fopen( name, "wb" );

    if ( f == ( FILE* )NULL ) {
        SIGNAL_ERRNO
        ERROR( DISK_IO_CHF_MODULE_ID, DISK_IO_E_OPEN, name )

        return DISK_IO_E_OPEN;
    }

    int by;
    int ret;
    int st = DISK_IO_S_OK;
    for ( int i = 0; i < size; ) {
        by = ( int )src[ i++ ];
        by |= ( int )src[ i++ ] << 4;

        ret = putc( by, f );
        if ( ret == EOF ) {
            st = DISK_IO_E_PUTC;

            SIGNAL_ERRNO
            ERROR( DISK_IO_CHF_MODULE_ID, st, name )
            break;
        }
    }

    if ( fclose( f ) == EOF ) {
        st = DISK_IO_E_CLOSE;

        SIGNAL_ERRNO
        ERROR( DISK_IO_CHF_MODULE_ID, st, name )
    }

    return st;
}

/* .+

.creation     : 11-Feb-1998
.description  :
This function reads the contents of the structure 's', with size 's_size',
from the disk file 'name' and returns a status code to the caller.

.call         :
st = ReadStructFromFile(name, s_size, s);
.input        :
const char *name, file name
size_t s_size, structure size
.output       :
void *s, pointer to the structure
.status_codes :
DISK_IO_I_CALLED	(signalled)
DISK_IO_E_OPEN
DISK_IO_E_READ
.notes        :
1.1, 11-Feb-1998, creation

.- */
int ReadStructFromFile( const char* name, size_t s_size, void* s )
{
    FILE* f = fopen( name, "rb" );
    int st = DISK_IO_S_OK;

    if ( f == ( FILE* )NULL ) {
        SIGNAL_ERRNO
        ERROR( DISK_IO_CHF_MODULE_ID, DISK_IO_E_OPEN, name )

        return DISK_IO_E_OPEN;
    }

    int ret = fread( s, s_size, ( size_t )1, f );
    if ( ret != 1 ) {
        st = DISK_IO_E_READ;

        SIGNAL_ERRNO
        ERROR( DISK_IO_CHF_MODULE_ID, st, name )
    }

    ( void )fclose( f );

    return st;
}

/* .+

.creation     : 11-Feb-1998
.description  :
This function writes the structure 's', with size 's_size', to the file
'name' and returns to the caller a status code.

.call         :
st =WriteStructToFile(s, s_size, name);
.input        :
const void *s, pointer to the structure to be written
size_t s_size, structure size
const char *name, output file name
.output       :
int st, status code
.status_codes :
DISK_IO_I_CALLED	(signalled)
DISK_IO_E_OPEN
DISK_IO_E_WRITE
DISK_IO_E_CLOSE
.notes        :
1.1, 11-Feb-1998, creation

.- */
int WriteStructToFile( const void* s, size_t s_size, const char* name )
{
    FILE* f = fopen( name, "wb" );
    int st = DISK_IO_S_OK;

    if ( f == ( FILE* )NULL ) {
        SIGNAL_ERRNO
        ERROR( DISK_IO_CHF_MODULE_ID, DISK_IO_E_OPEN, name )

        return DISK_IO_E_OPEN;
    }

    int ret = fwrite( s, s_size, ( size_t )1, f );
    if ( ret != 1 ) {
        st = DISK_IO_E_WRITE;

        SIGNAL_ERRNO
        ERROR( DISK_IO_CHF_MODULE_ID, st, name )
    }

    ret = fclose( f );
    if ( ret == EOF ) {
        st = DISK_IO_E_CLOSE;

        SIGNAL_ERRNO
        ERROR( DISK_IO_CHF_MODULE_ID, st, name )
    }

    return st;
}

/* .+

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
    int i;
    int by;
    Address cur;

#define N_SAVE_AREA 10
    Nibble save_area[ N_SAVE_AREA ];

    int st = DISK_IO_S_OK;

    /* Save first nibbles of target space into save_area */
    for ( cur = start, i = 0; cur < end && i < N_SAVE_AREA; cur++, i++ )
        save_area[ i ] = ReadNibble( cur );

    FILE* f = fopen( name, "rb" );
    if ( f == ( FILE* )NULL ) {
        SIGNAL_ERRNO
        ERROR( DISK_IO_CHF_MODULE_ID, DISK_IO_E_OPEN, name )

        return DISK_IO_E_OPEN;
    }

    /* Check and skip header */
    for ( i = 0; i < ( int )hdr_len; i++ ) {
        by = getc( f );

        if ( by == EOF ) {
            st = DISK_IO_E_GETC;

            SIGNAL_ERRNO
            ERROR( DISK_IO_CHF_MODULE_ID, st, name )
            break;
        } else if ( hdr[ i ] != '?' && by != hdr[ i ] ) {
            st = DISK_IO_E_BAD_HDR;

            ERROR( DISK_IO_CHF_MODULE_ID, st, name )
            break;
        }
    }

    if ( st == DISK_IO_S_OK ) {
        cur = start;

        /* Header check/skip OK; transfer */
        while ( ( by = getc( f ) ) != EOF ) {
            /* Next byte available in by; check available space */
            if ( cur >= end - 1 ) {
                st = DISK_IO_E_SIZE;

                ERROR( DISK_IO_CHF_MODULE_ID, st, name )
                break;
            }

            /* Store it */
            WriteNibble( cur++, ( Nibble )( by & 0x0F ) );
            WriteNibble( cur++, ( Nibble )( ( by & 0xF0 ) >> 4 ) );
        }

        /* Check why getc() failed */
        if ( ferror( f ) && !feof( f ) ) {
            st = DISK_IO_E_GETC;

            SIGNAL_ERRNO
            ERROR( DISK_IO_CHF_MODULE_ID, st, name )
        }

        /* Recover from save_area if transfer failed */
        if ( st )
            for ( cur = start, i = 0; cur < end && i < N_SAVE_AREA; cur++, i++ )
                WriteNibble( cur, save_area[ i ] );
    }

    ( void )fclose( f );

    return st;
}

/* .+

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
    int ret;
    int by;

    int st = DISK_IO_S_OK;

    FILE* f = fopen( name, "wb" );
    if ( f == ( FILE* )NULL ) {
        SIGNAL_ERRNO
        ERROR( DISK_IO_CHF_MODULE_ID, DISK_IO_E_OPEN, name )

        return DISK_IO_E_OPEN;
    }

    /* Write header; replace wildcard character '?' with 'S' */
    for ( int i = 0; i < ( int )hdr_len; i++ ) {
        ret = putc( hdr[ i ] == '?' ? 'S' : hdr[ i ], f );
        if ( ret == EOF ) {
            st = DISK_IO_E_PUTC;

            SIGNAL_ERRNO
            ERROR( DISK_IO_CHF_MODULE_ID, st, name )
            break;
        }
    }

    if ( st == DISK_IO_S_OK ) {
        Address cur = start;

        while ( cur < end - 1 ) {
            /* Make a byte with two nibbles */
            by = ( int )ReadNibble( cur++ );
            by |= ( int )ReadNibble( cur++ ) << 4;

            ret = putc( by, f );
            if ( ret == EOF ) {
                st = DISK_IO_E_PUTC;

                SIGNAL_ERRNO
                ERROR( DISK_IO_CHF_MODULE_ID, st, name )
                break;
            }
        }

        /* Write the last odd nibble, if necessary */
        if ( st == DISK_IO_S_OK && cur == end - 1 ) {
            by = ( int )ReadNibble( cur++ );

            ret = putc( by, f );
            if ( ret == EOF ) {
                st = DISK_IO_E_PUTC;

                SIGNAL_ERRNO
                ERROR( DISK_IO_CHF_MODULE_ID, st, name )
            }
        }
    }

    /* Close the output file anyway */
    if ( fclose( f ) == EOF ) {
        st = DISK_IO_E_CLOSE;

        SIGNAL_ERRNO
        ERROR( DISK_IO_CHF_MODULE_ID, st, name )
    }

    return st;
}
