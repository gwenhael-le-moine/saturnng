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

.creation     :	25-Sep-2000

.description  :
  This module emulates the Internal Flash Rom of the HP49.

  References:

    28F160S5/28F320S5 Data Sheet, by Intel Corp.

.notes        :
  $Log: flash49.c,v $
  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.10  2000/10/24 16:14:40  cibrario
  Added/Replaced GPL header

  Revision 3.5  2000/10/02 09:49:18  cibrario
  Linux support:
  - added a default case in StoreData()'s switch; this is cleaner, and
    makes gcc happier.

  Revision 3.3  2000/09/26 15:30:14  cibrario
  *** empty log message ***
.- */
#include "../libChf/src/Chf.h"

#include "config.h"
#include "cpu.h"
#include "modules.h"
#include "flash49.h"
#include "debug.h"

/*---------------------------------------------------------------------------
        Private Macro/Data type definitions
  ---------------------------------------------------------------------------*/

#define BLOCK_SIZE 0x10000
#define BLOCK_BASE_MASK 0xFFFF

#define ByteAddress( address ) ( ( address ) >> 1 )
#define NibbleAddress( address ) ( ( address ) << 1 )
#define BlockBase( address ) ( ( address ) & ~BLOCK_BASE_MASK )
#define IsOdd( address ) ( ( address ) & 0x1 )
#define LowNibble( d ) ( ( Nibble )( ( d ) & NIBBLE_MASK ) )
#define HighNibble( d ) ( ( Nibble )( ( ( d ) >> 4 ) & NIBBLE_MASK ) )
#define ShiftHigh( d ) ( ( d ) << 4 )

/* Flash cycle types */
enum FlashCycle {
    FLASH_CYCLE_READ = 0,
    FLASH_CYCLE_WRITE,

    FLASH_CYCLE_N /* Total # of cycle types */
};

/* State transition function */
typedef int ( *FlashF )( enum FlashState* state, enum FlashCycle cycle, XAddress address, int data );

/*---------------------------------------------------------------------------
        Private state variables
  ---------------------------------------------------------------------------*/

/* External storage */
extern struct ModStatus_49* mod_status_49;

static int r_buffer;              /* Nibble buffer during read */
static int w_buffer;              /* Nibble buffer during write */
static enum FlashState fsm_state; /* FSM state */

/* Write buffer */
#define WB_COUNT_MASK 0x1F
#define WB_SIZE 0x20
static int wb_count;      /* Counter for Write to Buffer */
static int wb_cdown;      /* Count down */
static XAddress wb_start; /* Start address for Write to Buffer */
static int wb[ WB_SIZE ]; /* Write buffer */

/*---------------------------------------------------------------------------
        State transition private functions
  ---------------------------------------------------------------------------*/

/* This function is called by default for unhandled state transitions */
static int BadCommand( enum FlashState* state, enum FlashCycle cycle, XAddress address, int data )
{
    /* Unknown command: signal and reset state to FLASH_ST_READ_ARRAY */
    ChfGenerate( FLASH_CHF_MODULE_ID, __FILE__, __LINE__, FLASH_W_BAD_CMD, CHF_WARNING, *state, cycle, address, data );
    ChfSignal( FLASH_CHF_MODULE_ID );

    *state = FLASH_ST_READ_ARRAY;
    return 0; /* Dummy result */
}

/* This function is called to read the Flash Rom array */
static int ReadArray( enum FlashState* state, enum FlashCycle cycle, XAddress address, int data )
{
    /* Read a byte from the array; no state transitions */
    return mod_status_49->flash[ NibbleAddress( address ) ] | ShiftHigh( mod_status_49->flash[ NibbleAddress( address ) + 1 ] );
}

