#ifndef _X_FUNC_H
#  define _X_FUNC_H 1

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

.creation     :	3-Nov-2000

.description  :
  Main header for the emulator's extended functions. References:

    SASM.DOC by HP  (HORN disk 4)
    Guide to the Saturn Processor Rev. 0.00f by Matthew Mastracci

.notes        :
  $Log: x_func.h,v $
  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.14  2000/11/13 11:13:14  cibrario
  Implemented fast load/save; improved keyboard interface emulation at
  high emulated CPU speed:

  - Defined new function codes X_FUNC_KGET, X_FUNC_SEND
  - Defined new status codes: X_FUNC_W_ABORTED, X_FUNC_W_FAILED,
    X_FUNC_W_ABORTED, X_FUNC_W_FAILED, X_FUNC_E_NO_BIN_HDR,
    X_FUNC_M_KGET, X_FUNC_M_SEND

  Revision 3.13  2000/11/09 11:42:22  cibrario
  *** empty log message ***
.- */

#  include "cpu.h"

/*---------------------------------------------------------------------------
        Chf condition codes
  ---------------------------------------------------------------------------*/
typedef enum {
    X_FUNC_I_CALLED = 101,     /* Function %s called */
    X_FUNC_I_CODE = 102,       /* Function code %d */
    X_FUNC_I_SET_SPEED = 103,  /* Speed set to %dMhz (%d mult.) */
    X_FUNC_I_MAX_SPEED = 104,  /* Emulator at max speed */
    X_FUNC_I_FILE_NAME = 105,  /* Transferring file name %s */
    X_FUNC_I_KGET = 106,       /* Kget start:%x end:%x hdr:%s */
    X_FUNC_I_SEND = 107,       /* Send start:%x end:%x hdr:%s */
    X_FUNC_W_BAD_CODE = 201,   /* Bad function code %d ignored */
    X_FUNC_W_ABORTED = 202,    /* Aborted by user */
    X_FUNC_W_FAILED = 203,     /* Operation failed */
    X_FUNC_E_NO_HALT = 301,    /* Cpu halt not allowed */
    X_FUNC_E_NO_SPEED = 302,   /* No speed control available */
    X_FUNC_E_NO_BIN_HDR = 303, /* Can't determine hdr for hw %s */
    X_FUNC_F_xxx = 401,
    X_FUNC_M_KGET = 501, /* FSB title for Kget function */
    X_FUNC_M_SEND = 502, /* FSB title for Send function */
} x_func_chf_condition_code_t;

/*---------------------------------------------------------------------------
        Function prototypes
  ---------------------------------------------------------------------------*/

void ExtendedFunction( Nibble function_code ); /* used in hdw.c */

#endif /*!_X_FUNC_H*/
