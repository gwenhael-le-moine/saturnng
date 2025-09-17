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

.creation     :	26-Jan-1998

.description  :
  This module contains a disassembler for the instruction set of the
  Saturn CPU.

  The syntax of the disassembled code conforms to the reference:
        SASM.DOC by HP  (HORN disk 4)
  with some extensions from
        Guide to the Saturn Processor Rev. 0.00f by Matthew Mastracci

  The disassembler is almost not table-driven, because its structure has
  been kept as similar as possible to the actual CPU emulator code.

.notes        :
  $Log: dis.c,v $
  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.10  2000/10/24 16:14:34  cibrario
  Added/Replaced GPL header

  Revision 3.5  2000/10/02 09:44:12  cibrario
  Linux support:
  - gcc does not like array subscripts with type 'char', and it is right.

  Revision 3.1  2000/09/20 13:47:07  cibrario
  Minor updates and fixes to avoid gcc compiler warnings on Solaris
  when -ansi -pedantic -Wall options are selected.

  Revision 1.2  2000/09/07  14:29:39  cibrario
  Bug fix: Incorrect use of strcpy() when disassembling "?HS=0\t%X"

  Revision 1.1  1998/02/13  14:05:30  cibrario
  Initial revision
.- */

#include <stdio.h>
#include <string.h> /* 3.1: strcpy(), strcat(), strlen() */

#include "config.h"
#include "cpu.h"
#include "dis.h"
#include "chf_wrapper.h"
#include "modules.h"

/*---------------------------------------------------------------------------
        Private functions/macros/variables
  ---------------------------------------------------------------------------*/

/* Mnemonics */
static const char* hex_digit[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F" };

static const char* reg_pair[] = { "AB", "BC", "CA", "DC" };

static const char* rn_name[] = { "R0", "R1", "R2", "R3", "R4", "?R5", "?R6", "?R7" };

static const char* field_sel[] = { "P", "WP", "XS", "X", "S", "M", "B", "W", "?[8]", "?[9]", "?[A]", "?[B]", "?[C]", "?[D]", "?[E]", "A" };

static const char* group_0_opc[] = { "RTNSXM", "RTN",  "RTNSC", "RTNCC", "SETHEX", "SETDEC", "RSTK=C", "C=RSTK",
                                     "CLRST",  "C=ST", "ST=C",  "CSTEX", "P=P+1",  "P=P-1",  NULL,     "RTI" };

static const char* group_13_opc[] = { "D0=A",  "D1=A",  "AD0EX", "AD1EX", "D0=C",  "D1=C",  "CD0EX", "CD1EX",
                                      "D0=AS", "D1=AS", "AD0XS", "AD1XS", "D0=CS", "D1=CS", "CD0XS", "CD1XS" };

static const char* group_14_opc[] = { "DAT0=A\tA", "DAT1=A\tA", "A=DAT0\tA", "A=DAT1\tA", "DAT0=C\tA", "DAT1=C\tA",
                                      "C=DAT0\tA", "C=DAT1\tA", "DAT0=A\tB", "DAT1=A\tB", "A=DAT0\tB", "A=DAT1\tB",
                                      "DAT0=C\tB", "DAT1=C\tB", "C=DAT0\tB", "C=DAT1\tB" };

static const char* group_15_opc[] = { "DAT0=A", "DAT1=A", "A=DAT0", "A=DAT1", "DAT0=C", "DAT1=C", "C=DAT0", "C=DAT1" };

static const char* group_80_opc[] = { "OUT=CS", "OUT=C", "A=IN",  "C=IN",  "UNCNFG", "CONFIG", "C=ID",  "SHUTDN",
                                      "?",      "C+P+1", "RESET", "BUSCC", "?",      "?",      "SREQ?", "?" };

static const char* group_81B_opc[] = { "?", "?", "PC=A", "PC=C", "A=PC", "C=PC", "APCEX", "CPCEX", "?", "?", "?", "?", "?", "?", "?", "?" };

/* Read two nibbles in two-complement form, starting from pc, */
static Address Get2Nibbles2C( Address pc )
{
    Address v = ( Address )FetchNibble( pc ) | ( ( Address )FetchNibble( pc + 1 ) << 4 );

    return ( v & 0x80 ) ? v - 0x100 : v;
}

/* Read three nibbles in two-complement form, starting from pc, */
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

/* Read five nibbles in absolute form, starting from pc */
static Address Get5NibblesAbs( Address pc )
{
    Address v = ( Address )FetchNibble( pc ) | ( ( Address )FetchNibble( pc + 1 ) << 4 ) | ( ( Address )FetchNibble( pc + 2 ) << 8 ) |
                ( ( Address )FetchNibble( pc + 3 ) << 12 ) | ( ( Address )FetchNibble( pc + 4 ) << 16 );

    return v;
}

/* Disassemble Hex constant, starting from 'start', for 'm' nibbles;
   returns the address of the next instruction
*/
static Address DisHexConstant( Address start, char* ob, int m )
{
    for ( int i = 0; i < m; i++ )
        strcat( ob, hex_digit[ ( int )FetchNibble( start + m - i - 1 ) ] );

    return start + m;
}

/* GOYES/RTNYES */
static Address DisGOYES_RTNYES( Address pc, char* ob )
{
    Address offset = Get2Nibbles2C( pc );

    /* Decode RTNYES/GOYES */
    ob += strlen( ob );

    if ( offset == 0 )
        sprintf( ob, "\n\tRTNYES" );
    else
        sprintf( ob, "\n\tGOYES\tA_%05X\t* Offset [%d]d", pc + offset, offset );

    /* Skip offset */
    return pc + 2;
}

/* Disassemble field selector */
static void DisFIELD_SEL( int fs, char* ob )
{
    ob += strlen( ob );
    sprintf( ob, "\t%s", field_sel[ fs ] );
}

/* Disassemble immediate field selector */
static void DisIMM_FIELD_SEL( int fs, char* ob )
{
    ob += strlen( ob );
    sprintf( ob, "\t%d", fs + 1 );
}

/* P=n, opcode 2n, length 2 */
static Address DisPEqn( Address pc, char* ob )
{
    sprintf( ob, "P=%d", FetchNibble( pc ) );
    pc++;
    return pc;
}

/* LC(m) n..n, opcode 3xn..n, length 3+m, x=m-1 */
static Address DisLC( Address pc, char* ob )
{
    Nibble m = FetchNibble( pc ) + 1;
    pc++;

    sprintf( ob, "LC(%d)\t", m );
    return DisHexConstant( pc, ob, m );
}

/* RTNC, GOC, opcode 4xx, length 3 - Special case: NOP3 */
static Address DisRTNC_GOC( Address pc, char* ob )
{
    Address offset = Get2Nibbles2C( pc );

    if ( offset == 0 )
        /* RTNC */
        sprintf( ob, "RTNC" );
    else if ( offset == 2 )
        /* NOP3 */
        sprintf( ob, "NOP3" );
    else
        /* GOC */
        sprintf( ob, "GOC\tA_%05X\t* Offset [%d]d", pc + offset, offset );

    /* Skip offset */
    return pc + 2;
}

/* RTNNC, GONC, opcode 5xx, length 3 */
static Address DisRTNNC_GONC( Address pc, char* ob )
{
    Address offset = Get2Nibbles2C( pc );

    if ( offset == 0 )
        /* RTNNC */
        sprintf( ob, "RTNNC" );
    else
        /* GONC */
        sprintf( ob, "GONC\tA_%05X\t* Offset [%d]d", pc + offset, offset );

    /* Skip offset */
    return pc + 2;
}

/* GOTO, opcode 6xxx, length 4 - Special cases: NOP4, NOP5 (p.holder) */
static Address DisGOTO( Address pc, char* ob )
{
    Address offset = Get3Nibbles2C( pc );

    if ( offset == 3 ) {
        /* NOP4 */
        sprintf( ob, "NOP4" );
        pc += 3;
    } else if ( offset == 4 ) {
        /* NOP5 */
        sprintf( ob, "NOP5" );
        pc += 4;
    } else {
        sprintf( ob, "GOTO\tA_%05X\t* Offset [%d]d", pc + offset, offset );
        pc += 3;
    }

    return pc;
}

/* GOSUB, opcode 7xxx, length 4 */
static Address DisGOSUB( Address pc, char* ob )
{
    Address offset = Get3Nibbles2C( pc );
    pc += 3;

    sprintf( ob, "GOSUB\tA_%05X\t* Offset [%d]d", pc + offset, offset );

    return pc;
}

/* ?..., GOYES/RTNYES, Test with Field Selector, opcode 9ftyy, length 5

   FS = f & 0x7
   OC = ((f & 0x8)>>1) | ((t & 0xC)>>2)
   RP = t % 0x3

   FS		Field Selector
   0		P
   1		WP
   2		XS
   3		X
   4		S
   5		M
   6		B
   7		W

   OC		Operation (Test) Code
   0		=
   1		#
   2		=0
   3		#0
   4		>
   5		<
   6		>=
   7		<=

   RP		Register Pair
   0		A,B
   1		B,C
   2		C,A
   3		D,C

   Opcode table
                                f	t	OC	RP
   ---
   A=B, B=A			a	0	0	0
   B=C, C=B			a	1	0	1
   C=A, A=C			a	2	0	2
   D=C, C=D			a	3	0	3
   ---
   A#B, B#A			a	4	1	0
   B#C, C#B			a	5	1	1
   A#C, C#A			a	6	1	2
   C#D, D#C			a	7	1	3
   ---
   A=0				a	8	2	0
   B=0				a	9	2	1
   C=0				a	A	2	2
   D=0				a	B	2	3
   ---
   A#0				a	C	3	0
   B#0				a	D	3	1
   C#0				a	E	3	2
   D#0				a	F	3	3
   ---
   A>B				b	0	4	0
   B>C				b	1	4	1
   C>A				b	2	4	2
   D>C				b	3	4	3
   ---
   A<B				b	4	5	0
   B<C				b	5	5	1
   C<A				b	6	5	2
   D<C				b	7	5	3
   ---
   A>=B				b	8	6	0
   B>=C				b	9	6	1
   C>=A				b	A	6	2
   D>=C				b	B	6	3
   ---
   A<=B				b	C	7	0
   B<=C				b	D	7	1
   C<=A				b	E	7	2
   D<=C				b	F	7	3
   ---
*/
static Address DisTest_9( Address pc, char* ob )
{
    Nibble f = FetchNibble( pc );
    pc++;
    Nibble t = FetchNibble( pc );
    pc++;

    int fs = GET_FS( f );
    int tc = GET_OC_2( f, t );
    int rp = GET_RP( t );

    /* Decode test code */
    switch ( tc ) {
        case 0x0:
            sprintf( ob, "?%c=%c", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 1 ] );
            break;

        case 0x1:
            sprintf( ob, "?%c#%c", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 1 ] );
            break;

        case 0x2:
            sprintf( ob, "?%c=0", reg_pair[ rp ][ 0 ] );
            break;

        case 0x3:
            sprintf( ob, "?%c#0", reg_pair[ rp ][ 0 ] );
            break;

        case 0x4:
            sprintf( ob, "?%c>%c", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 1 ] );
            break;

        case 0x5:
            sprintf( ob, "?%c<%c", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 1 ] );
            break;

        case 0x6:
            sprintf( ob, "?%c>=%c", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 1 ] );
            break;

        case 0x7:
            sprintf( ob, "?%c<=%c", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 1 ] );
            break;

        default:
            FATAL( CPU_CHF_MODULE_ID, CPU_F_INTERR, "Bad_Test_Code" )
            break;
    }

    /* Decode field selector */
    DisFIELD_SEL( fs, ob );

    /* Decode RTNYES/GOYES */
    return DisGOYES_RTNYES( pc, ob );
}

