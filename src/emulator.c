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

.identifier   : $Id: emulator.c,v 4.1 2000/12/11 09:54:19 cibrario Rel $
.context      : SATURN, Saturn CPU / HP48 emulator
.title	      : $RCSfile: emulator.c,v $
.kind	      : C source
.author	      : Ivan Cibrario B.
.site	      : CSTV-CNR
.creation     :	2-Feb-1998
.keywords     : *
.description  :
  This file contains the main loop of the emulator. For efficiency reasons,
  this module also emulates both T1 and T2 timers. References:

    SASM.DOC by HP  (HORN disk 4)
    Guide to the Saturn Processor Rev. 0.00f by Matthew Mastracci
    entries.srt by Mika Heiskanen  (mheiskan@vipunen.hut.fi)
    x48 source code by Eddie C. Dost  (ecd@dressler.de)

.include      : config.h, machdep.h, cpu.h

.notes	      :
  $Log: emulator.c,v $
  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.13  2000/11/09 11:30:40  cibrario
  Revised to add file selection box GUI element, CPU halt/run
  requests and emulator's extended functions:

  - The upper limit to the CPU speed can now be changed at runtime.
  - Fixed a bug in EmulatorLoopHandler(): it could return an invalid
    Chf action code.

  Revision 3.10  2000/10/24 16:14:39  cibrario
  Added/Replaced GPL header

  Revision 3.5  2000/10/02 09:48:04  cibrario
  Linux support:
  - EmulatorLoop(): revised to force an upper limit to the CPU speed if
    the compile-time option REAL_CPU_SPEED is defined: inner_loop is
    limited to INNER_LOOP_MAX and the excess time is spent sleeping.

  Revision 3.2  2000/09/22 13:43:09  cibrario
  Implemented preliminary support of HP49 hw architecture:
  - EmulatorInit() now invokes ModSelectDescription() to select
    the appropriate calculator's hw configuration depending on the
    setting of the 'hw' user option.

 * Revision 3.1  2000/09/20  14:12:45  cibrario
 * Revised to implement passive CPU shutdown:
 * - enhanced EmulatorLoopHandler() to handle the CPU_I_SHUTDN condition
 *   when CPU_SPIN_SHUTDN is not defined.
 *
 * Revision 2.5  2000/09/14  14:45:37  cibrario
 * Added call to HandleSerial() in EmulatorLoop(), in order to
 * handle external events related to serial port emulation.
 *
 * Revision 2.4  2000/09/12  15:17:18  cibrario
 * Updated EmulatorInit() in order to invoke CpuInit() before ModInit(),
 * so that interrupt requests generated by ModInit() are honored as
 * they should.  This is required to implement emulation of Port 1 and 2.
 *
 * Revision 2.1  2000/09/08  14:57:49  cibrario
 * - Removed explicit cast of second argument from calls to gettimeofday()
 * - Minor fixes needed by Chf Release 2
 * - Defined new convenience functions EmulatorInit() and EmulatorExit();
 *   they can be invoked to reset/initialize the emulation core and
 *   to prepare it to exit, respectively.
 *
 * Revision 1.1  1998/02/18  11:49:21  cibrario
 * Initial revision
 *

.- */

#ifndef lint
static char rcs_id[] = "$Id: emulator.c,v 4.1 2000/12/11 09:54:19 cibrario Rel $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "config.h"
#include "machdep.h"
#include "cpu.h"
#include "modules.h"
/* #include "x11_lcd.h" */
/* #include "x11.h" */
#include "serial.h"
#include "debug.h"

#define CHF_MODULE_ID CPU_CHF_MODULE_ID
#include "libChf/src/Chf.h"

/*---------------------------------------------------------------------------
        Private macros / variables / functions
  ---------------------------------------------------------------------------*/

#define T1_MULTIPLIER ( 8192 / 16 ) /* T2/T1 increment ratio */
#define T1_INTERVAL 62500           /* us per T1 increment */
#define T2_INTERVAL 122             /* us per T2 increment */

