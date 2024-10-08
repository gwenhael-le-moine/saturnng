#ifndef _DEBUG_H
#define _DEBUG_H 1

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

.identifier   : $Id: debug.h,v 4.1 2000/12/11 09:54:19 cibrario Rel $
.context      : SATURN, Saturn CPU / HP48 emulator
.title	      : $RCSfile: debug.h,v $
.kind	      : C header
.author	      : Ivan Cibrario B.
.site	      : CSTV-CNR
.creation     :	19-Jan-1998
.keywords     : *
.description  :
  This header defines the following macros:

        - debug0(debug_class, condition_code)
        - debug1(debug_class, condition_code, arg_1)
        - debug2(debug_class, condition_code, arg_1, arg_2)
        - debug3(debug_class, condition_code, arg_1, arg_2, arg_3)

  used throughout the source code for debugging purposes.

  If the DEBUG cpp symbol is defined, each invocation of these macros is
  expanded into a block of code that, at runtime, checks if the global
  variable 'debug_level' has set at least one of the bit 'debug_class' has
  set (in other words, it checks if the current debug level enables at
  least one of the classes to which the debugging condition belongs).

  If this condition is met, the code generates and immediately signals
  the given condition code using the Chf facility, with severity CHF_INFO,
  otherwise nothing is done.

  The arguments arg_1, arg_2, and arg_3 are used as additional arguments
  of the condition.

  If the DEBUG cpp symbol is not defined, the macros are defined as a null
  macros.

.include      : Chf.h

.notes	      :
  $Log: debug.h,v $
  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.13  2000/11/09 11:28:34  cibrario
  Revised to add file selection box GUI element, CPU halt/run
  requests and emulator's extended functions:

  - Added new debug class: DEBUG_C_X_FUNC

  Revision 3.10  2000/10/24 16:14:32  cibrario
  Added/Replaced GPL header

  Revision 3.3  2000/09/26 15:20:40  cibrario
  Revised to implement Flash ROM write access:
  - Added new debug class: DEBUG_C_FLASH

 * Revision 2.7  2000/09/19  10:51:13  cibrario
 * Added new debug class: DEBUG_C_MOD_CACHE
 *
 * Revision 2.5  2000/09/14  14:34:13  cibrario
 * Added new debug class: DEBUG_C_SERIAL
 *
 * Revision 1.1  1998/02/18  11:54:33  cibrario
 * Initial revision
 *

.- */

extern int debug_level;

#ifdef DEBUG
#  define debug_preamble( debug_class, condition_code )                                                                                    \
      {                                                                                                                                    \
          if ( debug_level & ( debug_class ) ) {                                                                                           \
          CHF_Condition( CHF_MODULE_ID ) condition_code, CHF_INFO

#  define debug_postamble                                                                                                                  \
      ChfEnd;                                                                                                                              \
      ChfSignal( CHF_MODULE_ID );                                                                                                          \
      }                                                                                                                                    \
      }

#  define debug0( debug_class, condition_code ) debug_preamble( debug_class, condition_code ) debug_postamble

#  define debug1( debug_class, condition_code, arg_1 ) debug_preamble( debug_class, condition_code ), arg_1 debug_postamble

#  define debug2( debug_class, condition_code, arg_1, arg_2 ) debug_preamble( debug_class, condition_code ), arg_1, arg_2 debug_postamble

#  define debug3( debug_class, condition_code, arg_1, arg_2, arg_3 )                                                                       \
      debug_preamble( debug_class, condition_code ), arg_1, arg_2, arg_3 debug_postamble

#else

#  define debug0( debug_class, condition_code )
#  define debug1( debug_class, condition_code, arg_1 )
#  define debug2( debug_class, condition_code, arg_1, arg_2 )
#  define debug3( debug_class, condition_code, arg_1, arg_2, arg_3 )

#endif

/*---------------------------------------------------------------------------
        Debug classes
  ---------------------------------------------------------------------------*/

#define DEBUG_C_TRACE 0x8000          /* Function Call trace */
#define DEBUG_C_MODULES 0x4000        /* Modules configuration */
#define DEBUG_C_DISPLAY 0x2000        /* Display activity */
#define DEBUG_C_INT 0x1000            /* Interrupt activity */
#define DEBUG_C_TIMERS 0x0800         /* Timers activity */
#define DEBUG_C_SERIAL 0x0400         /* 2.5: Serial port activity */
#define DEBUG_C_MOD_CACHE 0x0200      /* 2.7: Module cache */
#define DEBUG_C_IMPLEMENTATION 0x0100 /* Feature implementation */
#define DEBUG_C_FLASH 0x0080          /* 3.3: Flash ROM */
#define DEBUG_C_X_FUNC 0x0040         /* 3.13: Extended functions */
#define DEBUG_C_REVISION 0x0010       /* Revision information */
#define DEBUG_C_X11 0x0001            /* X11 Interface */
#define DEBUG_C_NONE 0

/*---------------------------------------------------------------------------
        Chf condition codes
  ---------------------------------------------------------------------------*/

#define DEBUG_W_NOT_SUPPORTED 201 /* Debug not supported */
#define DEBUG_W_BAD_CMD 202       /* Invalid command */

/*---------------------------------------------------------------------------
        Function prototypes
  ---------------------------------------------------------------------------*/

void SetDebugLevel( int new_level );

#endif /*!_DEBUG_H*/
