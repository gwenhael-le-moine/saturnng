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

.identifier   : $Id: display.c,v 4.1.1.1 2002/11/11 16:12:46 cibrario Exp $
.context      : SATURN, Saturn CPU / HP48 emulator
.title	      : $RCSfile: display.c,v $
.kind	      : C source
.author	      : Ivan Cibrario B.
.site	      : CSTV-CNR
.creation     :	29-Jan-1998
.keywords     : *
.description  :
  This source module emulates the Lcd driver of the Yorke chip.

  References:

    SASM.DOC by HP  (HORN disk 4)
    Guide to the Saturn Processor Rev. 0.00f by Matthew Mastracci
    entries.srt by Mika Heiskanen  (mheiskan@vipunen.hut.fi)
    x48 source code by Eddie C. Dost  (ecd@dressler.de)

  NOTE: In the current (r1.1) implementation, the control fields
        mod_status.hdw.lcd_offset and mod_status.hdw.lcd_contrast are
        not supported.  Therefore, the emulation accuracy is sometimes
        poor; for example, the Equation Writer does not work well with
        large equations.

.include      : config.h, machdep.h, cpu.h, modules.h, display.h

.notes	      :
  $Log: display.c,v $
  Revision 4.1.1.1  2002/11/11 16:12:46  cibrario
  Small screen support; preliminary

  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.10  2000/10/24 16:14:37  cibrario
  Added/Replaced GPL header

  Revision 3.8  2000/10/23 13:15:36  cibrario
  Bug fix:
  In InitLcd(), added a clip rectangle to GC, to avoid drawing non-existent
  pixels, that is, pixels that *do* exist in the frame buffer, but should
  never be viewed on screen.

  Revision 3.5  2000/10/02 09:44:42  cibrario
  Linux support:
  - gcc does not like array subscripts with type 'char', and it is right.

  Revision 3.1  2000/09/20 13:47:58  cibrario
  Minor updates and fixes to avoid gcc compiler warnings on Solaris
  when -ansi -pedantic -Wall options are selected.

 * Revision 1.1  1998/02/17  14:14:39  cibrario
 * Initial revision
 *

.- */

#ifndef lint
static char rcs_id[] = "$Id: display.c,v 4.1.1.1 2002/11/11 16:12:46 cibrario Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h> /* 3.1: memset() */

#include <X11/Xlib.h>

#include "config.h"
#include "machdep.h"
#include "cpu.h"
#include "modules.h"
#include "display.h"
#include "x11.h"
#include "debug.h"

#define CHF_MODULE_ID X11_CHF_MODULE_ID
#include <Chf.h>

#ifndef LCD_MAG
#  define LCD_MAG 2 /* 4.1.1.1: Compat. default */
#endif

#define NIBBLES_PER_ROW 34                   /* 136 pixel total */
#define MAX_ROWS 64                          /* 64 rows total */
#define N_ANN 6                              /* # of annunciators */
#define LCD_X_ORIGIN 1 + 4 * ( LCD_MAG - 1 ) /* x origin */

#if LCD_MAG == 1
#  define LCD_Y_ORIGIN 14 /* y origin */
#else
#  define LCD_Y_ORIGIN 20 /* y origin */
#endif

/* 3.8: Origin and size of clip rectangle */
#define LCD_CLIP_X_ORIGIN LCD_X_ORIGIN
#define LCD_CLIP_Y_ORIGIN 0 /* Don't clip annunciators */
#define LCD_CLIP_WIDTH 131 * LCD_MAG
#define LCD_CLIP_HEIGHT LCD_Y_ORIGIN + 64 * LCD_MAG

#define MASK_ANN_LEFT 0x81 /* Annunciator's bit masks */
#define MASK_ANN_RIGHT 0x82
#define MASK_ANN_ALPHA 0x84
#define MASK_ANN_BATTERY 0x88
#define MASK_ANN_BUSY 0x90
#define MASK_ANN_IO 0xA0

/*---------------------------------------------------------------------------
        Static/Global variables
  ---------------------------------------------------------------------------*/

