/* .+

.identifier   : $Id: chf_init.c,v 2.2 2001/01/25 14:05:23 cibrario Exp $
.context      : CHF, Condition Handling Facility
.title	      : $RCSfile: chf_init.c,v $, condition generation
.kind	      : C source
.author	      : Ivan Cibrario B.
.site	      : CSTV-CNR
.creation     :	 3-May-1996
.keywords     : *
.description  :
  This module implements the CHF initialization function ChfInit()

.include      : Chf.h

.notes	      :
  $Log: chf_init.c,v $
  Revision 2.2  2001/01/25 14:05:23  cibrario
  Added partial Win32 support (Windows CE only).

  Revision 2.1  2000/05/26  15:30:42  cibrario
  - Renamed static context chf_context to _chf_context; it is now in
    actual use only when mt support is not enabled, otherwise it is
    used only as a prototype to prime the per-thread contexts on demand
  - Conditional definition of mt support static variables
    (context_mutex to access the static master context, fputs_mutex to
    ensure the atomicity of DefaultHandler() while it is printing a
    sequence of grouped condition messages) and functions (DestroyContext()
    to destroy a per-thread context when the owning thread terminates)
  - Deeply revised ChfInit() and ChfExit() to implement multithreading
    support
  - Implemented function _ChfGetContext() to return a pointer to the
    per-thread Chf context, creating them on-demand.

  Revision 1.6  1997/01/15  13:37:19  cibrario
  The Chf condition handlers now have a third argument: the private handler
  context pointer; the code has been updated accordingly.

  Revision 1.4  1996/09/25  13:29:01  cibrario
  Added static char[] variable rcs_lib_id; it contains the value of the
  CHF_LIBRARY_ID macro. The header Chf.h sets that macro to its RCS Id,
  that is also the Id of the whole Chf library.

  Revision 1.1  1996/05/28  12:54:58  cibrario
  Initial revision


.- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Chf.h"
#include "ChfPriv.h"

/* -------------------------------------------------------------------------
   Global and static variables
   ------------------------------------------------------------------------- */

/* CHF context */
ChfContext _chf_context;

/* Message separator and severity names for ChfBuildMessage() */
static const char separator[] = "-";
static const char* severity_name[] = { "S", "I", "W", "E", "F" };

/* -------------------------------------------------------------------------
   Multithreading support
   ------------------------------------------------------------------------- */
#ifdef _REENTRANT
#  include <pthread.h>

/* Mutex to access chf_context during initialization and exit;
   mutex to puts condition messages on stderr (DefaultHandler)
*/
static pthread_mutex_t context_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t fputs_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Chf data key */
static pthread_key_t data_key;

/* This function is called to destroy a Chf context when the owning
   thread terminated.
*/
static void DestroyContext( void* context )
{
    free( ( ( ChfContext* )context )->message_buffer );
    free( ( ( ChfContext* )context )->handler_stack );
    free( ( ( ChfContext* )context )->condition_stack );
    free( context );
}
#endif

/* -------------------------------------------------------------------------
   Private functions
   ------------------------------------------------------------------------- */

