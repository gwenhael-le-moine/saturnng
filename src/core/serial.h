#ifndef _SERIAL_H
#  define _SERIAL_H 1

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





.author       : Ivan Cibrario B.

.creation     :	13-Sep-2000

.description  :
  This header contains all definitions and declarations related to the
  serial port emulation modules of the HP48. References:

    SASM.DOC by HP  (HORN disk 4)
    Guide to the Saturn Processor Rev. 0.00f by Matthew Mastracci
    entries.srt by Mika Heiskanen  (mheiskan@vipunen.hut.fi)
    x48 source code by Eddie C. Dost  (ecd@dressler.de)

.notes        :
  $Log: serial.h,v $
  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.16  2000/11/21 16:41:44  cibrario
  Ultrix/IRIX support:
  - New condition code: SERIAL_W_NOPTY

  Revision 3.10  2000/10/24 16:14:59  cibrario
  Added/Replaced GPL header

  Revision 3.2  2000/09/22 14:36:53  cibrario
  Implemented preliminary support of HP49 hw architecture:
  - New status codes: SERIAL_F_TCGETATTR, SERIAL_F_TCSETATTR

 * Revision 2.6  2000/09/15  09:25:06  cibrario
 * Added definition of the following status codes:
 * SERIAL_F_OPEN_MASTER, SERIAL_F_GRANTPT, SERIAL_F_UNLOCKPT,
 * SERIAL_F_OPEN_SLAVE, SERIAL_F_PUSH; the implementation of
 * USE_STREAMSPTY needs them.
 *
 * Revision 2.5  2000/09/14  15:43:57  cibrario
 * *** empty log message ***
 *

.- */

#  include "cpu.h"

/*---------------------------------------------------------------------------
        Macro/Data type definitions
  ---------------------------------------------------------------------------*/

#  define SERIAL_RCS_INFO "$Revision: 4.1 $ $State: Rel $"

/*---------------------------------------------------------------------------
        Chf condition codes
  ---------------------------------------------------------------------------*/

#  define SERIAL_I_CALLED 101      /* Function %s called */
#  define SERIAL_I_REVISION 102    /* Serial port emulation rev. %s */
#  define SERIAL_I_READ 103        /* Read %s -> %x */
#  define SERIAL_I_WRITE 104       /* Write %s %x -> %x */
#  define SERIAL_I_RBR 105         /* Read RBR -> %x */
#  define SERIAL_I_TBR 106         /* Write TBR <- %x */
#  define SERIAL_I_PTY_NAME 107    /* pty name is %s */
#  define SERIAL_W_EMPTY_RRB 201   /* Read from empty RX buffer, rcs=%x */
#  define SERIAL_W_FULL_TRB 202    /* Write into full TX buffer, tcs=%x */
#  define SERIAL_W_NOPTY 203       /* 3.16: Pty support not available */
#  define SERIAL_E_TRB_DRAIN 301   /* Error draining TX buffer */
#  define SERIAL_E_RRB_CHARGE 302  /* Error charging RX buffer */
#  define SERIAL_E_PTY_CLOSE 303   /* Error closing pty */
#  define SERIAL_F_OPENPTY 401     /* openpty() failed on master pty */
#  define SERIAL_F_FCNTL 402       /* fcntl() failed on master pty */
#  define SERIAL_F_OPEN_MASTER 403 /* Can't open pty master %s */
#  define SERIAL_F_GRANTPT 404     /* grantpt() failed on master pty */
#  define SERIAL_F_UNLOCKPT 405    /* unlockpt() failed on master pty */
#  define SERIAL_F_OPEN_SLAVE 406  /* Can't open pty slave %s */
#  define SERIAL_F_PUSH 407        /* ioctl(I_PUSH,%s) failed on slave */
#  define SERIAL_F_TCGETATTR 408   /* tcgetattr() failed on master */
#  define SERIAL_F_TCSETATTR 409   /* tcsetattr() failed on master */

/*---------------------------------------------------------------------------
        Function prototypes
  ---------------------------------------------------------------------------*/

/* Initialization */
const char* SerialInit( void );
void SerialClose( void );

/* Information about slave side of pty */
const char* SerialPtyName( void );

/* Register read */
Nibble Serial_IOC_Read( void );
Nibble Serial_RCS_Read( void );
Nibble Serial_TCS_Read( void );
int8 Serial_RBR_Read( void );

/* Register write */
void Serial_IOC_Write( Nibble n );
void Serial_RCS_Write( Nibble n );
void Serial_TCS_Write( Nibble n );
void Serial_CRER_Write( Nibble n );
void Serial_TBR_Write( int8 d );

/* Event handling */
void HandleSerial( void );

#endif /*!_SERIAL_H*/