/* 3.1: MAX_IDLE_X_LOOP_TIMEOUT must be low enough to prevent overflow
   of an int when computing the difference in microseconds between two
   struct timeval and to avoid starvation of the serial port emulation
   module (*); T1_MS_MULTIPLIER and T2_MS_DIVISOR are approximate
   values used only to compute e reasonable timeout for IdleXLoop();
   the actual update of timers is carried out using T1_INTERVAL
   and T2_INTERVAL, and should be more accurate.


   (*) XXX This constraint will be removed when the serial port emulation
       module will support asynchronous selection on pty fd and will
       be able to interact with the GUI's select mechanism.
*/
#define T1_MS_MULTIPLIER 63          /* 3.1: Milliseconds per T1 tick (~) */
#define T2_MS_DIVISOR 8              /* 3.1: T2 ticks per millisecond (~) */
#define MAX_IDLE_X_LOOP_TIMEOUT 1000 /* 3.1: Max timeout for IdleXLoop() */

#define T1_OVF_MASK NIBBLE_MASK /* 3.1: Timer overflow masks */
#define T2_OVF_MASK 0xFFFFFFFF

#define LCD_T1_MASK 0x3 /* LCD refresh timing mask */
#define INT_T1_MASK 0xF /* Int. req. timing mask */

static int emulator_int_req = 0; /* Interrupt request flag */

/* This function contains the main emulator loop; under normal conditions,
   it never returns to the caller. The only way to exit this function is
   to signal a Chf condition that triggers an unwind operation.
*/
static void EmulatorLoop( void )
{
    struct timeval old_t, cur_t;
    int ela;
    int inner_loop = cpu_status.inner_loop;
    int t1_count = 0;
    int i, j;

    debug1( DEBUG_C_TRACE, CPU_I_CALLED, "EmulatorLoop" );

    /* Ignore past interrupt requests */
    emulator_int_req = 0;

    /* Get current time of day */
    gettimeofday( &old_t, NULL );

    while ( 1 ) {
        /* T1 loop */
        for ( j = 0; j < T1_MULTIPLIER; j++ ) {
            /* Inner loop */
            for ( i = 0; i < inner_loop; i++ )
                OneStep();

            /* T2 update */
            if ( mod_status.hdw.t2_ctrl & T2_CTRL_TRUN ) {
                if ( --mod_status.hdw.t2_val == ( int )0xFFFFFFFF ) {
                    debug1( DEBUG_C_TIMERS, CPU_I_TIMER2_EX, mod_status.hdw.t2_ctrl );

                    mod_status.hdw.t2_ctrl |= T2_CTRL_SREQ;

                    if ( mod_status.hdw.t2_ctrl & T2_CTRL_WAKE )
                        CpuWake();

                    if ( mod_status.hdw.t2_ctrl & T2_CTRL_INT )
                        CpuIntRequest( INT_REQUEST_IRQ );
                }
            }
        }

        /* T1 update */
        mod_status.hdw.t1_val = ( mod_status.hdw.t1_val - 1 ) & NIBBLE_MASK;
        if ( mod_status.hdw.t1_val == 0xF ) {
            debug1( DEBUG_C_TIMERS, CPU_I_TIMER1_EX, mod_status.hdw.t1_ctrl );

            mod_status.hdw.t1_ctrl |= T1_CTRL_SREQ;

            if ( mod_status.hdw.t1_ctrl & T1_CTRL_WAKE )
                CpuWake();

            if ( mod_status.hdw.t1_ctrl & T1_CTRL_INT )
                CpuIntRequest( INT_REQUEST_IRQ );
        }

        /* LCD update */
        /* if ( ( t1_count++ & LCD_T1_MASK ) == 0 ) */
        /*     DrawLcd(); */

        /* Emulator Interrupt Request */
        if ( ( t1_count++ & INT_T1_MASK ) == 0 && emulator_int_req ) {
            ChfCondition CPU_I_EMULATOR_INT, CHF_INFO ChfEnd;
            ChfSignal();
        }

        /* UI Events handling */
        // ui_get_event();

        /* Handle serial port */
        HandleSerial();

        /* Adjust inner_loop limit */
        gettimeofday( &cur_t, NULL );

        ela = ( cur_t.tv_sec - old_t.tv_sec ) * 1000000 + ( cur_t.tv_usec - old_t.tv_usec );

        inner_loop = inner_loop * T1_INTERVAL / ela;
        if ( inner_loop < INNER_LOOP_MIN )
            inner_loop = INNER_LOOP_MIN;

        /* 3.13: Force an upper limit to the CPU speed if the compile-time option
           REAL_CPU_SPEED is defined: inner_loop is limited to
           cpu_status.inner_loop_max
           and the excess time, if any, is spent sleeping; usleep() is
           BSD 4.3-specific, but most recent systems should offer it anyway,
           well, I hope.
           The special value cpu_status.inner_loop_max==0 gives maximum speed.
        */
        if ( config.throttle )
            if ( cpu_status.inner_loop_max != 0 && inner_loop >= cpu_status.inner_loop_max ) {
                inner_loop = cpu_status.inner_loop_max;
                if ( T1_INTERVAL > ela )
                    usleep( T1_INTERVAL - ela );
            }

        cpu_status.inner_loop = inner_loop;
        old_t = cur_t;
    }
}

