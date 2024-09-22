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

.identifier   : $Id: romram.c,v 4.1 2000/12/11 09:54:19 cibrario Rel $
.context      : SATURN, Saturn CPU / HP48 emulator
.title	      : $RCSfile: romram.c,v $
.kind	      : C source
.author	      : Ivan Cibrario B.
.site	      : CSTV-CNR
.creation     :	23-Jan-1998
.keywords     : *
.description  :
  This module emulates the Internal Rom/Ram peripheral modules of the HP48.
  References:

    SASM.DOC by HP  (HORN disk 4)
    Guide to the Saturn Processor Rev. 0.00f by Matthew Mastracci
    entries.srt by Mika Heiskanen  (mheiskan@vipunen.hut.fi)
    x48 source code by Eddie C. Dost  (ecd@dressler.de)

.include      : config.h, machdep.h, cpu.h, modules.h

.notes	      :
  $Log: romram.c,v $
  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.10  2000/10/24 16:14:53  cibrario
  Added/Replaced GPL header

  Revision 3.2  2000/09/22 14:10:25  cibrario
  Implemented preliminary support of HP49 hw architecture:
  - Revised to handle the split of struct ModStatus in two

 * Revision 3.1  2000/09/20  13:57:24  cibrario
 * Minor updates and fixes to avoid gcc compiler warnings on Solaris
 * when -ansi -pedantic -Wall options are selected.
 *
 * Revision 2.4  2000/09/12  15:25:16  cibrario
 * Implemented emulation of Port 1 and 2; write protection and
 * HST/MP interrupt generation has been implemented as well.
 *
 * Revision 1.1  1998/02/17  11:49:40  cibrario
 * Initial revision
 *

.- */

#ifndef lint
static char rcs_id[] = "$Id: romram.c,v 4.1 2000/12/11 09:54:19 cibrario Rel $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <unistd.h> /* access() */
#include <errno.h>

#include "config.h"
#include "machdep.h"
#include "cpu.h"
#include "modules.h"
#include "disk_io.h"
#include "debug.h"

#define CHF_MODULE_ID MOD_CHF_MODULE_ID
#include <Chf.h>

/* 3.2: The rom/ram storage areas are now dynamically allocated in
   a private struct ModStatus_48. The dynamic allocation is performed during
   Rom initialization, and the following macro allows us to reuse the
   existing code with minimal updates.
*/
static struct ModStatus_48* mod_status_48;

#define mod_status_hdw mod_status.hdw
#define mod_status_rom mod_status_48->rom
#define mod_status_ram mod_status_48->ram
#define mod_status_port_1 mod_status_48->port_1
#define mod_status_port_2 mod_status_48->port_2

/*---------------------------------------------------------------------------
        Rom module
  ---------------------------------------------------------------------------*/

/* .+

.title	      : RomInit
.kind	      : C function
.creation     : 23-Jan-1998
.description  :
  This function allocates the dynamically-allocated portion of the
  module status structure, and initializes the Rom module.

.call	      :
                RomInit();
.input	      :
                void
.output	      :
                void
.status_codes :
                MOD_I_CALLED
                MOD_F_ROM_INIT
                MOD_F_MOD_STATUS_ALLOC
.notes	      :
  1.1, 23-Jan-1998, creation

.- */
void RomInit( void )
{
    debug1( DEBUG_C_TRACE, MOD_I_CALLED, "RomInit" );

    if ( ( mod_status_48 = ( struct ModStatus_48* )malloc( sizeof( struct ModStatus_48 ) ) ) == ( struct ModStatus_48* )NULL ) {
        ChfErrnoCondition;
        ChfCondition MOD_F_MOD_STATUS_ALLOC, CHF_FATAL, sizeof( struct ModStatus_48 ) ChfEnd;
        ChfSignal();
    }

    if ( ReadNibblesFromFile( args.rom_file_name, N_ROM_SIZE, mod_status_rom ) ) {
        ChfCondition MOD_F_ROM_INIT, CHF_FATAL ChfEnd;
        ChfSignal();
    }
}