static /*const*/ char nibble_bitmap_data[ NIBBLE_VALUES ][ LCD_MAG ] = {
#if LCD_MAG == 1
    { 0x00 }, /* ---- */
    { 0x01 }, /* *--- */
    { 0x02 }, /* -*-- */
    { 0x03 }, /* **-- */
    { 0x04 }, /* --*- */
    { 0x05 }, /* *-*- */
    { 0x06 }, /* -**- */
    { 0x07 }, /* ***- */
    { 0x08 }, /* ---* */
    { 0x09 }, /* *--* */
    { 0x0a }, /* -*-* */
    { 0x0b }, /* **-* */
    { 0x0c }, /* --** */
    { 0x0d }, /* *-** */
    { 0x0e }, /* -*** */
    { 0x0f }  /* **** */
#elif LCD_MAG == 2
    { 0x00, 0x00 }, /* ---- */
    { 0x03, 0x03 }, /* *--- */
    { 0x0c, 0x0c }, /* -*-- */
    { 0x0f, 0x0f }, /* **-- */
    { 0x30, 0x30 }, /* --*- */
    { 0x33, 0x33 }, /* *-*- */
    { 0x3c, 0x3c }, /* -**- */
    { 0x3f, 0x3f }, /* ***- */
    { 0xc0, 0xc0 }, /* ---* */
    { 0xc3, 0xc3 }, /* *--* */
    { 0xcc, 0xcc }, /* -*-* */
    { 0xcf, 0xcf }, /* **-* */
    { 0xf0, 0xf0 }, /* --** */
    { 0xf3, 0xf3 }, /* *-** */
    { 0xfc, 0xfc }, /* -*** */
    { 0xff, 0xff }  /* **** */
#else
#  error "Bad LCD_MAG; supported values are 1 and 2"
#endif
};

static /*const*/ struct {
    int mask;               /* Bit mask */
    int x, y;               /* Position */
    int w, h;               /* Width, Height */
    char bitmap_data[ 24 ]; /* Bitmap data */
}

#define ANN_X( i ) ( 8 * LCD_MAG + ( 22 * LCD_MAG + 1 ) * i )
#define ANN_Y( i ) ( 1 + 3 * ( LCD_MAG - 1 ) )

ann_data[ N_ANN ] = {
    {MASK_ANN_LEFT,    ANN_X( 0 ), ANN_Y( 0 ), 15, 12, { 0xfe, 0x3f, 0xff, 0x7f, 0x9f, 0x7f, 0xcf, 0x7f, 0xe7, 0x7f, 0x03, 0x78,
                                                       0x03, 0x70, 0xe7, 0x73, 0xcf, 0x73, 0x9f, 0x73, 0xff, 0x73, 0xfe, 0x33 }   },
    {MASK_ANN_RIGHT,   ANN_X( 1 ), ANN_Y( 1 ), 15, 12, { 0xfe, 0x3f, 0xff, 0x7f, 0xff, 0x7c, 0xff, 0x79, 0xff, 0x73, 0x0f, 0x60,
                                                        0x07, 0x60, 0xe7, 0x73, 0xe7, 0x79, 0xe7, 0x7c, 0xe7, 0x7f, 0xe6, 0x3f }  },
    {MASK_ANN_ALPHA,   ANN_X( 2 ), ANN_Y( 2 ), 15, 12, { 0xe0, 0x03, 0x18, 0x44, 0x0c, 0x4c, 0x06, 0x2c, 0x07, 0x2c, 0x07, 0x1c,
                                                        0x07, 0x0c, 0x07, 0x0c, 0x07, 0x0e, 0x0e, 0x4d, 0xf8, 0x38, 0x00, 0x00 }  },
    {MASK_ANN_BATTERY, ANN_X( 3 ), ANN_Y( 3 ), 15, 12, { 0x04, 0x10, 0x02, 0x20, 0x12, 0x24, 0x09, 0x48, 0xc9, 0x49, 0xc9, 0x49,
                                                          0xc9, 0x49, 0x09, 0x48, 0x12, 0x24, 0x02, 0x20, 0x04, 0x10, 0x00, 0x00 }},
    {MASK_ANN_BUSY,    ANN_X( 4 ), ANN_Y( 4 ), 15, 12, { 0xfc, 0x1f, 0x08, 0x08, 0x08, 0x08, 0xf0, 0x07, 0xe0, 0x03, 0xc0, 0x01,
                                                       0x40, 0x01, 0x20, 0x02, 0x10, 0x04, 0xc8, 0x09, 0xe8, 0x0b, 0xfc, 0x1f }   },
    {MASK_ANN_IO,      ANN_X( 5 ), ANN_Y( 5 ), 15, 12, { 0x0c, 0x00, 0x1e, 0x00, 0x33, 0x0c, 0x61, 0x18, 0xcc, 0x30, 0xfe, 0x7f,
                                                     0xfe, 0x7f, 0xcc, 0x30, 0x61, 0x18, 0x33, 0x0c, 0x1e, 0x00, 0x0c, 0x00 }     }
};

