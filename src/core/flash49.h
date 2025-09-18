#ifndef _FLASH49_H
#  define _FLASH49_H 1

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

.creation     :	25-Sep-2000

.description  :
  This header contains all definitions and declarations related to the
  Internal Flash ROM emulation module of the HP49. References:

  Known deficiencies of the current Flash ROM emulation:

  - some (many) commands are not emulated, even BCS ones.
  - program/erase times are not emulated
  - suspension is not supported

  References:

    28F160S5/28F320S5 Data Sheet, by Intel Corp.

.notes        :
  $Log: flash49.h,v $
  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.10  2000/10/24 16:14:41  cibrario
  Added/Replaced GPL header

  Revision 3.3  2000/09/26 15:30:07  cibrario
  *** empty log message ***
.- */

#  include "types.h"

/*---------------------------------------------------------------------------
        Macro/Data type definitions
  ---------------------------------------------------------------------------*/

/* Command Set Definitions, Table 3 on Data Sheet.
   Both BCS and SCS commands are listed here; commands marked with
   (*) are not implemented.

   The last fixed byte of multibyte commands has a '_2' suffix.
 */
typedef enum {
    FLASH_CMD_READ_ARRAY = 0xFF,   /* BCS */
    FLASH_CMD_READ_ID = 0x90,      /* BCS (*) */
    FLASH_CMD_READ_QUERY = 0x98,   /* SCS (*) */
    FLASH_CMD_READ_SR = 0x70,      /* BCS */
    FLASH_CMD_CLR_SR = 0x50,       /* BCS */
    FLASH_CMD_WRITE_BUFFER = 0xE8, /* SCS */
    FLASH_CMD_WRITE_BUFFER_2 = 0xD0,
    FLASH_CMD_BW_PGM = 0x40,     /* BCS (*) */
    FLASH_CMD_BW_PGM_ALT = 0x10, /* BCS, alternate (*) */
    FLASH_CMD_BL_ERASE = 0x20,   /* BCS */
    FLASH_CMD_BL_ERASE_2 = 0xD0,
    FLASH_CMD_SUSPEND = 0xB0,    /* BCS (*) */
    FLASH_CMD_RESUME = 0xD0,     /* BCS (*) */
    FLASH_CMD_STS_CONFIG = 0xB8, /* SCS (*) */
    FLASH_CMD_BL_LB = 0x60,      /* SCS (*) */
    FLASH_CMD_BL_LB_SET = 0x01,
    FLASH_CMD_BL_LB_CLR = 0xD0,
    FLASH_CMD_CHIP_ERASE = 0x30, /* SCS (*) */
    FLASH_CMD_CHIP_ERASE_2 = 0xD0,
} flash_command_t;

/* Status Register bit masks, Table 15 on Data Sheet
 */
typedef enum {
    FLASH_SR_WSMS = 0x80,   /* WSM state, 0=busy, 1=ready */
    FLASH_SR_ESS = 0x40,    /* Erase suspend, 1=suspended */
    FLASH_SR_ECLBS = 0x20,  /* 1=Error during erasure */
    FLASH_SR_BWSLBS = 0x10, /* 1=Error during program */
    FLASH_SR_VPPS = 0x08,   /* 1=Vpp error */
    FLASH_SR_BWSS = 0x04,   /* Program suspend, 1=suspended */
    FLASH_SR_DPS = 0x02,    /* 1=Lock encountered */
} flash_status_register_bit_mask_t;

/* Extended Status Register bit masks, Table 16 on Data Sheet
 */
typedef enum {
    FLASH_XSR_WBS = 0x80, /* Write buffer status 1=available */
} flash_extended_status_register_bit_mask_t;

/* State of the Flash FSM, derived from command descriptions on
   pages 16...28 and from flowcharts on Figure 6...12 of Data Sheet
*/
enum FlashState {
    FLASH_ST_READ_ARRAY = 0, /* Read Array after CMD_READ_ARRAY */
    FLASH_ST_READ_SR,        /* Read Status Reg. after CMD_READ_SR */
    FLASH_ST_READ_XSR,       /* Read XSR after CMD_WRITE_BUFFER */
    FLASH_ST_WRITE_DATA_1,   /* Write data after ST_WRITE_COUNT */
    FLASH_ST_WRITE_DATA_N,   /* Write data after first write */
    FLASH_ST_WRITE_CONFIRM,  /* Write confirmation after (ST_WRITE_DATA)* */
    FLASH_ST_BL_ERASE,       /* Block erase started */
    FLASH_ST_N               /* Total # of FSM states */
};

/*---------------------------------------------------------------------------
        Chf condition codes
  ---------------------------------------------------------------------------*/
typedef enum {
    FLASH_I_READ = 101,        /* Read from address %x: %d */
    FLASH_I_WRITE = 102,       /* Write address %x, datum %x */
    FLASH_I_FSM = 103,         /* FSM from state %d, cycle %d */
    FLASH_I_FSM_AD = 104,      /* FSM address %x, data %x */
    FLASH_I_FSM_RESULT = 105,  /* FSM next state %d, result %x */
    FLASH_I_FSM_OP = 106,      /* FSM operation %s */
    FLASH_W_BAD_CMD = 201,     /* Bad cmd st%d, cycle%d, a%x, d%d */
    FLASH_W_BAD_ADDRESS = 202, /* Bad addr st%d, cycle%d, a%x, d%d */
    FLASH_E_xxx = 301,
    FLASH_F_xxx = 401,
} flash49_chf_conditions_codes_t;

/*---------------------------------------------------------------------------
        Function prototypes
  ---------------------------------------------------------------------------*/

/* Read/Write operations, nibble-by-nibble */
Nibble FlashRead49( XAddress address );
void FlashWrite49( XAddress address, Nibble datum );

#endif /*!_FLASH49_H*/
