#ifndef _UI4x_INNER_H
#define _UI4x_INNER_H 1

#include "emulator.h"

#include "bitmaps_misc.h"

// Colors
#define UI4X_COLOR_HP_LOGO 0
#define UI4X_COLOR_HP_LOGO_BG 1
#define UI4X_COLOR_48GX_128K_RAM 2

#define UI4X_COLOR_FRAME 3

#define UI4X_COLOR_UPPER_FACEPLATE_EDGE_TOP 4
#define UI4X_COLOR_UPPER_FACEPLATE 5
#define UI4X_COLOR_UPPER_FACEPLATE_EDGE_BOTTOM 6

#define UI4X_COLOR_FACEPLATE_EDGE_TOP 7
#define UI4X_COLOR_FACEPLATE 8
#define UI4X_COLOR_FACEPLATE_EDGE_BOTTOM 9

#define UI4X_COLOR_KEYPAD_HIGHLIGHT 10
#define UI4X_COLOR_BUTTON_EDGE_TOP 11
#define UI4X_COLOR_BUTTON 12
#define UI4X_COLOR_BUTTON_EDGE_BOTTOM 13

#define UI4X_COLOR_PIXEL_OFF 14
#define UI4X_COLOR_PIXEL_ON 15
#define UI4X_COLOR_BLACK_PIXEL_OFF 16
#define UI4X_COLOR_BLACK_PIXEL_ON 17

#define UI4X_COLOR_LABEL 18
#define UI4X_COLOR_ALPHA 19
#define UI4X_COLOR_SHIFT_LEFT 20
#define UI4X_COLOR_SHIFT_RIGHT 21

#define FIRST_COLOR 0
#define NB_COLORS 22

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

    bool highlight;

    /* label on the button (text or bitmap) */
    int label_color;
    const char* label_text;
    unsigned char* label_graphic;
    unsigned int label_graphic_w, label_graphic_h;

    /* labels around the button */
    const char* letter;
    const char* left;
    const char* right;
    const char* sub;

    /* unused */
    short font_size;
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

/*************/
/* functions */
/*************/
extern int SmallTextWidth( const char* string, unsigned int length );
extern int BigTextWidth( const char* string, unsigned int length );

#endif /* _UI4x_INNER_H */
