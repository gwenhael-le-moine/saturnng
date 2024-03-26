/* .+

.identifier   : $Id: chf_sig.c,v 2.2 2001/01/25 14:07:42 cibrario Exp $
.context      : CHF, Condition Handling Facility
.title	      : $RCSfile: chf_sig.c,v $, condition generation
.kind	      : C source
.author	      : Ivan Cibrario B.
.site	      : CSTV-CNR
.creation     :	 3-May-1996
.keywords     : *
.description  :
  This module implements the condition signalling function of CHF

.include      : Chf.h

.notes	      :
  $Log: chf_sig.c,v $
  Revision 2.2  2001/01/25 14:07:42  cibrario
  Added partial Win32 support (Windows CE only).

  Revision 2.1  2000/05/26  14:31:28  cibrario
  - Fixed spelling of CHF_SIGNALLING -> CHF_SIGNALING
  - Replaced longjmp() with siglongjmp()
  - New ChfAction code CHF_UNWIND_KEEP: ChfSignal() unwinds the
    execution stack, but keeps the topmost condition group on the
    condition stack

  Revision 1.6  1997/01/15  13:34:45  cibrario
  Fixed a wrong adjustment of the condition handler stack pointer after
  an unwind operation.
  Updated the condition handler calls in order to pass to the handlers the
  private handler context pointer.

  Revision 1.1  1996/05/28  12:55:51  cibrario
  Initial revision


.- */

#ifndef lint
static char rcs_id[] = "$Id: chf_sig.c,v 2.2 2001/01/25 14:07:42 cibrario Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#ifndef _WIN32
#  include <errno.h>
#endif
#include <setjmp.h>

#ifdef _WIN32
#  include <windows.h>
#  include <tchar.h>
#endif

#include "Chf.h"
#include "ChfPriv.h"

