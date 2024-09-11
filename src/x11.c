/* -------------------------------------------------------------------------
   saturn - A poor-man's emulator of some HP calculators
   Copyright (C) 1998-2000 Ivan Cibrario Bertolotti

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the documentation of this program; if not, write to
   the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

   For more information, please contact the author, preferably by email,
   at the following address:

   Ivan Cibrario Bertolotti
   IRITI - National Research Council
   c/o IEN "Galileo Ferraris"
   Strada delle Cacce, 91
   10135 - Torino (ITALY)

   email: cibrario@iriti.cnr.it
   ------------------------------------------------------------------------- */

/* +-+ */

/* .+

.identifier   : $Id: x11.c,v 4.1.1.1 2002/11/11 16:11:47 cibrario Exp $
.context      : SATURN, Saturn CPU / HP48 emulator
.title	      : $RCSfile: x11.c,v $
.kind	      : C source
.author	      : Ivan Cibrario B.
.site	      : CSTV-CNR
.creation     :	5-Sep-2000
.keywords     : *
.description  :
  This file contains the X Window System interface of the emulator.

  It has been designed and implemented from scratch in less than three days;
  this means that it is far from being nice/clean/whatever, and has been
  thrown in with the sole purpose of giving a friendlier and somewhat
  customizable interface to the Saturn emulator, and to prepare for further
  improvments.

  Widget/resource naming conventions

  reset			Reset machine at startup
  monitor		Enter monitor at startup
  stateDir		Machine state directory
  cpu			Machine state file names (relative to stateDir, above)
  mod
  hdw
  rom
  ram
  port1
  port2
  hw			Select hardware configuration
  face			Select machine faceplace and keyboard layout

  main.<face>			XmRowColumn container
  main.<face>.nKeys		Number of keys for the given face
  main.<face>.kbd		XmForm container of individual keys
  main.<face>.kbd.<n>		XmForm for key #n, 0 <= n < nKeys
  main.<face>.kbd.<n>.ul	Upper left XmLabel of key #n
  main.<face>.kbd.<n>.ur	Upper right XmLabel of key #n
  main.<face>.kbd.<n>.ll	Lower left XmLabel of key #n
  main.<face>.kbd.<n>.lr	Lower right XmLabel of key #n
  main.<face>.kbd.<n>.btn	XmToggleButton of key #n
  main.<face>.kbd.<n>.btn.inOut	IN/OUT codes of key #n as a string
  main.<face>.frame		XmFrame for LCD screen display
  main.<face>.frame.lcd		XmDrawingArea for LCD screen display

  For all XmLabel and XmToggleButton widgets associated with the emulated
  keyboard, the application's resource:

  .compoundString

  can be set to override the labelString String resource with a XmString;
  this is necessary, for example, to let a label use mutiple fonts.
  The function CheckCompoundString() below does the translation.

.include      : config.h, machdep.h, cpu.h, x11.h

.notes	      :
  $Log: x11.c,v $
  Revision 4.1.1.1  2002/11/11 16:11:47  cibrario
  Small screen support; preliminary

  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.17  2000/11/23 17:03:01  cibrario
  Implemented sutil library and assorted bug fixes:
  - main GUI components must be reset in ErrorDialogHandler()

  Revision 3.16  2000/11/21 16:43:04  cibrario
  Ultrix/IRIX support:
  - Removed XmCR_APPLY callback from File Selection Box

  Revision 3.15  2000/11/15 14:13:57  cibrario
  GUI enhancements and assorted bug fixes:

  - Added initialization of information/error popup dialog box and
    of message TextField in main window.

  - Added handling of command-line option batchXfer and associated
    top-level application resource

  - Added Chf condition handler and related installation procedure
    to redirect to a GUI element, a popup dialog box, all non-fatal
    messages

  - All popup shells now have allowShellResize set to True (to allow
    them to accommodate children's size changes) and deleteResponse
    set to XmDO_NOTHING (to prevent their untimely destruction when
    WM_CLOSE is invoked on them)

  - Added a sanity check to abort the application if the active
    faceplate has no keys

  Revision 3.14  2000/11/13 10:44:48  cibrario
  Implemented fast load/save; improved keyboard interface emulation at
  high emulated CPU speed:

  - In ActivateFSB(), force a search action in the file selection box and
    ensure that the default file name is properly qualified.  This ensures
    that all files show up, including those created after the last
    activation, and prevents ambiguity when the application's current
    directory is not the same as the fsb's one.

  Revision 3.13  2000/11/09 11:37:09  cibrario
  Revised to add file selection box GUI element, CPU halt/run
  requests and emulator's extended functions:

  - Implemented new functions:
    fsbButtonPressed() (Xt Callback),
    InitializeFSB() (initialization function, invoked by InitializeGui()),
    ResetToggleButtons() (main GUI reset function)
    ActivateFSB() (public function to activate the file selection box)

  Revision 3.10  2000/10/24 16:15:03  cibrario
  Added/Replaced GPL header

  Revision 3.5  2000/10/02 09:53:30  cibrario
  Linux support:
  - added an explicit exit() invocation in deleteWindow(); the default
    Motif application's destructor is not invoked with Debian
    lesstif 0.89.4-3 when deleteWindow() is registered.

  Revision 3.1  2000/09/20 14:08:25  cibrario
  Revised to implement passive CPU shutdown:
  - implemented IdleXLoop() function and its ancillary Xt timeout
    handler, IdleTimeOutHandler()

 * Revision 2.8  2000/09/19  12:55:18  cibrario
 * The translation table of ToggleButtons is no longer set during
 * widget creation, to be able to install default keyboard
 * translations, via app-resource file, to buttons too.
 * Instead, XtAugmentTranslations() is used after widget creation.
 *
 * Revision 2.3  2000/09/12  12:30:42  cibrario
 * Bug fix: signalling of X11_I_REVISION (X11_RCS_INFO) was missing
 * in InitializeGui().
 *
 * Revision 2.2  2000/09/11  14:02:17  cibrario
 * Bug fix: removed spurious printf() from InitializeWidgets
 *
 * Revision 2.1  2000/09/08  15:36:33  cibrario
 * *** empty log message ***
 *

.- */

#ifndef lint
static char rcs_id[] = "$Id: x11.c,v 4.1.1.1 2002/11/11 16:11:47 cibrario Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

#include <X11/Xlib.h> /* Main X header */

#include <X11/Intrinsic.h>  /* Main Xt header */
#include <X11/StringDefs.h> /* Literal strings of widget attributes */

#include <Xm/Xm.h>        /* Main OSF/Motif header */
#include <Xm/MainW.h>     /* XmMainWindow widget */
#include <Xm/Form.h>      /* XmForm widget */
#include <Xm/Label.h>     /* XmLabel widget */
#include <Xm/ToggleB.h>   /* XmToggleButton widget */
#include <Xm/RowColumn.h> /* XmRowColumn widget */
#include <Xm/Frame.h>     /* XmFrame widget */
#include <Xm/DrawingA.h>  /* XmDrawingArea widget */
#include <Xm/FileSB.h>    /* XmFileSelectionBox widget */
#include <Xm/MessageB.h>  /* XmMessageBox widget */
#include <Xm/TextF.h>     /* XmTextField widget */
#include <Xm/Protocols.h> /* XmAddWMProtocolCallback() */

#include "config.h"
#include "machdep.h"
#include "cpu.h"     /* EmulatorExit(); required by keyb.h, too */
#include "display.h" /* LcdInit() */
#include "keyb.h"
#include "serial.h"
#include "x11.h"
#include "args.h"
#include "debug.h"

#define CHF_MODULE_ID X11_CHF_MODULE_ID
#include <Chf.h>

