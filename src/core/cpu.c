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

  - Added a delay loop in ExecIN(), when CPU_SLOW_IN is defined. the loop
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
  Bug fix: cpu.return_sp and .reset_req were not reset; this gave
  troubles when attempting to override a corrupt status with CpuReset().

  Revision 1.1  1998/02/17  15:25:16  cibrario
  Initial revision
.- */

#include <stdio.h>
#include <string.h>

#include "../libChf/src/Chf.h"

#include "config.h"
#include "cpu.h"
#include "cpu_buscc.h"
#include "modules.h"
#include "keyb.h"
#include "disk_io.h" /* 3.1: ReadStructFromFile/WriteStructToFile */
#include "chf_wrapper.h"
#include "dis.h"

/*---------------------------------------------------------------------------
        Global variables
  ---------------------------------------------------------------------------*/

int opcode;

struct Cpu cpu;

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
    { cpu.A, cpu.B, cpu.C, cpu.D };

static Nibble* const reg_pair_1[] =
    /*	AB,		BC,		CA,		DC		*/
    { cpu.B, cpu.C, cpu.A, cpu.C };

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

/* PushRSTK */
static void PushRSTK( const Address r )
{
    cpu.return_stack[ cpu.return_sp ] = r;
    cpu.return_sp = ( cpu.return_sp + 1 ) & RETURN_SP_MASK;
}

/* PopRSTK */
static Address PopRSTK( void )
{
    cpu.return_sp = ( cpu.return_sp - 1 ) & RETURN_SP_MASK;

    Address r = cpu.return_stack[ cpu.return_sp ];

    cpu.return_stack[ cpu.return_sp ] = ( Address )0;

    return r;
}

/*---------------------------------------------------------------------------
        Private functions: bus input/output
  ---------------------------------------------------------------------------*/

/* IN */
static void ExecIN( Nibble* r )
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
        cpu.PC -= 3;
        return;
    }

    /* Counter expired; reset counter and execute the instruction */
    count_down = ( ( cpu.inner_loop + ( INNER_LOOP_MAX / 2 ) ) / INNER_LOOP_MAX ) * CPU_SLOW_IN;
#endif
    cpu.IN = KeybIN( cpu.OUT );

    r[ 0 ] = ( Nibble )( cpu.IN & NIBBLE_MASK );
    r[ 1 ] = ( Nibble )( ( cpu.IN ) >> 4 & NIBBLE_MASK );
    r[ 2 ] = ( Nibble )( ( cpu.IN ) >> 8 & NIBBLE_MASK );
    r[ 3 ] = ( Nibble )( ( cpu.IN ) >> 12 & NIBBLE_MASK );
}

/*---------------------------------------------------------------------------
        Private functions: CPU control
  ---------------------------------------------------------------------------*/

static void ExecSHUTDN( void )
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
static void ReadDAT( Nibble* d, Address s, int fs )
{
    register int lo = cpu.fs_idx_lo[ fs ];
    register int hi = cpu.fs_idx_hi[ fs ];

    for ( register int n = lo; n <= hi; n++ ) {
        d[ n ] = ReadNibble( s );
        s++;
    }
}

/* Read a field of a DataRegister from memory, with immediate fs */
static void ReadDATImm( Nibble* d, Address s, int imm_fs )
{
    for ( register int n = 0; n <= imm_fs; n++ )
        d[ n ] = ReadNibble( s++ );
}

/* Write a field of a DataRegister into memory */
static void WriteDAT( Address d, const Nibble* r, int fs )
{
    register int lo = cpu.fs_idx_lo[ fs ];
    register int hi = cpu.fs_idx_hi[ fs ];

    for ( register int n = lo; n <= hi; n++ ) {
        WriteNibble( d, r[ n ] );
        d++;
    }
}

/* Write a field of a DataRegister into memory, with immediate fs */
static void WriteDATImm( Address d, const Nibble* r, int imm_fs )
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
        v |= ( ( Address )FetchNibble( cpu.PC ) << shift );
        cpu.PC++;
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
    register int p = ( int )cpu.P;

    for ( register int i = 0; i <= n; i++ ) {
        r[ p ] = FetchNibble( cpu.PC );
        p++;
        cpu.PC++;
        if ( p >= NIBBLE_PER_REGISTER )
            p = 0;
    }
}

/*---------------------------------------------------------------------------
        Private functions: P register setting
  ---------------------------------------------------------------------------*/
static void SetP( Nibble n )
{
    cpu.P = n;

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
        cpu.HST |= HST_SB_MASK;

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
        cpu.HST |= HST_SB_MASK;

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
        cpu.HST |= HST_SB_MASK;

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
        Address offset = Get2Nibbles2C( cpu.PC );

        if ( offset == 0 )
            /* RTNYES */
            cpu.PC = PopRSTK();
        else
            cpu.PC += offset;
    } else
        /* Not taken */
        cpu.PC += 2;
}

/*---------------------------------------------------------------------------
        Private functions: instruction stream decoding
  ---------------------------------------------------------------------------*/

