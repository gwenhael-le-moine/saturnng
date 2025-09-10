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
  This module emulates the Internal Flash Rom/Ram peripheral modules of
  the HP49.

  Known deficiencies of the current Flash ROM emulation:

  - for efficiency reasons, the Flash ROM state machine is bypassed
    when the Flash is read through the ROM controller.

  - see flash49.h for additional, more specific information about
    Flash ROM emulation issues.

  References:

    Guide to the Saturn Processor Rev. 1.0b by Matthew Mastracci
    HP49 Memory Explained, USENET post, by Steve Sousa.
    Emu48 Service Pack 20, by Christoph Giesselink.
    28F160S5/28F320S5 Data Sheet, by Intel Corp.

.notes        :
  $Log: romram49.c,v $
  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.10  2000/10/24 16:14:55  cibrario
  Added/Replaced GPL header

  Revision 3.4  2000/09/27 10:05:55  cibrario
  Bug fix: MP bit of HST no longer set in Ce2Init49() and NCe3Init49(),
  to avoid spurious warmstarts of the HP39/40 firmware.

  Revision 3.3  2000/09/26  14:56:43  cibrario
  Revised to implement Flash ROM write access:
  - mod_status_49 is no longer static; flash49.c needs access to the
    Flash ROM array.
  - implemented RomSave49(), RomWrite49().
  - NCe3 controller now accesses the Flash ROM when the LCR_LED bit is set;
    address translation is done here, the actual access is revectored to
    FlashRead49() and FlashWrite49().  Notice that the NCe3 controller
    must be registered with the MOD_MAP_FLAGS_ABS bit set in its map_flags.

  Revision 3.2  2000/09/22  14:48:13  cibrario
  *** empty log message ***
.- */

#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* access() */
#include "../libChf/src/Chf.h"

#include "config.h"
#include "cpu.h"
#include "modules.h"
#include "flash49.h"
#include "disk_io.h"
#include "debug.h"

#define FLASH_VIEW_SELECTOR 0x40000
#define FLASH_BANK_MASK 0x3FFFF

#define CE2_RAM_OFFSET 0x80000
#define NCE3_RAM_OFFSET 0xC0000
#define NCE3_RAM_MASK 0x3FFFF

#define HDW_LCR_OFFSET 0x1C
#define LCR_LED 0x8

/* 3.3: This is no longer static, because flash49.c needs access to
   the Flash ROM array... yes, I know this is not particularly nice,
*/
struct ModStatus_49* mod_status_49;

/*---------------------------------------------------------------------------
        Rom module
  ---------------------------------------------------------------------------*/

/* .+

.creation     : 21-Sep-2000
.description  :
  This function allocates the dynamically-allocated portion of the
  module status structure, and initializes the Flash Rom module.

.call         :
                RomInit49();
.input        :
                void
.output       :
                void
.status_codes :
                MOD_I_CALLED
                MOD_F_ROM_INIT
                MOD_F_MOD_STATUS_ALLOC
.notes        :
  3.2, 21-Sep-2000, creation

.- */
void RomInit49( void )
{
    mod_status_49 = ( struct ModStatus_49* )malloc( sizeof( struct ModStatus_49 ) );
    if ( mod_status_49 == ( struct ModStatus_49* )NULL ) {
        ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
        ChfGenerate( MOD_CHF_MODULE_ID, __FILE__, __LINE__, MOD_F_MOD_STATUS_ALLOC, CHF_FATAL, sizeof( struct ModStatus_49 ) );
        ChfSignal( MOD_CHF_MODULE_ID );
    }

    bool err = ReadNibblesFromFile( config.rom_path, N_FLASH_SIZE_49, mod_status_49->flash );
    if ( err ) {
        ChfGenerate( MOD_CHF_MODULE_ID, __FILE__, __LINE__, MOD_F_ROM_INIT, CHF_FATAL );
        ChfSignal( MOD_CHF_MODULE_ID );
    }
}

