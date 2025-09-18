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
  This file implements a simple interactive monitor, useful during
  test & debug.

.notes        :
  $Log: monitor.c,v $
  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.10  2000/10/24 16:14:51  cibrario
  Added/Replaced GPL header

  Revision 3.1  2000/09/20 13:52:42  cibrario
  Minor updates and fixes to avoid gcc compiler warnings on Solaris
  when -ansi -pedantic -Wall options are selected.

  Revision 1.1  1998/02/18  11:49:53  cibrario
  Initial revision
.- */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../options.h"

#include "chf_wrapper.h"
#include "cpu.h"
#include "disassembler.h"
#include "emulator.h"
#include "modules.h"
#include "monitor.h"

/*---------------------------------------------------------------------------
        Macro & Data type definitions
  ---------------------------------------------------------------------------*/

#define LINE_BUFFER_SIZE 512
#define TOK_DELIMITERS " \t\n"
#define ADDRESS_FMT "%x"
#define COUNT_FMT "%d"
#define PROMPT "> "
#define OK 0
#define FAILED 1

/*---------------------------------------------------------------------------
        Private functions - Command line parse
  ---------------------------------------------------------------------------*/

/* Read an Address from the command line */
static int ReadHexAddress( Address* addr )
{
    char* p = strtok( ( char* )NULL, TOK_DELIMITERS );
    return ( p == ( char* )NULL || sscanf( p, ADDRESS_FMT, ( unsigned int* )addr ) != 1 ) ? FAILED : OK;
}

/* Read a Nibble from the command line */
static int ReadHexDatum( Nibble* n )
{
    Address addr;
    int st = ReadHexAddress( &addr );

    if ( st == OK )
        *n = ( Nibble )addr;

    return st;
}

/* Read a DECIMAL count from the command line */
static int ReadCount( int* count )
{
    char* p = strtok( ( char* )NULL, TOK_DELIMITERS );

    return ( p == ( char* )NULL || sscanf( p, COUNT_FMT, count ) != 1 || *count <= 0 ) ? FAILED : OK;
}

/*---------------------------------------------------------------------------
        Private functions: dump
  ---------------------------------------------------------------------------*/
/* CPU */
static const char* DumpR( Nibble* r )
{
    static char b[ NIBBLE_PER_REGISTER + 1 ];
    static const char hex_char[ NIBBLE_PER_REGISTER ] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

    for ( int n = 0; n < NIBBLE_PER_REGISTER; n++ )
        b[ n ] = hex_char[ ( int )r[ NIBBLE_PER_REGISTER - 1 - n ] ];

    b[ NIBBLE_PER_REGISTER ] = '\0';

    return b;
}

/* .+

.creation     : 3-Feb-1998
.description  :
  This function dumps the current CPU status into the string buffer 'ob'.

.call         :
                DumpCpu(ob);
.input        :
                void
.output       :
                char ob[DUMP_CPU_OB_SIZE];
.status_codes :
                *
.notes        :
  1.1, 3-Feb-1998, creation

.- */
void DumpCpu( char ob[ DUMP_CPU_OB_SIZE ] )
{
    static const char* work_n[ N_WORKING_REGISTER ] = { "A", "B", "C", "D" };
    char dob[ DISASSEMBLE_OB_SIZE ];
    int n;

    /* Dump PC and current instruction */
    ( void )Disassemble( cpu.PC, dob );
    sprintf( ob, "\n%s\n\n", dob );
    ob += strlen( ob );

    /* Dump A, B, C, D */
    for ( n = 0; n < N_WORKING_REGISTER; n++ ) {
        sprintf( ob, "%s:\t%s\n", work_n[ n ], DumpR( cpu.work[ n ] ) );
        ob += strlen( ob );
    }

    sprintf( ob, "\n" );
    ob += strlen( ob );

    /* Dump Rn */
    for ( n = 0; n < N_SCRATCH_REGISTER; n++ ) {
        sprintf( ob, "R%d:\t%s\n", n, DumpR( cpu.R[ n ] ) );
        ob += strlen( ob );
    }

    sprintf( ob, "\n" );
    ob += strlen( ob );

    sprintf( ob, "D0:\t%05X\t\tD1:\t%05X\n", cpu.D0, cpu.D1 );
    ob += strlen( ob );

    sprintf( ob, "P:\t%01X\t\tIN:\t%04X\t\tOUT:\t%03X\n", cpu.P, cpu.IN, cpu.OUT );
    ob += strlen( ob );

    sprintf( ob, "HST:\t%01X\t\tST:\t%04X\n", cpu.HST, cpu.ST );
    ob += strlen( ob );

    sprintf( ob, "hexmode: %d, carry: %d, int_enable/pending/service: %d/%d/%d, shutdn:%d\n", cpu.hexmode, cpu.carry, cpu.int_enable,
             cpu.int_pending, cpu.int_service, cpu.shutdn );
    ob += strlen( ob );
}

/*---------------------------------------------------------------------------
        Private functions - Command execution
  ---------------------------------------------------------------------------*/

/* Run the emulator; this function exits normally only when an
   EmulatorIntRequest() is posted and satisfied
*/
static int run( void )
{
    Emulator();

    return OK;
}

/* Set the debug level */
static int debug( void )
{
    Address addr;
    int st = ReadHexAddress( &addr );

    if ( st )
        return FAILED;

    config.debug_level |= ( int )addr;

    return OK;
}

