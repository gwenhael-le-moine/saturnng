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

.identifier   : $Id: t48.c,v 4.1 2000/12/11 09:54:19 cibrario Rel $
.context      : SATURN, Saturn CPU / HP48 emulator
.title	      : $RCSfile: t48.c,v $
.kind	      : C source
.author	      : Ivan Cibrario B.
.site	      : CSTV-CNR
.creation     :	17-Feb-1998
.keywords     : *
.description  :
  This file contains the X Window System interface of the emulator.

.include      : config.h, machdep.h, cpu.h

.notes	      :
  $Log: t48.c,v $
  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.10  2000/10/24 16:15:00  cibrario
  Added/Replaced GPL header

  Revision 1.1  1998/02/19 12:01:36  cibrario
  Initial revision


.- */

#ifndef lint
static char rcs_id[] = "$Id: t48.c,v 4.1 2000/12/11 09:54:19 cibrario Rel $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <sys/time.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#include "config.h"
#include "machdep.h"
#include "cpu.h"
#include "modules.h"
#include "display.h"
#include "keyb.h"
#include "x11.h"
#include "args.h"
#include "debug.h"

#define CHF_MODULE_ID X11_CHF_MODULE_ID
#include <Chf.h>

/*---------------------------------------------------------------------------
   Chf parameters - Do not change.
   The ABNORMAL_EXIT_CODE is taken from stdlib.h (EXIT_FAILURE)
  ---------------------------------------------------------------------------*/

#define MAIN_MSGCAT_NAME "t48.cat"
#define CONDITION_STACK_SIZE 16
#define HANDLER_STACK_SIZE 8
#define ABNORMAL_EXIT_CODE EXIT_FAILURE

/*---------------------------------------------------------------------------
   Other parameters
  ---------------------------------------------------------------------------*/

#define APP_CLASS "T48"
#define LCD_BACKGROUND "green"
#define LCD_FOREGROUND "black"

/*---------------------------------------------------------------------------
        Private variables
  ---------------------------------------------------------------------------*/

static XtAppContext app_context;
static Widget shell_widget;

/* Command line options descriptor */
XrmOptionDescRec options[] = {
    /* option, specifier, argKind, value */
    { "-reset", "*reset", XrmoptionNoArg, "True" },
    { "-monitor", "*monitor", XrmoptionNoArg, "True" },
    { "-path", "*path", XrmoptionSepArg },
    { "-cpu", "*cpu", XrmoptionSepArg },
    { "-mod", "*mod", XrmoptionSepArg },
    { "-hdw", "*hdw", XrmoptionSepArg },
    { "-rom", "*rom", XrmoptionSepArg },
    { "-ram", "*ram", XrmoptionSepArg },
    { "-port1", "*port1", XrmoptionSepArg },
    { "-port2", "*port2", XrmoptionSepArg }
};

#define NUM_OPTIONS ( sizeof( options ) / sizeof( XrmOptionDescRec ) )

/* Application fallback resources */
String fallback_resources[] = {
    NULL /* Null terminated */
};

/* Application options container */
struct app_opt {
    Boolean reset;
    Boolean monitor;
    Pixel fg_pix;
    Pixel bg_pix;
    String path;
    String cpu;
    String mod;
    String hdw;
    String rom;
    String ram;
    String port1;
    String port2;
};