/* ?..., GOYES/RTNYES, Test with Field Selector, opcode 9ftyy, length 5 */
static void ExecTest_9( void )
{
    Nibble f = FetchNibble( cpu.PC );
    cpu.PC++;
    opcode *= 0x10;
    opcode += f;
    Nibble t = FetchNibble( cpu.PC );
    cpu.PC++;
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
    Nibble t = FetchNibble( cpu.PC );
    cpu.PC++;
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
    Nibble t = FetchNibble( cpu.PC );
    cpu.PC++;
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
    Nibble f = FetchNibble( cpu.PC );
    cpu.PC++;
    opcode *= 0x10;
    opcode += f;
    Nibble o = FetchNibble( cpu.PC );
    cpu.PC++;
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
    Nibble f = FetchNibble( cpu.PC );
    cpu.PC++;
    opcode *= 0x10;
    opcode += f;
    Nibble o = FetchNibble( cpu.PC );
    cpu.PC++;
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
    Nibble o = FetchNibble( cpu.PC );
    cpu.PC++;
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
    Nibble o = FetchNibble( cpu.PC );
    cpu.PC++;
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
    Nibble o = FetchNibble( cpu.PC );
    cpu.PC++;
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
    Nibble o = FetchNibble( cpu.PC );
    cpu.PC++;
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
    Nibble f = FetchNibble( cpu.PC );
    cpu.PC++;
    opcode *= 0x10;
    opcode += f;
    Nibble o = FetchNibble( cpu.PC );
    cpu.PC++;
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
    Nibble n = FetchNibble( cpu.PC );
    cpu.PC++;
    opcode *= 0x10;
    opcode += n;

    switch ( n ) {
        case 0x0: /* RTNSXM */
            cpu.HST |= HST_XM_MASK;
            cpu.PC = PopRSTK();
            break;
        case 0x1: /* RTN */
            cpu.PC = PopRSTK();
            break;
        case 0x2: /* RTNSC */
            cpu.carry = true;
            cpu.PC = PopRSTK();
            break;
        case 0x3: /* RTNCC */
            cpu.carry = false;
            cpu.PC = PopRSTK();
            break;
        case 0x4: /* SETHEX */
            cpu.hexmode = true;
            break;
        case 0x5: /* SETDEC */
            cpu.hexmode = false;
            break;
        case 0x6: /* RSTK=C */
            PushRSTK( R2Addr( cpu.C ) );
            break;
        case 0x7: /* C=RSTK */
            Addr2R( cpu.C, PopRSTK() );
            break;
        case 0x8: /* CLRST */
            cpu.ST &= CLRST_MASK;
            break;
        case 0x9: /* C=ST */
            /* Copy the 12 low-order bits of ST into C */

            cpu.C[ 0 ] = ( Nibble )( cpu.ST & NIBBLE_MASK );
            cpu.C[ 1 ] = ( Nibble )( ( cpu.ST >> 4 ) & NIBBLE_MASK );
            cpu.C[ 2 ] = ( Nibble )( ( cpu.ST >> 8 ) & NIBBLE_MASK );
            break;
        case 0xA: /* ST=C */
            /* Copy the 12 low-order bits of C into ST */
            cpu.ST = ( ProgramStatusRegister )cpu.C[ 0 ] | ( ( ProgramStatusRegister )cpu.C[ 1 ] << 4 ) |
                     ( ( ProgramStatusRegister )cpu.C[ 2 ] << 8 ) | ( cpu.ST & CLRST_MASK );
            break;
        case 0xB: /* CSTEX */
            /* Exchange the 12 low-order bits of C with the 12 low-order bits of ST */
            {
                ProgramStatusRegister tst = cpu.ST;

                cpu.ST = ( ProgramStatusRegister )cpu.C[ 0 ] | ( ( ProgramStatusRegister )cpu.C[ 1 ] << 4 ) |
                         ( ( ProgramStatusRegister )cpu.C[ 2 ] << 8 ) | ( cpu.ST & CLRST_MASK );

                cpu.C[ 0 ] = ( Nibble )( tst & NIBBLE_MASK );
                cpu.C[ 1 ] = ( Nibble )( ( tst >> 4 ) & NIBBLE_MASK );
                cpu.C[ 2 ] = ( Nibble )( ( tst >> 8 ) & NIBBLE_MASK );
            }
            break;
        case 0xC: /* P=P+1 */
            if ( cpu.P == NIBBLE_MASK ) {
                SetP( 0 );
                cpu.carry = true;
            } else {
                SetP( cpu.P + 1 );
                cpu.carry = false;
            }
            break;
        case 0xD: /* P=P-1 */
            if ( cpu.P == ( Nibble )0 ) {
                SetP( NIBBLE_MASK );
                cpu.carry = true;
            } else {
                SetP( cpu.P - 1 );
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
                cpu.PC = INT_HANDLER_PC;
            } else {
                /* Reenable interrupts and return */
                DEBUG0( CPU_CHF_MODULE_ID, DEBUG_C_INT, CPU_I_RTI_END )

                cpu.int_service = false;
                cpu.PC = PopRSTK();
            }
            break;

        default: /* Unknown opcode */
            ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, cpu.PC, n )
            break;
    }
}

