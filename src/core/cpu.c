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

.creation     :	2-Feb-1998

.description  :
  This file executes the Saturn CPU opcodes. References:

    SASM.DOC by HP  (HORN disk 4)
    Guide to the Saturn Processor Rev. 0.00f by Matthew Mastracci
    entries.srt by Mika Heiskanen  (mheiskan@vipunen.hut.fi)
    x48 source code by Eddie C. Dost  (ecd@dressler.de)

.notes        :
  $Log: cpu.c,v $
  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.14  2000/11/13 10:30:04  cibrario
  Implemented fast load/save; improved keyboard interface emulation at
  high emulated CPU speed:

  - Added a delay loop in copy_in_to_(), when CPU_SLOW_IN is defined. the loop
    is implemented executing the same instruction multiple times and is
    needed because the HP firmware uses an active loop instead of a
    timer to determine the keyboard automatic repeat rate.

  - Changed initial value of cpu.inner_loop_max after a CPU reset,
    to be as documented (that is, maximum speed).

  - During CPU initialization, both shutdn and halt flags are now
    resetted.

  Revision 3.13  2000/11/09 11:23:12  cibrario
  Revised to add file selection box GUI element, CPU halt/run
  requests and emulator's extended functions:

  - Implemented CpuHaltRequest(), CpuRunRequest(), CpuHaltAllowed()

  Revision 3.10  2000/10/24 16:14:28  cibrario
  Added/Replaced GPL header

  Revision 3.5  2000/10/02 09:42:09  cibrario
  Linux support:
  - gcc does not like array subscripts with type 'char', and it is right.

  Revision 3.1  2000/09/20 13:39:18  cibrario
  Minor updates and fixes to avoid gcc compiler warnings on Solaris
  when -ansi -pedantic -Wall options are selected.

  Revision 1.2  2000/09/07  14:31:34  cibrario
  Bug fix: cpu.rstk_ptr and .reset_req were not reset; this gave
  troubles when attempting to override a corrupt status with CpuReset().

  Revision 1.1  1998/02/17  15:25:16  cibrario
  Initial revision
.- */

#include <stdio.h>
#include <string.h>

#include "../libChf/src/Chf.h"

#include "chf_wrapper.h"
#include "cpu.h"
#include "cpu_buscc.h"
#include "disassembler.h"
#include "disk_io.h" /* 3.1: ReadStructFromFile/WriteStructToFile */
#include "keyboard.h"
#include "modules.h"

/* 3.14: CPU_SLOW_IN
   Define this symbol (recommended) to slow down the A=IN and C=IN
   instructions depending on the current emulated CPU speed.

   The value of the macro determines the gain of the relation between
   CPU speed and slow down ratio.
*/
#define CPU_SLOW_IN 16

/*---------------------------------------------------------------------------
        Global variables
  ---------------------------------------------------------------------------*/

int opcode;

Cpu cpu;

/*---------------------------------------------------------------------------
        Private variables
  ---------------------------------------------------------------------------*/

