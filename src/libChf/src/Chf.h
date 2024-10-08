#ifndef _CHF_H
#define _CHF_H 1

/* .+

.identifier   : $Id: Chf.h,v 2.2 2001/01/25 11:56:44 cibrario Exp $
.context      : CHF, Condition Handling Facility
.title	      : $RCSfile: Chf.h,v $, main header
.kind	      : C header
.author	      : Ivan Cibrario B.
.site	      : CSTV-CNR
.creation     :	 2-May-1996
.keywords     : *
.description  :
  This is the main header of the Condition Handling Facility

.include      : stdio.h setjmp.h (Win32: tchar.h)

.notes	      :
  $Log: Chf.h,v $
  Revision 2.2  2001/01/25 11:56:44  cibrario
  Added partial Win32 support (Windows CE only).

  Revision 2.1  2000/05/26 14:10:08  cibrario
  - Revised unwind context passing mechanism; redefined CHF_NULL_CONTEXT
  - New macros: CHF_NULL_HANDLER, CHF_MAJOR_RELEASE_NUMBER,
    CHF_MINOR_RELEASE_NUMBER
  - New ChfAction value: CHF_UNWIND_KEEP; fixed spelling of ChfAction value:
    CHF_SIGNALLING -> CHF_SIGNALING
  - Added structured condition handling macros: CHF_Try, CHF_Catch, CHF_EndTry

  Revision 1.6  1997/01/15  13:41:20  cibrario
  Defined the new data type void*, a generic (void *) pointer. Each
  condition handler can have a private handler context pointer, of type
  void*, that the function ChfPushHandler() stores and that is passed
  to the handler when it's activated.
  Fixed a wrong adjustment of the condition handlers stack pointer after
  an unwind operation.

  Revision 1.5  1996/10/04  09:45:30  cibrario
  Updated the condition message format in the private header ChfPriv.h to
  improve internationalization

  Revision 1.4  1996/09/25  13:21:11  cibrario
  Added macro CHF_LIBRARY_ID; it contains the current ID of the CHF library.
  The module chf_init.o will contain it as a static char[] variable.

  Revision 1.2  1996/06/11  13:02:10  cibrario
  Added prototype for ChfGetTopCondition()

  Revision 1.1  1996/05/28  12:56:47  cibrario
  Initial revision


.- */

#include <stdlib.h>
#include <errno.h>

/* -------------------------------------------------------------------------
   CHF implementation limits and other symbolic constants
   ------------------------------------------------------------------------- */

#define CHF_MAX_MESSAGE_LENGTH 256
#define CHF_UNKNOWN_LINE_NUMBER ( -1 )
#define CHF_UNKNOWN_FILE_NAME ( char* )NULL
#define CHF_NULL_DESCRIPTOR ( ChfDescriptor* )NULL
#define CHF_NULL_CONTEXT ( void* )NULL
#define CHF_NULL_POINTER ( void** )NULL
#define CHF_NULL_HANDLER ( ChfHandler ) NULL

#define CHF_MODULE_NAMES_SET 1
#define CHF_SET 2
#define CHF_ERRNO_SET 3

/* -------------------------------------------------------------------------
   Condition codes
   ------------------------------------------------------------------------- */

#define CHF_S_OK 0
#define CHF_F_COND_STACK_FULL 1  /* Condition stack is full */
#define CHF_F_HDLR_STACK_FULL 2  /* Handler stack is full */
#define CHF_F_HDLR_STACK_EMPTY 3 /* Handler stack is empty */
#define CHF_F_BAD_STATE 4        /* Bad CHF state for req. operation */
#define CHF_F_INVALID_ACTION 5   /* Invalid action from handler: %d */
#define CHF_F_MALLOC 6           /* Dynamic memory allocation failed */
#define CHF_F_NOT_AVAILABLE 7    /* Function not available */
#define CHF_F_SETLOCALE 10       /* setlocale() failed */
#define CHF_F_CATOPEN 11         /* catopen() failed */

/* -------------------------------------------------------------------------
   Type definitions
   ------------------------------------------------------------------------- */

/* Condition severity codes */
typedef enum { CHF_SUCCESS, CHF_INFO, CHF_WARNING, CHF_ERROR, CHF_FATAL } ChfSeverity;

/* Condition handler action codes */
typedef enum {
    CHF_CONTINUE,   /* Continue application */
    CHF_RESIGNAL,   /* Resignal to next handler */
    CHF_UNWIND,     /* Stack unwind */
    CHF_UNWIND_KEEP /* Unwind, keep last cond. group */
} ChfAction;

#define CHF_DEFAULT 0x0000 /* default flags */
#define CHF_ABORT 0x0001   /* use abort() instead of exit() */

/* Current CHF state */
typedef enum { CHF_UNKNOWN, CHF_IDLE, CHF_SIGNALING, CHF_UNWINDING, CHF_SIGNAL_UNWINDING } ChfState;

/* Condition descriptor */
typedef struct ChfDescriptor_S {
    int module_id;                          /* Module identifier */
    int condition_code;                     /* Condition code */
    ChfSeverity severity;                   /* Severity */
    int line_number;                        /* Line # or CHF_UNK_LINE_NUMBER */
    const char* file_name;                  /* File name or CHF_UNK_FILE_NAME */
    char message[ CHF_MAX_MESSAGE_LENGTH ]; /* Partial message */
    struct ChfDescriptor_S* next;           /* Link to next descriptor */
} ChfDescriptor;

/* Standalone message table */
typedef struct ChfTable_S {
    int module;               /* Module identifier */
    int code;                 /* Condition code */
    const char* msg_template; /* Message template */
} ChfTable;

/* Condition handler */
typedef ChfAction ( *ChfHandler )( const ChfDescriptor*, const ChfState, void* );

/* Message retrieval 'get_message' function */
typedef const char* ( *ChfMrsGet )( void*, const int, const int, const char* default_message );

/* Message retrieval 'exit' function */
typedef void ( *ChfMrsExit )( void* );

/* -------------------------------------------------------------------------
   Function prototypes
   ------------------------------------------------------------------------- */
/* Generate a condition into the stack */
void ChfGenerate( const int module_id, const char* file_name, const int line_number, const int condition_code, const ChfSeverity severity,
                  ... );

/******************/
/* used in saturn */
/******************/
/* Initialization with static message tables */
int ChfStaticInit( const int module_id, const char* app_name, /* Application's name */
                   const int options,                         /* Options */
                   const ChfTable* table,                     /* Static message table */
                   const size_t table_size,                   /* Size of the message table */
                   const int condition_stack_size,            /* Size of the condition stack */
                   const int handler_stack_size,              /* Size of the handler stack */
                   const int exit_code                        /* Abnormal exit code */
);

/* Push a new handler into the stack */
void ChfPushHandler( const int module_id, ChfHandler new_handler, /* Handler to be added */
                     void* unwind_context,                        /* Unwind context */
                     void* handler_context                        /* Private handler context */
);

/* Signal the current conditions */
void ChfSignal( const int module_id );

/* Exit */
void ChfExit( void );

#endif /*!_CHF_H*/
