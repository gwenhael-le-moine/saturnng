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





.author       : Ivan Cibrario B.

.creation     :	2-Oct-2000

.description  :
  This file packs a ROM image of emu48/49 (first argument) into a format
  suitable for saturn (second argument).

  This utility is totally unsupported and will likely be removed in
  the near future.

.include      : *

.notes        :
  $Log: pack.c,v $
  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.10  2000/10/24 16:14:52  cibrario
  Added/Replaced GPL header

  Revision 3.6  2000/10/02 13:55:06  cibrario
  *** empty log message ***


.- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "../libChf/src/Chf.h"

#include "cpu.h"
#include "disk_io.h"

/* Condition codes used by this utility */
#define UTIL_CHF_MODULE_ID 17

#define UTIL_I_PACK_USAGE 1
#define UTIL_F_PACK_CMD_LINE 2
#define UTIL_F_PACK_STAT 3
#define UTIL_F_PACK_SRC_SIZE 4
#define UTIL_F_PACK_MALLOC 5
#define UTIL_F_PACK_OPEN 6
#define UTIL_F_PACK_READ 7
#define UTIL_F_PACK_WRITE_NIBBLES 8

/* Maximum size of source ROM (bytes) handled by this utility; set to
   a reasonable value
*/
#define MAX_SRC_SIZE ( 4 * 1024 * 1024 )

/*---------------------------------------------------------------------------
   Chf parameters - Do not change.
  ---------------------------------------------------------------------------*/

#define CONDITION_STACK_SIZE 16
#define HANDLER_STACK_SIZE 8

/* Conditional prefix and mandatory suffix to make a message catalog
   name from cat_base_name.
*/
/* Message catalog */
static const char cat_base_name[] = "saturn";
ChfTable message_table[] = {
    {UTIL_CHF_MODULE_ID, UTIL_I_PACK_USAGE,         "Usage:\n	pack <emu48_source_rom> <saturn_dest_rom>"},
    {UTIL_CHF_MODULE_ID, UTIL_F_PACK_CMD_LINE,      "Command line syntax error"                         },
    {UTIL_CHF_MODULE_ID, UTIL_F_PACK_STAT,          "stat(%s) failed"                                   },
    {UTIL_CHF_MODULE_ID, UTIL_F_PACK_SRC_SIZE,      "Invalid source file size: %d"                      },
    {UTIL_CHF_MODULE_ID, UTIL_F_PACK_MALLOC,        "malloc(%d) failed"                                 },
    {UTIL_CHF_MODULE_ID, UTIL_F_PACK_OPEN,          "open(%s) failed"                                   },
    {UTIL_CHF_MODULE_ID, UTIL_F_PACK_READ,          "read(%s) failed"                                   },
    {UTIL_CHF_MODULE_ID, UTIL_F_PACK_WRITE_NIBBLES, "WriteNibblesToFile() failed"                       },
};

size_t message_table_size = sizeof( message_table ) / sizeof( message_table[ 0 ] );

/*---------------------------------------------------------------------------
        Public functions
  ---------------------------------------------------------------------------*/

/* .+



.creation     : 2-Oct-2000
.description  :
  Main program.

.notes        :
  3.6, 2-Oct-2000, creation

.- */
int main( int argc, char* argv[] )
{
    struct stat statb; /* stat() buffer on source file */
    char* b;           /* Source buffer */
    Nibble* nb;        /* Nibble buffer */
    int d;             /* Source file descriptor */
    int i;

    /* Chf initialization with msgcat subsystem;
       notice that on some systems (e.g. Digital UNIX) catopen() can succeed
       even if it was not able to open the right message catalog; better
       try it now.
    */
    if ( ChfStaticInit( argv[ 0 ],            /* Application's name */
                        CHF_DEFAULT,          /* Options */
                        message_table,        /* message catalog */
                        message_table_size,   /* message catalog size */
                        CONDITION_STACK_SIZE, /* Size of the condition stack */
                        HANDLER_STACK_SIZE,   /* Size of the handler stack */
                        EXIT_FAILURE          /* Abnormal exit code */
                        ) != CHF_S_OK ) {
        fprintf( stderr, "Chf initialization failed\n" );
        exit( EXIT_FAILURE );
    }

    /* Now, do some useful work; pack argv[1] into argv[2] */
    if ( argc != 3 ) {
        ChfGenerate( UTIL_CHF_MODULE_ID, __FILE__, __LINE__, UTIL_I_PACK_USAGE, CHF_INFO );
        ChfGenerate( UTIL_CHF_MODULE_ID, __FILE__, __LINE__, UTIL_F_PACK_CMD_LINE, CHF_FATAL );
        ChfSignal( UTIL_CHF_MODULE_ID );
    }

    /* Get the size of the source file */
    if ( stat( argv[ 1 ], &statb ) ) {
        ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
        ChfGenerate( UTIL_CHF_MODULE_ID, __FILE__, __LINE__, UTIL_F_PACK_STAT, CHF_FATAL, argv[ 1 ] );
        ChfSignal( UTIL_CHF_MODULE_ID );
    }

    /* Check that actual size is reasonable */
    if ( statb.st_size > MAX_SRC_SIZE ) {
        ChfGenerate( UTIL_CHF_MODULE_ID, __FILE__, __LINE__, UTIL_F_PACK_SRC_SIZE, CHF_FATAL, statb.st_size );
        ChfSignal( UTIL_CHF_MODULE_ID );
    }

    /* Allocate source buffer */
    if ( ( b = ( char* )malloc( statb.st_size ) ) == ( char* )NULL ||
         ( nb = ( Nibble* )malloc( sizeof( Nibble ) * statb.st_size ) ) == ( Nibble* )NULL ) {
        ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
        ChfGenerate( UTIL_CHF_MODULE_ID, __FILE__, __LINE__, UTIL_F_PACK_MALLOC, CHF_FATAL, statb.st_size );
        ChfSignal( UTIL_CHF_MODULE_ID );

        return EXIT_FAILURE;
    }

    /* open/read/close */
    if ( ( d = open( argv[ 1 ], O_RDONLY ) ) == -1 ) {
        ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
        ChfGenerate( UTIL_CHF_MODULE_ID, __FILE__, __LINE__, UTIL_F_PACK_OPEN, CHF_FATAL, argv[ 1 ] );
        ChfSignal( UTIL_CHF_MODULE_ID );
    }

    if ( read( d, b, statb.st_size ) != statb.st_size ) {
        ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );

        ( void )close( d );

        ChfGenerate( UTIL_CHF_MODULE_ID, __FILE__, __LINE__, UTIL_F_PACK_READ, CHF_FATAL, argv[ 1 ] );
        ChfSignal( UTIL_CHF_MODULE_ID );
    }

    ( void )close( d );

    /* Convert char -> Nibble */
    for ( i = 0; i < statb.st_size; i++ )
        nb[ i ] = ( Nibble )b[ i ];

    /* Source buffer no longer needed */
    free( b );

    /* Write */
    if ( WriteNibblesToFile( nb, statb.st_size, argv[ 2 ] ) ) {
        ChfGenerate( UTIL_CHF_MODULE_ID, __FILE__, __LINE__, UTIL_F_PACK_WRITE_NIBBLES, CHF_FATAL );
        ChfSignal( UTIL_CHF_MODULE_ID );
    }

    return EXIT_SUCCESS;
}
