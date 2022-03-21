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

#ifndef lint
static char rcs_id[] = "$Id: chf_abrt.c,v 2.2 2001/01/25 12:08:24 cibrario Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#ifndef _WIN32
#include <errno.h>
#endif
#include <setjmp.h>

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#endif

#include "Chf.h"
#include "ChfPriv.h"

#ifdef _REENTRANT
#include <pthread.h>
#endif


/* Abort codes message table; the relative position of the messages must
   match the numeric codes CHF_ABORT_xxxx defined in ChfPriv.h
*/
static const ChfChar *message_table[] =
{
  (const ChfChar *)NULL,
  ChfText("Not initialized"),
  ChfText("Temporary message buffer overflow"),
  ChfText("Invalid action from last chance handler"),
  ChfText("Already initialized"),
  ChfText("Unwind request while unwinding"),
  ChfText("Improperly handled condition"),
  ChfText("Fatal condition while unwinding"),
  ChfText("Condition stack overflow"),
  ChfText("Can't prime a new Chf context"),
  ChfText("Pthread interaction failed")
};

#define MESSAGE_TABLE_SIZE	(sizeof(message_table)/sizeof(const ChfChar *))


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
void ChfAbort(		/* Abort application */
  const int abort_code
)
{
#ifdef _WIN32
  if(abort_code != CHF_ABORT_SILENT)
  {
    TCHAR abort_msg[CHF_MAX_MESSAGE_LENGTH];
    HWND active_window;

    /* stderr not available;
       put complaint in a message box and display it
    */
    if(abort_code < 0  || abort_code >= MESSAGE_TABLE_SIZE)
      _stprintf(abort_msg,
		CHF_ABORT_BAD_CODE_FMT, abort_code);

    else
      _stprintf(abort_msg,
		CHF_ABORT_GOOD_CODE_FMT, message_table[abort_code]);

    /* Return value of MessageBox() ignored, because there is only
       one available choice (abort) here.  Avoid using a NULL handle.
    */
    if(chf_context.state != CHF_UNKNOWN
      && (active_window = GetActiveWindow()) != (HWND)NULL)
      (void)
      MessageBox(active_window,
	abort_msg,
	chf_context.app_name,
	MB_OK
	|MB_ICONERROR
	|MB_APPLMODAL|MB_SETFOREGROUND);
  }

  /* Immediately exit the application with exit code EXIT_FAILURE
     if CHF_ABORT option is set or if something is wrong with Chf state.
  */
  if(chf_context.state == CHF_UNKNOWN || chf_context.options & CHF_ABORT)
    exit(EXIT_FAILURE);

  else
    /* Else, exit the application anyway, but with the exit code
       registered by the application.  Don't use PostQuitMessage(),
       because the contract is that ChfAbort() never returns to the caller.
    */
#ifndef _REENTRANT
    exit(chf_context.exit_code);
#else
#error "_REENTRANT not supported yet"
#endif

#else
  if(abort_code != CHF_ABORT_SILENT)
  {
    fputs(CHF_ABORT_HEADER, stderr);

    if(abort_code < 0  || abort_code >= MESSAGE_TABLE_SIZE)
      fprintf(stderr, CHF_ABORT_BAD_CODE_FMT, abort_code);

    else
      fprintf(stderr, CHF_ABORT_GOOD_CODE_FMT, message_table[abort_code]);
  }

  if(chf_context.state == CHF_UNKNOWN || chf_context.options & CHF_ABORT)
    abort();

  else
#ifndef _REENTRANT
    exit(chf_context.exit_code);
#else
    pthread_exit((void *)(chf_context.exit_code));
#endif
#endif
}
