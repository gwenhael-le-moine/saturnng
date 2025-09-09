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

.identifier   : $Id: disk_io.c,v 4.1 2000/12/11 09:54:19 cibrario Rel $
.context      : SATURN, Saturn CPU / HP48 emulator
.title        : $RCSfile: disk_io.c,v $
.kind         : C source
.author       : Ivan Cibrario B.
.site         : CSTV-CNR
.creation     :	23-Jan-1998
.keywords     : *
.description  :
  This module implements the disk I/O functions used by the emulator to
  save and restore the machine status to/from disk files.

.include      : config.h, machdep.h, cpu.h, disk_io.h

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

#include "../libChf/src/Chf.h"

#include "config.h"
#include "cpu.h"
#include "disk_io.h"
#include "debug.h"

/* .+

.title        : ReadNibblesFromFile
.kind         : C function
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
    FILE* f;
    int i;
    int by;
    int st = DISK_IO_S_OK;

    debug1( DISK_IO_CHF_MODULE_ID, DEBUG_C_TRACE, DISK_IO_I_CALLED, "ReadNibblesFromFile" );

    if ( ( f = fopen( name, "rb" ) ) == ( FILE* )NULL ) {
        ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
        ChfGenerate( DISK_IO_CHF_MODULE_ID, __FILE__, __LINE__, st = DISK_IO_E_OPEN, CHF_ERROR, name );
    } else {
        for ( i = 0; i < size; ) {
            by = getc( f );

            if ( by == -1 ) {
                ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
                ChfGenerate( DISK_IO_CHF_MODULE_ID, __FILE__, __LINE__, st = DISK_IO_E_GETC, CHF_ERROR, name );
                break;
            }

            dest[ i++ ] = ( Nibble )( by & 0x0F );
            dest[ i++ ] = ( Nibble )( ( by & 0xF0 ) >> 4 );
        }

        ( void )fclose( f );
    }

    return st;
}

/* .+

.title        : WriteNibblesToFile
.kind         : C function
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
    FILE* f;
    int i;
    int by;
    int st = DISK_IO_S_OK;

    debug1( DISK_IO_CHF_MODULE_ID, DEBUG_C_TRACE, DISK_IO_I_CALLED, "WriteNibblesToFile" );

    if ( ( f = fopen( name, "wb" ) ) == ( FILE* )NULL ) {
        ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
        ChfGenerate( DISK_IO_CHF_MODULE_ID, __FILE__, __LINE__, st = DISK_IO_E_OPEN, CHF_ERROR, name );
    } else {
        for ( i = 0; i < size; ) {
            by = ( int )src[ i++ ];
            by |= ( int )src[ i++ ] << 4;

            if ( putc( by, f ) == EOF ) {
                ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
                ChfGenerate( DISK_IO_CHF_MODULE_ID, __FILE__, __LINE__, st = DISK_IO_E_PUTC, CHF_ERROR, name );
                break;
            }
        }

        if ( fclose( f ) == EOF ) {
            ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
            ChfGenerate( DISK_IO_CHF_MODULE_ID, __FILE__, __LINE__, st = DISK_IO_E_CLOSE, CHF_ERROR, name );
        }
    }

    return st;
}

/* .+

.title        : ReadStructFromFile
.kind         : C function
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
    FILE* f;
    int st = DISK_IO_S_OK;

    debug1( DISK_IO_CHF_MODULE_ID, DEBUG_C_TRACE, DISK_IO_I_CALLED, "ReadStructFromFile" );

    if ( ( f = fopen( name, "rb" ) ) == ( FILE* )NULL ) {
        ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
        ChfGenerate( DISK_IO_CHF_MODULE_ID, __FILE__, __LINE__, st = DISK_IO_E_OPEN, CHF_ERROR, name );
    } else {
        if ( fread( s, s_size, ( size_t )1, f ) != 1 ) {
            ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
            ChfGenerate( DISK_IO_CHF_MODULE_ID, __FILE__, __LINE__, st = DISK_IO_E_READ, CHF_ERROR, name );
        }

        ( void )fclose( f );
    }

    return st;
}

/* .+

.title        : WriteStructToFile
.kind         : C function
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
    FILE* f;
    int st = DISK_IO_S_OK;

    debug1( DISK_IO_CHF_MODULE_ID, DEBUG_C_TRACE, DISK_IO_I_CALLED, "WriteStructToFile" );

    if ( ( f = fopen( name, "wb" ) ) == ( FILE* )NULL ) {
        ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
        ChfGenerate( DISK_IO_CHF_MODULE_ID, __FILE__, __LINE__, st = DISK_IO_E_OPEN, CHF_ERROR, name );
    } else {
        if ( fwrite( s, s_size, ( size_t )1, f ) != 1 ) {
            ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
            ChfGenerate( DISK_IO_CHF_MODULE_ID, __FILE__, __LINE__, st = DISK_IO_E_WRITE, CHF_ERROR, name );
        }

        if ( fclose( f ) == EOF ) {
            ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
            ChfGenerate( DISK_IO_CHF_MODULE_ID, __FILE__, __LINE__, st = DISK_IO_E_CLOSE, CHF_ERROR, name );
        }
    }

    return st;
}
