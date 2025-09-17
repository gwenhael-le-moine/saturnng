#ifndef _KEYB_H
#  define _KEYB_H 1

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

.creation     :	17-Feb-1998

.description  :
  Header for the keyboard emulation module. References:

    SASM.DOC by HP  (HORN disk 4)
    Guide to the Saturn Processor Rev. 0.00f by Matthew Mastracci
    entries.srt by Mika Heiskanen  (mheiskan@vipunen.hut.fi)
    x48 source code by Eddie C. Dost  (ecd@dressler.de)

.notes        :
  $Log: keyb.h,v $
  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.13  2000/11/09 11:33:28  cibrario
  Revised to add file selection box GUI element, CPU halt/run
  requests and emulator's extended functions:

  - Added prototype of KeybReset()

  Revision 3.10  2000/10/24 16:14:45  cibrario
  Added/Replaced GPL header

  Revision 2.1  2000/09/08 15:02:14  cibrario
  Updated prototypes of KeybPress() and KeybRelease() to accommodate
  the new GUI.  As a consequence, suppressed the definition of
  'enum Key', no longer needed.

  Revision 1.1  1998/02/17  11:53:34  cibrario
  Initial revision
.- */

#  include "cpu.h"

/*---------------------------------------------------------------------------
        Function prototypes
  ---------------------------------------------------------------------------*/

void KeybRSI( void );
InputRegister KeybIN( OutputRegister out );
void KeybPress( int keycode );
void KeybRelease( int keycode );

#endif /*!_KEYB_H*/