/* Instruction Group_13 */
static void ExecGroup_13( void )
{
    Address ta;
    Nibble n = FetchNibble( cpu.PC );
    cpu.PC++;

    /* Copy/Exchange A/C and D0/D1 */
    switch ( n ) {
        case 0x0: /* D0=A */
            cpu.D0 = R2Addr( cpu.A );
            break;
        case 0x1: /* D1=A */
            cpu.D1 = R2Addr( cpu.A );
            break;
        case 0x2: /* AD0EX */
            ta = cpu.D0;
            cpu.D0 = R2Addr( cpu.A );
            Addr2R( cpu.A, ta );
            break;
        case 0x3: /* AD1EX */
            ta = cpu.D1;
            cpu.D1 = R2Addr( cpu.A );
            Addr2R( cpu.A, ta );
            break;
        case 0x4: /* D0=C */
            cpu.D0 = R2Addr( cpu.C );
            break;
        case 0x5: /* D1=C */
            cpu.D1 = R2Addr( cpu.C );
            break;
        case 0x6: /* CD0EX */
            ta = cpu.D0;
            cpu.D0 = R2Addr( cpu.C );
            Addr2R( cpu.C, ta );
            break;
        case 0x7: /* CD1EX */
            ta = cpu.D1;
            cpu.D1 = R2Addr( cpu.C );
            Addr2R( cpu.C, ta );
            break;
        case 0x8: /* D0=AS */
            cpu.D0 = R2AddrS( cpu.A ) | ( cpu.D0 & D_S_MASK );
            break;
        case 0x9: /* D1=AS */
            cpu.D1 = R2AddrS( cpu.A ) | ( cpu.D1 & D_S_MASK );
            break;
        case 0xA: /* AD0XS */
            ta = cpu.D0;
            cpu.D0 = R2AddrS( cpu.A ) | ( cpu.D0 & D_S_MASK );
            Addr2RS( cpu.A, ta );
            break;
        case 0xB: /* AD1XS */
            ta = cpu.D1;
            cpu.D1 = R2AddrS( cpu.A ) | ( cpu.D1 & D_S_MASK );
            Addr2RS( cpu.A, ta );
            break;
        case 0xC: /* D0=CS */
            cpu.D0 = R2AddrS( cpu.C ) | ( cpu.D0 & D_S_MASK );
            break;
        case 0xD: /* D1=CS */
            cpu.D1 = R2AddrS( cpu.C ) | ( cpu.D1 & D_S_MASK );
            break;
        case 0xE: /* CD0XS */
            ta = cpu.D0;
            cpu.D0 = R2AddrS( cpu.C ) | ( cpu.D0 & D_S_MASK );
            Addr2RS( cpu.C, ta );
            break;
        case 0xF: /* CD1XS */
            ta = cpu.D1;
            cpu.D1 = R2AddrS( cpu.C ) | ( cpu.D1 & D_S_MASK );
            Addr2RS( cpu.C, ta );
            break;
    }
}

/* Instruction Group_14 */
static void ExecGroup_14( void )
{
    Nibble n = FetchNibble( cpu.PC );
    cpu.PC++;

    /* Load/Store A/C to @D0/@D1, Field selector A or B */
    switch ( n ) {
        case 0x0: /* DAT0=A A */
            WriteDAT( cpu.D0, cpu.A, FS_A );
            break;
        case 0x1: /* DAT1=A A */
            WriteDAT( cpu.D1, cpu.A, FS_A );
            break;
        case 0x2: /* A=DAT0 A */
            ReadDAT( cpu.A, cpu.D0, FS_A );
            break;
        case 0x3: /* A=DAT1 A */
            ReadDAT( cpu.A, cpu.D1, FS_A );
            break;
        case 0x4: /* DAT0=C A */
            WriteDAT( cpu.D0, cpu.C, FS_A );
            break;
        case 0x5: /* DAT1=C A */
            WriteDAT( cpu.D1, cpu.C, FS_A );
            break;
        case 0x6: /* C=DAT0 A */
            ReadDAT( cpu.C, cpu.D0, FS_A );
            break;
        case 0x7: /* C=DAT1 A */
            ReadDAT( cpu.C, cpu.D1, FS_A );
            break;
        case 0x8: /* DAT0=A B */
            WriteDAT( cpu.D0, cpu.A, FS_B );
            break;
        case 0x9: /* DAT1=A B */
            WriteDAT( cpu.D1, cpu.A, FS_B );
            break;
        case 0xA: /* A=DAT0 B */
            ReadDAT( cpu.A, cpu.D0, FS_B );
            break;
        case 0xB: /* A=DAT1 B */
            ReadDAT( cpu.A, cpu.D1, FS_B );
            break;
        case 0xC: /* DAT0=C B */
            WriteDAT( cpu.D0, cpu.C, FS_B );
            break;
        case 0xD: /* DAT1=C B */
            WriteDAT( cpu.D1, cpu.C, FS_B );
            break;
        case 0xE: /* C=DAT0 B */
            ReadDAT( cpu.C, cpu.D0, FS_B );
            break;
        case 0xF: /* C=DAT1 B */
            ReadDAT( cpu.C, cpu.D1, FS_B );
            break;
    }
}