/* .+

.title	      : RomSave
.kind	      : C function
.creation     : 11-Feb-1998
.description  :
  This function saves the status of the Rom module; actually it does
  nothing.

.call	      :
                RomSave();
.input	      :
                void
.output	      :
                void
.status_codes :
                MOD_I_CALLED
.notes	      :
  1.1, 11-Feb-1998, creation

.- */
void RomSave( void ) { debug1( DEBUG_C_TRACE, MOD_I_CALLED, "RomSave" ); }

/* .+

.title	      : RomRead
.kind	      : C function
.creation     : 26-Jan-1998
.description  :
  This function reads a nibble from the internal ROM address 'rel_address'
  and returns it.

.call	      :
                d = RomRead(rel_address);
.input	      :
                Address rel_address, memory address
.output	      :
                Nibble *d, datum read from memory
.status_codes :
                MOD_I_CALLED
.notes	      :
  1.1, 26-Jan-1998, creation

.- */
Nibble RomRead( Address rel_address )
{
    debug1( DEBUG_C_TRACE, MOD_I_CALLED, "RomRead" );

    return mod_status_rom[ rel_address ];
}

/* .+

.title	      : RomWrite
.kind	      : C function
.creation     : 26-Jan-1998
.description  :
  This function is called when the CPU attempt to write into an internal
  ROM location. It signals an error condition and does nothing.

.call	      :
                RomWrite(rel_address, datum);
.input	      :
                Address rel_address, memory address
                Nibble datum, datum to be written into memory
.output	      :
                void
.status_codes :
                MOD_I_CALLED
                MOD_E_ROM_WRITE
.notes	      :
  1.1, 26-Jan-1998, creation

.- */
void RomWrite( Address rel_address, Nibble datum )
{
    debug1( DEBUG_C_TRACE, MOD_I_CALLED, "RomWrite" );

    ChfCondition MOD_E_ROM_WRITE, CHF_ERROR, rel_address, datum ChfEnd;
    ChfSignal();
}

/*---------------------------------------------------------------------------
        Main Ram module
  ---------------------------------------------------------------------------*/

/* .+

.title	      : RamInit
.kind	      : C function
.creation     : 23-Jan-1998
.description  :
  This function initializes the Ram module.

.call	      :
                RamInit();
.input	      :
                void
.output	      :
                void
.status_codes :
                MOD_I_CALLED
                MOD_W_RAM_INIT
.notes	      :
  1.1, 23-Jan-1998, creation

.- */
void RamInit( void )
{
    debug1( DEBUG_C_TRACE, MOD_I_CALLED, "RamInit" );

    if ( ReadNibblesFromFile( args.ram_file_name, N_RAM_SIZE, mod_status_ram ) ) {
        ChfCondition MOD_W_RAM_INIT, CHF_WARNING ChfEnd;
        ChfSignal();

        ( void )memset( mod_status_ram, 0, sizeof( mod_status_ram ) );
    }
}

/* .+

.title	      : RamSave
.kind	      : C function
.creation     : 11-Feb-1998
.description  :
  This function saves the status of the Ram module to disk.

.call	      :
                RamSave();
.input	      :
                void
.output	      :
                void
.status_codes :
                MOD_I_CALLED
                MOD_E_RAM_SAVE
.notes	      :
  1.1, 11-Feb-1998, creation
  2.4, 12-Sep-2000, update
    - upon failure, added push of ChfErrnoCondition to condition stack.

.- */
void RamSave( void )
{
    debug1( DEBUG_C_TRACE, MOD_I_CALLED, "RamSave" );

    if ( WriteNibblesToFile( mod_status_ram, N_RAM_SIZE, args.ram_file_name ) ) {
        ChfErrnoCondition;
        ChfCondition MOD_E_RAM_SAVE, CHF_ERROR ChfEnd;
        ChfSignal();
    }
}

