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
  - Added structured condition handling macros: ChfTry, ChfCatch, ChfEndTry

  Revision 1.6  1997/01/15  13:41:20  cibrario
  Defined the new data type ChfPointer, a generic (void *) pointer. Each
  condition handler can have a private handler context pointer, of type
  ChfPointer, that the function ChfPushHandler() stores and that is passed
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


/* -------------------------------------------------------------------------
   Win32 & UNICODE support
   ------------------------------------------------------------------------- */

#ifdef _WIN32
#define ChfChar			TCHAR
#define ChfText(x)		_T(x)
#define ChfSigjmp_buf		jmp_buf
#define ChfSigsetjmp(x,y)	setjmp(x)
#define ChfSiglongjmp(x,y)	longjmp(x,y)
#else
#define ChfChar			char
#define ChfText(x)		x
#define ChfSigjmp_buf		sigjmp_buf
#define ChfSigsetjmp(x,y)	sigsetjmp(x,y)
#define ChfSiglongjmp(x,y)	siglongjmp(x,y)
#endif


/* -------------------------------------------------------------------------
   CHF implementation limits and other symbolic constants
   ------------------------------------------------------------------------- */

#define CHF_MAX_MESSAGE_LENGTH	256
#define CHF_UNKNOWN_LINE_NUMBER	(-1)
#define CHF_UNKNOWN_FILE_NAME	(ChfChar *)NULL
#define CHF_NULL_DESCRIPTOR	(ChfDescriptor *)NULL
#define CHF_NULL_CONTEXT	(void *)NULL
#define CHF_NULL_POINTER	(ChfPointer *)NULL
#define CHF_NULL_HANDLER	(ChfHandler)NULL
#define CHF_LIBRARY_ID		ChfText("$Id: Chf.h,v 2.2 2001/01/25 11:56:44 cibrario Exp $")

#define CHF_MAJOR_RELEASE_NUMBER	2
#define CHF_MINOR_RELEASE_NUMBER	2

#define CHF_MODULE_NAMES_SET	1
#define CHF_SET			2
#define CHF_ERRNO_SET		3


/* -------------------------------------------------------------------------
   Condition codes
   ------------------------------------------------------------------------- */
  
#define CHF_S_OK		0
#define CHF_F_COND_STACK_FULL	1	/* Condition stack is full */
#define CHF_F_HDLR_STACK_FULL	2	/* Handler stack is full */
#define CHF_F_HDLR_STACK_EMPTY	3	/* Handler stack is empty */
#define CHF_F_BAD_STATE		4	/* Bad CHF state for req. operation */
#define CHF_F_INVALID_ACTION	5	/* Invalid action from handler: %d */
#define CHF_F_MALLOC		6	/* Dynamic memory allocation failed */
#define CHF_F_NOT_AVAILABLE	7	/* Function not available */
#define CHF_F_SETLOCALE		10	/* setlocale() failed */
#define CHF_F_CATOPEN		11	/* catopen() failed */


/* -------------------------------------------------------------------------
   Type definitions
   ------------------------------------------------------------------------- */

typedef enum			/* Condition severity codes */
{
  CHF_SUCCESS,
  CHF_INFO,				
  CHF_WARNING,
  CHF_ERROR,
  CHF_FATAL
}
  ChfSeverity;

typedef enum			/* Condition handler action codes */
{
  CHF_CONTINUE,				/* Continue application */
  CHF_RESIGNAL,				/* Resignal to next handler */
  CHF_UNWIND,				/* Stack unwind */
  CHF_UNWIND_KEEP			/* Unwind, keep last cond. group */
}
  ChfAction;

typedef int			/* CHF options */
  ChfOptions;

#define CHF_DEFAULT		0x0000	/* default flags */
#define CHF_ABORT		0x0001	/* use abort() instead of exit() */

typedef enum			/* Current CHF state */
{
  CHF_UNKNOWN,
  CHF_IDLE,
  CHF_SIGNALING,
  CHF_UNWINDING,
  CHF_SIGNAL_UNWINDING
}
  ChfState;

typedef struct ChfDescriptor_S	/* Condition descriptor */
{
  int module_id;			/* Module identifier */
  int condition_code;			/* Condition code */
  ChfSeverity severity;			/* Severity */
  int line_number;			/* Line # or CHF_UNK_LINE_NUMBER */
  const ChfChar *file_name;		/* File name or CHF_UNK_FILE_NAME */
  ChfChar message[CHF_MAX_MESSAGE_LENGTH];	/* Partial message */
  struct ChfDescriptor_S *next;		/* Link to next descriptor */
}
  ChfDescriptor;

typedef struct ChfTable_S	/* Standalone message table */
{
  int module;				/* Module identifier */
  int code;				/* Condition code */
  ChfChar *msg_template;		/* Message template */
}
  ChfTable;

typedef				/* Generic pointer */
  void *ChfPointer;

typedef				/* Condition handler */
  ChfAction (*ChfHandler)(
    const ChfDescriptor *,
    const ChfState,
    ChfPointer
);

typedef				/* Message retrieval 'get_message' function */
  const ChfChar * (*ChfMrsGet)(
    void *,
    const int,
    const int,
    const ChfChar *default_message
);

typedef				/* Message retrieval 'exit' function */
  void (*ChfMrsExit)(
    void *
);


