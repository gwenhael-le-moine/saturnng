/* .+

.author	      : Ivan Cibrario B.

.creation     :	17-May-1996

.description  :
  This module contains the CHF initialization function ChfMsgcatInit()

.notes	      :
  $Log: chf_msgc.c,v $
  Revision 2.2  2001/01/25 14:06:47  cibrario
  Added partial Win32 support (Windows CE only).

  Revision 1.3  1996/06/21  14:19:22  cibrario
  Bug fix: the private context of the message retrieval facility was
  never freed by ExitMessage()

  Revision 1.1  1996/05/28  12:55:15  cibrario
  Initial revision
.- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <nl_types.h>

#include "Chf.h"
#include "ChfPriv.h"

/* -------------------------------------------------------------------------
   Private type definitions
   ------------------------------------------------------------------------- */

typedef struct {
    nl_catd catalog; /* Message catalog descriptor */
} ChfMsgcatContext;

/* -------------------------------------------------------------------------
   Private functions
   ------------------------------------------------------------------------- */

static const char* GetMessage( void* private_context, const int module_id, const int condition_code, const char* default_message )
{
    return ( catgets( ( ( ChfMsgcatContext* )private_context )->catalog, module_id, condition_code, default_message ) );
}

static void ExitMessage( void* private_context )
{
    ( void )catclose( ( ( ChfMsgcatContext* )private_context )->catalog );
    free( private_context );
}
