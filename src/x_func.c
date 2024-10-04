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

.identifier   : $Id: x_func.c,v 4.1 2000/12/11 09:54:19 cibrario Rel $
.context      : SATURN, Saturn CPU / HPxx emulator
.title	      : $RCSfile: x_func.c,v $
.kind	      : C source
.author	      : Ivan Cibrario B.
.site	      : CSTV-CNR
.creation     :	3-Nov-2000
.keywords     : *
.description  :
  This module implements the emulator's extended functions, that is,
  functions that the real machine has not.

  References:

    Private communications with Prof. B. Parisse

.include      : config.h, machdep.h, cpu.h, x_func.h

.notes	      :
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

#ifndef lint
static char rcs_id[] = "$Id: x_func.c,v 4.1 2000/12/11 09:54:19 cibrario Rel $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "config.h"
#include "machdep.h"
#include "cpu.h"
#include "modules.h"
#include "disk_io.h"
#include "x_func.h"
#include "debug.h"

#define CHF_MODULE_ID X_FUNC_CHF_MODULE_ID
#include "libChf/src/Chf.h"

/*---------------------------------------------------------------------------
        Private functions: CPU access
  ---------------------------------------------------------------------------*/

/* Return the A field of a DataRegister as an integer. */
static int R2int( const Nibble* r )
{
    return ( ( ( int )r[ 0 ] ) | ( ( int )r[ 1 ] << 4 ) | ( ( int )r[ 2 ] << 8 ) | ( ( int )r[ 3 ] << 12 ) | ( ( int )r[ 4 ] << 16 ) );
}

/* Return the contents of the byte pointed by addr.
   Memory is accessed through ReadNibble()
*/
static int ByteFromAddress( Address addr ) { return ( int )ReadNibble( addr ) + ( int )ReadNibble( addr + 1 ) * 16; }

/* Return a dynamically-allocated copy of the contents of the IDNT
   object pointed by D1.  D1 points to the *body* of the
   RPL object, that is, to the IDNT length byte directly and *not*
   to the prologue.
*/
static char* NameFromD1( void )
{
    Address addr = cpu_status.D1;      /* Points to the IDNT body */
    int len = ByteFromAddress( addr ); /* IDNT length */
    char* name = malloc( len + 1 );    /* IDNT name buffer */
    int c;

    /* Read the name; toascii() is there to avoid 'strange' characters */
    for ( c = 0; c < len; c++ ) {
        addr += 2;
        name[ c ] = ( char )toascii( ByteFromAddress( addr ) );
    }

    name[ c ] = '\0'; /* Terminate and return the name */
    return name;
}

/*---------------------------------------------------------------------------
        Private functions: action routines
  ---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/

/* Set the emulator speed to the given value (in MHz); the desired speed
   value is held in the A field of the C CPU register.  No handshake.
*/
static void SetSpeed( Nibble function_code )
{
    debug1( DEBUG_C_TRACE, X_FUNC_I_CALLED, "SetSpeed" );

#ifndef REAL_CPU_SPEED
    ChfCondition( X_FUNC_CHF_MODULE_ID ) X_FUNC_E_NO_SPEED, CHF_ERROR ChfEnd;
    ChfSignal( X_FUNC_CHF_MODULE_ID );

#else
    {
        int new_speed;

        /* Get new_speed from A field of C register */
        new_speed = R2int( cpu_status.C );

        /* Compute inner loop limit; 4 is the real CPU speed in MHz when
           the limit is set to INNER_LOOP_MAX.  No overflow checks,
           because new_speed is >=0, has an architectural upper limit of 2^20,
           and int are at least 2^31.
        */
        cpu_status.inner_loop_max = ( new_speed * INNER_LOOP_MAX ) / 4;

        /* Notify the user about the speed change */
        if ( cpu_status.inner_loop_max )
            ChfCondition( X_FUNC_CHF_MODULE_ID ) X_FUNC_I_SET_SPEED, CHF_INFO, new_speed ChfEnd;
        else
            ChfCondition( X_FUNC_CHF_MODULE_ID ) X_FUNC_I_MAX_SPEED, CHF_INFO ChfEnd;

        ChfSignal( X_FUNC_CHF_MODULE_ID );
    }

#endif
}

/*---------------------------------------------------------------------------*/

/* This array holds the binary headers for all known hw configurations;
   here, '?' is a wildcard character when reading from file
   (see ReadObjectFromFile()) and is replaced by 'S' when writing
   to file (see WriteObjectToFile()).
*/
struct BinHdrMapping {
    char* hw;
    char* hdr;
};

static const struct BinHdrMapping bin_hdr_mapping[] = {
    {"hp48", "HPHP48-?"},
    {"hp49", "HPHP49-?"}
};

#define N_BIN_HDR_MAPPING ( int )( sizeof( bin_hdr_mapping ) / sizeof( bin_hdr_mapping[ 0 ] ) )