/* Check the mapping of an Address and print */
static int map_check( void )
{
    Address addr;
    int st = ReadHexAddress( &addr );

    if ( st )
        return FAILED;

    char ob[ MOD_MAP_CHECK_OB_SIZE ];
    monitor_ModMapCheck( addr, ob );
    puts( ob );

    return OK;
}

/* Print the current module map table */
static int map( void )
{
    char ob[ MOD_MAP_TABLE_OB_SIZE ];
    monitor_ModMapTable( ob );
    puts( ob );

    return OK;
}

/* Write nibbles into memory */
static int w( void )
{
    Address addr;
    Nibble n;
    if ( ReadHexAddress( &addr ) )
        return FAILED;
    while ( ReadHexDatum( &n ) == OK ) {
        WriteNibble( addr, n );
        addr++;
    }
    return OK;
}

/* Read nibbles from memory */
static int r( void )
{
    Address addr;
    int count;
    int st = ReadHexAddress( &addr );

    if ( st )
        return FAILED;

    if ( ReadCount( &count ) )
        count = 1;

    while ( count-- > 0 ) {
        printf( "A_%05X\t%X\n", addr, ( int )FetchNibble( addr ) );
        addr++;
    }

    return OK;
}

/* Disassemble */
static int d( void )
{
    Address addr;
    int count;
    char ob[ DISASSEMBLE_OB_SIZE ];

    int st = ReadHexAddress( &addr );

    if ( st )
        return FAILED;

    if ( ReadCount( &count ) )
        count = 1;

    while ( count-- > 0 ) {
        addr = Disassemble( addr, ob );
        puts( ob );
    }

    return OK;
}

/* Print CPU status */
static int show_cpu( void )
{
    char ob[ DUMP_CPU_OB_SIZE ];
    DumpCpu( ob );
    puts( ob );

    return OK;
}

/* Reset CPU */
static int reset_cpu( void )
{
    CpuReset();

    return OK;
}

/* Save & Exit */
static int mon_exit( void )
{
    ModSave();
    CpuSave();
    exit( EXIT_SUCCESS );

    return OK; /* 3.1: Keep compiler happy */
}

/* Quit without saving */
static int mon_quit( void )
{
    exit( EXIT_SUCCESS );

    return OK; /* 3.1: Keep compiler happy */
}

/*---------------------------------------------------------------------------
        Command table
  ---------------------------------------------------------------------------*/

struct TEntry {
    const char* name;
    const char* desc;
    int ( *function )( void );
};

#define TABLE_SIZE( t ) ( sizeof( t ) / sizeof( struct TEntry ) )

/* Forward declaration for the Help funcion */
static int Help( void );

static const struct TEntry table[] = {
    {"help",  "Print this information",                           Help     },
    {"run",   "Run the emulator with current CPU status",         run      },
    {"?",     "<addr>, Check address mapping",                    map_check},
    {"r",     "<addr> [count], Read nibbles from memory",         r        },
    {"w",     "<addr> [n]..., Write nibbles into memory",         w        },
    {"d",     "<addr> [count], Disassemble starting from 'addr'", d        },
    {"cpu",   "Print CPU status",                                 show_cpu },
    {"map",   "Print the contents of the module map table",       map      },
    {"debug", "Set the debugging level",                          debug    },
    {"reset", "Reset CPU",                                        reset_cpu},
    {"exit",  "Save emulator state & exit",                       mon_exit },
    {"quit",  "Quit emulator WITHOUT saving its state",           mon_quit }
};

/* Invoke the command 'tk' and return a status code */
static int InvokeCommand( char* tk )
{
    int i;
    for ( i = 0; i < ( int )TABLE_SIZE( table ) && strcmp( tk, table[ i ].name ); i++ )
        ;
    return i == TABLE_SIZE( table ) ? FAILED : table[ i ].function();
}

/* Print help information */
static int Help( void )
{
    for ( int i = 0; i < ( int )TABLE_SIZE( table ); i++ )
        printf( "%s\t\t%s\n", table[ i ].name, table[ i ].desc );

    return OK;
}

/* Handler for SIGINT during monitor execution */
static void sigint_handler( int s ) { EmulatorIntRequest(); }

/*---------------------------------------------------------------------------
        Public functions
  ---------------------------------------------------------------------------*/

/* .+

.creation     : 18-Feb-1998
.description  :
  This function implements a very simple interactive monitor.

.call         :
                Monitor();
.input        :
                void
.output       :
                void
.status_codes :
                CPU_W_BAD_MONITOR_CMD
                From lower level modules
.notes        :
  1.1, 18-Feb-1998, creation

.- */
void Monitor( void )
{
    char cmd[ LINE_BUFFER_SIZE ];
    char* tk;
    char* ret;

    /* Establish SIGINT handler */
    signal( SIGINT, sigint_handler );

    /* Infinite loop; it's exited only when a condition is signalled */
    while ( true ) {
        /* Write prompt */
        fputs( PROMPT, stdout );
        fflush( stdout );

        ret = fgets( cmd, LINE_BUFFER_SIZE, stdin );
        if ( ret == ( char* )NULL )
            continue;

        tk = strtok( cmd, TOK_DELIMITERS );
        if ( tk == ( char* )NULL )
            continue;

        bool err = InvokeCommand( tk );
        if ( err )
            WARNING( CPU_CHF_MODULE_ID, CPU_W_BAD_MONITOR_CMD, tk )
    }
}
