#ifndef _UI4x_INNER_H
#define _UI4x_INNER_H 1

#include "emulator.h"

#include "bitmaps_misc.h"
#include "bitmaps_fonts.h"

// Colors
/*                          48SX    48GX     49g    */
#define UI4X_COLOR_WHITE 0         /* #ffffff #ffffff #ffffff */
#define UI4X_COLOR_SHIFT_LEFT 1          /* #ffa600 #ffbaff #7476dd  */
#define UI4X_COLOR_SHIFT_RIGHT 2         /* #00d2ff #00ffcc #d46262  */
#define UI4X_COLOR_BUTTON_EDGE_TOP 3       /* #6d5d5d #646464 #646464 */
#define UI4X_COLOR_BUTTON 4        /* #5a4d4d #585858 #909b94  */
#define UI4X_COLOR_BUTTON_EDGE_BOTTOM 5       /* #4c4141 #4a4a4a #4a4a4a */
#define UI4X_COLOR_LCD_BG 6           /* #cadd5c #cadd5c #a8c0b0  */
#define UI4X_COLOR_LCD_PIXEL 7         /* #000080 #000080 #000020  */
#define UI4X_COLOR_FACEPLATE_EDGE_TOP 8       /* #6d4e4e #585858 #585858 */
#define UI4X_COLOR_FACEPLATE 9           /* #5a4040 #4a4a4a #4a4a4a */
#define UI4X_COLOR_FACEPLATE_EDGE_BOTTOM 10      /* #4c3636 #404040 #404040 */
#define UI4X_COLOR_UPPER_FACEPLATE_EDGE_TOP 11 /* #9b7654 #808080 #808080 */
#define UI4X_COLOR_UPPER_FACEPLATE 12     /* #7c5e43 #68686e #86a6b9  */
#define UI4X_COLOR_UPPER_FACEPLATE_EDGE_BOTTOM 13 /* #644b35 #54545a #97b7ca */
#define UI4X_COLOR_HP_LOGO 14         /* #cca96b #b0b0b8 #b0b0b8 */
#define UI4X_COLOR_HP_LOGO_BG 15    /* #404040 #68686e #68686e */
#define UI4X_COLOR_48GX_128K_RAM 16        /* #cab890 #f0f0f0 #f0f0f0 */
#define UI4X_COLOR_FRAME 17        /* #000000 #000000 #000000 */
#define UI4X_COLOR_KEYPAD_HIGHLIGHT 18     /* #3c2a2a #68686e #4f6165  */

#define FIRST_COLOR UI4X_COLOR_WHITE
#define LAST_COLOR UI4X_COLOR_KEYPAD_HIGHLIGHT
#define NB_COLORS ( LAST_COLOR + 1 )

/***********/
/* typedef */
/***********/
typedef struct letter_t {
    unsigned int w, h;
    unsigned char* bits;
} letter_t;

typedef struct color_t {
    int r, g, b, a;
    int mono_rgb;
    int gray_rgb;
} color_t;

typedef struct button_t {
    int x, y;
    int w, h;

    int lc;
    const char* label;
    short font_size;
    unsigned int lw, lh;
    unsigned char* lb;

    const char* letter;

    const char* left;
    short is_menu;
    const char* right;
    const char* sub;
} button_t;

/*************/
/* variables */
/*************/
extern letter_t small_font[ 128 ];
extern letter_t big_font[ 128 ];

extern color_t colors_48sx[ NB_COLORS ];
extern color_t colors_48gx[ NB_COLORS ];
extern color_t colors_49g[ NB_COLORS ];

extern button_t buttons_48sx[ NB_HP48_KEYS ];
extern button_t buttons_48gx[ NB_HP48_KEYS ];
extern button_t buttons_49g[ NB_HP49_KEYS ];

#define small_ascent 8
#define small_descent 4

/*************/
/* functions */
/*************/
extern int SmallTextWidth( const char* string, unsigned int length );
extern int BigTextWidth( const char* string, unsigned int length );

#endif /* _UI4x_INNER_H */