/* Return the header of binary files for current hw configuration;
   return NULL if the header cannot be determined.  In the latter case,
   generate an appropriate condition.
*/
static const char* BinaryHeader( void )
{
    int i;

    for ( i = 0; i < N_BIN_HDR_MAPPING; i++ )
        if ( strcmp( config.hw, bin_hdr_mapping[ i ].hw ) == 0 )
            return bin_hdr_mapping[ i ].hdr;

    ChfCondition( X_FUNC_CHF_MODULE_ID ) X_FUNC_E_NO_BIN_HDR, CHF_ERROR, config.hw ChfEnd;
    return ( char* )NULL;
}

/* This function is the continuation of Kget(); it is invoked when the
   user interaction with the FSB ends.
*/
static void KgetContinuation( int proceed, char* file_name )
{
    /* Check whether continuation should proceed */
    if ( !proceed ) {
        ChfCondition( X_FUNC_CHF_MODULE_ID ) X_FUNC_W_ABORTED, CHF_WARNING ChfEnd;
        ChfSignal( X_FUNC_CHF_MODULE_ID );
    } else {
        /* Ok to proceed; read:
           - target start address from A[A]
           - target end address from C[A]
           - binary header with BinaryHeader()
        */
        int start_addr = R2int( cpu_status.A );
        int end_addr = R2int( cpu_status.C );
        const char* bin_hdr = BinaryHeader();

        debug1( DEBUG_C_X_FUNC, X_FUNC_I_FILE_NAME, file_name );
        debug3( DEBUG_C_X_FUNC, X_FUNC_I_KGET, start_addr, end_addr, bin_hdr );

        if ( bin_hdr == ( const char* )NULL || ReadObjectFromFile( file_name, bin_hdr, ( Address )start_addr, ( Address )end_addr ) ) {
            ChfCondition( X_FUNC_CHF_MODULE_ID ) X_FUNC_W_FAILED, CHF_WARNING ChfEnd;
            ChfSignal( X_FUNC_CHF_MODULE_ID );
        }
    }

    CpuRunRequest();
}

/* This function is the continuation of Send(); it is invoked when the
   user interaction with the FSB ends.
*/
static void SendContinuation( int proceed, char* file_name )
{
    if ( !proceed ) {
        ChfCondition( X_FUNC_CHF_MODULE_ID ) X_FUNC_W_ABORTED, CHF_WARNING ChfEnd;
        ChfSignal( X_FUNC_CHF_MODULE_ID );
    } else {
        /* Ok to proceed; read:
           - source start address from A[A]
           - source end address from C[A]
           - binary header with BinaryHeader()
        */
        int start_addr = R2int( cpu_status.A );
        int end_addr = R2int( cpu_status.C );
        const char* bin_hdr = BinaryHeader();

        debug1( DEBUG_C_X_FUNC, X_FUNC_I_FILE_NAME, file_name );
        debug3( DEBUG_C_X_FUNC, X_FUNC_I_SEND, start_addr, end_addr, bin_hdr );

        if ( bin_hdr == ( const char* )NULL || WriteObjectToFile( ( Address )start_addr, ( Address )end_addr, bin_hdr, file_name ) ) {
            ChfCondition( X_FUNC_CHF_MODULE_ID ) X_FUNC_W_FAILED, CHF_WARNING ChfEnd;
            ChfSignal( X_FUNC_CHF_MODULE_ID );
        }
    }

    CpuRunRequest();
}

/* This function does the setup of a transfer, performing the following
   actions:

   - If CPU halt requests are not allowed, it signals X_FUNC_E_NO_HALT
   - Gets the FSB title from 'msg' and 'def_msg', using ChfGetMessage()
   - Gets the default file name using NameFromD1()
   - Invokes ActivateFSB() to pop the FSB up; continuation 'cont' will
     be invoked when the user interaction ends
   - Halts the CPU
 */
typedef void ( *FsbContinuation )( int proceed, char* file_name );
static void SetupXfer( int msg, const char* def_msg, FsbContinuation cont )
{
    debug1( DEBUG_C_TRACE, X_FUNC_I_CALLED, "SetupXfer" );

    if ( CpuHaltAllowed() ) {
        /* char* fsb_title = XtNewString( ChfGetMessage( CHF_MODULE_ID, msg, def_msg ) ); */

        /* char* fsb_file = NameFromD1(); */

        /* // ActivateFSB( fsb_title, fsb_file, cont ); */

        /* /\* Free *before* CpuHaltRequest() because it does not return, and */
        /*    ActivateFSB() copied its argument when necessary. */
        /* *\/ */
        /* XtFree( fsb_title ); */
        /* XtFree( fsb_file ); */

        /* ( void )CpuHaltRequest(); */
    } else {
        ChfCondition( X_FUNC_CHF_MODULE_ID ) X_FUNC_E_NO_HALT, CHF_ERROR ChfEnd;
        ChfSignal( X_FUNC_CHF_MODULE_ID );
    }
}