/* Condition handler for the EmulatorLoop */
static ChfAction EmulatorLoopHandler( const ChfDescriptor* d, const ChfState s, ChfPointer ctx )
{
    ChfAction act;

    /* Check Chf state */
    switch ( s ) {
        /* 2.1: Chf release 2 fixed the spelling of 'SIGNALING' */
        case CHF_SIGNALING:
            /* ChfSignal() in progress */
            if ( ChfGetModuleId( d ) == CPU_CHF_MODULE_ID ) {
                /* Condition from CPU modules; check Condition Code */
                switch ( ChfGetConditionCode( d ) ) {
                    case CPU_I_SHUTDN:
                        {
                            /* 3.1: CPU_SPIN_SHUTDN is not defined, and the cpu emulator
                               has just executed a shutdown instruction.
                               Let's do something a little tricky here:

                               1- redraw the LCD

                               2- handle serial port activities

                               3- determine which timer will expire first, and
                                  compute an approximate value of the maximum duration
                                  of the shutdown --> ms

                               4- handle serial port activities

                               5- enter the inner idle loop; it breaks when either an
                                  X Event occurred (possibly clearing the shutdown) or
                                  the shutdown timeout elapses

                               6- determine the actual time we spend in the idle loop
                                  (X timeouts are not accurate enough for this purpose)

                               7- update T1 and T2, check their state and wake/interrupt
                                  the CPU if necessary

                               Activities 3-7 above are enclosed in an outer loop because we
                               cannot be absolutely sure of the actual time spent
                               in the idle loop; moreover, not all X Events actually
                               spool up the CPU. The outer loop breaks when the CPU is
                               actually brought out of shutdown.

                               frac_t1 and frac_t2 contain the number of microseconds
                               not accounted for in the last T1/T2 update, respectively;
                               they help minimize the cumulative timing error induced
                               by executing the outer idle loop more than once.
                            */
                            struct timeval start_idle, end_idle;
                            int frac_t1 = 0, frac_t2 = 0;

                            gettimeofday( &start_idle, NULL );

                            /* Redraw the LCD immediately before entering idle loop;
                               this ensures that the latest LCD updated actually
                               get to the screen.
                            */
                            // ui_update_display();

                            /* Handle serial port activity before entering the outer idle
                               loop, because this could possibly bring the cpu out of
                               shutdown right now.
                            */
                            HandleSerial();

                            /* XXX
                               If either timer has a pending service request,
                               process it immediately.  It is not clear why it was
                               not processed *before* shutdown, though.
                            */
                            if ( mod_status.hdw.t1_ctrl & T1_CTRL_SREQ ) {
                                if ( mod_status.hdw.t1_ctrl & T1_CTRL_WAKE )
                                    CpuWake();

                                if ( mod_status.hdw.t1_ctrl & T1_CTRL_INT )
                                    CpuIntRequest( INT_REQUEST_IRQ );
                            }

                            if ( mod_status.hdw.t2_ctrl & T2_CTRL_SREQ ) {
                                if ( mod_status.hdw.t2_ctrl & T2_CTRL_WAKE )
                                    CpuWake();

                                if ( mod_status.hdw.t2_ctrl & T2_CTRL_INT )
                                    CpuIntRequest( INT_REQUEST_IRQ );
                            }

                            while ( cpu_status.shutdn ) {
                                unsigned long ms = MAX_IDLE_X_LOOP_TIMEOUT;
                                unsigned long mst;
                                int ela;
                                int ela_ticks;

                                debug3( DEBUG_C_TIMERS, CPU_I_TIMER_ST, "T1 (during SHUTDN)", mod_status.hdw.t1_ctrl,
                                        mod_status.hdw.t1_val );
                                debug3( DEBUG_C_TIMERS, CPU_I_TIMER_ST, "T2 (during SHUTDN)", mod_status.hdw.t2_ctrl,
                                        mod_status.hdw.t2_val );

                                /* Determine which timer will expire first */
                                if ( mod_status.hdw.t1_ctrl & ( T1_CTRL_INT | T1_CTRL_WAKE ) ) {
                                    /* T1 will do something on expiration */
                                    mst = ( ( unsigned long )mod_status.hdw.t1_val + 1 ) * T1_MS_MULTIPLIER;

                                    debug2( DEBUG_C_TIMERS, CPU_I_TIMER_EXP, "T1", mst );

                                    if ( mst < ms )
                                        ms = mst;
                                }

                                if ( ( mod_status.hdw.t2_ctrl & T2_CTRL_TRUN ) &&
                                     ( mod_status.hdw.t2_ctrl & ( T2_CTRL_INT | T2_CTRL_WAKE ) ) ) {
                                    /* T2 is running and will do something on expiration */
                                    mst = ( ( unsigned long )mod_status.hdw.t2_val + 1 ) / T2_MS_DIVISOR;

                                    debug2( DEBUG_C_TIMERS, CPU_I_TIMER_EXP, "T2", mst );

                                    if ( mst < ms )
                                        ms = mst;
                                }

                                /* Handle serial port activities at each iteration of
                                   the outer idle loop; this ensures that the serial
                                   port emulation will not starve.
                                */
                                HandleSerial();

                                /* Enter idle loop, possibly with timeout;
                                   The loop breaks when:
                                   - any X Event occurs (possibly clearing the shutdown)
                                   - the given timeout expires
                                */
                                debug1( DEBUG_C_TIMERS, CPU_I_IDLE_X_LOOP, ms );
                                // IdleXLoop( ms );

                                /* End of idle loop; compute actual elapsed time */
                                gettimeofday( &end_idle, NULL );

                                ela = ( end_idle.tv_sec - start_idle.tv_sec ) * 1000000 + ( end_idle.tv_usec - start_idle.tv_usec );

                                /* Update start_idle here to contain lag */
                                start_idle = end_idle;

                                debug1( DEBUG_C_TIMERS, CPU_I_ELAPSED, ela );

                                /* Update timers and act accordingly */
                                ela_ticks = ( ( ela + frac_t1 ) + T1_INTERVAL / 2 ) / T1_INTERVAL;
                                frac_t1 = ( ela + frac_t1 ) - ela_ticks * T1_INTERVAL;

                                if ( ela_ticks > mod_status.hdw.t1_val ) {
                                    debug1( DEBUG_C_TIMERS, CPU_I_TIMER1_EX, mod_status.hdw.t1_ctrl );

                                    mod_status.hdw.t1_ctrl |= T1_CTRL_SREQ;

                                    if ( mod_status.hdw.t1_ctrl & T1_CTRL_WAKE )
                                        CpuWake();

                                    if ( mod_status.hdw.t1_ctrl & T1_CTRL_INT )
                                        CpuIntRequest( INT_REQUEST_IRQ );
                                }

                                mod_status.hdw.t1_val = ( mod_status.hdw.t1_val - ela_ticks ) & T1_OVF_MASK;

                                if ( mod_status.hdw.t2_ctrl & T2_CTRL_TRUN ) {
                                    ela_ticks = ( ( ela + frac_t2 ) + T2_INTERVAL / 2 ) / T2_INTERVAL;
                                    frac_t2 = ( ela + frac_t2 ) - ela_ticks * T2_INTERVAL;

                                    if ( ela_ticks > mod_status.hdw.t2_val ) {
                                        debug1( DEBUG_C_TIMERS, CPU_I_TIMER2_EX, mod_status.hdw.t2_ctrl );

                                        mod_status.hdw.t2_ctrl |= T2_CTRL_SREQ;

                                        if ( mod_status.hdw.t2_ctrl & T2_CTRL_WAKE )
                                            CpuWake();

                                        if ( mod_status.hdw.t2_ctrl & T2_CTRL_INT )
                                            CpuIntRequest( INT_REQUEST_IRQ );
                                    }

                                    mod_status.hdw.t2_val = ( mod_status.hdw.t2_val - ela_ticks ) & T2_OVF_MASK;
                                }
                            }

                            debug3( DEBUG_C_TIMERS, CPU_I_TIMER_ST, "T1 (after SHUTDN)", mod_status.hdw.t1_ctrl, mod_status.hdw.t1_val );
                            debug3( DEBUG_C_TIMERS, CPU_I_TIMER_ST, "T2 (after SHUTDN)", mod_status.hdw.t2_ctrl, mod_status.hdw.t2_val );

                            act = CHF_CONTINUE;
                        }
                        break;

                    case CPU_I_EMULATOR_INT:
                        /* Emulator interrupt; unwind */
                        act = CHF_UNWIND;
                        break;

                    default:
                        /* Condition Code not handled; resignal */
                        act = CHF_RESIGNAL;
                }
            } else
                /* Condition from other modules; resignal */
                act = CHF_RESIGNAL;

            break;

        default:
            /* Other states; resignal the condition */
            act = CHF_RESIGNAL;
            break;
    }

    return act;
}