static Nibble lcd_buffer[ MAX_ROWS ][ NIBBLES_PER_ROW ];
static int ann_buffer;
static int clean;

static Display* display;
static Window window;
static GC gc;
static unsigned long fg_pixel, bg_pixel;
static unsigned int depth;

static Pixmap nibble_pixmap[ NIBBLE_VALUES ];
static Pixmap ann_pixmap[ N_ANN ];

/*---------------------------------------------------------------------------
        Private functions
  ---------------------------------------------------------------------------*/

/* .+

.title	      : InitPixmaps
.kind	      : C function
.creation     : 29-Jan-1998
.description  :
  This function initializes the pixmaps for the Lcd screen elements and
  stores them into the appropriate global variables.

.call	      :
                InitPixmaps();
.input	      :
                void
.output	      :
                void
.status_codes :
                X11_I_CALLED
                X11_F_X_ERROR
.notes	      :
  1.1, 29-Jan-1998, creation

.- */
static void InitPixmaps( void )
{
    int i;

    debug1( DEBUG_C_TRACE, X11_I_CALLED, "InitPixmaps" );

    /* Initialize nibble_pixmap */
    for ( i = 0; i < NIBBLE_VALUES; i++ ) {
        if ( ( nibble_pixmap[ i ] = XCreatePixmapFromBitmapData( display, window, nibble_bitmap_data[ i ], 4 * LCD_MAG, LCD_MAG, fg_pixel,
                                                                 bg_pixel, depth ) ) == None ) {
            ChfCondition X11_F_X_ERROR, CHF_FATAL ChfEnd;
            ChfSignal();
        }
    }

    /* Initialize ann_pixmap */
    for ( i = 0; i < N_ANN; i++ ) {
        if ( ( ann_pixmap[ i ] = XCreatePixmapFromBitmapData( display, window, ann_data[ i ].bitmap_data, ann_data[ i ].w, ann_data[ i ].h,
                                                              fg_pixel, bg_pixel, depth ) ) == None ) {
            ChfCondition X11_F_X_ERROR, CHF_FATAL ChfEnd;
            ChfSignal();
        }
    }
}

/* .+

.title	      : ClearLcd
.kind	      : C function
.creation     : 29-Jan-1998
.description  :
  This function clears the Lcd screen

.call	      :
                ClearLcd();
.input	      :
                void
.output	      :
                void
.status_codes :
                X11_I_CALLED
.notes	      :
  1.1, 29-Jan-1998, creation

.- */
static void ClearLcd( void )
{
    debug1( DEBUG_C_TRACE, X11_I_CALLED, "ClearLcd" );

    /* Clear Lcd display */
    ( void )memset( ( void* )lcd_buffer, 0, sizeof( lcd_buffer ) );
    ann_buffer = 0;

    XClearWindow( display, window );
    XFlush( display );
}

/*---------------------------------------------------------------------------
        Public funcitons
  ---------------------------------------------------------------------------*/