/* This function is called to parse the first byte of any command */
static int ParseCommand( enum FlashState* state, enum FlashCycle cycle, XAddress address, int data )
{
    switch ( data ) {
        case FLASH_CMD_READ_ARRAY:
            /* Transition to FLASH_ST_READ_ARRAY state */
            debug1( FLASH_CHF_MODULE_ID, DEBUG_C_FLASH, FLASH_I_FSM_OP, "Read Array" );
            *state = FLASH_ST_READ_ARRAY;
            break;

        case FLASH_CMD_CLR_SR:
            /* Clear status register; section 4.5 on Data Sheet.
               The current implementation does nothing, because
               the value of the status register is fixed. No state transitions.
            */
            debug1( FLASH_CHF_MODULE_ID, DEBUG_C_FLASH, FLASH_I_FSM_OP, "Clear Status" );
            break;

        case FLASH_CMD_WRITE_BUFFER:
            /* Write to Buffer; section 4.8 on Data Sheet.
               Transition to FLASH_ST_READ_XSR state.
            */
            debug1( FLASH_CHF_MODULE_ID, DEBUG_C_FLASH, FLASH_I_FSM_OP, "Write to Buffer (start)" );
            *state = FLASH_ST_READ_XSR;
            break;

        case FLASH_CMD_READ_SR:
            /* Read Status; section 4.4 on Data Sheet.
               Transition to FLASH_ST_READ_SR state.
            */
            debug1( FLASH_CHF_MODULE_ID, DEBUG_C_FLASH, FLASH_I_FSM_OP, "Read Status" );
            *state = FLASH_ST_READ_SR;
            break;

        case FLASH_CMD_BL_ERASE:
            /* Block Erase; section 4.6 on Data Sheet.
               Transition to FLASH_ST_BL_ERASE state.
               Consistency of block addresses is not checked.
            */
            debug1( FLASH_CHF_MODULE_ID, DEBUG_C_FLASH, FLASH_I_FSM_OP, "Erase Block (start)" );
            *state = FLASH_ST_BL_ERASE;
            break;

        default:
            /* Unknown command; signal, ignore, keep current state. */
            ChfGenerate( FLASH_CHF_MODULE_ID, __FILE__, __LINE__, FLASH_W_BAD_CMD, CHF_WARNING, *state, cycle, address, data );
            ChfSignal( FLASH_CHF_MODULE_ID );
            break;
    }

    return 0; /* No result; this is a write cycle */
}

/* This function returns to the caller the value of XSR */
static int ReadXSR( enum FlashState* state, enum FlashCycle cycle, XAddress address, int data )
{
    /* Return XSR status; a buffer is always available in the current
       emulation scheme.  Keep current state.
    */
    debug1( FLASH_CHF_MODULE_ID, DEBUG_C_FLASH, FLASH_I_FSM_OP, "Read XSR" );
    return FLASH_XSR_WBS;
}

/* This function returns to the caller the value of SR */
static int ReadSR( enum FlashState* state, enum FlashCycle cycle, XAddress address, int data )
{
    /* Return SR status; the WSM executes in zero time in the current
       emulation scheme.  Keep current state.
    */
    debug1( FLASH_CHF_MODULE_ID, DEBUG_C_FLASH, FLASH_I_FSM_OP, "Read SR" );
    return FLASH_SR_WSMS;
}

/* This function is called to store the WRITE_BUFFER byte count;
   both wb_count and wb_cdown are set; StoreData() decrements the
   latter, WriteConfirm() uses the former to determine how many bytes
   must write to the Flash array.
*/
static int StoreCount( enum FlashState* state, enum FlashCycle cycle, XAddress address, int data )
{
    /* Store WRITE_BUFFER count; next state is FLASH_ST_WRITE_DATA */
    debug1( FLASH_CHF_MODULE_ID, DEBUG_C_FLASH, FLASH_I_FSM_OP, "Write to Buffer (count)" );
    wb_count = wb_cdown = data & WB_COUNT_MASK;

    *state = FLASH_ST_WRITE_DATA_1;
    return 0; /* No result; this is a write cycle */
}

