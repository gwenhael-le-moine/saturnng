#ifndef _MODULES_H
#  define _MODULES_H 1

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

.creation     :	28-Jan-1998

.description  :
  This header contains all definitions and declarations related to the
  peripheral modules of the HP48. References:

    SASM.DOC by HP  (HORN disk 4)
    Guide to the Saturn Processor Rev. 0.00f by Matthew Mastracci
    entries.srt by Mika Heiskanen  (mheiskan@vipunen.hut.fi)
    x48 source code by Eddie C. Dost  (ecd@dressler.de)

.notes        :
  $Log: modules.h,v $
  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.10  2000/10/24 16:14:50  cibrario
  Added/Replaced GPL header

  Revision 3.3  2000/09/26 15:10:22  cibrario
  Revised to implement Flash ROM write access:
  - Added .map_flags field to struct ModDescriptionEntry
  - New status code MOD_E_ROM_SAVE

  Revision 3.2  2000/09/22  14:05:32  cibrario
  Implemented preliminary support of HP49 hw architecture:
  - moved ROM/RAM storage areas in ModStatus into private,
    configuration-specific, dynamically allocated data structures.
  - added flash-rom access accelerators for the hp49 hw configuration
  - added new status codes: MOD_E_NO_MATCH, MOD_F_MOD_STATUS_ALLOC,
    MOD_F_NO_DESCRIPTION
  - added prototype of new functions: ModSelectDescription(),
    ModRegisterDescription()

  Revision 3.1  2000/09/20  14:00:40  cibrario
  Minor updates and fixes to avoid gcc compiler warnings on Solaris
  when -ansi -pedantic -Wall options are selected.

  Revision 2.7  2000/09/19  11:12:46  cibrario
  Deeply revised to implement module config/unconfig cache.

  Revision 2.5  2000/09/14  15:20:18  cibrario
  Added serial port buffer registers .serial_rbr and .serial_tbr
  in struct ModHdw; the hdw module uses them to buffer nibbles
  to/from multi-nibble registers.

  Revision 2.4  2000/09/12  15:47:32  cibrario
  The follwing updates were required to implement emulation of Port 1
  and 2, and to prepare the support of additional hw configurations:
  - added an hw configuration-dependent field to struct ModHdw: .accel;
    this field holds information useful to accelerate frequently executed
    operations on a given hw architecture.
    In the current hw configuration, it is used to hold the last
    address extension loaded into the bank switcher's F/F.
    The .accel_valid field indicates whether .accel is valid or not;
    .accel_valid is cleared when the ModHdw structure has not been read
    back correctly from mass storage.
  - added a new member to ModHdw: .card_status;
    it represent the current card status (HDW relative address 0x0F).
  - added storage space in struct ModStatus to accommodate Port 1 and 2.
  - added new status codes: MOD_I_BS_ADDRESS, MOD_I_PORT_1_WP,
    MOD_I_PORT_2_WP, MOD_W_PORT_1_INIT, MOD_W_PORT_2_INIT,
    MOD_E_PORT_1_SAVE, MOD_E_CE1_WRITE, MOD_E_PORT_2_SAVE,
    MOD_E_NCE3_READ, MOD_E_NCE3_WRITE.

  Revision 2.1  2000/09/08  15:20:50  cibrario
  Updated template of MOD_W_BAD_KEY status code; added new status
  code MOD_W_BAD_OUT_BIT.  Both changes reflect the updates made to
  the keyboard emulation module in order to accommodate the new GUI.

  Revision 1.1  1998/02/17  14:55:04  cibrario
  Initial revision
.- */

#  include "types.h"

/*---------------------------------------------------------------------------
        Data type definitions
  ---------------------------------------------------------------------------*/

#  define N_MOD 6
#  define N_PAGE_TABLE_ENTRIES 16384
#  define N_HDW_SIZE 256

#  define N_ROM_SIZE_48 512 * 1024 * 2
#  define N_RAM_SIZE_48 128 * 1024 * 2
/* 2.4: Port_1 (CE2) size */
#  define N_PORT_1_SIZE_48 128 * 1024 * 2
/* 2.4: N_PORT_2_BANK_48
   This symbol is used to dimension the HP48GX Port_2: it denotes the
   number of 128 Kbyte banks the port must have and must be a power of 2
   between 1 and 32, inclusive.  When undefined, Port_2 is not emulated at all.
   The default value is 8, that is, Port_2 is emulated and its size is 1Mbyte.
*/
// #define N_PORT_2_BANK_48 ( config.model == MODEL_48GX ? 32 : 1 )
#  define N_PORT_2_BANK_48 32
/* 2.4: Port_2 (NCE3) size */
#  define N_PORT_2_SIZE_48 N_PORT_2_BANK_48 * 128 * 1024 * 2