/* Field selector indexes, lo/hi nibble.
   NOTE: The P and WP elements of the array must be dynamically adjusted
         since they depend on the current value of the P CPU register
*/
static const int fs_idx_lo[ N_FS ] =
    /*	P,  WP,  XS,   X,   S,   M,   B,    W
            ??,  ??,  ??,  ??,  ??,  ??,  ??,   A
    */
    { 0, 0, 2, 0, 15, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static const int fs_idx_hi[ N_FS ] =
    /*	 P,  WP,  XS,   X,   S,   M,   B,   W
            ??,  ??,  ??,  ??,  ??,  ??,  ??,   A
    */
    { 0, 0, 2, 2, 15, 14, 1, 15, 0, 0, 0, 0, 0, 0, 0, 4 };

/* Register Pair pointers */
static Nibble* const reg_pair_0[] =
    /*	AB,		BC,		CA,		DC		*/
    { cpu.reg[ A ], cpu.reg[ B ], cpu.reg[ C ], cpu.reg[ D ] };

static Nibble* const reg_pair_1[] =
    /*	AB,		BC,		CA,		DC		*/
    { cpu.reg[ B ], cpu.reg[ C ], cpu.reg[ A ], cpu.reg[ C ] };

/* Nibble bit masks */
static const Nibble nibble_bit_mask[] = { 0x1, 0x2, 0x4, 0x8 };

/* ProgramStatusRegister bit masks */
static const ProgramStatusRegister st_bit_mask[] = { 0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080,
                                                     0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000, 0x8000 };

/* Decimal sum/carry tables, range 0..31 */
static const int dec_sum[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1 };

static const int dec_carry[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

/* Decimal sub/borrow tables, range -10..15 */
static const int dec_sub_t[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5 };

static const int dec_borrow_t[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static const int* const dec_sub = dec_sub_t + 10;
static const int* const dec_borrow = dec_borrow_t + 10;

/* Decimal one's complement table */
static const int dec_one_c[] = { 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0 };

/*---------------------------------------------------------------------------
        Private functions: return stack handling
  ---------------------------------------------------------------------------*/
static void rstk_push( const Address r )
{
    cpu.rstk[ cpu.rstk_ptr ] = r;
    cpu.rstk_ptr = ( cpu.rstk_ptr + 1 ) & RSTK_PTR_MASK;
}

static Address rstk_pop( void )
{
    cpu.rstk_ptr = ( cpu.rstk_ptr - 1 ) & RSTK_PTR_MASK;

    Address r = cpu.rstk[ cpu.rstk_ptr ];

    cpu.rstk[ cpu.rstk_ptr ] = ( Address )0;

    return r;
}

/*---------------------------------------------------------------------------
        Private functions: bus input/output
  ---------------------------------------------------------------------------*/

/* IN */
static void copy_in_to_( Nibble* r )
{
    /* In */
#ifdef CPU_SLOW_IN
    /* We must slow the A=IN and C=IN instruction down a bit, depending
       on the emulated CPU speed.  This is necessary because the HP firmware
       uses an active loop instead of a timer to determine the keyboard
       automatic repeat rate.

       Since implementing a precise, tiny (~ 1 microsecond), passive delay
       in unix is almost impossible, we chose to execute the same
       instruction (A=IN or C=IN) multiple times by artificially resetting
       the PC as appropriate.

       The number of repetions depends linearly, with gain CPU_SLOW_IN,
       from the current value of cpu.inner_loop:
       cpu.inner_loop==INNER_LOOP_MAX corresponds to the nominal
       CPU speed of 4MHz and to a repetition rate of 1 (instructions are
       executed once as usual).
    */
    static int count_down = 0;

    /* Decrement counter; set PC back and return immediately if counter
       was not zero (counter not expired yet).
    */
    if ( count_down-- != 0 ) {
        cpu.pc -= 3;
        return;
    }

    /* Counter expired; reset counter and execute the instruction */
    count_down = ( ( cpu.inner_loop + ( INNER_LOOP_MAX / 2 ) ) / INNER_LOOP_MAX ) * CPU_SLOW_IN;
#endif
    cpu.in = KeybIN( cpu.out );

    r[ 0 ] = ( Nibble )( cpu.in & NIBBLE_MASK );
    r[ 1 ] = ( Nibble )( ( cpu.in ) >> 4 & NIBBLE_MASK );
    r[ 2 ] = ( Nibble )( ( cpu.in ) >> 8 & NIBBLE_MASK );
    r[ 3 ] = ( Nibble )( ( cpu.in ) >> 12 & NIBBLE_MASK );
}

/*---------------------------------------------------------------------------
        Private functions: CPU control
  ---------------------------------------------------------------------------*/

static void op807( void ) /* SHUTDN */
{
    /* Set shutdown flag */
    cpu.shutdn = true;

    /* the CPU module implements SHUTDN signalling the condition CPU_I_SHUTDN */
    SIGNAL0( CPU_CHF_MODULE_ID, CPU_I_SHUTDN, CHF_INFO )
}

/*---------------------------------------------------------------------------
        Private functions: data type conversions
  ---------------------------------------------------------------------------*/

/* Copies the A field of a DataRegister into an Address; this is not a
   loop to achieve greater execution speed.
*/
static Address R2Addr( const Nibble* r )
{
    return ( ( ( Address )r[ 0 ] ) | ( ( Address )r[ 1 ] << 4 ) | ( ( Address )r[ 2 ] << 8 ) | ( ( Address )r[ 3 ] << 12 ) |
             ( ( Address )r[ 4 ] << 16 ) );
}

/* Returns the nibs 0-3 of a DataRegister into an Address; this is not a
   loop to achieve greater execution speed.
*/
static Address R2AddrS( const Nibble* r )
{
    return ( ( ( Address )r[ 0 ] ) | ( ( Address )r[ 1 ] << 4 ) | ( ( Address )r[ 2 ] << 8 ) | ( ( Address )r[ 3 ] << 12 ) );
}

/* Copies an Address into the A field of a register; this is not a loop
   to achieve grater execution speed
*/
static void Addr2R( Nibble* d, Address a )
{
    d[ 0 ] = ( Nibble )( a & NIBBLE_MASK );
    a >>= 4;
    d[ 1 ] = ( Nibble )( a & NIBBLE_MASK );
    a >>= 4;
    d[ 2 ] = ( Nibble )( a & NIBBLE_MASK );
    a >>= 4;
    d[ 3 ] = ( Nibble )( a & NIBBLE_MASK );
    a >>= 4;
    d[ 4 ] = ( Nibble )( a & NIBBLE_MASK );
}

/* Copies an Address into nibs 0-3 of a register; this is not a loop
   to achieve grater execution speed
*/
static void Addr2RS( Nibble* d, Address a )
{
    d[ 0 ] = ( Nibble )( a & NIBBLE_MASK );
    a >>= 4;
    d[ 1 ] = ( Nibble )( a & NIBBLE_MASK );
    a >>= 4;
    d[ 2 ] = ( Nibble )( a & NIBBLE_MASK );
    a >>= 4;
    d[ 3 ] = ( Nibble )( a & NIBBLE_MASK );
}

/*---------------------------------------------------------------------------
        Private functions: data memory read/write
  ---------------------------------------------------------------------------*/

/* Read a field of a DataRegister from memory */
static void bus_read( Nibble* d, Address s, int fs )
{
    register int lo = cpu.fs_idx_lo[ fs ];
    register int hi = cpu.fs_idx_hi[ fs ];

    for ( register int n = lo; n <= hi; n++ ) {
        d[ n ] = ReadNibble( s );
        s++;
    }
}

/* Read a field of a DataRegister from memory, with immediate fs */
static void bus_read_immediate( Nibble* d, Address s, int imm_fs )
{
    for ( register int n = 0; n <= imm_fs; n++ )
        d[ n ] = ReadNibble( s++ );
}

/* Write a field of a DataRegister into memory */
static void bus_write( Address d, const Nibble* r, int fs )
{
    register int lo = cpu.fs_idx_lo[ fs ];
    register int hi = cpu.fs_idx_hi[ fs ];

    for ( register int n = lo; n <= hi; n++ ) {
        WriteNibble( d, r[ n ] );
        d++;
    }
}

/* Write a field of a DataRegister into memory, with immediate fs */
static void bus_write_immediate( Address d, const Nibble* r, int imm_fs )
{
    for ( register int n = 0; n <= imm_fs; n++ ) {
        WriteNibble( d, r[ n ] );
        d++;
    }
}

/*---------------------------------------------------------------------------
        Private functions: instruction fetch/immediate register load
  ---------------------------------------------------------------------------*/

/* Read two nibbles in two-complement form, starting from pc */
static Address Get2Nibbles2C( Address pc )
{
    Address v = ( Address )FetchNibble( pc ) | ( ( Address )FetchNibble( pc + 1 ) << 4 );

    return ( v & 0x80 ) ? v - 0x100 : v;
}

/* Read three nibbles in two-complement form, starting from pc */
static Address Get3Nibbles2C( Address pc )
{
    Address v = ( Address )FetchNibble( pc ) | ( ( Address )FetchNibble( pc + 1 ) << 4 ) | ( ( Address )FetchNibble( pc + 2 ) << 8 );

    return ( v & 0x800 ) ? v - 0x1000 : v;
}

/* Read four nibbles in two-complement form, starting from pc */
static Address Get4Nibbles2C( Address pc )
{
    Address v = ( Address )FetchNibble( pc ) | ( ( Address )FetchNibble( pc + 1 ) << 4 ) | ( ( Address )FetchNibble( pc + 2 ) << 8 ) |
                ( ( Address )FetchNibble( pc + 3 ) << 12 );

    return ( v & 0x8000 ) ? v - 0x10000 : v;
}

/* Read four nibbles in absolute form, starting from pc */
static Address Get5NibblesAbs( Address pc )
{
    Address v = ( Address )FetchNibble( pc ) | ( ( Address )FetchNibble( pc + 1 ) << 4 ) | ( ( Address )FetchNibble( pc + 2 ) << 8 ) |
                ( ( Address )FetchNibble( pc + 3 ) << 12 ) | ( ( Address )FetchNibble( pc + 4 ) << 16 );

    return v;
}

/* Fetch the lower 'n' nibbles of the D register pointed by 'd' from the
   current instruction body
*/
static void FetchD( Address* d, register int n )
{
    register Address mask = ADDRESS_MASK;
    register Address v = 0x00000;
    register int shift = 0;

    for ( register int i = 0; i < n; i++ ) {
        v |= ( ( Address )FetchNibble( cpu.pc ) << shift );
        cpu.pc++;
        mask <<= 4;
        shift += 4;
    }

    *d = ( *d & mask ) | v;
}

/* Fetch 'n'+1 nibbles of the DataRegister r from the current instruction body,
   starting from the nibble pointed by the P register.
*/
static void FetchR( Nibble* r, register int n )
{
    register int p = ( int )cpu.p;

    for ( register int i = 0; i <= n; i++ ) {
        r[ p ] = FetchNibble( cpu.pc );
        p++;
        cpu.pc++;
        if ( p >= NIBBLE_PER_REGISTER )
            p = 0;
    }
}

/*---------------------------------------------------------------------------
        Private functions: P register setting
  ---------------------------------------------------------------------------*/
static void SetP( Nibble n )
{
    cpu.p = n;

    cpu.fs_idx_lo[ FS_P ] = n;
    cpu.fs_idx_hi[ FS_P ] = n;
    cpu.fs_idx_hi[ FS_WP ] = n;
}

/*---------------------------------------------------------------------------
        Private functions: DataRegister tests
  ---------------------------------------------------------------------------*/

/* ?r=s */
static void TestRREq( int rp, int fs )
{
    register const Nibble* const r = reg_pair_0[ rp ];
    register const Nibble* const s = reg_pair_1[ rp ];
    register int lo = cpu.fs_idx_lo[ fs ];
    register int hi = cpu.fs_idx_hi[ fs ];

    for ( register int n = lo; n <= hi; n++ )
        if ( r[ n ] != s[ n ] ) {
            cpu.carry = false;
            return;
        };

    cpu.carry = true;
}

/* ?r=0 */
static void TestRZ( int rp, int fs )
{
    register const Nibble* const r = reg_pair_0[ rp ];
    register int lo = cpu.fs_idx_lo[ fs ];
    register int hi = cpu.fs_idx_hi[ fs ];

    for ( register int n = lo; n <= hi; n++ )
        if ( r[ n ] != ( Nibble )0 ) {
            cpu.carry = false;
            return;
        };

    cpu.carry = true;
}

/* ?r#s */
static void TestRRNe( int rp, int fs )
{
    register const Nibble* const r = reg_pair_0[ rp ];
    register const Nibble* const s = reg_pair_1[ rp ];
    register int lo = cpu.fs_idx_lo[ fs ];
    register int hi = cpu.fs_idx_hi[ fs ];

    for ( register int n = lo; n <= hi; n++ )
        if ( r[ n ] != s[ n ] ) {
            cpu.carry = true;
            return;
        };

    cpu.carry = false;
}

/* ?r#0 */
static void TestRNZ( int rp, int fs )
{
    register const Nibble* const r = reg_pair_0[ rp ];
    register int lo = cpu.fs_idx_lo[ fs ];
    register int hi = cpu.fs_idx_hi[ fs ];

    for ( register int n = lo; n <= hi; n++ )
        if ( r[ n ] != ( Nibble )0 ) {
            cpu.carry = true;
            return;
        };

    cpu.carry = false;
}

/* ?r>s */
static void TestRRGt( int rp, int fs )
{
    register const Nibble* const r = reg_pair_0[ rp ];
    register const Nibble* const s = reg_pair_1[ rp ];
    register int lo = cpu.fs_idx_lo[ fs ];
    register int hi = cpu.fs_idx_hi[ fs ];

    for ( register int n = hi; n >= lo; n-- ) {
        if ( r[ n ] > s[ n ] ) {
            cpu.carry = true;
            return;
        };
        if ( r[ n ] < s[ n ] ) {
            cpu.carry = false;
            return;
        };
    }

    cpu.carry = false;
}

/* ?r>=s */
static void TestRRGe( int rp, int fs )
{
    register const Nibble* const r = reg_pair_0[ rp ];
    register const Nibble* const s = reg_pair_1[ rp ];
    register int lo = cpu.fs_idx_lo[ fs ];
    register int hi = cpu.fs_idx_hi[ fs ];

    for ( register int n = hi; n >= lo; n-- ) {
        if ( r[ n ] > s[ n ] ) {
            cpu.carry = true;
            return;
        };
        if ( r[ n ] < s[ n ] ) {
            cpu.carry = false;
            return;
        };
    }

    cpu.carry = true;
}

/* ?r<s */
static void TestRRLt( int rp, int fs )
{
    register const Nibble* const r = reg_pair_0[ rp ];
    register const Nibble* const s = reg_pair_1[ rp ];
    register int lo = cpu.fs_idx_lo[ fs ];
    register int hi = cpu.fs_idx_hi[ fs ];

    for ( register int n = hi; n >= lo; n-- ) {
        if ( r[ n ] < s[ n ] ) {
            cpu.carry = true;
            return;
        };
        if ( r[ n ] > s[ n ] ) {
            cpu.carry = false;
            return;
        };
    }

    cpu.carry = false;
}

/* ?r<=s */
static void TestRRLe( int rp, int fs )
{
    register const Nibble* const r = reg_pair_0[ rp ];
    register const Nibble* const s = reg_pair_1[ rp ];
    register int lo = cpu.fs_idx_lo[ fs ];
    register int hi = cpu.fs_idx_hi[ fs ];

    for ( register int n = hi; n >= lo; n-- ) {
        if ( r[ n ] < s[ n ] ) {
            cpu.carry = true;
            return;
        };
        if ( r[ n ] > s[ n ] ) {
            cpu.carry = false;
            return;
        };
    }

    cpu.carry = true;
}

/*---------------------------------------------------------------------------
        Private functions: DataRegister operations
  ---------------------------------------------------------------------------*/

/* r=r+r */
static void AddRR( register Nibble* d, register const Nibble* a, register const Nibble* b, int fs )
{
    register int carry = 0;
    register int lo = cpu.fs_idx_lo[ fs ];
    register int hi = cpu.fs_idx_hi[ fs ];
    register int s;

    for ( register int n = lo; n <= hi; n++ ) {
        s = a[ n ] + b[ n ] + carry;

        if ( cpu.hexmode ) {
            d[ n ] = ( Nibble )( s & NIBBLE_MASK );
            carry = ( ( s & ~NIBBLE_MASK ) != 0 );
        } else {
            d[ n ] = dec_sum[ s ];
            carry = dec_carry[ s ];
        }
    }
    cpu.carry = ( bool )carry;
}

/* r=r+1 */
static void IncrR( register Nibble* d, int fs )
{
    register int carry = 1;
    register int lo = cpu.fs_idx_lo[ fs ];
    register int hi = cpu.fs_idx_hi[ fs ];
    register int s;

    for ( register int n = lo; n <= hi; n++ ) {
        s = d[ n ] + carry;

        if ( cpu.hexmode ) {
            d[ n ] = ( Nibble )( s & NIBBLE_MASK );
            carry = ( ( s & ~NIBBLE_MASK ) != 0 );
        } else {
            d[ n ] = dec_sum[ s ];
            carry = dec_carry[ s ];
        }
    }

    cpu.carry = ( bool )carry;
}

/* r=r-r */
static void SubRR( register Nibble* d, register Nibble* a, register Nibble* b, int fs )
{
    register int carry = 0;
    register int lo = cpu.fs_idx_lo[ fs ];
    register int hi = cpu.fs_idx_hi[ fs ];
    register int s;

    for ( register int n = lo; n <= hi; n++ ) {
        s = a[ n ] - b[ n ] - carry;

        if ( cpu.hexmode ) {
            d[ n ] = ( Nibble )( s & NIBBLE_MASK );
            carry = ( ( s & ~NIBBLE_MASK ) != 0 );
        } else {
            d[ n ] = dec_sub[ s ];
            carry = dec_borrow[ s ];
        }
    }

    cpu.carry = ( bool )carry;
}

/* r=r-1 */
static void DecrR( register Nibble* d, int fs )
{
    register int carry = 1;
    register int lo = cpu.fs_idx_lo[ fs ];
    register int hi = cpu.fs_idx_hi[ fs ];
    register int s;

    for ( register int n = lo; n <= hi; n++ ) {
        s = d[ n ] - carry;

        if ( cpu.hexmode ) {
            d[ n ] = ( Nibble )( s & NIBBLE_MASK );
            carry = ( ( s & ~NIBBLE_MASK ) != 0 );
        } else {
            d[ n ] = dec_sub[ s ];
            carry = dec_borrow[ s ];
        }
    }

    cpu.carry = ( bool )carry;
}

/* r=0 */
static void ClearR( register Nibble* d, int fs )
{
    register int lo = cpu.fs_idx_lo[ fs ];
    register int hi = cpu.fs_idx_hi[ fs ];

    for ( register int n = lo; n <= hi; n++ )
        d[ n ] = ( Nibble )0;
}

/* r=r */
static void CopyRR( register Nibble* d, register Nibble* s, int fs )
{
    register int lo = cpu.fs_idx_lo[ fs ];
    register int hi = cpu.fs_idx_hi[ fs ];

    for ( register int n = lo; n <= hi; n++ )
        d[ n ] = s[ n ];
}

/* rrEX */
static void ExchRR( register Nibble* d, register Nibble* s, int fs )
{
    register int lo = cpu.fs_idx_lo[ fs ];
    register int hi = cpu.fs_idx_hi[ fs ];
    register Nibble t;

    for ( register int n = lo; n <= hi; n++ ) {
        t = d[ n ];
        d[ n ] = s[ n ];
        s[ n ] = t;
    }
}

/* rSL */
static void ShiftLeftR( register Nibble* d, int fs )
{
    register int lo = cpu.fs_idx_lo[ fs ];
    register int hi = cpu.fs_idx_hi[ fs ];

    for ( register int n = hi; n > lo; n-- )
        d[ n ] = d[ n - 1 ];

    d[ lo ] = ( Nibble )0;
}

/* rSR */
static void ShiftRightR( register Nibble* d, int fs )
{
    register int lo = cpu.fs_idx_lo[ fs ];
    register int hi = cpu.fs_idx_hi[ fs ];

    if ( d[ lo ] != ( Nibble )0 )
        cpu.hst |= HST_SB_MASK;

    for ( register int n = lo; n < hi; n++ )
        d[ n ] = d[ n + 1 ];

    d[ hi ] = ( Nibble )0;
}

/* rSRB */
static void ShiftRightBitR( register Nibble* d, int fs )
{
    register int lo = cpu.fs_idx_lo[ fs ];
    register int hi = cpu.fs_idx_hi[ fs ];

    if ( ( d[ lo ] & nibble_bit_mask[ 0 ] ) != ( Nibble )0 )
        cpu.hst |= HST_SB_MASK;

    for ( register int n = lo; n < hi; n++ ) {
        d[ n ] >>= 1;
        d[ n ] |= ( ( d[ n + 1 ] & nibble_bit_mask[ 0 ] ) ? nibble_bit_mask[ 3 ] : 0 );
    }

    d[ hi ] >>= 1;
}

/* rSLC */
static void ShiftLeftCircR( register Nibble* d, int fs )
{
    register int lo = cpu.fs_idx_lo[ fs ];
    register int hi = cpu.fs_idx_hi[ fs ];
    register Nibble s = d[ hi ];

    for ( register int n = hi; n > lo; n-- )
        d[ n ] = d[ n - 1 ];

    d[ lo ] = s;
}

/* rSRC */
static void ShiftRightCircR( register Nibble* d, int fs )
{
    register int lo = cpu.fs_idx_lo[ fs ];
    register int hi = cpu.fs_idx_hi[ fs ];
    register Nibble s;

    if ( ( s = d[ lo ] ) != ( Nibble )0 )
        cpu.hst |= HST_SB_MASK;

    for ( register int n = lo; n < hi; n++ )
        d[ n ] = d[ n + 1 ];

    d[ hi ] = s;
}

/* r=-r */
static void TwoComplR( register Nibble* d, int fs )
{
    register int carry = 0;
    register int lo = cpu.fs_idx_lo[ fs ];
    register int hi = cpu.fs_idx_hi[ fs ];
    register int s;
    register int nz = 0;

    for ( register int n = lo; n <= hi; n++ ) {
        s = -d[ n ] - carry;

        if ( cpu.hexmode ) {
            d[ n ] = ( Nibble )( s & NIBBLE_MASK );
            carry = ( ( s & ~NIBBLE_MASK ) != 0 );
        } else {
            d[ n ] = dec_sub[ s ];
            carry = dec_borrow[ s ];
        }

        nz = nz || ( d[ n ] != ( Nibble )0 );
    }

    cpu.carry = nz;
}

/* r=-r-1 */
static void OneComplR( register Nibble* d, int fs )
{
    register int lo = cpu.fs_idx_lo[ fs ];
    register int hi = cpu.fs_idx_hi[ fs ];

    for ( register int n = lo; n <= hi; n++ ) {
        if ( cpu.hexmode )
            d[ n ] = ( 0xF - d[ n ] ) & NIBBLE_MASK;
        else
            d[ n ] = dec_one_c[ ( int )d[ n ] ];
    }

    cpu.carry = false;
}

/* r=r&r */
static void AndRR( register Nibble* d, register const Nibble* a, register const Nibble* b, int fs )
{
    register int lo = cpu.fs_idx_lo[ fs ];
    register int hi = cpu.fs_idx_hi[ fs ];

    for ( register int n = lo; n <= hi; n++ )
        d[ n ] = a[ n ] & b[ n ];
}

/* r=r!r */
static void OrRR( register Nibble* d, register const Nibble* a, register const Nibble* b, int fs )
{
    register int lo = cpu.fs_idx_lo[ fs ];
    register int hi = cpu.fs_idx_hi[ fs ];

    for ( register int n = lo; n <= hi; n++ )
        d[ n ] = a[ n ] | b[ n ];
}

/* Add immediate value 'v'+1 to the DataRegister 'r', Field Selector 'fs',
   always HEX mode
*/
static void AddRImm( Nibble* r, int fs, Nibble v )
{
    register int carry = ( int )v + 1;
    register int lo = cpu.fs_idx_lo[ fs ];
    register int hi = cpu.fs_idx_hi[ fs ];
    register int s;

    for ( register int n = lo; n <= hi; n++ ) {
        s = r[ n ] + carry;

        r[ n ] = ( Nibble )( s & NIBBLE_MASK );
        carry = ( ( s & ~NIBBLE_MASK ) != 0 );
    }

    cpu.carry = ( bool )carry;
}

/* Subtract immediate value 'v'+1 from the DataRegister 'r',
   Field Selector 'fs', always HEX mode
*/
static void SubRImm( register Nibble* r, int fs, Nibble v )
{
    register int carry = ( int )v + 1;
    register int lo = cpu.fs_idx_lo[ fs ];
    register int hi = cpu.fs_idx_hi[ fs ];
    register int s;

    for ( register int n = lo; n <= hi; n++ ) {
        s = r[ n ] - carry;

        r[ n ] = ( Nibble )( s & NIBBLE_MASK );
        carry = ( ( s & ~NIBBLE_MASK ) != 0 );
    }

    cpu.carry = ( bool )carry;
}

/*---------------------------------------------------------------------------
        Private functions: DataRegister bit operations
  ---------------------------------------------------------------------------*/
static void ExecBIT0( Nibble* r, Nibble n ) { r[ n / 4 ] &= ~nibble_bit_mask[ n % 4 ]; }

static void ExecBIT1( Nibble* r, Nibble n ) { r[ n / 4 ] |= nibble_bit_mask[ n % 4 ]; }

static void TestBIT0( Nibble* r, Nibble n ) { cpu.carry = ( ( r[ n / 4 ] & nibble_bit_mask[ n % 4 ] ) == 0 ); }

static void TestBIT1( Nibble* r, Nibble n ) { cpu.carry = ( ( r[ n / 4 ] & nibble_bit_mask[ n % 4 ] ) != 0 ); }

/*---------------------------------------------------------------------------
        Private functions: jumps/subroutine calls
  ---------------------------------------------------------------------------*/

/* GOYES/RTNYES */
static void ExecGOYES_RTNYES( void )
{
    if ( cpu.carry ) {
        /* Taken */
        Address offset = Get2Nibbles2C( cpu.pc );

        if ( offset == 0 )
            /* RTNYES */
            cpu.pc = rstk_pop();
        else
            cpu.pc += offset;
    } else
        /* Not taken */
        cpu.pc += 2;
}

/*---------------------------------------------------------------------------
        Private functions: instruction stream decoding
  ---------------------------------------------------------------------------*/

/* ?..., GOYES/RTNYES, Test with Field Selector, opcode 9ftyy, length 5 */
static void ExecTest_9( void )
{
    Nibble f = FetchNibble( cpu.pc );
    cpu.pc++;
    opcode *= 0x10;
    opcode += f;
    Nibble t = FetchNibble( cpu.pc );
    cpu.pc++;
    opcode *= 0x10;
    opcode += t;
    int fs = GET_FS( f );
    int tc = GET_OC_2( f, t );
    int rp = GET_RP( t );

    /* Decode test code */
    switch ( tc ) {
        case 0x0:
            TestRREq( rp, fs );
            break;
        case 0x1:
            TestRRNe( rp, fs );
            break;
        case 0x2:
            TestRZ( rp, fs );
            break;
        case 0x3:
            TestRNZ( rp, fs );
            break;
        case 0x4:
            TestRRGt( rp, fs );
            break;
        case 0x5:
            TestRRLt( rp, fs );
            break;
        case 0x6:
            TestRRGe( rp, fs );
            break;
        case 0x7:
            TestRRLe( rp, fs );
            break;

        default:
            FATAL( CPU_CHF_MODULE_ID, CPU_F_INTERR, "Bad_Test_Code" )
            break;
    }

    /* Execute GOYES/RTNYES */
    ExecGOYES_RTNYES();
}

/* ?..., GOYES/RTNYES, Test on A Fields, opcode 8Atyy, length 5 */
static void ExecTest_8A( void )
{
    Nibble t = FetchNibble( cpu.pc );
    cpu.pc++;
    opcode *= 0x10;
    opcode += t;
    int tc = GET_OC_1( t );
    int rp = GET_RP( t );

    /* Decode test code */
    switch ( tc ) {
        case 0x0:
            TestRREq( rp, FS_A );
            break;
        case 0x1:
            TestRRNe( rp, FS_A );
            break;
        case 0x2:
            TestRZ( rp, FS_A );
            break;
        case 0x3:
            TestRNZ( rp, FS_A );
            break;

        default:
            FATAL( CPU_CHF_MODULE_ID, CPU_F_INTERR, "Bad_Test_Code" )
            break;
    }

    /* Execute GOYES/RTNYES */
    ExecGOYES_RTNYES();
}

/* ?..., GOYES/RTNYES, Test on A Fields, opcode 8Btyy, length 5 */
static void ExecTest_8B( void )
{
    Nibble t = FetchNibble( cpu.pc );
    cpu.pc++;
    opcode *= 0x10;
    opcode += t;
    int tc = GET_OC_1( t );
    int rp = GET_RP( t );

    /* Decode test code */
    switch ( tc ) {
        case 0x0:
            TestRRGt( rp, FS_A );
            break;
        case 0x1:
            TestRRLt( rp, FS_A );
            break;
        case 0x2:
            TestRRGe( rp, FS_A );
            break;
        case 0x3:
            TestRRLe( rp, FS_A );
            break;

        default:
            FATAL( CPU_CHF_MODULE_ID, CPU_F_INTERR, "Bad_Test_Code" )
            break;
    }

    /* Execute GOYES/RTNYES */
    ExecGOYES_RTNYES();
}

/* ..., Register Operation with Field Selector, opcode Afo, length 3 */
static void ExecRegOp_A( void )
{
    Nibble f = FetchNibble( cpu.pc );
    cpu.pc++;
    opcode *= 0x10;
    opcode += f;
    Nibble o = FetchNibble( cpu.pc );
    cpu.pc++;
    opcode *= 0x10;
    opcode += o;
    int fs = GET_FS( f );
    int oc = GET_OC_2( f, o );
    int rp = GET_RP( o );

    /* Decode operation code */
    switch ( oc ) {
        case 0x0:
            AddRR( reg_pair_0[ rp ], reg_pair_0[ rp ], reg_pair_1[ rp ], fs );
            break;
        case 0x1:
            AddRR( reg_pair_0[ rp ], reg_pair_0[ rp ], reg_pair_0[ rp ], fs );
            break;
        case 0x2:
            AddRR( reg_pair_1[ rp ], reg_pair_1[ rp ], reg_pair_0[ rp ], fs );
            break;
        case 0x3:
            DecrR( reg_pair_0[ rp ], fs );
            break;
        case 0x4:
            ClearR( reg_pair_0[ rp ], fs );
            break;
        case 0x5:
            CopyRR( reg_pair_0[ rp ], reg_pair_1[ rp ], fs );
            break;
        case 0x6:
            CopyRR( reg_pair_1[ rp ], reg_pair_0[ rp ], fs );
            break;
        case 0x7:
            ExchRR( reg_pair_0[ rp ], reg_pair_1[ rp ], fs );
            break;

        default:
            FATAL( CPU_CHF_MODULE_ID, CPU_F_INTERR, "Bad_Operation_Code" )
            break;
    }
}

/* ..., Register Operation with Field Selector, opcode Bfo, length 3 */
static void ExecRegOp_B( void )
{
    Nibble f = FetchNibble( cpu.pc );
    cpu.pc++;
    opcode *= 0x10;
    opcode += f;
    Nibble o = FetchNibble( cpu.pc );
    cpu.pc++;
    opcode *= 0x10;
    opcode += o;
    int fs = GET_FS( f );
    int oc = GET_OC_2( f, o );
    int rp = GET_RP( o );

    /* Decode operation code */
    switch ( oc ) {
        case 0x0:
            SubRR( reg_pair_0[ rp ], reg_pair_0[ rp ], reg_pair_1[ rp ], fs );
            break;
        case 0x1:
            IncrR( reg_pair_0[ rp ], fs );
            break;
        case 0x2:
            SubRR( reg_pair_1[ rp ], reg_pair_1[ rp ], reg_pair_0[ rp ], fs );
            break;
        case 0x3:
            SubRR( reg_pair_0[ rp ], reg_pair_1[ rp ], reg_pair_0[ rp ], fs );
            break;
        case 0x4:
            ShiftLeftR( reg_pair_0[ rp ], fs );
            break;
        case 0x5:
            ShiftRightR( reg_pair_0[ rp ], fs );
            break;
        case 0x6:
            TwoComplR( reg_pair_0[ rp ], fs );
            break;
        case 0x7:
            OneComplR( reg_pair_0[ rp ], fs );
            break;

        default:
            FATAL( CPU_CHF_MODULE_ID, CPU_F_INTERR, "Bad_Operation_Code" )
            break;
    }
}

/* ..., Register Operation on A Fields, opcode Co, length 2 */
static void ExecRegOp_C( void )
{
    Nibble o = FetchNibble( cpu.pc );
    cpu.pc++;
    opcode *= 0x10;
    opcode += o;
    int oc = GET_OC_1( o );
    int rp = GET_RP( o );

    /* Decode operation code */
    switch ( oc ) {
        case 0x0:
            AddRR( reg_pair_0[ rp ], reg_pair_0[ rp ], reg_pair_1[ rp ], FS_A );
            break;
        case 0x1:
            AddRR( reg_pair_0[ rp ], reg_pair_0[ rp ], reg_pair_0[ rp ], FS_A );
            break;
        case 0x2:
            AddRR( reg_pair_1[ rp ], reg_pair_1[ rp ], reg_pair_0[ rp ], FS_A );
            break;
        case 0x3:
            DecrR( reg_pair_0[ rp ], FS_A );
            break;

        default:
            FATAL( CPU_CHF_MODULE_ID, CPU_F_INTERR, "Bad_Operation_Code" )
            break;
    }
}

/* ..., Register Operation on A Fields, opcode Do, length 2 */
static void ExecRegOp_D( void )
{
    Nibble o = FetchNibble( cpu.pc );
    cpu.pc++;
    opcode *= 0x10;
    opcode += o;
    int oc = GET_OC_1( o );
    int rp = GET_RP( o );

    /* Decode operation code */
    switch ( oc ) {
        case 0x0:
            ClearR( reg_pair_0[ rp ], FS_A );
            break;
        case 0x1:
            CopyRR( reg_pair_0[ rp ], reg_pair_1[ rp ], FS_A );
            break;
        case 0x2:
            CopyRR( reg_pair_1[ rp ], reg_pair_0[ rp ], FS_A );
            break;
        case 0x3:
            ExchRR( reg_pair_0[ rp ], reg_pair_1[ rp ], FS_A );
            break;

        default:
            FATAL( CPU_CHF_MODULE_ID, CPU_F_INTERR, "Bad_Operation_Code" )
            break;
    }
}

/* ..., Register Operation on A Fields, opcode Eo, length 2 */
static void ExecRegOp_E( void )
{
    Nibble o = FetchNibble( cpu.pc );
    cpu.pc++;
    opcode *= 0x10;
    opcode += o;
    int oc = GET_OC_1( o );
    int rp = GET_RP( o );

    /* Decode operation code */
    switch ( oc ) {
        case 0x0:
            SubRR( reg_pair_0[ rp ], reg_pair_0[ rp ], reg_pair_1[ rp ], FS_A );
            break;
        case 0x1:
            IncrR( reg_pair_0[ rp ], FS_A );
            break;
        case 0x2:
            SubRR( reg_pair_1[ rp ], reg_pair_1[ rp ], reg_pair_0[ rp ], FS_A );
            break;
        case 0x3:
            SubRR( reg_pair_0[ rp ], reg_pair_1[ rp ], reg_pair_0[ rp ], FS_A );
            break;

        default:
            FATAL( CPU_CHF_MODULE_ID, CPU_F_INTERR, "Bad_Operation_Code" )
            break;
    }
}

/* ..., Register Operation on A Fields, opcode Fo, length 2 */
static void ExecRegOp_F( void )
{
    Nibble o = FetchNibble( cpu.pc );
    cpu.pc++;
    opcode *= 0x10;
    opcode += 0;
    int oc = GET_OC_1( o );
    int rp = GET_RP( o );

    /* Decode operation code */
    switch ( oc ) {
        case 0x0:
            ShiftLeftR( reg_pair_0[ rp ], FS_A );
            break;
        case 0x1:
            ShiftRightR( reg_pair_0[ rp ], FS_A );
            break;
        case 0x2:
            TwoComplR( reg_pair_0[ rp ], FS_A );
            break;
        case 0x3:
            OneComplR( reg_pair_0[ rp ], FS_A );
            break;

        default:
            FATAL( CPU_CHF_MODULE_ID, CPU_F_INTERR, "Bad_Operation_Code" )
            break;
    }
}

/* .&., .!., AND/OR Operations, opcode 0Efo, length 4 */
static void ExecAND_OR( void )
{
    Nibble f = FetchNibble( cpu.pc );
    cpu.pc++;
    opcode *= 0x10;
    opcode += f;
    Nibble o = FetchNibble( cpu.pc );
    cpu.pc++;
    opcode *= 0x10;
    opcode += o;
    int oc = GET_OC_1( o );
    int rp = GET_RP( o );

    /* Decode operation code */
    switch ( oc ) {
        case 0x0:
            AndRR( reg_pair_0[ rp ], reg_pair_0[ rp ], reg_pair_1[ rp ], f );
            break;
        case 0x1:
            AndRR( reg_pair_1[ rp ], reg_pair_1[ rp ], reg_pair_0[ rp ], f );
            break;
        case 0x2:
            OrRR( reg_pair_0[ rp ], reg_pair_0[ rp ], reg_pair_1[ rp ], f );
            break;
        case 0x3:
            OrRR( reg_pair_1[ rp ], reg_pair_1[ rp ], reg_pair_0[ rp ], f );
            break;

        default:
            FATAL( CPU_CHF_MODULE_ID, CPU_F_INTERR, "Bad_Operation_Code" )
            break;
    }
}

/* Instruction Group_0 */
static void ExecGroup_0( void )
{
    Nibble n = FetchNibble( cpu.pc );
    cpu.pc++;
    opcode *= 0x10;
    opcode += n;

    switch ( n ) {
        case 0x0: /* RTNSXM */
            cpu.hst |= HST_XM_MASK;
            cpu.pc = rstk_pop();
            break;
        case 0x1: /* RTN */
            cpu.pc = rstk_pop();
            break;
        case 0x2: /* RTNSC */
            cpu.carry = true;
            cpu.pc = rstk_pop();
            break;
        case 0x3: /* RTNCC */
            cpu.carry = false;
            cpu.pc = rstk_pop();
            break;
        case 0x4: /* SETHEX */
            cpu.hexmode = true;
            break;
        case 0x5: /* SETDEC */
            cpu.hexmode = false;
            break;
        case 0x6: /* RSTK=C */
            rstk_push( R2Addr( cpu.reg[ C ] ) );
            break;
        case 0x7: /* C=RSTK */
            Addr2R( cpu.reg[ C ], rstk_pop() );
            break;
        case 0x8: /* CLRST */
            cpu.st &= CLRST_MASK;
            break;
        case 0x9: /* C=ST */
            /* Copy the 12 low-order bits of ST into C */
            cpu.reg[ C ][ 0 ] = ( Nibble )( cpu.st & NIBBLE_MASK );
            cpu.reg[ C ][ 1 ] = ( Nibble )( ( cpu.st >> 4 ) & NIBBLE_MASK );
            cpu.reg[ C ][ 2 ] = ( Nibble )( ( cpu.st >> 8 ) & NIBBLE_MASK );
            break;
        case 0xA: /* ST=C */
            /* Copy the 12 low-order bits of C into ST */
            cpu.st = ( ProgramStatusRegister )cpu.reg[ C ][ 0 ] | ( ( ProgramStatusRegister )cpu.reg[ C ][ 1 ] << 4 ) |
                     ( ( ProgramStatusRegister )cpu.reg[ C ][ 2 ] << 8 ) | ( cpu.st & CLRST_MASK );
            break;
        case 0xB: /* CSTEX */
            /* Exchange the 12 low-order bits of C with the 12 low-order bits of ST */
            {
                ProgramStatusRegister tst = cpu.st;

                cpu.st = ( ProgramStatusRegister )cpu.reg[ C ][ 0 ] | ( ( ProgramStatusRegister )cpu.reg[ C ][ 1 ] << 4 ) |
                         ( ( ProgramStatusRegister )cpu.reg[ C ][ 2 ] << 8 ) | ( cpu.st & CLRST_MASK );

                cpu.reg[ C ][ 0 ] = ( Nibble )( tst & NIBBLE_MASK );
                cpu.reg[ C ][ 1 ] = ( Nibble )( ( tst >> 4 ) & NIBBLE_MASK );
                cpu.reg[ C ][ 2 ] = ( Nibble )( ( tst >> 8 ) & NIBBLE_MASK );
            }
            break;
        case 0xC: /* P=P+1 */
            if ( cpu.p == NIBBLE_MASK ) {
                SetP( 0 );
                cpu.carry = true;
            } else {
                SetP( cpu.p + 1 );
                cpu.carry = false;
            }
            break;
        case 0xD: /* P=P-1 */
            if ( cpu.p == ( Nibble )0 ) {
                SetP( NIBBLE_MASK );
                cpu.carry = true;
            } else {
                SetP( cpu.p - 1 );
                cpu.carry = false;
            }
            break;
        case 0xE: /* AND_OR */
            ExecAND_OR();
            break;
        case 0xF: /* RTI */
            if ( cpu.int_pending != INT_REQUEST_NONE ) {
                DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_INT, CPU_I_RTI_LOOP, ( cpu.int_pending == INT_REQUEST_NMI ? "NMI" : "IRQ" ) )

                /* Service immediately any pending interrupt request */
                cpu.int_service = true;
                cpu.int_pending = INT_REQUEST_NONE;
                cpu.pc = INT_HANDLER_PC;
            } else {
                /* Reenable interrupts and return */
                DEBUG0( CPU_CHF_MODULE_ID, DEBUG_C_INT, CPU_I_RTI_END )

                cpu.int_service = false;
                cpu.pc = rstk_pop();
            }
            break;

        default: /* Unknown opcode */
            ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, cpu.pc, n )
            break;
    }
}

/* Instruction Group_13 */
static void ExecGroup_13( void )
{
    Address ta;
    Nibble n = FetchNibble( cpu.pc );
    cpu.pc++;

    /* Copy/Exchange A/C and D0/D1 */
    switch ( n ) {
        case 0x0: /* D0=A */
            cpu.d[ 0 ] = R2Addr( cpu.reg[ A ] );
            break;
        case 0x1: /* D1=A */
            cpu.d[ 1 ] = R2Addr( cpu.reg[ A ] );
            break;
        case 0x2: /* AD0EX */
            ta = cpu.d[ 0 ];
            cpu.d[ 0 ] = R2Addr( cpu.reg[ A ] );
            Addr2R( cpu.reg[ A ], ta );
            break;
        case 0x3: /* AD1EX */
            ta = cpu.d[ 1 ];
            cpu.d[ 1 ] = R2Addr( cpu.reg[ A ] );
            Addr2R( cpu.reg[ A ], ta );
            break;
        case 0x4: /* D0=C */
            cpu.d[ 0 ] = R2Addr( cpu.reg[ C ] );
            break;
        case 0x5: /* D1=C */
            cpu.d[ 1 ] = R2Addr( cpu.reg[ C ] );
            break;
        case 0x6: /* CD0EX */
            ta = cpu.d[ 0 ];
            cpu.d[ 0 ] = R2Addr( cpu.reg[ C ] );
            Addr2R( cpu.reg[ C ], ta );
            break;
        case 0x7: /* CD1EX */
            ta = cpu.d[ 1 ];
            cpu.d[ 1 ] = R2Addr( cpu.reg[ C ] );
            Addr2R( cpu.reg[ C ], ta );
            break;
        case 0x8: /* D0=AS */
            cpu.d[ 0 ] = R2AddrS( cpu.reg[ A ] ) | ( cpu.d[ 0 ] & D_S_MASK );
            break;
        case 0x9: /* D1=AS */
            cpu.d[ 1 ] = R2AddrS( cpu.reg[ A ] ) | ( cpu.d[ 1 ] & D_S_MASK );
            break;
        case 0xA: /* AD0XS */
            ta = cpu.d[ 0 ];
            cpu.d[ 0 ] = R2AddrS( cpu.reg[ A ] ) | ( cpu.d[ 0 ] & D_S_MASK );
            Addr2RS( cpu.reg[ A ], ta );
            break;
        case 0xB: /* AD1XS */
            ta = cpu.d[ 1 ];
            cpu.d[ 1 ] = R2AddrS( cpu.reg[ A ] ) | ( cpu.d[ 1 ] & D_S_MASK );
            Addr2RS( cpu.reg[ A ], ta );
            break;
        case 0xC: /* D0=CS */
            cpu.d[ 0 ] = R2AddrS( cpu.reg[ C ] ) | ( cpu.d[ 0 ] & D_S_MASK );
            break;
        case 0xD: /* D1=CS */
            cpu.d[ 1 ] = R2AddrS( cpu.reg[ C ] ) | ( cpu.d[ 1 ] & D_S_MASK );
            break;
        case 0xE: /* CD0XS */
            ta = cpu.d[ 0 ];
            cpu.d[ 0 ] = R2AddrS( cpu.reg[ C ] ) | ( cpu.d[ 0 ] & D_S_MASK );
            Addr2RS( cpu.reg[ C ], ta );
            break;
        case 0xF: /* CD1XS */
            ta = cpu.d[ 1 ];
            cpu.d[ 1 ] = R2AddrS( cpu.reg[ C ] ) | ( cpu.d[ 1 ] & D_S_MASK );
            Addr2RS( cpu.reg[ C ], ta );
            break;
    }
}

/* Instruction Group_14 */
static void ExecGroup_14( void )
{
    Nibble n = FetchNibble( cpu.pc );
    cpu.pc++;

    /* Load/Store A/C to @D0/@D1, Field selector A or B */
    switch ( n ) {
        case 0x0: /* DAT0=A A */
            bus_write( cpu.d[ 0 ], cpu.reg[ A ], FS_A );
            break;
        case 0x1: /* DAT1=A A */
            bus_write( cpu.d[ 1 ], cpu.reg[ A ], FS_A );
            break;
        case 0x2: /* A=DAT0 A */
            bus_read( cpu.reg[ A ], cpu.d[ 0 ], FS_A );
            break;
        case 0x3: /* A=DAT1 A */
            bus_read( cpu.reg[ A ], cpu.d[ 1 ], FS_A );
            break;
        case 0x4: /* DAT0=C A */
            bus_write( cpu.d[ 0 ], cpu.reg[ C ], FS_A );
            break;
        case 0x5: /* DAT1=C A */
            bus_write( cpu.d[ 1 ], cpu.reg[ C ], FS_A );
            break;
        case 0x6: /* C=DAT0 A */
            bus_read( cpu.reg[ C ], cpu.d[ 0 ], FS_A );
            break;
        case 0x7: /* C=DAT1 A */
            bus_read( cpu.reg[ C ], cpu.d[ 1 ], FS_A );
            break;
        case 0x8: /* DAT0=A B */
            bus_write( cpu.d[ 0 ], cpu.reg[ A ], FS_B );
            break;
        case 0x9: /* DAT1=A B */
            bus_write( cpu.d[ 1 ], cpu.reg[ A ], FS_B );
            break;
        case 0xA: /* A=DAT0 B */
            bus_read( cpu.reg[ A ], cpu.d[ 0 ], FS_B );
            break;
        case 0xB: /* A=DAT1 B */
            bus_read( cpu.reg[ A ], cpu.d[ 1 ], FS_B );
            break;
        case 0xC: /* DAT0=C B */
            bus_write( cpu.d[ 0 ], cpu.reg[ C ], FS_B );
            break;
        case 0xD: /* DAT1=C B */
            bus_write( cpu.d[ 1 ], cpu.reg[ C ], FS_B );
            break;
        case 0xE: /* C=DAT0 B */
            bus_read( cpu.reg[ C ], cpu.d[ 0 ], FS_B );
            break;
        case 0xF: /* C=DAT1 B */
            bus_read( cpu.reg[ C ], cpu.d[ 1 ], FS_B );
            break;
    }
}

/* Instruction Group_15 */
static void ExecGroup_15( void )
{
    /* Load/Store A/C to @D0/@D1, Other Field Selectors */
    Nibble n = FetchNibble( cpu.pc );
    cpu.pc++;
    opcode *= 0x10;
    opcode += n;
    Nibble f = FetchNibble( cpu.pc );
    cpu.pc++;
    opcode *= 0x10;
    opcode += f;

    int oc = GET_OC_3b( n );
    int is_immediate = CHECK_IMMEDIATE_FS_FLAG( n );

    switch ( oc ) {
        case 0x0: /* DAT0=A */
            if ( is_immediate )
                bus_write_immediate( cpu.d[ 0 ], cpu.reg[ A ], f );
            else
                bus_write( cpu.d[ 0 ], cpu.reg[ A ], f );
            break;
        case 0x1: /* DAT1=A */
            if ( is_immediate )
                bus_write_immediate( cpu.d[ 1 ], cpu.reg[ A ], f );
            else
                bus_write( cpu.d[ 1 ], cpu.reg[ A ], f );
            break;
        case 0x2: /* A=DAT0 */
            if ( is_immediate )
                bus_read_immediate( cpu.reg[ A ], cpu.d[ 0 ], f );
            else
                bus_read( cpu.reg[ A ], cpu.d[ 0 ], f );
            break;
        case 0x3: /* A=DAT1 */
            if ( is_immediate )
                bus_read_immediate( cpu.reg[ A ], cpu.d[ 1 ], f );
            else
                bus_read( cpu.reg[ A ], cpu.d[ 1 ], f );
            break;
        case 0x4: /* DAT0=C */
            if ( is_immediate )
                bus_write_immediate( cpu.d[ 0 ], cpu.reg[ C ], f );
            else
                bus_write( cpu.d[ 0 ], cpu.reg[ C ], f );
            break;
        case 0x5: /* DAT1=C */
            if ( is_immediate )
                bus_write_immediate( cpu.d[ 1 ], cpu.reg[ C ], f );
            else
                bus_write( cpu.d[ 1 ], cpu.reg[ C ], f );
            break;
        case 0x6: /* C=DAT0 */
            if ( is_immediate )
                bus_read_immediate( cpu.reg[ C ], cpu.d[ 0 ], f );
            else
                bus_read( cpu.reg[ C ], cpu.d[ 0 ], f );
            break;
        case 0x7: /* C=DAT1 */
            if ( is_immediate )
                bus_read_immediate( cpu.reg[ C ], cpu.d[ 1 ], f );
            else
                bus_read( cpu.reg[ C ], cpu.d[ 1 ], f );
            break;

        default:
            FATAL( CPU_CHF_MODULE_ID, CPU_F_INTERR, "Bad_Operation_Code" )
            break;
    }
}

/* Instruction Group_1 */
static void ExecGroup_1( void )
{
    Nibble n = FetchNibble( cpu.pc );
    cpu.pc++;
    opcode *= 0x10;
    opcode += n;

    int rn, c_or_a;
    Address ta;

    switch ( n ) {
        case 0x0: /* Rn=A/C */
            n = FetchNibble( cpu.pc );
            cpu.pc++;
            opcode *= 0x10;
            opcode += n;
            rn = GET_Rn( n );
            c_or_a = CHECK_FLAG_C_OR_A( n );

            CopyRR( cpu.reg_r[ rn ], ( c_or_a ? cpu.reg[ C ] : cpu.reg[ A ] ), FS_W );
            break;
        case 0x1: /* A/C=Rn */
            n = FetchNibble( cpu.pc );
            cpu.pc++;
            opcode *= 0x10;
            opcode += n;
            rn = GET_Rn( n );
            c_or_a = CHECK_FLAG_C_OR_A( n );

            CopyRR( ( c_or_a ? cpu.reg[ C ] : cpu.reg[ A ] ), cpu.reg_r[ rn ], FS_W );
            break;
        case 0x2: /* ARnEX, CRnEX */
            n = FetchNibble( cpu.pc );
            cpu.pc++;
            opcode *= 0x10;
            opcode += n;
            rn = GET_Rn( n );
            c_or_a = CHECK_FLAG_C_OR_A( n );

            ExchRR( ( c_or_a ? cpu.reg[ C ] : cpu.reg[ A ] ), cpu.reg_r[ rn ], FS_W );
            break;
        case 0x3:
            ExecGroup_13();
            break;
        case 0x4:
            ExecGroup_14();
            break;
        case 0x5:
            ExecGroup_15();
            break;
        case 0x6: /* D0=D0+n+1 */
            n = FetchNibble( cpu.pc );
            cpu.pc++;
            opcode *= 0x10;
            opcode += n;
            ta = ( cpu.d[ 0 ] + n + 1 ) & ADDRESS_MASK;
            cpu.carry = ( ta < cpu.d[ 0 ] );
            cpu.d[ 0 ] = ta;
            break;
        case 0x7: /* D1=D1+n+1 */
            n = FetchNibble( cpu.pc );
            cpu.pc++;
            opcode *= 0x10;
            opcode += n;
            ta = ( cpu.d[ 1 ] + n + 1 ) & ADDRESS_MASK;
            cpu.carry = ( ta < cpu.d[ 1 ] );
            cpu.d[ 1 ] = ta;
            break;
        case 0x8: /* D0=D0-(n+1) */
            n = FetchNibble( cpu.pc );
            cpu.pc++;
            opcode *= 0x10;
            opcode += n;
            ta = ( cpu.d[ 0 ] - n - 1 ) & ADDRESS_MASK;
            cpu.carry = ( ta > cpu.d[ 0 ] );
            cpu.d[ 0 ] = ta;
            break;
        case 0x9: /* D0=(2) nn */
            FetchD( &cpu.d[ 0 ], 2 );
            break;
        case 0xA: /* D0=(4) nn */
            FetchD( &cpu.d[ 0 ], 4 );
            break;
        case 0xB: /* D0=(5) nn */
            FetchD( &cpu.d[ 0 ], 5 );
            break;
        case 0xC: /* D1=D1-(n+1) */
            n = FetchNibble( cpu.pc );
            cpu.pc++;
            opcode *= 0x10;
            opcode += n;
            ta = ( cpu.d[ 1 ] - n - 1 ) & ADDRESS_MASK;
            cpu.carry = ( ta > cpu.d[ 1 ] );
            cpu.d[ 1 ] = ta;
            break;
        case 0xD: /* D1=(2) nn */
            FetchD( &cpu.d[ 1 ], 2 );
            break;
        case 0xE: /* D1=(4) nn */
            FetchD( &cpu.d[ 1 ], 4 );
            break;
        case 0xF: /* D1=(5) nn */
            FetchD( &cpu.d[ 1 ], 5 );
            break;

        default:
            /* Unknown opcode */
            ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, cpu.pc, n )
            break;
    }
}