/* Application resources/options descriptor */
XtResource app_res[] = {
    {"foreground", "Foreground", XtRPixel,   sizeof( Pixel ),   XtOffsetOf( struct app_opt, fg_pix ),  XtRString, LCD_FOREGROUND},
    {"background", "Background", XtRPixel,   sizeof( Pixel ),   XtOffsetOf( struct app_opt, bg_pix ),  XtRString, LCD_BACKGROUND},
    {"reset",      "Reset",      XtRBoolean, sizeof( Boolean ), XtOffsetOf( struct app_opt, reset ),   XtRString, "False"       },
    {"monitor",    "Monitor",    XtRBoolean, sizeof( Boolean ), XtOffsetOf( struct app_opt, monitor ), XtRString, "False"       },
    {"path",       "Path",       XtRString,  sizeof( String ),  XtOffsetOf( struct app_opt, path ),    XtRString, "."           },
    {"cpu",        "Cpu",        XtRString,  sizeof( String ),  XtOffsetOf( struct app_opt, cpu ),     XtRString, "cpu"         },
    {"mod",        "Mod",        XtRString,  sizeof( String ),  XtOffsetOf( struct app_opt, mod ),     XtRString, "mod"         },
    {"hdw",        "Hdw",        XtRString,  sizeof( String ),  XtOffsetOf( struct app_opt, hdw ),     XtRString, "hdw"         },
    {"rom",        "Rom",        XtRString,  sizeof( String ),  XtOffsetOf( struct app_opt, rom ),     XtRString, "rom"         },
    {"ram",        "Ram",        XtRString,  sizeof( String ),  XtOffsetOf( struct app_opt, ram ),     XtRString, "ram"         },
    {"port1",      "Port1",      XtRString,  sizeof( String ),  XtOffsetOf( struct app_opt, port1 ),   XtRString, "port1"       },
    {"port2",      "Port2",      XtRString,  sizeof( String ),  XtOffsetOf( struct app_opt, port2 ),   XtRString, "port2"       }
};

#define NUM_APP_RES ( sizeof( app_res ) / sizeof( XtResource ) )

/*---------------------------------------------------------------------------
        Public variables
  ---------------------------------------------------------------------------*/

/* Emulator options */
struct Args args;

/*---------------------------------------------------------------------------
        Private functions
  ---------------------------------------------------------------------------*/

/* KeySym -> HP48 'enum Key' translator */
static enum Key Ks2K( KeySym ks )
{
    switch ( ks ) {
        /* Backspace */
        case XK_BackSpace:
            return KEY_BKSP;

        /* Delete */
        case XK_Delete:
            return KEY_DEL;
        case XK_KP_Delete:
            return KEY_DEL;

        /* Enter */
        case XK_KP_Enter:
            return KEY_ENTER;
        case XK_Return:
            return KEY_ENTER;

        /* Cursor keys */
        case XK_KP_Left:
            return KEY_LEFT;
        case XK_Left:
            return KEY_LEFT;

        case XK_KP_Right:
            return KEY_RIGHT;
        case XK_Right:
            return KEY_RIGHT;

        case XK_KP_Up:
            return KEY_UP;
        case XK_Up:
            return KEY_UP;

        case XK_KP_Down:
            return KEY_DOWN;
        case XK_Down:
            return KEY_DOWN;

        /* Function keys */
        case XK_F1:
            return KEY_F1;
        case XK_F2:
            return KEY_F2;
        case XK_F3:
            return KEY_F3;
        case XK_F4:
            return KEY_F4;
        case XK_F5:
            return KEY_F5;
        case XK_F6:
            return KEY_F6;

        /* Shift L, R, Alpha, ON */
        case XK_Shift_L:
            return KEY_SH_L;
        case XK_Shift_R:
            return KEY_SH_R;
        case XK_Alt_L:
            return KEY_ALPHA;
        case XK_Alt_R:
            return KEY_ALPHA;
        case XK_Escape:
            return KEY_ON;

        /* Numeric keypad and surroundings */
        case XK_KP_0:
            return KEY_0;
        case XK_KP_1:
            return KEY_1;
        case XK_KP_2:
            return KEY_2;
        case XK_KP_3:
            return KEY_3;
        case XK_KP_4:
            return KEY_4;
        case XK_KP_5:
            return KEY_5;
        case XK_KP_6:
            return KEY_6;
        case XK_KP_7:
            return KEY_7;
        case XK_KP_8:
            return KEY_8;
        case XK_KP_9:
            return KEY_9;

        case XK_KP_Decimal:
            return KEY_DOT;

        case XK_KP_Add:
            return KEY_ADD;
        case XK_plus:
            return KEY_ADD;

        case XK_KP_Subtract:
            return KEY_SUB;
        case XK_minus:
            return KEY_SUB;

        case XK_KP_Multiply:
            return KEY_MUL;
        case XK_asterisk:
            return KEY_MUL;

        case XK_KP_Divide:
            return KEY_DIV;
        case XK_slash:
            return KEY_DIV;

        case XK_KP_Space:
            return KEY_SPC;
        case XK_space:
            return KEY_SPC;

        /* Upper half of the keyboard */
        case XK_1:
            return KEY_MTH;
        case XK_2:
            return KEY_PRG;
        case XK_3:
            return KEY_CST;
        case XK_4:
            return KEY_VAR;
        case XK_5:
            return KEY_UP;
        case XK_6:
            return KEY_NXT;

        case XK_q:
            return KEY_AP;
        case XK_w:
            return KEY_STO;
        case XK_e:
            return KEY_EVAL;
        case XK_r:
            return KEY_LEFT;
        case XK_t:
            return KEY_DOWN;
        case XK_y:
            return KEY_RIGHT;

        case XK_a:
            return KEY_SIN;
        case XK_s:
            return KEY_COS;
        case XK_d:
            return KEY_TAN;
        case XK_f:
            return KEY_SQRT;
        case XK_g:
            return KEY_POWER;
        case XK_h:
            return KEY_INV;

        case XK_z:
            return KEY_ENTER;
        case XK_x:
            return KEY_ENTER;
        case XK_c:
            return KEY_CHS;
        case XK_v:
            return KEY_EEX;
        case XK_b:
            return KEY_DEL;
        case XK_n:
            return KEY_BKSP;

        /* Other useful aliases */
        case XK_Tab:
            return KEY_NXT;
        case XK_apostrophe:
            return KEY_AP;
        case XK_period:
            return KEY_DOT;

        /* Save */
        case XK_Control_L:
            ModSave();
            CpuSave();
            exit( 0 );
    }