/* .+

.title	      : DefaultHandler
.kind	      : C function
.creation     : 17-May-1996
.description  :
  This function is the default condition handler of CHF. It's automatically
  pushed into the condition handler stack by ChfInit() and performs the
  following functions:

        - if called during an unwind, it returns immediately to the caller,
          requesting the action CHF_RESIGNAL, else

        - if the severity of the condition being signalled is greater than
          CHF_SUCCESS, it prints the messages associated with the entire
          condition group on stderr using the standard function
          ChfBuildMessage() to build the messages.

        - if the severity of the condition being signalled is less than
          CHF_FATAL, it returns to the caller requesting the action
          CHF_CONTINUE, else

        - if the CHF_FATAL condition was NOT signalled during an unwind
          operation, it returns to the caller requesting the action
          CHF_UNWIND, otherwise it requests the action CHF_RESIGNAL.

  WIN32:

  - stderr stream is not available and modal MessageBox require the
    parent's handle to work reliably; the default handler does not print
    anything

.call	      :
                action = DefaultHandler(desc, state, context);
.input	      :
                const ChfDescriptor *desc, condition descriptor
                const ChfState state, current CHF state
.output	      :
                ChfAction action, action requested by the handler
.status_codes :
                none
.notes	      :
  1.1, 16-May-1996, creation
  1.6, 15-Jan-1997, update:
    - the Chf condition handlers now have a third argument: the private
      handler context pointer.
  2.1, 25-May-2000, update:
    - added multithreading support
  2.2, 22-Jan-2001, update:
    - added Win32 support

.- */
static ChfAction DefaultHandler( const ChfDescriptor* desc, const ChfState state, void* handler_context )
{
    ChfAction action;
    const ChfDescriptor* d;

    if ( state == CHF_UNWINDING )
        /* If CHF is unwinding, do nothing */
        action = CHF_RESIGNAL;

    else {
        /* Print the condition messages, if necessary. The sequence of fputs()
           is done atomically if multithreading support is enabled.
           In Win32, the default handler does not print anything.
        */
        if ( desc->severity > CHF_SUCCESS ) {
#ifdef _REENTRANT
            if ( pthread_mutex_lock( &fputs_mutex ) )
                ChfAbort( CHF_ABORT_PTHREAD );
#endif
            for ( d = desc; d != CHF_NULL_DESCRIPTOR; d = d->next )
                fputs( ChfBuildMessage( d ), stderr );
#ifdef _REENTRANT
            if ( pthread_mutex_unlock( &fputs_mutex ) )
                ChfAbort( CHF_ABORT_PTHREAD );
#endif
        }

        /* Determine the handler action */
        switch ( desc->severity ) {
            case CHF_SUCCESS:
            case CHF_INFO:
            case CHF_WARNING:
            case CHF_ERROR:
                {
                    /* Continue execution if the severity is less than CHF_FATAL */
                    action = CHF_CONTINUE;
                    break;
                }

            default:
                {
                    /* The severity of the condition is CHF_FATAL; appempt to unwind if
                       the fatal condition wasn't signalled during another unwind.
                    */
                    action = ( ( state == CHF_SIGNAL_UNWINDING ) ? CHF_RESIGNAL : CHF_UNWIND );
                    break;
                }
        }
    }

    /* Return the action code to the Chf handler dispatcher */
    return action;
}

/* .+

.title	      : scopy
.kind	      : C function
.creation     : 16-May-1996
.description  :
  This function writes the NUL-terminated string pointed by 'q' starting
  from 'p', including the NUL terminator and without trepassing 'p_end'.
  The function returns a pointer to the NUL-terminator just written.

.call	      :
                np = scopy(p, q, p_end);
.input	      :
                char *p, starting position for the write
                const char *q, pointer to the string to be copied
                char *p_end, pointer to the end of the output area
.output	      :
                char *np, pointer to the NUL-terminator just written
.status_codes :
                none
.notes	      :
  1.1, 16-May-1996, creation

.- */
static char* scopy( char* p, const char* q, char* p_end )
{
    size_t q_len = strlen( q );
    size_t p_avail = p_end - p;

    if ( q_len < p_avail ) {
        strcpy( p, q );
        p += q_len;
    }

    else if ( p_avail > 1 ) {
        strncpy( p, q, p_avail - 2 );
        p[ p_avail - 1 ] = '\0';
        p = p_end;
    }

    return p;
}

/* -------------------------------------------------------------------------
   Public functions
   ------------------------------------------------------------------------- */