/*---------------------------------------------------------------------------
        Misc. parameters
  ---------------------------------------------------------------------------*/

#define APP_CLASS "Saturn"
#define MAX_CS_SEGMENT_LEN 80

/*---------------------------------------------------------------------------
        Private variables and type definitions
  ---------------------------------------------------------------------------*/

static XtAppContext app_context;
static Widget shell_widget;
static Widget file_sel_box;
static Widget error_dialog;
static Widget msg_text_field;

#define MAX_ERROR_DIALOG_COUNT 10
static int error_dialog_count;

/* Forward static function declarations */
static void ResetToggleButtons( Widget w );

/* Continuation procedure to invoke when a FSB button is pressed */
static FsbContinuation fsb_cont = ( FsbContinuation )NULL;

/* Command line option descriptors; these descriptors map command line
   options into X resource settings
*/
static XrmOptionDescRec options[] = {
    /* option, specifier, argKind, value */
    { "-reset", "*reset", XrmoptionNoArg, "True" },
    { "-monitor", "*monitor", XrmoptionNoArg, "True" },
    { "-batchXfer", "*batchXfer", XrmoptionNoArg, "True" },
    { "-stateDir", "*stateDir", XrmoptionSepArg },
    { "-cpu", "*cpu", XrmoptionSepArg },
    { "-mod", "*mod", XrmoptionSepArg },
    { "-hdw", "*hdw", XrmoptionSepArg },
    { "-rom", "*rom", XrmoptionSepArg },
    { "-ram", "*ram", XrmoptionSepArg },
    { "-port1", "*port1", XrmoptionSepArg },
    { "-port2", "*port2", XrmoptionSepArg },
    { "-face", "*face", XrmoptionSepArg },
    { "-hw", "*hw", XrmoptionSepArg }
};

#define NUM_OPTIONS XtNumber( options )

/* Application fallback resources */
static String fallback_resources[] = {
    NULL /* Null terminated */
};

/* Application resources container (top-level resources) */
struct app_opt {
    Boolean reset;
    Boolean monitor;
    Boolean batchXfer;
    String stateDir;
    String cpu;
    String mod;
    String hdw;
    String rom;
    String ram;
    String port1;
    String port2;
    String face;
    String hw;
};

/* Application resources container (per-face resources) */
struct face_opt {
    int nKeys;
};

/* Application resources container (per-button resources) */
struct btn_opt {
    String inOut;
};

/* Application resources container (compoundString resource) */
struct cs_opt {
    String compoundString;
};

/* Application resource descriptors (top-level resources) */
static XtResource app_res[] = {
    {"reset",     "Reset",     XtRBoolean, sizeof( Boolean ), XtOffsetOf( struct app_opt, reset ),     XtRString, "False"},
    {"monitor",   "Monitor",   XtRBoolean, sizeof( Boolean ), XtOffsetOf( struct app_opt, monitor ),   XtRString, "False"},
    {"batchXfer", "BatchXfer", XtRBoolean, sizeof( Boolean ), XtOffsetOf( struct app_opt, batchXfer ), XtRString, "False"},
    {"stateDir",  "StateDir",  XtRString,  sizeof( String ),  XtOffsetOf( struct app_opt, stateDir ),  XtRString, "."    },
    {"cpu",       "Cpu",       XtRString,  sizeof( String ),  XtOffsetOf( struct app_opt, cpu ),       XtRString, "cpu"  },
    {"mod",       "Mod",       XtRString,  sizeof( String ),  XtOffsetOf( struct app_opt, mod ),       XtRString, "mod"  },
    {"hdw",       "Hdw",       XtRString,  sizeof( String ),  XtOffsetOf( struct app_opt, hdw ),       XtRString, "hdw"  },
    {"rom",       "Rom",       XtRString,  sizeof( String ),  XtOffsetOf( struct app_opt, rom ),       XtRString, "rom"  },
    {"ram",       "Ram",       XtRString,  sizeof( String ),  XtOffsetOf( struct app_opt, ram ),       XtRString, "ram"  },
    {"port1",     "Port1",     XtRString,  sizeof( String ),  XtOffsetOf( struct app_opt, port1 ),     XtRString, "port1"},
    {"port2",     "Port2",     XtRString,  sizeof( String ),  XtOffsetOf( struct app_opt, port2 ),     XtRString, "port2"},
    {"face",      "Face",      XtRString,  sizeof( String ),  XtOffsetOf( struct app_opt, face ),      XtRString, "hp48" },
    {"hw",        "Hw",        XtRString,  sizeof( String ),  XtOffsetOf( struct app_opt, hw ),        XtRString, "hp48" }
};

#define NUM_APP_RES XtNumber( app_res )

/* Application resource descriptors (per-face resources) */
static XtResource face_res[] = {
    {
     "nKeys", "NKeys", XtRInt, sizeof( int ), XtOffsetOf( struct face_opt, nKeys ), XtRString, "0" /* Face has no keys by default */
    }
};

#define NUM_FACE_RES XtNumber( face_res )

/* Application resource descriptors (per-button resources) */
static XtResource btn_res[] = {
    {
     "inOut", "InOut", XtRString, sizeof( String ), XtOffsetOf( struct btn_opt, inOut ), XtRString, "00/0" /* Do-nothing inOut */
    }
};

#define NUM_BTN_RES XtNumber( btn_res )

/* Application resource descriptors (compoundString resource) */
static XtResource cs_res[] = {
    {
     "compoundString", "CompoundString", XtRString, sizeof( String ), XtOffsetOf( struct cs_opt, compoundString ), XtRString,
     NULL /* NULL string by default */
    }
};

#define NUM_CS_RES XtNumber( cs_res )

/* Xt Actions */

static void kbdKeyPress( Widget w, XEvent* xe, String* argv, Cardinal* argc );
static void kbdKeyRelease( Widget w, XEvent* xe, String* argv, Cardinal* argc );

static XtActionsRec xt_actions[] = {
    {"kbdKeyPress",   kbdKeyPress  },
    {"kbdKeyRelease", kbdKeyRelease}
};

#define NUM_XT_ACTIONS XtNumber( xt_actions )

/*---------------------------------------------------------------------------
        Public variables
  ---------------------------------------------------------------------------*/

/* Emulator options; they are initialized here from X resources */
struct Args args;

/*---------------------------------------------------------------------------
        Private functions
  ---------------------------------------------------------------------------*/

/* Path/name dynamic allocator */
static char* GetPathname( String path, String name )
{
    char* s = malloc( strlen( path ) + strlen( name ) + 2 );

    strcpy( s, path );
    strcat( s, "/" );
    strcat( s, name );
    return s;
}

/*---------------------------------------------------------------------------
        Private functions: Xt Actions and Callbacks
  ---------------------------------------------------------------------------*/

#ifdef LONG_PRESS_THR
/* 4.1.1.1: This variable saves the latest mouse button arm time; it will be
   used at button release, to compute how much time the button has been kept
   pressed. Notice that this is a bit simplistic, since it assumes that only
   one button is pressed at a time.
*/
static Time arm_time = CurrentTime;

/* Compute the difference between two X Server timestamps, taking wrap around
   of a CARD32 into account.
*/
#  define TimeDiff( n, o ) ( ( ( n ) - ( o ) ) & 0xFFFFFFFFUL )
#endif

/* This function is called when a keyboard button is armed */
static void kbdButtonArmed( Widget w, XtPointer client_data, XtPointer call_data )
{
    debug1( DEBUG_C_TRACE | DEBUG_C_X11, X11_I_CALLED, "kbdButtonArmed" );

#ifdef LONG_PRESS_THR
    /* Save arm_time for use at release */
    arm_time = ( ( XmToggleButtonCallbackStruct* )call_data )->event->xbutton.time;
#endif

    KeybPress( ( char* )client_data );
}

