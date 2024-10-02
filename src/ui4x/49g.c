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
     /* #ffbaff */
        .name = "left",
     .r = 0x74,
     .g = 0x76,
     .b = 0xdd,
     .a = 255,
     .mono_rgb = 255,
     .gray_rgb = 220,
     },
    {
     /* #00ffcc */
        .name = "right",
     .r = 0,
     .g = 0xd4,
     .b = 0x62,
     .a = 0x62,
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
     /* #585858 */
        .name = "button",
     .r = 0x90,
     .g = 0x9b,
     .b = 0x94,
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
     /* #cadd5c */
        .name = "lcd_col",
     .r = 202,
     .g = 221,
     .b = 92,
     .a = 255,
     .mono_rgb = 255,
     .gray_rgb = 205,
     },
    {
     /* #000080 */
        .name = "pix_col",
     .r = 0,
     .g = 0,
     .b = 0x20,
     .a = 255,
     .mono_rgb = 0,
     .gray_rgb = 20,
     },
    {
     /* #585858 */
        .name = "pad_top",
     .r = 88,
     .g = 88,
     .b = 88,
     .a = 255,
     .mono_rgb = 0,
     .gray_rgb = 88,
     },
    {
     /* #4a4a4a */
        .name = "pad",
     .r = 0x86,
     .g = 0xa6,
     .b = 0xb9,
     .a = 255,
     .mono_rgb = 0,
     .gray_rgb = 74,
     },
    {
     /* #404040 */
        .name = "pad_bot",
     .r = 64,
     .g = 64,
     .b = 64,
     .a = 255,
     .mono_rgb = 0,
     .gray_rgb = 64,
     },
    {
     /* #808080 */
        .name = "disp_pad_top",
     .r = 128,
     .g = 128,
     .b = 138,
     .a = 255,
     .mono_rgb = 0,
     .gray_rgb = 128,
     },
    {
     /* #68686E */
        .name = "disp_pad",
     .r = 0x86,
     .g = 0xa6,
     .b = 0xb9,
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
     /* #68686e */
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

button_t buttons_49g[ NB_KEYS ] = {
    {.name = "A",
     .x = 0,
     .y = 0,
     .w = 36,
     .h = 23,
     .lc = WHITE,
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
    {.name = "B",
     .x = 50,
     .y = 0,
     .w = 36,
     .h = 23,
     .lc = WHITE,
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
    {.name = "C",
     .x = 100,
     .y = 0,
     .w = 36,
     .h = 23,
     .lc = WHITE,
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
    {.name = "D",
     .x = 150,
     .y = 0,
     .w = 36,
     .h = 23,
     .lc = WHITE,
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
    {.name = "E",
     .x = 200,
     .y = 0,
     .w = 36,
     .h = 23,
     .lc = WHITE,
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
    {.name = "F",
     .x = 250,
     .y = 0,
     .w = 36,
     .h = 23,
     .lc = WHITE,
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

    {.name = "MTH",
     .x = 0,
     .y = 50,
     .w = 36,
     .h = 26,
     .lc = WHITE,
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
    {.name = "PRG",
     .x = 50,
     .y = 50,
     .w = 36,
     .h = 26,
     .lc = WHITE,
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
    {.name = "CST",
     .x = 100,
     .y = 50,
     .w = 36,
     .h = 26,
     .lc = WHITE,
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
    {.name = "VAR",
     .x = 150,
     .y = 50,
     .w = 36,
     .h = 26,
     .lc = WHITE,
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
    {.name = "UP",
     .x = 200,
     .y = 50,
     .w = 36,
     .h = 26,
     .lc = WHITE,
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
    {.name = "NXT",
     .x = 250,
     .y = 50,
     .w = 36,
     .h = 26,
     .lc = WHITE,
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

    {.name = "QUOTE",
     .x = 0,
     .y = 100,
     .w = 36,
     .h = 26,
     .lc = WHITE,
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
    {.name = "STO",
     .x = 50,
     .y = 100,
     .w = 36,
     .h = 26,
     .lc = WHITE,
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
    {.name = "EVAL",
     .x = 100,
     .y = 100,
     .w = 36,
     .h = 26,
     .lc = WHITE,
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
    {.name = "LEFT",
     .x = 150,
     .y = 100,
     .w = 36,
     .h = 26,
     .lc = WHITE,
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
    {.name = "DOWN",
     .x = 200,
     .y = 100,
     .w = 36,
     .h = 26,
     .lc = WHITE,
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
    {.name = "RIGHT",
     .x = 250,
     .y = 100,
     .w = 36,
     .h = 26,
     .lc = WHITE,
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

    {.name = "SIN",
     .x = 0,
     .y = 150,
     .w = 36,
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
     .right = "\x07",
     .sub = 0       },
    {.name = "COS",
     .x = 50,
     .y = 150,
     .w = 36,
     .h = 26,
     .lc = WHITE,
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
    {.name = "TAN",
     .x = 100,
     .y = 150,
     .w = 36,
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
     .right = "\x09",
     .sub = 0       },
    {.name = "SQRT",
     .x = 150,
     .y = 150,
     .w = 36,
     .h = 26,
     .lc = WHITE,
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
    {.name = "POWER",
     .x = 200,
     .y = 150,
     .w = 36,
     .h = 26,
     .lc = WHITE,
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
    {.name = "INV",
     .x = 250,
     .y = 150,
     .w = 36,
     .h = 26,
     .lc = WHITE,
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

    {.name = "ENTER",
     .x = 0,
     .y = 200,
     .w = 86,
     .h = 26,
     .lc = WHITE,
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
    {.name = "NEG",
     .x = 100,
     .y = 200,
     .w = 36,
     .h = 26,
     .lc = WHITE,
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
    {.name = "EEX",
     .x = 150,
     .y = 200,
     .w = 36,
     .h = 26,
     .lc = WHITE,
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
    {.name = "DEL",
     .x = 200,
     .y = 200,
     .w = 36,
     .h = 26,
     .lc = WHITE,
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
    {.name = "BS",
     .x = 250,
     .y = 200,
     .w = 36,
     .h = 26,
     .lc = WHITE,
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

    {.name = "ALPHA",
     .x = 0,
     .y = 250,
     .w = 36,
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
     .y = 250,
     .w = 46,
     .h = 26,
     .lc = WHITE,
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
    {.name = "8",
     .x = 120,
     .y = 250,
     .w = 46,
     .h = 26,
     .lc = WHITE,
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
    {.name = "9",
     .x = 180,
     .y = 250,
     .w = 46,
     .h = 26,
     .lc = WHITE,
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
    {.name = "DIV",
     .x = 240,
     .y = 250,
     .w = 46,
     .h = 26,
     .lc = WHITE,
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

    {.name = "SHL",
     .x = 0,
     .y = 300,
     .w = 36,
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
     .y = 300,
     .w = 46,
     .h = 26,
     .lc = WHITE,
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
    {.name = "5",
     .x = 120,
     .y = 300,
     .w = 46,
     .h = 26,
     .lc = WHITE,
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
    {.name = "6",
     .x = 180,
     .y = 300,
     .w = 46,
     .h = 26,
     .lc = WHITE,
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
    {.name = "MUL",
     .x = 240,
     .y = 300,
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
     .right = "\x1a",
     .sub = 0       },

    {.name = "SHR",
     .x = 0,
     .y = 350,
     .w = 36,
     .h = 26,
     .lc = RIGHT,
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
    {.name = "1",
     .x = 60,
     .y = 350,
     .w = 46,
     .h = 26,
     .lc = WHITE,
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
    {.name = "2",
     .x = 120,
     .y = 350,
     .w = 46,
     .h = 26,
     .lc = WHITE,
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
    {.name = "3",
     .x = 180,
     .y = 350,
     .w = 46,
     .h = 26,
     .lc = WHITE,
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
    {.name = "MINUS",
     .x = 240,
     .y = 350,
     .w = 46,
     .h = 26,
     .lc = WHITE,
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

    {.name = "ON",
     .x = 0,
     .y = 400,
     .w = 36,
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
     .y = 400,
     .w = 46,
     .h = 26,
     .lc = WHITE,
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
    {.name = "PERIOD",
     .x = 120,
     .y = 400,
     .w = 46,
     .h = 26,
     .lc = WHITE,
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
    {.name = "SPC",
     .x = 180,
     .y = 400,
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
     .right = "\x1f",
     .sub = 0       },
    {.name = "PLUS",
     .x = 240,
     .y = 400,
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
     .right = "\x1e",
     .sub = 0       },
};