/* ..., Register Operation with Field Selector, opcode Afo, length 3

   FS = f & 0x7
   OC = ((f & 0x8)>>1) | ((o & 0xC)>>2)
   RP = o % 0x3

   FS		Field Selector
   0		P
   1		WP
   2		XS
   3		X
   4		S
   5		M
   6		B
   7		W

   OC		Operation Code
   0		Add
   1		Double register
   2		Add, exchanging register pair
   3		Decrement register
   4		Clear register
   5		Copy register
   6		Copy register, exchanging register pair
   7		Exchange register pair contents

   RP		Register Pair
   0		A,B
   1		B,C
   2		C,A
   3		D,C

   Opcode table
                                f	o	OC	RP
   ---
   A=A+B			a	0	0	0
   B=B+C			a	1	0	1
   C=C+A			a	2	0	2
   D=D+C			a	3	0	3
   ---
   A=A+A			a	4	1	0
   B=B+B			a	5	1	1
   C=C+C			a	6	1	2
   D=D+D			a	7	1	3
   ---
   B=B+A			a	8	2	0
   C=C+B			a	9	2	1
   A=A+C			a	A	2	2
   C=C+D			a	B	2	3
   ---
   A=A-1			a	C	3	0
   B=B-1			a	D	3	1
   C=C-1			a	E	3	2
   D=D-1			a	F	3	3
   ---
   A=0				b	0	4	0
   B=0				b	1	4	1
   C=0				b	2	4	2
   D=0				b	3	4	3
   ---
   A=B				b	4	5	0
   B=C				b	5	5	1
   C=A				b	6	5	2
   D=C				c	7	5	3
   ---
   B=A				b	8	6	0
   C=B				b	9	6	1
   A=C				b	A	6	2
   C=D				b	B	6	3
   ---
   ABEX				b	C	7	0
   BCEX				b	D	7	1
   ACEX				b	E	7	2
   CDEX				b	F	7	3
   ---

*/
static Address DisRegOp_A( Address pc, char* ob )
{
    Nibble f = FetchNibble( pc );
    pc++;
    Nibble o = FetchNibble( pc );
    pc++;

    int fs = GET_FS( f );
    int oc = GET_OC_2( f, o );
    int rp = GET_RP( o );

    /* Decode operation code */
    switch ( oc ) {
        case 0x0:
            sprintf( ob, "%c=%c+%c", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 1 ] );
            break;

        case 0x1:
            sprintf( ob, "%c=%c+%c", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 0 ] );
            break;

        case 0x2:
            sprintf( ob, "%c=%c+%c", reg_pair[ rp ][ 1 ], reg_pair[ rp ][ 1 ], reg_pair[ rp ][ 0 ] );
            break;

        case 0x3:
            sprintf( ob, "%c=%c-1", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 0 ] );
            break;

        case 0x4:
            sprintf( ob, "%c=0", reg_pair[ rp ][ 0 ] );
            break;

        case 0x5:
            sprintf( ob, "%c=%c", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 1 ] );
            break;

        case 0x6:
            sprintf( ob, "%c=%c", reg_pair[ rp ][ 1 ], reg_pair[ rp ][ 0 ] );
            break;

        case 0x7:
            sprintf( ob, "%c%cEX", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 1 ] );
            break;

        default:
            FATAL( CPU_CHF_MODULE_ID, CPU_F_INTERR, "Bad_Operation_Code" )
            break;
    }

    /* Decode field selector */
    DisFIELD_SEL( fs, ob );

    return pc;
}