/* .+

.title	      : InitLcd
.kind	      : C function
.creation     : 29-Jan-1998
.description  :
  This function initializes the Lcd driver emulator and prepares it for use.
  The LCD screen is initially cleared.

.call	      :
                InitLcd(lcd_display, lcd_window, lcd_fg_pixel, lcd_bg_pixel);
.input	      :
                Display *lcd_display, X display
                Window lcd_window, X window to be used for display
                unsigned long lcd_fg_pixel, foreground color to be used
                unsigned long lcd_bg_pixel, background color to be used
.output	      :
                void
.status_codes :
                X11_I_CALLED
                X11_F_X_ERROR
.notes	      :
  1.1, 29-Jan-1998, creation
  3.8, 23-Oct-2000, bug fix:
  - added clip rectangle to GC, to avoid drawing non-existent pixels.

.- */
void InitLcd( Display* lcd_display, Window lcd_window, unsigned long lcd_fg_pixel, unsigned long lcd_bg_pixel )
{
    XWindowAttributes xwa;
    XGCValues gc_values;

    debug1( DEBUG_C_TRACE, X11_I_CALLED, "InitLcdWindow" );

    display = lcd_display;
    window = lcd_window;
    fg_pixel = lcd_fg_pixel;
    bg_pixel = lcd_bg_pixel;

    /* Get window attributes and initialize window depth */
    if ( XGetWindowAttributes( display, window, &xwa ) == 0 ) {
        ChfCondition X11_F_X_ERROR, CHF_FATAL ChfEnd;
        ChfSignal();
    }

    depth = xwa.depth;

    /* Create GC */
    gc_values.function = GXcopy;
    gc_values.plane_mask = AllPlanes;
    gc_values.subwindow_mode = IncludeInferiors;
    gc_values.foreground = lcd_fg_pixel;
    gc_values.background = lcd_bg_pixel;
    gc_values.graphics_exposures = False;

    gc = XCreateGC( display, window, GCFunction | GCPlaneMask | GCForeground | GCBackground | GCSubwindowMode | GCGraphicsExposures,
                    &gc_values );

    {
        /* 3.8: This clip rectangle prevents XCopyArea() (in DrawLcd()) from
           drawing non-visible pixels
        */
        XRectangle rect[ 1 ];

        rect[ 0 ].x = LCD_CLIP_X_ORIGIN; /* This is the clip rectangle */
        rect[ 0 ].y = LCD_CLIP_Y_ORIGIN;
        rect[ 0 ].width = LCD_CLIP_WIDTH;
        rect[ 0 ].height = LCD_CLIP_HEIGHT;

        XSetClipRectangles( display, gc, 0, 0, /* Alsolute clip X,Y origin */
                            rect, 1, YXBanded );
    }

    /* Initialize Pixmaps */
    InitPixmaps();

    /* Clear screen and initialize the static memory areas */
    ClearLcd();

    /* Set the 'display is clean' flag */
    clean = 1;
}