/* .+

.title	      : ChfGetMessage
.kind	      : C function
.creation     : 17-May-1996
.description  :
  This function retrieves the message associated with the pair
  ('module_id', 'condition_code') and returns a pointer to it. The function
  will return 'default_message' if it isn't able to retrieve the message.
  If module_id==CHF_ERRNO_SET, the function will use strerror(), if
  necessary, to retrieve the requested message.

  NOTE: This function will call ChfAbort() with abort code CHF_ABORT_INIT
        if CHF hasn't been correctly initialized.

  NOTE: The returned pointer points to per-thread static storage, which will be
        overwritten by subsequent calls to this function.

  WIN32:

  - strerror() is not supported; the last-chance translation of condition
    codes in CHF_ERRNO_SET is not performed

.call	      :
                message = ChfGetMessage(module_id, condition_code,
                  default_message);
.input	      :
                const int module_id, module identifier
                const int condition_code, condition code
                const char *default_message, default message
.output	      :
                const char *message, pointer to the retrieved message
.status_codes :
                none
.notes	      :
  1.1, 17-May-1996, creation
  2.2, 22-Jan-2001, update:
    - added Win32 support

.- */
const char* ChfGetMessage( /* Retrieve a condition message */
                           const int module_id, const int condition_code, const char* default_message )
{
    const char* message;

    /* Check that CHF has been correctly initialized */
    if ( chf_context.state == CHF_UNKNOWN )
        ChfAbort( CHF_ABORT_INIT );

    if ( ( message = chf_context.mrs_get( chf_context.mrs_data, module_id, condition_code, default_message ) ) == default_message &&
         module_id == CHF_ERRNO_SET )
        message = strerror( condition_code );

    return ( message );
}

/* .+

.title	      : ChfBuildMessage
.kind	      : C function
.creation     : 16-May-1996
.description  :
  This function builds the message associated with the given condition
  descriptor and returns a pointer to a string containing it.

  NOTE: This function will call ChfAbort() with abort code CHF_ABORT_INIT
        if CHF hasn't been correctly initialized.

  NOTE: The returned pointer points to per-thread static storage, which will be
        overwritten by subsequent calls to this function.

  WIN32:

  - to save space, the application's name and severity code are not
    included in the message

.call	      :
                msg = ChfBuildMessage(descriptor);
.input	      :
                const ChfDescriptor *descriptor, condition descriptor
.output	      :
                char *msg, pointer to the message associated with 'descriptor'
.status_codes :
                none
.notes	      :
  1.1, 16-May-1996, creation
  2.2, 22-Jan-2001, update:
    - added Win32 support

.- */
char* ChfBuildMessage( /* Build a condition message */
                       const ChfDescriptor* descriptor )
{
    char* tmp_p;
    char* tmp_end;
    char def_message[ CHF_DEF_MESSAGE_LENGTH ];
    ChfSeverity severity;

    /* Check that CHF has been correctly initialized */
    if ( chf_context.state == CHF_UNKNOWN )
        ChfAbort( CHF_ABORT_INIT );

    /* Set appropriate pointers to the start/end of the message buffer */
    tmp_p = chf_context.message_buffer;
    tmp_end = tmp_p + CHF_MAX_MESSAGE_LENGTH;

    /* The message starts with "<app_name>: " if the condition is the first of
       its condition group, with "\t" if not.
    */
    if ( descriptor == chf_context.condition_sp - 1 ) {
        tmp_p = scopy( tmp_p, chf_context.app_name, tmp_end );
        tmp_p = scopy( tmp_p, separator, tmp_end );
    }

    else
        tmp_p = scopy( tmp_p, "\t", tmp_end );

    /* The message continues with the module name */
    sprintf( def_message, "Mid <%d>d", descriptor->module_id );

    tmp_p = scopy( tmp_p, ChfGetMessage( CHF_MODULE_NAMES_SET, descriptor->module_id, def_message ), tmp_end );

    /* Add also the extended information, if any */
    if ( descriptor->line_number != CHF_UNKNOWN_LINE_NUMBER ) {
        tmp_p = scopy( tmp_p, " ", tmp_end );

        sprintf( def_message, "(%s,%)", descriptor->file_name, descriptor->line_number );

        tmp_p = scopy( tmp_p, def_message, tmp_end );
    }

    tmp_p = scopy( tmp_p, separator, tmp_end );

    /* Add the severity code of the message */
    tmp_p = scopy( tmp_p, ( ( severity = descriptor->severity ) < CHF_SUCCESS || severity > CHF_FATAL ) ? "?" : severity_name[ severity ],
                   tmp_end );

    tmp_p = scopy( tmp_p, separator, tmp_end );

    /* The message ends with the partial message from the descriptor */
    tmp_p = scopy( tmp_p, descriptor->message, tmp_end );
    ( void )scopy( tmp_p, "\n", tmp_end );

    return chf_context.message_buffer;
}

