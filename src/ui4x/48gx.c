#include "inner.h"

color_t colors_48gx[ NB_COLORS ] = {
    /* UI4X_COLOR_WHITE */
    {
     .r = 0xFF,
     .g = 0xFF,
     .b = 0xFF,
     .a = 0xFF,
     .mono_rgb = 0xFF,
     .gray_rgb = 0xFF,
     },
    /* UI4X_COLOR_SHIFT_LEFT */
    {
     .r = 0xFF,
     .g = 0xBA,
     .b = 0xFF,
     .a = 0xFF,
     .mono_rgb = 0xFF,
     .gray_rgb = 0xDC,
     },
    /* UI4X_COLOR_SHIFT_RIGHT */
    {
     .r = 0x0,
     .g = 0xFF,
     .b = 0xCC,
     .a = 0xFF,
     .mono_rgb = 0xFF,
     .gray_rgb = 0xA9,
     },
    /* UI4X_COLOR_BUTTON_EDGE_TOP */
    {
     .r = 0x68,
     .g = 0x68,
     .b = 0x68,
     .a = 0xFF,
     .mono_rgb = 0x0,
     .gray_rgb = 0x68,
     },
    /* UI4X_COLOR_BUTTON */
    {
     .r = 0x58,
     .g = 0x58,
     .b = 0x58,
     .a = 0xFF,
     .mono_rgb = 0x0,
     .gray_rgb = 0x58,
     },
    /* UI4X_COLOR_BUTTON_EDGE_BOTTOM */
    {
     .r = 0x4A,
     .g = 0x4A,
     .b = 0x4A,
     .a = 0xFF,
     .mono_rgb = 0x0,
     .gray_rgb = 0x4A,
     },
    /* UI4X_COLOR_LCD_BG */
    {
     .r = 0xCA,
     .g = 0xDD,
     .b = 0x5C,
     .a = 0xFF,
     .mono_rgb = 0xFF,
     .gray_rgb = 0xCD,
     },
    /* UI4X_COLOR_LCD_PIXEL */
    {
     .r = 0x0,
     .g = 0x0,
     .b = 0x80,
     .a = 0xFF,
     .mono_rgb = 0x0,
     .gray_rgb = 0x14,
     },
    /* UI4X_COLOR_FACEPLATE_EDGE_TOP */
    {
     .r = 0x58,
     .g = 0x58,
     .b = 0x58,
     .a = 0xFF,
     .mono_rgb = 0x0,
     .gray_rgb = 0x58,
     },
    /* UI4X_COLOR_FACEPLATE */
    {
     .r = 0x4A,
     .g = 0x4A,
     .b = 0x4A,
     .a = 0xFF,
     .mono_rgb = 0x0,
     .gray_rgb = 0x4A,
     },
    /* UI4X_COLOR_FACEPLATE_EDGE_BOTTOM */
    {
     .r = 0x40,
     .g = 0x40,
     .b = 0x40,
     .a = 0xFF,
     .mono_rgb = 0x0,
     .gray_rgb = 0x40,
     },
    /* UI4X_COLOR_UPPER_FACEPLATE_EDGE_TOP */
    {
     .r = 0x80,
     .g = 0x80,
     .b = 0x8A,
     .a = 0xFF,
     .mono_rgb = 0x0,
     .gray_rgb = 0x80,
     },
    /* UI4X_COLOR_UPPER_FACEPLATE */
    {
     .r = 0x68,
     .g = 0x68,
     .b = 0x6E,
     .a = 0xFF,
     .mono_rgb = 0x0,
     .gray_rgb = 0x68,
     },
    /* UI4X_COLOR_UPPER_FACEPLATE_EDGE_BOTTOM */
    {
     .r = 0x54,
     .g = 0x54,
     .b = 0x5A,
     .a = 0xFF,
     .mono_rgb = 0x0,
     .gray_rgb = 0x54,
     },
    /* UI4X_COLOR_HP_LOGO */
    {
     .r = 0xB0,
     .g = 0xB0,
     .b = 0xB8,
     .a = 0xFF,
     .mono_rgb = 0xFF,
     .gray_rgb = 0xB0,
     },
    /* UI4X_COLOR_HP_LOGO_BG */
    {
     .r = 0x68,
     .g = 0x68,
     .b = 0x6E,
     .a = 0xFF,
     .mono_rgb = 0x0,
     .gray_rgb = 0x68,
     },
    /* UI4X_COLOR_48GX_128K_RAM */
    {
     .r = 0xF0,
     .g = 0xF0,
     .b = 0xF0,
     .a = 0xFF,
     .mono_rgb = 0xFF,
     .gray_rgb = 0xF0,
     },
    /* UI4X_COLOR_FRAME */
    {
     .r = 0x0,
     .g = 0x0,
     .b = 0x0,
     .a = 0xFF,
     .mono_rgb = 0xFF,
     .gray_rgb = 0x0,
     },
    /* UI4X_COLOR_KEYPAD_HIGHLIGHT */
    {
     .r = 0x68,
     .g = 0x68,
     .b = 0x6E,
     .a = 0xFF,
     .mono_rgb = 0x0,
     .gray_rgb = 0x68,
     },
};