#  define N_FLASH_SIZE_49 2048 * 1024 * 2 /* 3.2 */
#  define N_RAM_SIZE_49 512 * 1024 * 2    /* 3.2 */

#  define MOD_MAP_CHECK_OB_SIZE 128
#  define MOD_MAP_TABLE_OB_SIZE 512

/* 2.7: Number of entries in module config cache */
#  define N_MOD_CACHE_ENTRIES 8

/*---------------------------------------------------------------------------
        Macros

  MOD_ADDRESS returns the base address of a page, given its number (Address)
  MOD_OFFSET returns the page offset of an address (int)
  MOD_PAGE returns the page number of an address (Address)
  ---------------------------------------------------------------------------*/

#  define MOD_ADDRESS( page ) ( ( Address )( page ) << 6 )
#  define MOD_PAGE( address ) ( ( int )( ( ( address ) & 0xFFFC0 ) >> 6 ) )
#  define MOD_OFFSET( address ) ( ( address ) & 0x0003F )

/*
        ModDescription

  This const array contains an entry for each peripheral module connected
  to the peripheral bus of the Saturn CPU; the entry describes the
  characteristics of the module.

  name:		the mnemonic name of the module; the Saturn CPU doesn't
                actually use this information, but it's still useful during
                debugging.

  id:		the ID of the module, returned by the C=ID instruction
                when the module is unconfigured.

  access_prio:	the access priority of the module, when the address spaces of
                more than one module overlap. Higher values correspond to
                higher priorities.

                The configuration priority of the module, when there is more
                than one unconfigured module on the peripheral bus, is
                determined implicitly by the order in which the module
                descriptions into the array. The modules that come first
                in the array are configured first.

  init:		this function is called, without arguments, during VM startup
                to initialize the device. For example, the initialization
                function for the ROM module will read the ROM image from
                disk and store them into the module status structure.

  read:		this function reads a nibble from the module. It receives the
                relative address of the nibble to be read. The read function
                can return an interrupt request for the CPU.

  write:	this function writes a nibble to the module. It receives the
                relative address and the value of the nibble to be written.
                The write function can return an interrupt request for the CPU.

  r_config:     this flag contains the configuration status of the module after
                a bus reset. If the after-reset configuration status is
                MOD_CONFIGURED, the module can never be unconfigured.

  r_abs_base_addr: absolute base address of the module after a bus reset.
                It should be set only if the module is at least partially
                configured automatically after a bus reset.

  r_size:	size of the address window of the module after a bus reset.
                It should be set only if the module is at least partially
                configured automatically after a bus reset.

  map_flags:	special map flags:
        MOD_MAP_FLAGS_ABS	pass absolute addresses to module
                                read/write functions

  Notice that the current implementation requires that the index of
  the HDW registers in the mod_description table must be fixed
  and equal to MOD_HDW_INDEX... this is unfortunate.
*/

typedef void ( *ModInitFunction )( void );

typedef void ( *ModSaveFunction )( void );

typedef Nibble ( *ModReadFunction )( Address rel_addr );

typedef void ( *ModWriteFunction )( Address rel_addr, Nibble data );

typedef enum { MOD_UNCONFIGURED, MOD_SIZE_CONFIGURED, MOD_CONFIGURED } mod_config_t;

struct ModDescriptionEntry {
    const char* name;
    Address id;
    int access_prio;
#  define MOD_MIN_ACCESS_PRIO ( -1 )

    ModInitFunction init;
    ModSaveFunction save;
    ModReadFunction read;
    ModWriteFunction write;
    mod_config_t r_config;
    Address r_abs_base_addr;
    Address r_size;

    int map_flags;              /* 3.3 */
#  define MOD_MAP_FLAGS_ABS 0x1 /* Abs addresses to r/w */
};

#  define MOD_HDW_INDEX 1
typedef const struct ModDescriptionEntry ModDescription[ N_MOD ];

/*
        ModMapInfo

  This array contains an entry for each peripheral module connected
  to the peripheral bus of the Saturn CPU; the entry describes the
  dynamic mapping information of the module:

  config:	contains the current configuration status of the module.

  abs_base_addr: contains the current absolute base address of the module.
                It's valid only if the module is currently configured.

  size:		contains the current size of the address window of the module.
                It's valid only if the module is currently configured.
*/
struct ModMapInfoEntry {
    mod_config_t config;
    Address abs_base_addr;
    Address size;
};