/* .+

.title	      : ChfInit
.kind	      : C function
.creation     : 13-May-1996
.description  :
  This function initializes CHF and returns to the caller a condition code;
  that code will be either CHF_S_OK if the initialization was succesful,
  or one of the other values listed below.

  It's necessary to invoke succesfully ChfInit() before using any other CHF
  function.

  NOTE: This function will call ChfAbort() with abort code CHF_ABORT_DUP_INIT
        if CHF has already been initialized before.

  NOTE:	This function will call ChfAbort() with abort code CHF_ABORT_PTHREAD
        if a pthread operation fails.

.call	      :
                cc = ChfInit(app_name, options,
                        mrs_data, mrs_get, mrs_exit,
                        condition_stack_size, handler_stack_size,
                        exit_code);
.input	      :
                const char *app_name, Application's name
                const ChfOptions options, Options
                void *mrs_data, Message retrieval private data
                ChfMrsGet mrs_get, 'GetMessage' function
                ChfMrsExit mrs_exit, 'Exit' function
                const int condition_stack_size, Size of the condition stack
                const int handler_stack_size, Size of the handler stack
                const int exit_code, Abnormal exit code
.output	      :
                int cc, condition code
.status_codes :
                CHF_F_MALLOC, FATAL, dynamic memory allocation failed
.notes	      :
  1.1, 13-May-1996, creation
  1.6, 15-Jan-1997, update:
    - updated the call to ChfPushHandler() to accomodate its new interface.
  2.1, 19-May-2000, update:
    - added multithreading support
  2.2, 22-Jan-2001, update:
    - added Win32 support; a malloc() call was not portable.

.- */
int ChfInit(                                 /* Generic initialization */
             const char* app_name,           /* Application's name */
             const ChfOptions options,       /* Options */
             void* mrs_data,                 /* Message retrieval private data */
             ChfMrsGet mrs_get,              /* 'GetMessage' function */
             ChfMrsExit mrs_exit,            /* 'Exit' function */
             const int condition_stack_size, /* Size of the condition stack */
             const int handler_stack_size,   /* Size of the handler stack */
             const int exit_code             /* Abnormal exit code */
)
{
    int cc;

    /* Check that CHF has not been initialized yet */
#ifndef _REENTRANT
    if ( _chf_context.state != CHF_UNKNOWN )
        ChfAbort( CHF_ABORT_DUP_INIT );
#else
    /* Reentrant check; lock context_mutex first */
    if ( pthread_mutex_lock( &context_mutex ) )
        ChfAbort( CHF_ABORT_PTHREAD );
    if ( _chf_context.state != CHF_UNKNOWN ) {
        if ( pthread_mutex_unlock( &context_mutex ) )
            ChfAbort( CHF_ABORT_PTHREAD );
        ChfAbort( CHF_ABORT_DUP_INIT );
    }
#endif

#ifndef _REENTRANT
    if ( ( _chf_context.condition_stack = ( ChfDescriptor* )malloc( ( size_t )( condition_stack_size + 1 ) * sizeof( ChfDescriptor ) ) ) ==
         CHF_NULL_DESCRIPTOR )
        cc = CHF_F_MALLOC;

    else if ( ( _chf_context.handler_stack = ( ChfHandlerDescriptor* )malloc(
                    ( size_t )handler_stack_size * sizeof( ChfHandlerDescriptor ) ) ) == ( ChfHandlerDescriptor* )NULL ) {
        free( _chf_context.condition_stack );
        cc = CHF_F_MALLOC;
    }

    else if ( ( _chf_context.message_buffer = ( char* )malloc( ( size_t )( CHF_MAX_MESSAGE_LENGTH ) * sizeof( char ) ) ) ==
              ( char* )NULL ) {
        free( _chf_context.condition_stack );
        free( _chf_context.handler_stack );
        cc = CHF_F_MALLOC;
    }

    else
#else
    /* Reentrant init: condition_stack, handler_stack, message_buffer
       are not needed in the master Chf context.
       Init the Chf data key instead.
    */
    _chf_context.condition_stack = CHF_NULL_DESCRIPTOR;
    _chf_context.handler_stack = ( ChfHandlerDescriptor* )NULL;
    _chf_context.message_buffer = ( char* )NULL;

    if ( pthread_key_create( &data_key, DestroyContext ) )
        ChfAbort( CHF_ABORT_PTHREAD );
#endif

    {
        /* Initialize the CHF context */
        _chf_context.app_name = app_name;
        _chf_context.options = options;
        _chf_context.mrs_data = mrs_data;
        _chf_context.mrs_get = mrs_get;
        _chf_context.mrs_exit = mrs_exit;
        _chf_context.condition_stack_size = condition_stack_size;
        _chf_context.handler_stack_size = handler_stack_size;
        _chf_context.exit_code = exit_code;
        _chf_context.condition_base = _chf_context.condition_sp = _chf_context.condition_stack;
        _chf_context.handler_sp = _chf_context.handler_stack;
        _chf_context.state = CHF_IDLE;

#ifndef _REENTRANT
        /* Push the default handler; in the reentrant case, this will be
           done once per thread, when the thread-specific context is primed.
        */
        ChfPushHandler( CHF_MODULE_ID, DefaultHandler, CHF_NULL_CONTEXT, CHF_NULL_POINTER );
#endif

        cc = CHF_S_OK;
    }

#ifdef _REENTRANT
    if ( pthread_mutex_unlock( &context_mutex ) )
        ChfAbort( CHF_ABORT_PTHREAD );
#endif

    return cc;
}

