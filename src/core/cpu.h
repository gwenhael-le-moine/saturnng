#ifndef _CPU_H
#  define _CPU_H 1

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

.creation     :	*

.description  :
  Main header for the Saturn CPU emulation modules. References:

    SASM.DOC by HP  (HORN disk 4)
    Guide to the Saturn Processor Rev. 0.00f by Matthew Mastracci
    entries.srt by Mika Heiskanen  (mheiskan@vipunen.hut.fi)
    x48 source code by Eddie C. Dost  (ecd@dressler.de)

.notes        :
  $Log: cpu.h,v $
  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.14  2000/11/13 11:31:16  cibrario
  Implemented fast load/save; improved keyboard interface emulation at
  high emulated CPU speed:

  - Revision number bump with no changes

  Revision 3.13  2000/11/09 11:27:14  cibrario
  Revised to add file selection box GUI element, CPU halt/run
  requests and emulator's extended functions:

  - Added new fields:
        struct Cpu.halt (number of pending halt requests)
        struct Cpu.inner_loop_max (upper limit of inner_loop)
  - New condition codes: CPU_I_HALT, CPU_I_RUN, CPU_E_NO_HALT
  - New prototypes: CpuHaltRequest(), CpuRunRequest(), CpuHaltAllowed()

  Revision 3.10  2000/10/24 16:14:31  cibrario
  Added/Replaced GPL header

  Revision 3.8  2000/10/23 13:13:08  cibrario
  Bug fix:
  Adjusted INNER_LOOP_MAX and INNER_LOOP_MED to get closer to
  real CPU speed.

  Revision 3.5  2000/10/02 09:43:37  cibrario
  Linux support:
  - Added definition of INNER_LOOP_MAX; it is used to give an upper limit
    to the emulated CPU speed on fast machines, when REAL_CPU_SPEED is
    defined.

  Revision 3.1  2000/09/20 13:42:30  cibrario
  Revised to implement passive CPU shutdown:
  - new status codes CPU_I_TIMER_ST, CPU_I_TIMER_EXP, CPU_I_IDLE_X_LOOP,
    CPU_I_ELAPSED

  Revision 2.4  2000/09/12  15:19:47  cibrario
  Added definition of XAddress (extended address) data type; it is
  required to implement emulation of Port 1 and 2.

  Revision 2.1  2000/09/08  14:48:52  cibrario
  - Declared prototypes of new functions EmulatorInit() and EmulatorExit()
  - Defined new 'enum ExitOption' data type, used by EmulatorExit()

  Revision 1.1  1998/02/18  11:50:43  cibrario
  Initial revision
.- */

#  include <stdbool.h>

/*---------------------------------------------------------------------------
        Macro/Data type definitions - require types.h

  N_SCRATCH_REGISTER_ALL, used during scratch register space allocation
  is larger than necessary to avoid additional checks on the validity of
  the R register index fields during emulation
  ---------------------------------------------------------------------------*/
#  include "types.h"

#  define OPCODE_LENGTH( oc )                                                                                                              \
      ( ( oc / 0x100000 > 0 ) + ( oc / 0x10000 > 0 ) + ( oc / 0x1000 > 0 ) + ( oc / 0x100 > 0 ) + ( oc / 0x10 > 0 ) + 1 )

/* General */
#  define NIBBLE_PER_REGISTER 16
#  define N_WORKING_REGISTER 4
#  define N_SCRATCH_REGISTER 5
#  define N_SCRATCH_REGISTER_ALL 8
#  define N_DATA_POINTER_REGISTER 2
#  define RETURN_STACK_SIZE 8
#  define NIBBLE_VALUES 16

#  define INT_HANDLER_PC ( ( Address )0x0000F )

/* Instruction opcode access macros:
     GET_FS(f)		returns the short field-selector value from the
                        given nibble (bits 2..0)

     GET_IMMEDIATE_FS_FLAG(f)	returns the immediate-field-selector flag from the
                        given nibble (bit 3)
                         =0: regular field selector
                        !=0: immediate field selector

     GET_OC_1(o)		returns the short operation code from the given
                        nibble (bits 3..2 >>2)

     GET_OC_2(f, o)	returns the long operation code from the given
                        nibbles (f bit 3, o bits 3..2)

     GET_OC_3b(o)	returns the long operation code from the given
                        nibble (bits 2..0)

     GET_RP(o)		returns the register-pair identifier from the given
                        nibble (bits 1..0)

     GET_Rn(r)		returns the R register index from the given nibble
                        (bits 2..0)

     GET_AC(r)		returns the A/C register flag from the given nibble
                        (bit 3)
                         =0: register A
                        !=0: register C

     GET_AS(r)		returns the add/subtract flag from the given nibble
                        (bit 3)
                         =0: add
                        !=0: subtract
*/
#  define GET_FS( f ) ( ( f ) & 0x7 )
#  define GET_IMMEDIATE_FS_FLAG( o ) ( ( o ) & 0x8 )
#  define GET_OC_1( o ) ( ( ( o ) & 0xC ) >> 2 )
#  define GET_OC_2( f, o ) ( ( ( ( f ) & 0x8 ) >> 1 ) | ( ( ( o ) & 0xC ) >> 2 ) )
#  define GET_OC_3b( o ) ( ( o ) & 0x7 )
#  define GET_RP( o ) ( ( o ) & 0x3 )
#  define GET_Rn( r ) ( ( r ) & 0x7 )
#  define GET_AC( r ) ( ( r ) & 0x8 )
#  define GET_AS( r ) ( ( r ) & 0x8 )