/* This function is called to store a byte into the write buffer.
   The first write cycle also sets the buffer's base address (wb_start).
   The function transitions to state FLASH_ST_WRITE_CONFIRM when all
   bytes have been stored.
*/
static int StoreData( enum FlashState* state, enum FlashCycle cycle, XAddress address, int data )
{
    int index;

    debug1( FLASH_CHF_MODULE_ID, DEBUG_C_FLASH, FLASH_I_FSM_OP, "Write to Buffer (data)" );

    /* Store WRITE_BUFFER data; the first write also stores the
       buffer starting address.
    */
    switch ( *state ) {

        case FLASH_ST_WRITE_DATA_1:
            wb_start = address;
            wb[ 0 ] = data;
            *state = FLASH_ST_WRITE_DATA_N;
            break;

        case FLASH_ST_WRITE_DATA_N:
            index = address - wb_start;
            if ( index < 0 || index >= WB_SIZE ) {
                ChfGenerate( FLASH_CHF_MODULE_ID, __FILE__, __LINE__, FLASH_W_BAD_ADDRESS, CHF_WARNING, *state, cycle, address, data );
                ChfSignal( FLASH_CHF_MODULE_ID );
            } else
                wb[ index ] = data;
            break;

        default:
            *state = FLASH_ST_READ_ARRAY;
            break;
    }

    if ( --wb_cdown < 0 )
        *state = FLASH_ST_WRITE_CONFIRM;

    return 0; /* No result; this is a write cycle */
}

/* This function expects a Write to Buffer confirmation command
   (FLASH_CMD_WRITE_BUFFER_2); if it is received, the write buffer
   is copied into the Flash Rom array, otherwise the write cycle is
   aborted.  In both cases, the new state is FLASH_ST_READ_ARRAY.
*/
static int WriteConfirm( enum FlashState* state, enum FlashCycle cycle, XAddress address, int data )
{
    debug1( FLASH_CHF_MODULE_ID, DEBUG_C_FLASH, FLASH_I_FSM_OP, "Write to Buffer (end)" );

    /* Expect Write to Buffer confirmation code */
    if ( data == FLASH_CMD_WRITE_BUFFER_2 ) {
        int i;

        /* Confirmation OK; write.
           Remember that wb_count is the byte count MINUS 1.
        */
        for ( i = 0; i <= wb_count; i++ ) {
            mod_status_49->flash[ NibbleAddress( wb_start + i ) ] = LowNibble( wb[ i ] );
            mod_status_49->flash[ NibbleAddress( wb_start + i ) + 1 ] = HighNibble( wb[ i ] );
        }
    }

    *state = FLASH_ST_READ_ARRAY;
    return 0; /* No result */
}

/* If the FLASH_CMD_BL_ERASE_2 command is received, this function erases
   the block pointed by the current address; otherwise the block erase
   operation is aborted.
   In both cases, the new state is FLASH_ST_READ_SR.
*/
static int BlockErase( enum FlashState* state, enum FlashCycle cycle, XAddress address, int data )
{
    debug1( FLASH_CHF_MODULE_ID, DEBUG_C_FLASH, FLASH_I_FSM_OP, "Block Erase (end)" );

    /* Expect Write to Buffer confirmation code */
    if ( data == FLASH_CMD_BL_ERASE_2 ) {
        XAddress block_base = BlockBase( address );
        int i;

        /* Confirmation OK; erase */
        for ( i = 0; i < BLOCK_SIZE; i++ ) {
            mod_status_49->flash[ NibbleAddress( block_base + i ) ] = ( Nibble )0xF;
            mod_status_49->flash[ NibbleAddress( block_base + i ) + 1 ] = ( Nibble )0xF;
        }
    }

    *state = FLASH_ST_READ_SR;
    return 0; /* No result */
}

/*---------------------------------------------------------------------------
   FSM state diagram; two-dimensional array of FlashF; each function
   is invoked when the FSM is in a given state (first index), and
   a particular cycle (second index) is requested.
  ---------------------------------------------------------------------------*/