button_t buttons_48gx[ NB_HP48_KEYS ] = {
    {.x = 0,
     .y = 0,
     .w = 36,
     .h = 23,
     .lc = UI4X_COLOR_WHITE,
     .label = 0,
     .font_size = 0,
     .lw = menu_label_width,
     .lh = menu_label_height,
     .lb = menu_label_bitmap,
     .letter = "A",
     .left = 0,
     .is_menu = 0,
     .right = 0,
     .sub = 0       },
    {.x = 50,
     .y = 0,
     .w = 36,
     .h = 23,
     .lc = UI4X_COLOR_WHITE,
     .label = 0,
     .font_size = 0,
     .lw = menu_label_width,
     .lh = menu_label_height,
     .lb = menu_label_bitmap,
     .letter = "B",
     .left = 0,
     .is_menu = 0,
     .right = 0,
     .sub = 0       },
    {.x = 100,
     .y = 0,
     .w = 36,
     .h = 23,
     .lc = UI4X_COLOR_WHITE,
     .label = 0,
     .font_size = 0,
     .lw = menu_label_width,
     .lh = menu_label_height,
     .lb = menu_label_bitmap,
     .letter = "C",
     .left = 0,
     .is_menu = 0,
     .right = 0,
     .sub = 0       },
    {.x = 150,
     .y = 0,
     .w = 36,
     .h = 23,
     .lc = UI4X_COLOR_WHITE,
     .label = 0,
     .font_size = 0,
     .lw = menu_label_width,
     .lh = menu_label_height,
     .lb = menu_label_bitmap,
     .letter = "D",
     .left = 0,
     .is_menu = 0,
     .right = 0,
     .sub = 0       },
    {.x = 200,
     .y = 0,
     .w = 36,
     .h = 23,
     .lc = UI4X_COLOR_WHITE,
     .label = 0,
     .font_size = 0,
     .lw = menu_label_width,
     .lh = menu_label_height,
     .lb = menu_label_bitmap,
     .letter = "E",
     .left = 0,
     .is_menu = 0,
     .right = 0,
     .sub = 0       },
    {.x = 250,
     .y = 0,
     .w = 36,
     .h = 23,
     .lc = UI4X_COLOR_WHITE,
     .label = 0,
     .font_size = 0,
     .lw = menu_label_width,
     .lh = menu_label_height,
     .lb = menu_label_bitmap,
     .letter = "F",
     .left = 0,
     .is_menu = 0,
     .right = 0,
     .sub = 0       },

    {.x = 0,
     .y = 50,
     .w = 36,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = "MTH",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "G",
     .left = "RAD",
     .is_menu = 0,
     .right = "POLAR",
     .sub = 0       },
    {.x = 50,
     .y = 50,
     .w = 36,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = "PRG",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "H",
     .left = 0,
     .is_menu = 0,
     .right = "CHARS",
     .sub = 0       },
    {.x = 100,
     .y = 50,
     .w = 36,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = "CST",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "I",
     .left = 0,
     .is_menu = 0,
     .right = "MODES",
     .sub = 0       },
    {.x = 150,
     .y = 50,
     .w = 36,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = "VAR",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "J",
     .left = 0,
     .is_menu = 0,
     .right = "MEMORY",
     .sub = 0       },
    {.x = 200,
     .y = 50,
     .w = 36,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = 0,
     .font_size = 0,
     .lw = up_width,
     .lh = up_height,
     .lb = up_bitmap,
     .letter = "K",
     .left = 0,
     .is_menu = 0,
     .right = "STACK",
     .sub = 0       },
    {.x = 250,
     .y = 50,
     .w = 36,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = "NXT",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "L",
     .left = "PREV",
     .is_menu = 0,
     .right = "MENU",
     .sub = 0       },

    {.x = 0,
     .y = 100,
     .w = 36,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = 0,
     .font_size = 0,
     .lw = quote_width,
     .lh = quote_height,
     .lb = quote_bitmap,
     .letter = "M",
     .left = "UP",
     .is_menu = 0,
     .right = "HOME",
     .sub = 0       },
    {.x = 50,
     .y = 100,
     .w = 36,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = "STO",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "N",
     .left = "DEF",
     .is_menu = 0,
     .right = "RCL",
     .sub = 0       },
    {.x = 100,
     .y = 100,
     .w = 36,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = "EVAL",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "O",
     .left = "\x06NUM",
     .is_menu = 0,
     .right = "UNDO",
     .sub = 0       },
    {.x = 150,
     .y = 100,
     .w = 36,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = 0,
     .font_size = 0,
     .lw = left_width,
     .lh = left_height,
     .lb = left_bitmap,
     .letter = "P",
     .left = "PICTURE",
     .is_menu = 0,
     .right = 0,
     .sub = 0       },
    {.x = 200,
     .y = 100,
     .w = 36,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = 0,
     .font_size = 0,
     .lw = down_width,
     .lh = down_height,
     .lb = down_bitmap,
     .letter = "Q",
     .left = "VIEW",
     .is_menu = 0,
     .right = 0,
     .sub = 0       },
    {.x = 250,
     .y = 100,
     .w = 36,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = 0,
     .font_size = 0,
     .lw = right_width,
     .lh = right_height,
     .lb = right_bitmap,
     .letter = "R",
     .left = "SWAP",
     .is_menu = 0,
     .right = 0,
     .sub = 0       },

    {.x = 0,
     .y = 150,
     .w = 36,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = "SIN",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "S",
     .left = "ASIN",
     .is_menu = 0,
     .right = "\x07",
     .sub = 0       },
    {.x = 50,
     .y = 150,
     .w = 36,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = "COS",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "T",
     .left = "ACOS",
     .is_menu = 0,
     .right = "\x08",
     .sub = 0       },
    {.x = 100,
     .y = 150,
     .w = 36,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = "TAN",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "U",
     .left = "ATAN",
     .is_menu = 0,
     .right = "\x09",
     .sub = 0       },
    {.x = 150,
     .y = 150,
     .w = 36,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = 0,
     .font_size = 0,
     .lw = sqrt_width,
     .lh = sqrt_height,
     .lb = sqrt_bitmap,
     .letter = "V",
     .left = "\x13",
     .is_menu = 0,
     .right = "\x14",
     .sub = 0       },
    {.x = 200,
     .y = 150,
     .w = 36,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = 0,
     .font_size = 0,
     .lw = power_width,
     .lh = power_height,
     .lb = power_bitmap,
     .letter = "W",
     .left = "\x15",
     .is_menu = 0,
     .right = "LOG",
     .sub = 0       },
    {.x = 250,
     .y = 150,
     .w = 36,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = 0,
     .font_size = 0,
     .lw = inv_width,
     .lh = inv_height,
     .lb = inv_bitmap,
     .letter = "X",
     .left = "\x16",
     .is_menu = 0,
     .right = "LN",
     .sub = 0       },

    {.x = 0,
     .y = 200,
     .w = 86,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = "ENTER",
     .font_size = 2,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "EQUATION",
     .is_menu = 0,
     .right = "MATRIX",
     .sub = 0       },
    {.x = 100,
     .y = 200,
     .w = 36,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = 0,
     .font_size = 0,
     .lw = neg_width,
     .lh = neg_height,
     .lb = neg_bitmap,
     .letter = "Y",
     .left = "EDIT",
     .is_menu = 0,
     .right = "CMD",
     .sub = 0       },
    {.x = 150,
     .y = 200,
     .w = 36,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = "EEX",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "Z",
     .left = "PURG",
     .is_menu = 0,
     .right = "ARG",
     .sub = 0       },
    {.x = 200,
     .y = 200,
     .w = 36,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = "DEL",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "CLEAR",
     .is_menu = 0,
     .right = 0,
     .sub = 0       },
    {.x = 250,
     .y = 200,
     .w = 36,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = 0,
     .font_size = 0,
     .lw = bs_width,
     .lh = bs_height,
     .lb = bs_bitmap,
     .letter = 0,
     .left = "DROP",
     .is_menu = 0,
     .right = 0,
     .sub = 0       },

    {.x = 0,
     .y = 250,
     .w = 36,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = 0,
     .font_size = 0,
     .lw = alpha_width,
     .lh = alpha_height,
     .lb = alpha_bitmap,
     .letter = 0,
     .left = "USER",
     .is_menu = 0,
     .right = "ENTRY",
     .sub = 0       },
    {.x = 60,
     .y = 250,
     .w = 46,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = "7",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "SOLVE",
     .is_menu = 1,
     .right = 0,
     .sub = 0       },
    {.x = 120,
     .y = 250,
     .w = 46,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = "8",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "PLOT",
     .is_menu = 1,
     .right = 0,
     .sub = 0       },
    {.x = 180,
     .y = 250,
     .w = 46,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = "9",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "SYMBOLIC",
     .is_menu = 1,
     .right = 0,
     .sub = 0       },
    {.x = 240,
     .y = 250,
     .w = 46,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = 0,
     .font_size = 0,
     .lw = div_width,
     .lh = div_height,
     .lb = div_bitmap,
     .letter = 0,
     .left = "\x17 ",
     .is_menu = 0,
     .right = "\x18",
     .sub = 0       },

    {.x = 0,
     .y = 300,
     .w = 36,
     .h = 26,
     .lc = UI4X_COLOR_SHIFT_LEFT,
     .label = 0,
     .font_size = 0,
     .lw = shl_width,
     .lh = shl_height,
     .lb = shl_bitmap,
     .letter = 0,
     .left = 0,
     .is_menu = 0,
     .right = 0,
     .sub = 0       },
    {.x = 60,
     .y = 300,
     .w = 46,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = "4",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "TIME",
     .is_menu = 1,
     .right = 0,
     .sub = 0       },
    {.x = 120,
     .y = 300,
     .w = 46,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = "5",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "STAT",
     .is_menu = 1,
     .right = 0,
     .sub = 0       },
    {.x = 180,
     .y = 300,
     .w = 46,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = "6",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "UNITS",
     .is_menu = 1,
     .right = 0,
     .sub = 0       },
    {.x = 240,
     .y = 300,
     .w = 46,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = 0,
     .font_size = 0,
     .lw = mul_width,
     .lh = mul_height,
     .lb = mul_bitmap,
     .letter = 0,
     .left = "\x19 ",
     .is_menu = 0,
     .right = "\x1a",
     .sub = 0       },

    {.x = 0,
     .y = 350,
     .w = 36,
     .h = 26,
     .lc = UI4X_COLOR_SHIFT_RIGHT,
     .label = 0,
     .font_size = 0,
     .lw = shr_width,
     .lh = shr_height,
     .lb = shr_bitmap,
     .letter = 0,
     .left = " ",
     .is_menu = 1,
     .right = 0,
     .sub = 0       },
    {.x = 60,
     .y = 350,
     .w = 46,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = "1",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "I/O",
     .is_menu = 1,
     .right = 0,
     .sub = 0       },
    {.x = 120,
     .y = 350,
     .w = 46,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = "2",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "LIBRARY",
     .is_menu = 1,
     .right = 0,
     .sub = 0       },
    {.x = 180,
     .y = 350,
     .w = 46,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = "3",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "EQ LIB",
     .is_menu = 1,
     .right = 0,
     .sub = 0       },
    {.x = 240,
     .y = 350,
     .w = 46,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = 0,
     .font_size = 0,
     .lw = minus_width,
     .lh = minus_height,
     .lb = minus_bitmap,
     .letter = 0,
     .left = "\x1b ",
     .is_menu = 0,
     .right = "\x1c",
     .sub = 0       },

    {.x = 0,
     .y = 400,
     .w = 36,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = "ON",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "CONT",
     .is_menu = 0,
     .right = "OFF",
     .sub = "CANCEL"},
    {.x = 60,
     .y = 400,
     .w = 46,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = "0",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "\x04 ",
     .is_menu = 0,
     .right = "\x03",
     .sub = 0       },
    {.x = 120,
     .y = 400,
     .w = 46,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = ".",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "\x02 ",
     .is_menu = 0,
     .right = "\x01",
     .sub = 0       },
    {.x = 180,
     .y = 400,
     .w = 46,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = "SPC",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "\x05 ",
     .is_menu = 0,
     .right = "\x1f",
     .sub = 0       },
    {.x = 240,
     .y = 400,
     .w = 46,
     .h = 26,
     .lc = UI4X_COLOR_WHITE,
     .label = 0,
     .font_size = 0,
     .lw = plus_width,
     .lh = plus_height,
     .lb = plus_bitmap,
     .letter = 0,
     .left = "\x1d ",
     .is_menu = 0,
     .right = "\x1e",
     .sub = 0       },
};