/* This function is called when a keyboard button is disarmed */
static void kbdButtonDisarmed( Widget w, XtPointer client_data, XtPointer call_data )
{
    XmToggleButtonCallbackStruct* info = ( XmToggleButtonCallbackStruct* )call_data;

    XEvent* event = info->event;

    debug1( DEBUG_C_TRACE | DEBUG_C_X11, X11_I_CALLED, "kbdButtonDisarmed" );

#ifdef LONG_PRESS_THR
    if ( TimeDiff( event->xbutton.time, arm_time ) > LONG_PRESS_THR ) {
        /* Button pressed for more than LONG_PRESS_THR; keep it pressed */
        XmToggleButtonSetState( w, True, False );
    } else
#endif

        if ( event->type == ButtonRelease && event->xbutton.button == 3 ) {
        /* Keep the button pressed */
        XmToggleButtonSetState( w, True, False );
    } else {
        /* Release the button */
        XmToggleButtonSetState( w, False, False );

        KeybRelease( ( char* )client_data );
    }
}

/* This function is called when the lcd widget receives an Expose event */
static void lcdExposed( Widget w, XtPointer client_data, XtPointer call_data )
{
    XmDrawingAreaCallbackStruct* expose_data = ( XmDrawingAreaCallbackStruct* )call_data;

    debug1( DEBUG_C_TRACE | DEBUG_C_X11, X11_I_CALLED, "lcdExposed" );

    if ( expose_data->event->type == Expose && expose_data->event->xexpose.count == 0 )
        RefreshLcd();
}

/* This function is called when the main shell window is about to be
   destroyed by the wm
*/
static void deleteWindow( Widget w, XtPointer client_data, XtPointer call_data )
{
    debug1( DEBUG_C_TRACE | DEBUG_C_X11, X11_I_CALLED, "deleteWindow" );

    /* 3.5: Added linux support.

       Attempt to save the emulator state on mass storage; default
       Motif callback should actually destroy the GUI and exit the
       application.

       However, this does not work with Debian lesstif 0.89.4-3,
       so an explicit exit() has been added here; it should not
       harm other architectures.
    */
    EmulatorExit( SAVE_AND_EXIT );
    exit( EXIT_SUCCESS );
}

/* This function is called when the session manager wants the
   application to save its state.  This procedure should update the
   WM_COMMAND property of the shell window when done.
*/
static void saveYourself( Widget w, XtPointer client_data, XtPointer call_data )
{
    int argc; /* Argument vector to/from WM_COMMAND property */
    char** argv;

    debug1( DEBUG_C_TRACE | DEBUG_C_X11, X11_I_CALLED, "saveYourself" );

    /* Attempt to save the emulator state on mass storage; default
       Motif callback will actually destroy the GUI and exit the
       application.
    */
    EmulatorExit( SAVE_AND_EXIT );

    /* Update the WM_COMMAND property on w, and let the session manager
       continue its work.
    */
    if ( !XGetCommand( XtDisplay( w ), XtWindow( w ), &argv, &argc ) ) {
        /* Property not found / wrong format / ... */
        ChfCondition X11_E_NO_WM_COMMAND, CHF_ERROR ChfEnd;
        ChfSignal();
    } else {
        /* Set the property to the same value it had and free the string
           list returned by Xlib.
        */
        XSetCommand( XtDisplay( w ), XtWindow( w ), argv, argc );
        XFreeStringList( argv );
    }
}

/* This function is called when a shortcut key is pressed */
static void kbdKeyPress( Widget w, XEvent* xe, String* argv, Cardinal* argc )
{
    debug1( DEBUG_C_TRACE | DEBUG_C_X11, X11_I_CALLED, "kbdKeyPress" );

    if ( *argc == 1 ) {
        debug1( DEBUG_C_X11, X11_I_KEY_PRESS, argv[ 0 ] );
        KeybPress( argv[ 0 ] );
    } else {
        ChfCondition X11_W_BAD_ACTION_CALL, CHF_WARNING, argc ChfEnd;
        ChfSignal();
    }
}

/* This function is called when a shortcut key is released */
static void kbdKeyRelease( Widget w, XEvent* xe, String* argv, Cardinal* argc )
{
    debug1( DEBUG_C_TRACE | DEBUG_C_X11, X11_I_CALLED, "kbdKeyRelease" );

    if ( *argc == 1 ) {
        debug1( DEBUG_C_X11, X11_I_KEY_RELEASE, argv[ 0 ] );
        KeybRelease( argv[ 0 ] );
    } else {
        ChfCondition X11_W_BAD_ACTION_CALL, CHF_WARNING, argc ChfEnd;
        ChfSignal();
    }
}

/* This function is called when a button of the FileSelectionBox
   is activated.
*/
static void fsbButtonPressed( Widget w, XtPointer client_data, XtPointer call_data )
{
    XmFileSelectionBoxCallbackStruct* fsb_data = ( XmFileSelectionBoxCallbackStruct* )call_data;

    /* Continuation must proceed only if OK was activated.

       3.16: XmCR_APPLY must not be handled here, because it corresponds
       to the 'Filter' key.
    */
    int proceed = ( fsb_data->reason == XmCR_OK );

    char* value = ( char* )NULL;

    debug1( DEBUG_C_TRACE | DEBUG_C_X11, X11_I_CALLED, "fsbButtonPressed" );

    if ( fsb_cont != ( FsbContinuation )NULL ) {
        /* If continuation must proceed, get the current user selection
           from the file selection box and convert it into a char *;
           the conversion should never fail.
        */
        if ( proceed && !XmStringGetLtoR( fsb_data->value, XmFONTLIST_DEFAULT_TAG, &value ) ) {
            ChfCondition X11_E_NO_FSB_TSEG, CHF_ERROR ChfEnd;
            ChfSignal();
        } else {
            /* Invoke continuation; value is meaningful only when
               proceed is true.
            */
            fsb_cont( proceed, value );

            /* Free the dynamically-allocated user selection, if any.
               XtFree() handles NULL pointers correctly.
            */
            XtFree( value );
        }
    } else {
        /* Continuation not set; do nothing */
        ChfCondition X11_W_NO_FSB_CONT, CHF_WARNING ChfEnd;
        ChfSignal();
    }

    /* Remove the popup shell */
    XtUnmanageChild( w );
}

/* This utility function builds an XmString from a string, taking newlines
   and tabs into account: when a newline is found, the separator 'sep'
   is put into the string; when a tab is found, the spacer 'spc' is put
   into the string.  The function destroys the original string s in the
   process.
*/
static XmString XmStringFromString( char* s, XmString sep, XmString spc )
{
    char* p; /* String scan pointer */
    char c;  /* Current String character */

    XmString x; /* XmString from current segment */
    XmString n; /* Result of concatenation */

    XmString m = XmStringCreate( "", XmFONTLIST_DEFAULT_TAG ); /* Buffer */

    p = s;
    while ( ( c = *p ) != '\0' ) {
        if ( c == '\n' || c == '\t' ) {
            /* Terminate current segment */
            *p = '\0';

            /* Append segment to XmString, if not empty */
            if ( p > s ) {
                x = XmStringCreate( s, XmFONTLIST_DEFAULT_TAG );
                n = XmStringConcat( m, x );

                XmStringFree( m );
                XmStringFree( x );
                m = n;
            }

            /* Append either sep or spc */
            n = XmStringConcat( m, ( c == '\n' ) ? sep : spc );

            XmStringFree( m );
            m = n;

            /* Skip current segment and start a new one */
            p++;
            s = p;
        } else
            /* Current segment continues */
            p++;
    }

    if ( p > s ) {
        /* Non-empty pending segment; append */
        x = XmStringCreate( s, XmFONTLIST_DEFAULT_TAG );
        n = XmStringConcat( m, x );

        XmStringFree( m );
        XmStringFree( x );
        m = n;
    }

    return m;
}

