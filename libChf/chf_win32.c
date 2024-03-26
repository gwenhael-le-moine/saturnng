/* .+

.identifier   : $Id: chf_win32.c,v 2.2 2001/01/25 14:11:58 cibrario Exp $
.context      : CHF, Condition Handling Facility
.title	      : $RCSfile: chf_win32.c,v $, Win32 initialization function
.kind	      : C source
.author	      : Ivan Cibrario B.
.site	      : CSTV-CNR
.creation     :	19-Jan-2001
.keywords     : *
.description  :
  This module contains the CHF initialization function ChfWin32Init()

.include      : Chf.h

.notes	      :
  $Log: chf_win32.c,v $
  Revision 2.2  2001/01/25 14:11:58  cibrario
  *** empty log message ***


.- */

#ifndef lint
static char rcs_id[] = "$Id: chf_win32.c,v 2.2 2001/01/25 14:11:58 cibrario Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#ifndef _WIN32
#  include <errno.h>
#endif
#include <setjmp.h>
#include <string.h>

#ifdef _WIN32
#  include <windows.h>
#  include <tchar.h>
#endif

#include "Chf.h"
#include "ChfPriv.h"

/* -------------------------------------------------------------------------
   Global and static variables
   ------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------
   Private type definitions
   ------------------------------------------------------------------------- */

#ifdef _WIN32
typedef struct {
    HINSTANCE instance;                       /* App. instance handle */
    ChfChar buffer[ CHF_MAX_MESSAGE_LENGTH ]; /* Temporary buffer */
} ChfWin32Context;
#endif

/* -------------------------------------------------------------------------
   Private functions
   ------------------------------------------------------------------------- */

#ifdef _WIN32
static const ChfChar* Win32GetMessage( void* private_context, const int module_id, const int condition_code,
                                       const ChfChar* default_message )
{
    if ( !LoadString( ( ( ChfWin32Context* )private_context )->instance, module_id * 1000 + condition_code,
                      ( ( ChfWin32Context* )private_context )->buffer, CHF_MAX_MESSAGE_LENGTH - 1 ) )
        return default_message;

    return ( ( ChfWin32Context* )private_context )->buffer;
}

static void ExitMessage( void* private_context ) { free( private_context ); }
#endif

/* -------------------------------------------------------------------------
   Public functions
   ------------------------------------------------------------------------- */

/* .+

.title	      : ChfWin32Init
.kind	      : C function
.creation     : 19-Jan-2001
.description  :
  This function initializes CHF and returns to the caller a condition code;
  that code will be either CHF_S_OK if the initialization was succesful,
  or one of the other values listed below.

  It's necessary to invoke succesfully either ChfWin32Init() or one of the
  other CHF initialization routines before using any other CHF function.

  NOTE: This function will call ChfAbort() with abort code CHF_ABORT_DUP_INIT
        if CHF has already been initialized before.

  WIN32:

  - This function is available in Win32 only; it will return
    CHF_F_NOT_AVAILABLE on Unix platforma.

  - message retrieval is done through the LoadString() Win32 function.
    This function does not support message sets, so the linear message id
    passed to it is made by module_id*1000 + condition_code.  The following
    limits are in effect:
    0 <= condition_code <= 999
    0 <= module_id <= 64

.call	      :
                cc = ChfWin32Init(app_name, options,
                        msgcat_name,
                        condition_stack_size, handler_stack_size,
                        exit_code);
.input	      :
                const ChfChar *app_name, Application's name
                const ChfOptions options, Options
                HINSTANCE instance, App. instance handle
                const int condition_stack_size, Size of the condition stack
                const int handler_stack_size, Size of the handler stack
                const int exit_code, Abnormal exit code
.output	      :
                int cc, condition code
.status_codes :
                CHF_F_MALLOC, FATAL, memory allocation failed
                CHF_F_NOT_AVAILABLE, FATAL, function not available
.notes	      :
  2.2, 19-Jan-2001, creation

.- */
int ChfWin32Init(                           /* Initialization within _WIN32 */
                  const ChfChar* app_name,  /* Application's name */
                  const ChfOptions options, /* Options */
#ifndef _WIN32
                  void* instance, /* Fake arguments */
#else
                  HINSTANCE instance, /* App. instance handle */
#endif
                  const int condition_stack_size, /* Size of the condition stack */
                  const int handler_stack_size,   /* Size of the handler stack */
                  const int exit_code             /* Abnormal exit code */
)
{
#ifndef _WIN32
    /* This function is available only in Win32 */
    return CHF_F_NOT_AVAILABLE;

#else
    ChfWin32Context* private_context;
    int cc;

    if ( ( private_context = ( ChfWin32Context* )malloc( sizeof( ChfWin32Context ) ) ) == ( ChfWin32Context* )NULL )
        cc = CHF_F_MALLOC;

    else if ( ( cc = ChfInit( app_name, options, ( void* )private_context, Win32GetMessage, ExitMessage, condition_stack_size,
                              handler_stack_size, exit_code ) ) != CHF_S_OK ) {
        free( private_context );
    }

    else {
        /* Save Win32 specific context items into private Chf context */
        private_context->instance = instance;

        cc = CHF_S_OK;
    }

    return cc;

#endif
}