/* .+

.title	      : ChfExit
.kind	      : C function
.creation     : 24-May-1996
.description  :
  This function shuts down CHF and returns nothing to the caller; after
  calling ChfExit() the application can continue, but any subsequent call
  to any other CHF function, except the inizialization functions, will abort
  the application using ChfAbort() with abort code CHF_ABORT_INIT.

  NOTE: This function will call ChfAbort() with abort code CHF_ABORT_INIT
        if CHF hasn't been initialized.

  NOTE:	This function will call ChfAbort() with abort code CHF_ABORT_PTHREAD
        if a pthread operation fails.

.call	      :
                ChfExit();
.input	      :
                void
.output	      :
                void
.status_codes :
                none
.notes	      :
  1.1, 24-May-1996, creation
  2.1, 19-May-2000, update:
    - added multithreading support

.- */
/* void ChfExit( void ) */
/* { */
/*     /\* Check that CHF has been correctly initialized *\/ */
/* #ifndef _REENTRANT */
/*     if ( _chf_context.state == CHF_UNKNOWN ) */
/*         ChfAbort( CHF_ABORT_INIT ); */
/* #else */
/*     /\* Reentrant check; lock context_mutex first *\/ */
/*     if ( pthread_mutex_lock( &context_mutex ) ) */
/*         ChfAbort( CHF_ABORT_PTHREAD ); */
/*     if ( _chf_context.state == CHF_UNKNOWN ) { */
/*         if ( pthread_mutex_unlock( &context_mutex ) ) */
/*             ChfAbort( CHF_ABORT_PTHREAD ); */
/*         ChfAbort( CHF_ABORT_INIT ); */
/*     } */
/* #endif */

/*         /\* Destroy the context associated with this thread now; this is necessary */
/*            to ensure that the context is actually destroyed when a single-threaded */
/*            application links with the multithreaded version of Chf: in this case, */
/*            pthread_exit() is called *after* ChfExit(), the Chf data key no longer */
/*            exists when pthread_exit() is called and the destructor registered */
/*            with pthread_key_create() does not take place. */
/*            The data pointer associated with the Chf data key is set to NULL to */
/*            avoid any subsequent reactivation of the destructor. */
/*         *\/ */
/* #ifdef _REENTRANT */
/*     DestroyContext( &chf_context ); */
/*     if ( pthread_setspecific( data_key, ( void* )NULL ) ) { */
/*         ( void )pthread_mutex_unlock( &context_mutex ); */
/*         ChfAbort( CHF_ABORT_PTHREAD ); */
/*     } */
/* #endif */