/* Instruction Group_808 */
static void ExecGroup_808( void )
{
    Nibble n = FetchNibble( cpu.pc );
    cpu.pc++;
    opcode *= 0x10;
    opcode += n;

    switch ( n ) {
        case 0x0: /* INTON */
            /* Enable maskable interrupts */
            cpu.int_enable = true;
            break;
        case 0x1: /* RSI */
            /* Discard last nibble of RSI opcode */
            cpu.pc++;

            KeybRSI();
            break;
        case 0x2: /* LA(m) n..n */
            n = FetchNibble( cpu.pc );
            cpu.pc++;
            FetchR( cpu.reg[ A ], n );
            break;
        case 0x3: /* BUSCB */

            WARNING( CPU_CHF_MODULE_ID, CPU_F_INTERR, "BUSCB" )
            break;
        case 0x4: /* ABIT=0 d */
            n = FetchNibble( cpu.pc );
            cpu.pc++;
            ExecBIT0( cpu.reg[ A ], n );
            break;
        case 0x5: /* ABIT=1 d */
            n = FetchNibble( cpu.pc );
            cpu.pc++;
            ExecBIT1( cpu.reg[ A ], n );
            break;
        case 0x6: /* ?ABIT=0 d */
            n = FetchNibble( cpu.pc );
            cpu.pc++;
            TestBIT0( cpu.reg[ A ], n );
            ExecGOYES_RTNYES();
            break;
        case 0x7: /* ?ABIT=1 d */
            n = FetchNibble( cpu.pc );
            cpu.pc++;
            TestBIT1( cpu.reg[ A ], n );
            ExecGOYES_RTNYES();
            break;
        case 0x8: /* CBIT=0 d */
            n = FetchNibble( cpu.pc );
            cpu.pc++;
            ExecBIT0( cpu.reg[ C ], n );
            break;
        case 0x9: /* CBIT=1 d */
            n = FetchNibble( cpu.pc );
            cpu.pc++;
            ExecBIT1( cpu.reg[ C ], n );
            break;
        case 0xA: /* ?CBIT=0 d */
            n = FetchNibble( cpu.pc );
            cpu.pc++;
            TestBIT0( cpu.reg[ C ], n );
            ExecGOYES_RTNYES();
            break;
        case 0xB: /* ?CBIT=1 d */
            n = FetchNibble( cpu.pc );
            cpu.pc++;
            TestBIT1( cpu.reg[ C ], n );
            ExecGOYES_RTNYES();
            break;
        case 0xC: /* PC=(A) */
            cpu.pc = Get5NibblesAbs( R2Addr( cpu.reg[ A ] ) );
            break;
        case 0xD: /* BUSCD */

            WARNING( CPU_CHF_MODULE_ID, CPU_F_INTERR, "BUSCD" )
            break;
        case 0xE: /* PC=(C) */
            cpu.pc = Get5NibblesAbs( R2Addr( cpu.reg[ C ] ) );
            break;
        case 0xF: /* INTOFF */
            cpu.int_enable = false;
            break;

        default:
            /* Unknown opcode */
            ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, cpu.pc, n )
            break;
    }
}