/* ..., Register Operation with Field Selector, opcode Bfo, length 3

   FS = f & 0x7
   OC = ((f & 0x8)>>1) | ((o & 0xC)>>2)
   RP = o % 0x3

   FS		FIeld Selector
   0		P
   1		WP
   2		XS
   3		X
   4		S
   5		M
   6		B
   7		W

   OC		Operation Code
   0		Subtract
   1		Increment register
   2		Subtract, exchanging register pair
   3		Subtract, exchanging operands only
   4		Shift register left one nibble
   5		Shift register right one nibble, set sticky bit (SB) if #0
   6		Two's / Ten's complement register
   7		One's complement register

   RP		Register Pair
   0		A,B
   1		B,C
   2		C,A
   3		D,C

   Opcode table
                                f	o	OC	RP
   ---
   A=A-B			a	0	0	0
   B=B-C			a	1	0	1
   C=C-A			a	2	0	2
   D=D-C			a	3	0	3
   ---
   A=A+1			a	4	1	0
   B=B+1			a	5	1	1
   C=C+1			a	6	1	2
   D=D+1			a	7	1	3
   ---
   B=B-A			a	8	2	0
   C=C-B			a	9	2	1
   A=A-C			a	A	2	2
   C=C-D			a	B	2	3
   ---
   A=B-A			a	C	3	0
   B=C-B			a	D	3	1
   C=A-C			a	E	3	2
   D=C-D			a	F	3	3
   ---
   ASL				b	0	4	0
   BSL				b	1	4	1
   CSL				b	2	4	2
   DSL				b	3	4	3
   ---
   ASR				b	4	5	0
   BSR				b	5	5	1
   CSR				b	6	5	2
   DSR				b	7	5	3
   ---
   A=-A				b	8	6	0
   B=-B				b	9	6	1
   C=-C				b	A	6	2
   D=-D				b	B	6	3
   ---
   A=-A-1			b	C	7	0
   B=-B-1			b	D	7	1
   C=-C-1			b	E	7	2
   D=-D-1			b	F	7	3
   ---

*/
static Address DisRegOp_B( Address pc, char* ob )
{
    Nibble f = FetchNibble( pc );
    pc++;
    Nibble o = FetchNibble( pc );
    pc++;

    int fs = GET_FS( f );
    int oc = GET_OC_2( f, o );
    int rp = GET_RP( o );

    /* Decode operation code */
    switch ( oc ) {
        case 0x0:
            sprintf( ob, "%c=%c-%c", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 1 ] );
            break;

        case 0x1:
            sprintf( ob, "%c=%c+1", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 0 ] );
            break;

        case 0x2:
            sprintf( ob, "%c=%c-%c", reg_pair[ rp ][ 1 ], reg_pair[ rp ][ 1 ], reg_pair[ rp ][ 0 ] );
            break;

        case 0x3:
            sprintf( ob, "%c=%c-%c", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 1 ], reg_pair[ rp ][ 0 ] );
            break;

        case 0x4:
            sprintf( ob, "%cSL", reg_pair[ rp ][ 0 ] );
            break;

        case 0x5:
            sprintf( ob, "%cSR", reg_pair[ rp ][ 0 ] );
            break;

        case 0x6:
            sprintf( ob, "%c=-%c", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 0 ] );
            break;

        case 0x7:
            sprintf( ob, "%c=-%c-1", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 0 ] );
            break;

        default:
            FATAL( CPU_CHF_MODULE_ID, CPU_F_INTERR, "Bad_Operation_Code" )
            break;
    }

    /* Decode field selector */
    DisFIELD_SEL( fs, ob );

    return pc;
}

/* ..., Register Operation on A Fields, opcode Co, length 2

   FS = implicit, always A
   OC = ((o & 0xC)>>2)
   RP = o % 0x3

   OC		Operation Code
   0		Add
   1		Double register
   2		Add, exchanging register pair
   3		Decrement register

   RP		Register Pair
   0		A,B
   1		B,C
   2		C,A
   3		D,C

   Opcode table
                                o	OC	RP
   ---
   A=A+B			0	0	0
   B=B+C			1	0	1
   C=C+A			2	0	2
   D=D+C			3	0	3
   ---
   A=A+A			4	1	0
   B=B+B			5	1	1
   C=C+C			6	1	2
   D=D+D			7	1	3
   ---
   B=B+A			8	2	0
   C=C+B			9	2	1
   A=A+C			A	2	2
   C=C+D			B	2	3
   ---
   A=A-1			C	3	0
   B=B-1			D	3	1
   C=C-1			E	3	2
   D=D-1			F	3	3
   ---
*/
static Address DisRegOp_C( Address pc, char* ob )
{
    Nibble o = FetchNibble( pc );
    pc++;

    int oc = GET_OC_1( o );
    int rp = GET_RP( o );

    /* Decode operation code */
    switch ( oc ) {
        case 0x0:
            sprintf( ob, "%c=%c+%c", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 1 ] );
            break;

        case 0x1:
            sprintf( ob, "%c=%c+%c", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 0 ] );
            break;

        case 0x2:
            sprintf( ob, "%c=%c+%c", reg_pair[ rp ][ 1 ], reg_pair[ rp ][ 1 ], reg_pair[ rp ][ 0 ] );
            break;

        case 0x3:
            sprintf( ob, "%c=%c-1", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 0 ] );
            break;

        default:
            FATAL( CPU_CHF_MODULE_ID, CPU_F_INTERR, "Bad_Operation_Code" )
            break;
    }

    /* Decode field selector */
    DisFIELD_SEL( FS_A, ob );

    return pc;
}

/* ..., Register Operation on A Fields, opcode Do, length 2

   FS = implicit, always A
   OC = ((o & 0xC)>>2)
   RP = o % 0x3

   OC		Operation Code
   0		Clear register
   1		Copy register
   2		Copy register, exchanging register pair
   3		Exchange register pair contents

   RP		Register Pair
   0		A,B
   1		B,C
   2		C,A
   3		D,C

   Opcode table
                                o	OC	RP
   ---
   A=0				0	0	0
   B=0				1	0	1
   C=0				2	0	2
   D=0				3	0	3
   ---
   A=B				4	1	0
   B=C				5	1	1
   C=A				6	1	2
   D=C				7	1	3
   ---
   B=A				8	2	0
   C=B				9	2	1
   A=C				A	2	2
   C=D				B	2	3
   ---
   ABEX				C	3	0
   BCEX				D	3	1
   ACEX				E	3	2
   CDEX				F	3	3

*/
static Address DisRegOp_D( Address pc, char* ob )
{
    Nibble o = FetchNibble( pc );
    pc++;

    int oc = GET_OC_1( o );
    int rp = GET_RP( o );

    /* Decode operation code */
    switch ( oc ) {
        case 0x0:
            sprintf( ob, "%c=0", reg_pair[ rp ][ 0 ] );
            break;

        case 0x1:
            sprintf( ob, "%c=%c", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 1 ] );
            break;

        case 0x2:
            sprintf( ob, "%c=%c", reg_pair[ rp ][ 1 ], reg_pair[ rp ][ 0 ] );
            break;

        case 0x3:
            sprintf( ob, "%c%cEX", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 1 ] );
            break;

        default:
            FATAL( CPU_CHF_MODULE_ID, CPU_F_INTERR, "Bad_Operation_Code" )
            break;
    }

    /* Decode field selector */
    DisFIELD_SEL( FS_A, ob );

    return pc;
}