/* .+

.creation     : 21-Sep-2000
.description  :
  This function saves the status of the Flash Rom.

.call         :
                RomSave49();
.input        :
                void
.output       :
                void
.status_codes :
                MOD_I_CALLED
                MOD_E_ROM_SAVE
.notes        :
  3.2, 21-Sep-2000, creation
  3.3, 25-Sep-2000, implemented

.- */
void RomSave49( void )
{
    bool err = WriteNibblesToFile( mod_status_49->flash, N_FLASH_SIZE_49, config.rom_path );
    if ( err ) {
        ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
        ChfGenerate( MOD_CHF_MODULE_ID, __FILE__, __LINE__, MOD_E_ROM_SAVE, CHF_ERROR );
        ChfSignal( MOD_CHF_MODULE_ID );
    }
}

/* .+

.creation     : 21-Sep-2000
.description  :
  This function reads a nibble from the internal Flash Rom address
  'rel_address' and returns it.

.call         :
                d = RomRead49(rel_address);
.input        :
                Address rel_address, memory address
.output       :
                Nibble *d, datum read from memory
.status_codes :
                MOD_I_CALLED
.notes        :
  3.2, 21-Sep-2000, creation

.- */
Nibble RomRead49( Address rel_address )
{
    register XAddress view = mod_status.hdw.accel.a49.view[ ( rel_address & FLASH_VIEW_SELECTOR ) != 0 ];

    return mod_status_49->flash[ view | ( rel_address & FLASH_BANK_MASK ) ];
}

/* .+

.creation     : 21-Sep-2000
.description  :
  This function writes the nibble 'datum' into the address 'rel_address'
  of the internal RAM.

  Flash ROM cannot be written using the ROM controller; however,
  the current (1.19-4) HP49 firmware apparently executes ROM
  write cycles through this controller when the ON key is pressed.
  Those cycles are silently ignored.

.call         :
                RomWrite49(rel_address, datum);
.input        :
                Address rel_address, memory address
                Nibble datum, datum to be written into memory
.output       :
                void
.status_codes :
                MOD_I_CALLED
.notes        :
  3.2, 21-Sep-2000, creation
  3.3, 26-Sep-2000, implemented

.- */
void RomWrite49( Address rel_address, Nibble datum )
{
    /* Ignore write cycles through ROM controller; HP49 ROM 1.19-4
       can do this when to ON key is pressed.
    */
}

/*---------------------------------------------------------------------------
        Main Ram module
  ---------------------------------------------------------------------------*/

/* .+

.creation     : 21-Sep-2000
.description  :
  This function initializes the Ram module.

.call         :
                RamInit49();
.input        :
                void
.output       :
                void
.status_codes :
                MOD_I_CALLED
                MOD_W_RAM_INIT
.notes        :
  3.2, 21-Sep-2000, creation

.- */
void RamInit49( void )
{
    bool err = ReadNibblesFromFile( config.ram_path, N_RAM_SIZE_49, mod_status_49->ram );
    if ( err ) {
        ChfGenerate( MOD_CHF_MODULE_ID, __FILE__, __LINE__, MOD_W_RAM_INIT, CHF_WARNING );
        ChfSignal( MOD_CHF_MODULE_ID );

        ( void )memset( mod_status_49->ram, 0, sizeof( mod_status_49->ram ) );
    }
}

/* .+

.creation     : 21-Sep-2000
.description  :
  This function saves the status of the Ram module to disk.

.call         :
                RamSave49();
.input        :
                void
.output       :
                void
.status_codes :
                MOD_I_CALLED
                MOD_E_RAM_SAVE
.notes        :
  3.2, 21-Sep-2000, creation

.- */
void RamSave49( void )
{
    bool err = WriteNibblesToFile( mod_status_49->ram, N_RAM_SIZE_49, config.ram_path );
    if ( err ) {
        ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
        ChfGenerate( MOD_CHF_MODULE_ID, __FILE__, __LINE__, MOD_E_RAM_SAVE, CHF_ERROR );
        ChfSignal( MOD_CHF_MODULE_ID );
    }
}