/*---------------------------------------------------------------------------
        Public functions
  ---------------------------------------------------------------------------*/

/* .+

.title	      : Emulator
.kind	      : C function
.creation     : 17-Feb-1998
.description  :
  This function implements the main emulator loop. For efficiency reasons,
  it also emulates both T1 and T2 timers. Under normal conditions, this
  function returns to the caller only when an emulator interrupt request
  has been posted using EmulatorIntRequest().

  The only way to exit this function (with a non-local jump) is to signal
  a Chf Condition that triggers an unwind operation.

.call	      :
                Emulator();
.input	      :
                void
.output	      :
                void
.status_codes :
                CPU_I_CALLED
                CPU_I_TIMER1_EX
                CPU_I_TIMER2_EX
                Other conditions signalled by lower level modules
.notes	      :
  1.1, 17-Feb-1998, creation

.- */
void Emulator( void )
{
    jmp_buf unwind_context;

    debug1( DEBUG_C_TRACE, CPU_I_CALLED, "Emulator" );

    /* Setup unwind_context */
    if ( setjmp( unwind_context ) == 0 ) {
        /* Push condition handler, with NULL context */
        ChfPushHandler( EmulatorLoopHandler, &unwind_context, ( ChfPointer )NULL );

        /* Activate emulator loop */
        EmulatorLoop();
    } /*  else { */
    /*     /\* Unwinding after an emulator interrupt *\/ */
    /* } */
}