/* ..., Register Operation on A Fields, opcode Eo, length 2

   FS = implicit, always A
   OC = ((o & 0xC)>>2)
   RP = o % 0x3

   OC		Operation Code
   0		Subtract
   1		Increment register
   2		Subtract, exchanging register pair
   3		Subtract, exchanging operands only

   RP		Register Pair
   0		A,B
   1		B,C
   2		C,A
   3		D,C

   Opcode table
                                o	OC	RP
   ---
   A=A-B			0	0	0
   B=B-C			1	0	1
   C=C-A			2	0	2
   D=D-C			3	0	3
   ---
   A=A+1			4	1	0
   B=B+1			5	1	1
   C=C+1			6	1	2
   D=D+1			7	1	3
   ---
   B=B-A			8	2	0
   C=C-B			9	2	1
   A=A-C			A	2	2
   C=C-D			B	2	3
   ---
   A=B-A			C	3	0
   B=C-B			D	3	1
   C=A-C			E	3	2
   D=C-D			F	3	3
   ---

*/
static Address DisRegOp_E( Address pc, char* ob )
{
    Nibble o = FetchNibble( pc );
    pc++;

    int oc = GET_OC_1( o );
    int rp = GET_RP( o );

    /* Decode operation code */
    switch ( oc ) {
        case 0x0:
            sprintf( ob, "%c=%c-%c", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 1 ] );
            break;

        case 0x1:
            sprintf( ob, "%c=%c+1", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 0 ] );
            break;

        case 0x2:
            sprintf( ob, "%c=%c-%c", reg_pair[ rp ][ 1 ], reg_pair[ rp ][ 1 ], reg_pair[ rp ][ 0 ] );
            break;

        case 0x3:
            sprintf( ob, "%c=%c-%c", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 1 ], reg_pair[ rp ][ 0 ] );
            break;

        default:
            FATAL( CPU_CHF_MODULE_ID, CPU_F_INTERR, "Bad_Operation_Code" )
            break;
    }

    /* Decode field selector */
    DisFIELD_SEL( FS_A, ob );

    return pc;
}

/* ..., Register Operation on A Fields, opcode Fo, length 2

   FS = implicit, always A
   OC = ((o & 0xC)>>2)
   RP = o % 0x3

   OC		Operation Code
   0		Shift register left one nibble
   1		Shift register right one nibble, set sticky bit (SB) if #0
   2		Two's / Ten's complement register
   3		One's complement register

   RP		Register Pair
   0		A,B
   1		B,C
   2		C,A
   3		D,C

   Opcode table
                                o	OC	RP
   ---
   ASL				0	0	0
   BSL				1	0	1
   CSL				2	0	2
   DSL				3	0	3
   ---
   ASR				4	1	0
   BSR				5	1	1
   CSR				6	1	2
   DSR				7	1	3
   ---
   A=-A				8	2	0
   B=-B				9	2	1
   C=-C				A	2	2
   D=-D				B	2	3
   ---
   A=-A-1			C	3	0
   B=-B-1			D	3	1
   C=-C-1			E	3	2
   D=-D-1			F	3	3
   ---

*/
static Address DisRegOp_F( Address pc, char* ob )
{
    Nibble o = FetchNibble( pc );
    pc++;

    int oc = GET_OC_1( o );
    int rp = GET_RP( o );

    /* Decode operation code */
    switch ( oc ) {
        case 0x0:
            sprintf( ob, "%cSL", reg_pair[ rp ][ 0 ] );
            break;

        case 0x1:
            sprintf( ob, "%cSR", reg_pair[ rp ][ 0 ] );
            break;

        case 0x2:
            sprintf( ob, "%c=-%c", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 0 ] );
            break;

        case 0x3:
            sprintf( ob, "%c=-%c-1", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 0 ] );
            break;

        default:
            FATAL( CPU_CHF_MODULE_ID, CPU_F_INTERR, "Bad_Operation_Code" )
            break;
    }

    /* Decode field selector */
    DisFIELD_SEL( FS_A, ob );

    return pc;
}

/* .&., .!., AND/OR Operations, opcode 0Efo, length 4

   FS = f
   OC = ((o & 0xC)>>2)
   RP = o % 0x3

   FS		FIeld Selector
   0		P
   1		WP
   2		XS
   3		X
   4		S
   5		M
   6		B
   7		W
   8		A

   OC		Operation Code
   0		And
   1		And, exchanging register pair
   2		Or
   3		Or, exchanging register pair

   RP		Register Pair
   0		A,B
   1		B,C
   2		C,A
   3		D,C

   Opcode table
                                f	o	OC	RP
   ---
   A=A&B			a	0	0	0
   B=B&C			a	1	0	1
   C=C&A			a	2	0	2
   D=D&C			a	3	0	3
   ---
   B=B&A			a	4	1	0
   C=C&B			a	5	1	1
   A=A&C			a	6	1	2
   C=C&D			a	7	1	3
   ---
   A=A!B			a	8	2	0
   B=B!C			a	9	2	1
   C=C!A			a	A	2	2
   D=D!C			a	B	2	3
   ---
   B=B!A			a	C	3	0
   C=C!B			a	D	3	1
   A=A!C			a	E	3	2
   C=C!D			a	F	3	3
   ---

*/
static Address DisAND_OR( Address pc, char* ob )
{
    Nibble f = FetchNibble( pc );
    pc++;
    Nibble o = FetchNibble( pc );
    pc++;

    int oc = GET_OC_1( o );
    int rp = GET_RP( o );

    /* Decode operation code */
    switch ( oc ) {
        case 0x0:
            sprintf( ob, "%c=%c&%c", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 1 ] );
            break;

        case 0x1:
            sprintf( ob, "%c=%c&%c", reg_pair[ rp ][ 1 ], reg_pair[ rp ][ 1 ], reg_pair[ rp ][ 0 ] );
            break;

        case 0x2:
            sprintf( ob, "%c=%c!%c", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 1 ] );
            break;

        case 0x3:
            sprintf( ob, "%c=%c!%c", reg_pair[ rp ][ 1 ], reg_pair[ rp ][ 1 ], reg_pair[ rp ][ 0 ] );
            break;

        default:
            FATAL( CPU_CHF_MODULE_ID, CPU_F_INTERR, "Bad_Operation_Code" )
            break;
    }

    /* Decode field selector */
    DisFIELD_SEL( f, ob );

    return pc;
}

/* Instruction Group_0
   Prefix 0E introduces AND/OR opcodes
*/
static Address DisGroup_0( Address pc, char* ob )
{
    Nibble n = FetchNibble( pc );
    pc++;

    switch ( n ) {
        case 0x0:
            /* RTNSXM */
        case 0x1:
            /* RTN */
        case 0x2:
            /* RTNSC */
        case 0x3:
            /* RTNCC */
        case 0x4:
            /* SETHEX */
        case 0x5:
            /* SETDEC */
        case 0x6:
            /* RSTK=C */
        case 0x7:
            /* C=RSTK */
        case 0x8:
            /* CLRST */
        case 0x9:
            /* C=ST */
        case 0xA:
            /* ST=C */
        case 0xB:
            /* CSTEX */
        case 0xC:
            /* P=P+1 */
        case 0xD:
            /* P=P-1 */
        case 0xF:
            /* RTI */
            strcpy( ob, group_0_opc[ ( int )n ] );
            break;

        case 0xE:
            /* AND_OR */
            pc = DisAND_OR( pc, ob );
            break;

        default:
            /* Unknown opcode */
            strcpy( ob, "?" );

            ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, pc, n )
            break;
    }

    return pc;
}