/* This is the emulator's extended function for 'kget': this function
   transfers a file from disk into the calculator's memory.
*/
static void Kget( Nibble function_code )
{
    debug1( DEBUG_C_TRACE, X_FUNC_I_CALLED, "Kget" );

    /* Setup File Selection Box if transfers are *not* in batch mode */
    if ( !config.batchXfer )
        SetupXfer( X_FUNC_M_KGET, "Kget", KgetContinuation );
    else {
        /* Ok to proceed; read:
           - file name from @D1
           - target start address from A[A]
           - target end address from C[A]
           - binary header with BinaryHeader()
        */
        char* file_name = NameFromD1();
        int start_addr = R2int( cpu_status.A );
        int end_addr = R2int( cpu_status.C );
        const char* bin_hdr = BinaryHeader();

        debug1( DEBUG_C_X_FUNC, X_FUNC_I_FILE_NAME, file_name );
        debug3( DEBUG_C_X_FUNC, X_FUNC_I_KGET, start_addr, end_addr, bin_hdr );

        if ( bin_hdr == ( const char* )NULL || ReadObjectFromFile( file_name, bin_hdr, ( Address )start_addr, ( Address )end_addr ) ) {
            ChfCondition( X_FUNC_CHF_MODULE_ID ) X_FUNC_W_FAILED, CHF_WARNING ChfEnd;
            ChfSignal( X_FUNC_CHF_MODULE_ID );
        }
    }
}

/* This is the emulator's extended function for 'send': this function
   transfers an object from the calculator's memory into a disk file.
*/
static void Send( Nibble function_code )
{
    debug1( DEBUG_C_TRACE, X_FUNC_I_CALLED, "Send" );

    /* Setup File Selection Box if transfers are *not* in batch mode */
    if ( !config.batchXfer )
        SetupXfer( X_FUNC_M_SEND, "Send", SendContinuation );
    else {
        /* Ok to proceed; read:
           - file name from @D1
           - source start address from A[A]
           - source end address from C[A]
           - binary header with BinaryHeader()
        */
        char* file_name = NameFromD1();
        int start_addr = R2int( cpu_status.A );
        int end_addr = R2int( cpu_status.C );
        const char* bin_hdr = BinaryHeader();

        debug1( DEBUG_C_X_FUNC, X_FUNC_I_FILE_NAME, file_name );
        debug3( DEBUG_C_X_FUNC, X_FUNC_I_SEND, start_addr, end_addr, bin_hdr );

        if ( bin_hdr == ( const char* )NULL || WriteObjectToFile( ( Address )start_addr, ( Address )end_addr, bin_hdr, file_name ) ) {
            ChfCondition( X_FUNC_CHF_MODULE_ID ) X_FUNC_W_FAILED, CHF_WARNING ChfEnd;
            ChfSignal( X_FUNC_CHF_MODULE_ID );
        }
    }
}

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
   Dispatch table of emulator's extended functions, indexed by function code;
   the function code is propagated to functions in the table.
  ---------------------------------------------------------------------------*/

typedef void ( *XFunc )( Nibble );

static const XFunc function[] = {
    SetSpeed, /* Function code 0 */
    Kget,     /* 1 */
    Send      /* 2 */
};

#define N_X_FUNC ( int )( sizeof( function ) / sizeof( function[ 0 ] ) )

/*---------------------------------------------------------------------------
        Public functions
  ---------------------------------------------------------------------------*/

/* .+

.title	      : ExtendedFunction
.kind	      : C function
.creation     : 3-Nov-2000
.description  :
  This function executes the emulator's extended function identified
  by 'function_code'; communications with the triggering code on
  the calculator side are made through CPU registers.

.call	      :
                ExtendedFunction(function_code);
.input	      :
                Nibble function_code, function code
.output	      :
                void
.status_codes :
                X_FUNC_I_CALLED
                X_FUNC_I_CODE
                X_FUNC_W_BAD_CODE
                * Any other condition code generated by action functions
.notes	      :
  3.13, 3-Nov-2000, creation

.- */
void ExtendedFunction( Nibble function_code )
{
    debug1( DEBUG_C_TRACE, X_FUNC_I_CALLED, "ExtendedFunction" );
    debug1( DEBUG_C_X_FUNC, X_FUNC_I_CODE, function_code );

    /* Some sanity checks, first */
    if ( function_code < 0 || function_code >= N_X_FUNC || function[ ( int )function_code ] == ( XFunc )NULL ) {
        ChfCondition( X_FUNC_CHF_MODULE_ID ) X_FUNC_W_BAD_CODE, CHF_WARNING, function_code ChfEnd;
        ChfSignal( X_FUNC_CHF_MODULE_ID );
    }
    /* Dispatch */
    else
        function[ ( int )function_code ]( function_code );
}
