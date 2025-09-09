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

.creation     :	23-Jan-1998

.description  :
  This module emulates the Hdw peripheral module, that controls all
  peripheral devices of the HP48. The Hdw Read/Write functions simply update
  the contents of the mod_status.hdw structure to reflect the contents of the
  actual Hdw registers. The actual emulation of the devices is performed
  by other source modules. References:

    SASM.DOC by HP  (HORN disk 4)
    Guide to the Saturn Processor Rev. 0.00f by Matthew Mastracci
    entries.srt by Mika Heiskanen  (mheiskan@vipunen.hut.fi)
    x48 source code by Eddie C. Dost  (ecd@dressler.de)

.notes        :
  $Log: hdw.c,v $
  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.13  2000/11/09 11:32:05  cibrario
  Revised to add file selection box GUI element, CPU halt/run
  requests and emulator's extended functions:

  - Added code to trampoline to ExtendedFunction() when the calculator
    writes something into the lower nibble of the serial port's
    Receiver Buffer Register.

  Revision 3.10  2000/10/24 16:14:42  cibrario
  Added/Replaced GPL header

  Revision 3.2  2000/09/22 13:46:30  cibrario
  Implemented preliminary support of HP49 hw architecture:
  - The HP49 firmware (1.19-4) reads a nibble from 0x30 for unknown reasons;
    enabled reads from relative
    addresses 0x30..0x34 without signaling a warning.

 * Revision 3.1  2000/09/20  13:48:52  cibrario
 * Minor updates and fixes to avoid gcc compiler warnings on Solaris
 * when -ansi -pedantic -Wall options are selected.
 *
 * Revision 2.5  2000/09/14  15:08:59  cibrario
 * Update HdwRead() and HdwWrite() to support serial port emulation;
 * read/write from/to serial port register are mapped into invocation
 * of functions in the serial port emulation module.  This module
 * merely provides buffering for multi-nibble hdw registers.
 *
 * Revision 2.4  2000/09/12  15:24:27  cibrario
 * Bug fix and update required to implement emulation of Port 1 and 2:
 * - fixed an improper memset() call in HdwInit()
 * - HdwRead() now returns the value of mod_status.hdw_card_status when
 *   relative address 0x0F is read from.
 *
 * Revision 1.1  1998/02/17  11:49:59  cibrario
 * Initial revision
 *

.- */

#include <string.h> /* 3.1: memset() */
#include "../libChf/src/Chf.h"

#include "config.h"
#include "machdep.h"
#include "cpu.h"
#include "modules.h"
#include "disk_io.h"
#include "serial.h" /* 2.5: Serial port emulation module */
#include "x_func.h" /* 3.13: Extended emulator functions */
#include "debug.h"

static const int addr_mask[] = { 0x0000F, 0x000F0, 0x00F00, 0x0F000, 0xF0000 };

static const int32 int32_mask[] = { 0x0000000F, 0x000000F0, 0x00000F00, 0x0000F000, 0x000F0000, 0x00F00000, 0x0F000000, 0xF0000000 };

/* .+

.creation     : 23-Jan-1998
.description  :
  This function initializes the Hdw module, restoring the status of the
  peripheral devices associated to it from disk.

.call         :
                HdwInit();
.input        :
                void
.output       :
                void
.status_codes :
                MOD_I_CALLED
                MOD_W_HDW_INIT
.notes        :
  1.1, 23-Jan-1998, creation
  2.4, 11-Sep-2000, bug fix:
    memset() invocation was improper, and could lead to memory corruption

.- */
void HdwInit( void )
{
    debug1( MOD_CHF_MODULE_ID, DEBUG_C_TRACE, MOD_I_CALLED, "HdwInit" );

    if ( ReadStructFromFile( config.hdw_path, sizeof( mod_status.hdw ), &mod_status.hdw ) ) {
        ChfGenerate( MOD_CHF_MODULE_ID, __FILE__, __LINE__, MOD_W_HDW_INIT, CHF_WARNING );
        ChfSignal( MOD_CHF_MODULE_ID );

        ( void )memset( &mod_status.hdw, 0, sizeof( mod_status.hdw ) );
    }
}