/* .+

.title	      : RamRead
.kind	      : C function
.creation     : 26-Jan-1998
.description  :
  This function reads a nibble from the internal RAM address 'rel_address'
  and returns it.

.call	      :
                d = RamRead(rel_address);
.input	      :
                Address rel_address, memory address
.output	      :
                Nibble *d, datum read from memory
.status_codes :
                MOD_I_CALLED
.notes	      :
  1.1, 26-Jan-1998, creation

.- */
Nibble RamRead( Address rel_address )
{
    debug1( DEBUG_C_TRACE, MOD_I_CALLED, "RamRead" );

    return mod_status_ram[ rel_address ];
}

/* .+

.title	      : RamWrite
.kind	      : C function
.creation     : 26-Jan-1998
.description  :
  This function writes the nibble 'datum' into the address 'rel_address'
  of the internal RAM.

.call	      :
                RamWrite(rel_address, datum);
.input	      :
                Address rel_address, memory address
                Nibble datum, datum to be written into memory
.output	      :
                void
.status_codes :
                MOD_I_CALLED
.notes	      :
  1.1, 26-Jan-1998, creation

.- */
void RamWrite( Address rel_address, Nibble datum )
{
    debug1( DEBUG_C_TRACE, MOD_I_CALLED, "RamWrite" );

    mod_status_ram[ rel_address ] = datum;
}

/*---------------------------------------------------------------------------
        Ce1  module
  ---------------------------------------------------------------------------*/

/* .+

.title	      : Ce1Init
.kind	      : C function
.creation     : 23-Jan-1998
.description  :
  This function initializes the Ce1 module, corresponding to the
  Back Switcher.

.call	      :
                Ce1Init();
.input	      :
                void
.output	      :
                void
.status_codes :
                MOD_I_CALLED
.notes	      :
  1.1, 23-Jan-1998, creation
  2.4, 11-Sep-2000, implemented

.- */
void Ce1Init( void )
{
    debug1( DEBUG_C_TRACE, MOD_I_CALLED, "Ce1Init" );

    /* Check if bank-switcher accelerators are valid; if not, initialize
       them to a reasonable value (that is, select Port_2 bank 0).
    */
    if ( !mod_status_hdw.accel_valid ) {
        mod_status_hdw.accel_valid = 1;
        mod_status_hdw.accel.a48.bs_address = ( XAddress )0;
    }
}

/* .+

.title	      : Ce1Save
.kind	      : C function
.creation     : 11-Feb-1998
.description  :
  This function saves the status of the Ce1 module.

.call	      :
                Ce1Save();
.input	      :
                void
.output	      :
                void
.status_codes :
                MOD_I_CALLED
.notes	      :
  1.1, 11-Feb-1998, creation
  2.4, 11-Sep-2000, implemented

.- */
void Ce1Save( void )
{
    debug1( DEBUG_C_TRACE, MOD_I_CALLED, "Ce1Save" );

    /* Nothing to be done herel the bank-switcher accelerators are saved
       by the hdw modules
    */
}

/* .+

.title	      : Ce1Read
.kind	      : C function
.creation     : 23-Jan-1998
.description  :
  This function reads a nibble from the Ce1 module; the address of
  the access cycle is converted into an XAddress and saved in
  mod_status_hdw.accel.a48.bs_address.  It will be used to supply the
  most significant bits of Port_2 addresses when accessing that port.

.call	      :
                d = Ce1Read(rel_address);
.input	      :
                Address rel_address, memory address
.output	      :
                Nibble *d, datum read from memory
.status_codes :
                MOD_I_CALLED
                MOD_I_BS_ADDRESS
.notes	      :
  1.1, 23-Jan-1998, creation
  2.4, 11-Sep-2000, implemented

.- */
Nibble Ce1Read( Address rel_address )
{
    debug1( DEBUG_C_TRACE, MOD_I_CALLED, "Ce1Read" );
    debug1( DEBUG_C_MODULES, MOD_I_BS_ADDRESS, rel_address );

    /* Save the read address into the hdw accelerators.
       bs_address can be directly or-ed with a relative port address to
       obtain a valid index in Port_2
    */
#ifdef N_PORT_2_BANK
    mod_status_hdw.accel.a48.bs_address = ( ( XAddress )( ( rel_address >> 1 ) & 0x1F ) << 18 ) & ( N_PORT_2_SIZE - 1 );
#endif

    return ( Nibble )0x0;
}

