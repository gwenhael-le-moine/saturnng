/* .+

.identifier   : $Id: chf_abrt.c,v 2.2 2001/01/25 12:08:24 cibrario Exp $
.context      : CHF, Condition Handling Facility
.title	      : $RCSfile: chf_abrt.c,v $, condition generation
.kind	      : C source
.author	      : Ivan Cibrario B.
.site	      : CSTV-CNR
.creation     :	 3-May-1996
.keywords     : *
.description  :
  This module implements the CHF function ChfAbort()

.include      : Chf.h

.notes	      :
  $Log: chf_abrt.c,v $
  Revision 2.2  2001/01/25 12:08:24  cibrario
  Added partial Win32 support (Windows CE only).

  Revision 2.1  2000/05/26  14:22:07  cibrario
  - Conditional inclusion of pthread.h (mt support)
  - Expanded abort message table with mt support messages
  - ChfAbort() with CHF_ABORT flag clear and mt support enabled now
    exits invoking thread only

  Revision 1.1  1996/05/28  12:53:26  cibrario
  Initial revision


.- */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <setjmp.h>

#include "Chf.h"
#include "ChfPriv.h"

#ifdef _REENTRANT
#  include <pthread.h>
#endif

/* Abort codes message table; the relative position of the messages must
   match the numeric codes CHF_ABORT_xxxx defined in ChfPriv.h
*/
static const char* message_table[] = { ( const char* )NULL,
                                       "Not initialized",
                                       "Temporary message buffer overflow",
                                       "Invalid action from last chance handler",
                                       "Already initialized",
                                       "Unwind request while unwinding",
                                       "Improperly handled condition",
                                       "Fatal condition while unwinding",
                                       "Condition stack overflow",
                                       "Can't prime a new Chf context",
                                       "Pthread interaction failed" };

#define MESSAGE_TABLE_SIZE ( sizeof( message_table ) / sizeof( const char* ) )

/* .+

.title	      : ChfAbort
.kind	      : C function
.creation     : 13-May-1996
.description  :
  This function prints the message associated with 'abort_code' and then
  immediately aborts either the application (when multithreading support not
  enabled or CHF_ABORT set) or the invoking thread only (multithreading
  support enabled and CHF_ABORT not set). The abort is performed either:
    - using abort() if either CHF has not been correctly initialized or
      the chf_context.options flag CHF_ABORT is set
    - using exit(chf_context.exit_code) (multithreading support not enabled)
      or pthread_exit(chf_context.exit_code) (multithreading support enabled)
      if the flag is clear

  No message is printed if the abort code is CHF_ABORT_SILENT; this code
  is used, for example, by the default condition handler to terminate the
  application when a CHF_FATAL condition occours.

  NOTE: This function must be called only when either a serious internal CHF
        failure occurs or it's necessary to abort the application.

  WIN32:

  - stderr stream is not supported; the abort message is displayed in a
    message box only if Chf has been correctly initialized, otherwise the
    abort will be done silently

  - abort() is not supported and has been replaced by exit(EXIT_FAILURE)

.call	      :
                ChfAbort(abort_code);
.input	      :
                const int abort_code, abort_code
.output	      :
                void
.status_codes :
                none
.notes	      :
  1.1, 13-May-1996, creation
  2.1, 19-May-2000, update:
    - added multithreading support
  2.2, 22-Jan-2001, update:
    - added Win32 support

.- */
void ChfAbort( /* Abort application */
               const int abort_code )
{
    if ( abort_code != CHF_ABORT_SILENT ) {
        fputs( CHF_ABORT_HEADER, stderr );

        if ( abort_code < 0 || abort_code >= ( int )MESSAGE_TABLE_SIZE )
            fprintf( stderr, CHF_ABORT_BAD_CODE_FMT, abort_code );

        else
            fprintf( stderr, CHF_ABORT_GOOD_CODE_FMT, message_table[ abort_code ] );
    }

    if ( chf_context.state == CHF_UNKNOWN || chf_context.options & CHF_ABORT )
        abort();

    else
#ifndef _REENTRANT
        exit( chf_context.exit_code );
#else
        pthread_exit( ( void* )( chf_context.exit_code ) );
#endif
}