/* .+

.title	      : DrawLcd
.kind	      : C function
.creation     : 29-Jan-1998
.description  :
  This function redraws the Lcd screen from the information contained in
  the mod_status.hdw structure.

.call	      :
                DrawLcd();
.input	      :
                void
.output	      :
                void
.status_codes :
                X11_I_CALLED
                X11_I_LCD_PAR
.notes	      :
  1.1, 29-Jan-1998, creation

.- */
void DrawLcd( void )
{
    Address addr = mod_status.hdw.lcd_base_addr;
    int y, x;
    Nibble v;

    debug1( DEBUG_C_TRACE, X11_I_CALLED, "DrawLcd" );

    /* If the debug class DEBUG_C_DISPLAY is enabled, print the display
       parameters
    */
    debug2( DEBUG_C_DISPLAY, X11_I_LCD_PAR, "_base_addr", ( int )mod_status.hdw.lcd_base_addr );

    debug2( DEBUG_C_DISPLAY, X11_I_LCD_PAR, "_on", ( int )mod_status.hdw.lcd_on );

    debug2( DEBUG_C_DISPLAY, X11_I_LCD_PAR, "_contrast", ( int )mod_status.hdw.lcd_contrast );

    debug2( DEBUG_C_DISPLAY, X11_I_LCD_PAR, "_vlc", ( int )mod_status.hdw.lcd_vlc );

    debug2( DEBUG_C_DISPLAY, X11_I_LCD_PAR, "_offset", ( int )mod_status.hdw.lcd_offset );

    debug2( DEBUG_C_DISPLAY, X11_I_LCD_PAR, "_line_offset", ( int )mod_status.hdw.lcd_line_offset );

    debug2( DEBUG_C_DISPLAY, X11_I_LCD_PAR, "_menu_addr", ( int )mod_status.hdw.lcd_menu_addr );

    debug2( DEBUG_C_DISPLAY, X11_I_LCD_PAR, "_ann", ( int )mod_status.hdw.lcd_ann );

    /* Check if display is on */
    if ( !mod_status.hdw.lcd_on ) {
        /* Display is off; clear lcd if necessary */
        if ( !clean ) {
            /* Set the 'display is clean' flag  and clear the screen */
            clean = 1;
            ClearLcd();
        }
        return;
    }

    /* The display is on and will be no longer clean */
    clean = 0;

    /* Scan active display rows */
    for ( y = 0; y <= mod_status.hdw.lcd_vlc; y++ ) {
        /* Scan columns */
        for ( x = 0; x < NIBBLES_PER_ROW; x++ ) {
            v = FetchNibble( addr++ );
            if ( v != lcd_buffer[ y ][ x ] ) {
                lcd_buffer[ y ][ x ] = v;

                XCopyArea( display, nibble_pixmap[ ( int )v ], window, gc, 0, 0, /* src_x, src_y */
                           4 * LCD_MAG, LCD_MAG,                                 /* width, height */
                           x * 4 * LCD_MAG + LCD_X_ORIGIN, y * LCD_MAG + LCD_Y_ORIGIN );
            }
        }

        addr += mod_status.hdw.lcd_line_offset;
    }

    /* Scan menu display rows */
    addr = mod_status.hdw.lcd_menu_addr;
    for ( ; y < MAX_ROWS; y++ ) {
        /* Scan columns */
        for ( x = 0; x < NIBBLES_PER_ROW; x++ ) {
            v = FetchNibble( addr++ );
            if ( v != lcd_buffer[ y ][ x ] ) {
                lcd_buffer[ y ][ x ] = v;
                XCopyArea( display, nibble_pixmap[ ( int )v ], window, gc, 0, 0, /* src_x, src_y */
                           4 * LCD_MAG, LCD_MAG,                                 /* width, height */
                           x * 4 * LCD_MAG + LCD_X_ORIGIN, y * LCD_MAG + LCD_Y_ORIGIN );
            }
        }
    }

    /* Scan annunciators */
    if ( mod_status.hdw.lcd_ann != ann_buffer ) {
        ann_buffer = mod_status.hdw.lcd_ann;

        for ( y = 0; y < N_ANN; y++ ) {
            if ( ( ann_buffer & ann_data[ y ].mask ) == ann_data[ y ].mask ) {
                XCopyArea( display, ann_pixmap[ y ], window, gc, 0, 0, /* src_x, src_y */
                           ann_data[ y ].w, ann_data[ y ].h,           /* width, height */
                           ann_data[ y ].x, ann_data[ y ].y );
            } else {
                XClearArea( display, window, ann_data[ y ].x, ann_data[ y ].y, ann_data[ y ].w, ann_data[ y ].h, False /* No exposures */
                );
            }
        }
    }

    /* Flush display */
    XFlush( display );
}

/* .+

.title	      : RefreshLcd
.kind	      : C function
.creation     : 17-Feb-1998
.description  :
  This function refreshes the Lcd screen after a X Window Expose event.

.call	      :
                RefreshLcd();
.input	      :
                void
.output	      :
                void
.status_codes :
                X11_I_CALLED
.notes	      :
  1.1, 17-Feb-1998, creation

.- */
void RefreshLcd( void )
{
    debug1( DEBUG_C_TRACE, X11_I_CALLED, "RefreshLcd" );

    ClearLcd();
    DrawLcd();
}