/* Instruction Group_15 */
static void ExecGroup_15( void )
{
    /* Load/Store A/C to @D0/@D1, Other Field Selectors */
    Nibble n = FetchNibble( cpu.PC );
    cpu.PC++;
    opcode *= 0x10;
    opcode += n;
    Nibble f = FetchNibble( cpu.PC );
    cpu.PC++;
    opcode *= 0x10;
    opcode += f;

    int oc = GET_OC_3b( n );
    int is = GET_IMMEDIATE_FS_FLAG( n );

    switch ( oc ) {
        case 0x0: /* DAT0=A */
            if ( is )
                WriteDATImm( cpu.D0, cpu.A, f );
            else
                WriteDAT( cpu.D0, cpu.A, f );
            break;
        case 0x1: /* DAT1=A */
            if ( is )
                WriteDATImm( cpu.D1, cpu.A, f );
            else
                WriteDAT( cpu.D1, cpu.A, f );
            break;
        case 0x2: /* A=DAT0 */
            if ( is )
                ReadDATImm( cpu.A, cpu.D0, f );
            else
                ReadDAT( cpu.A, cpu.D0, f );
            break;
        case 0x3: /* A=DAT1 */
            if ( is )
                ReadDATImm( cpu.A, cpu.D1, f );
            else
                ReadDAT( cpu.A, cpu.D1, f );
            break;
        case 0x4: /* DAT0=C */
            if ( is )
                WriteDATImm( cpu.D0, cpu.C, f );
            else
                WriteDAT( cpu.D0, cpu.C, f );
            break;
        case 0x5: /* DAT1=C */
            if ( is )
                WriteDATImm( cpu.D1, cpu.C, f );
            else
                WriteDAT( cpu.D1, cpu.C, f );
            break;
        case 0x6: /* C=DAT0 */
            if ( is )
                ReadDATImm( cpu.C, cpu.D0, f );
            else
                ReadDAT( cpu.C, cpu.D0, f );
            break;
        case 0x7: /* C=DAT1 */
            if ( is )
                ReadDATImm( cpu.C, cpu.D1, f );
            else
                ReadDAT( cpu.C, cpu.D1, f );
            break;

        default:
            FATAL( CPU_CHF_MODULE_ID, CPU_F_INTERR, "Bad_Operation_Code" )
            break;
    }
}

/* Instruction Group_1 */
static void ExecGroup_1( void )
{
    Nibble n = FetchNibble( cpu.PC );
    cpu.PC++;
    opcode *= 0x10;
    opcode += n;

    int rn, ac;
    Address ta;

    switch ( n ) {
        case 0x0: /* Rn=A/C */
            n = FetchNibble( cpu.PC );
            cpu.PC++;
            opcode *= 0x10;
            opcode += n;
            rn = GET_Rn( n );
            ac = GET_AC( n );

            CopyRR( cpu.R[ rn ], ( ac ? cpu.C : cpu.A ), FS_W );
            break;
        case 0x1: /* A/C=Rn */
            n = FetchNibble( cpu.PC );
            cpu.PC++;
            opcode *= 0x10;
            opcode += n;
            rn = GET_Rn( n );
            ac = GET_AC( n );

            CopyRR( ( ac ? cpu.C : cpu.A ), cpu.R[ rn ], FS_W );
            break;
        case 0x2: /* ARnEX, CRnEX */
            n = FetchNibble( cpu.PC );
            cpu.PC++;
            opcode *= 0x10;
            opcode += n;
            rn = GET_Rn( n );
            ac = GET_AC( n );

            ExchRR( ( ac ? cpu.C : cpu.A ), cpu.R[ rn ], FS_W );
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
            n = FetchNibble( cpu.PC );
            cpu.PC++;
            opcode *= 0x10;
            opcode += n;
            ta = ( cpu.D0 + n + 1 ) & ADDRESS_MASK;
            cpu.carry = ( ta < cpu.D0 );
            cpu.D0 = ta;
            break;
        case 0x7: /* D1=D1+n+1 */
            n = FetchNibble( cpu.PC );
            cpu.PC++;
            opcode *= 0x10;
            opcode += n;
            ta = ( cpu.D1 + n + 1 ) & ADDRESS_MASK;
            cpu.carry = ( ta < cpu.D1 );
            cpu.D1 = ta;
            break;
        case 0x8: /* D0=D0-(n+1) */
            n = FetchNibble( cpu.PC );
            cpu.PC++;
            opcode *= 0x10;
            opcode += n;
            ta = ( cpu.D0 - n - 1 ) & ADDRESS_MASK;
            cpu.carry = ( ta > cpu.D0 );
            cpu.D0 = ta;
            break;
        case 0x9: /* D0=(2) nn */
            FetchD( &cpu.D0, 2 );
            break;
        case 0xA: /* D0=(4) nn */
            FetchD( &cpu.D0, 4 );
            break;
        case 0xB: /* D0=(5) nn */
            FetchD( &cpu.D0, 5 );
            break;
        case 0xC: /* D1=D1-(n+1) */
            n = FetchNibble( cpu.PC );
            cpu.PC++;
            opcode *= 0x10;
            opcode += n;
            ta = ( cpu.D1 - n - 1 ) & ADDRESS_MASK;
            cpu.carry = ( ta > cpu.D1 );
            cpu.D1 = ta;
            break;
        case 0xD: /* D1=(2) nn */
            FetchD( &cpu.D1, 2 );
            break;
        case 0xE: /* D1=(4) nn */
            FetchD( &cpu.D1, 4 );
            break;
        case 0xF: /* D1=(5) nn */
            FetchD( &cpu.D1, 5 );
            break;

        default:
            /* Unknown opcode */
            ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, cpu.PC, n )
            break;
    }
}