/* .+

.title	      : Ce1Write
.kind	      : C function
.creation     : 23-Jan-1998
.description  :
  This function writes a nibble to the Ce1 module; the write attempt
  is ignored and the status code MOD_E_CE1_WRITE is signaled. The
  state of mod_status_hdw.accel.a48.bs_address is *not* changed.

.call	      :
                Ce1Write(rel_address, datum);
.input	      :
                Address rel_address, memory address
                Nibble datum, datum to be written into memory
.output	      :
                void
.status_codes :
                MOD_I_CALLED
                MOD_E_CE1_WRITE
.notes	      :
  1.1, 23-Jan-1998, creation
  2.4, 11-Sep-2000, implemented

.- */
void Ce1Write( Address rel_address, Nibble datum )
{
    debug1( DEBUG_C_TRACE, MOD_I_CALLED, "Ce1Write" );

    ChfCondition MOD_E_CE1_WRITE, CHF_ERROR, rel_address, datum ChfEnd;
    ChfSignal();
}

/*---------------------------------------------------------------------------
        Ce2  module
  ---------------------------------------------------------------------------*/

/* .+

.title	      : Ce2Init
.kind	      : C function
.creation     : 23-Jan-1998
.description  :
  This function initializes the Ce2 module, corresponding to Port 1.

.call	      :
                Ce2Init();
.input	      :
                void
.output	      :
                void
.status_codes :
                MOD_I_CALLED
                MOD_W_PORT_1_INIT
                MOD_I_PORT_1_WP
.notes	      :
  1.1, 23-Jan-1998, creation
  2.4, 11-Sep-2000, implemented

.- */
void Ce2Init( void )
{
    Nibble new_status;

    debug1( DEBUG_C_TRACE, MOD_I_CALLED, "Ce2Init" );

    if ( ReadNibblesFromFile( args.port_1_file_name, N_PORT_1_SIZE, mod_status_port_1 ) ) {
        ChfCondition MOD_W_PORT_1_INIT, CHF_WARNING ChfEnd;
        ChfSignal();

        ( void )memset( mod_status_port_1, 0, sizeof( mod_status_port_1 ) );

        new_status = mod_status_hdw.card_status & ~( CE2_CARD_PRESENT | CE2_CARD_WE );
    } else {
        /* Card present; check write protection */
        new_status = mod_status_hdw.card_status | CE2_CARD_PRESENT;

        if ( access( args.port_1_file_name, W_OK ) == 0 )
            new_status |= CE2_CARD_WE;
        else {
            new_status &= ~CE2_CARD_WE;

            ChfErrnoCondition;
            ChfCondition MOD_I_PORT_1_WP, CHF_INFO ChfEnd;
            ChfSignal();
        }
    }

    if ( new_status != mod_status_hdw.card_status ) {
        /* card_status changed; update, set MP bit in HST and post
           interrupt request.
        */
        mod_status_hdw.card_status = new_status;
        cpu_status.HST |= HST_MP_MASK;
        CpuIntRequest( INT_REQUEST_IRQ );
    }
}