/* Instruction Group_1

   Opcode table for:
     10r	Rn = A/C (Copy A/C into R0..R4)
     11r	A/C = Rn (Copy R0..R4 into A/C)
     12r	ARnEX, CRnEX (Exchange A/C and R0..R4)

   RN = r & 0x7
   AC = r & 0x8
                                Opcode		RN	AC
   ---
   R0=A				100		0	0
   R1=A				101		1	0
   R2=A				102		2	0
   R3=A				103		3	0
   R4=A				104		4	0
   ---
   R0=C				108		0	8
   R1=C				109		1	8
   R2=C				10A		2	8
   R3=C				10B		3	8
   R4=C				10C		4	8
   ---
   A=R0				110		0	0
   A=R1				111		1	0
   A=R2				112		2	0
   A=R3				113		3	0
   A=R4				114		4	0
   ---
   C=R0				118		0	8
   C=R1				119		1	8
   C=R2				11A		2	8
   C=R3				11B		3	8
   C=R4				11C		4	8
   ---
   AR0EX			120		0	0
   AR1EX			121		1	0
   AR2EX			122		2	0
   AR3EX			123		3	0
   AR4EX			124		4	0
   ---
   CR0EX			128		0	8
   CR1EX			129		1	8
   CR2EX			12A		2	8
   CR3EX			12B		3	8
   CR4EX			12C		4	8

   Opcode table for:
     13r
                                Opcode
   ---
   D0=A				130
   D1=A				131
   AD0EX			132
   AD1EX			133
   D0=C				134
   D1=C				135
   CD0EX			136
   CD1EX			137
   D0=AS			138
   D1=AS			139
   AD0XS			13A
   AD1XS			13B
   D0=CS			13C
   D1=CS			13D
   CD0XS			13E
   CD1XS			13F
   Opcode table for:
     14r
                                Opcode
   ---
   DAT0=A	A		140
   DAT1=A	A		141
   A=DAT0	A		142
   A=DAT1	A		143
   DAT0=C	A		144
   DAT1=C	A		145
   C=DAT0	A		146
   C=DAT1	A		147
   DAT0=A	B		148
   DAT1=A	B		149
   A=DAT0	B		14A
   A=DAT1	B		14B
   DAT0=C	B		14C
   DAT1=C	B		14D
   C=DAT0	B		14E
   C=DAT1	B		14F
   Opcode table for:
     15of

     OC = o & 0x7
     IS = o & 0x8

                                o	f	OC	IS
   ---
   DAT0=A fs			0	a	0	0
   DAT1=A fs			1	a	1	0
   A=DAT0 fs			2	a	2	0
   A=DAT1 fs			3	a	3	0
   DAT0=C fs			4	a	4	0
   DAT1=C fs			5	a	5	0
   C=DAT0 fs			6	a	6	0
   C=DAT1 fs			7	a	7	0
   DAT0=A d			8	d-1	0	8
   DAT1=A d			9	d-1	1	8
   A=DAT0 d			A	d-1	2	8
   A=DAT1 d			B	d-1	3	8
   DAT0=C d			C	d-1	4	8
   DAT1=C d			D	d-1	5	8
   C=DAT0 d			E	d-1	6	8
   C=DAT1 d			F	d-1	7	8
   Opcode table for:
     16m,	D0=D0+m+1
     17m,	D1=D1+m+1
     18m,	D0=D0-(m+1)
     1Cm,	D1=D1-(m+1)

   Opcode table for:
     19nn	D0=(2)	nn
     1Annnn	D0=(4)	nnnn
     1Bnnnnn	D0=(5)	nnnnn
     1Dnn	D1=(2)	nn
     1Ennnn	D1=(4)	nnnn
     1Fnnnnn	D1=(5)	nnnnn
*/
static Address DisGroup_1( Address pc, char* ob )
{
    Nibble n = FetchNibble( pc );
    pc++;
    Nibble f;
    int rn, ac;
    int oc, is;

    switch ( n ) {
        case 0x0:
            /* Rn=A/C */
            n = FetchNibble( pc );
            pc++;
            rn = GET_Rn( n );
            ac = GET_AC( n );

            sprintf( ob, "%s=%s", rn_name[ rn ], ( ac ? "C" : "A" ) );
            break;

        case 0x1:
            /* A/C=Rn */
            n = FetchNibble( pc );
            pc++;
            rn = GET_Rn( n );
            ac = GET_AC( n );

            sprintf( ob, "%s=%s", ( ac ? "C" : "A" ), rn_name[ rn ] );
            break;

        case 0x2:
            /* ARnEX, CRnEX */
            n = FetchNibble( pc );
            pc++;
            rn = GET_Rn( n );
            ac = GET_AC( n );

            sprintf( ob, "%s%sEX", ( ac ? "C" : "A" ), rn_name[ rn ] );
            break;

        case 0x3:
            /* Copy/Exchange A/C and D0/D1 */
            n = FetchNibble( pc );
            pc++;
            strcpy( ob, group_13_opc[ ( int )n ] );
            break;

        case 0x4:
            /* Load/Store A/C to @D0/@D1, Field selector A or B */
            n = FetchNibble( pc );
            pc++;
            strcpy( ob, group_14_opc[ ( int )n ] );
            break;

        case 0x5:
            /* Load/Store A/C to @D0/@D1, Other Field Selectors */
            n = FetchNibble( pc );
            pc++;
            f = FetchNibble( pc );
            pc++;
            oc = GET_OC_3b( n );
            is = GET_IMMEDIATE_FS_FLAG( n );

            /* Decode operation code */
            strcpy( ob, group_15_opc[ oc ] );

            if ( is )
                /* Immediate field selector */
                DisIMM_FIELD_SEL( f, ob );
            else
                /* Regular field selector */
                DisFIELD_SEL( f, ob );

            break;

        case 0x6:
            /* D0=D0+n+1 */
            n = FetchNibble( pc );
            pc++;
            sprintf( ob, "D0=D0+%d", n + 1 );
            break;

        case 0x7:
            /* D1=D1+n+1 */
            n = FetchNibble( pc );
            pc++;
            sprintf( ob, "D1=D1+%d", n + 1 );
            break;

        case 0x8:
            /* D0=D0-(n+1) */
            n = FetchNibble( pc );
            pc++;
            sprintf( ob, "D0=D0-%d", n + 1 );
            break;

        case 0x9:
            /* D0=(2) nn */
            strcpy( ob, "D0=(2)\t" );
            pc = DisHexConstant( pc, ob, 2 );
            break;

        case 0xA:
            /* D0=(4) nn */
            strcpy( ob, "D0=(4)\t" );
            pc = DisHexConstant( pc, ob, 4 );
            break;

        case 0xB:
            /* D0=(5) nn */
            strcpy( ob, "D0=(5)\t" );
            pc = DisHexConstant( pc, ob, 5 );
            break;

        case 0xC:
            /* D1=D1-(n+1) */
            n = FetchNibble( pc );
            pc++;
            sprintf( ob, "D1=D1-%d", n + 1 );
            break;

        case 0xD:
            /* D1=(2) nn */
            strcpy( ob, "D1=(2)\t" );
            pc = DisHexConstant( pc, ob, 2 );
            break;

        case 0xE:
            /* D1=(4) nn */
            strcpy( ob, "D1=(4)\t" );
            pc = DisHexConstant( pc, ob, 4 );
            break;

        case 0xF:
            /* D1=(5) nn */
            strcpy( ob, "D1=(5)\t" );
            pc = DisHexConstant( pc, ob, 5 );
            break;

        default:
            /* Unknown opcode */
            strcpy( ob, "?" );

            ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, pc, n )
            break;
    }

    return pc;
}