/* -------------------------------------------------------------------------
   Condition generation macros
   ------------------------------------------------------------------------- */

#if defined(CHF_EXTENDED_INFO)
#define ChfCondition							\
  ChfGenerate(								\
    CHF_MODULE_ID,							\
    ChfText(__FILE__), __LINE__,

#ifdef _WIN32
#define ChfErrnoCondition
#else
#define ChfErrnoCondition						\
  ChfGenerate(								\
    CHF_ERRNO_SET,							\
    ChfText(__FILE__), __LINE__,					\
    errno,								\
    CHF_ERROR								\
  )
#endif

#else
#define ChfCondition							\
  ChfGenerate(								\
    CHF_MODULE_ID,							\
    CHF_UNKNOWN_FILE_NAME, CHF_UNKNOWN_LINE_NUMBER,

#ifdef _WIN32
#define ChfErrnoCondition
#else
#define ChfErrnoCondition						\
  ChfGenerate(								\
    CHF_ERRNO_SET,							\
    CHF_UNKNOWN_FILE_NAME, CHF_UNKNOWN_LINE_NUMBER,			\
    errno,								\
    CHF_ERROR								\
  )
#endif

#endif

#define ChfEnd								\
  )
 

/* -------------------------------------------------------------------------
   Structured condition handling
   ------------------------------------------------------------------------- */

#define ChfTry \
  {\
    ChfSigjmp_buf _chf_sigjmp_buf;\
    if(ChfSigsetjmp(_chf_sigjmp_buf, 1) == 0)\
    {\
      ChfPushHandler(CHF_NULL_HANDLER, _chf_sigjmp_buf, CHF_NULL_POINTER);

#define ChfCatch \
      ChfPopHandler();\
    }\
    else\
    {

#define ChfEndTry \
      ChfDiscard();\
    }\
  }


/* -------------------------------------------------------------------------
   Other macros
   ------------------------------------------------------------------------- */

#define ChfGetNextDescriptor(d)	(d)->next
#define ChfGetModuleId(d)	(d)->module_id
#define ChfGetConditionCode(d)	(d)->condition_code
#define ChfGetSeverity(d)	(d)->severity
#define ChfGetLineNumber(d)	(d)->line_number
#define ChfGetFileName(d)	(d)->file_name
#define ChfGetPartialMessage(d)	(d)->message



/* -------------------------------------------------------------------------
   Function prototypes
   ------------------------------------------------------------------------- */

int ChfInit(			/* Generic initialization */
  const ChfChar *app_name,		/* Application's name */
  const ChfOptions options,		/* Options */
  void *mrs_data,			/* Message retrieval private data */
  ChfMrsGet mrs_get,			/* 'GetMessage' function */
  ChfMrsExit mrs_exit,			/* 'Exit' function */
  const int condition_stack_size,	/* Size of the condition stack */
  const int handler_stack_size,		/* Size of the handler stack */
  const int exit_code			/* Abnormal exit code */
);

int ChfMsgcatInit(		/* Initialization with msgcat subsystem */
  const ChfChar *app_name,		/* Application's name */
  const ChfOptions options,		/* Options */
  const ChfChar *msgcat_name,		/* Name of the message catalog */
  const int condition_stack_size,	/* Size of the condition stack */
  const int handler_stack_size,		/* Size of the handler stack */
  const int exit_code			/* Abnormal exit code */
);

int ChfStaticInit(		/* Initialization with static message tables */
  const ChfChar *app_name,		/* Application's name */
  const ChfOptions options,		/* Options */
  const ChfTable *table,		/* Static message table */
  const size_t table_size,		/* Size of the message table */
  const int condition_stack_size,	/* Size of the condition stack */
  const int handler_stack_size,		/* Size of the handler stack */
  const int exit_code			/* Abnormal exit code */
);

int ChfWin32Init(		/* Initialization within _WIN32 */
  const ChfChar *app_name,		/* Application's name */
  const ChfOptions options,		/* Options */
#ifndef _WIN32
  void *instance,			/* Fake arguments */
#else
  HINSTANCE instance,			/* App. instance handle */
#endif
  const int condition_stack_size,	/* Size of the condition stack */
  const int handler_stack_size,		/* Size of the handler stack */
  const int exit_code			/* Abnormal exit code */
);

void ChfExit(			/* Exit */
  void
);

void ChfAbort(			/* Abort application */
  const int abort_code
);

void ChfPushHandler(		/* Push a new handler into the stack */
  ChfHandler new_handler,		/* Handler to be added */
  void *unwind_context,			/* Unwind context */
  ChfPointer handler_context		/* Private handler context */
);

void ChfPopHandler(		/* Pop a handler */
  void
);

ChfChar *ChfBuildMessage(	/* Build a condition message */
  const ChfDescriptor *descriptor
);

void ChfSignal(			/* Signal the current conditions */
  void
);

void ChfDiscard(		/* Discard the current conditions */
  void
);

void ChfGenerate(		/* Generate a condition into the stack */
  const int module_id,
  const ChfChar *file_name,
  const int line_number,
  const int condition_code,
  const ChfSeverity severity,
  ...
);

const ChfChar *ChfGetMessage(	/* Retrieve a condition message */
  const int module_id,
  const int condition_code,
  const ChfChar *default_message
);

const ChfDescriptor *ChfGetTopCondition(	/* Retrieve top condition */
  void
);