/* This is a Chf handler; it is called when any condition is signalled
   and prints the message through the error dialog.  This handler also
   intercepts the SERIAL_CHF_MODULE_ID/SERIAL_I_PTY_NAME message and
   puts it into the main message display area.

   When the number of messages in the error dialog exceeds
   MAX_ERROR_DIALOG_COUNT, further messages are immediately
   resignaled; also, condition X11_W_TOO_MANY_MSG is signaled once.
*/
static ChfAction ErrorDialogHandler( const ChfDescriptor* d, const ChfState s, ChfPointer ctx )
{
    ChfAction act;

    /* Check Chf state */
    switch ( s ) {

        case CHF_SIGNALING:
            /* ChfSignal() in progress */
            if ( ChfGetSeverity( d ) == CHF_FATAL ) {
                /* Severity is FATAL; the application is exiting.
                   No point to use a message window; resignal now.
                */
                act = CHF_RESIGNAL;
            } else if ( ChfGetModuleId( d ) == SERIAL_CHF_MODULE_ID && ChfGetConditionCode( d ) == SERIAL_I_PTY_NAME ) {
                /* Pseudo-terminal message; this is very important.
                   Put it into the message display area.

                   This is also an example of how you can intercept a
                   condition message and do anything with it *without*
                   changing a line of code elsewhere and, in particular,
                   in the place where the condition is generated.
                */
                Arg xt_args[ 20 ];
                int n;

                n = 0;
                XtSetArg( xt_args[ n ], XmNvalue, ChfGetPartialMessage( d ) );
                n++;
                XtSetValues( msg_text_field, xt_args, n );

                act = CHF_CONTINUE;
            } else {
                /* If maximum value of error_dialog_count has been reached,
                   resignal.
                */
                if ( error_dialog_count++ == MAX_ERROR_DIALOG_COUNT ) {
                    ChfCondition X11_W_TOO_MANY_MSG, CHF_WARNING, MAX_ERROR_DIALOG_COUNT ChfEnd;
                    ChfSignal();

                    act = CHF_RESIGNAL;
                } else if ( error_dialog_count > MAX_ERROR_DIALOG_COUNT )
                    act = CHF_RESIGNAL;
                else {
                    unsigned char dialog_type = XmDIALOG_INFORMATION;
                    XmString sep = XmStringSeparatorCreate();
                    XmString spc = XmStringCreate( "    ", XmFONTLIST_DEFAULT_TAG );
                    XmString m, c, o;

                    Arg xt_args[ 20 ];
                    int n;

                    /* 3.17: Reset GUI: keys and (recursively) buttons;
                       see comment in ActivateFSB() for more information.
                    */
                    KeybReset();
                    ResetToggleButtons( shell_widget );

                    /* Determine dialog_type from top condition's severity */
                    if ( ChfGetSeverity( d ) == CHF_ERROR )
                        dialog_type = XmDIALOG_ERROR;
                    else if ( ChfGetSeverity( d ) == CHF_WARNING )
                        dialog_type = XmDIALOG_WARNING;

                    /* Put the message into the dialog; be careful with newlines */
                    m = XmStringFromString( ChfBuildMessage( d ), sep, spc );

                    while ( ( d = ChfGetNextDescriptor( d ) ) != CHF_NULL_DESCRIPTOR ) {
                        o = XmStringFromString( ChfBuildMessage( d ), sep, spc );
                        c = XmStringConcat( m, o );
                        XmStringFree( m );
                        m = c;
                        XmStringFree( o );
                    }

                    /* Get old value of messageString */
                    n = 0;
                    XtSetArg( xt_args[ n ], XmNmessageString, &o );
                    n++;
                    XtGetValues( error_dialog, xt_args, n );

                    /* Append a separator to old messageString */
                    c = XmStringConcat( o, sep );
                    XmStringFree( o );
                    o = c;

                    /* Append new string to the old one */
                    c = XmStringConcat( o, m );
                    XmStringFree( o );
                    o = c;

                    /* Set the error dialog resources */
                    n = 0;
                    XtSetArg( xt_args[ n ], XmNdialogType, dialog_type );
                    n++;
                    XtSetArg( xt_args[ n ], XmNmessageString, o );
                    n++;
                    XtSetValues( error_dialog, xt_args, n );

                    /* Free XmStrings */
                    XmStringFree( o );
                    XmStringFree( m );
                    XmStringFree( sep );
                    XmStringFree( spc );

                    /* Display the error dialog */
                    XtManageChild( error_dialog );

                    act = CHF_CONTINUE;
                }
            }
            break;

        default:
            /* Other states; resignal the condition */
            act = CHF_RESIGNAL;
            break;
    }

    return act;
}

/* This callback is invoked when the OK button of the error dialog
   is pressed.
*/
static void errorButtonPressed( Widget w, XtPointer client_data, XtPointer call_data )
{
    XmString e = XmStringCreate( "", XmFONTLIST_DEFAULT_TAG );

    Arg xt_args[ 20 ];
    int n;

    /* Clear messageString */
    n = 0;
    XtSetArg( xt_args[ n ], XmNmessageString, e );
    n++;
    XtSetValues( error_dialog, xt_args, n );

    XmStringFree( e );

    /* Reset message counter */
    error_dialog_count = 0;

    /* Remove the popup shell */
    XtUnmanageChild( w );
}

/*---------------------------------------------------------------------------
        Private functions: Initialization
  ---------------------------------------------------------------------------*/

/* This function initializes the Xt application context and
   gets the application resources. Some Xt variables (app_context and
   the main shell widget) are static.
   Instead, the application's options structure (opt) is passed by
   reference as an argument.
*/
static void InitializeXt( int argc, char* argv[], struct app_opt* opt )
{
    Atom a;

    Arg xt_args[ 20 ];
    int n;

    debug1( DEBUG_C_TRACE | DEBUG_C_X11, X11_I_CALLED, "InitializeXt" );

    /* Enable default Xt language setup procedure */
    XtSetLanguageProc( NULL, NULL, NULL );

    /* Setup Arg vector for shell widget creation */
    n = 0;
    XtSetArg( xt_args[ n ], XmNallowShellResize, True );
    n++;

    /* Initialize application, parse command line options,
       create its main shell.
    */
    shell_widget = XtAppInitialize( &app_context, APP_CLASS, options, NUM_OPTIONS, &argc, argv, fallback_resources, xt_args, n );

    /* Add WMProtocolCallback for WM_DELETE_WINDOW and WM_SAVE_YOURSELF */
    if ( ( a = XmInternAtom( XtDisplay( shell_widget ), "WM_DELETE_WINDOW", True ) ) != None )
        /* XmAddWMProtocolCallback() invokes XmAddProtocolCallback();
           in turn, XmAddProtocolCallback() automatically calls
           XmAddProtocols() if the protocol has not yet been registered.
           Therefore, XmAddWMProtocols(shell_widget, &a, 1) is not needed here.
        */
        XmAddWMProtocolCallback( shell_widget, a, deleteWindow, ( XtPointer )NULL );
    else {
        ChfCondition X11_W_UNKNOWN_ATOM, CHF_WARNING, "WM_DELETE_WINDOW" ChfEnd;
        ChfSignal();
    }

    if ( ( a = XmInternAtom( XtDisplay( shell_widget ), "WM_SAVE_YOURSELF", True ) ) != None )
        XmAddWMProtocolCallback( shell_widget, a, saveYourself, ( XtPointer )NULL );
    else {
        ChfCondition X11_W_UNKNOWN_ATOM, CHF_WARNING, "WM_SAVE_YOURSELF" ChfEnd;
        ChfSignal();
    }

    /* Spot unknown options - argv[0] always contains program name */
    if ( argc > 1 ) {
        int i;
        for ( i = 1; i < argc; i++ )
            ChfCondition X11_E_BAD_OPTION, CHF_ERROR, argv[ i ] ChfEnd;

        ChfCondition X11_I_USAGE, CHF_INFO, argv[ 0 ] ChfEnd;
        ChfSignal();
    }

    /* Get application options and fill the 'struct app_opt opt' structure
       (statically named options only)
    */
    XtGetApplicationResources( shell_widget, ( XtPointer )opt, app_res, NUM_APP_RES, ( ArgList )NULL, ( Cardinal )0 );

    /* Announce the active face (debug only) */
    debug1( DEBUG_C_X11, X11_I_FACE, opt->face );

    /* Install Xt actions */
    XtAppAddActions( app_context, xt_actions, NUM_XT_ACTIONS );
}

