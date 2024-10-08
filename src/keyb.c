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

.identifier   : $Id: keyb.c,v 4.1 2000/12/11 09:54:19 cibrario Rel $
.context      : SATURN, Saturn CPU / HP48 emulator
.title	      : $RCSfile: keyb.c,v $
.kind	      : C source
.author	      : Ivan Cibrario B.
.site	      : CSTV-CNR
.creation     :	29-Jan-1998
.keywords     : *
.description  :
  This module emulates the keyboard interface of the Yorke chip.
  References:

    SASM.DOC by HP  (HORN disk 4)
    Guide to the Saturn Processor Rev. 0.00f by Matthew Mastracci
    entries.srt by Mika Heiskanen  (mheiskan@vipunen.hut.fi)
    x48 source code by Eddie C. Dost  (ecd@dressler.de)

  NOTE: In the current (r1.1) implementation,
        the emulation accuracy could be poor.

.include      : config.h, machdep.h, cpu.h, modules.h, keyb.h

.notes	      :
  $Log: keyb.c,v $
  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.13  2000/11/09 11:32:49  cibrario
  Revised to add file selection box GUI element, CPU halt/run
  requests and emulator's extended functions:

  - Implemented KeybReset()

  Revision 3.10  2000/10/24 16:14:44  cibrario
  Added/Replaced GPL header

  Revision 2.1  2000/09/08 15:17:32  cibrario
  Deep revision to accommodate the new GUI: all facilities to map a
  key code (enum Key) into an IN/OUT pair have been removed,
  because the GUI now does this function itself; KeybPress() and
  KeybRelease() now directly accept an IN/OUT pair as input.

 * Revision 1.1  1998/02/17  11:53:23  cibrario
 * Initial revision
 *

.- */

#include <stdio.h>
#include <string.h>

#include "config.h"
#include "machdep.h"
#include "cpu.h"
#include "modules.h"
#include "keyb.h"
#include "debug.h"

#define CHF_MODULE_ID MOD_CHF_MODULE_ID
#include "libChf/src/Chf.h"

#define OUT_BITS 12

/* cur_in:

   This array contains the current value the CPU IN register will assume
   for each bit set in the OUT register.
*/
static InputRegister cur_in[ OUT_BITS ];

/* .+

.title	      : KeybRSI
.kind	      : C function
.creation     : 11-Feb-1998
.description  :
  This function is called by the CPU emulator when the RSI instruction is
  executed. It resets the keyboard interrupt system and posts a maskable
  interrupt request if any key is pressed.

  NOTE: This function currently (r1.1) always posts an IRQ request; perhaps,
        if the ON key is down, a NMI request should be posted instead.

.call	      :
                KeybRSI();
.input	      :
                void
.output	      :
                void
.status_codes :
                *
.notes	      :
  1.1, 17-Feb-1998, creation

.- */
void KeybRSI( void )
{
    /* Post an IRQ if the IN register is not zero */

    CpuIntRequest( KeybIN( ( OutputRegister )0x1FF ) != ( InputRegister )0 ? INT_REQUEST_IRQ : INT_REQUEST_NONE );
}

/* .+

.title	      : KeybIn
.kind	      : C function
.creation     : 11-Feb-1998
.description  :
  This function is called by the CPU emulator when either a C=IN or a A=IN
  instruction is executed. It scans the keyboard and returns the current
  value of the IN register for the given value of the OUT reguster.

.call	      :
                in = KeybIN(out);
.input	      :
                OutputRegister out, current value of the OUT register
.output	      :
                InputRegister in, computed value of the IN register
.status_codes :
                *
.notes	      :
  1.1, 17-Feb-1998, creation

.- */
InputRegister KeybIN( OutputRegister out )
{
    /* Compute the current value of the IN register */
    InputRegister in = ( InputRegister )0;

    /* For each bit set in the 'out' register, OR the corresponding IN register
       value into 'in'
    */
    for ( int bit = 0; bit < OUT_BITS; bit++ ) {
        if ( out & 0x01 )
            in |= cur_in[ bit ];
        out >>= 1;
    }

    return in;
}

