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

.identifier   : $Id: hw_config.c,v 4.1 2000/12/11 09:54:19 cibrario Rel $
.context      : SATURN, Saturn CPU / HPxx emulator
.title	      : $RCSfile: hw_config.c,v $
.kind	      : C source
.author	      : Ivan Cibrario B.
.site	      : CSTV-CNR
.creation     :	21-Sep-2000
.keywords     : *
.description  :
  This module contains the module description tables for all HPxx
  hardware configurations currently supported.  Moreover, it implements
  the function ModSelectDescription(hw), to select and register a
  module description table depending on an hardware configuration
  selection string (hw).  References:

    Guide to the Saturn Processor Rev. 1.0b by Matthew Mastracci
    HP49 Memory Explained, USENET post, by Steve Sousa.

.include      : config.h machdep.h, cpu.h, modules.h

.notes	      :
  $Log: hw_config.c,v $
  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.10  2000/10/24 16:14:43  cibrario
  Added/Replaced GPL header

  Revision 3.3  2000/09/26 15:18:16  cibrario
  Revised to implement Flash ROM write access:
  - updated description of ModDescription table components
  - the HP49 NCE3 controller now has MOD_MAP_FLAGS_ABS set in .map_flags

 * Revision 3.2  2000/09/22  14:40:18  cibrario
 * *** empty log message ***
 *

.- */

#ifndef lint
static char rcs_id[] = "$Id: hw_config.c,v 4.1 2000/12/11 09:54:19 cibrario Rel $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

#include "config.h"
#include "machdep.h"
#include "cpu.h"
#include "modules.h"
#include "debug.h"

#define CHF_MODULE_ID MOD_CHF_MODULE_ID
#include <Chf.h>

/*---------------------------------------------------------------------------
        Module description tables
  ---------------------------------------------------------------------------*/

extern void RomInit( void );
extern void HdwInit( void );
extern void RamInit( void );
extern void Ce1Init( void );
extern void Ce2Init( void );
extern void NCe3Init( void );

extern void RomSave( void );
extern void HdwSave( void );
extern void RamSave( void );
extern void Ce1Save( void );
extern void Ce2Save( void );
extern void NCe3Save( void );

extern Nibble RomRead( Address );
extern Nibble HdwRead( Address );
extern Nibble RamRead( Address );
extern Nibble Ce1Read( Address );
extern Nibble Ce2Read( Address );
extern Nibble NCe3Read( Address );

extern void RomWrite( Address, Nibble );
extern void HdwWrite( Address, Nibble );
extern void RamWrite( Address, Nibble );
extern void Ce1Write( Address, Nibble );
extern void Ce2Write( Address, Nibble );
extern void NCe3Write( Address, Nibble );

extern void RomInit49( void );
extern void HdwInit49( void );
extern void RamInit49( void );
extern void Ce1Init49( void );
extern void Ce2Init49( void );
extern void NCe3Init49( void );

extern void RomSave49( void );
extern void HdwSave49( void );
extern void RamSave49( void );
extern void Ce1Save49( void );
extern void Ce2Save49( void );
extern void NCe3Save49( void );

extern Nibble RomRead49( Address );
extern Nibble HdwRead49( Address );
extern Nibble RamRead49( Address );
extern Nibble Ce1Read49( Address );
extern Nibble Ce2Read49( Address );
extern Nibble NCe3Read49( Address );

extern void RomWrite49( Address, Nibble );
extern void HdwWrite49( Address, Nibble );
extern void RamWrite49( Address, Nibble );
extern void Ce1Write49( Address, Nibble );
extern void Ce2Write49( Address, Nibble );
extern void NCe3Write49( Address, Nibble );