/* Initialize the fast-load popup window */
static void InitializeFSB( void )
{
    Arg xt_args[ 20 ];
    int n;

    /* Setup Arg vector for file selection dialog & box, and create them.
       It's not necessary to set XmNscreen here, because we want the
       default screen and this is dynamically set in shell_widget.

       3.15: Set allowShellResize to True; this way the shell will be
       able to accommodate children's size changes.  Also set
       deleteResponse to XmDO_NOTHING; this prevents dangerous implicit
       destruction of the shell widget when the wm allows direct WM_CLOSE
       actions on it.
    */
    n = 0;
    XtSetArg( xt_args[ n ], XmNallowShellResize, True );
    n++;
    XtSetArg( xt_args[ n ], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL );
    n++;
    XtSetArg( xt_args[ n ], XmNdeleteResponse, XmDO_NOTHING );
    n++;

    file_sel_box = XmCreateFileSelectionDialog( shell_widget, "fsb", xt_args, n );

    /* Make unused buttons insensitive */
    XtSetSensitive( XmFileSelectionBoxGetChild( file_sel_box, XmDIALOG_HELP_BUTTON ), False );

    /* Add callbacks for relevant buttons.

       3.16: XmNapplyCallback corresponds to the 'Filter' key in this case,
       and must not be handled directly by the application.
    */
    XtAddCallback( file_sel_box, XmNokCallback, fsbButtonPressed, NULL );

    XtAddCallback( file_sel_box, XmNcancelCallback, fsbButtonPressed, NULL );
}

/* Initialize the dialog used to display error messages, and install
   a Chf handler to use it.
*/
static void InitializeErrorDialog( void )
{
    Arg xt_args[ 20 ];
    int n;

    /* Setup Arg vector for error dialog and create it.
       It's not necessary to set XmNscreen here, because we want the
       default screen and this is dynamically set in shell_widget.

       Set allowShellResize to True; this way the shell will be
       able to accommodate children's size changes.  Also set
       deleteResponse to XmDO_NOTHING; this prevents dangerous implicit
       destruction of the shell widget when the wm allows direct WM_CLOSE
       actions on it.
    */
    n = 0;
    XtSetArg( xt_args[ n ], XmNallowShellResize, True );
    n++;
    XtSetArg( xt_args[ n ], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL );
    n++;
    XtSetArg( xt_args[ n ], XmNdeleteResponse, XmDO_NOTHING );
    n++;

    error_dialog = XmCreateErrorDialog( shell_widget, "error", xt_args, n );

    /* Make unused buttons insensitive */
    XtSetSensitive( XmMessageBoxGetChild( error_dialog, XmDIALOG_HELP_BUTTON ), False );

    XtSetSensitive( XmMessageBoxGetChild( error_dialog, XmDIALOG_CANCEL_BUTTON ), False );

    /* Add callback for the OK button */
    XtAddCallback( error_dialog, XmNokCallback, errorButtonPressed, NULL );

    /* Install a Chf handler for it */
    ChfPushHandler( ErrorDialogHandler, CHF_NULL_CONTEXT, CHF_NULL_POINTER );

    /* Reset message count */
    error_dialog_count = 0;
}

/* This function checks if widget w has a compoundString application's
   resource set.  If yes, the resource is parsed and the labelString
   widget resource is set accordingly. compoundString syntax:

   <compoundString>: ([# <fontlist_tag>] <string>])*
   <fontlist_tag>:
        #	-- Put a single '#' in current segment
        <blank> -- Create a new segment using XmFONTLIST_DEFAULT_TAG as a tag
        <any>	-- Create a new segment using <any> as a tag

   Each segment is limited to MAX_CS_SEGMENT_LEN characters; longer
   segments are silently truncated.

   Too lazy to build a full-fledged resource translator...
*/
static void CheckCompoundString( Widget w )
{
    struct cs_opt opt;

    Arg xt_args[ 10 ];
    int n;

    XtGetApplicationResources( w, ( XtPointer )&opt, cs_res, NUM_CS_RES, ( ArgList )NULL, ( Cardinal )0 );

    if ( opt.compoundString != NULL ) {
        char user_tag[ 2 ];                     /* User tag buffer, 1 char */
        char seg_buf[ MAX_CS_SEGMENT_LEN + 1 ]; /* Segment buffer */
        char* cur_tag = XmFONTLIST_DEFAULT_TAG; /* Tag of current segment */
        char* cur_sptr = opt.compoundString;    /* Parser's source ptr */
        char* cur_dptr = seg_buf;               /* Dest ptr in seg_buf */
        char cur_c;                             /* Current source char */
        char next_c;                            /* Next source char */
        XmString xm_string;                     /* Parsed xm_string */
        XmString cur_xm;                        /* Parsed segment */
        XmString new_xm;

        debug2( DEBUG_C_X11, X11_I_FOUND_CS, XtName( XtParent( w ) ), opt.compoundString );

        user_tag[ 1 ] = '\0';
        xm_string = XmStringCreate( "", XmFONTLIST_DEFAULT_TAG );

        /* Parse the compoundString */
        while ( ( cur_c = *cur_sptr++ ) != '\0' ) {
            if ( cur_c == '#' ) {
                /* Tag marker */
                if ( ( next_c = *cur_sptr ) == '\0' ) {
                    /* Syntax error; ignore trailing # */
                } else {
                    if ( next_c == '#' ) {
                        /* Escaped #; store in current segment and skip */
                        if ( cur_dptr - seg_buf < MAX_CS_SEGMENT_LEN )
                            *cur_dptr++ = next_c;
                    } else {
                        /* Close current segment and append to xm_string */
                        if ( cur_dptr - seg_buf > 0 ) {
                            *cur_dptr = '\0';
                            cur_xm = XmStringCreate( seg_buf, cur_tag );
                            new_xm = XmStringConcat( xm_string, cur_xm );
                            XmStringFree( xm_string );
                            XmStringFree( cur_xm );
                            xm_string = new_xm;
                            cur_dptr = seg_buf;
                        }

                        /* Set new current tag */
                        if ( next_c == ' ' )
                            cur_tag = XmFONTLIST_DEFAULT_TAG;
                        else {
                            user_tag[ 0 ] = next_c;
                            cur_tag = user_tag;
                        }
                    }

                    cur_sptr++;
                }
            } else {
                /* Store char in current segment */
                if ( cur_dptr - seg_buf < MAX_CS_SEGMENT_LEN )
                    *cur_dptr++ = cur_c;
            }
        }

        /* Close ending segment and append to xm_string */
        if ( cur_dptr - seg_buf > 0 ) {
            *cur_dptr = '\0';
            cur_xm = XmStringCreate( seg_buf, cur_tag );
            new_xm = XmStringConcat( xm_string, cur_xm );
            XmStringFree( xm_string );
            XmStringFree( cur_xm );
            xm_string = new_xm;
        }

        /* Pant... now push xm_string into the widget */
        n = 0;
        XtSetArg( xt_args[ n ], XmNlabelString, xm_string );
        n++;
        XtSetValues( w, xt_args, n );
        XmStringFree( xm_string );
    }
}