typedef struct ModMapInfoEntry ModMapInfo[ N_MOD ];

/*
        ModPageTable

  This array contains an entry (of type ModPageTableEntry) for each 'page'
  (of size #40 nibbles) of the Saturn CPU physical address space. For
  each page, the following information is stored:

  index:	the index of the module that responds to the address range of
                the page in the ModDescription table.The special value
                MOD_NO_MOD_INDEX indicates that no module responds to the
                address range.

  rel_base_addr: the relative base address of the page in the address
                space of the module that responds to the address range of
                the page, if any.

  read, write:	the read/write functions of the module that responds to the
                address range of the page, if any.

        Relative address calculation for module access

  The Saturn Physical Address (SPA) is divided into two portions:

  * Page Index (PI):	PI = (SPA & 0xFFFC0) >> 6
  * Offset (OFF):	OFF = (SPA & 0x0003F)

  The Page Index determines which module will respond to the module
  access operation. ModPageTable[PI] contains the following information:

  * Relative Base Address (RBA)

  The relative address (RA) therefore will be RA = RBA | OFF; then, the
  appropriate module access function, found again in ModPageTable[PI],
  is called.

*/
struct ModPageTableEntry {
    int index;
#  define MOD_NO_MOD_INDEX ( -1 )

    Address rel_base_addr;
    ModReadFunction read;
    ModWriteFunction write;
};

typedef struct ModPageTableEntry ModPageTable[ N_PAGE_TABLE_ENTRIES ];

/*	struct ModCache (2.7)

  This structure holds the caching information for module config/unconfig.

  The .config field is an array of ModCacheConfigEntry, and contains
  the module configuration cache information. Each entry is a pair
  (tag, map_ptr).  The map_ptr field, when non-null,
  points to the struct ModMap to be used when a module config command,
  with the given tag address as argument, is executed.

  The .victim field points to the entry of .config to be used
  whenever a new fresh cache entry is needed after a cache miss.
  Currently, It is incremented by one at each replacement, thus
  implementing a very simple fifo replacement policy.

  The .unconfig field is an array of struct ModMap pointers, and
  contains the module unconfiguration cache information.
  The .unconfig[i] array element, when non-null, points to the
  struct ModMap to be used when a module unconfig command, unconfiguring
  the i-th module, is executed.

  The .config_point field is set if the struct ModMap is a point
  of the module configuration tree where a config was completed.
  It it used to walk back correctly when caching an unconfig.

  This .ref_count is incremented by one when the struct ModMap
  is referenced by an unconfig link; it is used to avoid freeing
  referenced structures.

  The .link field links all cached struct ModMap together.
*/
struct ModCacheTableEntry {
    Address tag;
    struct ModMap* map_ptr;
};

struct ModCache {
    struct ModCacheTableEntry config[ N_MOD_CACHE_ENTRIES ];
    int victim;

    struct ModMap*( unconfig[ N_MOD ] );

    int config_point;
    int ref_count;

    struct ModMap* link;
};

/*
        struct ModMap

  This structure contains all the mapping information about the peripheral
  modules of the Saturn CPU. Its components are:

  map_info:	this array describes the dynamic mapping information of
                each module connected to the Saturn peripheral bus.

  page_table:	this array describes the current layout of the address space
                of the Saturn CPU.

  cache (2.7):	this structure holds caching information used to speed up
                module config/unconfig instructions

*/
struct ModMap {
    ModMapInfo map_info;
    ModPageTable page_table;
    struct ModCache cache;
};

/*
        struct ModStatus

  This structure contains the actual status of all peripheral modules of the
  Saturn CPU. The status of all modules is centralized to allow any device
  to easily access the status of other devices.

        struct ModHdw

  This substructure contains the status of all peripheral devices controlled
  by the hdw module.

  3.2: To support the HP49 hw configuration, the original ModStatus structure
  has been splitted in two:

  - (new) struct ModStatus: contains configuration-independent status
    information (.hdw), and configuration-dependent accelerators (.hdw.accel).
    The overall layout of the structure is the same for all configurations,
    and it is publicily accessible.

  - struct ModStatus_xx (corresponding to all fields of the original
    ModStatus structure except .hdw): contains the storage areas for
    ROM and RAM in configuration 'xx', and is private to the
    configuration-specific ROM/RAM emulation module.

*/
struct ModHdw48Accel {
    XAddress bs_address; /* Bank Switcher ext. address */
};

struct ModHdw49Accel {
    XAddress view[ 2 ]; /* Base of Flash views */
};

struct ModHdw {
    Nibble hdw[ N_HDW_SIZE ]; /* HDW registers */