/* Instruction Group_808 */
static void ExecGroup_808( void )
{
    Nibble n = FetchNibble( cpu.PC );
    cpu.PC++;
    opcode *= 0x10;
    opcode += n;

    switch ( n ) {
        case 0x0: /* INTON */
            /* Enable maskable interrupts */
            cpu.int_enable = true;
            break;
        case 0x1: /* RSI */
            /* Discard last nibble of RSI opcode */
            cpu.PC++;

            KeybRSI();
            break;
        case 0x2: /* LA(m) n..n */
            n = FetchNibble( cpu.PC );
            cpu.PC++;
            FetchR( cpu.A, n );
            break;
        case 0x3: /* BUSCB */

            WARNING( CPU_CHF_MODULE_ID, CPU_F_INTERR, "BUSCB" )
            break;
        case 0x4: /* ABIT=0 d */
            n = FetchNibble( cpu.PC );
            cpu.PC++;
            ExecBIT0( cpu.A, n );
            break;
        case 0x5: /* ABIT=1 d */
            n = FetchNibble( cpu.PC );
            cpu.PC++;
            ExecBIT1( cpu.A, n );
            break;
        case 0x6: /* ?ABIT=0 d */
            n = FetchNibble( cpu.PC );
            cpu.PC++;
            TestBIT0( cpu.A, n );
            ExecGOYES_RTNYES();
            break;
        case 0x7: /* ?ABIT=1 d */
            n = FetchNibble( cpu.PC );
            cpu.PC++;
            TestBIT1( cpu.A, n );
            ExecGOYES_RTNYES();
            break;
        case 0x8: /* CBIT=0 d */
            n = FetchNibble( cpu.PC );
            cpu.PC++;
            ExecBIT0( cpu.C, n );
            break;
        case 0x9: /* CBIT=1 d */
            n = FetchNibble( cpu.PC );
            cpu.PC++;
            ExecBIT1( cpu.C, n );
            break;
        case 0xA: /* ?CBIT=0 d */
            n = FetchNibble( cpu.PC );
            cpu.PC++;
            TestBIT0( cpu.C, n );
            ExecGOYES_RTNYES();
            break;
        case 0xB: /* ?CBIT=1 d */
            n = FetchNibble( cpu.PC );
            cpu.PC++;
            TestBIT1( cpu.C, n );
            ExecGOYES_RTNYES();
            break;
        case 0xC: /* PC=(A) */
            cpu.PC = Get5NibblesAbs( R2Addr( cpu.A ) );
            break;
        case 0xD: /* BUSCD */

            WARNING( CPU_CHF_MODULE_ID, CPU_F_INTERR, "BUSCD" )
            break;
        case 0xE: /* PC=(C) */
            cpu.PC = Get5NibblesAbs( R2Addr( cpu.C ) );
            break;
        case 0xF: /* INTOFF */
            cpu.int_enable = false;
            break;

        default:
            /* Unknown opcode */
            ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, cpu.PC, n )
            break;
    }
}