/* Instruction Group_80 */
static void ExecGroup_80( void )
{
    Nibble n = FetchNibble( cpu.pc );
    cpu.pc++;
    opcode *= 0x10;
    opcode += n;

    switch ( n ) {
        case 0x0: /* OUT=CS */
            cpu.out = ( ( OutputRegister )cpu.reg[ C ][ 0 ] ) | ( cpu.out & 0xFF0 );
            break;
        case 0x1: /* OUT=C */
            cpu.out = ( ( OutputRegister )cpu.reg[ C ][ 0 ] ) | ( ( OutputRegister )cpu.reg[ C ][ 1 ] << 4 ) |
                      ( ( OutputRegister )cpu.reg[ C ][ 2 ] << 8 );
            break;
        case 0x2: /* A=IN */
            copy_in_to_( cpu.reg[ A ] );
            break;
        case 0x3: /* C=IN */
            copy_in_to_( cpu.reg[ C ] );
            break;
        case 0x4: /* UNCNFG */
            ModUnconfig( R2Addr( cpu.reg[ C ] ) );
            break;
        case 0x5: /* CONFIG */
            ModConfig( R2Addr( cpu.reg[ C ] ) );
            break;
        case 0x6: /* C=ID */
            Addr2R( cpu.reg[ C ], ModGetID() );
            break;
        case 0x7: /* SHUTDN */
            op807();
            break;
        case 0x8: /* Group 808 */
            ExecGroup_808();
            break;
        case 0x9: /* C+P+1 */
            AddRImm( cpu.reg[ C ], FS_A, cpu.p );
            break;
        case 0xA: /* RESET */
            ModReset();
            break;
        case 0xB: /* BUSCC */
            if ( config.enable_BUSCC )
                ExecGroup_80B();
            else
                DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "BUSCC not implemented" )
            break;
        case 0xC: /* C=P n */
            n = FetchNibble( cpu.pc );
            cpu.pc++;
            cpu.reg[ C ][ ( int )n ] = cpu.p;
            break;
        case 0xD: /* P=C n */
            n = FetchNibble( cpu.pc );
            cpu.pc++;
            SetP( cpu.reg[ C ][ ( int )n ] );
            break;
        case 0xE: /* SREQ? */

            WARNING( CPU_CHF_MODULE_ID, CPU_F_INTERR, "SREQ" )
            break;
        case 0xF: /* CPEX */
            {
                Nibble tmp = cpu.p;
                n = FetchNibble( cpu.pc );
                cpu.pc++;
                opcode *= 0x10;
                opcode += n;
                SetP( cpu.reg[ C ][ ( int )n ] );
                cpu.reg[ C ][ ( int )n ] = tmp;
            }
            break;

        default:
            ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, cpu.pc, n )
            break;
    }
}

