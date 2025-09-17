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

.creation     :	21-Sep-2000

.description  :
  This module contains the module description tables for all HPxx
  hardware configurations currently supported.  Moreover, it implements
  the function ModSelectDescription(hw), to select and register a
  module description table depending on an hardware configuration
  selection string (hw).  References:

    Guide to the Saturn Processor Rev. 1.0b by Matthew Mastracci
    HP49 Memory Explained, USENET post, by Steve Sousa.

.notes        :
  $Log: hw_config.c,v $
  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.10  2000/10/24 16:14:43  cibrario
  Added/Replaced GPL header

  Revision 3.3  2000/09/26 15:18:16  cibrario
  Revised to implement Flash ROM write access:
  - updated description of ModDescription table components
  - the HP49 NCE3 controller now has MOD_MAP_FLAGS_ABS set in .map_flags

  Revision 3.2  2000/09/22  14:40:18  cibrario
  *** empty log message ***
.- */
#include "../libChf/src/Chf.h"

#include "config.h"
#include "modules.h"
#include "chf_wrapper.h"
#include "romram48.h"
#include "romram49.h"

/*---------------------------------------------------------------------------
        Module description tables
  ---------------------------------------------------------------------------*/

static const ModDescription hw48_description = {
    {"ROM              (ROM)",  0x00, 0, RomInit48,  RomSave48,  RomRead48,  RomWrite48,  MOD_CONFIGURED,      0x00000, 0xFFFFF, 0},
    {"Hardware Regs.   (HDW)",  0x19, 5, HdwInit,    HdwSave,    HdwRead,    HdwWrite,    MOD_SIZE_CONFIGURED, 0x00000, 0x00040, 0},
    {"Internal RAM     (RAM)",  0x03, 4, RamInit48,  RamSave48,  RamRead48,  RamWrite48,  MOD_UNCONFIGURED,    0,       0,       0},
    {"Bank Select      (CE1)",  0x05, 2, Ce1Init48,  Ce1Save48,  Ce1Read48,  Ce1Write48,  MOD_UNCONFIGURED,    0,       0,       0},
    {"Port 1 Control   (CE2)",  0x07, 3, Ce2Init48,  Ce2Save48,  Ce2Read48,  Ce2Write48,  MOD_UNCONFIGURED,    0,       0,       0},
    {"Port 2 Control   (NCE3)", 0x01, 1, NCe3Init48, NCe3Save48, NCe3Read48, NCe3Write48, MOD_UNCONFIGURED,    0,       0,       0}
};
static const ModDescription hw49_description = {
    {"ROM              (ROM)",  0x00, 0, RomInit49,  RomSave49,  RomRead49,  RomWrite49,  MOD_CONFIGURED,      0x00000, 0xFFFFF, 0                },
    {"Hardware Regs.   (HDW)",  0x19, 5, HdwInit,    HdwSave,    HdwRead,    HdwWrite,    MOD_SIZE_CONFIGURED, 0x00000, 0x00040, 0                },
    {"IRAM             (RAM)",  0x03, 4, RamInit49,  RamSave49,  RamRead49,  RamWrite49,  MOD_UNCONFIGURED,    0,       0,       0                },
    {"Bank Select      (CE1)",  0x05, 2, Ce1Init49,  Ce1Save49,  Ce1Read49,  Ce1Write49,  MOD_UNCONFIGURED,    0,       0,       0                },
    {"ERAM Bank 0      (CE2)",  0x07, 3, Ce2Init49,  Ce2Save49,  Ce2Read49,  Ce2Write49,  MOD_UNCONFIGURED,    0,       0,       0                },
    {"ERAM Bank 1      (NCE3)", 0x01, 1, NCe3Init49, NCe3Save49, NCe3Read49, NCe3Write49, MOD_UNCONFIGURED,    0,       0,       MOD_MAP_FLAGS_ABS}
};

/*---------------------------------------------------------------------------
                                Public functions
  ---------------------------------------------------------------------------*/

/* .+

.creation     : 21-Sep-2000
.description  :
  This function selects and registers (invoking ModRegisterDescription())
  a module description table depending on the hardware configuration
  string passed as argument.

.call         :
                ModSelectDescription(hw)
.input        :
                const char *hw, hardware configuration string
.output       :
                void
.status_codes :
                MOD_I_CALLED
                MOD_E_NO_MATCH
.notes        :
  1.1, 28-Jan-1998, creation

.- */
void ModSelectDescription( int model )
{
    switch ( model ) {
        case MODEL_48SX:
        case MODEL_48GX:
            ModRegisterDescription( hw48_description );
            break;
        case MODEL_40G:
        case MODEL_49G:
        case MODEL_50G:
            ModRegisterDescription( hw49_description );
            break;
        default:
            ERROR( MOD_CHF_MODULE_ID, MOD_E_NO_MATCH, "Unknown model" )
            exit( EXIT_FAILURE );
    }
}
