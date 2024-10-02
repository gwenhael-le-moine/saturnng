#include "inner.h"

color_t colors_49g[ NB_COLORS ] = {
    {
     /* #FFFFFF */
        .name = "white",
     .r = 255,
     .g = 255,
     .b = 255,
     .a = 255,
     .mono_rgb = 255,
     .gray_rgb = 255,
     },
    {
     /* #00bfff */
        .name = "left",
     .r = 0x00,
     .g = 0xbf,
     .b = 0xff,
     .a = 255,
     .mono_rgb = 255,
     .gray_rgb = 220,
     },
    {
     /* #cd5c5c */
        .name = "right",
     .r = 0xcd,
     .g = 0x5c,
     .b = 0x5c,
     .a = 0xff,
     .mono_rgb = 255,
     .gray_rgb = 169,
     },
    {
     /* #646464 */
        .name = "but_top",
     .r = 104,
     .g = 104,
     .b = 104,
     .a = 255,
     .mono_rgb = 0,
     .gray_rgb = 104,
     },
    {
     /* #304055 */
        .name = "button",
     .r = 0x30,
     .g = 0x40,
     .b = 0x55,
     .a = 255,
     .mono_rgb = 0,
     .gray_rgb = 88,
     },
    {
     /* #4a4a4a */
        .name = "but_bot",
     .r = 74,
     .g = 74,
     .b = 74,
     .a = 255,
     .mono_rgb = 0,
     .gray_rgb = 74,
     },
    {
     /* #cccccc */
        .name = "lcd_col",
     .r = 0xcc,
     .g = 0xcc,
     .b = 0xcc,
     .a = 255,
     .mono_rgb = 255,
     .gray_rgb = 205,
     },
    {
     /* #000000 */
        .name = "pix_col",
     .r = 0x00,
     .g = 0x00,
     .b = 0x00,
     .a = 255,
     .mono_rgb = 0,
     .gray_rgb = 20,
     },
    {
     /* #888888 */
        .name = "pad_top",
     .r = 88,
     .g = 88,
     .b = 88,
     .a = 255,
     .mono_rgb = 0,
     .gray_rgb = 88,
     },
    {
     /* #304055 */
        .name = "pad",
     .r = 0x30,
     .g = 0x40,
     .b = 0x55,
     .a = 255,
     .mono_rgb = 0,
     .gray_rgb = 74,
     },
    {
     /* #646464 */
        .name = "pad_bot",
     .r = 64,
     .g = 64,
     .b = 64,
     .a = 255,
     .mono_rgb = 0,
     .gray_rgb = 64,
     },
    {
     /* #80808a */
        .name = "disp_pad_top",
     .r = 128,
     .g = 128,
     .b = 138,
     .a = 255,
     .mono_rgb = 0,
     .gray_rgb = 128,
     },
    {
     /* #304055 */
        .name = "disp_pad",
     .r = 0x30,
     .g = 0x40,
     .b = 0x55,
     .a = 255,
     .mono_rgb = 0,
     .gray_rgb = 104,
     },
    {
     /* #54545a */
        .name = "disp_pad_bot",
     .r = 84,
     .g = 84,
     .b = 90,
     .a = 255,
     .mono_rgb = 0,
     .gray_rgb = 84,
     },
    {
     /* #b0b0b8 */
        .name = "logo",
     .r = 176,
     .g = 176,
     .b = 184,
     .a = 255,
     .mono_rgb = 255,
     .gray_rgb = 176,
     },
    {
     /* #68686e */
        .name = "logo_back",
     .r = 104,
     .g = 104,
     .b = 110,
     .a = 255,
     .mono_rgb = 0,
     .gray_rgb = 104,
     },
    {
     /* #f0f0f0 */
        .name = "label",
     .r = 240,
     .g = 240,
     .b = 240,
     .a = 255,
     .mono_rgb = 255,
     .gray_rgb = 240,
     },
    {
     /* #000000 */
        .name = "frame",
     .r = 0,
     .g = 0,
     .b = 0,
     .a = 255,
     .mono_rgb = 255,
     .gray_rgb = 0,
     },
    {
     /* #4f6165 */
        .name = "underlay",
     .r = 0x4f,
     .g = 0x61,
     .b = 0x65,
     .a = 255,
     .mono_rgb = 0,
     .gray_rgb = 104,
     },
    {
     /* #000000 */
        .name = "black",
     .r = 0,
     .g = 0,
     .b = 0,
     .a = 255,
     .mono_rgb = 0,
     .gray_rgb = 0,
     },
};

#define KB_LINE_HEIGHT 45

