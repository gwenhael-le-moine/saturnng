/* .+

.identifier   : $Id: chf_hdlr.c,v 2.2 2001/01/25 12:12:46 cibrario Exp $
.context      : CHF, Condition Handling Facility
.title	      : $RCSfile: chf_hdlr.c,v $, condition generation
.kind	      : C source
.author	      : Ivan Cibrario B.
.site	      : CSTV-CNR
.creation     :	 3-May-1996
.keywords     : *
.description  :
  This module implements the CHF functions ChfPushHandler() and
  ChfPopHandler()

.include      : Chf.h

.notes	      :
  $Log: chf_hdlr.c,v $
  Revision 2.2  2001/01/25 12:12:46  cibrario
  Added partial Win32 support (Windows CE only).

  Revision 2.1  2000/05/26  14:45:04  cibrario
  - Implemented StructuredHelper(), the structured condition handling
    helper handler
  - Updated ChfPushHandler() to push the structured condition handling
    helper when new_handler is CHF_NULL_HANDLER
  - unwind_context is now a sigjmp_buf, passed as argument directly,
    that is, without additional address operators
  - improved documentation of ChfPopHandler()

  Revision 1.6  1997/01/15  13:44:39  cibrario
  The function ChfPushHandler() has the new argument 'handler_context'.

  Revision 1.1  1996/05/28  12:54:28  cibrario
  Initial revision


.- */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <setjmp.h>

#include "Chf.h"
#include "ChfPriv.h"

/* .+

.title	      : StructuredHelper
.kind	      : C function
.creation     : 19-May-2000
.description  :
  This function is the structured condition handling helper of CHF.
  It's automatically pushed into the condition handler stack by
  ChfPushHandler() when its 'new_handler' argument is CHF_NULL_HANDLER,
  and performs the following functions:

        - if called during an ordinary signalling operation with a
          CHF_FATAL condition, it requests the action CHF_UNWIND_KEEP

        - if called when Chf is in any other state, or with a
          severity less than CHF_FATAL, it requests the action CHF_RESIGNAL

  The structured condition handling helper currently makes no use of
  handler_context.

.call	      :
                action = StructuredHelper(desc, state, context);
.input	      :
                const ChfDescriptor *desc, condition descriptor
                const ChfState state, current CHF state
.output	      :
                ChfAction action, action requested by the handler
.status_codes :
                none
.notes	      :
  2.1, 19-May-2000, creation

.- */
static ChfAction StructuredHelper( const ChfDescriptor* desc, const ChfState state, void* handler_context )
{
    ChfAction action;
    const ChfDescriptor* d;

    return ( ( state == CHF_SIGNALING && desc->severity == CHF_FATAL ) ? CHF_UNWIND_KEEP : CHF_RESIGNAL );
}