/* .+

.creation     : 21-Sep-2000
.description  :
  This function reads a nibble from the internal RAM address 'rel_address'
  and returns it.

.call         :
                d = RamRead49(rel_address);
.input        :
                Address rel_address, memory address
.output       :
                Nibble *d, datum read from memory
.status_codes :
                MOD_I_CALLED
.notes        :
  3.2, 21-Sep-2000, creation

.- */
Nibble RamRead49( Address rel_address ) { return mod_status_49->ram[ rel_address ]; }

/* .+

.creation     : 21-Sep-2000
.description  :
  This function writes the nibble 'datum' into the address 'rel_address'
  of the internal RAM.

.call         :
                RamWrite49(rel_address, datum);
.input        :
                Address rel_address, memory address
                Nibble datum, datum to be written into memory
.output       :
                void
.status_codes :
                MOD_I_CALLED
.notes        :
  3.2, 21-Sep-2000, creation

.- */
void RamWrite49( Address rel_address, Nibble datum ) { mod_status_49->ram[ rel_address ] = datum; }

/*---------------------------------------------------------------------------
        Ce1  module
  ---------------------------------------------------------------------------*/

/* .+

.creation     : 21-Sep-2000
.description  :
  This function initializes the Ce1 module, corresponding to the
  Back Switcher.

.call         :
                Ce1Init49();
.input        :
                void
.output       :
                void
.status_codes :
                MOD_I_CALLED
.notes        :
  3.2, 21-Sep-2000, creation

.- */
void Ce1Init49( void )
{
    /* Check if bank-switcher accelerators are valid; if not, initialize
       them to a reasonable value (that is, select Flash Rom bank 0 for
       both views).
    */
    if ( !mod_status.hdw.accel_valid ) {
        mod_status.hdw.accel_valid = 1;
        mod_status.hdw.accel.a49.view[ 0 ] = mod_status.hdw.accel.a49.view[ 1 ] = ( XAddress )0;
    }
}

/* .+

.creation     : 21-Sep-2000
.description  :
  This function saves the status of the Ce1 module.

.call         :
                Ce1Save49();
.input        :
                void
.output       :
                void
.status_codes :
                MOD_I_CALLED
.notes        :
  3.2, 21-Sep-2000, creation

.- */
void Ce1Save49( void )
{
    /* Nothing to be done here; the bank-switcher accelerators are saved
       by the hdw modules
    */
}

/* This fragment of code is used by both Ce1Read49() and Ce1Write49();
   the macro definition is here to allow us to write the code once and
   use it many times without incurring a function call overhead.
*/
#define Ce1SetViews                                                                                                                        \
    {                                                                                                                                      \
        mod_status.hdw.accel.a49.view[ 0 ] = ( ( XAddress )( ( rel_address >> 5 ) & 0x03 ) << 18 );                                        \
                                                                                                                                           \
        mod_status.hdw.accel.a49.view[ 1 ] = ( ( XAddress )( ( rel_address >> 1 ) & 0x0F ) << 18 );                                        \
    }

/* .+

.creation     : 21-Sep-2000
.description  :
  This function reads a nibble from the Ce1 module; the address of
  the access cycle is converted into a pair of XAddress
  (base addresses of Flash Rom views) and saved into
  mod_status_hdw.accel.a49.view[].  They will be used to supply the
  most significant bits of addresses when accessing Flash Rom.

.call         :
                d = Ce1Read49(rel_address);
.input        :
                Address rel_address, memory address
.output       :
                Nibble *d, datum read from memory
.status_codes :
                MOD_I_CALLED
                MOD_I_BS_ADDRESS
.notes        :
  3.2, 21-Sep-2000, creation

.- */
Nibble Ce1Read49( Address rel_address )
{

    debug1( MOD_CHF_MODULE_ID, DEBUG_C_MODULES, MOD_I_BS_ADDRESS, rel_address );

    /* Save the ROM view base addresses address into the hdw accelerators.
       view[] can be directly or-ed with a relative port address to
       obtain a valid index in Flash Rom.
    */
    Ce1SetViews;

    return ( Nibble )0x0;
}