/* Instruction Group_808
 */
static Address DisGroup_808( Address pc, char* ob )
{
    Nibble n = FetchNibble( pc );
    pc++;
    Nibble m;

    switch ( n ) {
        case 0x0:
            /* INTON */
            strcpy( ob, "INTON" );
            break;

        case 0x1:
            /* RSI */
            strcpy( ob, "RSI" );
            pc++;
            break;

        case 0x2:
            /* LA(m) n..n */
            m = FetchNibble( pc ) + 1;
            pc++;
            sprintf( ob, "LA(%d)\t", m );
            pc = DisHexConstant( pc, ob, m );
            break;

        case 0x3:
            /* BUSCB */
            strcpy( ob, "BUSCB" );
            break;

        case 0x4:
            /* ABIT=0 d */
            m = FetchNibble( pc );
            pc++;
            sprintf( ob, "ABIT=0 %d", m );
            break;

        case 0x5:
            /* ABIT=1 d */
            m = FetchNibble( pc );
            pc++;
            sprintf( ob, "ABIT=1 %d", m );
            break;

        case 0x6:
            /* ?ABIT=0 d */
            m = FetchNibble( pc );
            pc++;
            sprintf( ob, "?ABIT=0 %d", m );
            pc = DisGOYES_RTNYES( pc, ob );
            break;

        case 0x7:
            /* ?ABIT=1 d */
            m = FetchNibble( pc );
            pc++;
            sprintf( ob, "?ABIT=1 %d", m );
            pc = DisGOYES_RTNYES( pc, ob );
            break;

        case 0x8:
            /* CBIT=0 d */
            m = FetchNibble( pc );
            pc++;
            sprintf( ob, "CBIT=0 %d", m );
            break;

        case 0x9:
            /* CBIT=1 d */
            m = FetchNibble( pc );
            pc++;
            sprintf( ob, "CBIT=1 %d", m );
            break;

        case 0xA:
            /* ?CBIT=0 d */
            m = FetchNibble( pc );
            pc++;
            sprintf( ob, "?CBIT=0 %d", m );
            pc = DisGOYES_RTNYES( pc, ob );
            break;

        case 0xB:
            /* ?CBIT=1 d */
            m = FetchNibble( pc );
            pc++;
            sprintf( ob, "?CBIT=1 %d", m );
            pc = DisGOYES_RTNYES( pc, ob );
            break;

        case 0xC:
            /* PC=(A) */
            strcpy( ob, "PC=(A)" );
            break;

        case 0xD:
            /* BUSCD */
            strcpy( ob, "BUSCD" );
            break;

        case 0xE:
            /* PC=(C) */
            strcpy( ob, "PC=(C)" );
            break;

        case 0xF:
            /* INTOFF */
            strcpy( ob, "INTOFF" );
            break;

        default:
            /* Unknown opcode */
            strcpy( ob, "?" );

            ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, pc, n )
            break;
    }

    return pc;
}

/* Instruction Group_80
 */
static Address DisGroup_80( Address pc, char* ob )
{
    Nibble n = FetchNibble( pc );
    pc++;

    switch ( n ) {
        case 0x0:
            /* OUT=CS */
        case 0x1:
            /* OUT=C */
        case 0x2:
            /* A=IN */
        case 0x3:
            /* C=IN */
        case 0x4:
            /* UNCNFG */
        case 0x5:
            /* CONFIG */
        case 0x6:
            /* C=ID */
        case 0x7:
            /* SHUTDN */
        case 0x9:
            /* C+P+1 */
        case 0xA:
            /* RESET */
        case 0xB:
            /* BUSCC */
        case 0xE:
            /* SREQ? */
            strcpy( ob, group_80_opc[ ( int )n ] );
            break;

        case 0x8:
            /* Group 808 */
            pc = DisGroup_808( pc, ob );
            break;

        case 0xC:
            /* C=P n */
            n = FetchNibble( pc );
            pc++;
            sprintf( ob, "C=P\t%d", n );
            break;

        case 0xD:
            /* P=C n */
            n = FetchNibble( pc );
            pc++;
            sprintf( ob, "P=C\t%d", n );
            break;

        case 0xF:
            /* CPEX */
            n = FetchNibble( pc );
            pc++;
            sprintf( ob, "CPEX\t%d", n );
            break;

        default:
            /* Unknown opcode */
            strcpy( ob, "?" );

            ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, pc, n )
            break;
    }

    return pc;
}

/* Special functions Group_81

   Opcode Table for:
   rp=0, opcode 818f0m, r=r+-CON rfs, d
   f= field selector
   m= d-1

                        Opcode
   ---
   A=A+CON rfs,d	818f0m
   A=A-CON rfs,d	818f8m
   B=B+CON rfs,d	818f1m
   B=B-CON rfs,d	818f9m
   C=C+CON rfs,d	818f2m
   C=C-CON rfs,d	818fAm
   D=D+CON rfs,d	818f3m
   D=D-CON rfs,d	818fBm
   ---

   Opcode Table for:
   rp=1, opcode 819.., rSRB.F fs
   ---
   ASRB.F fs		819f0
   BSRB.F fs		819f1
   CSRB.F fs		819f2
   DSRB.F fs		819f3
   ---

   rp=2, opcode 81A..
   ---
   R0=A.F fs		81Af00
   R1=A.F fs		81Af01
   R2=A.F fs		81Af02
   R3=A.F fs		81Af03
   R4=A.F fs		81Af04
   ---
   R0=C.F fs		81Af08
   R1=C.F fs		81Af09
   R2=C.F fs		81Af0A
   R3=C.F fs		81Af0B
   R4=C.F fs		81Af0C
   ---
   A=R0.F fs		81Af10
   A=R1.F fs		81Af11
   A=R2.F fs		81Af12
   A=R3.F fs		81Af13
   A=R4.F fs		81Af14
   ---
   C=R0.F fs		81Af18
   C=R1.F fs		81Af19
   C=R2.F fs		81Af1A
   C=R3.F fs		81Af1B
   C=R4.F fs		81Af1C
   ---
   AR0EX.F fs		81Af20
   AR1EX.F fs		81Af21
   AR2EX.F fs		81Af22
   AR3EX.F fs		81Af23
   AR4EX.F fs		81Af24
   ---
   CR0EX.F fs		81Af28
   CR1EX.F fs		81Af29
   CR2EX.F fs		81Af2A
   CR3EX.F fs		81Af2B
   CR4EX.F fs		81Af2C
   Opcode Table for:
   rp=3, opcode 81Bn
                        Direct execution

*/
static Address DisSpecialGroup_81( Address pc, char* ob, int rp )
{
    Nibble n, f, m;
    int rn, ac;

    switch ( rp ) {
        case 0x0:
            /* r=r+-CON fs, d */
            f = FetchNibble( pc );
            pc++;
            n = FetchNibble( pc );
            pc++;
            m = FetchNibble( pc );
            pc++;
            rp = GET_RP( n );

            sprintf( ob, "%c=%c%cCON", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 0 ], ( GET_AS( n ) ? '-' : '+' ) );

            /* Decode field selector */
            DisFIELD_SEL( f, ob );

            /* Decode constant */
            ob += strlen( ob );
            sprintf( ob, ", %d", m + 1 );
            break;

        case 0x1:
            /* rSRB.f fs */
            f = FetchNibble( pc );
            pc++;
            n = FetchNibble( pc );
            pc++;
            rp = GET_RP( n );

            sprintf( ob, "%cSRB.F", reg_pair[ rp ][ 0 ] );

            /* Decode field selector */
            DisFIELD_SEL( f, ob );
            break;

        case 0x2:
            /* Rn=r.F fs, r=R0.F fs, rRnEX.F fs */
            f = FetchNibble( pc );
            pc++;
            n = FetchNibble( pc );
            pc++;
            m = FetchNibble( pc );
            pc++;
            rn = GET_Rn( m );
            ac = GET_AC( m );

            switch ( n ) {
                case 0x0:
                    /* Rn=r.F fs */
                    sprintf( ob, "%s=%s.F", rn_name[ rn ], ( ac ? "C" : "A" ) );
                    DisFIELD_SEL( f, ob );
                    break;

                case 0x1:
                    /* r=R0.F fs */
                    sprintf( ob, "%s=%s.F", ( ac ? "C" : "A" ), rn_name[ rn ] );
                    DisFIELD_SEL( f, ob );
                    break;

                case 0x2:
                    /* rRnEX.F fs */
                    sprintf( ob, "%s%sEX.F", ( ac ? "C" : "A" ), rn_name[ rn ] );
                    DisFIELD_SEL( f, ob );
                    break;

                default:
                    ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, pc, n )
                    break;
            }

            break;

        case 0x3:
            /* Group 81B */
            n = FetchNibble( pc );
            pc++;
            strcpy( ob, group_81B_opc[ ( int )n ] );
            break;

        default:
            FATAL( CPU_CHF_MODULE_ID, CPU_F_INTERR, "Bad_Register_Pair" )
            break;
    }

    return pc;
}

