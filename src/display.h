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

.identifier   : $Id: display.h,v 4.1 2000/12/11 09:54:19 cibrario Rel $
.context      : SATURN, Saturn CPU / HP48 emulator
.title	      : $RCSfile: display.h,v $
.kind	      : C header
.author	      : Ivan Cibrario B.
.site	      : CSTV-CNR
.creation     :	28-Jan-1998
.keywords     : *
.description  :
  This header contains all definitions and declarations related to the
  HP48's LCD display emulator. References:

    SASM.DOC by HP  (HORN disk 4)
    Guide to the Saturn Processor Rev. 0.00f by Matthew Mastracci
    entries.srt by Mika Heiskanen  (mheiskan@vipunen.hut.fi)
    x48 source code by Eddie C. Dost  (ecd@dressler.de)

.include      : config.h, machdep.h, cpu.h, modules.h, X11/Xlib.h

.notes	      :
  $Log: display.h,v $
  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.10  2000/10/24 16:14:38  cibrario
  Added/Replaced GPL header

  Revision 1.1  1998/02/17 11:51:31  cibrario
  Initial revision


.- */


/*---------------------------------------------------------------------------
	Data type definitions - require config.h, machdep.h, cpu.h
  ---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------
	Macros
  ---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------
	Global variables
  ---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------
	Function prototypes
  ---------------------------------------------------------------------------*/

void InitLcd(Display *lcd_display, Window lcd_window,
  unsigned long lcd_fg_pixel, unsigned long lcd_bg_pixel);

void DrawLcd(void);
void RefreshLcd(void);