/* .+

.creation     : 11-Feb-1998
.description  :
  This function saves the current status of the peripheral devices associated
  to the Hdw module to disk.

.call         :
                HdwSave();
.input        :
                void
.output       :
                void
.status_codes :
                MOD_I_CALLED
                MOD_E_HDW_SAVE
.notes        :
  1.1, 11-Feb-1998, creation

.- */
void HdwSave( void )
{
    debug1( MOD_CHF_MODULE_ID, DEBUG_C_TRACE, MOD_I_CALLED, "HdwSave" );

    if ( WriteStructToFile( &mod_status.hdw, sizeof( mod_status.hdw ), config.hdw_path ) ) {
        ChfGenerate( MOD_CHF_MODULE_ID, __FILE__, __LINE__, MOD_E_HDW_SAVE, CHF_ERROR );
        ChfSignal( MOD_CHF_MODULE_ID );
    }
}

/* .+

.creation     : 23-Jan-1998
.description  :
  This function reads a nibble from the Hdw module.

.call         :
                d = HdwRead(rel_address);
.input        :
                Address rel_address, relative address
.output       :
                Nibble d, data
.status_codes :
                MOD_I_CALLED
                MOD_W_HDW_READ
.notes        :
  1.1, 23-Jan-1998, creation
  2.4, 11-Sep-2000, update
    - read from rel_address 0x0F now returns the current value of
      mod_status.hdw.card_status; its value is set during the
      initialization of other peripheral modules.
  2.5, 14-Sep-2000, update
    - added support for serial port emulation

.- */
Nibble HdwRead( Address rel_address )
{
    debug1( MOD_CHF_MODULE_ID, DEBUG_C_TRACE, MOD_I_CALLED, "HdwRead" );

    /* In the following switch, each case corresponds to one hdw register.
       If the register must be read from the shadow space mod_status.hdw.hdw[],
       simply put a break in the case, otherwise code any special action for
       the register and end the case with a return.
    */
    switch ( rel_address ) {
        case 0x00: /* LCD driver registers */
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x0B:
        case 0x0C:
        case 0x25:
        case 0x26:
        case 0x27:
        case 0x28:
        case 0x29:
            break;

        case 0x04: /* CRC register */
        case 0x05:
        case 0x06:
        case 0x07:
            return ( Nibble )( ( mod_status.hdw.crc >> ( ( rel_address - 0x04 ) * 4 ) ) & 0x0F );

        case 0x08: /* Power status */
            /* No power status related interrupt have occoured */
            return ( Nibble )0;

        case 0x09: /* Power control */
            break;

        case 0x0D: /* Serial port baud-rate register */
            break;

        case 0x10: /* Serial port interrupt and I/O control register */
            return Serial_IOC_Read();

        case 0x11: /* Serial port receiver control/status register */
            return Serial_RCS_Read();

        case 0x12: /* Serial port transmitter control/status register */
            return Serial_TCS_Read();

        /* Serial port receiver buffer register; the actual read takes place
           when the LS nibble is read; serial_rbr buffers the MS nibble.
        */
        case 0x14:
            return ( mod_status.hdw.serial_rbr = Serial_RBR_Read() ) & 0x0F;

        case 0x15:
            return ( mod_status.hdw.serial_rbr >> 4 ) & 0x0F;

        case 0x0E: /* Card interface */
            break;

        case 0x0F: /* Card interface */
            /* 2.4: Return current card status */
            return mod_status.hdw.card_status;

        case 0x18: /* Service request */
        case 0x19:
            break;

        case 0x1A: /* IR registers */
        case 0x1C:
        case 0x1D:
            break;

        case 0x1B: /* Base nibble offset */
            break;

        case 0x1E: /* Scratch pad */
            break;

        case 0x1F: /* Base Nibble */
            break;

        case 0x2E: /* Timer 1 Control */
            return mod_status.hdw.t1_ctrl;

        case 0x2F: /* Timer 2 Control */
            return mod_status.hdw.t2_ctrl;

        /* 3.2: The HP49 firmware (1.19-4) reads a nibble from 0x30 */
        case 0x30:
        case 0x31:
        case 0x32:
        case 0x33:
        case 0x34:
            return ( Nibble )0x0;

        case 0x37: /* Timer 1 value */
            return mod_status.hdw.t1_val;

        case 0x38: /* Timer 2 value */
        case 0x39:
        case 0x3A:
        case 0x3B:
        case 0x3C:
        case 0x3D:
        case 0x3E:
        case 0x3F:
            return ( Nibble )( ( mod_status.hdw.t2_val >> ( ( rel_address - 0x38 ) * 4 ) ) & 0x0F );

        default:
            ChfGenerate( MOD_CHF_MODULE_ID, __FILE__, __LINE__, MOD_W_HDW_READ, CHF_WARNING, rel_address );
            ChfSignal( MOD_CHF_MODULE_ID );
            return ( Nibble )0xF;
    }

    /* Read from hdw register array */
    return mod_status.hdw.hdw[ rel_address ];
}

