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

.identifier   : $Id: debug.c,v 4.1 2000/12/11 09:54:19 cibrario Rel $
.context      : SATURN, Saturn CPU / HP48 emulator
.title	      : $RCSfile: debug.c,v $
.kind	      : C source
.author	      : Ivan Cibrario B.
.site	      : CSTV-CNR
.creation     :	28-Jan-1998
.keywords     : *
.description  :
  Debugging support.

.include      : debug.h

.notes	      :
  $Log: debug.c,v $
  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.10  2000/10/24 16:14:32  cibrario
  Added/Replaced GPL header

  Revision 1.1  1998/02/17 11:42:30  cibrario
  Initial revision


.- */

#include <stdio.h>

#include "config.h"
#include "debug.h"

#define CHF_MODULE_ID DEBUG_CHF_MODULE_ID
#include "libChf/src/Chf.h"

/*---------------------------------------------------------------------------
        Static/Global variables
  ---------------------------------------------------------------------------*/

int debug_level = DEBUG_C_NONE;

/*---------------------------------------------------------------------------
        Public functions
  ---------------------------------------------------------------------------*/

/* .+

.title	      : SetDebugLevel
.kind	      : C function
.creation     : 28-Jan-1998
.description  :
  If runtime debug support is enabled (symbol DEBUG defined during executable
  image build) this function updates the 'debug_level' flag, otherwise
  it signals a condition and does nothing more.

.call	      :
                SetDebugLevel(new_level)
.input	      :
                int new_level, new value of the debug_level flag
.output	      :
                void
.status_codes :
                DEBUG_W_NOT_SUPPORTED
.notes	      :
  1.1, 28-Jan-1998, creation

.- */
void SetDebugLevel( int new_level )
{
#ifdef DEBUG
    debug_level |= new_level;
#else
    CHF_Condition( DEBUG_CHF_MODULE_ID ) DEBUG_W_NOT_SUPPORTED, CHF_WARNING );
    ChfSignal( DEBUG_CHF_MODULE_ID );
#endif
}