/* Special functions Group_81 */
static void ExecSpecialGroup_81( int rp )
{
    Nibble n, f, m;
    int rn, c_or_a;

    switch ( rp ) {
        case 0x0: /* r=r+-CON fs, d */
            f = FetchNibble( cpu.pc );
            cpu.pc++;
            opcode *= 0x10;
            opcode += f;
            n = FetchNibble( cpu.pc );
            cpu.pc++;
            opcode *= 0x10;
            opcode += n;
            m = FetchNibble( cpu.pc );
            cpu.pc++;
            opcode *= 0x10;
            opcode += m;
            rp = GET_RP( n );

            if ( CHECK_FLAG_SUBTRACT_OR_ADD( n ) ) /* Subtract */
                SubRImm( reg_pair_0[ rp ], f, m );
            else /* Add */
                AddRImm( reg_pair_0[ rp ], f, m );
            break;
        case 0x1: /* rSRB.f fs */
            f = FetchNibble( cpu.pc );
            cpu.pc++;
            opcode *= 0x10;
            opcode += f;
            n = FetchNibble( cpu.pc );
            cpu.pc++;
            opcode *= 0x10;
            opcode += n;
            rp = GET_RP( n );
            ShiftRightBitR( reg_pair_0[ rp ], f );
            break;
        case 0x2: /* Rn=r.F fs, r=R0.F fs, rRnEX.F fs */
            f = FetchNibble( cpu.pc );
            cpu.pc++;
            opcode *= 0x10;
            opcode += f;
            n = FetchNibble( cpu.pc );
            cpu.pc++;
            opcode *= 0x10;
            opcode += n;
            m = FetchNibble( cpu.pc );
            cpu.pc++;
            opcode *= 0x10;
            opcode += m;
            rn = GET_Rn( m );
            c_or_a = CHECK_FLAG_C_OR_A( m );

            switch ( n ) {
                case 0x0: /* Rn=r.F fs */
                    CopyRR( cpu.reg_r[ rn ], ( c_or_a ? cpu.reg[ C ] : cpu.reg[ A ] ), f );
                    break;

                case 0x1: /* r=R0.F fs */
                    CopyRR( ( c_or_a ? cpu.reg[ C ] : cpu.reg[ A ] ), cpu.reg_r[ rn ], f );
                    break;

                case 0x2: /* rRnEX.F fs */
                    ExchRR( ( c_or_a ? cpu.reg[ C ] : cpu.reg[ A ] ), cpu.reg_r[ rn ], f );
                    break;

                default:
                    ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, cpu.pc, n )
                    break;
            }
            break;
        case 0x3: /* Group 81B */
            n = FetchNibble( cpu.pc );
            cpu.pc++;
            opcode *= 0x10;
            opcode += n;
            switch ( n ) {
                case 0x1:
                    if ( config.big_screen && config.enable_BUSCC )
                        DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "//TODO: LOOP2 (RPL2 80B00)" )
                    else
                        ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, cpu.pc, n )
                    break;

                case 0x2: /* PC=A */
                    cpu.pc = R2Addr( cpu.reg[ A ] );
                    break;

                case 0x3: /* PC=C */
                    cpu.pc = R2Addr( cpu.reg[ C ] );
                    break;

                case 0x4: /* A=PC */
                    Addr2R( cpu.reg[ A ], cpu.pc );
                    break;

                case 0x5: /* C=PC */
                    Addr2R( cpu.reg[ C ], cpu.pc );
                    break;

                case 0x6: /* APCEX */
                    {
                        Address t;
                        t = R2Addr( cpu.reg[ A ] );
                        Addr2R( cpu.reg[ A ], cpu.pc );
                        cpu.pc = t;
                        break;
                    }

                case 0x7: /* CPCEX */
                    {
                        Address t;
                        t = R2Addr( cpu.reg[ C ] );
                        Addr2R( cpu.reg[ C ], cpu.pc );
                        cpu.pc = t;
                        break;
                    }

                default:
                    ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, cpu.pc, n )
                    break;
            }
            break;

        default:
            FATAL( CPU_CHF_MODULE_ID, CPU_F_INTERR, "Bad_Register_Pair" )
            break;
    }
}

