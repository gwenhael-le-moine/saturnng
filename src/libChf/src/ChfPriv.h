#ifndef _CHF_PRIV_H
#define _CHF_PRIV_H 1

/* .+

.identifier   : $Id: ChfPriv.h,v 2.2 2001/01/25 11:57:57 cibrario Exp $
.context      : CHF, Condition Handling Facility
.title	      : $RCSfile: ChfPriv.h,v $, main header
.kind	      : C header
.author	      : Ivan Cibrario B.
.site	      : CSTV-CNR
.creation     :	 2-May-1996
.keywords     : *
.description  :
  This is the private header of the Condition Handling Facility

.include      : *

.notes	      :
  $Log: ChfPriv.h,v $
  Revision 2.2  2001/01/25 11:57:57  cibrario
  Added partial Win32 support (Windows CE only).

  Revision 2.1  2000/05/26  14:14:36  cibrario
  - Defined new abort codes: CHF_ABORT_GET_CONTEXT, CHF_ABORT_PTHREAD
  - Redefined .unwind_context field of ChfHandlerDescriptor
  - Conditional retarget of chf_context for multithreading support
  - Declared new private function: _ChfGetContext() (mt support only)

  Revision 1.6  1997/01/15  13:38:24  cibrario
  Added the new field .handler_context to struct ChfHandlerDescriptor_S, to
  store, for each condition handler, the private handler context pointer.

  Revision 1.5  1996/10/04  09:43:51  cibrario
  Updated the condition message format to improve internationalization

  Revision 1.1  1996/05/28  12:56:37  cibrario
  Initial revision


.- */

#include "Chf.h"

/* -------------------------------------------------------------------------
   Macros
   ------------------------------------------------------------------------- */

#define CHF_MODULE_ID CHF_SET
#define CHF_TMP_MESSAGE_LENGTH ( 2 * CHF_MAX_MESSAGE_LENGTH )
#define CHF_DEF_MESSAGE_LENGTH 40

/* -------------------------------------------------------------------------
   Abort codes used with ChfAbort()
   ------------------------------------------------------------------------- */

#define CHF_ABORT_SILENT 0
#define CHF_ABORT_INIT 1
#define CHF_ABORT_MSG_OVF 2
#define CHF_ABORT_INVALID_ACTION 3
#define CHF_ABORT_DUP_INIT 4
#define CHF_ABORT_ALREADY_UNWINDING 5
#define CHF_ABORT_IMPROPERLY_HANDLED 6
#define CHF_ABORT_FATAL_UNWINDING 7
#define CHF_ABORT_COND_STACK_OVF 8
#define CHF_ABORT_GET_CONTEXT 9
#define CHF_ABORT_PTHREAD 10

/* -------------------------------------------------------------------------
   Type definitions
   ------------------------------------------------------------------------- */

typedef struct ChfHandlerDescriptor_S {
    ChfHandler handler;
    void* unwind_context;
    void* handler_context;
} ChfHandlerDescriptor;

/* CHF Context */
typedef struct ChfContext_S {
    ChfState state;                      /* Current CHF state */
    const char* app_name;                /* Application's name */
    ChfOptions options;                  /* Options */
    void* mrs_data;                      /* Message retrieval private data */
    ChfMrsGet mrs_get;                   /* 'GetMessage' function */
    ChfMrsExit mrs_exit;                 /* 'Exit' function */
    int condition_stack_size;            /* Size of the condition stack */
    int handler_stack_size;              /* Size of the handler stack */
    int exit_code;                       /* Abnormal exit code */
    ChfDescriptor* condition_stack;      /* Condition stack */
    ChfDescriptor* condition_base;       /* Current condition stack base */
    ChfDescriptor* condition_sp;         /* Current condition stack pointer */
    ChfHandlerDescriptor* handler_stack; /* Handler stack */
    ChfHandlerDescriptor* handler_sp;    /* Current handler stack pointer */
    char* message_buffer;                /* Message buffer */
} ChfContext;

/* -------------------------------------------------------------------------
   Multithreading support
 ------------------------------------------------------------------------- */
#ifdef _REENTRANT
#  define chf_context ( *_ChfGetContext() )
#else
#  define chf_context _chf_context
#endif

/* -------------------------------------------------------------------------
   Structured condition handling
   ------------------------------------------------------------------------- */

/* #define CHF_Try \ */
/*     { \ */
/*         sigjmp_buf _chf_sigjmp_buf; \ */
/*         if ( sigsetjmp( _chf_sigjmp_buf, 1 ) == 0 ) { \ */
/*             ChfPushHandler( CHF_NULL_HANDLER, _chf_sigjmp_buf, CHF_NULL_POINTER ); */

/* #define CHF_Catch \ */
/*     ChfPopHandler( CHF_MODULE_ID ); \ */
/*     } \ */
/*     else \ */
/*     { */

/* #define CHF_EndTry \ */
/*     ChfDiscard(); \ */
/*     } \ */
/*     } */

/* -------------------------------------------------------------------------
   Global variables
 ------------------------------------------------------------------------- */

extern ChfContext _chf_context; /* CHF Context */

/* -------------------------------------------------------------------------
   Private function prototypes
 ------------------------------------------------------------------------- */
#ifdef _REENTRANT
ChfContext* _ChfGetContext( void );
#endif

/* Generic initialization */
int ChfInit( const char* app_name,           /* Application's name */
             const ChfOptions options,       /* Options */
             void* mrs_data,                 /* Message retrieval private data */
             ChfMrsGet mrs_get,              /* 'GetMessage' function */
             ChfMrsExit mrs_exit,            /* 'Exit' function */
             const int condition_stack_size, /* Size of the condition stack */
             const int handler_stack_size,   /* Size of the handler stack */
             const int exit_code             /* Abnormal exit code */
);

/* Initialization with msgcat subsystem */
/* int ChfMsgcatInit( const char* app_name,           /\* Application's name *\/ */
/*                    const ChfOptions options,       /\* Options *\/ */
/*                    const char* msgcat_name,        /\* Name of the message catalog *\/ */
/*                    const int condition_stack_size, /\* Size of the condition stack *\/ */
/*                    const int handler_stack_size,   /\* Size of the handler stack *\/ */
/*                    const int exit_code             /\* Abnormal exit code *\/ */
/* ); */

/* /\* Pop a handler *\/ */
/* void ChfPopHandler( const int module_id ); */

/* /\* Discard the current conditions *\/ */
/* void ChfDiscard( void ); */

/* Exit */
/* void ChfExit( void ); */

/* Abort application */
void ChfAbort( const int abort_code );

/* Build a condition message */
char* ChfBuildMessage( const ChfDescriptor* descriptor );

/* Retrieve top condition */
/* const ChfDescriptor* ChfGetTopCondition( const int module_id ); */

/* Retrieve a condition message */
const char* ChfGetMessage( const int module_id, const int condition_code, const char* default_message );

#endif /*!_CHF_PRIV_H*/
