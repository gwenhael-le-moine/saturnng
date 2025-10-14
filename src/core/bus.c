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

.creation     :	26-Jan-1998

.description  :
  This module contains the peripheral module configuration and access
  functions. All memory and i/o device accesses made by the Saturn CPU
  are handled by this module. References:

    SASM.DOC by HP  (HORN disk 4)
    Guide to the Saturn Processor Rev. 0.00f by Matthew Mastracci
    entries.srt by Mika Heiskanen  (mheiskan@vipunen.hut.fi)
    x48 source code by Eddie C. Dost  (ecd@dressler.de)

  NOTE: Preliminary profiling information on Digital UNIX shows that,
        without module config/unconfig cache (revision 1.1), while the
        Saturn CPU is doing bank switching over 48% of the CPU time is
        spent in RebuildPageTable(), with a peak of 60% when opening
        the PLOT input form.

        With the module config/unconfig cache enabled (revision 2.7),
        RebuildPageTable() drops to the 45th place in the gprof's
        listing of per-function cpu time.

        Moreover, even during intensive calculations, over 30% of the CPU
        time is spent in either bus_fetch_nibble() or RomRead() in either case.

.notes        :
  $Log: modules.c,v $
  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.10  2000/10/24 16:14:47  cibrario
  Added/Replaced GPL header

  Revision 3.5  2000/10/02 09:50:29  cibrario
  Linux support:
  - initialized 'winner' to an illegal value in RebuildPageTable(),
    to track 'impossible' module configurations.

  Revision 3.3  2000/09/26 15:16:31  cibrario
  Revised to implement Flash ROM write access:
  - in RebuildPageTable(), if the BUS_MAP_FLAGS_ABS is set in the
    .map_flags field of a modules description, the page table rebuild
    algorithm is modified to pass absolute (instead of relative
    addresses to the module read/write functions.

  Revision 3.2  2000/09/22  13:55:33  cibrario
  Implemented preliminary support of HP49 hw architecture:
  - The module description table can now be registered dynamically; its
    definition has been moved into hw_config.c
  - New function bus_set_description(), to register a module
    description table dynamically.
  - bus_init() now refuses to work if no module description table has
    been registered yet.
  - enabled forced alignment of
    module configuration sizes and addresses in bus_configure()

  Revision 3.1  2000/09/20  14:00:02  cibrario
  Minor updates and fixes to avoid gcc compiler warnings on Solaris
  when -ansi -pedantic -Wall options are selected.

  Revision 2.7  2000/09/19  11:11:08  cibrario
  Deeply revised to implement module config/unconfig cache.

  Revision 1.1  1998/02/18  08:16:35  cibrario
  Initial revision
.- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../libChf/src/Chf.h"

#include "bus.h"
#include "chf_wrapper.h"
#include "disk_io.h"

/*---------------------------------------------------------------------------
        Macros

  BUS_OFFSET returns the page offset of an address (int)
  BUS_PAGE returns the page number of an address (Address)
  ---------------------------------------------------------------------------*/

#define BUS_PAGE( address ) ( ( int )( ( ( address ) & 0xFFFC0 ) >> 6 ) )
#define BUS_OFFSET( address ) ( ( address ) & 0x0003F )

#define BUS_MIN_ACCESS_PRIO ( -1 )
#define BUS_HDW_INDEX 1
#define BUS_NO_BUS_INDEX ( -1 )

/*---------------------------------------------------------------------------
        Static/Global variables
  ---------------------------------------------------------------------------*/

struct BusStatus bus_status; /* Status information - global */

/* 2.7: Replaced the statically-allocated module mapping structure with a
   pointer to a dynamically-allocated structure, to be able to switch
   between different structures fast.  The BUS_MAP macro can be used to
   refer to the current module mapping structure, and bus_map_ptr
   points to it.
*/
static struct BusMap* bus_map_ptr; /* Module mapping information */
#define BUS_MAP ( *bus_map_ptr )

/* 2.7: All dynamically-allocated module mapping structures are linked
   together; cache_head points to the head of the list.  The list
   is used to flush the cache when necessary.
*/
static struct BusMap* cache_head = ( struct BusMap* )NULL;

/* 3.2: The BusDescription table is now configured invoking
   bus_set_description() before invoking any other function in this
   module.
*/
static const struct BusDescriptionEntry* bus_description;

/*---------------------------------------------------------------------------
        Private functions
  ---------------------------------------------------------------------------*/

/* .+

.creation     : 26-Jan-1998
.description  :
  This function is called when a read access is attempted for an unmapped
  address.

  It signals a warning and returns 0x0 to the caller. Since two consecutive
  zeros represent a RTNSXM instruction, this is an easy way to detect when
  a gosub transfers control to an unmapped address.

.call         :
                d = BadRead(addr);
.input        :
                Address addr, address
.output       :
                Nibble d, datum
.status_codes :
                BUS_E_BAD_READ
.notes        :
  1.1, 26-Jan-1998, creation

.- */
static Nibble BadRead( Address addr )
{
    ERROR( BUS_CHF_MODULE_ID, BUS_E_BAD_READ, addr )

    return ( Nibble )0x0;
}

/* .+

.creation     : 26-Jan-1998
.description  :
  This function is called when a write access is attempted for an unmapped
  address.

  It signals a warning and does not execute the write.

.call         :
                BadWrite(addr, datum);
.input        :
                Address addr, address
                Nibble datum, datum to be written
.output       :
                void
.status_codes :
                BUS_E_BAD_WRITE
.notes        :
  1.1, 26-Jan-1998, creation

.- */
static void BadWrite( Address addr, Nibble datum ) { ERROR( BUS_CHF_MODULE_ID, BUS_E_BAD_WRITE, addr, datum ) }

/* .+

.creation     : 26-Jan-1998
.description  :
  This function rebuilds the module page table from page 'lo' to page 'hi',
  inclusive, using the information contained in the current module
  mapping structure (BUS_MAP.map_info).

.call         :
                RebuildPageTable(lo, hi);
.input        :
                int lo, first page table entry to rebuild
                int hi, last page table entry to rebuild
.output       :
                void
.status_codes :
                BUS_I_CALLED
.notes        :
  1.1, 26-Jan-1998, creation
  3.3, 25-Sep-2000, update
    - implemented BUS_MAP_FLAGS_ABS flag in bus_description[].map_flags,
      to allow module read/write functions to receive absolute addresses
      instead of relative ones.
  3.5, 2-Oct-2000, update
    - initialized 'winner' to an illegal value to track 'impossible'
      module configurations.

.- */
static void RebuildPageTable( int page_start, int page_end )
{
    Address page_addr;
    int prio;
    int winner = -1;

    /* Scan all pages in the [lo, hi] range */
    for ( int page = page_start; page <= page_end; page++ ) {
        /* Calculate the base page address for the current page */
        page_addr = ( Address )( page ) << 6; // returns the base address of a page, given its number (Address)

        /* Scan the module mapping information table, searching for the module
           with the highest access priority that responds to the address
           page_addr. If succesful, 'prio' contains the access priority of the
           winner, and 'winner' contains its index, otherwise prio is set
           to BUS_MIN_ACCESS_PRIO.
        */
        prio = BUS_MIN_ACCESS_PRIO;
        for ( int mod = 0; mod < N_BUS_SIZE; mod++ ) {
            if ( BUS_MAP.map_info[ mod ].config == BUS_CONFIGURED && page_addr >= BUS_MAP.map_info[ mod ].abs_base_addr &&
                 page_addr < BUS_MAP.map_info[ mod ].abs_base_addr + BUS_MAP.map_info[ mod ].size &&
                 prio < bus_description[ mod ].access_prio ) {
                winner = mod;
                prio = bus_description[ mod ].access_prio;
            }
        }

        if ( prio == BUS_MIN_ACCESS_PRIO ) {
            /* The page is unmapped; set the module index to the special value
               BUS_NO_BUS_INDEX, the relative base address to zero and the
               read/write functions to BadRead/BadWrite, to catch accesses to
               unmapped addresses.
            */
            BUS_MAP.page_table[ page ].index = BUS_NO_BUS_INDEX;
            BUS_MAP.page_table[ page ].rel_base_addr = 0x00000;
            BUS_MAP.page_table[ page ].read = BadRead;
            BUS_MAP.page_table[ page ].write = BadWrite;
        } else {
            /* The page is mapped
               3.3: If the BUS_MAP_FLAGS_ABS is set in the winner's module
                    description, the base address of the page is set to its
                    absolute address; this way, the module read/write functions
                    will receive absolute addresses instead of relative ones.
            */
            BUS_MAP.page_table[ page ].index = winner;
            BUS_MAP.page_table[ page ].rel_base_addr = ( bus_description[ winner ].map_flags & BUS_MAP_FLAGS_ABS )
                                                           ? page_addr
                                                           : page_addr - BUS_MAP.map_info[ winner ].abs_base_addr;
            BUS_MAP.page_table[ page ].read = bus_description[ winner ].read;
            // FIXME: 48gx bug: This is the place where the RomWrite fonction is set. Should we avoid ROM being able to win?
            BUS_MAP.page_table[ page ].write = bus_description[ winner ].write;
        }
    }
}

/* .+

.creation     : 15-Sep-2000
.description  :
  This function clears all caching information associated with the
  struct BusMap pointed by its argument, and returns a pointer to
  the same structure.

.call         :
                d = ClearCachingInfo(d);
.input        :
                struct BusMap *d, ptr to the structure to be wiped off
.output       :
                struct BusMap *d, ptr to affected structure
.status_codes :
                BUS_I_CALLED
.notes        :
  2.7, 15-Sep-2000, creation

.- */
static struct BusMap* ClearCachingInfo( struct BusMap* d )
{
    static const struct BusCacheTableEntry empty = { ( Address )0, ( struct BusMap* )NULL };

    int i;

    for ( i = 0; i < N_BUS_CACHE_ENTRIES; i++ )
        d->cache.config[ i ] = empty;

    d->cache.victim = 0;

    for ( i = 0; i < N_BUS_SIZE; i++ )
        d->cache.unconfig[ i ] = ( struct BusMap* )NULL;

    d->cache.config_point = 0;
    d->cache.ref_count = 0;

    return d;
}

/* .+

.creation     : 15-Sep-2000
.description  :
  This function allocates a new struct BusMap, links it into the list of
  cached BusMap , and returns a pointer to it; the function
  signals a fatal condition if the allocation fails.

  Notice that this function does not initialize the struct BusMap in any
  way; in particular, it does not clear the caching information.

.call         :
                p = NewBusMap();
.input        :
                void
.output       :
                struct BusMap *p, pointer to the new struct BusMap
.status_codes :
                BUS_I_CALLED
                BUS_I_PERF_CTR, performance counter: %s value: %d
                BUS_F_MAP_ALLOC, allocation of new map failed
.notes        :
  2.7, 15-Sep-2000, creation

.- */
static struct BusMap* NewBusMap( void )
{
    struct BusMap* new = ( struct BusMap* )malloc( sizeof( struct BusMap ) );

    if ( new == ( struct BusMap* )NULL ) {
        SIGNAL_ERRNO
        FATAL0( BUS_CHF_MODULE_ID, BUS_F_MAP_ALLOC )
    }

    /* Link new structure to the cache list */
    new->cache.link = cache_head;
    cache_head = new;

    return new;
}

/* .+

.creation     : 15-Sep-2000
.description  :
  This function copies the contents of a struct BusMap into another,
  and clears the caching information of the destination structure.
  Returns a pointer to the destination structure.

  The linkage of the destination structure in the cached struct BusMap
  list (.link field) is preserved.

.call         :
                d = CopyBusMap(d, s);
.input        :
                const struct BusMap *s, ptr to source structure
.output       :
                struct BusMap *d, ptr to destination structure
.status_codes :
                BUS_I_CALLED
.notes        :
  2.7, 15-Sep-2000, creation

.- */
static struct BusMap* CopyBusMap( struct BusMap* d, const struct BusMap* s )
{
    struct BusMap* link = d->cache.link; /* Save .link of dest. */

    *d = *s;
    d->cache.link = link; /* Restore .link */

    return ClearCachingInfo( d );
}

/* .+

.creation     : 15-Sep-2000
.description  :
  This function replaces the struct BusMap pointed by *d with a copy of
  the struct BusMap pointed by s; if *d is currently a NULL pointer,
  a new struct BusMap is dynamically allocated before doing the copy
  and a pointer to the new structure is stored into *d, else the
  existing struct BusMap is overwritten.

  This function signals a fatal condition if the allocation of a new
  struct BusMap is required, and fails.

  This function always clears the caching information in the
  destination structure.

.call         :
                ReplaceBusMap(d, s);
.input        :
                struct BusMap **d, ptr to destination structure ptr
                const struct BusMap *s, ptr to source structure
.output       :
                struct BusMap **d, ptr to destination structure ptr;
                updated when original value of *d was NULL
.status_codes :
                BUS_I_CALLED
                BUS_I_PERF_CTR, performance counter: %s value: %d
                BUS_F_MAP_ALLOC, allocation of new map failed
.notes        :
  2.7, 15-Sep-2000, creation

.- */
static void ReplaceBusMap( struct BusMap** d, const struct BusMap* s )
{
    if ( *d == ( struct BusMap* )NULL )
        /* Allocation needed; cache cleared after allocation */
        *d = CopyBusMap( NewBusMap(), s );
    else
        CopyBusMap( *d, s );
}

/* .+

.creation     : 15-Sep-2000
.description  :
  This function flushes the whole cache, freeing all structures previously
  allocated by AllocBusMap(), except that pointed by save.  This function
  also clears the caching information contained in save, because this
  information is no longer valid after the flush.

.call         :
                FlushCache(save);
.input        :
                struct BusMap *save
.output       :
                void
.status_codes :
                BUS_I_CALLED
                BUS_F_BAD_ALLOC_C, bad alloc_c (%d) after FlushCache()
.notes        :
  2.7, 15-Sep-2000, creation

.- */
static void FlushCache( struct BusMap* save )
{
    struct BusMap* n;

    /* Scan the cache list; free all elements except that pointed by 'save' */
    struct BusMap* p = cache_head;
    while ( p != ( struct BusMap* )NULL ) {
        n = p->cache.link;

        if ( p != save )
            free( p );

        p = n;
    }

    /* The cache list now contains only 'save' */
    save->cache.link = ( struct BusMap* )NULL;
    cache_head = save;

    /* Clear the caching information in 'save' */
    ClearCachingInfo( save );
}

/* .+

.creation     : 15-Sep-2000
.description  :
  This function checks if there is an entry in the module configuration cache
  associated with the current struct BusMap with tag 'tag';
  if this is the case, it returns a pointer to the cached struct BusMap
  just found, otherwise it returns NULL (cache miss).

.call         :
                p = AccessConfigCache(tag);
.input        :
                Address tag, cache tag
.output       :
                struct BusMap *p, cached pointer, or NULL
.status_codes :
                BUS_I_CALLED
.notes        :
  2.7, 15-Sep-2000, creation

.- */
static struct BusMap* AccessConfigCache( Address tag )
{
    for ( int i = 0; i < N_BUS_CACHE_ENTRIES; i++ )
        if ( BUS_MAP.cache.config[ i ].tag == tag )
            return BUS_MAP.cache.config[ i ].map_ptr;

    return ( struct BusMap* )NULL;
}

/* .+

.creation     : 15-Sep-2000
.description  :
  This function follows the .unconfig cache links with index i, starting
  from the current struct BusMap, until it finds a struct BusMap
  whose .cache.config_point is set, or stumbles into a NULL pointer.

  When successful, this function returns a pointer to the cached
  struct BusMap just found, otherwise it returns NULL (cache miss).

.call         :
                p = AccessUnconfigCache(i);
.input        :
                int i, unconfig cache index (unconfigured module index)
.output       :
                struct BusMap *p, cached pointer, or NULL
.status_codes :
                BUS_I_CALLED
.notes        :
  2.7, 15-Sep-2000, creation

.- */
static struct BusMap* AccessUnconfigCache( int i )
{
    struct BusMap* p = BUS_MAP.cache.unconfig[ i ];

    while ( p != ( struct BusMap* )NULL && !p->cache.config_point )
        p = p->cache.unconfig[ i ];

    return p;
}

/* .+

.creation     : 19-Sep-2000
.description  :
  This function selects a victim entry in the module configuration
  cache table of the current struct BusMap, updates the victim selection
  info associated with the current map, and returns a pointer to the
  victim entry.

  If the search fails and the 'retry' argument is non-zero, this
  function signals a warning (BUS_W_NO_VICTIM), flushes the whole cache
  (by means of FlushCache()), and retries the search.  If even the
  second attempt fails, it signals a fatal condition (BUS_F_NO_VICTIM).

  If the search fails and the 'retry' argument is zero, this
  function immediately signals the fatal condition BUS_F_NO_VICTIM.

.call         :
                victim = SelectConfigVictim();
.input        :
                void
.output       :
                struct BusCacheTableEntry *victim, pointer to victim entry
.status_codes :
                BUS_I_CALLED
                BUS_W_NO_VICTIM
                BUS_F_NO_VICTIM
                BUS_F_BAD_ALLOC_C, bad alloc_c (%d) after FlushCache()
.notes        :
  2.7, 15-Sep-2000, creation

.- */
struct BusCacheTableEntry* SelectConfigVictim( int retry )
{
    int v = BUS_MAP.cache.victim;
    struct BusCacheTableEntry* victim = ( struct BusCacheTableEntry* )NULL;

    /* Scan the config cache entries, starting at .cache.victim,
       until a suitable one is found or the index loops around
    */
    do {
        /* A config cache entry is suitable for use if:
           - it is empty (map_ptr == NULL)
           - or the reference count of the associated map is 0
        */
        if ( ( BUS_MAP.cache.config[ v ].map_ptr == ( struct BusMap* )NULL ) || BUS_MAP.cache.config[ v ].map_ptr->cache.ref_count == 0 )
            victim = &( BUS_MAP.cache.config[ v ] );

        v = ( v + 1 ) % N_BUS_CACHE_ENTRIES;
    } while ( victim == ( struct BusCacheTableEntry* )NULL && v != BUS_MAP.cache.victim );

    if ( victim == ( struct BusCacheTableEntry* )NULL ) {
        if ( retry ) {
            /* Unable to find a victim; flush the cache and retry */
            WARNING0( BUS_CHF_MODULE_ID, BUS_W_NO_VICTIM )

            FlushCache( bus_map_ptr );

            victim = SelectConfigVictim( 0 );
        } else {
            /* Unable to find a victim; retry is not an option; give up */
            FATAL0( BUS_CHF_MODULE_ID, BUS_F_NO_VICTIM )
        }
    } else
        /* Found a victim; update next-victim index */
        BUS_MAP.cache.victim = v;

    return victim;
}

/* .+

.creation     : 19-Sep-2000
.description  :
  This function checks if there is in the cache a struct BusMap
  containing the same module configuration information (.map_info field)
  as the current one.

  If it founds a matching structure, this function returns a pointer
  to it, otherwise it returns NULL.

  This function should be used after execution of an unconfig instruction
  that encountered an early cache miss.

.call         :
                p = CheckForLateHit();
.input        :
                void
.output       :
                struct BusMap *p, cached pointer, or NULL
.status_codes :
                BUS_I_CALLED
.notes        :
  2.7, 19-Sep-2000, creation

.- */
static struct BusMap* CheckForLateHit( void )
{
    struct BusMap* p = cache_head;
    int i;

    /* Scan the cache to find an entry with the same modules configuration
       as the current one; return a pointer to it if successful
    */
    while ( p != ( struct BusMap* )NULL ) {
        /* Don't attempt to match an entry against itself */
        if ( p != bus_map_ptr ) {
            /* only .map_info contents must match */
            for ( i = 0; i < N_BUS_SIZE; i++ ) {
                /* Break the for if a difference was found */
                if ( ( bus_map_ptr->map_info[ i ].config != p->map_info[ i ].config ) ||
                     ( bus_map_ptr->map_info[ i ].abs_base_addr != p->map_info[ i ].abs_base_addr ) ||
                     ( bus_map_ptr->map_info[ i ].size != p->map_info[ i ].size ) )
                    break;
            }

            /* Break the while if we found a match ('for' was not broken) */
            if ( i == N_BUS_SIZE )
                break;
        }

        /* Go to the next cache entry */
        p = p->cache.link;
    }

    return p;
}

/* .+

.creation     : 19-Sep-2000
.description  :
  This function frees the cached struct BusMap pointed by p, preserving
  cache list linkage.

  It is responsibility of the caller to ensure that the structure is no
  longer referenced.

.call         :
                p = CheckForLateHit();
.input        :
                void
.output       :
                struct BusMap *p, cached pointer, or NULL
.status_codes :
                BUS_I_CALLED
                BUS_F_CHAIN_CORRUPTED
.notes        :
  2.7, 15-Sep-2000, creation

.- */
static void FreeBusMap( struct BusMap* p )
{
    struct BusMap* n;

    /* Free the struct BusMap pointed by p, preserving the linkage of
       other entries.  The caller must ensure that the entry is not
       referenced by any other entry through cache pointers.
    */
    if ( p == cache_head ) {
        /* Free the list head */
        cache_head = p->cache.link;
        free( p );
    } else {
        /* Scan the cache; at end, n is either null (!) or points to the
           cache entry that immediately precedes p
        */
        n = cache_head;
        while ( ( n != ( struct BusMap* )NULL ) && n->cache.link != p )
            n = n->cache.link;

        /* Should never happen */
        if ( n == ( struct BusMap* )NULL ) {
            FATAL0( BUS_CHF_MODULE_ID, BUS_F_CHAIN_CORRUPTED )
        }

        /* Bypass element pointed by p and free it */
        n->cache.link = p->cache.link;
        free( p );
    }
}

/*---------------------------------------------------------------------------
        Public functions
  ---------------------------------------------------------------------------*/

/* .+

.creation     : 21-Sep-2000
.description  :
  This function registers the BusDescription table pointed by 'p'; all
  other module emulation functions will refer to that table in the future.

  It is mandatory to invoke this function with a valid BusDescription
  table pointer as argument *before* using any other module emulation
  function, either directly or indirectly.

  All error conditions are signalled using the Chf facility; the function
  returns 'void' to the caller.

.call         :
                bus_set_description(p);
.input        :
                BusDescription p, module description table to be registered
.output       :
                void
.status_codes :
                BUS_I_CALLED

.notes        :
  3.2, 21-Sep-2000, creation

.- */
void bus_set_description( BusDescription p ) { bus_description = p; }

/* .+

.creation     : 23-Jan-1998
.description  :
  This function initializes all peripheral modules, calling its initialization
  entry point; it must be called exactly once during startup. Then, it
  initializes the modules mapping information either reading it from file or
  resetting all modules.

  All error conditions are signalled using the Chf facility; the function
  returns 'void' to the caller.

.call         :
                bus_init();
.input        :
                void
.output       :
                void
.status_codes :
                BUS_I_CALLED
                BUS_I_REVISION
                BUS_I_INITIALIZING
                BUS_I_PERF_CTR, performance counter: %s value: %d
                BUS_W_RESETTING_ALL
                BUS_F_MAP_ALLOC
                BUS_F_NO_DESCRIPTION

                NOTE: This function can also (indirectly) report any condition
                code generated and/or signalled by the module initialization
                functions.
.notes        :
  1.1, 23-Jan-1998, creation
  2.7, 15-Sep-2000, update
    - revised to implement module config/unconfig cache
  3.2, 21-Sep-2000, update
    - added sanity check on bus_description

.- */
void bus_init( void )
{
    /* First, a little sanity check on bus_description: ensure that
       bus_set_description() has been called at least once with a
       non-NULL argument.
    */
    if ( bus_description == NULL )
        FATAL0( BUS_CHF_MODULE_ID, BUS_F_NO_DESCRIPTION )

    /* Scan the bus_description table, initializing all modules */
    for ( int mod = 0; mod < N_BUS_SIZE; mod++ ) {
        DEBUG( BUS_CHF_MODULE_ID, DEBUG_C_MODULES, BUS_I_INITIALIZING, bus_description[ mod ].name )
        bus_description[ mod ].init();
    }

    /* Allocate the root struct BusMap and set it as the current one;
       the structure can be accessed using either bus_map_ptr or BUS_MAP.
    */
    bus_map_ptr = ClearCachingInfo( NewBusMap() );

    /* Attempt to restore the bus_map from file; reset modules if the read
       fails.
     */
    bool err = ReadStructFromFile( config.bus_path, sizeof( BUS_MAP.map_info ), &BUS_MAP.map_info );
    if ( err ) {
        WARNING0( BUS_CHF_MODULE_ID, BUS_W_RESETTING_ALL )

        /* Reset all modules */
        bus_reset();
    } else
        /* Rebuild page table (not saved on disk) */
        RebuildPageTable( 0, N_PAGE_TABLE_ENTRIES - 1 );
}

/* .+

.creation     : 11-Feb-1998
.description  :
  This function saves the status of all peripheral modules, calling its
  'save' entry point. Then, it saves the modules mapping information, too.

  All error conditions are signalled using the Chf facility; the function
  returns 'void' to the caller.

.call         :
                bus_save();
.input        :
                void
.output       :
                void
.status_codes :
                BUS_I_CALLED
                BUS_I_SAVING
                BUS_W_RESETTING

                NOTE: This function can also (indirectly) report any condition
                code generated and/or signalled by the module initialization
                functions.
.notes        :
  1.1, 11-Feb-1998, creation

.- */
void bus_save( void )
{
    /* Scan the bus_description table, initializing all modules */
    for ( int mod = 0; mod < N_BUS_SIZE; mod++ ) {
        DEBUG( BUS_CHF_MODULE_ID, DEBUG_C_MODULES, BUS_I_SAVING, bus_description[ mod ].name )
        bus_description[ mod ].save();
    }

    /* Attempt to save the bus_map from file */
    bool err = WriteStructToFile( &BUS_MAP.map_info, sizeof( BUS_MAP.map_info ), config.bus_path );
    if ( err )
        FATAL0( BUS_CHF_MODULE_ID, BUS_F_MAP_SAVE )
}

/* .+

.creation     : 26-Jan-1998
.description  :
  This function returns the ID of the next module to be configured and the
  last module configuration address. Returns 0x00000 if all modules are
  configured.

  Module ID structure
  Bits		Meaning

  19..8		Three most-significant nibbles of the last configuration
                address specified for this module.

  7..3		F: The next CONFIG data will specify a module base address
                0: The next CONFIG data will specify a module size

  3..0		Module ID data
                If the size of the module has already been configured, the
                base ID of the module is increased by one.

.call         :
                id = ModGetId();
.input        :
                void
.output       :
                Address id, ID of the next module to be configured.
.status_codes :
                BUS_I_CALLED
                BUS_I_GET_ID
.notes        :
  1.1, 26-Jan-1998, creation

.- */
Address bus_get_id( void )
{
    int mod;
    Address id;

    /* Scan the module information table searching for either an unconfigured
       or a partially configured module
    */
    for ( mod = 0; mod < N_BUS_SIZE && BUS_MAP.map_info[ mod ].config == BUS_CONFIGURED; mod++ )
        ;

    if ( mod == N_BUS_SIZE )
        /* All modules are configured */
        id = ( Address )0x00000;
    else
        /* Build the module id */
        id = ( BUS_MAP.map_info[ mod ].abs_base_addr & 0xFFF00 ) |
             ( BUS_MAP.map_info[ mod ].config == BUS_UNCONFIGURED ? 0x00000 : 0x000F0 ) |
             ( bus_description[ mod ].id + ( BUS_MAP.map_info[ mod ].config == BUS_UNCONFIGURED ? 0 : 1 ) );

    DEBUG( BUS_CHF_MODULE_ID, DEBUG_C_MODULES, BUS_I_GET_ID, id )
    return id;
}

/* .+

.creation     : 26-Jan-1998
.description  :
  This function resets all peripheral modules and rebuilds the module page
  table used for module access.

.call         :
                bus_reset();
.input        :
                void
.output       :
                void
.status_codes :
                BUS_I_CALLED
                BUS_I_RESETTING
                BUS_F_BAD_ALLOC_C, bad alloc_c (%d) after FlushCache()
.notes        :
  1.1, 26-Jan-1998, creation
  2.7, 15-Sep-2000, update
    - revised to implement module config/unconfig cache

.- */
void bus_reset( void )
{
    /* Scan the bus_description table, initializing the module
       mapping information BUS_MAP.map_info.
    */
    for ( int mod = 0; mod < N_BUS_SIZE; mod++ ) {
        DEBUG( BUS_CHF_MODULE_ID, DEBUG_C_MODULES, BUS_I_RESETTING, bus_description[ mod ].name )

        /* Set the module configuration status */
        BUS_MAP.map_info[ mod ].config = bus_description[ mod ].r_config;
        BUS_MAP.map_info[ mod ].abs_base_addr = bus_description[ mod ].r_abs_base_addr;
        BUS_MAP.map_info[ mod ].size = bus_description[ mod ].r_size;
    }

    /* Rebuild the module page table */
    RebuildPageTable( 0, N_PAGE_TABLE_ENTRIES - 1 );

    /* Flush the whole struct BusMap cache, preserving the current map */
    FlushCache( bus_map_ptr );

    /* Mark the current struct BusMap to be a configuration point;
       this flag is used by the unconfig cache code to correctly
       undo the last config
    */
    BUS_MAP.cache.config_point = 1;
}

/* .+

.creation     : 26-Jan-1998
.description  :
  This function configures a module, using the given 'config_info'.

  The target module will be the first unconfigured or partially configured
  module found in the BUS_MAP.map_info table.

  If the target module is unconfigured, bus_configure sets the size of its
  address space to 0x100000 - 'config_info'; the module then becomes
  partially configured.

  If the target module is already partially configured, bus_configure sets
  its base address to 'config_info', completing the configuration process.

  In the latter case, bus_configure rebuilds the page table used for module access
  to reflect the visibility of the new module in the CPU address space.

.call         :
                void bus_configure(config_info);
.input        :
                Address config_info, configuration information
.output       :
                void
.status_codes :
                BUS_I_CALLED
                BUS_I_CONFIG
                BUS_I_CACHED_CONFIG
                BUS_I_PERF_CTR, performance counter: %s value: %d
                BUS_W_BAD_CONFIG
                BUS_W_NO_VICTIM
                BUS_F_MAP_ALLOC
                BUS_F_NO_VICTIM
.notes        :
  1.1, 26-Jan-1998, creation
  2.7, 15-Sep-2000, update
    - implemented module config/unconfig cache
  3.2, 22-Sep-2000, update
    - enabled forced alignment
      of config_info
.- */
void bus_configure( Address config_info )
{
    int mod;

    /* 3.2: The HP49 firmware (1.19-4) can generate misaligned config
            addresses, that is, addresses that are not a multiple of 0x100;
            silently align them here.
    */
    config_info &= ~0xFF;

    /* ACCESS CONFIG CACHE */
    struct BusMap* nxt = AccessConfigCache( config_info );
    if ( nxt != ( struct BusMap* )NULL ) {
        /* CACHE HIT; switch bus_map_ptr */
        bus_map_ptr = nxt;

        DEBUG( BUS_CHF_MODULE_ID, DEBUG_C_BUS_CACHE, BUS_I_CACHED_CONFIG, config_info )
        return;
    }

    /* CACHE MISS */

    /* Select a 'victim' cache table entry and update victim
       selection info; retry after flushing the cache if necessary.

       Initialize victim's tag to current config_info.

       Initialize victim's map_ptr; this can be either a new
       allocation if the pointer was NULL, or a replacement.
       This clears the caching information of the map_ptr's pointee.

       Switch bus_map_ptr to the new structure

       The unconfig cache pointer of the new structure will be set when
       the index of the module being configured will be known.
    */
    struct BusCacheTableEntry* victim = SelectConfigVictim( 1 );

    victim->tag = config_info;
    ReplaceBusMap( &( victim->map_ptr ), bus_map_ptr );

    struct BusMap* old = bus_map_ptr;
    bus_map_ptr = victim->map_ptr;

    /* Scan the module information table searching for either an unconfigured
       or a partially configured module
    */
    for ( mod = 0; mod < N_BUS_SIZE && BUS_MAP.map_info[ mod ].config == BUS_CONFIGURED; mod++ )
        ;

    if ( mod == N_BUS_SIZE ) {
        /* All modules are configured - Signal a warning */
        // 48gx bugs here when running VERSION
        // both x48ng and hpemung silently ignore this
        INFO( BUS_CHF_MODULE_ID, BUS_W_BAD_CONFIG, config_info )
        return;
    }

    if ( BUS_MAP.map_info[ mod ].config == BUS_UNCONFIGURED ) {
        /* First call: The module was unconfigured; configure its size */
        BUS_MAP.map_info[ mod ].size = 0x100000 - config_info;
        BUS_MAP.map_info[ mod ].config = BUS_SIZE_CONFIGURED;
    } else {
        /* Second call: The module size was already configured; configure its base address */
        BUS_MAP.map_info[ mod ].abs_base_addr = config_info;
        BUS_MAP.map_info[ mod ].config = BUS_CONFIGURED;

        /* Rebuild the page table */
        RebuildPageTable( BUS_PAGE( BUS_MAP.map_info[ mod ].abs_base_addr ),
                          BUS_PAGE( BUS_MAP.map_info[ mod ].abs_base_addr + BUS_MAP.map_info[ mod ].size - 1 ) );

        /* Mark the current struct BusMap to be a configuration point;
           this flag is used by the unconfig cache code to correctly
           undo the last config
        */
        BUS_MAP.cache.config_point = 1;

        DEBUG( BUS_CHF_MODULE_ID, DEBUG_C_MODULES | DEBUG_C_BUS_CACHE, BUS_I_CONFIG, bus_description[ mod ].name,
               BUS_MAP.map_info[ mod ].abs_base_addr, BUS_MAP.map_info[ mod ].size )
    }

    /* Set the unconfig cache pointer of module 'mod' to the old BusMap,
       and increment its reference counter, to avoid freeing it
       improperly.
    */
    BUS_MAP.cache.unconfig[ mod ] = old;
    old->cache.ref_count++;
}

/* .+

.creation     : 26-Jan-1998
.description  :
  This function unconfigures the module currently configured at address
  'unconfig_info' and returns it to its after-reset configuration status.

  bus_unconfigure also rebuilds the page table used for module access
  to reflect the loss of visibility of the module in the CPU address space.

.call         :
                bus_unconfigure(unconfig_info);
.input        :
                Address unconfig_info, Unconfig information
.output       :
                void
.status_codes :
                BUS_I_CALLED
                BUS_I_UNCONFIG
                BUS_I_CACHED_UNCONFIG
                BUS_I_PERF_CTR, performance counter: %s value: %d
                BUS_W_BAD_UNCONFIG
                BUS_F_MAP_ALLOC
                BUS_F_CHAIN_CORRUPTED
.notes        :
  1.1, 26-Jan-1998, creation
  2.7, 15-Sep-2000, update
    - implemented module config/unconfig cache

.- */
void bus_unconfigure( Address unconfig_info )
{
    int mod = BUS_MAP.page_table[ BUS_PAGE( unconfig_info ) ].index;

    /* Determine the module to unconfigure */
    if ( ( mod == BUS_NO_BUS_INDEX ) || ( bus_description[ mod ].r_config == BUS_CONFIGURED ) ) {
        /* There isn't any module configured at the given address -
           Signal a warning
         */
        /* or */
        /* The module is automatically configured after reset; it can never
           be unconfigured.
         */
        // both x48ng and hpemung silently ignore this
        INFO( BUS_CHF_MODULE_ID, BUS_W_BAD_UNCONFIG, unconfig_info )
        return;
    }

    /* Unconfiguring module 'mod': ACCESS UNCONFIG CACHE */
    struct BusMap* nxt = AccessUnconfigCache( mod );
    if ( nxt != ( struct BusMap* )NULL ) {
        /* CACHE HIT; switch bus_map_ptr */
        bus_map_ptr = nxt;

        DEBUG0( BUS_CHF_MODULE_ID, DEBUG_C_BUS_CACHE, BUS_I_CACHED_UNCONFIG )
        return;
    }

    /* CACHE MISS

       A clone of the current struct BusMap is allocated and updated
       according to the unconfig instruction being executed.

       Then, CheckForLateHit() is called to check whether in the
       module mapping cache there is a struct BusMap identical to
       the updated one.

       - If there is, the .unconfig[i] link is updated to point to
         the cache entry just found.

       - If there is not, the whole cache is flushed and all cached
         BusMap structures allocated so far are freed, except the
         current one.  I hope this occurrence is rare.
    */

    /* Save pointer to the old map and switch to a temporary one */
    struct BusMap* old = bus_map_ptr;
    bus_map_ptr = CopyBusMap( NewBusMap(), bus_map_ptr );

    /* Update the mapping information table */
    BUS_MAP.map_info[ mod ].config = bus_description[ mod ].r_config;

    /* Rebuild the page table */
    RebuildPageTable( BUS_PAGE( BUS_MAP.map_info[ mod ].abs_base_addr ),
                      BUS_PAGE( BUS_MAP.map_info[ mod ].abs_base_addr + BUS_MAP.map_info[ mod ].size - 1 ) );

    /* Reset the module configuration status; the abs_base_addr of the module
       is not reset because its old value is still needed by ModGetId()
       The size is reset for the modules that are already BUS_SIZE_CONFIGURED
       immediately after reset.
    */
    BUS_MAP.map_info[ mod ].size = bus_description[ mod ].r_size;

    nxt = CheckForLateHit();
    if ( nxt != ( struct BusMap* )NULL ) {
        /* Update pointer from the old map to the new one, and increment
           reference counter of the referenced structure
        */
        old->cache.unconfig[ mod ] = nxt;
        nxt->cache.ref_count++;

        /* Discard the temporary map and switch to the cached one */
        FreeBusMap( bus_map_ptr );
        bus_map_ptr = nxt;

        DEBUG0( BUS_CHF_MODULE_ID, DEBUG_C_BUS_CACHE, BUS_I_UNCONFIG_L_HIT )
    } else {
        /* Continue to use the new map with no caching information,
           and hope that further configuration activities will link it
           back in the immediate future.
        */

        /* Mark the current struct BusMap to be a configuration point;
           this flag is used by the unconfig cache code to correctly
           undo the last config
        */
        BUS_MAP.cache.config_point = 1;

        DEBUG0( BUS_CHF_MODULE_ID, DEBUG_C_BUS_CACHE, BUS_I_UNCONFIG_L_MISS )

        DEBUG( BUS_CHF_MODULE_ID, DEBUG_C_MODULES | DEBUG_C_BUS_CACHE, BUS_I_UNCONFIG, bus_description[ mod ].name,
               BUS_MAP.map_info[ mod ].abs_base_addr, BUS_MAP.map_info[ mod ].size );
    }
}

/* .+

.creation     : 26-Jan-1998
.description  :
  This function fetches a nibble from the address 'addr' and returns it.

  NOTE: This function DOES NOT update the hardware CRC register.

.call         :
                d = bus_fetch_nibble(addr);
.input        :
                Address addr, address
.output       :
                Nibble *d, datum
.status_codes :
                NOTE: This function indirectly reports and condition generated
                and/or signalled by the module read function.
.notes        :
  1.1, 26-Jan-1998, creation

.- */
Nibble bus_fetch_nibble( Address addr )
{
    register int page = BUS_PAGE( addr );

    return BUS_MAP.page_table[ page ].read( BUS_MAP.page_table[ page ].rel_base_addr | BUS_OFFSET( addr ) );
}

/* .+

.creation     : 26-Jan-1998
.description  :
  This function reads a nibble from the address 'addr' and returns it.

  NOTE: This function updates the hardware CRC register if the target of the
        read operation is not the HDW module. The current (1.1) implementation
        of this feature is inefficient because the .index field of the
        addressed page must be checked against BUS_HDW_INDEX for each
        access.

.call         :
                d = bus_read_nibble(addr);
.input        :
                Address addr, address
.output       :
                Nibble *d, datum
.status_codes :
                NOTE: This function indirectly reports and condition generated
                and/or signalled by the module read function.
.notes        :
  1.1, 26-Jan-1998, creation

.- */
Nibble bus_read_nibble( Address addr )
{
    register int page = BUS_PAGE( addr );

    /* Read the nibble from the peripheral module */
    register Nibble d = BUS_MAP.page_table[ page ].read( BUS_MAP.page_table[ page ].rel_base_addr | BUS_OFFSET( addr ) );

    /* Update the crc register, if appropriate */
    if ( BUS_MAP.page_table[ page ].index != BUS_HDW_INDEX )
        bus_status.hdw.crc = ( bus_status.hdw.crc >> 4 ) ^ ( ( ( bus_status.hdw.crc ^ d ) & 0x0F ) * 0x1081 );

    /* Return to the caller */
    return d;
}

/* .+

.creation     : 26-Jan-1998
.description  :
  This function writes the nibble 'datum' to the address 'addr'

  NOTE: This function DOES NOT update the hardware CRC register.

.call         :
                bus_write_nibble(addr, datum);
.input        :
                Address addr, destination address
                Nibble datum, nibble to be written
.output       :
                void
.status_codes :
                NOTE: This function indirectly reports and condition generated
                and/or signalled by the module write function.
.notes        :
  1.1, 26-Jan-1998, creation

.- */
void bus_write_nibble( Address addr, Nibble datum )
{
    register int page = BUS_PAGE( addr );

    BUS_MAP.page_table[ page ].write( BUS_MAP.page_table[ page ].rel_base_addr | BUS_OFFSET( addr ), datum );
}

/*---------------------------------------------------------------------------
        Monitor functions
  ---------------------------------------------------------------------------*/

/* .+

.creation     : 26-Jan-1998
.description  :
  This function fills the string 'ob' with the current mapping information
  for address 'addr'; it is used by the emulator monitor only.

.call         :
                monitor_BusMapCheck(addr, char ob[BUS_MAP_CHECK_OB_SIZE]);
.input        :
                Address addr;
.output       :
                void
.status_codes :
                *
.notes        :
  1.1, 26-Jan-1998, creation

.- */
void monitor_BusMapCheck( Address addr, char ob[ BUS_MAP_CHECK_OB_SIZE ] )
{
    int page = BUS_PAGE( addr );
    Address offset = BUS_OFFSET( addr );
    int mod = BUS_MAP.page_table[ page ].index;

    if ( mod == BUS_NO_BUS_INDEX )
        sprintf( ob, "A[%05X] -> *Not Mapped*", addr );
    else {
        Address rel_addr = BUS_MAP.page_table[ page ].rel_base_addr | offset;

        sprintf( ob, "A[%05X] -> M[%s] R[%05X]", addr, bus_description[ mod ].name, rel_addr );
    }

    ChfSignal( BUS_CHF_MODULE_ID );
}

/* .+

.creation     : 26-Jan-1998
.description  :
  This function fills the string 'ob' with the current mapping table for
  all modules; it is used by the emulator monitor only.

.call         :
                monitor_BusMapTable(char ob[BUS_MAP_TABLE_OB_SIZE]);
.input        :
                Address addr;
.output       :
                void
.status_codes :
                *
.notes        :
  1.1, 26-Jan-1998, creation

.- */
void monitor_BusMapTable( char ob[ BUS_MAP_TABLE_OB_SIZE ] )
{
    sprintf( ob, "%s\n", "Device\t\t\tAddress\tSize\tStatus" );
    ob += strlen( ob );

    for ( int mod = 0; mod < N_BUS_SIZE; mod++ ) {
        sprintf( ob, "%s\t%05X\t%05X\t%s", bus_description[ mod ].name, BUS_MAP.map_info[ mod ].abs_base_addr, BUS_MAP.map_info[ mod ].size,
                 BUS_MAP.map_info[ mod ].config == BUS_CONFIGURED
                     ? "Configured"
                     : ( BUS_MAP.map_info[ mod ].config == BUS_SIZE_CONFIGURED ? "Size_configured" : "*Unconfigured*" ) );

        strcat( ob, "\n" );
        ob += strlen( ob );
    }
}
