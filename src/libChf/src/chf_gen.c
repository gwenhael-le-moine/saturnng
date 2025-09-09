/* .+

.author	      : Ivan Cibrario B.

.creation     :	 3-May-1996

.description  :
  This module contains the condition generation function of CHF

.notes	      :
  $Log: chf_gen.c,v $
  Revision 2.2  2001/01/25 12:10:22  cibrario
  Added partial Win32 support (Windows CE only).

  Revision 1.1  1996/05/28  12:53:59  cibrario
  Initial revision
.- */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "Chf.h"
#include "ChfPriv.h"

/* .+

.creation     :  3-May-1996
.description  :
  This function generates a condition descriptor for the condition described
  by 'module_id', 'file_name', 'line_number', 'condition_code' and 'severity',
  and the partial message associated with it, specialized with the additional
  arguments '...'; the new condition descriptor is put onto the top of the
  condition stack.

  If the condition stack is full, this function generates the condition
  CHF_F_COND_STACK_FULL and immediately invokes ChfSignal() for the current
  condition stack.

  NOTE: This function calls the CHF function 'ChfAbort()' to
        abort the application if either:
        - CHF has not been initialized correctly (abort code CHF_ABORT_INIT)
        - there is an overflow in the internal buffer used during the
          generation of the partial message associated with the condition
          (abort code CHF_ABORT_MSG_OVF)
        - there was an attempt to generate a condition while the CHF condition
          CHF_F_COND_STACK_FULL (condition stack full) was being signalled
          (abort code CHF_ABORT_COND_STACK_OVF)
.call	      :
                ChfGenerate(module_id, file_name, line_number,
                        condition_code, severity, ...);
.input	      :
                const int module_id, module identifier
                const char *file_name, file name
                const int line_number, line number
                const int condition_code, condition code
                const ChfSeverity severity, severity
                ..., additional arguments
.output	      :
                void
.status_codes :
                (*) CHF_F_COND_STACK_FULL, the condition stack is full
.notes	      :
  1.1,  3-May-1996, creation

.- */
/* Generate a condition into the stack */
void ChfGenerate( const int module_id, const char* file_name, const int line_number, const int condition_code, const ChfSeverity severity,
                  ... )
{
    ChfDescriptor* new_descriptor;
    va_list aux_arg;

    /* Check that CHF has been correctly initialized */
    if ( chf_context.state == CHF_UNKNOWN )
        ChfAbort( CHF_ABORT_INIT );

    /* Prepare the additional arguments list */
    va_start( aux_arg, severity );

    if ( ( new_descriptor = chf_context.condition_sp ) - chf_context.condition_stack >= chf_context.condition_stack_size ) {
        /* The condition stack is full;
           generate the CHF_F_COND_STACK_FULL condition and signal it immediately,
           using the last available slot of the stack, if it's still empty,
           otherwise abort the application.
        */
        if ( new_descriptor - chf_context.condition_stack == chf_context.condition_stack_size ) {
            new_descriptor->module_id = module_id;
            new_descriptor->condition_code = CHF_F_COND_STACK_FULL;
            new_descriptor->severity = CHF_FATAL;
            new_descriptor->line_number = CHF_UNKNOWN_LINE_NUMBER;
            new_descriptor->file_name = CHF_UNKNOWN_FILE_NAME;

            strncpy( new_descriptor->message, ChfGetMessage( module_id, CHF_F_COND_STACK_FULL, "Condition stack is full" ),
                     CHF_MAX_MESSAGE_LENGTH - 1 );
            new_descriptor->message[ CHF_MAX_MESSAGE_LENGTH - 1 ] = '\0';

            new_descriptor->next = CHF_NULL_DESCRIPTOR;
            chf_context.condition_sp++;

            ChfSignal( module_id );
        } else
            ChfAbort( CHF_ABORT_COND_STACK_OVF );
    } else {
        char def_message[ CHF_DEF_MESSAGE_LENGTH ];
        char tmp_message[ CHF_TMP_MESSAGE_LENGTH ];

        new_descriptor->module_id = module_id;
        new_descriptor->condition_code = condition_code;
        new_descriptor->severity = severity;
        new_descriptor->line_number = line_number;
        new_descriptor->file_name = file_name;

        /* Generate the default message */
        sprintf( def_message, "Code <%d>d", condition_code );

        /* Generate the partial message associated with the condition using a
           temporary area
        */
        if ( vsprintf( tmp_message, ChfGetMessage( module_id, condition_code, def_message ), aux_arg ) >= CHF_TMP_MESSAGE_LENGTH )
            ChfAbort( CHF_ABORT_MSG_OVF );

        /* Copy the message into the condition descriptor */
        strncpy( new_descriptor->message, tmp_message, CHF_MAX_MESSAGE_LENGTH - 1 );
        new_descriptor->message[ CHF_MAX_MESSAGE_LENGTH - 1 ] = '\0';

        /* Link the new descriptor with the current descriptor list, if it
           isn't the first descriptor of the list
        */
        new_descriptor->next = ( new_descriptor == chf_context.condition_base ) ? CHF_NULL_DESCRIPTOR : new_descriptor - 1;

        chf_context.condition_sp++;
    }
}