/* .+

.creation     : 23-Jan-1998
.description  :
  This function writes a nibble to the Hdw module.

.call         :
                HdwWrite(rel_address, data);
.input        :
                Address rel_address, relative address
                Nibble data, data to be written
.output       :
                void
.status_codes :
                MOD_I_CALLED
                MOD_W_HDW_WRITE
.notes        :
  1.1, 23-Jan-1998, creation
  2.5, 14-Sep-2000, update
    - added support for serial port emulation

.- */
void HdwWrite( Address rel_address, Nibble data )
{
    debug1( MOD_CHF_MODULE_ID, DEBUG_C_TRACE, MOD_I_CALLED, "HdwWrite" );

    /* This switch has a case for each 'known' hdw register. The code inside the
       case performs the actions specific for that register; the code following
       the switch, instead, simply takes care to shadow the hdw register into
       the mod_status.hdw.hdw[] array
    */
    switch ( rel_address ) {
        case 0x00: /* LCD horizontal offset, LCD enable flag */
            mod_status.hdw.lcd_offset = ( int )data & 0x07;
            mod_status.hdw.lcd_on = ( ( data & 0x08 ) != 0 );
            break;

        case 0x01: /* LCD contrast, LS nibble */
            mod_status.hdw.lcd_contrast &= 0x10;
            mod_status.hdw.lcd_contrast |= ( int )data;
            break;

        case 0x02: /* LCD contrast, MS bit */
            mod_status.hdw.lcd_contrast &= 0x0F;
            mod_status.hdw.lcd_contrast |= ( ( ( int )data & 0x01 ) << 4 );
            break;

        case 0x03: /* LCD test control */
            break;

        case 0x04: /* CRC register */
        case 0x05:
        case 0x06:
        case 0x07:
            mod_status.hdw.crc &= ~addr_mask[ rel_address - 0x04 ];
            mod_status.hdw.crc |= ( ( int )data << ( ( rel_address - 0x04 ) * 4 ) );
            break;

        case 0x08: /* Power status and power control */
        case 0x09:
            break;

        case 0x0B: /* LCD annunciator control (low nibble) */
            mod_status.hdw.lcd_ann &= 0xF0;
            mod_status.hdw.lcd_ann |= ( int )data;
            break;

        case 0x0C: /* LCD annunciator control (high nibble) */
            mod_status.hdw.lcd_ann &= 0x0F;
            mod_status.hdw.lcd_ann |= ( ( int )data << 4 );
            break;

        case 0x0D: /* Serial port baud rate */
            break;

        case 0x0E: /* Card interface */
        case 0x0F:
            break;

        case 0x10: /* Serial port interrupt and I/O control/status */
            Serial_IOC_Write( data );
            break;

        case 0x11: /* Serial port receiver control/status register */
            Serial_RCS_Write( data );
            break;

        case 0x12: /* Serial port transmitter control/status register */
            Serial_TCS_Write( data );
            break;

        case 0x13: /* Clear serial port receive error */
            Serial_CRER_Write( data );
            break;

        /* 3.13: A write operation into the receiver buffer register
           triggers an extended emulator function.
        */
        case 0x14:
            ExtendedFunction( data );
            break;

        /* Serial port transmitter buffer; the actual write takes place
           when the MS nibble is written; serial_tbr buffers the LS nibble.
        */
        case 0x16:
            mod_status.hdw.serial_tbr = ( mod_status.hdw.serial_tbr & 0xF0 ) | ( int8 )data;
            break;

        case 0x17:
            mod_status.hdw.serial_tbr = ( mod_status.hdw.serial_tbr & 0x0F ) | ( ( int8 )data << 4 );
            Serial_TBR_Write( mod_status.hdw.serial_tbr );
            break;

        case 0x18: /* Service request */
        case 0x19:
            break;

        case 0x1A: /* IR Control Register */
            break;

        case 0x1B: /* Base nibble offset */
            break;

        case 0x1C: /* IR Status Register */
            break;

        case 0x1D: /* IR Led Buffer */
            break;

        case 0x1E: /* Scratch Pad */
            break;

        case 0x1F: /* Base Nibble */
            break;

        case 0x20: /* LCD base address register (write only) */
        case 0x21:
        case 0x22:
        case 0x23:
        case 0x24:
            mod_status.hdw.lcd_base_addr &= ~addr_mask[ rel_address - 0x20 ];
            mod_status.hdw.lcd_base_addr |= ( ( int )data << ( ( rel_address - 0x20 ) * 4 ) );
            break;

        case 0x25: /* LCD line offset register */
        case 0x26:
        case 0x27:
            mod_status.hdw.lcd_line_offset &= ~addr_mask[ rel_address - 0x25 ];
            mod_status.hdw.lcd_line_offset |= ( ( int )data << ( ( rel_address - 0x25 ) * 4 ) );
            break;

        case 0x28: /* LCD vertical line count (low nibble) */
            mod_status.hdw.lcd_vlc &= 0x30;
            mod_status.hdw.lcd_vlc |= ( int )data;
            break;

        case 0x29: /* LCD vertical line count (higher 2 bits), others (TBD) */
            mod_status.hdw.lcd_vlc &= 0x0F;
            mod_status.hdw.lcd_vlc |= ( ( ( int )data & 0x03 ) << 4 );
            break;

        case 0x2E: /* Timer 1 Control */
            mod_status.hdw.t1_ctrl = data;
            break;

        case 0x2F: /* Timer 2 Control */
            mod_status.hdw.t2_ctrl = data;
            break;

        case 0x30: /* LCD menu address register (write only) */
        case 0x31:
        case 0x32:
        case 0x33:
        case 0x34:
            mod_status.hdw.lcd_menu_addr &= ~addr_mask[ rel_address - 0x30 ];
            mod_status.hdw.lcd_menu_addr |= ( ( int )data << ( ( rel_address - 0x30 ) * 4 ) );
            break;

        case 0x37: /* Timer 1 value */
            mod_status.hdw.t1_val = data;
            break;

        case 0x38: /* Timer 2 value */
        case 0x39:
        case 0x3A:
        case 0x3B:
        case 0x3C:
        case 0x3D:
        case 0x3E:
        case 0x3F:
            mod_status.hdw.t2_val &= ~int32_mask[ rel_address - 0x38 ];
            mod_status.hdw.t2_val |= ( ( int32 )data << ( ( rel_address - 0x38 ) * 4 ) );
            break;

        default:
            ChfGenerate( MOD_CHF_MODULE_ID, __FILE__, __LINE__, MOD_W_HDW_WRITE, CHF_WARNING, rel_address, ( int )data );
            ChfSignal( MOD_CHF_MODULE_ID );
    }

    /* Save copy into hdw register array */
    mod_status.hdw.hdw[ rel_address ] = data;
}