/* Instruction Group_80 */
static void ExecGroup_80( void )
{
    Nibble n = FetchNibble( cpu.PC );
    cpu.PC++;
    opcode *= 0x10;
    opcode += n;

    switch ( n ) {
        case 0x0: /* OUT=CS */
            cpu.OUT = ( ( OutputRegister )cpu.C[ 0 ] ) | ( cpu.OUT & 0xFF0 );
            break;
        case 0x1: /* OUT=C */
            cpu.OUT = ( ( OutputRegister )cpu.C[ 0 ] ) | ( ( OutputRegister )cpu.C[ 1 ] << 4 ) | ( ( OutputRegister )cpu.C[ 2 ] << 8 );
            break;
        case 0x2: /* A=IN */
            ExecIN( cpu.A );
            break;
        case 0x3: /* C=IN */
            ExecIN( cpu.C );
            break;
        case 0x4: /* UNCNFG */
            ModUnconfig( R2Addr( cpu.C ) );
            break;
        case 0x5: /* CONFIG */
            ModConfig( R2Addr( cpu.C ) );
            break;
        case 0x6: /* C=ID */
            Addr2R( cpu.C, ModGetID() );
            break;
        case 0x7: /* SHUTDN */
            ExecSHUTDN();
            break;
        case 0x8: /* Group 808 */
            ExecGroup_808();
            break;
        case 0x9: /* C+P+1 */
            AddRImm( cpu.C, FS_A, cpu.P );
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
            n = FetchNibble( cpu.PC );
            cpu.PC++;
            cpu.C[ ( int )n ] = cpu.P;
            break;
        case 0xD: /* P=C n */
            n = FetchNibble( cpu.PC );
            cpu.PC++;
            SetP( cpu.C[ ( int )n ] );
            break;
        case 0xE: /* SREQ? */

            WARNING( CPU_CHF_MODULE_ID, CPU_F_INTERR, "SREQ" )
            break;
        case 0xF: /* CPEX */
            {
                Nibble tmp = cpu.P;
                n = FetchNibble( cpu.PC );
                cpu.PC++;
                opcode *= 0x10;
                opcode += n;
                SetP( cpu.C[ ( int )n ] );
                cpu.C[ ( int )n ] = tmp;
            }
            break;

        default:
            ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, cpu.PC, n )
            break;
    }
}