/* Instruction Group_8 */
static void ExecGroup_8( void )
{
    Nibble n = FetchNibble( cpu.pc );
    cpu.pc++;
    opcode *= 0x10;
    opcode += n;
    Address addr;
    int oc, rp;

    switch ( n ) {
        case 0x0:
            ExecGroup_80();
            break;
        case 0x1: /* rSLC, rSRC, rSRB, Special Group_81 */
            n = FetchNibble( cpu.pc );
            cpu.pc++;
            opcode *= 0x10;
            opcode += n;
            oc = GET_OC_1( n );
            rp = GET_RP( n );

            switch ( oc ) {
                case 0x0: /* rSLC */
                    ShiftLeftCircR( reg_pair_0[ rp ], FS_W );
                    break;

                case 0x1: /* rSRC */
                    ShiftRightCircR( reg_pair_0[ rp ], FS_W );
                    break;

                case 0x2: /* Special Group_81 */
                    ExecSpecialGroup_81( rp );
                    break;

                case 0x3: /* rSRB */
                    ShiftRightBitR( reg_pair_0[ rp ], FS_W );
                    break;

                default:
                    FATAL( CPU_CHF_MODULE_ID, CPU_F_INTERR, "Bad_Operation_Code" )
                    break;
            }
            break;
        case 0x2: /* CLRHSn */
            cpu.hst &= ~FetchNibble( cpu.pc );
            cpu.pc++;
            break;
        case 0x3: /* ?HS=0 */
            n = FetchNibble( cpu.pc );
            cpu.pc++;
            opcode *= 0x10;
            opcode += n;
            cpu.carry = ( ( cpu.hst & n ) == 0 );
            ExecGOYES_RTNYES();
            break;
        case 0x4: /* ST=0 n */
            n = FetchNibble( cpu.pc );
            cpu.pc++;
            cpu.st &= ~st_bit_mask[ ( int )n ];
            break;
        case 0x5: /* ST=1 n */
            n = FetchNibble( cpu.pc );
            cpu.pc++;
            cpu.st |= st_bit_mask[ ( int )n ];
            break;
        case 0x6: /* ?ST=0 n */
            n = FetchNibble( cpu.pc );
            cpu.pc++;
            cpu.carry = ( ( cpu.st & st_bit_mask[ ( int )n ] ) == 0 );
            ExecGOYES_RTNYES();
            break;
        case 0x7: /* ?ST=1 n */
            n = FetchNibble( cpu.pc );
            cpu.pc++;
            cpu.carry = ( ( cpu.st & st_bit_mask[ ( int )n ] ) != 0 );
            ExecGOYES_RTNYES();
            break;
        case 0x8: /* ?P#n */
            n = FetchNibble( cpu.pc );
            cpu.pc++;
            cpu.carry = ( cpu.p != n );
            ExecGOYES_RTNYES();
            break;
        case 0x9: /* ?P=n */
            n = FetchNibble( cpu.pc );
            cpu.pc++;
            cpu.carry = ( cpu.p == n );
            ExecGOYES_RTNYES();
            break;
        case 0xA: /* Test */
            ExecTest_8A();
            break;
        case 0xB: /* Test */
            ExecTest_8B();
            break;
        case 0xC: /* GOLONG */
            addr = Get4Nibbles2C( cpu.pc );
            cpu.pc += addr;
            break;
        case 0xD: /* GOVLNG */
            cpu.pc = Get5NibblesAbs( cpu.pc );
            break;
        case 0xE: /* GOSUBL */
            addr = Get4Nibbles2C( cpu.pc );
            cpu.pc += 4;
            rstk_push( cpu.pc );
            cpu.pc += addr;
            break;
        case 0xF: /* GOSBVL */
            rstk_push( cpu.pc + 5 );
            cpu.pc = Get5NibblesAbs( cpu.pc );
            break;

        default:
            ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, cpu.pc, n )
            break;
    }
}

