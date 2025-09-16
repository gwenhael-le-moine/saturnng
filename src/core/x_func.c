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

.creation     :	3-Nov-2000

.description  :
  This module implements the emulator's extended functions, that is,
  functions that the real machine has not.

  References:

    Private communications with Prof. B. Parisse

.notes        :
  $Log: x_func.c,v $
  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.15  2000/11/15 14:16:45  cibrario
  GUI enhancements and assorted bug fixes:
  - Implemented command-line option -batchXfer

  Revision 3.14  2000/11/13 11:11:01  cibrario
  Implemented fast load/save; improved keyboard interface emulation at
  high emulated CPU speed:

  - Added credits in file doc
  - Implemented CPU status access functions ByteFromAddress(),
    NameFromD1()
  - Implemented new static function BinaryHeader(), to select an header
    of binary files appropriate for the current hw configuration
  - Implemented new static functions Kget()/KgetContinuation(), to load
    a file from disk into the calculator, and Send/SendContinuation(),
    to save a file from the calculator's memory into a disk file.
  - Removed test functions TestFSB() and TestFSBContinuation()
  - Implemented static helper function SetupXfer(), to setup a disk
    transfer.
  - Updated static function[] table

  Revision 3.13  2000/11/09 11:42:22  cibrario
  *** empty log message ***
.- */

#include <stdio.h>

#include "../options.h"

#include "cpu.h"
#include "disk_io.h"
#include "x_func.h"
#include "chf_wrapper.h"

/*---------------------------------------------------------------------------
        Private functions: action routines
  ---------------------------------------------------------------------------*/

/* Return the header of binary files for current hw configuration;
   '?' is a wildcard character when reading from file
   (see ReadObjectFromFile()) and is replaced by 'S' when writing
   to file (see WriteObjectToFile()).
   return NULL if the header cannot be determined.  In the latter case,
   generate an appropriate condition.
*/
static const char* BinaryHeader( void )
{
    switch ( config.model ) {
        case MODEL_48SX:
        case MODEL_48GX:
            return "HPHP48-?";
            break;
        case MODEL_40G:
        case MODEL_49G:
        case MODEL_50G:
            return "HPHP49-?";
            break;
        default:
            fprintf( stderr, "Error: Unknown model %i\n", config.model );
            return ( char* )NULL;
    }
}

/*---------------------------------------------------------------------------
        Public functions
  ---------------------------------------------------------------------------*/

void import_file( char* filename )
{
    /* const char* bin_hdr = BinaryHeader(); */

    /* if ( bin_hdr == ( const char* )NULL ) */
    /*     return; */

    /* int start_addr = R2int( cpu_status.A ); */
    /* int end_addr = R2int( cpu_status.C ); */

    /* DEBUG( X_FUNC_CHF_MODULE_ID, DEBUG_C_X_FUNC, X_FUNC_I_FILE_NAME, filename ) */
    /* DEBUG( X_FUNC_CHF_MODULE_ID, DEBUG_C_X_FUNC, X_FUNC_I_KGET, start_addr, end_addr, bin_hdr ) */

    /* ReadObjectFromFile( filename, bin_hdr, ( Address )start_addr, ( Address )end_addr ); */
}

/* This is the emulator's extended function for 'send': this function
   transfers an object from the calculator's memory into a disk file.
*/
void export_file( char* filename )
{
    /* const char* bin_hdr = BinaryHeader(); */

    /* if ( bin_hdr == ( const char* )NULL ) */
    /*     return; */

    /* int start_addr = R2int( cpu_status.A ); */
    /* int end_addr = R2int( cpu_status.C ); */

    /* DEBUG( X_FUNC_CHF_MODULE_ID, DEBUG_C_X_FUNC, X_FUNC_I_FILE_NAME, filename ) */
    /* DEBUG( X_FUNC_CHF_MODULE_ID, DEBUG_C_X_FUNC, X_FUNC_I_SEND, start_addr, end_addr, bin_hdr ) */

    /* WriteObjectToFile( ( Address )start_addr, ( Address )end_addr, bin_hdr, filename ); */
}

void set_speed( unsigned int new_speed_mhz )
{
    /* Compute inner loop limit; 4 is the real CPU speed in MHz when
       the limit is set to INNER_LOOP_MAX.  No overflow checks,
       because new_speed is >=0, has an architectural upper limit of 2^20,
       and int are at least 2^31.
     */
    cpu_status.inner_loop_max = ( new_speed_mhz * INNER_LOOP_MAX ) / 4;
}