/* ?..., GOYES/RTNYES, Test on A Fields, opcode 8Atyy, length 5

   FS = implicit, always A
   OC = ((t & 0xC)>>2)
   RP = t % 0x3

   OC		Operation (Test) Code
   0		=
   1		#
   2		=0
   3		#0

   RP		Register Pair
   0		A,B
   1		B,C
   2		C,A
   3		D,C

   Opcode table
                                t	OC	RP
   ---
   A=B, B=A			0	0	0
   B=C, C=B			1	0	1
   C=A, A=C			2	0	2
   D=C, C=D			3	0	3
   ---
   A#B, B#A			4	1	0
   B#C, C#B			5	1	1
   A#C, C#A			6	1	2
   C#D, D#C			7	1	3
   ---
   A=0				8	2	0
   B=0				9	2	1
   C=0				A	2	2
   D=0				B	2	3
   ---
   A#0				C	3	0
   B#0				D	3	1
   C#0				E	3	2
   D#0				F	3	3
   ---

*/
static Address DisTest_8A( Address pc, char* ob )
{
    Nibble t = FetchNibble( pc );
    pc++;

    int tc = GET_OC_1( t );
    int rp = GET_RP( t );

    /* Decode test code */
    switch ( tc ) {
        case 0x0:
            sprintf( ob, "?%c=%c", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 1 ] );
            break;

        case 0x1:
            sprintf( ob, "?%c#%c", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 1 ] );
            break;

        case 0x2:
            sprintf( ob, "?%c=0", reg_pair[ rp ][ 0 ] );
            break;

        case 0x3:
            sprintf( ob, "?%c#0", reg_pair[ rp ][ 0 ] );
            break;

        default:
            FATAL( CPU_CHF_MODULE_ID, CPU_F_INTERR, "Bad_Test_Code" )
            break;
    }

    /* Decode field selector */
    DisFIELD_SEL( FS_A, ob );

    /* Decode RTNYES/GOYES */
    return DisGOYES_RTNYES( pc, ob );
}

/* ?..., GOYES/RTNYES, Test on A Fields, opcode 8Btyy, length 5

   FS = implicit, always A
   OC = ((t & 0xC)>>2)
   RP = t % 0x3

   OC		Operation (Test) Code
   0		>
   1		<
   2		>=
   3		<=

   RP		Register Pair
   0		A,B
   1		B,C
   2		C,A
   3		D,C

   Opcode table
                                t	OC	RP
   ---
   A>B				0	0	0
   B>C				1	0	1
   C>A				2	0	2
   D>C				3	0	3
   ---
   A<B				4	1	0
   B<C				5	1	1
   C<A				6	1	2
   D<C				7	1	3
   ---
   A>=B				8	2	0
   B>=C				9	2	1
   C>=A				A	2	2
   D>=C				B	2	3
   ---
   A<=B				C	3	0
   B<=C				D	3	1
   C<=A				E	3	2
   D<=C				F	3	3
   ---

*/
static Address DisTest_8B( Address pc, char* ob )
{
    Nibble t = FetchNibble( pc );
    pc++;

    int tc = GET_OC_1( t );
    int rp = GET_RP( t );

    /* Decode test code */
    switch ( tc ) {
        case 0x0:
            sprintf( ob, "?%c>%c", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 1 ] );
            break;

        case 0x1:
            sprintf( ob, "?%c<%c", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 1 ] );
            break;

        case 0x2:
            sprintf( ob, "?%c>=%c", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 1 ] );
            break;

        case 0x3:
            sprintf( ob, "?%c<=%c", reg_pair[ rp ][ 0 ], reg_pair[ rp ][ 1 ] );
            break;

        default:
            FATAL( CPU_CHF_MODULE_ID, CPU_F_INTERR, "Bad_Test_Code" )
            break;
    }

    /* Decode field selector */
    DisFIELD_SEL( FS_A, ob );

    /* Decode RTNYES/GOYES */
    return DisGOYES_RTNYES( pc, ob );
}

