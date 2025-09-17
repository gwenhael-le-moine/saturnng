#ifndef _CHF_WRAPPER_H
#  define _CHF_WRAPPER_H 1

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

.creation     :	19-Jan-1998

.description  :
  This header defines the following macros:

        - DEBUG0(debug_class, message_id)
        - DEBUG(debug_class, message_id, arg_1)
        - DEBUG(debug_class, message_id, arg_1, arg_2)
        - DEBUG(debug_class, message_id, arg_1, arg_2, arg_3)

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

.notes        :
  $Log: chf_wrapper.h,v $
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

  Revision 2.7  2000/09/19  10:51:13  cibrario
  Added new debug class: DEBUG_C_MOD_CACHE

  Revision 2.5  2000/09/14  14:34:13  cibrario
  Added new debug class: DEBUG_C_SERIAL

  Revision 1.1  1998/02/18  11:54:33  cibrario
  Initial revision
.- */

#  include "../libChf/src/Chf.h"

#  include "../options.h"

#  define _DEBUG_PREFIX( module_id, debug_class, message_id )                                                                              \
      {                                                                                                                                    \
          if ( config.debug_level & ( debug_class ) ) {                                                                                    \
    ChfGenerate( module_id, __FILE__, __LINE__, message_id, CHF_INFO

#  define _LOG_PREFIX( module_id, message_id, severity )                                                                                   \
      {                                                                                                                                    \
          if ( severity > CHF_INFO || config.verbose ) {                                                                                   \
    ChfGenerate( module_id, __FILE__, __LINE__, message_id, severity

#  define _SIGNAL_PREFIX( module_id, message_id, severity )                                                                                \
      {                                                                                                                                    \
          { ChfGenerate( module_id, __FILE__, __LINE__, message_id, severity

#  define _POSTFIX( module_id ) );                                                                                                         \
      ChfSignal( module_id );                                                                                                              \
      }                                                                                                                                    \
      }

#  define SIGNAL( module_id, message_id, severity, ... )                                                                                   \
      _SIGNAL_PREFIX( module_id, message_id, severity ), __VA_ARGS__ _POSTFIX( module_id )
#  define SIGNAL0( module_id, message_id, severity ) _SIGNAL_PREFIX( module_id, message_id, severity ) _POSTFIX( module_id )

#  define DEBUG( module_id, debug_class, message_id, ... )                                                                                 \
      _DEBUG_PREFIX( module_id, debug_class, message_id ), __VA_ARGS__ _POSTFIX( module_id )
#  define DEBUG0( module_id, debug_class, message_id ) _DEBUG_PREFIX( module_id, debug_class, message_id ) _POSTFIX( module_id )

#  define LOG( module_id, message_id, severity, ... ) _LOG_PREFIX( module_id, message_id, severity ), __VA_ARGS__ _POSTFIX( module_id )
#  define LOG0( module_id, message_id, severity ) _LOG_PREFIX( module_id, message_id, severity ) _POSTFIX( module_id )

#  define SUCCESS( module_id, message_id, ... ) LOG( module_id, message_id, CHF_SUCCESS, __VA_ARGS__ )
#  define SUCCESS0( module_id, message_id ) LOG0( module_id, message_id, CHF_SUCCESS )

#  define INFO( module_id, message_id, ... ) LOG( module_id, message_id, CHF_INFO, __VA_ARGS__ )
#  define INFO0( module_id, message_id ) LOG0( module_id, message_id, CHF_INFO )

#  define WARNING( module_id, message_id, ... ) SIGNAL( module_id, message_id, CHF_WARNING, __VA_ARGS__ )
#  define WARNING0( module_id, message_id ) SIGNAL0( module_id, message_id, CHF_WARNING )

#  define ERROR( module_id, message_id, ... ) SIGNAL( module_id, message_id, CHF_ERROR, __VA_ARGS__ )
#  define ERROR0( module_id, message_id ) SIGNAL0( module_id, message_id, CHF_ERROR )

#  define FATAL( module_id, message_id, ... ) SIGNAL( module_id, message_id, CHF_FATAL, __VA_ARGS__ )
#  define FATAL0( module_id, message_id ) SIGNAL0( module_id, message_id, CHF_FATAL )

#  define SIGNAL_ERRNO ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );

/*---------------------------------------------------------------------------
        Debug classes
  ---------------------------------------------------------------------------*/
typedef enum {
    DEBUG_C_MODULES = 0x4000,        /* Modules configuration */
    DEBUG_C_INT = 0x1000,            /* Interrupt activity */
    DEBUG_C_TIMERS = 0x0800,         /* Timers activity */
    DEBUG_C_SERIAL = 0x0400,         /* 2.5: Serial port activity */
    DEBUG_C_MOD_CACHE = 0x0200,      /* 2.7: Module cache */
    DEBUG_C_IMPLEMENTATION = 0x0100, /* Feature implementation */
    DEBUG_C_FLASH = 0x0080,          /* 3.3: Flash ROM */
    DEBUG_C_OPCODES = 0x0001,        /* OpCodes */
    DEBUG_C_NONE = 0,
} debug_class_t;

extern ChfTable message_table[];
extern size_t message_table_size;

#endif /*!_CHF_WRAPPER_H*/