/*---------------------------------------------------------------------------
        Public functions
  ---------------------------------------------------------------------------*/

/* .+

.creation     : 3-Feb-1998
.description  :
  This function resets the CPU, performing the following operations:

  - Copies the field selector index arrays to the cpu structure
  - Set P=0
  - Clears registers A, B, C, D, Rn
  - Clears registers D0, D1
  - Sets PC to zero
  - Clears registers IN, OUT, ST, HST
  - Sets hex mode for arithmetic operations
  - Clears carry, int_enable, int_service, int_pending, and shutdn
  - The inner_loop limit is set to INNER_LOOP_MED

.call         :
                CpuReset();
.input        :
                void
.output       :
                void
.status_codes :
                CPU_I_CALLED
                CPU_E_BAD_OPCODE
                CPU_F_INTERR
.notes        :
  1.1, 3-Feb-1998, creation
  1.2, 7-Sep-2000, bug fix
    - cpu.rstk_ptr and .reset_req were not reset; this gave troubles
      when attempting to override a corrupt status with CpuReset().
  3.13, 2-Nov-2000, update
    - cpu.halt and cpu.inner_loop_max need reset
  3.14, 10-Nov-2000, bug fix
    - cpu.inner_loop_max must be reset to 0, because the default
      emulator speed is maximum speed.
.- */
void CpuReset( void )
{
    int n;

    /* Copy field selector index arrays to the cpu structure */
    ( void )memcpy( cpu.fs_idx_lo, fs_idx_lo, sizeof( fs_idx_lo ) );
    ( void )memcpy( cpu.fs_idx_hi, fs_idx_hi, sizeof( fs_idx_hi ) );

    /* Set P=0 and adjust fs index arrays */
    SetP( ( Nibble )0 );

    /* Clear A, B, C, D */
    for ( n = 0; n < N_WORKING_REGISTER; n++ )
        ClearR( cpu.reg[ n ], FS_W );

    /* Clear Rn */
    for ( n = 0; n < N_SCRATCH_REGISTER; n++ )
        ClearR( cpu.reg_r[ n ], FS_W );

    /* Clear D0, D1 */
    cpu.d[ 0 ] = cpu.d[ 1 ] = ( Address )0;

    /* Clear PC */
    cpu.pc = ( Address )0;

    /* Clear IN, OUT, ST, HST */
    cpu.in = ( InputRegister )0;
    cpu.out = ( OutputRegister )0;
    cpu.st = ( ProgramStatusRegister )0;
    cpu.hst = ( Nibble )0;

    /* Fill the return stack with (Address)0 */
    cpu.rstk_ptr = 0;
    for ( n = 0; n < RETURN_STACK_SIZE; n++ )
        cpu.rstk[ n ] = ( Address )0;

    /* Set hexmode */
    cpu.hexmode = true;

    /* Clear carry */
    cpu.carry = false;

    /* Disable maskable interrupts */
    cpu.int_enable = false;

    /* No interrupts are pending (for now) */
    cpu.int_service = false;
    cpu.int_pending = false;

    /* The CPU is running */
    cpu.shutdn = false;

    /* Set inner_loop and inner_loop_max to default values */
    cpu.inner_loop = INNER_LOOP_MED;
    cpu.inner_loop_max = 0;
}

