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

.identifier   : $Id: pack.c,v 4.1 2000/12/11 09:54:19 cibrario Rel $
.context      : SATURN, Saturn CPU / HP48 emulator
.title	      : $RCSfile: pack.c,v $
.kind	      : C source
.author	      : Ivan Cibrario B.
.site	      : CSTV-CNR
.creation     :	2-Oct-2000
.keywords     : *
.description  :
  This file packs a ROM image of emu48/49 (first argument) into a format
  suitable for saturn (second argument).

  This utility is totally unsupported and will likely be removed in
  the near future.

.include      : *

.notes	      :
  $Log: pack.c,v $
  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.10  2000/10/24 16:14:52  cibrario
  Added/Replaced GPL header

  Revision 3.6  2000/10/02 13:55:06  cibrario
  *** empty log message ***


.- */

#ifndef lint
static char rcs_id[] = "$Id";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "config.h"
#include "machdep.h"
#include "cpu.h"
#include "disk_io.h"

#define CHF_MODULE_ID UTIL_CHF_MODULE_ID
#include <Chf.h>

/* Maximum size of source ROM (bytes) handled by this utility; set to
   a reasonable value
*/
#define MAX_SRC_SIZE ( 4 * 1024 * 1024 )

/*---------------------------------------------------------------------------
   Chf parameters - Do not change.
   The ABNORMAL_EXIT_CODE is taken from stdlib.h (EXIT_FAILURE)
  ---------------------------------------------------------------------------*/

#define CONDITION_STACK_SIZE 16
#define HANDLER_STACK_SIZE 8
#define ABNORMAL_EXIT_CODE EXIT_FAILURE

/* Conditional prefix and mandatory suffix to make a message catalog
   name from cat_base_name.
*/
static const char cat_prefix[] = "./";
static const char cat_suffix[] = ".cat";

#define CAT_PREFIX_LEN ( sizeof( cat_prefix ) + 1 )
#define CAT_SUFFIX_LEN ( sizeof( cat_suffix ) + 1 )

/* Message catalog base_name */
static const char cat_base_name[] = "saturn";

/* Condition codes used by this utility */
#define UTIL_I_PACK_USAGE 1
#define UTIL_F_PACK_CMD_LINE 2
#define UTIL_F_PACK_STAT 3
#define UTIL_F_PACK_SRC_SIZE 4
#define UTIL_F_PACK_MALLOC 5
#define UTIL_F_PACK_OPEN 6
#define UTIL_F_PACK_READ 7
#define UTIL_F_PACK_WRITE_NIBBLES 8

/*---------------------------------------------------------------------------
        Public functions
  ---------------------------------------------------------------------------*/