static FlashF F[ FLASH_ST_N ][ FLASH_CYCLE_N ] = {
    {ReadArray,  ParseCommand}, /* FLASH_ST_READ_ARRAY */
    {ReadSR,     ParseCommand}, /* FLASH_ST_READ_SR */
    {ReadXSR,    StoreCount  }, /* FLASH_ST_READ_XSR */
    {BadCommand, StoreData   }, /* FLASH_ST_WRITE_DATA_1 */
    {BadCommand, StoreData   }, /* FLASH_ST_WRITE_DATA_N */
    {BadCommand, WriteConfirm}, /* FLASH_ST_WRITE_CONFIRM */
    {BadCommand, BlockErase  }  /* FLASH_ST_BL_ERASE */
};

/*---------------------------------------------------------------------------
        Other private functions
  ---------------------------------------------------------------------------*/

/* This function invokes the FSM to execute the given 'cycle',
   with 'address' and 'data' as arguments.  Returns the
   result of the FSM, if any.
*/
static int FSM( enum FlashCycle cycle, XAddress address, int data )
{
    int result;

    debug2( FLASH_CHF_MODULE_ID, DEBUG_C_FLASH, FLASH_I_FSM, fsm_state, cycle );
    debug2( FLASH_CHF_MODULE_ID, DEBUG_C_FLASH, FLASH_I_FSM_AD, address, data );

    result = F[ fsm_state ][ cycle ]( &fsm_state, cycle, address, data );

    debug2( FLASH_CHF_MODULE_ID, DEBUG_C_FLASH, FLASH_I_FSM_RESULT, fsm_state, result );
    return result;
}

/*---------------------------------------------------------------------------
        Public functions
  ---------------------------------------------------------------------------*/

/* .+

.creation     : 25-Sep-2000
.description  :
  This function reads the nibble @address from Flash Rom and returns it
  to the caller.

  This function DOES NOT supports even/odd nibble accesses at non-contiguous
  addresses.

.call         :
                n = FlashRead49(address);
.input        :
                XAddress address
.output       :
                Nibble n
.status_codes :
                FLASH_I_READ
                FLASH_I_FSM_OP
                FLASH_W_BAD_CMD
.notes        :
  3.3, 25-Sep-2000, creation

.- */
Nibble FlashRead49( XAddress address )
{
    Nibble result;

    if ( IsOdd( address ) )
        /* Odd address, return buffered data from previous read */
        result = HighNibble( r_buffer );
    else {
        /* Even address, invoke FSM */
        r_buffer = FSM( FLASH_CYCLE_READ, ByteAddress( address ), 0 );
        result = LowNibble( r_buffer );
    }

    debug2( FLASH_CHF_MODULE_ID, DEBUG_C_TRACE | DEBUG_C_FLASH, FLASH_I_READ, address, result );

    return result;
}

/* .+

.creation     : 25-Sep-2000
.description  :
  This function writes nibble datum @address into Flash Rom.

  This function DOES NOT supports even/odd nibble accesses at non-contiguous
  addresses.

.call         :
                FlashWrite49(address, datum);
.input        :
                XAddress address
                Nibble datum
.output       :
                void
.status_codes :
                FLASH_I_WRITE
                FLASH_I_FSM_OP
                FLASH_W_BAD_CMD
                FLASH_W_BAD_ADDRESS
.notes        :
  3.3, 25-Sep-2000, creation

.- */
void FlashWrite49( XAddress address, Nibble datum )
{
    debug2( FLASH_CHF_MODULE_ID, DEBUG_C_TRACE | DEBUG_C_FLASH, FLASH_I_WRITE, address, datum );

    if ( IsOdd( address ) )
        /* Odd address, invoke FSM; ignore result */
        FSM( FLASH_CYCLE_WRITE, ByteAddress( address ), w_buffer | ShiftHigh( datum ) );
    else
        /* Even address, buffer datum */
        w_buffer = datum;
}
