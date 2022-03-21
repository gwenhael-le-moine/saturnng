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

.identifier   : $Id: x11.h,v 4.1 2000/12/11 09:54:19 cibrario Rel $
.context      : SATURN, Saturn CPU / HP48 emulator
.title	      : $RCSfile: x11.h,v $
.kind	      : C header
.author	      : Ivan Cibrario B.
.site	      : CSTV-CNR
.creation     :	17-Feb-1998
.keywords     : *
.description  :
  X11 header.

.include      : *

.notes	      :
  $Log: x11.h,v $
  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.15  2000/11/15 14:14:45  cibrario
  GUI enhancements and assorted bug fixes:

  - Defined new condition codes: X11_W_TOO_MANY_MSG and X11_F_NO_KEYS

  Revision 3.14  2000/11/13 11:31:16  cibrario
  Implemented fast load/save; improved keyboard interface emulation at
  high emulated CPU speed:

  - Revision number bump with no changes

  Revision 3.13  2000/11/09 11:38:50  cibrario
  Revised to add file selection box GUI element, CPU halt/run
  requests and emulator's extended functions:

  - New data type: FsbContinuation
  - New status codes: X11_I_HIER, X11_I_HIER_NC, X11_W_NO_FSB_CONT,
    X11_E_NO_FSB_TSEG
  - New prototype: ActivateFSB()

  Revision 3.10  2000/10/24 16:15:04  cibrario
  Added/Replaced GPL header

  Revision 3.1  2000/09/20 14:02:31  cibrario
  Revised to implement passive CPU shutdown:
  - added prototype of IdleXLoop()

 * Revision 2.8  2000/09/19  12:57:35  cibrario
 * Minor revision to both Saturn.ad and x11.c, to allow keyboard
 * shortcuts to work on the whole surface of the application's window.
 *
 * Revision 2.3  2000/09/12  12:38:10  cibrario
 * Added definition of X11_RCS_INFO (RCS identification of GUI subsystem)
 *
 * Revision 1.2  2000/09/08  15:29:07  cibrario
 * Changes required by the new GUI:
 * - Updated templates of status codes X11_I_KEY_PRESS and
 *   X11_I_KEY_RELEASE.
 * - Removed status code X11_I_X_EVENT.
 * - Added status codes X11_I_FACE, X11_I_NKEYS, X11_I_KEY,
 *   X11_I_FOUND_CS, X11_W_BAD_ACTION_CALL, X11_W_UNKNOWN_ATOM,
 *   X11_E_NO_WM_COMMAND.
 * - Declared prototype of new function InitializeGui().
 *
 * Revision 1.1  1998/02/19  11:42:59  cibrario
 * Initial revision
 *

.- */


/*---------------------------------------------------------------------------
	Macro/Data type definitions
  ---------------------------------------------------------------------------*/

#define X11_RCS_INFO		"$Revision: 4.1 $ $State: Rel $"


typedef void (*FsbContinuation)(int proceed, char *file_name);


/*---------------------------------------------------------------------------
	Global variables
  ---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------
	Chf condition codes
  ---------------------------------------------------------------------------*/

#define X11_I_CALLED		101	/* Function %s called */
#define X11_I_LCD_PAR		102	/* LCD Parameter %s: %x */
#define X11_I_LCD_EXPOSE	103	/* LCD X Expose event, count=%d */
#define X11_I_KEY_PRESS		104	/* 2.1: Pressed key %s */
#define X11_I_KEY_RELEASE	105	/* 2.1: Released key %s */
#define X11_I_REVISION		107	/* X11 interface revision: %s */
#define X11_I_USAGE		108	/* Usage %s: ... */
#define X11_I_FACE		109	/* Selected face %s */
#define X11_I_NKEYS		110	/* The face has %d keys */
#define X11_I_KEY		111	/* Creating key %d, inOut %s */
#define X11_I_FOUND_CS		112	/* Found cs widget %s, value %s */
#define X11_I_HIER		113	/* Traversing widget %s */
#define X11_I_HIER_NC		114	/* Current widget has %d children */
#define X11_W_BAD_ACTION_CALL	201	/* Xt Action called with %d args */
#define X11_W_UNKNOWN_ATOM	202	/* X Atom %s unknown */
#define X11_W_NO_FSB_CONT	203	/* 3.13: FSB continuation not set */
#define X11_W_TOO_MANY_MSG	204	/* 3.15: Too many messages (max %d) */
#define X11_E_BAD_OPTION	301	/* Invalid option: %s */
#define X11_E_NO_WM_COMMAND	302	/* WM_COMMAND property bad/not set */
#define X11_E_NO_FSB_TSEG	303	/* 3.13: No txt seg in fsb XmString */
#define X11_F_X_ERROR		401	/* X Window System fatal error */
#define X11_F_NO_KEYS		402	/* 3.15: Face has no keys */


/*---------------------------------------------------------------------------
	Function prototypes
  ---------------------------------------------------------------------------*/

void HandleXEvents(void);
void IdleXLoop(unsigned long max_wait);

void InitializeGui(int argc, char *argv[]);

void ActivateFSB(char *title, char *file_name, FsbContinuation continuation);
