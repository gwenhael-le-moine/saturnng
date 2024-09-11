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

.identifier   : $Id: saturn.c,v 4.1 2000/12/11 09:54:19 cibrario Rel $
.context      : SATURN, Saturn CPU / HP48 emulator
.title	      : $RCSfile: saturn.c,v $
.kind	      : C source
.author	      : Ivan Cibrario B.
.site	      : CSTV-CNR
.creation     :	8-Sep-2000
.keywords     : *
.description  :
  This file contains the main program of the Saturn CPU / HP4x emulator

.include      : *

.notes	      :
  $Log: saturn.c,v $
  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.15  2000/11/15 14:07:53  cibrario
  GUI enhancements and assorted bug fixes:

  - made Chf initialization more robust with respect to incorrect
    locale settings

  - made stand-alone messages conforming to Chf standard format

  - the copyright notice is now output through the GUI, too, if stdout
    is not a tty

  Revision 3.14  2000/11/13 11:31:16  cibrario
  Implemented fast load/save; improved keyboard interface emulation at
  high emulated CPU speed:

  - Revision number bump with no changes

  Revision 3.11  2000/10/25 11:14:35  cibrario
  *** empty log message ***

  Revision 3.10  2000/10/24 16:14:56  cibrario
  Added/Replaced GPL header

  Revision 3.9  2000/10/24 16:11:20  cibrario
  The main program now has its own set of condition codes;
  added printf of MAIN_M_COPYRIGHT and MAIN_M_LICENSE as suggested by GPL

  Revision 3.8  2000/10/23 13:16:49  cibrario
  Bug fix:
  Improper use of sizeof() in main() could give a segmentation fault.

  Revision 3.1  2000/09/20 13:58:41  cibrario
  Minor updates and fixes to avoid gcc compiler warnings on Solaris
  when -ansi -pedantic -Wall options are selected.

 * Revision 2.5  2000/09/14  15:32:07  cibrario
 * Added invotation of SerialInit() in main program, to initialize
 * the serial port emulation module.
 *
 * Revision 2.1  2000/09/08  15:46:02  cibrario
 * *** empty log message ***
 *

.- */

#ifndef lint
static char rcs_id[] = "$Id: saturn.c,v 4.1 2000/12/11 09:54:19 cibrario Rel $";
#endif

#include <stdio.h>
#include <stdlib.h>
// #include <setjmp.h>
#include <string.h> /* 3.1: strcpy(), strcat() */
#include <unistd.h> /* isatty() */

#include "config.h"
#include "machdep.h"
#include "cpu.h"
#include "x11.h"
#include "serial.h"
#include "args.h"
#include "debug.h"

/* Chf condition codes (main program only) */

#define CHF_MODULE_ID MAIN_CHF_MODULE_ID
#include <Chf.h>

#define MAIN_M_COPYRIGHT 501
#define MAIN_M_LICENSE 502

/*---------------------------------------------------------------------------
   Chf parameters - Do not change.
   The ABNORMAL_EXIT_CODE is taken from stdlib.h (EXIT_FAILURE)
  ---------------------------------------------------------------------------*/

#define CONDITION_STACK_SIZE 16
#define HANDLER_STACK_SIZE 8
#define ABNORMAL_EXIT_CODE EXIT_FAILURE

/* Conditional prefix and mandatory suffix to make a message catalog
   name from argv[0]
*/
static const char cat_prefix[] = "./";
static const char cat_suffix[] = ".cat";

#define CAT_PREFIX_LEN ( sizeof( cat_prefix ) + 1 )
#define CAT_SUFFIX_LEN ( sizeof( cat_suffix ) + 1 )

static void adjust_setlocale( void )
{
    fprintf( stderr, "saturn-W-locale probably bad; reverting to C locale\n" );

    putenv( "LC_ALL=C" );
    putenv( "LC_COLLATE=C" );
    putenv( "LC_CTYPE=C" );
    putenv( "LC_MESSAGES=C" );
    putenv( "LC_MONETARY=C" );
    putenv( "LC_NUMERIC=C" );
    putenv( "LC_TIME=C" );
    putenv( "LANG=C" );
}

/*---------------------------------------------------------------------------
        Public functions
  ---------------------------------------------------------------------------*/