/* This function creates the widget set corresponding to a key, and
   returns the widget id of the top widget
*/
static Widget CreateKey( int k, Widget parent )
{
    char container_name[ 8 ];
    Widget w, l, b;
    XtTranslations t;
    struct btn_opt opt;

    Arg xt_args[ 20 ];
    int n;

    debug1( DEBUG_C_TRACE | DEBUG_C_X11, X11_I_CALLED, "CreateKey" );

    /* Generate container_name */
    sprintf( container_name, "%d", k );

    /* Container widget (xmForm) */
    /* Force nonmodal navigation only; don't allow overlapping children */
    n = 0;
#ifdef FORCE_NONMODAL
    XtSetArg( xt_args[ n ], XmNnavigationType, XmNONE );
    n++;
    XtSetArg( xt_args[ n ], XmNtraversalOn, False );
    n++;
#endif
    XtSetArg( xt_args[ n ], XmNallowOverlap, False );
    n++;

    w = XtCreateManagedWidget( container_name, xmFormWidgetClass, parent, xt_args, n );

    /* Upper labels */
    /* Must be widgets to set own foreground color */
    n = 0;
    XtSetArg( xt_args[ n ], XmNleftAttachment, XmATTACH_FORM );
    n++;
    XtSetArg( xt_args[ n ], XmNtopAttachment, XmATTACH_FORM );
    n++;

    l = XtCreateManagedWidget( "ul", xmLabelWidgetClass, w, xt_args, n );

    CheckCompoundString( l );

    n = 0;
    XtSetArg( xt_args[ n ], XmNalignment, XmALIGNMENT_END );
    n++;
    XtSetArg( xt_args[ n ], XmNleftAttachment, XmATTACH_WIDGET );
    n++;
    XtSetArg( xt_args[ n ], XmNleftWidget, l );
    n++;
    XtSetArg( xt_args[ n ], XmNrightAttachment, XmATTACH_FORM );
    n++;
    XtSetArg( xt_args[ n ], XmNtopAttachment, XmATTACH_FORM );
    n++;

    CheckCompoundString( XtCreateManagedWidget( "ur", xmLabelWidgetClass, w, xt_args, n ) );

    /* ToggleButton */
    /* - Disable ToggleButton indicator; shadowThickness must be >0 to
       enable 3D shadowing

       2.8: The translation table of the ToggleButton is no longer set
       here, to be able to install default keyboard translations,
       via app-resource file, to buttons too.
       Instead, XtAugmentTranslations() is used after widget creation.
    */
    n = 0;
    XtSetArg( xt_args[ n ], XmNindicatorOn, False );
    n++;
    XtSetArg( xt_args[ n ], XmNleftAttachment, XmATTACH_FORM );
    n++;
    XtSetArg( xt_args[ n ], XmNrightAttachment, XmATTACH_FORM );
    n++;
    XtSetArg( xt_args[ n ], XmNtopAttachment, XmATTACH_WIDGET );
    n++;
    XtSetArg( xt_args[ n ], XmNtopWidget, l );
    n++;

    b = XtCreateManagedWidget( "btn", xmToggleButtonWidgetClass, w, xt_args, n );

    /* Augment the translation table to allow clicks with button 3;
       this key is used to hold a key pressed.
    */
    XtAugmentTranslations( b, ( t = XtParseTranslationTable( "#augment <Btn3Down>: Arm()\n"
                                                             "<Btn3Up>: Select() Disarm()\n" ) ) );

    XtFree( ( void* )t ); /* Free the translation table */
    CheckCompoundString( b );

    /* Lower labels */
    /* Must be widgets to set own foreground color */
    n = 0;
    XtSetArg( xt_args[ n ], XmNleftAttachment, XmATTACH_FORM );
    n++;
    XtSetArg( xt_args[ n ], XmNtopAttachment, XmATTACH_WIDGET );
    n++;
    XtSetArg( xt_args[ n ], XmNtopWidget, b );
    n++;

    l = XtCreateManagedWidget( "ll", xmLabelWidgetClass, w, xt_args, n );

    CheckCompoundString( l );

    n = 0;
    XtSetArg( xt_args[ n ], XmNalignment, XmALIGNMENT_END );
    n++;
    XtSetArg( xt_args[ n ], XmNleftAttachment, XmATTACH_WIDGET );
    n++;
    XtSetArg( xt_args[ n ], XmNleftWidget, l );
    n++;
    XtSetArg( xt_args[ n ], XmNrightAttachment, XmATTACH_FORM );
    n++;
    XtSetArg( xt_args[ n ], XmNtopAttachment, XmATTACH_WIDGET );
    n++;
    XtSetArg( xt_args[ n ], XmNtopWidget, b );
    n++;

    CheckCompoundString( XtCreateManagedWidget( "lr", xmLabelWidgetClass, w, xt_args, n ) );

    /* Get application resources of ToggleButton and fill
       the 'struct btn_opt opt' structure appropriately
    */
    XtGetApplicationResources( b, ( XtPointer )&opt, btn_res, NUM_BTN_RES, ( ArgList )NULL, ( Cardinal )0 );

    debug2( DEBUG_C_X11, X11_I_KEY, k, opt.inOut );

    /* Add Arm/Disarm callbacks on ToggleButton */
    XtAddCallback( b, XmNarmCallback, kbdButtonArmed, opt.inOut );

    XtAddCallback( b, XmNdisarmCallback, kbdButtonDisarmed, opt.inOut );

    return w;
}