    return KEY_NULL;
}

/* X Event handler, called by the X Toolkit when appropriate */
static void XEventHandler( Widget w, XtPointer cl_data, XEvent* ev, Boolean* cont )
{
    KeySym ks;

    debug1( DEBUG_C_TRACE | DEBUG_C_X11, X11_I_CALLED, "XEventHandler" );

    /* Continue to dispatch */
    *cont = True;

    if ( ev->type == Expose ) {
        debug1( DEBUG_C_X11, X11_I_LCD_EXPOSE, ev->xexpose.count );
        if ( ev->xexpose.count == 0 )
            RefreshLcd();
    }

    else if ( ev->type == KeyPress || ev->type == KeyRelease ) {
        ( void )XLookupString( ( XKeyEvent* )ev, ( char* )NULL, 0, &ks, ( XComposeStatus* )NULL );

        if ( ev->type == KeyPress ) {
            debug1( DEBUG_C_X11, X11_I_KEY_PRESS, ks );
            KeybPress( Ks2K( ks ) );
        }

        else if ( ev->type = KeyRelease ) {
            debug1( DEBUG_C_X11, X11_I_KEY_RELEASE, ks );
            KeybRelease( Ks2K( ks ) );
        }
    }

    else {
        /* Unknown X Event - discard */
        debug1( DEBUG_C_X11, X11_I_X_EVENT, ev->type );
    }
}

/* Path/name dynamic allocator */
static char* GetPathname( String path, String name )
{
    char* s = malloc( strlen( path ) + strlen( name ) + 2 );

    strcpy( s, path );
    strcat( s, "/" );
    strcat( s, name );
    return s;
}