/* .+

.title	      : Ce2Save
.kind	      : C function
.creation     : 11-Feb-1998
.description  :
  This function saves the status of the Ce2 module, if it is
  not write-protected.

.call	      :
                Ce2Save();
.input	      :
                void
.output	      :
                void
.status_codes :
                MOD_I_CALLED
                MOD_E_PORT_1_SAVE
.notes	      :
  1.1, 11-Feb-1998, creation
  2.4, 11-Sep-2000, implemented

.- */
void Ce2Save( void )
{
    debug1( DEBUG_C_TRACE, MOD_I_CALLED, "Ce2Save" );

    /* Attempt to save only if port is write-enabled */
    if ( ( mod_status_hdw.card_status & CE2_CARD_WE ) && WriteNibblesToFile( mod_status_port_1, N_PORT_1_SIZE, args.port_1_file_name ) ) {
        ChfErrnoCondition;
        ChfCondition MOD_E_PORT_1_SAVE, CHF_ERROR ChfEnd;
        ChfSignal();
    }
}

/* .+

.title	      : Ce2Read
.kind	      : C function
.creation     : 23-Jan-1998
.description  :
  This function reads a nibble from the Ce2 module.

.call	      :
                d = Ce2Read(rel_address)
.input	      :
                Address rel_address, memory address
.output	      :
                Nibble *d, datum read from memory
.status_codes :
                MOD_I_CALLED
.notes	      :
  1.1, 23-Jan-1998, creation
  2.4, 11-Sep-2000, implemented

.- */
Nibble Ce2Read( Address rel_address )
{
    debug1( DEBUG_C_TRACE, MOD_I_CALLED, "Ce2Read" );

    return mod_status_port_1[ rel_address ];
}

/* .+

.title	      : Ce2Write
.kind	      : C function
.creation     : 23-Jan-1998
.description  :
  This function writes a nibble to the Ce2 module.

.call	      :
                Ce2Write(rel_address, datum);
.input	      :
                Address rel_address, memory address
                Nibble datum, datum to be written into memory
.output	      :
                void
.status_codes :
                MOD_I_CALLED
.notes	      :
  1.1, 23-Jan-1998, creation
  2.4, 11-Sep-2000, implemented

.- */
void Ce2Write( Address rel_address, Nibble datum )
{
    debug1( DEBUG_C_TRACE, MOD_I_CALLED, "Ce2Write" );

    mod_status_port_1[ rel_address ] = datum;
}

/*---------------------------------------------------------------------------
        NCe3  module
  ---------------------------------------------------------------------------*/

/* .+

.title	      : NCe3Init
.kind	      : C function
.creation     : 23-Jan-1998
.description  :
  This function initializes the NCe3 module, corresponding to the
  (bank switched) port 2.

.call	      :
                NCe3Init();
.input	      :
                void
.output	      :
                void
.status_codes :
                MOD_I_CALLED
                MOD_W_PORT_2_INIT
                MOD_I_PORT_2_WP
.notes	      :
  1.1, 23-Jan-1998, creation
  2.4, 11-Sep-2000, implemented

.- */
void NCe3Init( void )
{
    Nibble new_status;

    debug1( DEBUG_C_TRACE, MOD_I_CALLED, "NCe3Init" );

#ifdef N_PORT_2_BANK
    if ( ReadNibblesFromFile( args.port_2_file_name, N_PORT_2_SIZE, mod_status_port_2 ) ) {
        ChfCondition MOD_W_PORT_2_INIT, CHF_WARNING ChfEnd;
        ChfSignal();

        ( void )memset( mod_status_port_2, 0, sizeof( mod_status_port_2 ) );

        new_status = mod_status_hdw.card_status & ~( NCE3_CARD_PRESENT | NCE3_CARD_WE );
    } else {
        /* Card present; check write protection */
        new_status = mod_status_hdw.card_status | NCE3_CARD_PRESENT;

        if ( access( args.port_2_file_name, W_OK ) == 0 )
            new_status |= NCE3_CARD_WE;
        else {
            new_status &= ~NCE3_CARD_WE;

            ChfErrnoCondition;
            ChfCondition MOD_I_PORT_2_WP, CHF_INFO ChfEnd;
            ChfSignal();
        }
    }

#else
    /* If N_PORT_2_BANK is undefined, Port 2 is not emulated */
    new_status = mod_status_hdw.card_status & ~( NCE3_CARD_PRESENT | NCE3_CARD_WE );

#endif

    if ( new_status != mod_status_hdw.card_status ) {
        /* card_status changed; update, set MP bit in HST and post
           interrupt request.
        */
        mod_status_hdw.card_status = new_status;
        cpu_status.HST |= HST_MP_MASK;
        CpuIntRequest( INT_REQUEST_IRQ );
    }
}