/* This function creates, initializes and realizes the widget tree.
   Initialization includes callback installation.
*/
static void InitializeWidgets( String face, Display** lcd_display, Window* lcd_window, unsigned long* lcd_fg, unsigned long* lcd_bg )
{
    Widget mw, rc, kbd, f, lcd;
    int k;
    struct face_opt opt;
    Pixel lcd_fg_pixel, lcd_bg_pixel;

    Arg xt_args[ 20 ];
    int n;

    debug1( DEBUG_C_TRACE | DEBUG_C_X11, X11_I_CALLED, "InitializeWidgets" );

    /* Main window */
    /* Force nonmodal navigation only */
    n = 0;
#ifdef FORCE_NONMODAL
    XtSetArg( xt_args[ n ], XmNnavigationType, XmNONE );
    n++;
    XtSetArg( xt_args[ n ], XmNtraversalOn, False );
    n++;
#endif

    mw = XtCreateManagedWidget( "main", xmMainWindowWidgetClass, shell_widget, xt_args, n );

    /* RowColumn container for display, keyboard and message area */
    /* Force nonmodal navigation only */
    n = 0;
#ifdef FORCE_NONMODAL
    XtSetArg( xt_args[ n ], XmNnavigationType, XmNONE );
    n++;
    XtSetArg( xt_args[ n ], XmNtraversalOn, False );
    n++;
#endif

    rc = XtCreateManagedWidget( face, xmRowColumnWidgetClass, mw, xt_args, n );

    /* Get <face>.nKeys resource (opt->nKeys) */
    XtGetApplicationResources( rc, ( XtPointer )&opt, face_res, NUM_FACE_RES, ( ArgList )NULL, ( Cardinal )0 );

    /* Announce resource value (debug only) */
    debug1( DEBUG_C_X11, X11_I_NKEYS, opt.nKeys );

    /* 3.15: Cannot continue if the active faceplate has no keys;
       the application resource file is probably wrong.
    */
    if ( opt.nKeys <= 0 ) {
        ChfCondition X11_F_NO_KEYS, CHF_FATAL, face ChfEnd;
        ChfSignal();
    }

    /* LCD Screen */
    n = 0;
#ifdef FORCE_NONMODAL
    XtSetArg( xt_args[ n ], XmNnavigationType, XmNONE );
    n++;
    XtSetArg( xt_args[ n ], XmNtraversalOn, False );
    n++;
#endif
    f = XtCreateManagedWidget( "frame", xmFrameWidgetClass, rc, xt_args, n );

    n = 0;
    lcd = XtCreateManagedWidget( "lcd", xmDrawingAreaWidgetClass, f, xt_args, n );

    /* Add expose callback on lcd widget */
    XtAddCallback( lcd, XmNexposeCallback, lcdExposed, ( XtPointer )NULL );

    /* Keyboard form */
    /* Force nonmodal navigation only */
    n = 0;
#ifdef FORCE_NONMODAL
    XtSetArg( xt_args[ n ], XmNnavigationType, XmNONE );
    n++;
    XtSetArg( xt_args[ n ], XmNtraversalOn, False );
    n++;
#endif
    XtSetArg( xt_args[ n ], XmNallowOverlap, False );
    n++;

    kbd = XtCreateManagedWidget( "kbd", xmFormWidgetClass, /* widget_class */
                                 rc, xt_args, n );

    /* Create keys */
    for ( k = 0; k < opt.nKeys; k++ )
        CreateKey( k, kbd );

    /* 3.15: Put a message TextField under the keyboard;
       it is used to display the most important messages
    */
    n = 0;
    XtSetArg( xt_args[ n ], XmNeditable, False );
    n++;

    msg_text_field = XtCreateManagedWidget( "msg", xmTextFieldWidgetClass, /* widget_class */
                                            rc, xt_args, n );

    /* Realize the widget tree; this is required *before* execution of
       the code that follows.
    */
    XtRealizeWidget( shell_widget );

    /* Resize the main window so that scrollbars are initially not needed;
       the outer shell_widget must have XmNallowShellResize set to True
       for this to work.

       This method is not so nice, but it circumvents the well-known
       100x100 (undocumented) default size of XmMainWindow and
       XmScrolledWindow widgets.
    */
    {
        Dimension w, h, sht;

        n = 0;
        XtSetArg( xt_args[ n ], XmNwidth, &w );
        n++;
        XtSetArg( xt_args[ n ], XmNheight, &h );
        n++;
        XtGetValues( rc, xt_args, n );

        n = 0;
        XtSetArg( xt_args[ n ], XmNshadowThickness, &sht );
        n++;
        XtGetValues( mw, xt_args, n );

        n = 0;
        XtSetArg( xt_args[ n ], XmNwidth, w + 2 * sht );
        n++;
        XtSetArg( xt_args[ n ], XmNheight, h + 2 * sht );
        n++;
        XtSetValues( mw, xt_args, n );
    }

    /* Get window, foreground and background pixel values of lcd window,
       and store them into output arguments.  The widget must be
       realized.

       'Pixel' should be the same as 'unsigned long', but can't be sure.
    */
    n = 0;
    XtSetArg( xt_args[ n ], XmNforeground, &lcd_fg_pixel );
    n++;
    XtSetArg( xt_args[ n ], XmNbackground, &lcd_bg_pixel );
    n++;
    XtGetValues( lcd, xt_args, n );

    *lcd_display = XtDisplay( lcd );
    *lcd_window = XtWindow( lcd );
    *lcd_fg = ( unsigned long )lcd_fg_pixel;
    *lcd_bg = ( unsigned long )lcd_bg_pixel;
}

/* This function handles the timeout registered by IdleXLoop();
   it does nothing, but its invocation unlocks the XtAppProcessEvent()
   invocation done in the same function.
*/
static void IdleTimeOutHandler( XtPointer closure, XtIntervalId* id )
{
    debug1( DEBUG_C_TRACE, X11_I_CALLED, "IdleTimeOutHandler" );
    *( ( int* )closure ) = 1;
}

/* This function resets all toggle buttons of the emulated keyboard,
   recursively descending the widget tree starting from w.
*/
static void ResetToggleButtons( Widget w )
{

    debug1( DEBUG_C_TRACE, X11_I_CALLED, "ResetToggleButtons" );
    debug1( DEBUG_C_TRACE | DEBUG_C_X11, X11_I_HIER, XtName( w ) );

    if ( XtIsSubclass( w, xmToggleButtonWidgetClass ) ) {
        XEvent xe;                   /* Fake X Event */
        Display* d = XtDisplay( w ); /* Display */
        Window xw = XtWindow( w );   /* Window */

        /* Create a fake X Event to pass to the action proc;
           a full-fledged event with all fields set (almost) correctly
           is necessary because we don't know in advance how the action
           proc will use it.
        */
        xe.type = ButtonRelease;
        xe.xbutton.serial = LastKnownRequestProcessed( d );
        xe.xbutton.send_event = True;
        xe.xbutton.display = d;
        xe.xbutton.window = xw;
        xe.xbutton.root = RootWindow( d, XScreenNumberOfScreen( XtScreen( w ) ) );
        xe.xbutton.subwindow = xw;
        xe.xbutton.time = XtLastTimestampProcessed( d );
        xe.xbutton.x = xe.xbutton.y = 0;
        xe.xbutton.x_root = xe.xbutton.y_root = 0;
        xe.xbutton.state = ( unsigned int )0;
        xe.xbutton.button = ( unsigned int )1;
        xe.xbutton.same_screen = True;

        /* Disarm the ToggleButton */
        XtCallActionProc( w, "Disarm", &xe, NULL, 0 );
    } else if ( XtIsComposite( w ) ) {
        WidgetList children;
        Cardinal num_children;
        int c;

        Arg xt_args[ 20 ];
        int n;

        /* Widget is composite; walk down recursively.  Recursion will
           end, because this is a tree... at least I hope.  8-)
        */
        n = 0;
        XtSetArg( xt_args[ n ], XmNchildren, &children );
        n++;
        XtSetArg( xt_args[ n ], XmNnumChildren, &num_children );
        n++;

        XtGetValues( w, xt_args, n );

        debug1( DEBUG_C_TRACE | DEBUG_C_X11, X11_I_HIER_NC, num_children );

        for ( c = 0; c < num_children; c++ )
            ResetToggleButtons( children[ c ] );
    }
}

/*---------------------------------------------------------------------------
        Public functions
  ---------------------------------------------------------------------------*/

/* .+

.title	      : HandleXEvents
.kind	      : C function
.creation     : 19-Feb-1998
.description  :
  This function is called by the main emulator loop about every 1/16s.
  It must handle the X Events relevant for the application.

.call	      :
                HandleXEvents();
.input	      :
                void
.output	      :
                void
.status_codes :
                X11_I_CALLED
                X11_I_LCD_EXPOSE
                X11_I_KEY_PRESS
                X11_I_KEY_RELEASE
                X11_W_BAD_ACTION_CALL
                * Any other status code signaled by the main emulation
                  engine when reacting to the event
.notes	      :
  1.1, 19-Feb-1998, creation
  2.1, 6-Sep-2000,
    revised to accommodate the new GUI

.- */
void HandleXEvents( void )
{
    debug1( DEBUG_C_TRACE, X11_I_CALLED, "HandleXEvents" );

    /* If there is at least one event pending, process the events */
    while ( XtAppPending( app_context ) )
        XtAppProcessEvent( app_context, XtIMAll );
}