    /* LCD driver */
    Address lcd_base_addr; /* LCD driver base address */
    int lcd_on;            /* LCD driver enable flag */
    int lcd_contrast;      /* LCD contrast value */
    int lcd_vlc;           /* LCD vertical line count */
    int lcd_offset;        /* LCD horizontal offset */
    int lcd_line_offset;   /* LCD line offset */
    Address lcd_menu_addr; /* LCD menu base address */
    int lcd_ann;           /* LCD annunciators status */

    /* Timers */
    Nibble t1_ctrl; /* Timer 1 control */
#  define T1_CTRL_EXTRA 0x01
#  define T1_CTRL_INT 0x02
#  define T1_CTRL_WAKE 0x04
#  define T1_CTRL_SREQ 0x08

    Nibble t2_ctrl; /* Timer 2 control */
#  define T2_CTRL_TRUN 0x01
#  define T2_CTRL_INT 0x02
#  define T2_CTRL_WAKE 0x04
#  define T2_CTRL_SREQ 0x08

    Nibble t1_val; /* Timer 1 value */
    int32 t2_val;  /* Timer 2 value */

    /* 2.4: New member required to support Port emulation */
    Nibble card_status; /* Card status (hdw register 0x0F) */
#  define NCE3_CARD_PRESENT 0x01
#  define CE2_CARD_PRESENT 0x02
#  define NCE3_CARD_WE 0x04
#  define CE2_CARD_WE 0x08

    /* 2.4: Hw configuration-specific members used as accelerators;
       accel_valid is non-zero if the accelerators are valid
    */
    int accel_valid;

    union {
        struct ModHdw48Accel a48;
        struct ModHdw49Accel a49;
    } accel;

    /* 2.5: Serial port buffer registers */
    Byte serial_rbr;
    Byte serial_tbr;

    /* Misc */
    int16 crc; /* CRC */
};

struct ModStatus {
    struct ModHdw hdw; /* HDW status */
};

struct ModStatus_48 {
    Nibble rom[ N_ROM_SIZE_48 ];       /* Internal ROM */
    Nibble ram[ N_RAM_SIZE_48 ];       /* Internal RAM */
    Nibble port_1[ N_PORT_1_SIZE_48 ]; /* 2.4: Port_1 (CE2) storage */

    /* 2.4: Port_2 (NCE3) storage; only needed if N_PORT_2_BANK_48 is defined */
#  ifdef N_PORT_2_BANK_48
    Nibble port_2[ N_PORT_2_SIZE_48 ];
#  endif
};

struct ModStatus_49 {
    Nibble flash[ N_FLASH_SIZE_49 ]; /* Internal Flash ROM */
    Nibble ram[ N_RAM_SIZE_49 ];     /* Internal RAM */
    Nibble *ce2, *nce3;              /* ERAM bases */
};

/*---------------------------------------------------------------------------
        Global variables
  ---------------------------------------------------------------------------*/

extern struct ModStatus mod_status;

/*---------------------------------------------------------------------------
        Chf condition codes
  ---------------------------------------------------------------------------*/
