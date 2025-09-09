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
        time is spent in either FetchNibble() or RomRead() in either case.

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
  - in RebuildPageTable(), if the MOD_MAP_FLAGS_ABS is set in the
    .map_flags field of a modules description, the page table rebuild
    algorithm is modified to pass absolute (instead of relative
    addresses to the module read/write functions.

  Revision 3.2  2000/09/22  13:55:33  cibrario
  Implemented preliminary support of HP49 hw architecture:
  - The module description table can now be registered dynamically; its
    definition has been moved into hw_config.c
  - New function ModRegisterDescription(), to register a module
    description table dynamically.
  - ModInit() now refuses to work if no module description table has
    been registered yet.
  - enabled forced alignment of
    module configuration sizes and addresses in ModConfig()
 
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

#include "config.h"
#include "cpu.h"
#include "modules.h"
#include "disk_io.h"
#include "debug.h"

/*---------------------------------------------------------------------------
        Static/Global variables
  ---------------------------------------------------------------------------*/

struct ModStatus mod_status; /* Status information - global */

/* 2.7: Replaced the statically-allocated module mapping structure with a
   pointer to a dynamically-allocated structure, to be able to switch
   between different structures fast.  The mod_map macro can be used to
   refer to the current module mapping structure, and mod_map_ptr
   points to it.
*/
static struct ModMap* mod_map_ptr; /* Module mapping information */
#define MOD_MAP ( *mod_map_ptr )

/* 2.7: All dynamically-allocated module mapping structures are linked
   together; cache_head points to the head of the list.  The list
   is used to flush the cache when necessary.
*/
static struct ModMap* cache_head = ( struct ModMap* )NULL;

/* 3.2: The ModDescription table is now configured invoking
   ModRegisterDescription() before invoking any other function in this
   module.
*/
static const struct ModDescriptionEntry* mod_description;

/*---------------------------------------------------------------------------
        Debugging & performance analysis data
  ---------------------------------------------------------------------------*/

#ifdef DEBUG

static int alloc_c = 0; /* Counter of live AllocModMap() invocations */
static int flush_c = 0; /* Counter of FlushCache() invocations */
static int hit_c = 0;   /* Cache hit counter */
static int lhit_c = 0;  /* Cache late unconfig hit counter */
static int miss_c = 0;  /* Cache miss (without replacement) counter */
static int repl_c = 0;  /* Entry replacement counter */

#  define IncPerfCtr( x ) x++
#  define DecPerfCtr( x ) x--
#  define PrintPerfCtr( x ) debug2( MOD_CHF_MODULE_ID, DEBUG_C_MOD_CACHE, MOD_I_PERF_CTR, #x, x )

#  define PrintCacheStats                                                                                                                  \
      {                                                                                                                                    \
          PrintPerfCtr( alloc_c );                                                                                                         \
          PrintPerfCtr( flush_c );                                                                                                         \
          PrintPerfCtr( hit_c );                                                                                                           \
          PrintPerfCtr( lhit_c );                                                                                                          \
          PrintPerfCtr( miss_c );                                                                                                          \
          PrintPerfCtr( repl_c );                                                                                                          \
      }

#else

#  define IncPerfCtr( x )
#  define DecPerfCtr( x )
#  define PrintPerfCtr( x )

#  define PrintCacheStats

#endif

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
                MOD_E_BAD_READ
.notes        :
  1.1, 26-Jan-1998, creation

.- */
static Nibble BadRead( Address addr )
{
    ChfGenerate( MOD_CHF_MODULE_ID, __FILE__, __LINE__, MOD_E_BAD_READ, CHF_ERROR, addr );
    ChfSignal( MOD_CHF_MODULE_ID );

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
                MOD_E_BAD_WRITE
.notes        :
  1.1, 26-Jan-1998, creation

.- */
static void BadWrite( Address addr, Nibble datum )
{
    ChfGenerate( MOD_CHF_MODULE_ID, __FILE__, __LINE__, MOD_E_BAD_WRITE, CHF_ERROR, addr, datum );
    ChfSignal( MOD_CHF_MODULE_ID );
}

/* .+

.creation     : 26-Jan-1998
.description  :
  This function rebuilds the module page table from page 'lo' to page 'hi',
  inclusive, using the information contained in the current module
  mapping structure (MOD_MAP.map_info).

.call         :
                RebuildPageTable(lo, hi);
.input        :
                int lo, first page table entry to rebuild
                int hi, last page table entry to rebuild
.output       :
                void
.status_codes :
                MOD_I_CALLED
.notes        :
  1.1, 26-Jan-1998, creation
  3.3, 25-Sep-2000, update
    - implemented MOD_MAP_FLAGS_ABS flag in mod_description[].map_flags,
      to allow module read/write functions to receive absolute addresses
      instead of relative ones.
  3.5, 2-Oct-2000, update
    - initialized 'winner' to an illegal value to track 'impossible'
      module configurations.

.- */
static void RebuildPageTable( int lo, int hi )
{
    Address page_addr;
    int prio;
    int winner = -1;

    debug1( MOD_CHF_MODULE_ID, DEBUG_C_TRACE, MOD_I_CALLED, "RebuildPageTable" );

    /* Scan all pages in the [lo, hi] range */
    for ( int page = lo; page <= hi; page++ ) {
        /* Calculate the base page address for the current page */
        page_addr = ModAddress( page );

        /* Scan the module mapping information table, searching for the module
           with the highest access priority that responds to the address
           page_addr. If succesful, 'prio' contains the access priority of the
           winner, and 'winner' contains its index, otherwise prio is set
           to MOD_MIN_ACCESS_PRIO.
        */
        prio = MOD_MIN_ACCESS_PRIO;
        for ( int mod = 0; mod < N_MOD; mod++ ) {
            if ( MOD_MAP.map_info[ mod ].config == MOD_CONFIGURED && page_addr >= MOD_MAP.map_info[ mod ].abs_base_addr &&
                 page_addr < MOD_MAP.map_info[ mod ].abs_base_addr + MOD_MAP.map_info[ mod ].size &&
                 prio < mod_description[ mod ].access_prio ) {
                winner = mod;
                prio = mod_description[ mod ].access_prio;
            }
        }

        if ( prio == MOD_MIN_ACCESS_PRIO ) {
            /* The page is unmapped; set the module index to the special value
               MOD_NO_MOD_INDEX, the relative base address to zero and the
               read/write functions to BadRead/BadWrite, to catch accesses to
               unmapped addresses.
            */
            MOD_MAP.page_table[ page ].index = MOD_NO_MOD_INDEX;
            MOD_MAP.page_table[ page ].rel_base_addr = 0x00000;
            MOD_MAP.page_table[ page ].read = BadRead;
            MOD_MAP.page_table[ page ].write = BadWrite;
        } else {
            /* The page is mapped
               3.3: If the MOD_MAP_FLAGS_ABS is set in the winner's module
                    description, the base address of the page is set to its
                    absolute address; this way, the module read/write functions
                    will receive absolute addresses instead of relative ones.
            */
            MOD_MAP.page_table[ page ].index = winner;
            MOD_MAP.page_table[ page ].rel_base_addr = ( mod_description[ winner ].map_flags & MOD_MAP_FLAGS_ABS )
                                                           ? page_addr
                                                           : page_addr - MOD_MAP.map_info[ winner ].abs_base_addr;
            MOD_MAP.page_table[ page ].read = mod_description[ winner ].read;
            // FIXME: 48gx VERSION bug: This is the place where the RomWrite fonction is set. Should we avoid ROM being able to win?
            MOD_MAP.page_table[ page ].write = mod_description[ winner ].write;
        }
    }
}

/* .+

.creation     : 15-Sep-2000
.description  :
  This function clears all caching information associated with the
  struct ModMap pointed by its argument, and returns a pointer to
  the same structure.

.call         :
                d = ClearCachingInfo(d);
.input        :
                struct ModMap *d, ptr to the structure to be wiped off
.output       :
                struct ModMap *d, ptr to affected structure
.status_codes :
                MOD_I_CALLED
.notes        :
  2.7, 15-Sep-2000, creation

.- */
static struct ModMap* ClearCachingInfo( struct ModMap* d )
{
    static const struct ModCacheTableEntry empty = { ( Address )0, ( struct ModMap* )NULL };

    int i;

    debug1( MOD_CHF_MODULE_ID, DEBUG_C_TRACE, MOD_I_CALLED, "ClearCachingInfo" );

    for ( i = 0; i < N_MOD_CACHE_ENTRIES; i++ )
        d->cache.config[ i ] = empty;
    d->cache.victim = 0;

    for ( i = 0; i < N_MOD; i++ )
        d->cache.unconfig[ i ] = ( struct ModMap* )NULL;
    d->cache.config_point = 0;
    d->cache.ref_count = 0;

    return d;
}

/* .+

.creation     : 15-Sep-2000
.description  :
  This function allocates a new struct ModMap, links it into the list of
  cached ModMap , and returns a pointer to it; the function
  signals a fatal condition if the allocation fails.

  Notice that this function does not initialize the struct ModMap in any
  way; in particular, it does not clear the caching information.

  If DEBUG is appropriately enabled, this function prints out the
  current value of all cache performance counters (PrintCacheStats).

.call         :
                p = NewModMap();
.input        :
                void
.output       :
                struct ModMap *p, pointer to the new struct ModMap
.status_codes :
                MOD_I_CALLED
                MOD_I_PERF_CTR, performance counter: %s value: %d
                MOD_F_MAP_ALLOC, allocation of new map failed
.notes        :
  2.7, 15-Sep-2000, creation

.- */
static struct ModMap* NewModMap( void )
{
    struct ModMap* new;

    debug1( MOD_CHF_MODULE_ID, DEBUG_C_TRACE, MOD_I_CALLED, "NewModMap" );

    if ( ( new = ( struct ModMap* )malloc( sizeof( struct ModMap ) ) ) == ( struct ModMap* )NULL ) {
        ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
        ChfGenerate( MOD_CHF_MODULE_ID, __FILE__, __LINE__, MOD_F_MAP_ALLOC, CHF_FATAL );
        ChfSignal( MOD_CHF_MODULE_ID );
    }

    /* Link new structure to the cache list */
    new->cache.link = cache_head;
    cache_head = new;

    IncPerfCtr( alloc_c );
    PrintCacheStats;

    return new;
}

/* .+

.creation     : 15-Sep-2000
.description  :
  This function copies the contents of a struct ModMap into another,
  and clears the caching information of the destination structure.
  Returns a pointer to the destination structure.

  The linkage of the destination structure in the cached struct ModMap
  list (.link field) is preserved.

.call         :
                d = CopyModMap(d, s);
.input        :
                const struct ModMap *s, ptr to source structure
.output       :
                struct ModMap *d, ptr to destination structure
.status_codes :
                MOD_I_CALLED
.notes        :
  2.7, 15-Sep-2000, creation

.- */
static struct ModMap* CopyModMap( struct ModMap* d, const struct ModMap* s )
{
    struct ModMap* link = d->cache.link; /* Save .link of dest. */

    debug1( MOD_CHF_MODULE_ID, DEBUG_C_TRACE, MOD_I_CALLED, "CopyModMap" );

    *d = *s;
    d->cache.link = link; /* Restore .link */
    return ClearCachingInfo( d );
}

/* .+

.creation     : 15-Sep-2000
.description  :
  This function replaces the struct ModMap pointed by *d with a copy of
  the struct ModMap pointed by s; if *d is currently a NULL pointer,
  a new struct ModMap is dynamically allocated before doing the copy
  and a pointer to the new structure is stored into *d, else the
  existing struct ModMap is overwritten.

  This function signals a fatal condition if the allocation of a new
  struct ModMap is required, and fails.

  This function always clears the caching information in the
  destination structure.

.call         :
                ReplaceModMap(d, s);
.input        :
                struct ModMap **d, ptr to destination structure ptr
                const struct ModMap *s, ptr to source structure
.output       :
                struct ModMap **d, ptr to destination structure ptr;
                updated when original value of *d was NULL
.status_codes :
                MOD_I_CALLED
                MOD_I_PERF_CTR, performance counter: %s value: %d
                MOD_F_MAP_ALLOC, allocation of new map failed
.notes        :
  2.7, 15-Sep-2000, creation

.- */
static void ReplaceModMap( struct ModMap** d, const struct ModMap* s )
{
    debug1( MOD_CHF_MODULE_ID, DEBUG_C_TRACE, MOD_I_CALLED, "ReplaceModMap" );

    if ( *d == ( struct ModMap* )NULL )
        /* Allocation needed; cache cleared after allocation */
        *d = CopyModMap( NewModMap(), s );
    else {
        CopyModMap( *d, s );
        IncPerfCtr( repl_c );
    }
}

/* .+

.creation     : 15-Sep-2000
.description  :
  This function flushes the whole cache, freeing all structures previously
  allocated by AllocModMap(), except that pointed by save.  This function
  also clears the caching information contained in save, because this
  information is no longer valid after the flush.

.call         :
                FlushCache(save);
.input        :
                struct ModMap *save
.output       :
                void
.status_codes :
                MOD_I_CALLED
                MOD_F_BAD_ALLOC_C, bad alloc_c (%d) after FlushCache()
.notes        :
  2.7, 15-Sep-2000, creation

.- */
static void FlushCache( struct ModMap* save )
{
    struct ModMap *p, *n;

    debug1( MOD_CHF_MODULE_ID, DEBUG_C_TRACE, MOD_I_CALLED, "FlushCache" );

    /* Scan the cache list; free all elements except that pointed by 'save' */
    p = cache_head;
    while ( p != ( struct ModMap* )NULL ) {

        n = p->cache.link;

        if ( p != save ) {
            free( p );
            DecPerfCtr( alloc_c );
        }

        p = n;
    }

    /* The cache list now contains only 'save' */
    save->cache.link = ( struct ModMap* )NULL;
    cache_head = save;

    /* Clear the caching information in 'save' */
    ClearCachingInfo( save );

    IncPerfCtr( flush_c );

#ifdef DEBUG
    /* The alloc_c performance counter must be exactly 1 now */
    if ( alloc_c != 1 ) {
        ChfGenerate( MOD_CHF_MODULE_ID, __FILE__, __LINE__, MOD_F_BAD_ALLOC_C, CHF_ERROR, alloc_c );
        ChfSignal( MOD_CHF_MODULE_ID );
    }
#endif
}

/* .+

.creation     : 15-Sep-2000
.description  :
  This function checks if there is an entry in the module configuration cache
  associated with the current struct ModMap with tag 'tag';
  if this is the case, it returns a pointer to the cached struct ModMap
  just found, otherwise it returns NULL (cache miss).

.call         :
                p = AccessConfigCache(tag);
.input        :
                Address tag, cache tag
.output       :
                struct ModMap *p, cached pointer, or NULL
.status_codes :
                MOD_I_CALLED
.notes        :
  2.7, 15-Sep-2000, creation

.- */
static struct ModMap* AccessConfigCache( Address tag )
{
    int i;

    debug1( MOD_CHF_MODULE_ID, DEBUG_C_TRACE, MOD_I_CALLED, "AccessConfigCache" );

    for ( i = 0; i < N_MOD_CACHE_ENTRIES; i++ )
        if ( MOD_MAP.cache.config[ i ].tag == tag )
            return MOD_MAP.cache.config[ i ].map_ptr;

    return ( struct ModMap* )NULL;
}

/* .+

.creation     : 15-Sep-2000
.description  :
  This function follows the .unconfig cache links with index i, starting
  from the current struct ModMap, until it finds a struct ModMap
  whose .cache.config_point is set, or stumbles into a NULL pointer.

  When successful, this function returns a pointer to the cached
  struct ModMap just found, otherwise it returns NULL (cache miss).

.call         :
                p = AccessUnconfigCache(i);
.input        :
                int i, unconfig cache index (unconfigured module index)
.output       :
                struct ModMap *p, cached pointer, or NULL
.status_codes :
                MOD_I_CALLED
.notes        :
  2.7, 15-Sep-2000, creation

.- */
static struct ModMap* AccessUnconfigCache( int i )
{
    struct ModMap* p;

    debug1( MOD_CHF_MODULE_ID, DEBUG_C_TRACE, MOD_I_CALLED, "AccessUnconfigCache" );

    p = MOD_MAP.cache.unconfig[ i ];

    while ( p != ( struct ModMap* )NULL && !p->cache.config_point )
        p = p->cache.unconfig[ i ];

    return p;
}

/* .+

.creation     : 19-Sep-2000
.description  :
  This function selects a victim entry in the module configuration
  cache table of the current struct ModMap, updates the victim selection
  info associated with the current map, and returns a pointer to the
  victim entry.

  If the search fails and the 'retry' argument is non-zero, this
  function signals a warning (MOD_W_NO_VICTIM), flushes the whole cache
  (by means of FlushCache()), and retries the search.  If even the
  second attempt fails, it signals a fatal condition (MOD_F_NO_VICTIM).

  If the search fails and the 'retry' argument is zero, this
  function immediately signals the fatal condition MOD_F_NO_VICTIM.

.call         :
                victim = SelectConfigVictim();
.input        :
                void
.output       :
                struct ModCacheTableEntry *victim, pointer to victim entry
.status_codes :
                MOD_I_CALLED
                MOD_W_NO_VICTIM
                MOD_F_NO_VICTIM
                MOD_F_BAD_ALLOC_C, bad alloc_c (%d) after FlushCache()
.notes        :
  2.7, 15-Sep-2000, creation

.- */
struct ModCacheTableEntry* SelectConfigVictim( int retry )
{
    int v = MOD_MAP.cache.victim;
    struct ModCacheTableEntry* victim = ( struct ModCacheTableEntry* )NULL;

    debug1( MOD_CHF_MODULE_ID, DEBUG_C_TRACE, MOD_I_CALLED, "AccessUnconfigCache" );

    /* Scan the config cache entries, starting at .cache.victim,
       until a suitable one is found or the index loops around
    */
    do {
        /* A config cache entry is suitable for use if:
           - it is empty (map_ptr == NULL)
           - or the reference count of the associated map is 0
        */
        if ( ( MOD_MAP.cache.config[ v ].map_ptr == ( struct ModMap* )NULL ) || MOD_MAP.cache.config[ v ].map_ptr->cache.ref_count == 0 )
            victim = &( MOD_MAP.cache.config[ v ] );

        v = ( v + 1 ) % N_MOD_CACHE_ENTRIES;
    } while ( victim == ( struct ModCacheTableEntry* )NULL && v != MOD_MAP.cache.victim );

    if ( victim == ( struct ModCacheTableEntry* )NULL ) {
        if ( retry ) {
            /* Unable to find a victim; flush the cache and retry */
            ChfGenerate( MOD_CHF_MODULE_ID, __FILE__, __LINE__, MOD_W_NO_VICTIM, CHF_WARNING );
            ChfSignal( MOD_CHF_MODULE_ID );

            FlushCache( mod_map_ptr );

            victim = SelectConfigVictim( 0 );
        } else {
            /* Unable to find a victim; retry is not an option; give up */
            ChfGenerate( MOD_CHF_MODULE_ID, __FILE__, __LINE__, MOD_F_NO_VICTIM, CHF_FATAL );
            ChfSignal( MOD_CHF_MODULE_ID );
        }
    } else
        /* Found a victim; update next-victim index */
        MOD_MAP.cache.victim = v;

    return victim;
}

/* .+

.creation     : 19-Sep-2000
.description  :
  This function checks if there is in the cache a struct ModMap
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
                struct ModMap *p, cached pointer, or NULL
.status_codes :
                MOD_I_CALLED
.notes        :
  2.7, 19-Sep-2000, creation

.- */
static struct ModMap* CheckForLateHit( void )
{
    struct ModMap* p;
    int i;

    debug1( MOD_CHF_MODULE_ID, DEBUG_C_TRACE, MOD_I_CALLED, "AccessUnconfigCache" );

    p = cache_head;

    /* Scan the cache to find an entry with the same modules configuration
       as the current one; return a pointer to it if successful
    */
    while ( p != ( struct ModMap* )NULL ) {
        /* Don't attempt to match an entry against itself */
        if ( p != mod_map_ptr ) {
            /* only .map_info contents must match */
            for ( i = 0; i < N_MOD; i++ ) {
                /* Break the for if a difference was found */
                if ( ( mod_map_ptr->map_info[ i ].config != p->map_info[ i ].config ) ||
                     ( mod_map_ptr->map_info[ i ].abs_base_addr != p->map_info[ i ].abs_base_addr ) ||
                     ( mod_map_ptr->map_info[ i ].size != p->map_info[ i ].size ) )
                    break;
            }

            /* Break the while if we found a match ('for' was not broken) */
            if ( i == N_MOD )
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
  This function frees the cached struct ModMap pointed by p, preserving
  cache list linkage.

  It is responsibility of the caller to ensure that the structure is no
  longer referenced.

.call         :
                p = CheckForLateHit();
.input        :
                void
.output       :
                struct ModMap *p, cached pointer, or NULL
.status_codes :
                MOD_I_CALLED
                MOD_F_CHAIN_CORRUPTED
.notes        :
  2.7, 15-Sep-2000, creation

.- */
static void FreeModMap( struct ModMap* p )
{
    struct ModMap* n;

    debug1( MOD_CHF_MODULE_ID, DEBUG_C_TRACE, MOD_I_CALLED, "FreeModMap" );

    /* Free the struct ModMap pointed by p, preserving the linkage of
       other entries.  The caller must ensure that the entry is not
       referenced by any other entry through cache pointers.
    */
    if ( p == cache_head ) {
        /* Free the list head */
        cache_head = p->cache.link;
        free( p );
        DecPerfCtr( alloc_c );
    } else {
        /* Scan the cache; at end, n is either null (!) or points to the
           cache entry that immediately precedes p
        */
        n = cache_head;
        while ( ( n != ( struct ModMap* )NULL ) && n->cache.link != p )
            n = n->cache.link;

        /* Should never happen */
        if ( n == ( struct ModMap* )NULL ) {
            ChfGenerate( MOD_CHF_MODULE_ID, __FILE__, __LINE__, MOD_F_CHAIN_CORRUPTED, CHF_FATAL );
            ChfSignal( MOD_CHF_MODULE_ID );
        }

        /* Bypass element pointed by p and free it */
        n->cache.link = p->cache.link;
        free( p );
        DecPerfCtr( alloc_c );
    }
}

/*---------------------------------------------------------------------------
        Public functions
  ---------------------------------------------------------------------------*/

/* .+

.creation     : 21-Sep-2000
.description  :
  This function registers the ModDescription table pointed by 'p'; all
  other module emulation functions will refer to that table in the future.

  It is mandatory to invoke this function with a valid ModDescription
  table pointer as argument *before* using any other module emulation
  function, either directly or indirectly.

  All error conditions are signalled using the Chf facility; the function
  returns 'void' to the caller.

.call         :
                ModRegisterDescription(p);
.input        :
                ModDescription p, module description table to be registered
.output       :
                void
.status_codes :
                MOD_I_CALLED

.notes        :
  3.2, 21-Sep-2000, creation

.- */
void ModRegisterDescription( ModDescription p )
{
    debug1( MOD_CHF_MODULE_ID, DEBUG_C_TRACE, MOD_I_CALLED, "ModRegisterDescription" );
    mod_description = p;
}

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
                ModInit();
.input        :
                void
.output       :
                void
.status_codes :
                MOD_I_CALLED
                MOD_I_REVISION
                MOD_I_INITIALIZING
                MOD_I_PERF_CTR, performance counter: %s value: %d
                MOD_W_RESETTING_ALL
                MOD_F_MAP_ALLOC
                MOD_F_NO_DESCRIPTION

                NOTE: This function can also (indirectly) report any condition
                code generated and/or signalled by the module initialization
                functions.
.notes        :
  1.1, 23-Jan-1998, creation
  2.7, 15-Sep-2000, update
    - revised to implement module config/unconfig cache
  3.2, 21-Sep-2000, update
    - added sanity check on mod_description

.- */
void ModInit( void )
{
    int mod;

    debug1( MOD_CHF_MODULE_ID, DEBUG_C_TRACE, MOD_I_CALLED, "ModInit" );

    /* First, a little sanity check on mod_description: ensure that
       ModRegisterDescription() has been called at least once with a
       non-NULL argument.
    */
    if ( mod_description == NULL ) {
        ChfGenerate( MOD_CHF_MODULE_ID, __FILE__, __LINE__, MOD_F_NO_DESCRIPTION, CHF_FATAL );
        ChfSignal( MOD_CHF_MODULE_ID );
    }

    /* Scan the mod_description table, initializing all modules */
    for ( mod = 0; mod < N_MOD; mod++ ) {
        debug1( MOD_CHF_MODULE_ID, DEBUG_C_MODULES, MOD_I_INITIALIZING, mod_description[ mod ].name );
        mod_description[ mod ].init();
    }

    /* Allocate the root struct ModMap and set it as the current one;
       the structure can be accessed using either mod_map_ptr or MOD_MAP.
    */
    mod_map_ptr = ClearCachingInfo( NewModMap() );

    /* Attempt to restore the mod_map from file; reset modules if the read
       fails.
    */
    if ( ReadStructFromFile( config.mod_path, sizeof( MOD_MAP.map_info ), &MOD_MAP.map_info ) ) {
        ChfGenerate( MOD_CHF_MODULE_ID, __FILE__, __LINE__, MOD_W_RESETTING_ALL, CHF_WARNING );
        ChfSignal( MOD_CHF_MODULE_ID );

        /* Reset all modules */
        ModReset();
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
                ModSave();
.input        :
                void
.output       :
                void
.status_codes :
                MOD_I_CALLED
                MOD_I_SAVING
                MOD_W_RESETTING

                NOTE: This function can also (indirectly) report any condition
                code generated and/or signalled by the module initialization
                functions.
.notes        :
  1.1, 11-Feb-1998, creation

.- */
void ModSave( void )
{
    int mod;

    debug1( MOD_CHF_MODULE_ID, DEBUG_C_TRACE, MOD_I_CALLED, "ModSave" );

    /* Scan the mod_description table, initializing all modules */
    for ( mod = 0; mod < N_MOD; mod++ ) {
        debug1( MOD_CHF_MODULE_ID, DEBUG_C_MODULES, MOD_I_SAVING, mod_description[ mod ].name );
        mod_description[ mod ].save();
    }

    /* Attempt to save the mod_map from file */
    if ( WriteStructToFile( &MOD_MAP.map_info, sizeof( MOD_MAP.map_info ), config.mod_path ) ) {
        ChfGenerate( MOD_CHF_MODULE_ID, __FILE__, __LINE__, MOD_F_MAP_SAVE, CHF_FATAL );
        ChfSignal( MOD_CHF_MODULE_ID );
    }
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
                MOD_I_CALLED
                MOD_I_GET_ID
.notes        :
  1.1, 26-Jan-1998, creation

.- */
Address ModGetID( void )
{
    int mod;
    Address id;

    debug1( MOD_CHF_MODULE_ID, DEBUG_C_TRACE, MOD_I_CALLED, "ModGetID" );

    /* Scan the module information table searching for either an unconfigured
       or a partially configured module
    */
    for ( mod = 0; mod < N_MOD && MOD_MAP.map_info[ mod ].config == MOD_CONFIGURED; mod++ )
        ;

    if ( mod == N_MOD )
        /* All modules are configured */
        id = ( Address )0x00000;
    else
        /* Build the module id */
        id = ( MOD_MAP.map_info[ mod ].abs_base_addr & 0xFFF00 ) |
             ( MOD_MAP.map_info[ mod ].config == MOD_UNCONFIGURED ? 0x00000 : 0x000F0 ) |
             ( mod_description[ mod ].id + ( MOD_MAP.map_info[ mod ].config == MOD_UNCONFIGURED ? 0 : 1 ) );

    debug1( MOD_CHF_MODULE_ID, DEBUG_C_MODULES, MOD_I_GET_ID, id );
    return id;
}

/* .+

.creation     : 26-Jan-1998
.description  :
  This function resets all peripheral modules and rebuilds the module page
  table used for module access.

.call         :
                ModReset();
.input        :
                void
.output       :
                void
.status_codes :
                MOD_I_CALLED
                MOD_I_RESETTING
                MOD_F_BAD_ALLOC_C, bad alloc_c (%d) after FlushCache()
.notes        :
  1.1, 26-Jan-1998, creation
  2.7, 15-Sep-2000, update
    - revised to implement module config/unconfig cache

.- */
void ModReset( void )
{
    int mod;

    debug1( MOD_CHF_MODULE_ID, DEBUG_C_TRACE, MOD_I_CALLED, "ModReset" );

    /* Scan the mod_description table, initializing the module
       mapping information MOD_MAP.map_info.
    */
    for ( mod = 0; mod < N_MOD; mod++ ) {
        debug1( MOD_CHF_MODULE_ID, DEBUG_C_MODULES, MOD_I_RESETTING, mod_description[ mod ].name );

        /* Set the module configuration status */
        MOD_MAP.map_info[ mod ].config = mod_description[ mod ].r_config;
        MOD_MAP.map_info[ mod ].abs_base_addr = mod_description[ mod ].r_abs_base_addr;
        MOD_MAP.map_info[ mod ].size = mod_description[ mod ].r_size;
    }

    /* Rebuild the module page table */
    RebuildPageTable( 0, N_PAGE_TABLE_ENTRIES - 1 );

    /* Flush the whole struct ModMap cache, preserving the current map */
    FlushCache( mod_map_ptr );

    /* Mark the current struct ModMap to be a configuration point;
       this flag is used by the unconfig cache code to correctly
       undo the last config
    */
    MOD_MAP.cache.config_point = 1;
}

/* .+

.creation     : 26-Jan-1998
.description  :
  This function configures a module, using the given 'config_info'.

  The target module will be the first unconfigured or partially configured
  module found in the MOD_MAP.map_info table.

  If the target module is unconfigured, ModConfig sets the size of its
  address space to 0x100000 - 'config_info'; the module then becomes
  partially configured.

  If the target module is already partially configured, ModConfig sets
  its base address to 'config_info', completing the configuration process.

  In the latter case, ModConfig rebuilds the page table used for module access
  to reflect the visibility of the new module in the CPU address space.

.call         :
                void ModConfig(config_info);
.input        :
                Address config_info, configuration information
.output       :
                void
.status_codes :
                MOD_I_CALLED
                MOD_I_CONFIG
                MOD_I_CACHED_CONFIG
                MOD_I_PERF_CTR, performance counter: %s value: %d
                MOD_W_BAD_CONFIG
                MOD_W_NO_VICTIM
                MOD_F_MAP_ALLOC
                MOD_F_NO_VICTIM
.notes        :
  1.1, 26-Jan-1998, creation
  2.7, 15-Sep-2000, update
    - implemented module config/unconfig cache
  3.2, 22-Sep-2000, update
    - enabled forced alignment
      of config_info
.- */
void ModConfig( Address config_info )
{
    struct ModMap *old, *nxt;
    struct ModCacheTableEntry* victim;

    int mod;

    debug1( MOD_CHF_MODULE_ID, DEBUG_C_TRACE, MOD_I_CALLED, "ModConfig" );

    /* 3.2: The HP49 firmware (1.19-4) can generate misaligned config
            addresses, that is, addresses that are not a multiple of 0x100;
            silently align them here.
    */
    config_info &= ~0xFF;

    /* ACCESS CONFIG CACHE */
    if ( ( nxt = AccessConfigCache( config_info ) ) != ( struct ModMap* )NULL ) {
        /* CACHE HIT; switch mod_map_ptr */
        mod_map_ptr = nxt;

        IncPerfCtr( hit_c );

        debug1( MOD_CHF_MODULE_ID, DEBUG_C_MOD_CACHE, MOD_I_CACHED_CONFIG, config_info );
        return;
    }

    /* CACHE MISS */
    IncPerfCtr( miss_c );

    /* Select a 'victim' cache table entry and update victim
       selection info; retry after flushing the cache if necessary.

       Initialize victim's tag to current config_info.

       Initialize victim's map_ptr; this can be either a new
       allocation if the pointer was NULL, or a replacement.
       This clears the caching information of the map_ptr's pointee.

       Switch mod_map_ptr to the new structure

       The unconfig cache pointer of the new structure will be set when
       the index of the module being configured will be known.
    */
    victim = SelectConfigVictim( 1 );

    victim->tag = config_info;
    ReplaceModMap( &( victim->map_ptr ), mod_map_ptr );

    old = mod_map_ptr;
    mod_map_ptr = victim->map_ptr;

    /* Scan the module information table searching for either an unconfigured
       or a partially configured module
    */
    for ( mod = 0; mod < N_MOD && MOD_MAP.map_info[ mod ].config == MOD_CONFIGURED; mod++ )
        ;

    if ( mod == N_MOD ) {
        /* All modules are configured - Signal a warning */
        // FIXME: 48gx bugs here when running VERSION
        ChfGenerate( MOD_CHF_MODULE_ID, __FILE__, __LINE__, MOD_W_BAD_CONFIG, CHF_WARNING, config_info );
        ChfSignal( MOD_CHF_MODULE_ID );
    } else {
        if ( MOD_MAP.map_info[ mod ].config == MOD_UNCONFIGURED ) {
            /* The module was unconfigured; configure its size */
            MOD_MAP.map_info[ mod ].size = 0x100000 - config_info;
            MOD_MAP.map_info[ mod ].config = MOD_SIZE_CONFIGURED;
        } else {
            /* The module size was already configured; configure its base address */
            MOD_MAP.map_info[ mod ].abs_base_addr = config_info;
            MOD_MAP.map_info[ mod ].config = MOD_CONFIGURED;

            /* Rebuild the page table */
            RebuildPageTable( ModPage( MOD_MAP.map_info[ mod ].abs_base_addr ),
                              ModPage( MOD_MAP.map_info[ mod ].abs_base_addr + MOD_MAP.map_info[ mod ].size - 1 ) );

            /* Mark the current struct ModMap to be a configuration point;
               this flag is used by the unconfig cache code to correctly
               undo the last config
            */
            MOD_MAP.cache.config_point = 1;

            debug3( MOD_CHF_MODULE_ID, DEBUG_C_MODULES | DEBUG_C_MOD_CACHE, MOD_I_CONFIG, mod_description[ mod ].name,
                    MOD_MAP.map_info[ mod ].abs_base_addr, MOD_MAP.map_info[ mod ].size );
        }

        /* Set the unconfig cache pointer of module 'mod' to the old ModMap,
           and increment its reference counter, to avoid freeing it
           improperly.
        */
        MOD_MAP.cache.unconfig[ mod ] = old;
        old->cache.ref_count++;
    }
}

/* .+

.creation     : 26-Jan-1998
.description  :
  This function unconfigures the module currently configured at address
  'unconfig_info' and returns it to its after-reset configuration status.

  ModUnconfig also rebuilds the page table used for module access
  to reflect the loss of visibility of the module in the CPU address space.

.call         :
                ModUnconfig(unconfig_info);
.input        :
                Address unconfig_info, Unconfig information
.output       :
                void
.status_codes :
                MOD_I_CALLED
                MOD_I_UNCONFIG
                MOD_I_CACHED_UNCONFIG
                MOD_I_PERF_CTR, performance counter: %s value: %d
                MOD_W_BAD_UNCONFIG
                MOD_F_MAP_ALLOC
                MOD_F_CHAIN_CORRUPTED
.notes        :
  1.1, 26-Jan-1998, creation
  2.7, 15-Sep-2000, update
    - implemented module config/unconfig cache

.- */
void ModUnconfig( Address unconfig_info )
{
    struct ModMap *nxt, *old;
    int mod;

    debug1( MOD_CHF_MODULE_ID, DEBUG_C_TRACE, MOD_I_CALLED, "ModUnconfig" );

    /* Determine the module to unconfigure */
    if ( ( mod = MOD_MAP.page_table[ ModPage( unconfig_info ) ].index ) == MOD_NO_MOD_INDEX ) {
        /* There isn't any module configured at the given address -
           Signal a warning
        */
        ChfGenerate( MOD_CHF_MODULE_ID, __FILE__, __LINE__, MOD_W_BAD_UNCONFIG, CHF_WARNING, unconfig_info );
        ChfSignal( MOD_CHF_MODULE_ID );
    } else if ( mod_description[ mod ].r_config == MOD_CONFIGURED ) {
        /* The module is automatically configured after reset; it can never
           be unconfigured.
        */
    } else {
        /* Unconfiguring module 'mod': ACCESS UNCONFIG CACHE */
        if ( ( nxt = AccessUnconfigCache( mod ) ) != ( struct ModMap* )NULL ) {
            /* CACHE HIT; switch mod_map_ptr */
            mod_map_ptr = nxt;

            IncPerfCtr( hit_c );

            debug0( MOD_CHF_MODULE_ID, DEBUG_C_MOD_CACHE, MOD_I_CACHED_UNCONFIG );
            return;
        }

        /* CACHE MISS

           A clone of the current struct ModMap is allocated and updated
           according to the unconfig instruction being executed.

           Then, CheckForLateHit() is called to check whether in the
           module mapping cache there is a struct ModMap identical to
           the updated one.

           - If there is, the .unconfig[i] link is updated to point to
             the cache entry just found.

           - If there is not, the whole cache is flushed and all cached
             ModMap structures allocated so far are freed, except the
             current one.  I hope this occurrence is rare.
        */

        /* Save pointer to the old map and switch to a temporary one */
        old = mod_map_ptr;
        mod_map_ptr = CopyModMap( NewModMap(), mod_map_ptr );

        /* Update the mapping information table */
        MOD_MAP.map_info[ mod ].config = mod_description[ mod ].r_config;

        /* Rebuild the page table */
        RebuildPageTable( ModPage( MOD_MAP.map_info[ mod ].abs_base_addr ),
                          ModPage( MOD_MAP.map_info[ mod ].abs_base_addr + MOD_MAP.map_info[ mod ].size - 1 ) );

        /* Reset the module configuration status; the abs_base_addr of the module
           is not reset because its old value is still needed by ModGetId()
           The size is reset for the modules that are already MOD_SIZE_CONFIGURED
           immediately after reset.
        */
        MOD_MAP.map_info[ mod ].size = mod_description[ mod ].r_size;

        if ( ( nxt = CheckForLateHit() ) != ( struct ModMap* )NULL ) {
            /* Update pointer from the old map to the new one, and increment
               reference counter of the referenced structure
            */
            old->cache.unconfig[ mod ] = nxt;
            nxt->cache.ref_count++;

            /* Discard the temporary map and switch to the cached one */
            FreeModMap( mod_map_ptr );
            mod_map_ptr = nxt;

            IncPerfCtr( lhit_c );
            debug0( MOD_CHF_MODULE_ID, DEBUG_C_MOD_CACHE, MOD_I_UNCONFIG_L_HIT );
        } else {
            /* Continue to use the new map with no caching information,
               and hope that further configuration activities will link it
               back in the immediate future.
            */

            /* Mark the current struct ModMap to be a configuration point;
               this flag is used by the unconfig cache code to correctly
               undo the last config
            */
            MOD_MAP.cache.config_point = 1;

            IncPerfCtr( miss_c );

            debug0( MOD_CHF_MODULE_ID, DEBUG_C_MOD_CACHE, MOD_I_UNCONFIG_L_MISS );

            debug3( MOD_CHF_MODULE_ID, DEBUG_C_MODULES | DEBUG_C_MOD_CACHE, MOD_I_UNCONFIG, mod_description[ mod ].name,
                    MOD_MAP.map_info[ mod ].abs_base_addr, MOD_MAP.map_info[ mod ].size );
        }
    }
}

/* .+

.creation     : 26-Jan-1998
.description  :
  This function fetches a nibble from the address 'addr' and returns it.

  NOTE: This function DOES NOT update the hardware CRC register.

.call         :
                d = FetchNibble(addr);
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
Nibble FetchNibble( Address addr )
{
    register int page = ModPage( addr );

    return MOD_MAP.page_table[ page ].read( MOD_MAP.page_table[ page ].rel_base_addr | ModOffset( addr ) );
}

/* .+

.creation     : 26-Jan-1998
.description  :
  This function reads a nibble from the address 'addr' and returns it.

  NOTE: This function updates the hardware CRC register if the target of the
        read operation is not the HDW module. The current (1.1) implementation
        of this feature is inefficient because the .index field of the
        addressed page must be checked against MOD_HDW_INDEX for each
        access.

.call         :
                d = ReadNibble(addr);
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
Nibble ReadNibble( Address addr )
{
    register int page = ModPage( addr );
    register Nibble d;

    /* Read the nibble from the peripheral module */
    d = MOD_MAP.page_table[ page ].read( MOD_MAP.page_table[ page ].rel_base_addr | ModOffset( addr ) );

    /* Update the crc register, if appropriate */
    if ( MOD_MAP.page_table[ page ].index != MOD_HDW_INDEX )
        mod_status.hdw.crc = ( mod_status.hdw.crc >> 4 ) ^ ( ( ( mod_status.hdw.crc ^ d ) & 0x0F ) * 0x1081 );

    /* Return to the caller */
    return d;
}

/* .+

.creation     : 26-Jan-1998
.description  :
  This function writes the nibble 'datum' to the address 'addr'

  NOTE: This function DOES NOT update the hardware CRC register.

.call         :
                WriteNibble(addr, datum);
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
void WriteNibble( Address addr, Nibble datum )
{
    register int page = ModPage( addr );

    MOD_MAP.page_table[ page ].write( MOD_MAP.page_table[ page ].rel_base_addr | ModOffset( addr ), datum );
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
                ModMapCheck(addr, char ob[MOD_MAP_CHECK_OB_SIZE]);
.input        :
                Address addr;
.output       :
                void
.status_codes :
                *
.notes        :
  1.1, 26-Jan-1998, creation

.- */
void ModMapCheck( Address addr, char ob[ MOD_MAP_CHECK_OB_SIZE ] )
{
    int page;
    Address offset;
    int mod;

    page = ModPage( addr );
    offset = ModOffset( addr );

    if ( ( mod = MOD_MAP.page_table[ page ].index ) == MOD_NO_MOD_INDEX )
        sprintf( ob, "A[%05X] -> *Not Mapped*", addr );
    else {
        Address rel_addr;
        rel_addr = MOD_MAP.page_table[ page ].rel_base_addr | offset;

        sprintf( ob, "A[%05X] -> M[%s] R[%05X]", addr, mod_description[ mod ].name, rel_addr );
    }

    ChfSignal( MOD_CHF_MODULE_ID );
}

/* .+

.creation     : 26-Jan-1998
.description  :
  This function fills the string 'ob' with the current mapping table for
  all modules; it is used by the emulator monitor only.

.call         :
                ModMapTable(char ob[MOD_MAP_TABLE_OB_SIZE]);
.input        :
                Address addr;
.output       :
                void
.status_codes :
                *
.notes        :
  1.1, 26-Jan-1998, creation

.- */
void ModMapTable( char ob[ MOD_MAP_TABLE_OB_SIZE ] )
{
    int mod;

    sprintf( ob, "%s\n", "Device\t\t\tAddress\tSize\tStatus" );
    ob += strlen( ob );

    for ( mod = 0; mod < N_MOD; mod++ ) {
        sprintf( ob, "%s\t%05X\t%05X\t%s", mod_description[ mod ].name, MOD_MAP.map_info[ mod ].abs_base_addr, MOD_MAP.map_info[ mod ].size,
                 MOD_MAP.map_info[ mod ].config == MOD_CONFIGURED
                     ? "Configured"
                     : ( MOD_MAP.map_info[ mod ].config == MOD_SIZE_CONFIGURED ? "Size_configured" : "*Unconfigured*" ) );

        strcat( ob, "\n" );
        ob += strlen( ob );
    }
}
