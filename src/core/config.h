#ifndef _CONFIG_H
#  define _CONFIG_H 1

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

.identifier   : $Id: config.h,v 4.1.1.1 2002/11/11 16:13:29 cibrario Exp $
.context      : SATURN, Saturn CPU / HP48 emulator
.title        : $RCSfile: config.h,v $
.kind         : C header
.author       : Ivan Cibrario B.
.site         : CSTV-CNR
.creation     :	28-Jan-1998
.keywords     : *
.description  :
  This file configures the Saturn / HP48 emulator.

.include      : *

.notes        :
  $Log: config.h,v $
  Revision 4.1.1.1  2002/11/11 16:13:29  cibrario
  Small screen support; preliminary

  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.16  2000/11/21 16:38:23  cibrario
  Ultrix/IRIX support:
  - Include config_x.h (automatic exceptions to user configuration)

  Revision 3.14  2000/11/13 10:23:43  cibrario
  Implemented fast load/save; improved keyboard interface emulation at
  high emulated CPU speed:

  - New configuration option: CPU_SLOW_IN (enabled by default)

  Revision 3.13  2000/11/09 11:21:18  cibrario
  Revised to add file selection box GUI element, CPU halt/run
  requests and emulator's extended functions:

  - Updated documentation of CPU_SPIN_SHUTDN and REAL_CPU_SPEED
  - Defined new Chf module identifier: X_FUNC_CHF_MODULE_ID

  Revision 3.10  2000/10/24 16:14:25  cibrario
  Added/Replaced GPL header

  Revision 3.6  2000/10/02 13:52:32  cibrario
  ROM handling utilities:
  - Added new Chf module identifier: UTIL_CHF_MODULE_ID

  Revision 3.5  2000/10/02 09:40:35  cibrario
  Linux support:
  - documented new compile-time option: REAL_CPU_SPEED

  Revision 3.3  2000/09/26 15:24:20  cibrario
  Revised to implement Flash ROM write access:
  - Added new Chf module identifier FLASH_CHF_MODULE_ID

 * Revision 3.2  2000/09/22  13:34:43  cibrario
 * Implemented preliminary support of HP49 hw architecture:
 * - Re-enabled DEBUG
 * - Documented new HP49_SUPPORT option, and enabled it by default
 *
 * Revision 3.1  2000/09/20  14:15:42  cibrario
 * Revised to implement passive CPU shutdown:
 * - disabled DEBUG option by default
 * - enhanced documentation of CPU_SPIN_SHUTDN; this option is now
 *   disabled by default.
 *
 * Revision 2.6  2000/09/15  09:28:48  cibrario
 * Enhanced documentation of serial emulation options; added
 * template of SERIAL_FORCE_STREAMSPTY definition (commented out
 * by default)
 *
 * Revision 2.5  2000/09/14  15:40:24  cibrario
 * - Added description and default values of configuration options
 *   SERIAL_FORCE_OPENPTY and SERIAL_FORCE_STREAMSPTY.
 * - Added new Chf module identifier SERIAL_CHF_MODULE_ID, used by the
 *   serial port emulation modules.
 *
 * Revision 2.4  2000/09/12  15:50:40  cibrario
 * Added description and default definition of N_PORT_2_BANK
 *
 * Revision 2.1  2000/09/08  15:43:46  cibrario
 * Disabled DEBUG option by default; documented new GUI option
 * FORCE_NONMODAL.
 *
 * Revision 1.1  1998/02/17  14:57:15  cibrario
 * Initial revision
 *

.- */

/* 2.4: N_PORT_2_BANK
   This symbol is used to dimension the HP48GX Port_2: it denotes the
   number of 128 Kbyte banks the port must have and must be a power of 2
   between 1 and 32, inclusive.  When undefined, Port_2 is not emulated at all.
   The default value is 8, that is, Port_2 is emulated and its size is 1Mbyte.
*/
// #define N_PORT_2_BANK ( config.model == MODEL_48GX ? 32 : 1 )
#  define N_PORT_2_BANK 32

/* 2.5: SERIAL_FORCE_OPENPTY, SERIAL_FORCE_STREAMSPTY
   Optionally define exactly one of these symbols to force the use of a
   particular pty implementation; if no symbols are defined, the
   serial port emulation modules will do their best to automatically
   determine the most appropriate implementation for the platform at hand.
*/
/* #define SERIAL_FORCE_OPENPTY */
/* #define SERIAL_FORCE_STREAMSPTY */

/* 3.14: CPU_SLOW_IN
   Define this symbol (recommended) to slow down the A=IN and C=IN
   instructions depending on the current emulated CPU speed.

   The value of the macro determines the gain of the relation between
   CPU speed and slow down ratio.
*/
#  define CPU_SLOW_IN 16

/* Chf Module Identifiers:
   Each main module of the emulator has its own Chf Module Identifier; the
   values defined here must match those actually used in the message catalogs.
*/
#  define MAIN_CHF_MODULE_ID 10
#  define CPU_CHF_MODULE_ID 11
#  define MOD_CHF_MODULE_ID 12
#  define DISK_IO_CHF_MODULE_ID 13
#  define X11_CHF_MODULE_ID 14
#  define SERIAL_CHF_MODULE_ID 15 /* 2.5 */
#  define FLASH_CHF_MODULE_ID 16  /* 3.3 */
#  define UTIL_CHF_MODULE_ID 17   /* 3.6 */
#  define X_FUNC_CHF_MODULE_ID 18 /* 3.13 */
#  define DEBUG_CHF_MODULE_ID 30

#endif /*!_CONFIG_H*/