static const struct {
    const char* hw;
    ModDescription description;
} table[] = {
    {"hp48",
     { {
            "ROM              (ROM)",
            0x00,
            0,
            RomInit,
            RomSave,
            RomRead,
            RomWrite,
            MOD_CONFIGURED,
            0x00000,
            0xFFFFF,
        },
        {
            "Hardware Regs.   (HDW)",
            0x19,
            5,
            HdwInit,
            HdwSave,
            HdwRead,
            HdwWrite,
            MOD_SIZE_CONFIGURED,
            0x00000,
            0x00040,
        },
        {
            "Internal RAM     (RAM)",
            0x03,
            4,
            RamInit,
            RamSave,
            RamRead,
            RamWrite,
            MOD_UNCONFIGURED,
            0,
            0,
        },
        {
            "Bank Select      (CE1)",
            0x05,
            2,
            Ce1Init,
            Ce1Save,
            Ce1Read,
            Ce1Write,
            MOD_UNCONFIGURED,
            0,
            0,
        },
        {
            "Port 1 Control   (CE2)",
            0x07,
            3,
            Ce2Init,
            Ce2Save,
            Ce2Read,
            Ce2Write,
            MOD_UNCONFIGURED,
            0,
            0,
        },
        {
            "Port 2 Control   (NCE3)",
            0x01,
            1,
            NCe3Init,
            NCe3Save,
            NCe3Read,
            NCe3Write,
            MOD_UNCONFIGURED,
            0,
            0,
        } }                    },

    {"hp49",
     { {
            "ROM              (ROM)",
            0x00,
            0,
            RomInit49,
            RomSave49,
            RomRead49,
            RomWrite49,
            MOD_CONFIGURED,
            0x00000,
            0xFFFFF,
        },
        {
            "Hardware Regs.   (HDW)",
            0x19,
            5,
            HdwInit,
            HdwSave,
            HdwRead,
            HdwWrite,
            MOD_SIZE_CONFIGURED,
            0x00000,
            0x00040,
        },
        {
            "IRAM             (RAM)",
            0x03,
            4,
            RamInit49,
            RamSave49,
            RamRead49,
            RamWrite49,
            MOD_UNCONFIGURED,
            0,
            0,
        },
        {
            "Bank Select      (CE1)",
            0x05,
            2,
            Ce1Init49,
            Ce1Save49,
            Ce1Read49,
            Ce1Write49,
            MOD_UNCONFIGURED,
            0,
            0,
        },
        {
            "ERAM Bank 0      (CE2)",
            0x07,
            3,
            Ce2Init49,
            Ce2Save49,
            Ce2Read49,
            Ce2Write49,
            MOD_UNCONFIGURED,
            0,
            0,
        },
        { "ERAM Bank 1      (NCE3)", 0x01, 1, NCe3Init49, NCe3Save49, NCe3Read49, NCe3Write49, MOD_UNCONFIGURED, 0, 0,
          MOD_MAP_FLAGS_ABS } }}
};

#define N_DESCRIPTIONS ( sizeof( table ) / sizeof( table[ 0 ] ) )

/*---------------------------------------------------------------------------
                                Public functions
  ---------------------------------------------------------------------------*/

/* .+

.title	      : ModSelectDescription
.kind	      : C function
.creation     : 21-Sep-2000
.description  :
  This function selects and registers (invoking ModRegisterDescription())
  a module description table depending on the hardware configuration
  string passed as argument.

.call	      :
                ModSelectDescription(hw)
.input	      :
                const char *hw, hardware configuration string
.output	      :
                void
.status_codes :
                MOD_I_CALLED
                MOD_E_NO_MATCH
.notes	      :
  1.1, 28-Jan-1998, creation

.- */
void ModSelectDescription( const char* hw )
{
    int i;

    debug1( DEBUG_C_TRACE, MOD_I_CALLED, "ModSelectDescription" );

    for ( i = 0; i < N_DESCRIPTIONS && strcmp( hw, table[ i ].hw ); i++ )
        ;

    if ( i == N_DESCRIPTIONS ) {
        ChfCondition MOD_E_NO_MATCH, CHF_ERROR, hw ChfEnd;
        ChfSignal();
    } else
        ModRegisterDescription( table[ i ].description );
}