/* Special functions Group_81 */
static void ExecSpecialGroup_81( int rp )
{
    Nibble n, f, m;
    int rn, ac;

    switch ( rp ) {
        case 0x0: /* r=r+-CON fs, d */
            f = FetchNibble( cpu.PC );
            cpu.PC++;
            opcode *= 0x10;
            opcode += f;
            n = FetchNibble( cpu.PC );
            cpu.PC++;
            opcode *= 0x10;
            opcode += n;
            m = FetchNibble( cpu.PC );
            cpu.PC++;
            opcode *= 0x10;
            opcode += m;
            rp = GET_RP( n );

            if ( GET_AS( n ) ) /* Subtract */
                SubRImm( reg_pair_0[ rp ], f, m );
            else /* Add */
                AddRImm( reg_pair_0[ rp ], f, m );
            break;
        case 0x1: /* rSRB.f fs */
            f = FetchNibble( cpu.PC );
            cpu.PC++;
            opcode *= 0x10;
            opcode += f;
            n = FetchNibble( cpu.PC );
            cpu.PC++;
            opcode *= 0x10;
            opcode += n;
            rp = GET_RP( n );
            ShiftRightBitR( reg_pair_0[ rp ], f );
            break;
        case 0x2: /* Rn=r.F fs, r=R0.F fs, rRnEX.F fs */
            f = FetchNibble( cpu.PC );
            cpu.PC++;
            opcode *= 0x10;
            opcode += f;
            n = FetchNibble( cpu.PC );
            cpu.PC++;
            opcode *= 0x10;
            opcode += n;
            m = FetchNibble( cpu.PC );
            cpu.PC++;
            opcode *= 0x10;
            opcode += m;
            rn = GET_Rn( m );
            ac = GET_AC( m );

            switch ( n ) {
                case 0x0: /* Rn=r.F fs */
                    CopyRR( cpu.R[ rn ], ( ac ? cpu.C : cpu.A ), f );
                    break;

                case 0x1: /* r=R0.F fs */
                    CopyRR( ( ac ? cpu.C : cpu.A ), cpu.R[ rn ], f );
                    break;

                case 0x2: /* rRnEX.F fs */
                    ExchRR( ( ac ? cpu.C : cpu.A ), cpu.R[ rn ], f );
                    break;

                default:
                    ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, cpu.PC, n )
                    break;
            }
            break;
        case 0x3: /* Group 81B */
            n = FetchNibble( cpu.PC );
            cpu.PC++;
            opcode *= 0x10;
            opcode += n;
            switch ( n ) {
                case 0x1:
                    if ( config.big_screen && config.enable_BUSCC )
                        DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "//TODO: LOOP2 (RPL2 80B00)" )
                    else
                        ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, cpu.PC, n )
                    break;

                case 0x2: /* PC=A */
                    cpu.PC = R2Addr( cpu.A );
                    break;

                case 0x3: /* PC=C */
                    cpu.PC = R2Addr( cpu.C );
                    break;

                case 0x4: /* A=PC */
                    Addr2R( cpu.A, cpu.PC );
                    break;

                case 0x5: /* C=PC */
                    Addr2R( cpu.C, cpu.PC );
                    break;

                case 0x6: /* APCEX */
                    {
                        Address t;
                        t = R2Addr( cpu.A );
                        Addr2R( cpu.A, cpu.PC );
                        cpu.PC = t;
                        break;
                    }

                case 0x7: /* CPCEX */
                    {
                        Address t;
                        t = R2Addr( cpu.C );
                        Addr2R( cpu.C, cpu.PC );
                        cpu.PC = t;
                        break;
                    }

                default:
                    ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, cpu.PC, n )
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
    Nibble n = FetchNibble( cpu.PC );
    cpu.PC++;
    opcode *= 0x10;
    opcode += n;
    Address addr;
    int oc, rp;

    switch ( n ) {
        case 0x0:
            ExecGroup_80();
            break;
        case 0x1: /* rSLC, rSRC, rSRB, Special Group_81 */
            n = FetchNibble( cpu.PC );
            cpu.PC++;
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
            cpu.HST &= ~FetchNibble( cpu.PC );
            cpu.PC++;
            break;
        case 0x3: /* ?HS=0 */
            n = FetchNibble( cpu.PC );
            cpu.PC++;
            opcode *= 0x10;
            opcode += n;
            cpu.carry = ( ( cpu.HST & n ) == 0 );
            ExecGOYES_RTNYES();
            break;
        case 0x4: /* ST=0 n */
            n = FetchNibble( cpu.PC );
            cpu.PC++;
            cpu.ST &= ~st_bit_mask[ ( int )n ];
            break;
        case 0x5: /* ST=1 n */
            n = FetchNibble( cpu.PC );
            cpu.PC++;
            cpu.ST |= st_bit_mask[ ( int )n ];
            break;
        case 0x6: /* ?ST=0 n */
            n = FetchNibble( cpu.PC );
            cpu.PC++;
            cpu.carry = ( ( cpu.ST & st_bit_mask[ ( int )n ] ) == 0 );
            ExecGOYES_RTNYES();
            break;
        case 0x7: /* ?ST=1 n */
            n = FetchNibble( cpu.PC );
            cpu.PC++;
            cpu.carry = ( ( cpu.ST & st_bit_mask[ ( int )n ] ) != 0 );
            ExecGOYES_RTNYES();
            break;
        case 0x8: /* ?P#n */
            n = FetchNibble( cpu.PC );
            cpu.PC++;
            cpu.carry = ( cpu.P != n );
            ExecGOYES_RTNYES();
            break;
        case 0x9: /* ?P=n */
            n = FetchNibble( cpu.PC );
            cpu.PC++;
            cpu.carry = ( cpu.P == n );
            ExecGOYES_RTNYES();
            break;
        case 0xA: /* Test */
            ExecTest_8A();
            break;
        case 0xB: /* Test */
            ExecTest_8B();
            break;
        case 0xC: /* GOLONG */
            addr = Get4Nibbles2C( cpu.PC );
            cpu.PC += addr;
            break;
        case 0xD: /* GOVLNG */
            cpu.PC = Get5NibblesAbs( cpu.PC );
            break;
        case 0xE: /* GOSUBL */
            addr = Get4Nibbles2C( cpu.PC );
            cpu.PC += 4;
            PushRSTK( cpu.PC );
            cpu.PC += addr;
            break;
        case 0xF: /* GOSBVL */
            PushRSTK( cpu.PC + 5 );
            cpu.PC = Get5NibblesAbs( cpu.PC );
            break;

        default:
            ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, cpu.PC, n )
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
    - cpu.return_sp and .reset_req were not reset; this gave troubles
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
        ClearR( cpu.work[ n ], FS_W );

    /* Clear Rn */
    for ( n = 0; n < N_SCRATCH_REGISTER; n++ )
        ClearR( cpu.R[ n ], FS_W );

    /* Clear D0, D1 */
    cpu.D0 = cpu.D1 = ( Address )0;

    /* Clear PC */
    cpu.PC = ( Address )0;

    /* Clear IN, OUT, ST, HST */
    cpu.IN = ( InputRegister )0;
    cpu.OUT = ( OutputRegister )0;
    cpu.ST = ( ProgramStatusRegister )0;
    cpu.HST = ( Nibble )0;

    /* Fill the return stack with (Address)0 */
    cpu.return_sp = 0;
    for ( n = 0; n < RETURN_STACK_SIZE; n++ )
        cpu.return_stack[ n ] = ( Address )0;

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
    cpu.shutdn = cpu.halt = false;

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
    cpu.shutdn = cpu.halt = false;
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
            PushRSTK( cpu.PC );
            cpu.PC = INT_HANDLER_PC;

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
    if ( cpu.shutdn && !cpu.halt ) {
        DEBUG0( CPU_CHF_MODULE_ID, DEBUG_C_INT, CPU_I_WAKE )

        /* Clear SHUTDN flag */
        cpu.shutdn = false;

        /* Clear PC if necessary */
        /* if ( cpu.OUT == (OutputRegister)0 )
           cpu.PC = (Address)0;
         */
    }
}