/* .+

.creation     : 11-Feb-1998
.description  :
  This function initializes the Saturn CPU, reading its status from disk.
  If something goes wrong with the disk I/O, the function resets the CPU.

.call         :
                CpuInit();
.input        :
                void
.output       :
                void
.status_codes :
                CPU_I_CALLED
                CPU_I_REVISION
                CPU_W_RESETTING
.notes        :
  1.1, 11-Feb-1998, creation
  3.14, 10-Nov-2000, update
    - clear both shutdn and halt cpu flags here; this helps when the CPU
      state was saved and reloaded when the CPU was halted.
.- */
void CpuInit( void )
{
    if ( ReadStructFromFile( config.cpu_path, sizeof( cpu ), &cpu ) ) {
        WARNING0( CPU_CHF_MODULE_ID, CPU_W_RESETTING )

        CpuReset();
    }

    /* The CPU is running */
    cpu.shutdn = false;
}

/* .+

.creation     : 11-Feb-1998
.description  :
  This function saves the current Saturn CPU status to disk.

.call         :
                CpuSave();
.input        :
                void
.output       :
                void
.status_codes :
                CPU_I_CALLED
                CPU_E_SAVE
.notes        :
  1.1, 11-Feb-1998, creation

.- */
void CpuSave( void )
{
    if ( WriteStructToFile( &cpu, sizeof( cpu ), config.cpu_path ) )
        ERROR0( CPU_CHF_MODULE_ID, CPU_E_SAVE )
}

/* .+

.creation     : 11-Feb-1998
.description  :
  This function posts an interrupt request for the Saturn CPU.

  The NMI interrupt requests are always honored; the IRQ requests are
  honored immediately only if the CPU interrupts are enabled, otherwise
  they will be honored as soon as the CPU reenables interrupts.

  NOTE: The interrupt request can be INT_REQUEST_NONE; in this case, this
        function does not post any interrupt request.

.call         :
                CpuIntRequest(ireq);
.input        :
                enum IntRequest ireq, interrupt request type, or
                        INT_REQUEST_NONE
.output       :
                void
.status_codes :
                CPU_I_CALLED
                CPU_I_INT
                CPU_I_INT_PENDING
.notes        :
  1.1, 11-Feb-1998, creation

.- */
void CpuIntRequest( int_request_t ireq )
{
    if ( ( ireq == INT_REQUEST_IRQ && cpu.int_enable ) || ireq == INT_REQUEST_NMI ) {
        /* Wake the CPU if it's sleeping */
        CpuWake();

        /* Check if immediate vectoring is ok */
        if ( !cpu.int_service ) {
            /* Vector immediately */
            cpu.int_service = true;
            cpu.int_pending = INT_REQUEST_NONE;
            rstk_push( cpu.pc );
            cpu.pc = INT_HANDLER_PC;

            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_INT, CPU_I_INT, ( ireq == INT_REQUEST_NMI ? "NMI" : "IRQ" ) )
        } else {
            /* int_service is set; save the request for later processing */
            cpu.int_pending = ireq;

            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_INT, CPU_I_INT_PENDING, ( ireq == INT_REQUEST_NMI ? "NMI" : "IRQ" ) )
        }
    }
}

/* .+

.creation     : 11-Feb-1998
.description  :
  This function awakes the CPU if it has executed a SHUTDN instruction
  and no halt requests are pending (see CpuHaltRequest() for more
  information).

  If the CPU is running, this function has no effect.

.call         :
                CpuWake();
.input        :
                void
.output       :
                void
.status_codes :
                CPU_I_CALLED
                CPU_I_WAKE
.notes        :
  1.1, 11-Feb-1998, creation
  3.13, 2-Nov-2000, update:
    - the CPU must be awoken only if no halt request is pending

.- */
void CpuWake( void )
{
    if ( cpu.shutdn ) {
        DEBUG0( CPU_CHF_MODULE_ID, DEBUG_C_INT, CPU_I_WAKE )

        /* Clear SHUTDN flag */
        cpu.shutdn = false;

        /* Clear PC if necessary */
        /* if ( cpu.out == (OutputRegister)0 )
           cpu.pc = (Address)0;
         */
    }
}

/* .+

.creation     : 3-Feb-1998
.description  :
  This function executes a Saturn instruction starting from the current
  program counter, updating accordingly the global cpu data structure.

  The function signals all exceptional situations through Chf conditions.

.call         :
                OneStep()
.input        :
                void
.output       :
                void
.status_codes :
                CPU_I_EXECUTING
                CPU_E_BAD_OPCODE
                CPU_F_INTERR
.notes        :
  1.1, 3-Feb-1998, creation

.- */
void OneStep( void )
{
    opcode = 0;

    Address offset;
    /* Get first instruction nibble */
    Nibble n = FetchNibble( cpu.pc );
    cpu.pc++;
    opcode *= 0x10;
    opcode += n;

    switch ( n ) {
        case 0x0: /* Group_0 */
            ExecGroup_0();
            break;
        case 0x1: /* Group_1 */
            ExecGroup_1();
            break;
        case 0x2: /* P=n */
            n = FetchNibble( cpu.pc );
            cpu.pc++;
            SetP( n );
            break;
        case 0x3: /* LC(m) n...n */
            n = FetchNibble( cpu.pc );
            cpu.pc++;
            FetchR( cpu.reg[ C ], n );
            break;
        case 0x4: /* RTNC/GOC */
            if ( cpu.carry ) {
                offset = Get2Nibbles2C( cpu.pc );
                if ( offset == 0 )
                    cpu.pc = rstk_pop();
                else
                    cpu.pc += offset;
            } else
                cpu.pc += 2;

            break;
        case 0x5: /* RTNNC/GONC */
            if ( !cpu.carry ) {
                offset = Get2Nibbles2C( cpu.pc );
                if ( offset == 0 )
                    cpu.pc = rstk_pop();
                else
                    cpu.pc += offset;
            } else
                cpu.pc += 2;

            break;
        case 0x6: /* GOTO */
            cpu.pc += Get3Nibbles2C( cpu.pc );
            break;
        case 0x7: /* GOSUB */
            offset = Get3Nibbles2C( cpu.pc );
            cpu.pc += 3;
            rstk_push( cpu.pc );
            cpu.pc += offset;
            break;
        case 0x8: /* Group_8 */
            ExecGroup_8();
            break;
        case 0x9: /* Test */
            ExecTest_9();
            break;
        case 0xA: /* Register Operation, group A */
            ExecRegOp_A();
            break;
        case 0xB: /* Register Operation, group B */
            ExecRegOp_B();
            break;
        case 0xC: /* Register Operation, group C */
            ExecRegOp_C();
            break;
        case 0xD: /* Register Operation, group D */
            ExecRegOp_D();
            break;
        case 0xE: /* Register Operation, group E */
            ExecRegOp_E();
            break;
        case 0xF: /* Register Operation, group F */
            ExecRegOp_F();
            break;

        default:
            ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, cpu.pc, n )
            break;
    }

    if ( config.debug_level & ( DEBUG_C_IMPLEMENTATION ) ) {
        char dis_opcode[ DISASSEMBLE_OB_SIZE ];
        Disassemble( opcode, dis_opcode );

        fprintf( stderr, "%s\n", dis_opcode );
    }
}

void set_speed( unsigned int new_speed_mhz )
{
    /* Compute inner loop limit; 4 is the real CPU speed in MHz when
       the limit is set to INNER_LOOP_MAX.  No overflow checks,
       because new_speed is >=0, has an architectural upper limit of 2^20,
       and int are at least 2^31.
     */
    cpu.inner_loop_max = ( new_speed_mhz * INNER_LOOP_MAX ) / 4;
}