button_t buttons_49g[ NB_HP49_KEYS ] = {
    {.name = "A",
     .x = 0,
     .y = 0,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = "F1",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "A",
     .left = "Y=",
     .is_menu = 0,
     .right = 0,
     .sub = 0},
    {.name = "B",
     .x = 50,
     .y = 0,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = "f2",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "B",
     .left = "WIN",
     .is_menu = 0,
     .right = 0,
     .sub = 0},
    {.name = "C",
     .x = 100,
     .y = 0,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = "F3",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "C",
     .left = "GRAPH",
     .is_menu = 0,
     .right = 0,
     .sub = 0},
    {.name = "D",
     .x = 150,
     .y = 0,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = "F4",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "D",
     .left = "2D/3D",
     .is_menu = 0,
     .right = 0,
     .sub = 0},
    {.name = "E",
     .x = 200,
     .y = 0,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = "F5",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "E",
     .left = "TBLSET",
     .is_menu = 0,
     .right = 0,
     .sub = 0},
    {.name = "F",
     .x = 250,
     .y = 0,
     .w = 36,
     .h = 26,
     .lc = WHITE,
     .label = "F6",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "F",
     .left = "TABLE",
     .is_menu = 0,
     .right = 0,
     .sub = 0},

    {.name = "APPS",
     .x = 0,
     .y = 1 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "APPS",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "G",
     .left = "FILES",
     .is_menu = 0,
     .right = "BEGIN",
     .sub = 0},
    {.name = "MODE",
     .x = 60,
     .y = 1 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "MODE",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "H",
     .left = "CUSTOM",
     .is_menu = 0,
     .right = "END",
     .sub = 0},
    {.name = "TOOL",
     .x = 120,
     .y = 1 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "TOOL",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "I",
     .left = "i",
     .is_menu = 0,
     .right = "I",
     .sub = 0},

    {.name = "VAR",
     .x = 0,
     .y = 2 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "VAR",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "J",
     .left = "UPDIR",
     .is_menu = 0,
     .right = "COPY",
     .sub = 0},
    {.name = "STO",
     .x = 60,
     .y = 2 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "STO",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "K",
     .left = "RCL",
     .is_menu = 0,
     .right = "CUT",
     .sub = 0},
    {.name = "NXT",
     .x = 120,
     .y = 2 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = quote_width,
     .lh = quote_height,
     .lb = quote_bitmap,
     .letter = "L",
     .left = "PREV",
     .is_menu = 0,
     .right = "PASTE",
     .sub = 0},

    {.name = "LEFT",
     .x = 180,
     .y = 1.4 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = left_width,
     .lh = left_height,
     .lb = left_bitmap,
     .letter = 0,
     .left = 0,
     .is_menu = 0,
     .right = 0,
     .sub = 0},
    {.name = "UP",
     .x = 210,
     .y = 0.75 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = up_width,
     .lh = up_height,
     .lb = up_bitmap,
     .letter = 0,
     .left = 0,
     .is_menu = 0,
     .right = 0,
     .sub = 0},
    {.name = "RIGHT",
     .x = 240,
     .y = 1.4 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = right_width,
     .lh = right_height,
     .lb = right_bitmap,
     .letter = 0,
     .left = 0,
     .is_menu = 0,
     .right = 0,
     .sub = 0},
    {.name = "DOWN",
     .x = 210,
     .y = 2 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = down_width,
     .lh = down_height,
     .lb = down_bitmap,
     .letter = 0,
     .left = 0,
     .is_menu = 0,
     .right = 0,
     .sub = 0},

    {.name = "HIST",
     .x = 0,
     .y = 3 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "HIST",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "M",
     .left = "CMD",
     .is_menu = 0,
     .right = "UNDO",
     .sub = 0},
    {.name = "CAT",
     .x = 60,
     .y = 3 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "CAT",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "N",
     .left = "PRG",
     .is_menu = 0,
     .right = "CHARS",
     .sub = 0},
    {.name = "EQW",
     .x = 120,
     .y = 3 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "EQW",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "O",
     .left = "MTRW",
     .is_menu = 0,
     .right = "'",
     .sub = 0},
    {.name = "SYMB",
     .x = 180,
     .y = 3 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "SYMB",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "P",
     .left = "MTH",
     .is_menu = 0,
     .right = "EVAL",
     .sub = 0},
    {.name = "BS",
     .x = 240,
     .y = 3 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "<=",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "DEL",
     .is_menu = 0,
     .right = "CLEAR",
     .sub = 0},

    {.name = "POW",
     .x = 0,
     .y = 4 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = power_width,
     .lh = power_height,
     .lb = power_bitmap,
     .letter = "Q",
     .left = "\x16",
     .is_menu = 0,
     .right = "LN",
     .sub = 0},
    {.name = "SQRT",
     .x = 60,
     .y = 4 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = sqrt_width,
     .lh = sqrt_height,
     .lb = sqrt_bitmap,
     .letter = "R",
     .left = "\x13",
     .is_menu = 0,
     .right = "\x14",
     .sub = 0},
    {.name = "SIN",
     .x = 120,
     .y = 4 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "SIN",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "S",
     .left = "ASIN",
     .is_menu = 0,
     .right = "\x09",
     .sub = 0},
    {.name = "COS",
     .x = 180,
     .y = 4 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "COS",
     .font_size = 2,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "T",
     .left = "ACOS",
     .is_menu = 0,
     .right = "\x07",
     .sub = 0},
    {.name = "TAN",
     .x = 240,
     .y = 4 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "TAN",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "U",
     .left = "ATAN",
     .is_menu = 0,
     .right = "\x08",
     .sub = 0},

    {.name = "EEX",
     .x = 0,
     .y = 5 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "EEX",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "V",
     .left = "\x15",
     .is_menu = 0,
     .right = "LOG",
     .sub = 0},
    {.name = "INV",
     .x = 60,
     .y = 5 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = neg_width,
     .lh = neg_height,
     .lb = neg_bitmap,
     .letter = "W",
     .left = "/=",
     .is_menu = 0,
     .right = "=",
     .sub = 0},
    {.name = "X",
     .x = 120,
     .y = 5 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "X",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = "X",
     .left = "<=",
     .is_menu = 0,
     .right = "<",
     .sub = 0},
    {.name = "INV",
     .x = 180,
     .y = 5 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = inv_width,
     .lh = inv_height,
     .lb = inv_bitmap,
     .letter = "Y",
     .left = ">=",
     .is_menu = 0,
     .right = ">",
     .sub = 0},
    {.name = "DIV",
     .x = 240,
     .y = 5 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = div_width,
     .lh = div_height,
     .lb = div_bitmap,
     .letter = "Z",
     .left = "ABS ",
     .is_menu = 0,
     .right = "ARG",
     .sub = 0},

    {.name = "ALPHA",
     .x = 0,
     .y = 6 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
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
    {.name = "7",
     .x = 60,
     .y = 6 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "7",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "S.SLV",
     .is_menu = 0,
     .right = "NUM.SLV",
     .sub = 0       },
    {.name = "8",
     .x = 120,
     .y = 6 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "8",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "EXP&LN",
     .is_menu = 0,
     .right = "TRIG",
     .sub = 0       },
    {.name = "9",
     .x = 180,
     .y = 6 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "9",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "FINANCE",
     .is_menu = 0,
     .right = "TIME",
     .sub = 0       },
    {.name = "MUL",
     .x = 240,
     .y = 6 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = mul_width,
     .lh = mul_height,
     .lb = mul_bitmap,
     .letter = 0,
     .left = "\x19 ",
     .is_menu = 0,
     .right = "\x1c",
     .sub = 0       },

    {.name = "SHL",
     .x = 0,
     .y = 7 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = LEFT,
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
    {.name = "4",
     .x = 60,
     .y = 7 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "4",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "CALC",
     .is_menu = 0,
     .right = "ALG",
     .sub = 0       },
    {.name = "5",
     .x = 120,
     .y = 7 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "5",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "MATRICES",
     .is_menu = 0,
     .right = "STAT",
     .sub = 0       },
    {.name = "6",
     .x = 180,
     .y = 7 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "6",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "CONVERT",
     .is_menu = 0,
     .right = "UNITS",
     .sub = 0       },
    {.name = "MINUS",
     .x = 240,
     .y = 7 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = minus_width,
     .lh = minus_height,
     .lb = minus_bitmap,
     .letter = 0,
     .left = "\x17 ",
     .is_menu = 0,
     .right = "\x1a",
     .sub = 0       },

    {.name = "SHR",
     .x = 0,
     .y = 8 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = RIGHT,
     .label = 0,
     .font_size = 0,
     .lw = shr_width,
     .lh = shr_height,
     .lb = shr_bitmap,
     .letter = 0,
     .left = " ",
     .is_menu = 0,
     .right = 0,
     .sub = 0       },
    {.name = "1",
     .x = 60,
     .y = 8 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "1",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "ARITH",
     .is_menu = 0,
     .right = "CMPLX",
     .sub = 0       },
    {.name = "2",
     .x = 120,
     .y = 8 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "2",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "DEF",
     .is_menu = 0,
     .right = "LIB",
     .sub = 0       },
    {.name = "3",
     .x = 180,
     .y = 8 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "3",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "\x02",
     .is_menu = 0,
     .right = "BASE",
     .sub = 0       },
    {.name = "PLUS",
     .x = 240,
     .y = 8 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = 0,
     .font_size = 0,
     .lw = plus_width,
     .lh = plus_height,
     .lb = plus_bitmap,
     .letter = 0,
     .left = "\x1d ",
     .is_menu = 0,
     .right = "\x1b",
     .sub = 0       },

    {.name = "ON",
     .x = 0,
     .y = 9 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
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
    {.name = "0",
     .x = 60,
     .y = 9 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "0",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "INF ",
     .is_menu = 0,
     .right = "\x03",
     .sub = 0       },
    {.name = "PERIOD",
     .x = 120,
     .y = 9 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = ".",
     .font_size = 1,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = ":: ",
     .is_menu = 0,
     .right = "\x01",
     .sub = 0       },
    {.name = "SPC",
     .x = 180,
     .y = 9 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "SPC",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "\x05 ",
     .is_menu = 0,
     .right = "\x02",
     .sub = 0       },
    {.name = "ENTER",
     .x = 240,
     .y = 9 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .lc = WHITE,
     .label = "ENTER",
     .font_size = 0,
     .lw = 0,
     .lh = 0,
     .lb = 0,
     .letter = 0,
     .left = "ANS ",
     .is_menu = 0,
     .right = "\x06NUM",
     .sub = 0       },
};