/* Instruction Group_8

   Opcode table for:
     81s

     OC = (s & 0xC) >> 2
     RP = (s & 0x3)

                                Opcode		OC	RP
   ---
   ASLC				810		0	0
   BSLC				811		0	1
   CSLC				812		0	2
   DSLC				813		0	3
   ---
   ASRC				814		1	0
   BSRC				815		1	1
   CSRC				816		1	2
   DSRC				817		1	3
   ---
                                818		2	0, Special
                                819		2	1, Special
                                81A		2	2, Special
                                81B		2	3, Special
   ---
   ASRB				81C		3	0
   BSRB				81D		3	1
   CSRB				81E		3	2
   DSRB				81F		3	3
   ---

   Opcode table for:
     82n,	CLRHSN n, Clear one or more HST bits, n=bit mask
     83nyy,	?HS=0 n, Check HST bits, n=bit mask
     84n	ST=0 n, Clear bit n of ST register
     85n	ST=1 n, Set bit n of ST register
     86nyy,	?ST=0 n, Check ST bits, n=bit number
     87nyy,	?ST=1 n, Check ST bits, n=bit number
     88nyy,	?P#n, Check P#n, jump/return/set carry if true
     89nyy,	?P=n, Check P=n, jump/return/set carry if true
     8A...,	Tests
     8B...,	Tests
     8Cnnnn,	GOLONG
     8Dnnnnn,	GOVLNG
     8Ennnn,	GOSUBL
     8Fnnnnn,	GOSBVL
*/
static Address DisGroup_8( Address pc, char* ob )
{
    Nibble n = FetchNibble( pc );
    pc++;
    Address addr;
    int oc, rp;

    switch ( n ) {
        case 0x0:
            pc = DisGroup_80( pc, ob );
            break;

        case 0x1:
            /* rSLC, rSRC, rSRB, Special Group_81 */
            n = FetchNibble( pc );
            pc++;
            oc = GET_OC_1( n );
            rp = GET_RP( n );

            switch ( oc ) {
                case 0x0:
                    /* rSLC */
                    sprintf( ob, "%cSLC", reg_pair[ rp ][ 0 ] );
                    break;

                case 0x1:
                    /* rSRC */
                    sprintf( ob, "%cSRC", reg_pair[ rp ][ 0 ] );
                    break;

                case 0x2:
                    /* Special Group_81 */
                    pc = DisSpecialGroup_81( pc, ob, rp );
                    break;

                case 0x3:
                    /* rSRB */
                    sprintf( ob, "%cSRB", reg_pair[ rp ][ 0 ] );
                    break;

                default:
                    FATAL( CPU_CHF_MODULE_ID, CPU_F_INTERR, "Bad_Operation_Code" )
                    break;
            }
            break;

        case 0x2:
            /* CLRHSn */
            n = FetchNibble( pc );
            pc++;

            switch ( n ) {
                case 0x1:
                    strcpy( ob, "XM=0" );
                    break;

                case 0x2:
                    strcpy( ob, "SB=0" );
                    break;

                case 0x4:
                    strcpy( ob, "SR=0" );
                    break;

                case 0x8:
                    strcpy( ob, "MP=0" );
                    break;

                case 0xF:
                    strcpy( ob, "CLRHST" );
                    break;

                default:
                    sprintf( ob, "CLRHSN\t%X", n );
                    break;
            }
            break;

        case 0x3:
            /* ?HS=0 */
            n = FetchNibble( pc );
            pc++;

            switch ( n ) {
                case 0x1:
                    strcpy( ob, "?XM=0" );
                    break;

                case 0x2:
                    strcpy( ob, "?SB=0" );
                    break;

                case 0x4:
                    strcpy( ob, "?SR=0" );
                    break;

                case 0x8:
                    strcpy( ob, "?MP=0" );
                    break;

                default:
                    sprintf( ob, "?HS=0\t%X", n );
                    break;
            }

            /* Decode RTNYES/GOYES */
            pc = DisGOYES_RTNYES( pc, ob );
            break;

        case 0x4:
            /* ST=0 n */
            n = FetchNibble( pc );
            pc++;

            sprintf( ob, "ST=0\t%d", n );
            break;

        case 0x5:
            /* ST=1 n */
            n = FetchNibble( pc );
            pc++;

            sprintf( ob, "ST=1\t%d", n );
            break;

        case 0x6:
            /* ?ST=0 n */
            n = FetchNibble( pc );
            pc++;

            /* Decode bit number */
            sprintf( ob, "?ST=0\t%d", n );

            /* Decode RTNYES/GOYES */
            pc = DisGOYES_RTNYES( pc, ob );
            break;

        case 0x7:
            /* ?ST=1 n */
            n = FetchNibble( pc );
            pc++;

            /* Decode bit number */
            sprintf( ob, "?ST=1\t%d", n );

            /* Decode RTNYES/GOYES */
            pc = DisGOYES_RTNYES( pc, ob );
            break;

        case 0x8:
            /* ?P#n */
            n = FetchNibble( pc );
            pc++;

            /* Decode bit number */
            sprintf( ob, "?P#%d", n );

            /* Decode RTNYES/GOYES */
            pc = DisGOYES_RTNYES( pc, ob );
            break;

        case 0x9:
            /* ?P=n */
            n = FetchNibble( pc );
            pc++;

            /* Decode bit number */
            sprintf( ob, "?P=%d", n );

            /* Decode RTNYES/GOYES */
            pc = DisGOYES_RTNYES( pc, ob );
            break;

        case 0xA:
            /* Test */
            pc = DisTest_8A( pc, ob );
            break;

        case 0xB:
            /* Test */
            pc = DisTest_8B( pc, ob );
            break;

        case 0xC:
            /* GOLONG */
            addr = Get4Nibbles2C( pc );
            sprintf( ob, "GOLONG\tA_%05X\t* Offset [%d]d", pc + addr, addr );
            pc += 4;
            break;

        case 0xD:
            /* GOVLNG */
            addr = Get5NibblesAbs( pc );
            sprintf( ob, "GOVLNG\tA_%05X", addr );
            pc += 5;
            break;

        case 0xE:
            /* GOSUBL */
            addr = Get4Nibbles2C( pc );
            pc += 4;

            sprintf( ob, "GOSUBL\tA_%05X\t* Offset [%d]d", pc + addr, addr );
            break;
            break;

        case 0xF:
            /* GOSBVL */
            addr = Get5NibblesAbs( pc );
            sprintf( ob, "GOSBVL\tA_%05X", addr );
            pc += 5;
            break;

        default:
            /* Unknown opcode */
            strcpy( ob, "?" );

            ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, pc, n )
            break;
    }

    return pc;
}

/*---------------------------------------------------------------------------
        Public functions
  ---------------------------------------------------------------------------*/

/* .+

.creation     : 28-Jan-1998
.description  :
  This function disassembles a Saturn instruction starting from 'pc' and
  puts the result into the output buffer 'ob'.

.call         :
                new_pc = Disassemble(pc, ob)
.input        :
                Address pc, address of the instruction to disassemble
.output       :
                char ob[DISASSEMBLE_OB_SIZE], text output buffer
                Address new_pc, address of the next instruction
.status_codes :
                CPU_E_BAD_OPCODE
                CPU_F_INTERR
.notes        :
  1.1, 26-Jan-1998, creation

.- */
Address Disassemble( Address pc, char ob[ DISASSEMBLE_OB_SIZE ] )
{
    /* Disassemble current program counter */
    sprintf( ob, "[%05X]\t", pc );
    ob += strlen( ob );

    /* Get first instruction nibble */
    Nibble n = FetchNibble( pc );
    pc++;

    switch ( n ) {
        case 0x0:
            /* Group_0 */
            pc = DisGroup_0( pc, ob );
            break;

        case 0x1:
            /* Group_1 */
            pc = DisGroup_1( pc, ob );
            break;

        case 0x2:
            /* P=n */
            pc = DisPEqn( pc, ob );
            break;

        case 0x3:
            /* LC(m) n...n */
            pc = DisLC( pc, ob );
            break;

        case 0x4:
            /* RTNC/GOC */
            pc = DisRTNC_GOC( pc, ob );
            break;

        case 0x5:
            /* RTNNC/GONC */
            pc = DisRTNNC_GONC( pc, ob );
            break;

        case 0x6:
            /* GOTO */
            pc = DisGOTO( pc, ob );
            break;

        case 0x7:
            /* GOSUB */
            pc = DisGOSUB( pc, ob );
            break;

        case 0x8:
            /* Group_8 */
            pc = DisGroup_8( pc, ob );
            break;

        case 0x9:
            /* Test */
            pc = DisTest_9( pc, ob );
            break;

        case 0xA:
            /* Register Operation, group A */
            pc = DisRegOp_A( pc, ob );
            break;

        case 0xB:
            /* Register Operation, group B */
            pc = DisRegOp_B( pc, ob );
            break;

        case 0xC:
            /* Register Operation, group C */
            pc = DisRegOp_C( pc, ob );
            break;

        case 0xD:
            /* Register Operation, group D */
            pc = DisRegOp_D( pc, ob );
            break;

        case 0xE:
            /* Register Operation, group E */
            pc = DisRegOp_E( pc, ob );
            break;

        case 0xF:
            /* Register Operation, group F */
            pc = DisRegOp_F( pc, ob );
            break;

        default:
            /* Unknown opcode */
            strcpy( ob, "?" );

            ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, pc, n )
            break;
    }

    return pc & ADDRESS_MASK;
}
