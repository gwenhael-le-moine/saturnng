/* .+

.identifier   : $Id: chf_msgc.c,v 2.2 2001/01/25 14:06:47 cibrario Exp $
.context      : CHF, Condition Handling Facility
.title	      : $RCSfile: chf_msgc.c,v $, condition generation
.kind	      : C source
.author	      : Ivan Cibrario B.
.site	      : CSTV-CNR
.creation     :	17-May-1996
.keywords     : *
.description  :
  This module contains the CHF initialization function ChfMsgcatInit()

.include      : Chf.h

.notes	      :
  $Log: chf_msgc.c,v $
  Revision 2.2  2001/01/25 14:06:47  cibrario
  Added partial Win32 support (Windows CE only).

  Revision 1.3  1996/06/21  14:19:22  cibrario
  Bug fix: the private context of the message retrieval facility was
  never freed by ExitMessage()

  Revision 1.1  1996/05/28  12:55:15  cibrario
  Initial revision


.- */

#ifndef lint
static char rcs_id[] = "$Id: chf_msgc.c,v 2.2 2001/01/25 14:06:47 cibrario Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <setjmp.h>
#include <string.h>
#include <locale.h>
#include <nl_types.h>

#include "Chf.h"
#include "ChfPriv.h"

/* -------------------------------------------------------------------------
   Global and static variables
   ------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------
   Private type definitions
   ------------------------------------------------------------------------- */

typedef struct {
    nl_catd catalog; /* Message catalog descriptor */
} ChfMsgcatContext;

/* -------------------------------------------------------------------------
   Private functions
   ------------------------------------------------------------------------- */

static const char* GetMessage( void* private_context, const int module_id, const int condition_code, const char* default_message )
{
    return ( catgets( ( ( ChfMsgcatContext* )private_context )->catalog, module_id, condition_code, default_message ) );
}

static void ExitMessage( void* private_context )
{
    ( void )catclose( ( ( ChfMsgcatContext* )private_context )->catalog );
    free( private_context );
}

/* -------------------------------------------------------------------------
   Public functions
   ------------------------------------------------------------------------- */

/* .+

.title	      : ChfMsgcatInit
.kind	      : C function
.creation     : 17-May-1996
.description  :
  This function initializes CHF and returns to the caller a condition code;
  that code will be either CHF_S_OK if the initialization was succesful,
  or one of the other values listed below.

  It's necessary to invoke succesfully either ChfMsgcatInit() or one of the
  other CHF initialization routines before using any other CHF function.

  NOTE: This function will call ChfAbort() with abort code CHF_ABORT_DUP_INIT
        if CHF has already been initialized before.

  WIN32:

  - this function is not available due to lack of system support, and
    always returns CHF_F_NOT_AVAILABLE

.call	      :
                cc = ChfMsgcatInit(app_name, options,
                        msgcat_name,
                        condition_stack_size, handler_stack_size,
                        exit_code);
.input	      :
                const char *app_name, Application's name
                const ChfOptions options, Options
                const char *msgcat_name, Name of the message catalog
                const int condition_stack_size, Size of the condition stack
                const int handler_stack_size, Size of the handler stack
                const int exit_code, Abnormal exit code
.output	      :
                int cc, condition code
.status_codes :
                CHF_F_SETLOCALE, setlocale() failed
                CHF_F_CATOPEN, catopen() failed
                CHF_F_MALLOC, FATAL, memory allocation failed
                CHF_F_NOT_AVAILABLE, FATAL, function not available
.notes	      :
  1.1, 17-May-1996, creation
  2.2, 22-Jan-2001, update:
    - added Win32 support

.- */
int ChfMsgcatInit(                                 /* Initialization with msgcat subsystem */
                   const ChfChar* app_name,        /* Application's name */
                   const ChfOptions options,       /* Options */
                   const ChfChar* msgcat_name,     /* Name of the message catalog */
                   const int condition_stack_size, /* Size of the condition stack */
                   const int handler_stack_size,   /* Size of the handler stack */
                   const int exit_code             /* Abnormal exit code */
)
{
    ChfMsgcatContext* private_context;
    int cc;

    if ( ( private_context = ( ChfMsgcatContext* )malloc( sizeof( ChfMsgcatContext ) ) ) == ( ChfMsgcatContext* )NULL )
        cc = CHF_F_MALLOC;
    else if ( setlocale( LC_ALL, "" ) == ( char* )NULL ) {
        free( private_context );
        cc = CHF_F_SETLOCALE;
    } else if ( ( private_context->catalog = catopen( msgcat_name, 0 ) ) == ( nl_catd )( -1 ) ) {
        free( private_context );
        cc = CHF_F_CATOPEN;
    } else if ( ( cc = ChfInit( app_name, options, ( void* )private_context, GetMessage, ExitMessage, condition_stack_size,
                                handler_stack_size, exit_code ) ) != CHF_S_OK ) {
        ( void )catclose( private_context->catalog );
        free( private_context );
    } else
        cc = CHF_S_OK;

    return cc;
}