/* .+

.title	      : IdleXLoop
.kind	      : C function
.creation     : 19-Sep-2000
.description  :
  This function is called by the main emulator loop when the CPU
  has executed a SHUTDN instruction.

  It must handle the X Events relevant for the application and
  return to its caller when either max_wait milliseconds elaps
  or at least an X Event has occurred.

  It's ok to stay here a bit more than max_wait milliseconds,
  the emulator loop will compensate for the additional delay...

.call	      :
                IdleXLoop(max_wait);
.input	      :
                unsigned long max_wait, wait timeout (milliseconds)
.output	      :
                void
.status_codes :
                X11_I_CALLED
                X11_I_LCD_EXPOSE
                X11_I_KEY_PRESS
                X11_I_KEY_RELEASE
                X11_W_BAD_ACTION_CALL
                * Any other status code signaled by the main emulation
                  engine when reacting to the event
.notes	      :
  3.1, 19-Sep-2000, creation

.- */
void IdleXLoop( unsigned long max_wait )
{
    XtIntervalId timer;
    int expired = 0;

    debug1( DEBUG_C_TRACE, X11_I_CALLED, "IdleXLoop" );

    /* Register the timeout handler; this ensures that at least one
       event will arrive in about max_wait milliseconds (the timer
       expiration event).
    */
    timer = XtAppAddTimeOut( app_context, max_wait, IdleTimeOutHandler, ( XtPointer )&expired );

    /* Wait until there is at least one event pending, and process it */
    XtAppProcessEvent( app_context, XtIMAll );

    /* Process additional events if any, without blocking */
    HandleXEvents();

    /* Remote timeout handler if not expired yet; this avoid accumulation
       of pending, useless timeouts
    */
    if ( !expired )
        XtRemoveTimeOut( timer );
}

/* .+

.title	      : InitializeGui
.kind	      : C function
.creation     : 6-Sep-2000
.description  :
  This function initializes the GUI and store into *lcd the
  widget id of the lcd output window.

.call	      :
                InitializeGui(argc, argv, lcd);
.input	      :
                int argc, command line argument count
                char *argv[], command line argument vector
.output	      :
                void
.status_codes :
                X11_I_CALLED
                X11_I_USAGE
                X11_I_FACE
                X11_I_FOUND_CS
                X11_I_KEY
                X11_I_NKEYS
                X11_W_UNKNOWN_ATOM
                X11_E_BAD_OPTION
.notes	      :
  2.1, 6-Sep-2000, creation
  2.3, 11-Sep-2000, bug fix
    - signalling of X11_I_REVISION (X11_RCS_INFO) was missing
  3.15, 14-Nov-2000, update
    - added initialization of args.batchXfer
    - added initialization of error dialog

.- */
void InitializeGui( int argc, char* argv[] )
{
    struct app_opt opt;
    Display* lcd_display;
    Window lcd_window;
    unsigned long lcd_fg, lcd_bg;

    debug1( DEBUG_C_TRACE | DEBUG_C_REVISION, X11_I_REVISION, X11_RCS_INFO );

    /* Initialize GUI */
    InitializeXt( argc, argv, &opt );
    InitializeFSB();
    InitializeErrorDialog();
    InitializeWidgets( opt.face, &lcd_display, &lcd_window, &lcd_fg, &lcd_bg );

    /* Fill the emulator options data structure. */
    args.reset = opt.reset;
    args.monitor = opt.monitor;
    args.batchXfer = opt.batchXfer;
    args.mod_file_name = GetPathname( opt.stateDir, opt.mod );
    args.cpu_file_name = GetPathname( opt.stateDir, opt.cpu );
    args.hdw_file_name = GetPathname( opt.stateDir, opt.hdw );
    args.rom_file_name = GetPathname( opt.stateDir, opt.rom );
    args.ram_file_name = GetPathname( opt.stateDir, opt.ram );
    args.port_1_file_name = GetPathname( opt.stateDir, opt.port1 );
    args.port_2_file_name = GetPathname( opt.stateDir, opt.port2 );
    args.hw = opt.hw;

    /* Initialize LCD window */
    InitLcd( lcd_display, lcd_window, lcd_fg, lcd_bg );
}

/* .+

.title	      : ActivateFSB
.kind	      : C function
.creation     : 7-Nov-2000
.description  :
  This function activates the File Selection Box.  When one of its buttons
  is activated, the File Selection Box is removed and 'continuation'
  is invoked.

  Notice that the 'file_name' argument to the continuation is meaningful
  only when 'proceed' is true: if 'proceed' is false, the continuation
  should cancel its operation.

.call	      :
                ActivateFSB(title, continuation)
.input	      :
                char *title, title of the File Selection Box
                char *file_name, default file name to select
                FsbContinuation continuation, procedure to invoke when
                  one of the File Selection Box buttons is pressed
.output	      :
                void
.status_codes :
                X11_I_CALLED
.notes	      :
  3.13, 7-Nov-2000, creation
  3.14, 10-Nov-2000, bug fix
    - force a search in file_sel_box; ensure that file_name is properly
      qualified

.- */
void ActivateFSB( char* title, char* file_name, FsbContinuation continuation )
{
    XmString xm_title, xm_base_name, xm_qual_name;
    char *base_name, *qual_name;

    Arg xt_args[ 20 ];
    int n;

    debug1( DEBUG_C_TRACE, X11_I_CALLED, "ActivateFSB" );

    /* Reset GUI: keys and (recursively) buttons.  This is necessary,
       because the FSB is full application modal and immediately
       redirects all X Events (including key and button release events)
       onto itself; this may leave the GUI in an inconsistent state
       if it is not reset by hand here.
    */
    KeybReset();
    ResetToggleButtons( shell_widget );

    /* The title of the file selection dialog shell is an XmString */
    xm_title = XmStringCreate( title, XmFONTLIST_DEFAULT_TAG );

    /* Force a new search in file_sel_box; this ensures that
       any new file actually shows up in the lists.
    */
    XmFileSelectionDoSearch( file_sel_box, ( XmString )NULL );

    /* Get base name from search */
    n = 0;
    XtSetArg( xt_args[ n ], XmNdirSpec, &xm_base_name );
    n++;
    XtGetValues( file_sel_box, xt_args, n );

    /* Concatenate the base name with file_name; this is done in the
       text domain, because the concatenation in the XmString domain
       does not work correctly in this case (the resulting string has
       two segments; even if they are compatible they are not
       coalesced into a single one).
    */
    if ( !XmStringGetLtoR( xm_base_name, XmFONTLIST_DEFAULT_TAG, &base_name ) ) {
        ChfCondition X11_E_NO_FSB_TSEG, CHF_ERROR ChfEnd;
        ChfSignal();

        base_name = "";
    }

    qual_name = ( char* )XtMalloc( strlen( base_name ) + strlen( file_name ) + 1 );

    strcpy( qual_name, base_name );
    strcat( qual_name, file_name );

    xm_qual_name = XmStringCreate( qual_name, XmFONTLIST_DEFAULT_TAG );

    /* Set title of file selection dialog shell indirectly, through
       the FSB XmNdialogTitle resource, and set the default file name
       (value of XmNdirSpec) to 'xm_file_name'.
    */
    n = 0;
    XtSetArg( xt_args[ n ], XmNdialogTitle, xm_title );
    n++;
    XtSetArg( xt_args[ n ], XmNdirSpec, xm_qual_name );
    n++;
    XtSetValues( file_sel_box, xt_args, n );

    /* Free all dynamically-allocated strings */
    XtFree( base_name );
    XtFree( qual_name );
    XmStringFree( xm_title );
    XmStringFree( xm_base_name );
    XmStringFree( xm_qual_name );

    /* Store continuation */
    fsb_cont = continuation;

    /* Pop file selection dialog up */
    XtManageChild( file_sel_box );
}