/* .+

.title	      : KeybPress
.kind	      : C function
.creation     : 11-Feb-1998
.description  :
  This function tells to the keyboard emulator that key 'key' has been
  pressed. It updates the internal keyboard status information and, if
  necessary, posts an interrupt request to the CPU.

.call	      :
                KeybPress(key);
.input	      :
                const char *key, identifies the key that has been pressed.
.output	      :
                void
.status_codes :
                MOD_W_BAD_KEY
                MOD_W_BAD_OUT_BIT
.notes	      :
  1.1, 17-Feb-1998, creation
  2.1, 6-Sep-2000,
    deeply revised to accomodate the new GUI

.- */
void KeybPress( const char* key )
{
    if ( strcmp( key, "*" ) == 0 ) {
        /* This is the ON key */
        /* Set all 0x8000 lines */
        for ( int i = 0; i < OUT_BITS; i++ )
            cur_in[ i ] |= 0x8000;

        /* Post an interrupt request to the CPU */
        CpuIntRequest( INT_REQUEST_NMI );
    } else {
        unsigned int in_val, out_bit;

        if ( sscanf( key, "%x/%x", &out_bit, &in_val ) != 2 ) {
            CHF_Condition( MOD_CHF_MODULE_ID ) MOD_W_BAD_KEY, CHF_WARNING, key ChfEnd;
            ChfSignal( MOD_CHF_MODULE_ID );
            /* } else if ( out_bit < 0 || out_bit >= OUT_BITS ) { */
            /*     CHF_Condition( MOD_CHF_MODULE_ID ) MOD_W_BAD_OUT_BIT, CHF_WARNING, out_bit ChfEnd; */
            /*     ChfSignal( MOD_CHF_MODULE_ID ); */
        } else {
            /* Update the cur_in array */
            cur_in[ out_bit ] |= in_val;

            /* Post an interrupt request to the CPU */
            CpuIntRequest( INT_REQUEST_NMI );
        }
    }
}

/* .+

.title	      : KeybRelease
.kind	      : C function
.creation     : 11-Feb-1998
.description  :
  This function tells to the keyboard emulator that key 'key' has been
  released. It updates the internal keyboard status information.

.call	      :
                KeybRelease(key);
.input	      :
                const char *key, identifies the key that has been released.
.output	      :
                void
.status_codes :
                MOD_W_BAD_KEY
                MOD_W_BAD_OUT_BIT
.notes	      :
  1.1, 17-Feb-1998, creation
  2.1, 6-Sep-2000,
    deeply revised to accomodate the new GUI

.- */
void KeybRelease( const char* key )
{
    if ( strcmp( key, "*" ) == 0 ) {
        /* This is the ON key */
        /* Reset all 0x8000 lines */
        for ( int i = 0; i < OUT_BITS; i++ )
            cur_in[ i ] &= 0x7FFF;
    } else {
        unsigned int in_val, out_bit;

        if ( sscanf( key, "%x/%x", &out_bit, &in_val ) != 2 ) {
            CHF_Condition( MOD_CHF_MODULE_ID ) MOD_W_BAD_KEY, CHF_WARNING, key ChfEnd;
            ChfSignal( MOD_CHF_MODULE_ID );
            /* } else if ( out_bit < 0 || out_bit >= OUT_BITS ) { */
            /*     CHF_Condition( MOD_CHF_MODULE_ID ) MOD_W_BAD_OUT_BIT, CHF_WARNING, out_bit ChfEnd; */
            /*     ChfSignal( MOD_CHF_MODULE_ID ); */
        } else {
            /* Update the cur_in array */
            cur_in[ out_bit ] &= ~in_val;
        }
    }
}

/* .+

.title	      : KeybReset
.kind	      : C function
.creation     : 7-Nov-2000
.description  :
  This function resets the emulated keyboard; all keys are released.

.call	      :
                KeybReset();
.input	      :
                void
.output	      :
                void
.status_codes :
.notes	      :
  3.13, 7-Nov-2000, creation

.- */
void KeybReset( void )
{
    int i;

    /* Reset all 0x8000 lines */
    for ( i = 0; i < OUT_BITS; i++ )
        cur_in[ i ] = 0;
}