/* .+

.creation     : 21-Sep-2000
.description  :
  This function reads a nibble from the Ce1 module; the address of
  the access cycle is converted into a pair of XAddress
  (base addresses of Flash Rom views) and saved into
  mod_status_hdw.accel.a49.view[].  They will be used to supply the
  most significant bits of addresses when accessing Flash Rom.

.call         :
                Ce1Write49(rel_address, datum);
.input        :
                Address rel_address, memory address
                Nibble datum, datum to be written into memory; ignored
.output       :
                void
.status_codes :
                MOD_I_CALLED
                MOD_I_BS_ADDRESS
.notes        :
  3.2, 21-Sep-2000, creation

.- */
void Ce1Write49( Address rel_address, Nibble datum )
{

    debug1( MOD_CHF_MODULE_ID, DEBUG_C_MODULES, MOD_I_BS_ADDRESS, rel_address );

    /* Save the ROM view base addresses address into the hdw accelerators.
       view[] can be directly or-ed with a relative port address to
       obtain a valid index in Flash Rom.
    */
    Ce1SetViews;
}

/*---------------------------------------------------------------------------
        Ce2  module
  ---------------------------------------------------------------------------*/

/* .+

.creation     : 21-Sep-2000
.description  :
  This function initializes the Ce2 module, corresponding to
  the first bank of ERAM.

.call         :
                Ce2Init49();
.input        :
                void
.output       :
                void
.status_codes :
                MOD_I_CALLED
.notes        :
  3.2, 21-Sep-2000, creation
  3.4, 27-Sep-2000, update:
    - MP bit in HST no longer set, to avoid spurious warmstarts of the
      HP39/40 firmware.

.- */
void Ce2Init49( void )
{
    /* Set base of ce2 area */
    mod_status_49->ce2 = mod_status_49->ram + CE2_RAM_OFFSET;

    /* CE2 always present and write enabled */
    mod_status.hdw.card_status |= ( CE2_CARD_PRESENT | CE2_CARD_WE );

    /* card_status changed; update, set MP bit in HST and post
       interrupt request.
    */
    /* cpu_status.HST |= HST_MP_MASK; */
    /* CpuIntRequest(INT_REQUEST_IRQ); */
}

/* .+

.creation     : 21-Sep-2000
.description  :
  This function saves the status of the Ce2 module.

.call         :
                Ce2Save49();
.input        :
                void
.output       :
                void
.status_codes :
                MOD_I_CALLED
.notes        :
  3.2, 21-Sep-2000, creation

.- */
void Ce2Save49( void ) { /* Do nothing; the whole RAM is saved by RamSave49() */ }

/* .+

.creation     : 21-Sep-2000
.description  :
  This function reads a nibble from the Ce2 module.

.call         :
                d = Ce2Read49(rel_address)
.input        :
                Address rel_address, memory address
.output       :
                Nibble *d, datum read from memory
.status_codes :
                MOD_I_CALLED
.notes        :
  3.2, 21-Sep-2000, creation

.- */
Nibble Ce2Read49( Address rel_address ) { return mod_status_49->ce2[ rel_address ]; }

/* .+

.creation     : 21-Sep-2000
.description  :
  This function writes a nibble to the Ce2 module.

.call         :
                Ce2Write49(rel_address, datum);
.input        :
                Address rel_address, memory address
                Nibble datum, datum to be written into memory
.output       :
                void
.status_codes :
                MOD_I_CALLED
.notes        :
  3.2, 21-Sep-2000, creation

.- */
void Ce2Write49( Address rel_address, Nibble datum ) { mod_status_49->ce2[ rel_address ] = datum; }

/*---------------------------------------------------------------------------
        NCe3  module
  ---------------------------------------------------------------------------*/