/* .+

.creation     : 2-Nov-2000
.description  :
  This function makes an halt request to the CPU emulator.

  The halt condition is similar to the shutdn condition, and
  actually forces a shutdn, but it cannot be broken by CpuWake();
  only CpuRunRequest() can do this.  When the CPU is halted:

  - instruction execution is suspended
  - IRQ and NMI service is delayed until the halt condition is broken
  - timers are not updated (they will be resynchronized later)
  - GUI events are handled

  Multiple calls to CpuHaltRequest()/CpuRunRequest() can be nested;
  the CPU remains halted as long as there are more than zero pending
  halt requests.

  CpuHaltRequest() relies on the presence of a suitable handler
  of the CPU_I_SHUTDN condition; this is currently true only
  if CpuHaltRequest() is invoked when the emulator loop is active.

  Notice that setting the CPU_SPIN_SHUTDN build-time option
  in config.h disables all halt requests; both CpuHaltRequest()
  and CpuRunRequest() return -1 in this case.

  The function returns the updated number of pending halt requests,
  or -1 if halt/run requests are disabled; in the latter case,
  the CPU_E_NO_HALT condition is generated and signalled, too.

  The function may never return to the caller if the CPU_SPIN_SHUTDN
  is handled locally by the handler, or if an unwind occurs.

.call         :
                ph = CpuHaltRequest();
.input        :
                void
.output       :
                int ph, updated number of pending halt requests, or
                        -1 if halt/run requests are disabled
.status_codes :
                CPU_I_CALLED
                CPU_I_HALT
                CPU_E_NO_HALT
.notes        :
  3.13, 2-Nov-2000, creation

*/
int CpuHaltRequest( void )
{
    if ( !cpu.halt ) {
        DEBUG0( CPU_CHF_MODULE_ID, DEBUG_C_INT, CPU_I_HALT )

        cpu.halt = true;
        /* CPU must actually be halted: call ExecSHUTDN() to simulate
           the execution of a regular SHUTDN instruction.

           CpuWake() will check .halt before clearing this condition.
        */
        ExecSHUTDN();
    }

    return cpu.halt;
}

/* .+

.creation     : 2-Nov-2000
.description  :
  This function undoes exactly one CpuHaltRequest(); it has no effect
  if the CPU is not halted.  See CpuHaltRequest() for more information.

  The function returns the updated number of pending halt requests,
  or -1 if halt/run requests are disabled; in the latter case,
  the CPU_W_NO_HALT condition is generated, but not signalled, too.

.call         :
                ph = CpuRunRequest();
.input        :
                void
.output       :
                int ph, updated number of pending halt requests, or
                        -1 if halt requests are disabled
.status_codes :
                CPU_I_CALLED
                CPU_I_RUN
                CPU_E_NO_HALT
.notes        :
  3.13, 2-Nov-2000, creation

*/
int CpuRunRequest( void )
{
    if ( cpu.halt ) {
        DEBUG0( CPU_CHF_MODULE_ID, DEBUG_C_INT, CPU_I_RUN )

        cpu.halt = false;
        /* CPU must actually be awoken: call CpuWake() */
        CpuWake();
    }

    return cpu.halt;
}

/* .+

.creation     : 7-Nov-2000
.description  :
  This function return a non-zero value if CpuHaltRequest()
  is allowed, zero otherwise.

.call         :
                s = CpuHaltRequest();
.input        :
                void
.output       :
                int s, non-zero if CpuHaltRequest() is allowed, 0 otherwise
.status_codes :
                CPU_I_CALLED
.notes        :
  3.13, 7-Nov-2000, creation

*/
bool CpuHaltAllowed( void ) { return true; }

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
    Nibble n = FetchNibble( cpu.PC );
    cpu.PC++;
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
            n = FetchNibble( cpu.PC );
            cpu.PC++;
            SetP( n );
            break;
        case 0x3: /* LC(m) n...n */
            n = FetchNibble( cpu.PC );
            cpu.PC++;
            FetchR( cpu.C, n );
            break;
        case 0x4: /* RTNC/GOC */
            if ( cpu.carry ) {
                offset = Get2Nibbles2C( cpu.PC );
                if ( offset == 0 )
                    cpu.PC = PopRSTK();
                else
                    cpu.PC += offset;
            } else
                cpu.PC += 2;

            break;
        case 0x5: /* RTNNC/GONC */
            if ( !cpu.carry ) {
                offset = Get2Nibbles2C( cpu.PC );
                if ( offset == 0 )
                    cpu.PC = PopRSTK();
                else
                    cpu.PC += offset;
            } else
                cpu.PC += 2;

            break;
        case 0x6: /* GOTO */
            cpu.PC += Get3Nibbles2C( cpu.PC );
            break;
        case 0x7: /* GOSUB */
            offset = Get3Nibbles2C( cpu.PC );
            cpu.PC += 3;
            PushRSTK( cpu.PC );
            cpu.PC += offset;
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
            ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, cpu.PC, n )
            break;
    }

    if ( config.debug_level & ( DEBUG_C_IMPLEMENTATION ) ) {
        char dis_opcode[ DISASSEMBLE_OB_SIZE ];
        Disassemble( opcode, dis_opcode );

        // fprintf( stderr, "opcode: %x (%i) => %s\n", opcode, OPCODE_LENGTH( opcode ), dis_opcode );
        fprintf( stderr, "%s\n", dis_opcode );
    }
}