/* .+

.title	      : ChfPushHandler
.kind	      : C function
.creation     : 13-May-1996
.description  :
  This function pushes the new condition handler 'new_handler' with its
  associated longjmp context pointed by 'unwind_context' into the handler
  stack and returns CHF_S_OK to the caller.  If 'new_handler' is
  CHF_NULL_HANDLER, the special structured condition handling helper
  'StructuredHelper()' is pushed instead.

  Moreover, this function saves a copy of the pointer 'handler_context'; it
  will be passed to 'new_handler' upon each subsequent activation, and
  therefore can be used as a private handler context pointer. The user must
  assure that the information pointed by 'handler_context', if any, will
  remain valid until 'new_handler' is popped from the condition stack.
  'handler_context' may be set to the special (null) value CHF_NULL_POINTER to
  indicate that the handler hasn't any private context information.

  If, in the future, the handler will request the CHF_UNWIND action, the
  setjmp() function invocation that established 'unwind_context' will appear
  to return again.

  'unwind_context' can be the reserved (null) pointer CHF_NULL_CONTEXT; in
  this case, if the handler will request the CHF_UNWIND_ACTION, the
  application will be silently terminated calling ChfAbort() with abort code
  CHF_ABORT_SILENT.

  If some error occours during the execution, the function will generate
  and immediately signal one of the conditions listed below and marked with
  (*). The function will never return dorectly to the caller, since all
  conditions are CHF_FATAL.

  NOTE: This function calls ChfAbort() with abort code CHF_ABORT_INIT if
        the CHF subsystem has not been initialized.

.call	      :
                ChfPushHandler(new_handler, unwind_context);
.input	      :
                ChfHandler new_handler, new condition handler
                void *unwind_context, handler unwind context pointer
                void* handler_context, private handler context pointer
.output	      :
                void
.status_codes :
                (*) CHF_F_BAD_STATE, bad CHF state for requested operation
                (*) CHF_F_HDLR_STACK_FULL, the handler stack is full
.notes	      :
  1.1, 13-May-1996, creation
  1.6, 15-Jan-1997, update:
    - added the argument 'handler_context'
    - improved documentation
  2.1, 19-May-2000, update:
    - now using sigjmp_buf as unwind_context
    - added StructuredHelper handling

.- */
void ChfPushHandler( /* Push a new handler into the stack */
                     const int module_id, ChfHandler new_handler, void* unwind_context, void* handler_context )
{
    /* Make sure that CHF has been correctly initialized and is idle */
    if ( chf_context.state == CHF_UNKNOWN )
        ChfAbort( CHF_ABORT_INIT );

    if ( chf_context.state != CHF_IDLE ) {
        CHF_Condition( module_id ) CHF_F_BAD_STATE, CHF_FATAL ChfEnd;

        ChfSignal( module_id );
    }

    /* Check if the handler stack is full */
    else if ( chf_context.handler_sp - chf_context.handler_stack >= chf_context.handler_stack_size ) {
        CHF_Condition( module_id ) CHF_F_HDLR_STACK_FULL, CHF_FATAL ChfEnd;

        ChfSignal( module_id );
    }

    else {
        chf_context.handler_sp->unwind_context = unwind_context;
        chf_context.handler_sp->handler_context = handler_context;
        chf_context.handler_sp->handler = ( ( new_handler == CHF_NULL_HANDLER ) ? StructuredHelper : new_handler );
        chf_context.handler_sp++;
    }
}

/* .+

.title	      : ChfPopHandler
.kind	      : C function
.creation     : 13-May-1996
.description  :
  This function pops the topmost condition handler from the handler stack and
  returns to the caller.

  If some error occours during the execution, the function will generate
  and immediately signal one of the conditions listed below and marked with
  (*). The function will never return directly to the caller, since all
  conditions are CHF_FATAL.

  NOTE: This function calls ChfAbort() with abort code CHF_ABORT_INIT if
        the CHF subsystem has not been initialized.

.call	      :
                ChfPopHandler();
.input	      :
                void
.output	      :
                void
.status_codes :
                CHF_F_BAD_STATE, bad CHF state for requested operation
                CHF_F_HDLR_STACK_FULL, the handler stack is full
.notes	      :
  1.1, 13-May-1996, creation
  1.6, 15-Jan-1997, update:
    - improved documentation
  2.1, 19-May-2000, update:
    - improved documentation

.- */
/* void ChfPopHandler( /\* Pop a handler *\/ */
/*                     const int module_id ) */
/* { */
/*     /\* Make sure that CHF has been correctly initialized and is idle *\/ */
/*     if ( chf_context.state == CHF_UNKNOWN ) */
/*         ChfAbort( CHF_ABORT_INIT ); */

/*     if ( chf_context.state != CHF_IDLE ) { */
/*         CHF_Condition( module_id ) CHF_F_BAD_STATE, CHF_FATAL ChfEnd; */

/*         ChfSignal( module_id ); */
/*     } */

/*     /\* Check if the handler stack is empty *\/ */
/*     else if ( chf_context.handler_sp == chf_context.handler_stack ) { */
/*         CHF_Condition( module_id ) CHF_F_HDLR_STACK_EMPTY, CHF_FATAL ChfEnd; */

/*         ChfSignal( module_id ); */
/*     } */

/*     /\* Discard the topmost condition handler *\/ */
/*     else */
/*         --chf_context.handler_sp; */
/* } */
