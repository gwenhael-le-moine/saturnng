#ifndef _UI4x_INNER_H
#define _UI4x_INNER_H 1

#include "emulator.h"

#include "bitmaps_misc.h"
#include "bitmaps_fonts.h"

// Colors
/*                          48SX    48GX     49g    */
#define WHITE 0         /* #ffffff #ffffff #ffffff */
#define LEFT 1          /* #ffa600 #ffbaff #7476dd  */
#define RIGHT 2         /* #00d2ff #00ffcc #d46262  */
#define BUT_TOP 3       /* #6d5d5d #646464 #646464 */
#define BUTTON 4        /* #5a4d4d #585858 #909b94  */
#define BUT_BOT 5       /* #4c4141 #4a4a4a #4a4a4a */
#define LCD 6           /* #cadd5c #cadd5c #a8c0b0  */
#define PIXEL 7         /* #000080 #000080 #000020  */
#define PAD_TOP 8       /* #6d4e4e #585858 #585858 */
#define PAD 9           /* #5a4040 #4a4a4a #4a4a4a */
#define PAD_BOT 10      /* #4c3636 #404040 #404040 */
#define DISP_PAD_TOP 11 /* #9b7654 #808080 #808080 */
#define DISP_PAD 12     /* #7c5e43 #68686e #86a6b9  */
#define DISP_PAD_BOT 13 /* #644b35 #54545a #97b7ca */
#define LOGO 14         /* #cca96b #b0b0b8 #b0b0b8 */
#define LOGO_BACK 15    /* #404040 #68686e #68686e */
#define LABEL 16        /* #cab890 #f0f0f0 #f0f0f0 */
#define FRAME 17        /* #000000 #000000 #000000 */
#define UNDERLAY 18     /* #3c2a2a #68686e #4f6165  */
#define BLACK 19        /* #000000 #000000 #000000 */

#define FIRST_COLOR WHITE
#define LAST_COLOR BLACK
#define NB_COLORS ( LAST_COLOR + 1 )

/***********/
/* typedef */
/***********/
typedef struct letter_t {
    unsigned int w, h;
    unsigned char* bits;
} letter_t;

typedef struct color_t {
    const char* name;
    int r, g, b, a;
    int mono_rgb;
    int gray_rgb;
} color_t;

typedef struct button_t {
    const char* name;

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

extern button_t buttons_48sx[ NB_KEYS ];
extern button_t buttons_48gx[ NB_KEYS ];
extern button_t buttons_49g[ NB_KEYS ];

#define small_ascent 8
#define small_descent 4

/*************/
/* functions */
/*************/
extern int SmallTextWidth( const char* string, unsigned int length );
extern int BigTextWidth( const char* string, unsigned int length );

#endif /* _UI4x_INNER_H */