/* .+

.title	      : NCe3Save
.kind	      : C function
.creation     : 11-Feb-1998
.description  :
  This function saves the status of the NCe3 module.

.call	      :
                NCe3Save();
.input	      :
                void
.output	      :
                void
.status_codes :
                MOD_I_CALLED
                MOD_E_PORT_2_SAVE
.notes	      :
  1.1, 11-Feb-1998, creation
  2.4, 11-Sep-2000, implemented

.- */
void NCe3Save( void )
{
    debug1( DEBUG_C_TRACE, MOD_I_CALLED, "NCe3Save" );

#ifdef N_PORT_2_BANK
    /* Attempt to save only if port is write-enabled */
    if ( ( mod_status_hdw.card_status & NCE3_CARD_WE ) && WriteNibblesToFile( mod_status_port_2, N_PORT_2_SIZE, args.port_2_file_name ) ) {
        ChfErrnoCondition;
        ChfCondition MOD_E_PORT_2_SAVE, CHF_ERROR ChfEnd;
        ChfSignal();
    }
#endif
}

/* .+

.title	      : NCe3Read
.kind	      : C function
.creation     : 23-Jan-1998
.description  :
  This function reads a nibble from the NCe3 module.

.call	      :
                d = NCe3Read(rel_address)
.input	      :
                Address rel_address, memory address
.output	      :
                Nibble *d, datum read from memory
.status_codes :
                MOD_I_CALLED
                MOD_E_NCE3_READ

.notes	      :
  1.1, 23-Jan-1998, creation
  2.4, 11-Sep-2000, implemented

.- */
Nibble NCe3Read( Address rel_address )
{
    debug1( DEBUG_C_TRACE, MOD_I_CALLED, "NCe3Read" );

#ifdef N_PORT_2_BANK
    return mod_status_port_2[ rel_address | mod_status_hdw.accel.a48.bs_address ];

#else
    ChfCondition MOD_E_NCE3_READ, CHF_ERROR, rel_address ChfEnd;
    ChfSignal();
    return ( Nibble )0;

#endif
}

/* .+

.title	      : NCe3Write
.kind	      : C function
.creation     : 23-Jan-1998
.description  :
  This function writes a nibble to the NCe3 module;
  it is not currently implemented.

.call	      :
                NCe3Write(rel_address, datum);
.input	      :
                Address rel_address, memory address
                Nibble datum, datum to be written into memory
.output	      :
                void
.status_codes :
                MOD_I_CALLED
                MOD_E_NCE3_WRITE

.notes	      :
  1.1, 23-Jan-1998, creation
  2.4, 11-Sep-2000, implemented

.- */
void NCe3Write( Address rel_address, Nibble datum )
{
    debug1( DEBUG_C_TRACE, MOD_I_CALLED, "NCe3Write" );

#ifdef N_PORT_2_BANK
    mod_status_port_2[ rel_address | mod_status_hdw.accel.a48.bs_address ] = datum;

#else
    ChfCondition MOD_E_NCE3_WRITE, CHF_ERROR, rel_address, datum ChfEnd;
    ChfSignal();

#endif
}
