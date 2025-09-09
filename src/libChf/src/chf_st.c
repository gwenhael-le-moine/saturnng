/* .+

.author	      : Ivan Cibrario B.

.creation     :	24-May-1996

.description  :
  This module implements the CHF initialization function ChfStaticInit()

.notes	      :
  $Log: chf_st.c,v $
  Revision 2.2  2001/01/25 14:08:45  cibrario
  Added partial Win32 support (Windows CE only).

  Revision 1.1  1996/05/28  12:56:14  cibrario
  Initial revision
.- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Chf.h"
#include "ChfPriv.h"

/* -------------------------------------------------------------------------
   Private type definitions
   ------------------------------------------------------------------------- */

typedef struct {
    const ChfTable* table;
    size_t size;
} ChfStaticContext;

/* -------------------------------------------------------------------------
   Private functions
   ------------------------------------------------------------------------- */

#define GT 1
#define LT -1
#define EQ 0

static int Search( const void* l, const void* r )
{
    if ( ( ( ChfTable* )l )->module > ( ( ChfTable* )r )->module )
        return ( GT );
    else if ( ( ( ChfTable* )l )->module < ( ( ChfTable* )r )->module )
        return ( LT );
    else if ( ( ( ChfTable* )l )->code > ( ( ChfTable* )r )->code )
        return ( GT );
    else if ( ( ( ChfTable* )l )->code < ( ( ChfTable* )r )->code )
        return ( LT );

    return ( EQ );
}

static const char* StGetMessage( void* private_context, const int module_id, const int condition_code, const char* default_message )
{
    ChfTable key;
    ChfTable* res;

    key.module = module_id;
    key.code = condition_code;

    if ( ( res = bsearch( &key, ( ( ChfStaticContext* )private_context )->table, ( ( ChfStaticContext* )private_context )->size,
                          sizeof( ChfTable ), Search ) ) == ( void* )NULL )
        return ( default_message );

    return ( ( ( ChfTable* )res )->msg_template );
}

static void ExitMessage( void* private_context ) {}

/* -------------------------------------------------------------------------
   Public functions
   ------------------------------------------------------------------------- */

/* .+

.creation     : 24-May-1996
.description  :
  This function initializes CHF and returns to the caller a condition code;
  that code will be either CHF_S_OK if the initialization was succesful,
  or one of the other values listed below.

  It's necessary to invoke succesfully either ChfStaticInit() or one of the
  other CHF initialization routines before using any other CHF function.

  NOTE: This function will call ChfAbort() with abort code CHF_ABORT_DUP_INIT
        if CHF has already been initialized before.

.call	      :
                cc = ChfStaticInit(app_name, options,
                        table, table_size,
                        condition_stack_size, handler_stack_size,
                        exit_code);
.input	      :
                const char *app_name, Application's name
                const int options, Options
                const ChfTable *table, pointer to the static message table
                const size_t table_size, size of the table (# of entries)
                const int condition_stack_size, Size of the condition stack
                const int handler_stack_size, Size of the handler stack
                const int exit_code, Abnormal exit code
.output	      :
                int cc, condition code
.status_codes :
                CHF_F_MALLOC, FATAL, memory allocation failed
.notes	      :
  1.1, 27-May-1996, creation

.- */
/* Initialization with static message tables */
int ChfStaticInit( const int module_id, const char* app_name, /* Application's name */
                   const int options,                         /* Options */
                   const ChfTable* table,                     /* Static message table */
                   const size_t table_size,                   /* Size of the message table */
                   const int condition_stack_size,            /* Size of the condition stack */
                   const int handler_stack_size,              /* Size of the handler stack */
                   const int exit_code                        /* Abnormal exit code */
)
{
    ChfStaticContext* private_context;
    int cc;

    if ( ( private_context = ( ChfStaticContext* )malloc( sizeof( ChfStaticContext ) ) ) == ( ChfStaticContext* )NULL )
        cc = CHF_F_MALLOC;
    else if ( ( cc = ChfInit( module_id, app_name, options, ( void* )private_context, StGetMessage, ExitMessage, condition_stack_size,
                              handler_stack_size, exit_code ) ) != CHF_S_OK )
        free( private_context );
    else {
        private_context->table = table;
        private_context->size = table_size;
        cc = CHF_S_OK;
    }

    return cc;
}
