#ifndef _ARGS_H
#define _ARGS_H 1

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

.identifier   : $Id: args.h,v 4.1 2000/12/11 09:54:19 cibrario Rel $
.context      : SATURN, Saturn CPU / HP48 emulator
.title	      : $RCSfile: args.h,v $
.kind	      : C header
.author	      : Ivan Cibrario B.
.site	      : CSTV-CNR
.creation     :	19-Jan-1998
.keywords     : *
.description  :
  This header declares a global data structure containing the emulator
  invocation arguments; this data structure is initialized before startup,
  either by means of argc/argv or in other ways.

.include      : config.h

.notes	      :
  $Log: args.h,v $
  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.15  2000/11/15 14:03:06  cibrario
  GUI enhancements and assorted bug fixes:
  - added .batchXfer boolean (corresponding to command-line option
    -batchXfer) to struct Args

  Revision 3.10  2000/10/24 16:14:25  cibrario
  Added/Replaced GPL header

  Revision 2.1  2000/09/08 14:45:06  cibrario
  Added the following fields to 'struct Args': reset, monitor, hw.
  The latter mimes the setting of the -hw command line option and
  of the *hw top-level application resource; the former two have
  been added to export them cleanly from the GUI modules.

 * Revision 1.1  1998/02/17  11:58:37  cibrario
 * Initial revision
 *

.- */

/*---------------------------------------------------------------------------
        Data type definitions - require config.h
  ---------------------------------------------------------------------------*/

struct Args {
    int reset;     /* 2.1: Force emulator reset */
    int monitor;   /* 2.1: Call monitor() on startup */
    int batchXfer; /* 3.15: Non-interactive file transfers */
    char* mod_file_name;
    char* cpu_file_name;
    char* hdw_file_name;
    char* rom_file_name;
    char* ram_file_name;
    char* port_1_file_name;
    char* port_2_file_name;
    char* hw; /* 2.1: Hardware configuration (unused) */
};

/*---------------------------------------------------------------------------
        Global variables
  ---------------------------------------------------------------------------*/

extern struct Args args;

#endif /*!_ARGS_H*/
