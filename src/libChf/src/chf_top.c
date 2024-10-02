/* .+

.identifier   : $Id: chf_top.c,v 2.2 2001/01/25 14:09:21 cibrario Exp $
.context      : CHF, Condition Handling Facility
.title	      : $RCSfile: chf_top.c,v $
.kind	      : C source
.author	      : Ivan Cibrario B.
.site	      : CSTV-CNR
.creation     :	 5-Jun-1996
.keywords     : *
.description  :
  This module implements the CHF function ChfGetTopCondition()

.include      : Chf.h

.notes	      :
  $Log: chf_top.c,v $
  Revision 2.2  2001/01/25 14:09:21  cibrario
  Added partial Win32 support (Windows CE only).

  Revision 2.1  2000/05/26  14:23:33  cibrario
  ChfGetTopCondition() used to return a pointer to the wrong condition
  descriptor; fixed.

  Revision 1.2  1996/06/11  12:47:17  cibrario
  file creation


.- */

#ifndef lint
static char rcs_id[] = "$Id: chf_top.c,v 2.2 2001/01/25 14:09:21 cibrario Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <setjmp.h>
#include <string.h>

#include "Chf.h"
#include "ChfPriv.h"

/* -------------------------------------------------------------------------
   Public functions
   ------------------------------------------------------------------------- */

/* .+

.title	      : ChfGetTopCondition
.kind	      : C function
.creation     :  5-Jun-1996
.description  :
  This function returns to the caller a pointer to the top condition of
  the current condition group. It generates and immediately signals the
  condition CHF_F_BAD_STATE if the current condition group is empty.


  NOTE: During condition signalling, CHF creates a new, empty, condition group
        immediately before starting the invocation sequence of the condition
        handlers, as described in the documentation. Therefore
        ChfGetTopCondition(), if called from a condition handler, will return
        a pointer to the top condition generated during the handling ONLY, and
        NOT to the top condition of the condition group being signalled. The
        latter pointer is directly available, as an argument, to the condition
        handlers.

  NOTE: This function will call ChfAbort() with abort code CHF_ABORT_INIT
        if CHF hasn't been correctly initialized.

  NOTE: The returned pointer is no longer valid when any other CHF function
        is called after ChfGetTopCondition().

.call	      :
                d = ChfGetTopCondition();
.input	      :
                void
.output	      :
                const ChfDescriptor *d, condition descriptor
.status_codes :

.notes	      :
  1.2, 17-May-1996, creation
  2.1, 24-May-2000, bug fix:
    - condition stack referenced incorrectly

.- */
const ChfDescriptor* ChfGetTopCondition( /* Retrieve top condition */
                                         const int module_id )
{
    ChfDescriptor* d;

    /* Check that CHF has been correctly initialized */
    if ( chf_context.state == CHF_UNKNOWN )
        ChfAbort( CHF_ABORT_INIT );

    if ( ( d = chf_context.condition_sp ) == chf_context.condition_base ) {
        ChfCondition( module_id ) CHF_F_BAD_STATE, CHF_FATAL ChfEnd;
        ChfSignal( module_id );
    }

    /* The top element of the condition group is the element immediately
       below the stack pointer.
    */
    return d - 1;
}