/* .+

.title	      : ChfSignal
.kind	      : C function
.creation     : 10-May-1996
.description  :
  This function signals the topmost condition group currently in the
  condition stack, and performs the actions requested by the condition
  handlers.

  NOTE: This function uses the CHF function 'ChfAbort()' to
        abort the application if either
        - CHF has not been initialized correctly (abort code CHF_ABORT_INIT)
        - the last handler on the handler stack has returned an invalid action
          code (abort code CHF_ABORT_INVALID_ACTION)
        - one of the handlers has requested the CHF_UNWIND action while
          CHF was already unwinding (abort code CHF_ABORT_ALREADY_UNWINDING)
        - a CHF_FATAL condition was signalled while CHF was unwinding
          (abort code CHF_ABORT_FATAL_UNWINDING)
        - all the handlers refused to handle a condition (abort code
          CHF_ABORT_IMPROPERLY_HANDLED)

.call	      :
                ChfSignal();
.input	      :
                void
.output	      :
                void
.status_codes :
                (*) CHF_F_COND_STACK_FULL, the condition stack is full
                (*) CHF_F_INVALID_ACTION, invalid handler action (%d)
.notes	      :
  1.1, 10-May-1996, creation
  1.6, 15-Jan-1997, update & bug fix:
    - fixed a wrong adjustment of the condition handler stack pointer after
      an unwind operation.
    - updated the condition handler calls in order to pass to the handlers the
      private handler context pointer, too.
  2.1, 19-May-2000, update:
    - added support for structured condition handling

.- */
void ChfSignal( void )
{
    ChfState saved_state;
    ChfDescriptor* saved_condition_base;
    ChfDescriptor* current_condition;
    ChfHandlerDescriptor* saved_handler_sp;
    ChfHandlerDescriptor* handler_up;
    ChfHandlerDescriptor* unwind_handler;
    ChfAction handler_result;

    /* Check that CHF has been correctly initialized and save the current CHF
       state. If CHF was CHF_IDLE change state to CHF_SIGNALING, else if CHF was
       CHF_UNWINDING change to CHF_SIGNAL_UNWINDING, otherwise remain in the
       previous state (that must be CHF_SIGNALING)
    */
    if ( chf_context.state == CHF_UNKNOWN )
        ChfAbort( CHF_ABORT_INIT );
    saved_state = chf_context.state;

    if ( chf_context.state == CHF_IDLE )
        chf_context.state = CHF_SIGNALING;
    else if ( chf_context.state == CHF_UNWINDING )
        chf_context.state = CHF_SIGNAL_UNWINDING;

    if ( chf_context.condition_sp > chf_context.condition_base ) {
        /* Save the base of the current condition group and then update it in
           order to allow further generation of conditions inside the condition
           handlers that will be called soon.
        */
        current_condition = chf_context.condition_sp - 1;
        saved_condition_base = chf_context.condition_base;
        chf_context.condition_base = chf_context.condition_sp;

        /* Save the current condition handler pointer */
        saved_handler_sp = chf_context.handler_sp;

        /* Call the condition handlers; the loop will exit either:
           - when the handler stack is empty, or
           - when the current handler returns either CHF_CONTINUE or CHF_UNWIND
        */
        handler_result = CHF_RESIGNAL;
        while ( handler_result == CHF_RESIGNAL && chf_context.handler_sp > chf_context.handler_stack ) {
            chf_context.handler_sp--;

            /* The current condition handler, described by chf_context.handler_sp,
               can recursively invoke ChfGenerate() and ChfSignal().

               ChfGenerate() will store the new condition group starting from
               chf_context.condition_sp, that points to the first free slot
               of the condition stack. During the first generation, since
               chf_context.condition_sp == chf_context.condition_base, the
               link pointer of the condition will be NULL and, therefore,
               the condition will be the first of a new condition group.

               ChfSignal() will signal the condition group described by the
               stack block from chf_context.condition_base to
               chf_context.condition_sp-1, if it contains at least one condition;
               it will call the handlers starting from chf_context.handler_sp-1,
               that describes the handler immediately preceding the current handler.
            */
            handler_result =
                chf_context.handler_sp->handler( current_condition, chf_context.state, chf_context.handler_sp->handler_context );

            /* When the CHF state is CHF_SIGNALING, any condition group generated
               but not yet signalled when the current handler exits must be merged
               with the condition group currently being signalled, in order to allow
               the condition handlers to add their own conditions to the condition
               group. If the severity of the previous condition group was CHF_FATAL,
               the severity of the new group is forced to CHF_FATAL, too.

               When the CHF state is CHF_UNWINDING, the condition group for
               which the UNWIND has been requested is 'frozen', no further
               modifications are allowed on it, and the condition group is simply
               discarded.
            */
            if ( chf_context.condition_sp > chf_context.condition_base ) {
                if ( chf_context.state == CHF_SIGNALING ) {
                    /* Force the new severity to CHF_FATAL if necessary */
                    if ( ChfGetSeverity( current_condition ) == CHF_FATAL )
                        ChfGetSeverity( chf_context.condition_sp - 1 ) = CHF_FATAL;

                    /* Link together the condition groups */
                    chf_context.condition_base->next = current_condition;
                    current_condition = chf_context.condition_sp - 1;
                    chf_context.condition_base = chf_context.condition_sp;
                }

                else
                    chf_context.condition_sp = chf_context.condition_base;
            }

            /* The action CHF_CONTINUE is not allowed if the current condition
               severity is CHF_FATAL; it's automatically changed to CHF_RESIGNAL
            */
            if ( handler_result == CHF_CONTINUE && ChfGetSeverity( current_condition ) == CHF_FATAL )
                handler_result = CHF_RESIGNAL;
        }

        /* Perform the action requested by the last condition handler invoked */
        switch ( handler_result ) {
            case CHF_CONTINUE:
                {
                    /* Restore the handler stack pointer; the next ChfSignal() invoked
                       from our same nesting level will invoke our same handler chain
                       again.
                    */
                    chf_context.handler_sp = saved_handler_sp;

                    /* Discard the current condition group */
                    chf_context.condition_base = chf_context.condition_sp = saved_condition_base;

                    /* Restore che saved CHF state */
                    chf_context.state = saved_state;

                    /* Continue from the instruction following the ChfSignal() */
                    break;
                }

            case CHF_UNWIND:
            case CHF_UNWIND_KEEP:
                {
                    /* Unwind the execution stack. Check that another unwind isn't
                       already in progress
                    */
                    if ( chf_context.state == CHF_UNWINDING )
                        ChfAbort( CHF_ABORT_ALREADY_UNWINDING );

                    else {
                        /* Change CHF state */
                        chf_context.state = CHF_UNWINDING;

                        /* chf_context.handler_sp points to the condition handler that
                           has requested the unwind; call all the handlers again, starting
                           from saved_handler_sp (top of the handler stack) up to and
                           including chf_context.handler_sp.
                        */
                        handler_up = saved_handler_sp;

                        while ( handler_up > chf_context.handler_sp ) {
                            ChfAction unw_handler_result;
                            handler_up--;

                            /* The current condition handler, described by handler_up
                               can recursively invoke ChfGenerate() and ChfSignal().

                               ChfGenerate() will store the new condition group starting from
                               chf_context.condition_sp, that points to the first free slot
                               of the condition stack. During the first generation, since
                               chf_context.condition_sp == chf_context.condition_base, the
                               link pointer of the condition will be NULL and, therefore,
                               the condition will be the first of a new condition group.

                               ChfSignal() will generate the condition group described by the
                               stack block from chf_context.condition_base to
                               chf_context.condition_sp-1, if it contains at least one
                               condition; it will call the handlers starting from
                               chf_context.handler_sp-1, that describes the handler
                               immediately preceding the handler that has requested the unwind.

                               Further unwind requests are not allowed, and will trigger
                               the condition CHF_F_UNWINDING
                            */
                            unw_handler_result = handler_up->handler( current_condition, chf_context.state, handler_up->handler_context );

                            /* When the CHF state is CHF_UNWINDING, any condition group
                               generated but not yet signalled when the current handler
                               returns must be discarded
                            */
                            chf_context.condition_sp = chf_context.condition_base;
                        }

                        /* Restore the handler stack pointer, discarding the unwinded
                           condition handlers. chf_context.handler_sp points to the
                           handler that requested the unwind; that handler has been
                           unwinded and its location is now the first free slot in the
                           condition handler stack.
                        */
                        unwind_handler = chf_context.handler_sp;

                        if ( handler_result == CHF_UNWIND ) {
                            /* Normal unwind:
                               restore the condition stack pointers, discarding all condition
                               groups.
                            */
                            chf_context.condition_base = chf_context.condition_sp = chf_context.condition_stack;
                        } else {
                            /* Special unwind for structured condition handling:
                               restore the condition_base pointer only, to keep the
                               topmost condition group on the condition stack. This way,
                               the condition group remains accessible after the unwind.
                            */
                            chf_context.condition_base = saved_condition_base;
                        }

                        /* Change the CHF state to CHF_IDLE, and execute longjmp().
                           If the handler hasn't a valid unwind_context associated with it,
                           simply abort the application.
                        */
                        chf_context.state = CHF_IDLE;

                        if ( unwind_handler->unwind_context == CHF_NULL_CONTEXT )
                            ChfAbort( CHF_ABORT_SILENT );
                        else
                            ChfSiglongjmp( unwind_handler->unwind_context, 1 );
                    }

                    break;
                }

            case CHF_RESIGNAL:
                {
                    ChfAbort( ( chf_context.state == CHF_SIGNALING ) ? CHF_ABORT_IMPROPERLY_HANDLED : CHF_ABORT_FATAL_UNWINDING );

                    break;
                }

            default:
                {
                    /* Invalid handler action detected; generate and immediately signal a
                       condition if the broken handler isn't the last handler on the stack,
                       otherwise call ChfAbort()
                    */
                    if ( chf_context.handler_sp > chf_context.handler_stack ) {
                        ChfCondition CHF_F_INVALID_ACTION, CHF_FATAL, handler_result ChfEnd;

                        ChfSignal();
                    }

                    else
                        ChfAbort( CHF_ABORT_INVALID_ACTION );

                    break;
                }
        }
    }

    /* Restore the old CHF state */
    chf_context.state = saved_state;
}

/* .+

.title	      : ChfDiscard
.kind	      : C function
.creation     : 17-May-1996
.description  :
  This function discards the topmost condition group currently in the
  condition stack, without signalling it. The function does nothing if
  the condition stack is empty.

  NOTE: This function uses the CHF function 'ChfAbort()' to
        abort the application if either
        - CHF has not been initialized correctly (abort code CHF_ABORT_INIT)

.call	      :
                ChfDiscard();
.input	      :
                void
.output	      :
                void
.status_codes :
                none
.notes	      :
  1.1, 17-May-1996, creation

.- */
void ChfDiscard( /* Discard the current conditions */
                 void )
{
    /* Check that CHF has been correctly initialized */
    if ( chf_context.state == CHF_UNKNOWN )
        ChfAbort( CHF_ABORT_INIT );

    /* Reset the current condition stack pointer to the current condition
       stack base pointer
    */
    chf_context.condition_sp = chf_context.condition_base;
}
