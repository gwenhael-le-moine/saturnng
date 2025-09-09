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

.identifier   : $Id: monitor.c,v 4.1 2000/12/11 09:54:19 cibrario Rel $
.context      : SATURN, Saturn CPU / HP48 emulator
.title        : $RCSfile: monitor.c,v $
.kind         : C source
.author       : Ivan Cibrario B.
.site         : CSTV-CNR
.creation     :	28-Jan-1998
.keywords     : *
.description  :
  This file implements a simple interactive monitor, useful during
  test & debug.

.include      : config.h, machdep.h, cpu.h

.notes        :
  $Log: monitor.c,v $
  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.10  2000/10/24 16:14:51  cibrario
  Added/Replaced GPL header

  Revision 3.1  2000/09/20 13:52:42  cibrario
  Minor updates and fixes to avoid gcc compiler warnings on Solaris
  when -ansi -pedantic -Wall options are selected.

 * Revision 1.1  1998/02/18  11:49:53  cibrario
 * Initial revision
 *

.- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>


#include "../libChf/src/Chf.h"
#include "../options.h"

#include "config.h"
#include "cpu.h"
#include "modules.h"

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
    int st;
    if ( ( st = ReadHexAddress( &addr ) ) == OK )
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
    if ( ReadHexAddress( &addr ) )
        return FAILED;
    config.debug_level |= ( int )addr;
    return OK;
}

/* Check the mapping of an Address and print */
static int map_check( void )
{
    Address addr;
    char ob[ MOD_MAP_CHECK_OB_SIZE ];
    if ( ReadHexAddress( &addr ) )
        return FAILED;
    ModMapCheck( addr, ob );
    puts( ob );
    return OK;
}

/* Print the current module map table */
static int map( void )
{
    char ob[ MOD_MAP_TABLE_OB_SIZE ];
    ModMapTable( ob );
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
    while ( ReadHexDatum( &n ) == OK )
        WriteNibble( addr++, n );
    return OK;
}

/* Read nibbles from memory */
static int r( void )
{
    Address addr;
    int count;
    if ( ReadHexAddress( &addr ) )
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

    if ( ReadHexAddress( &addr ) )
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
static int cpu( void )
{
    char ob[ DUMP_CPU_STATUS_OB_SIZE ];
    DumpCpuStatus( ob );
    puts( ob );
    return OK;
}

/* Reset CPU */
static int reset( void )
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
    {"cpu",   "Print CPU status",                                 cpu      },
    {"map",   "Print the contents of the module map table",       map      },
    {"debug", "Set the debugging level",                          debug    },
    {"reset", "Reset CPU",                                        reset    },
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

.title        : Monitor
.kind         : C function
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
    char old_cmd[ LINE_BUFFER_SIZE ];
    char* tk;

    /* Clear old_cmd buffer */
    strcpy( old_cmd, "" );

    /* Establish SIGINT handler */
    signal( SIGINT, sigint_handler );

    /* Infinite loop; it's exited only when a condition is signalled */
    while ( true ) {
        /* Write prompt */
        fputs( PROMPT, stdout );
        fflush( stdout );

        if ( fgets( cmd, LINE_BUFFER_SIZE, stdin ) == ( char* )NULL || ( tk = strtok( cmd, TOK_DELIMITERS ) ) == ( char* )NULL ) {
            /* New command empty; try old command */
            if ( ( tk = strtok( old_cmd, TOK_DELIMITERS ) ) != ( char* )NULL )
                if ( InvokeCommand( tk ) )
                    ChfGenerate( CPU_CHF_MODULE_ID, __FILE__, __LINE__, CPU_W_BAD_MONITOR_CMD, CHF_WARNING, tk );
            ChfSignal( CPU_CHF_MODULE_ID );
        } else {
            /* Save command */
            strcpy( old_cmd, cmd );

            /* New command */
            if ( InvokeCommand( tk ) ) {
                ChfGenerate( CPU_CHF_MODULE_ID, __FILE__, __LINE__, CPU_W_BAD_MONITOR_CMD, CHF_WARNING, tk );
                ChfSignal( CPU_CHF_MODULE_ID );
            }
        }
    }
}