/* .+

.creation     : 21-Sep-2000
.description  :
  This function initializes the Ce2 module, corresponding to
  the first bank of ERAM.

.call         :
                NCe3Init49();
.input        :
                void
.output       :
                void
.status_codes :
                MOD_I_CALLED
.notes        :
  3.2, 21-Sep-2000, creation
  3.4, 27-Sep-2000, update:
    - MP bit in HST no longer set, to avoid spurious warmstarts of the
      HP39/40 firmware.

.- */
void NCe3Init49( void )
{
    /* Set base of nce3 area */
    mod_status_49->nce3 = mod_status_49->ram + NCE3_RAM_OFFSET;

    /* NCE3 always present and write enabled */
    mod_status.hdw.card_status |= ( NCE3_CARD_PRESENT | NCE3_CARD_WE );

#if 0
    /* card_status changed; update, set MP bit in HST and post
       interrupt request.
    */
    cpu_status.HST |= HST_MP_MASK;
    CpuIntRequest(INT_REQUEST_IRQ);
#endif
}

/* .+

.creation     : 21-Sep-2000
.description  :
  This function saves the status of the NCe3 module.

.call         :
                NCe3Save49();
.input        :
                void
.output       :
                void
.status_codes :
                MOD_I_CALLED
.notes        :
  3.2, 21-Sep-2000, creation

.- */
void NCe3Save49( void ) { /* Do nothing; the whole RAM is saved by RamSave49() */ }

/* .+

.creation     : 21-Sep-2000
.description  :
  This function reads a nibble from the NCe3 module.
  If LCR_LED bit is not set, data comes from ERAM Bank 1, if LCR_LED is set,
  data comes from the Flash ROM.

  In both cases, address translation is done here.

  This module must have the MOD_MAP_FLAGS_ABS set in
  its mod_description[].map_flags; this way, rel_address will be
  the ABSOLUTE address of the memory access.

.call         :
                d = NCe3Read49(rel_address)
.input        :
                Address rel_address, memory address (ABSOLUTE)
.output       :
                Nibble *d, datum read from memory
.status_codes :
                MOD_I_CALLED
.notes        :
  3.2, 21-Sep-2000, creation
  3.3, 25-Sep-2000, update
    - added vectors into flash49 for FlashROM read ops

.- */
Nibble NCe3Read49( Address rel_address )
{
    return ( mod_status.hdw.hdw[ HDW_LCR_OFFSET ] & LCR_LED )
               ? FlashRead49( mod_status.hdw.accel.a49.view[ ( rel_address & FLASH_VIEW_SELECTOR ) != 0 ] |
                              ( rel_address & FLASH_BANK_MASK ) )
               : mod_status_49->nce3[ rel_address & NCE3_RAM_MASK ];
}

/* .+

.creation     : 21-Sep-2000
.description  :
  This function writes a nibble to the NCe3 module.
  If LCR_LED bit is not set, data goes to ERAM Bank 1, if LCR_LED is set,
  data goes to the Flash ROM.

  In both cases, address translation is done here.

  This module must have the MOD_MAP_FLAGS_ABS set in
  its mod_description[].map_flags; this way, rel_address will be
  the ABSOLUTE address of the memory access.
.call         :
                NCe3Write49(rel_address, datum);
.input        :
                Address rel_address, memory address (ABSOLUTE)
                Nibble datum, datum to be written into memory
.output       :
                void
.status_codes :
                MOD_I_CALLED
.notes        :
  3.2, 21-Sep-2000, creation
  3.3, 25-Sep-2000, update
    - added vectors into flash49 for FlashROM write ops

.- */
void NCe3Write49( Address rel_address, Nibble datum )
{
    if ( mod_status.hdw.hdw[ HDW_LCR_OFFSET ] & LCR_LED )
        FlashWrite49( mod_status.hdw.accel.a49.view[ ( rel_address & FLASH_VIEW_SELECTOR ) != 0 ] | ( rel_address & FLASH_BANK_MASK ),
                      datum );
    else
        mod_status_49->nce3[ rel_address & NCE3_RAM_MASK ] = datum;
}