typedef enum {
    MOD_I_CALLED = 101,            /* Function %s called */
    MOD_I_INITIALIZING = 102,      /* Initializing module %s */
    MOD_I_RESETTING = 103,         /* Resetting module %s */
    MOD_I_GET_ID = 106,            /* ModGetID returning %x */
    MOD_I_CONFIG = 107,            /* ModConfig %s %x %x completed */
    MOD_I_UNCONFIG = 108,          /* ModUnconfig %s %x %x completed */
    MOD_I_SAVING = 109,            /* Saving status of module %s */
    MOD_I_NOT_IMPLEMENTED = 110,   /* Function %s not implemented */
    MOD_I_REVISION = 111,          /* Modules revision: %s */
    MOD_I_BS_ADDRESS = 112,        /* 2.4: Bank Switcher address: %x */
    MOD_I_PORT_1_WP = 113,         /* 2.4: Port 1 is write protected */
    MOD_I_PORT_2_WP = 114,         /* 2.4: Port 2 is write protected */
    MOD_I_PERF_CTR = 115,          /* 2.7: Value of PerfCtr %s is %d */
    MOD_I_CACHED_UNCONFIG = 116,   /* 2.7: Cached ModUnconfig completed */
    MOD_I_CACHED_CONFIG = 117,     /* 2.7: Cached ModConfig %x comp. */
    MOD_I_UNCONFIG_L_HIT = 118,    /* 2.7: Late unconfig hit */
    MOD_I_UNCONFIG_L_MISS = 119,   /* 2.7: Late unconfig miss */
    MOD_W_BAD_CONFIG = 202,        /* Bad ModConfig %x ignored */
    MOD_W_BAD_UNCONFIG = 203,      /* Bad ModUnconfig %x ignored */
    MOD_W_HDW_WRITE = 204,         /* Bad HdwWrite %x, %x */
    MOD_W_HDW_READ = 205,          /* Bad HdwRead %x */
    MOD_W_RESETTING_ALL = 206,     /* Resetting all modules */
    MOD_W_RAM_INIT = 207,          /* Can't initialize internal RAM */
    MOD_W_HDW_INIT = 208,          /* Can't initialize HDW */
    MOD_W_BAD_KEY = 209,           /* 2.1: Bad key %s ignored */
    MOD_W_BAD_OUT_BIT = 210,       /* 2.1: Bad out_bit %x ignored */
    MOD_W_PORT_1_INIT = 211,       /* 2.4: Can't initialize Port 1 */
    MOD_W_PORT_2_INIT = 212,       /* 2.4: Can't initialize Port 2 */
    MOD_W_NO_VICTIM = 213,         /* 2.7: No cache victim; flush/retry */
    MOD_E_BAD_READ = 301,          /* Read unmapped addr %x */
    MOD_E_BAD_WRITE = 302,         /* Write unmapped addr %x datum %x */
    MOD_E_ROM_WRITE = 303,         /* Write into ROM addr %x datum %x */
    MOD_E_RAM_SAVE = 304,          /* Can't save internal RAM status */
    MOD_E_HDW_SAVE = 305,          /* Can't save HDW status */
    MOD_E_PORT_1_SAVE = 306,       /* 2.4: Can't save Port 1 status */
    MOD_E_CE1_WRITE = 307,         /* 2.4: Ce1Write addr %x datum %x */
    MOD_E_PORT_2_SAVE = 308,       /* 2.4: Can't save Port 2 status */
    MOD_E_NCE3_READ = 309,         /* 2.4: Read from NCE3 addr %x */
    MOD_E_NCE3_WRITE = 310,        /* 2.4: Wr. to NCE3 addr %x datum %x */
    MOD_E_NO_MATCH = 311,          /* 3.2: Hw desription %s not found */
    MOD_E_ROM_SAVE = 312,          /* 3.3: Can't save Flash ROM */
    MOD_F_MAP_SAVE = 401,          /* Can't save mod_map information */
    MOD_F_ROM_INIT = 402,          /* Can't initialize internal ROM */
    MOD_F_MAP_ALLOC = 403,         /* Dynamic map allocation failed */
    MOD_F_BAD_ALLOC_C = 404,       /* 2.7: Bad alloc_c %d aft FlushCache*/
    MOD_F_CHAIN_CORRUPTED = 405,   /* 2.7: ModMap chain corrupted */
    MOD_F_NO_VICTIM = 406,         /* 2.7: No cache victim after flush */
    MOD_F_MOD_STATUS_ALLOC = 407,  /* 3.2: ModStatus_xx alloc failed %d */
    MOD_F_NO_DESCRIPTION = 408,    /* 3.2: No module description */
    MOD_M_NOT_MAPPED = 501,        /* Address %x not mapped */
    MOD_M_MAPPED = 502,            /* Address %x mapped to %s:%x */
    MOD_M_MAP_TABLE_TITLE = 503,   /* */
    MOD_M_MAP_TABLE_ROW = 504,     /* %s %x %x %s */
    MOD_M_MAP_CONFIGURED = 505,    /* Configured */
    MOD_M_MAP_SZ_CONFIGURED = 506, /* Size configured */
    MOD_M_MAP_UNCONFIGURED = 507,  /* Unconfigured */
} modules_chf_message_id_t;

/*---------------------------------------------------------------------------
        Function prototypes
  ---------------------------------------------------------------------------*/

/* Initialization */
void ModRegisterDescription( ModDescription p );
void ModInit( void );
void ModSave( void );
void ModReset( void );

/* Configuration */
Address ModGetID( void );
void ModConfig( Address config_info );
void ModUnconfig( Address unconfig_info );

/* Read/Write */
Nibble FetchNibble( Address addr );
Nibble ReadNibble( Address addr );
void WriteNibble( Address addr, Nibble datum );

/* Monitor */
void monitor_ModMapCheck( Address addr, char ob[ MOD_MAP_CHECK_OB_SIZE ] );
void monitor_ModMapTable( char ob[ MOD_MAP_TABLE_OB_SIZE ] );

#endif /*!_MODULES_H*/