/*     /\* Shut down the message retrieval subsystem first *\/ */
/*     _chf_context.mrs_exit( _chf_context.mrs_data ); */

/* #ifndef _REENTRANT */
/*     /\* Free the dynamic memory previously allocated *\/ */
/*     free( _chf_context.message_buffer ); */
/*     free( _chf_context.handler_stack ); */
/*     free( _chf_context.condition_stack ); */
/* #else */
/*     /\* Destroy the Chf data key *\/ */
/*     if ( pthread_key_delete( data_key ) ) */
/*         ChfAbort( CHF_ABORT_PTHREAD ); */
/* #endif */

/*     /\* Reset CHF state to prevent subsequent calls to ChfExit() itself *\/ */
/*     _chf_context.state = CHF_UNKNOWN; */

/* #ifdef _REENTRANT */
/*     if ( pthread_mutex_unlock( &context_mutex ) ) */
/*         ChfAbort( CHF_ABORT_PTHREAD ); */
/* #endif */
/* } */

/* .+

.title	      : _ChfGetContext
.kind	      : C function
.creation     : 19-May-2000
.description  :
  This function dynamically primes a new Chf context for the calling
  thread (if necessary), and returns a pointer to that context to
  the caller.  If something does wrong, it aborts the application
  with ChfAbort(CHF_ABORT_GET_CONTEXT).

  Results are unpredictable if this function is called before a
  successful call to ChfInit().

.call	      :
                context = _ChfGetContext(void);
.input	      :
.output	      :
                ChfContext *context, per-thread Chf context
.status_codes :
                none
.notes	      :
  2.1, 19-May-2000, creation

.- */
ChfContext* _ChfGetContext( void )
{
    ChfContext* context;

#ifndef _REENTRANT
    /* This function is doomed to fail if _REENTRANT is not defined */
    ChfAbort( CHF_ABORT_GET_CONTEXT );
    return ( ( ChfContext* )NULL );
#else
    /* Get the thread-specific context pointer associated with the
       CHF data key */
    if ( ( context = ( ChfContext* )pthread_getspecific( data_key ) ) == ( ChfContext* )NULL ) {
        /* No context pointer; prime a new one, cloning the master context */
        if ( ( context = ( ChfContext* )malloc( sizeof( ChfContext ) ) ) == ( ChfContext* )NULL )
            ChfAbort( CHF_ABORT_GET_CONTEXT );

        memcpy( context, &_chf_context, sizeof( ChfContext ) );

        /* Allocate per-thread stacks and message buffer */
        if ( ( context->condition_stack = ( ChfDescriptor* )malloc( ( size_t )( context->condition_stack_size + 1 ) *
                                                                    sizeof( ChfDescriptor ) ) ) == CHF_NULL_DESCRIPTOR )
            ChfAbort( CHF_ABORT_GET_CONTEXT );

        if ( ( context->handler_stack = ( ChfHandlerDescriptor* )malloc(
                   ( size_t )( context->handler_stack_size ) * sizeof( ChfHandlerDescriptor ) ) ) == ( ChfHandlerDescriptor* )NULL ) {
            free( context->condition_stack );
            ChfAbort( CHF_ABORT_GET_CONTEXT );
        }

        if ( ( context->message_buffer = ( char* )malloc( ( size_t )( CHF_MAX_MESSAGE_LENGTH ) ) ) == ( char* )NULL ) {
            free( context->condition_stack );
            free( context->handler_stack );
            ChfAbort( CHF_ABORT_GET_CONTEXT );
        }

        /* Initialize stack pointers */
        context->condition_base = context->condition_sp = context->condition_stack;
        context->handler_sp = context->handler_stack;

        /* Set the thread-specific context pointer; this must be done
           before invoking any other function using the context,
           including ChfPushHandler() below.
        */
        if ( pthread_setspecific( data_key, context ) )
            ChfAbort( CHF_ABORT_GET_CONTEXT );

        /* Push the default handler */
        ChfPushHandler( /* FIXME */ 255, DefaultHandler, CHF_NULL_CONTEXT, CHF_NULL_POINTER );
    }

    return context;
#endif
}