/* .+

.title	      : main
.kind	      : C function
.creation     : 2-Oct-2000
.description  :
  Main program.

.notes	      :
  3.6, 2-Oct-2000, creation

.- */
int main( int argc, char* argv[] )
{
    char* cat_name;    /* Message catalog name */
    struct stat statb; /* stat() buffer on source file */
    char* b;           /* Source buffer */
    Nibble* nb;        /* Nibble buffer */
    int d;             /* Source file descriptor */
    int i;
    int st;

    if ( ( cat_name = malloc( sizeof( cat_base_name ) + CAT_PREFIX_LEN + CAT_SUFFIX_LEN + 1 ) ) == NULL ) {
        fprintf( stderr, "Cat_name initialization failed\n" );
        exit( ABNORMAL_EXIT_CODE );
    }

    /* Generate catalog name, without optional prefix */
    strcpy( cat_name, cat_base_name );
    strcat( cat_name, cat_suffix );

    /* Chf initialization with msgcat subsystem;
       notice that on some systems (e.g. Digital UNIX) catopen() can succeed
       even if it was not able to open the right message catalog; better
       try it now.
    */
    if ( ( st = ChfMsgcatInit( argv[ 0 ],            /* Application's name */
                               CHF_DEFAULT,          /* Options */
                               cat_name,             /* Name of the message catalog */
                               CONDITION_STACK_SIZE, /* Size of the condition stack */
                               HANDLER_STACK_SIZE,   /* Size of the handler stack */
                               ABNORMAL_EXIT_CODE    /* Abnormal exit code */
                               ) ) != CHF_S_OK ||
         ChfGetMessage( CHF_MODULE_ID, UTIL_I_PACK_USAGE, NULL ) == NULL ) {
        if ( st != CHF_S_OK && st != CHF_F_CATOPEN ) {
            fprintf( stderr, "Chf initialization failed\n" );
            exit( ABNORMAL_EXIT_CODE );
        } else {
            fprintf( stderr, "Default message catalog open failed; trying alternate\n" );

            /* Bring down Chf before initializing it again */
            if ( st == CHF_S_OK )
                ChfExit();

            /* Try alternate message catalog name (with prefix) */
            strcpy( cat_name, cat_prefix );
            strcat( cat_name, cat_base_name );
            strcat( cat_name, cat_suffix );

            if ( ( st = ChfMsgcatInit( argv[ 0 ],            /* Application's name */
                                       CHF_DEFAULT,          /* Options */
                                       cat_name,             /* Name of the message catalog */
                                       CONDITION_STACK_SIZE, /* Size of the condition stack */
                                       HANDLER_STACK_SIZE,   /* Size of the handler stack */
                                       ABNORMAL_EXIT_CODE    /* Abnormal exit code */
                                       ) ) != CHF_S_OK ||
                 ChfGetMessage( CHF_MODULE_ID, UTIL_I_PACK_USAGE, NULL ) == NULL ) {
                fprintf( stderr, "Alternate Chf initialization failed\n" );
                exit( ABNORMAL_EXIT_CODE );
            }
        }
    }

    /* cat_name no longer needed */
    free( cat_name );

    /* Now, do some useful work; pack argv[1] into argv[2] */
    if ( argc != 3 ) {
        ChfCondition UTIL_I_PACK_USAGE, CHF_INFO ChfEnd;
        ChfCondition UTIL_F_PACK_CMD_LINE, CHF_FATAL ChfEnd;
        ChfSignal();
    }

    /* Get the size of the source file */
    if ( stat( argv[ 1 ], &statb ) ) {
        ChfErrnoCondition;
        ChfCondition UTIL_F_PACK_STAT, CHF_FATAL, argv[ 1 ] ChfEnd;
        ChfSignal();
    }

    /* Check that actual size is reasonable */
    if ( statb.st_size > MAX_SRC_SIZE ) {
        ChfCondition UTIL_F_PACK_SRC_SIZE, CHF_FATAL, statb.st_size ChfEnd;
        ChfSignal();
    }

    /* Allocate source buffer */
    if ( ( b = ( char* )malloc( statb.st_size ) ) == ( char* )NULL ||
         ( nb = ( Nibble* )malloc( sizeof( Nibble ) * statb.st_size ) ) == ( Nibble* )NULL ) {
        ChfErrnoCondition;
        ChfCondition UTIL_F_PACK_MALLOC, CHF_FATAL, statb.st_size ChfEnd;
        ChfSignal();

        return EXIT_FAILURE;
    }

    /* open/read/close */
    if ( ( d = open( argv[ 1 ], O_RDONLY ) ) == -1 ) {
        ChfErrnoCondition;
        ChfCondition UTIL_F_PACK_OPEN, CHF_FATAL, argv[ 1 ] ChfEnd;
        ChfSignal();
    }

    if ( read( d, b, statb.st_size ) != statb.st_size ) {
        ChfErrnoCondition;

        ( void )close( d );

        ChfCondition UTIL_F_PACK_READ, CHF_FATAL, argv[ 1 ] ChfEnd;
        ChfSignal();
    }

    ( void )close( d );

    /* Convert char -> Nibble */
    for ( i = 0; i < statb.st_size; i++ )
        nb[ i ] = ( Nibble )b[ i ];

    /* Source buffer no longer needed */
    free( b );

    /* Write */
    if ( WriteNibblesToFile( nb, statb.st_size, argv[ 2 ] ) ) {
        ChfCondition UTIL_F_PACK_WRITE_NIBBLES, CHF_FATAL ChfEnd;
        ChfSignal();
    }

    return EXIT_SUCCESS;
}