/* Initialize the X interface */
void InitializeX( int argc, char* argv[], struct app_opt* opt )
{
    unsigned long fg_pix, bg_pix;
    XSetWindowAttributes attr;
    Arg xt_args[ 20 ];
    int n;

    debug1( DEBUG_C_TRACE, X11_I_CALLED, "InitializeX" );

    /* Setup Arg vector for shell widget creation */
    n = 0;
    XtSetArg( xt_args[ n ], XtNwidth, 131 * 2 + 2 * 8 );
    n++;
    XtSetArg( xt_args[ n ], XtNminWidth, 131 * 2 + 2 * 8 );
    n++;
    XtSetArg( xt_args[ n ], XtNmaxWidth, 131 * 2 + 2 * 8 );
    n++;
    XtSetArg( xt_args[ n ], XtNheight, 64 * 2 + 28 );
    n++;
    XtSetArg( xt_args[ n ], XtNminHeight, 64 * 2 + 28 );
    n++;
    XtSetArg( xt_args[ n ], XtNmaxHeight, 64 * 2 + 28 );
    n++;
    XtSetArg( xt_args[ n ], XtNx, 0 );
    n++;
    XtSetArg( xt_args[ n ], XtNy, 0 );
    n++;
    XtSetArg( xt_args[ n ], XtNinput, True );
    n++;

    /* Initialize application, parse command line options,
       create its main shell.
    */
    shell_widget = XtAppInitialize( &app_context, APP_CLASS, options, NUM_OPTIONS, &argc, argv, fallback_resources, xt_args, n );

    /* Check unknown options - argv[0] always contains program name */
    if ( argc > 1 ) {
        int i;
        for ( i = 1; i < argc; i++ )
            ChfCondition X11_E_BAD_OPTION, CHF_ERROR, argv[ i ] ChfEnd;

        ChfCondition X11_I_USAGE, CHF_INFO, argv[ 0 ] ChfEnd;
        ChfSignal();
    }

    /* Get application options and fill the 'struct app_opt opt' structure */
    XtGetApplicationResources( shell_widget, ( XtPointer )opt, app_res, NUM_APP_RES, ( ArgList )NULL, ( Cardinal )0 );

    /* Add private event handler for the main shell */
    XtAddEventHandler( shell_widget, KeyPressMask | KeyReleaseMask | ExposureMask, False, /* Don't call with non-maskable events */
                       XEventHandler, ( XtPointer )NULL );

    /* Realize shell widget */
    XtRealizeWidget( shell_widget );
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
                X11_I_X_EVENT
.notes	      :
  1.1, 19-Feb-1998, creation

.- */
void HandleXEvents( void )
{
    debug1( DEBUG_C_TRACE, X11_I_CALLED, "HandleXEvents" );

    /* If there is at least one event pending, process the first event.
       NOTE: This is an IF instead of a WHILE for two reasons:

       - to be sure that X Events related to the keyboard interface are
         actually 'seen' by the HP48, since the current keyboard emulator
         has no key memory.

       - this function will be called again very soon, so the Events queue
         will not starve anyway.
    */

    if ( XtAppPending( app_context ) )
        XtAppProcessEvent( app_context, XtIMAll );
}

/* .+

.title	      : main
.kind	      : C function
.creation     : 19-Feb-1998
.description  :
  Main program.

.notes	      :
  1.1, 19-Feb-1998, creation

.- */
int main( int argc, char* argv[] )
{
    struct app_opt opt;

    /* Chf initialization with msgcat subsystem */
    if ( ChfMsgcatInit( argv[ 0 ],            /* Application's name */
                        CHF_DEFAULT,          /* Options */
                        MAIN_MSGCAT_NAME,     /* Name of the message catalog */
                        CONDITION_STACK_SIZE, /* Size of the condition stack */
                        HANDLER_STACK_SIZE,   /* Size of the handler stack */
                        ABNORMAL_EXIT_CODE    /* Abnormal exit code */
                        ) != CHF_S_OK ) {
        fprintf( stderr, "Chf initialization failed\n" );
        exit( ABNORMAL_EXIT_CODE );
    }

    debug1( DEBUG_C_TRACE | DEBUG_C_REVISION, X11_I_REVISION, "$Id: t48.c,v 4.1 2000/12/11 09:54:19 cibrario Rel $" );

    /* Initialize X */
    InitializeX( argc, argv, &opt );

    /* Initialize LCD window */
    InitLcd( XtDisplay( shell_widget ), XtWindow( shell_widget ), opt.fg_pix, opt.bg_pix );

    /* Fill the emulator options data structure */
    args.cpu_file_name = GetPathname( opt.path, opt.cpu );
    args.mod_file_name = GetPathname( opt.path, opt.mod );
    args.hdw_file_name = GetPathname( opt.path, opt.hdw );
    args.rom_file_name = GetPathname( opt.path, opt.rom );
    args.ram_file_name = GetPathname( opt.path, opt.ram );
    args.port_1_file_name = GetPathname( opt.path, opt.port1 );
    args.port_2_file_name = GetPathname( opt.path, opt.port2 );

    /* Initialize peripheral modules */
    ModInit();

    /* Initialize peripheral modules */
    CpuInit();

    /* Reset the system, if required */
    if ( opt.reset ) {
        ModReset();
        CpuReset();
    }

    if ( opt.monitor ) {
        /* Call Monitor */
        Monitor();
    }

    else {
        /* Call Emulator directly */
        Emulator();
    }

    return EXIT_SUCCESS;
}