/* .+

.title	      : main
.kind	      : C function
.creation     : 8-Sep-2000
.description  :
  Main program.

.notes	      :
  2.1, 6-Sep-2000, creation
  2.5, 14-Sep-2000, update
    - added invotation of SerialInit().
  3.9, 23-Oct-2000, update
    - main() now has its own set of condition codes
  3.15, 15-Nov-2000, update
    - made Chf initialization more robust with respect to bad locales

.- */
int main( int argc, char* argv[] )
{
    char* cat_name;
    int st;
    int retry = 0;

    if ( ( cat_name = malloc( strlen( argv[ 0 ] ) + CAT_PREFIX_LEN + CAT_SUFFIX_LEN + 1 ) ) == NULL ) {
        fprintf( stderr, "saturn-E-cat_name initialization failed\n" );
        exit( ABNORMAL_EXIT_CODE );
    }

    /* Generate catalog name, without optional prefix */
    strcpy( cat_name, argv[ 0 ] );
    strcat( cat_name, cat_suffix );

    /* 3.15: Retry the initialization steps below two times; before trying
       the second time, adjust the setlocale() environment variables
       with adjust_setlocale()
    */
    while ( retry < 2 ) {
        /* Chf initialization with msgcat subsystem; notice that on
           some systems (e.g. Digital UNIX) catopen() can succeed even
           if it was not able to open the right message catalog;
           better try it now.
        */
        if ( ( st = ChfMsgcatInit( argv[ 0 ],            /* Application's name */
                                   CHF_DEFAULT,          /* Options */
                                   cat_name,             /* Name of the message catalog */
                                   CONDITION_STACK_SIZE, /* Size of the condition stack */
                                   HANDLER_STACK_SIZE,   /* Size of the handler stack */
                                   ABNORMAL_EXIT_CODE    /* Abnormal exit code */
                                   ) ) != CHF_S_OK ||
             ChfGetMessage( CHF_MODULE_ID, MAIN_M_COPYRIGHT, NULL ) == NULL )
            fprintf( stderr, "saturn-E-Primary Chf initialization failed (%d)\n", st );
        else
            break;

        /* Bring down Chf before initializing it again */
        if ( st == CHF_S_OK )
            ChfExit();

        /* Try alternate message catalog name (with prefix) */
        strcpy( cat_name, cat_prefix );
        strcat( cat_name, argv[ 0 ] );
        strcat( cat_name, cat_suffix );

        if ( ( st = ChfMsgcatInit( argv[ 0 ],            /* Application's name */
                                   CHF_DEFAULT,          /* Options */
                                   cat_name,             /* Name of the message catalog */
                                   CONDITION_STACK_SIZE, /* Size of the condition stack */
                                   HANDLER_STACK_SIZE,   /* Size of the handler stack */
                                   ABNORMAL_EXIT_CODE    /* Abnormal exit code */
                                   ) ) != CHF_S_OK ||
             ChfGetMessage( CHF_MODULE_ID, MAIN_M_COPYRIGHT, NULL ) == NULL )
            fprintf( stderr, "saturn-E-Alternate Chf initialization failed (%d)\n", st );
        else
            break;

        /* Bring down Chf before initializing it again */
        if ( st == CHF_S_OK )
            ChfExit();

        /* Attempt to adjust setlocale() environment variables */
        if ( retry++ == 0 )
            adjust_setlocale();
    }

    if ( retry == 2 ) {
        fprintf( stderr, "saturn-F-Application aborted\n" );
        exit( ABNORMAL_EXIT_CODE );
    }

    /* cat_name no longer needed */
    free( cat_name );

    /* 3.9: Print out MAIN_M_COPYRIGHT and MAIN_M_LICENSE on stdout now */
    fprintf( stdout, ChfGetMessage( CHF_MODULE_ID, MAIN_M_COPYRIGHT, "" ), "$Revision: 4.1 $" );
    fprintf( stdout, ChfGetMessage( CHF_MODULE_ID, MAIN_M_LICENSE, "" ) );

    /* Initialize GUI and associated lcd display emulation module */
    InitializeGui( argc, argv );

    /* Initialize serial port emulation.
       This function returns the name of the slave side of the pty;
       it is unused here, because the GUI condition handler intercepts
       a condition containing the same information and displays the pty name
       on the main emulator's window.
    */
    ( void )SerialInit();

    /* Initialize emulator proper */
    EmulatorInit();

    /* 3.15: Repeat copyright message on GUI if stdout is not a tty */
    if ( !isatty( fileno( stdout ) ) ) {
        ChfCondition MAIN_M_LICENSE, CHF_INFO ChfEnd;
        ChfCondition MAIN_M_COPYRIGHT, CHF_INFO, "$Revision: 4.1 $" ChfEnd;

        ChfSignal();
    }

    if ( args.monitor ) {
        /* Invoke Monitor */
        void Monitor( void );
        Monitor();
    } else
        /* Call Emulator directly */
        Emulator();

    return EXIT_SUCCESS;
}