/* .+

.title	      : EmulatorIntRequest
.kind	      : C function
.creation     : 18-Feb-1998
.description  :
  This function posts an interrupt request for the running emulator loop.
  The request will be satisfied as soon as possible and Emulator() will
  return to the caller.

.call	      :
                EmulatorIntRequest();
.input	      :
                void
.output	      :
                void
.status_codes :
                *
.notes	      :
  1.1, 18-Feb-1998, creation

.- */
void EmulatorIntRequest( void ) { emulator_int_req = 1; }

/* .+

.title	      : EmulatorInit
.kind	      : C function
.creation     : 8-Sep-2000
.description  :
  This function initializes the cpu and modules emulator subsystems;
  if the reset emulator option is set, a reset is forced on both
  subsystems, too.

.call	      :
                EmulatorInit();
.input	      :
                void
.output	      :
                void
.status_codes :
                * Status codes signaled by CpuInit() and CpuReset()
                * Status codes signaled by ModInit() and ModReset()
.notes	      :
  2.1, 8-Sep-2000, creation
  2.4, 12-Sep-2000, update
    - invoke CpuInit() before ModInit() so that interrupt requests
      generated by ModInit() are honored as they should.
  3.2, 21-Sep-2000, update:
    - now invoking ModSelectDescription(args.hw) to select and
      register an appropriate module description table depending on
      args.hw option.

.- */
void EmulatorInit( void )
{
    /* Select a module description table */
    ModSelectDescription( config.hw );

    /* Initialize cpu and modules subsystems */
    CpuInit();
    ModInit();

    /* Reset if appropriate */
    if ( config.reset ) {
        CpuReset();
        ModReset();
    }
}

/* .+

.title	      : EmulatorExit
.kind	      : C function
.creation     : 8-Sep-2000
.description  :
  This function prepares the emulator to exit.  If 'opt' is SAVE_AND_EXIT,
  it also attempts to save the emulator's state on mass storage.  Notice
  that this function never exits the application directly, but always
  returns to the caller unless an unrecoverable error occurs.

.call	      :
                EmulatorExit(opt);
.input	      :
                enum ExitOption opt, emulator exit option
.output	      :
                void
.status_codes :
                * Status codes signaled by CpuSave() and ModSave()
.notes	      :
  2.1, 8-Sep-2000, creation

.- */
void EmulatorExit( enum ExitOption opt )
{
    switch ( opt ) {
        case SAVE_AND_EXIT:
            /* Save state of cpu and modules subsystems */
            ModSave();
            CpuSave();
            break;

        default:
            /* Default behavior; do nothing */
            break;
    }
}
