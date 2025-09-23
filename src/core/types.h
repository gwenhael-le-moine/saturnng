#ifndef _MACHDEP_H
#  define _MACHDEP_H 1

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

.creation     :	28-Jan-1998

.description  :
  type definitions.

.notes        :
  $Log: types.h,v $
  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.10  2000/10/24 16:14:45  cibrario
  Added/Replaced GPL header

  Revision 1.1  1998/02/17 11:58:48  cibrario
  Initial revision
.- */

/*---------------------------------------------------------------------------
        Data type definitions
        All Data types must be SIGNED
  ---------------------------------------------------------------------------*/

/* Machine-dependent */
typedef char int4;
typedef char int8;
typedef int int12;
typedef int int16;
typedef int int20;
typedef int int32;

typedef int4 Nibble;
typedef int8 Byte;
typedef int20 Address;
typedef int12 OutputRegister;
typedef int16 InputRegister;

/* The XAddress data type holds extended addresses used to access Port 2 */
typedef int32 XAddress;

#endif /*!_MACHDEP_H*/