/* Field selector codes */
typedef enum {
    FS_P = 0,
    FS_WP = 1,
    FS_XS = 2,
    FS_X = 3,
    FS_S = 4,
    FS_M = 5,
    FS_B = 6,
    FS_W = 7,
    FS_A = 15,
    N_FS = 16, /* Total # of FS codes */
} field_selector_code_t;

/* Masks */
#  define NIBBLE_MASK ( ( Nibble )0xF )
#  define ADDRESS_MASK ( ( Address )0xFFFFF )

#  define CLRST_MASK ( ( ProgramStatusRegister )0xF000 )
#  define D_S_MASK ( ( Address )0xF0000 )
#  define RSTK_PTR_MASK 0x7

typedef int16 ProgramStatusRegister;
typedef Nibble DataRegister[ NIBBLE_PER_REGISTER ];

typedef enum { INT_REQUEST_NONE, INT_REQUEST_IRQ, INT_REQUEST_NMI } int_request_t;

typedef struct {
    DataRegister reg[ N_WORKING_REGISTER ];
    DataRegister reg_r[ N_SCRATCH_REGISTER_ALL ];
    Address d[ N_DATA_POINTER_REGISTER ];

    Nibble p;
    Address pc;
    InputRegister in;
    OutputRegister out;
    ProgramStatusRegister st;

    Nibble hst;
#  define HST_MP_MASK 0x08
#  define HST_SR_MASK 0x04
#  define HST_SB_MASK 0x02
#  define HST_XM_MASK 0x01

    Address rstk[ RETURN_STACK_SIZE ];
    int rstk_ptr;

    int fs_idx_lo[ N_FS ];
    int fs_idx_hi[ N_FS ];
    bool hexmode;              /* DEC/HEX mode */
    bool carry;                /* Carry bit */
    bool shutdn;               /* SHUTDN flag */
    bool halt;                 /* Halt flag */
    bool int_enable;           /* Int. enable */
    bool int_service;          /* Int. service */
    int_request_t int_pending; /* Pending interrupt request */

    /* 3.13: inner_loop_max gives the upper limit of the CPU speed if the
       compile-time option REAL_CPU_SPEED is defined.  When the CPU is reset
       it has the default value INNER_LOOP_MAX, that should be close to the
       real cpu speed (~4MHz).
    */
    int inner_loop;     /* Inner loop multiplier */
    int inner_loop_max; /* Max value of inner_loop */
#  define INNER_LOOP_MAX 26
#  define INNER_LOOP_MED 13
#  define INNER_LOOP_MIN 2
} Cpu;

enum RegisterNames { A, B, C, D };

/*---------------------------------------------------------------------------
        Global variables
  ---------------------------------------------------------------------------*/

extern Cpu cpu;

extern int opcode;

/*---------------------------------------------------------------------------
        Chf condition codes
  ---------------------------------------------------------------------------*/
typedef enum {
    CPU_I_CALLED = 101,          /* Function %s called */
    CPU_I_EXECUTING = 102,       /* Executing @PC %X */
    CPU_I_SHUTDN = 103,          /* Shutdown */
    CPU_I_WAKE = 104,            /* Wake */
    CPU_I_INT = 105,             /* %s request accepted */
    CPU_I_INT_PENDING = 106,     /* %s request pending */
    CPU_I_RTI_LOOP = 107,        /* RTI loop to service %s */
    CPU_I_RTI_END = 108,         /* RTI returning */
    CPU_I_INTON = 109,           /* INTON servicing %s */
    CPU_I_REVISION = 110,        /* CPU emulation revision: %s */
    CPU_I_TIMER1_EX = 111,       /* Timer 1 expired; ctrl=%x */
    CPU_I_TIMER2_EX = 112,       /* Timer 1 expired; ctrl=%x */
    CPU_I_EMULATOR_INT = 113,    /* Emulator interrupt req. detected */
    CPU_I_TIMER_ST = 114,        /* 3.1: Timer %s st: ctrl %x, val %x */
    CPU_I_TIMER_EXP = 115,       /* 3.1: Timer %s expiration %d ms */
    CPU_I_IDLE_X_LOOP = 116,     /* 3.1: Start idle loop, t/out %d ms */
    CPU_I_ELAPSED = 117,         /* 3.1: Spent %d us in idle loop */
    CPU_I_HALT = 118,            /* 3.13: CPU halted */
    CPU_I_RUN = 119,             /* 3.13: CPU running */
    CPU_W_RESETTING = 201,       /* Resetting CPU */
    CPU_W_BAD_MONITOR_CMD = 202, /* Bad monitor command: %s */
    CPU_E_BAD_OPCODE = 301,      /* Bad opc. pc=%x, value=%x */
    CPU_E_SAVE = 302,            /* Can't save CPU status */
    CPU_E_NO_HALT = 303,         /* 3.13: Halt/Run not allowed */
    CPU_E_BAD_OPCODE2 = 304,     /* Bad opc. pc=%x, value=%x */
    CPU_F_INTERR = 401,          /* Internal error %s */
    CPU_F_BAD_SHUTDN = 402,      /* Unexpected CPU shutdown */
} cpu_chf_message_id_t;

/*---------------------------------------------------------------------------
        Function prototypes
  ---------------------------------------------------------------------------*/

void CpuInit( void );
void CpuReset( void );
void CpuSave( void );
void OneStep( void );
void CpuIntRequest( int_request_t ireq );
void CpuWake( void );
int CpuHaltRequest( void );  /* 3.13 */
int CpuRunRequest( void );   /* 3.13 */
bool CpuHaltAllowed( void ); /* 3.13 */

void set_speed( unsigned int new_speed_mhz );

#endif /*!_CPU_H*/
