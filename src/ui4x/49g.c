#include "inner.h"

color_t colors_49g[ NB_COLORS ] = {
    /* UI4X_COLOR_HP_LOGO */
    {
     .r = 0xB0,
     .g = 0xB0,
     .b = 0xB8,
     .a = 0xff,
     .mono_rgb = 0xff,
     .gray_rgb = 0xB0,
     },
    /* UI4X_COLOR_HP_LOGO_BG */
    {
     .r = 0x68,
     .g = 0x68,
     .b = 0x6E,
     .a = 0xff,
     .mono_rgb = 0x00,
     .gray_rgb = 0x68,
     },
    /* UI4X_COLOR_48GX_128K_RAM */
    {
     .r = 0xF0,
     .g = 0xF0,
     .b = 0xF0,
     .a = 0xff,
     .mono_rgb = 0xff,
     .gray_rgb = 0xF0,
     },
    /* UI4X_COLOR_FRAME */
    {
     .r = 0x00,
     .g = 0x00,
     .b = 0x00,
     .a = 0xff,
     .mono_rgb = 0xff,
     .gray_rgb = 0x00,
     },
    /* UI4X_COLOR_UPPER_FACEPLATE_EDGE_TOP */
    {
     .r = 0x00,
     .g = 0x00,
     .b = 0x00,
     .a = 0xff,
     .mono_rgb = 0x00,
     .gray_rgb = 0x80,
     },
    /* UI4X_COLOR_UPPER_FACEPLATE */
    {
     .r = 0x26,
     .g = 0x26,
     .b = 0x26,
     .a = 0xff,
     .mono_rgb = 0x00,
     .gray_rgb = 0x68,
     },
    /* UI4X_COLOR_UPPER_FACEPLATE_EDGE_BOTTOM */
    {
     .r = 0x00,
     .g = 0x00,
     .b = 0x00,
     .a = 0xff,
     .mono_rgb = 0x00,
     .gray_rgb = 0x54,
     },
    /* UI4X_COLOR_FACEPLATE_EDGE_TOP */
    {
     .r = 0x58,
     .g = 0x58,
     .b = 0x58,
     .a = 0xff,
     .mono_rgb = 0x00,
     .gray_rgb = 0x58,
     },
    /* UI4X_COLOR_FACEPLATE */
    {
     .r = 0x27,
     .g = 0x27,
     .b = 0x27,
     .a = 0xff,
     .mono_rgb = 0x00,
     .gray_rgb = 0x4A,
     },
    /* UI4X_COLOR_FACEPLATE_EDGE_BOTTOM */
    {
     .r = 0x40,
     .g = 0x40,
     .b = 0x40,
     .a = 0xff,
     .mono_rgb = 0x00,
     .gray_rgb = 0x40,
     },
    /* UI4X_COLOR_KEYPAD_HIGHLIGHT */
    {
     .r = 0x4f,
     .g = 0x61,
     .b = 0x65,
     .a = 0xff,
     .mono_rgb = 0x00,
     .gray_rgb = 0x68,
     },
    /* UI4X_COLOR_BUTTON_EDGE_TOP */
    {
     .r = 0x4b,
     .g = 0x4b,
     .b = 0x4b,
     .a = 0xff,
     .mono_rgb = 0x00,
     .gray_rgb = 0x68,
     },
    /* UI4X_COLOR_BUTTON */
    {
     .r = 0x3f,
     .g = 0x3f,
     .b = 0x3f,
     .a = 0xff,
     .mono_rgb = 0x00,
     .gray_rgb = 0x58,
     },
    /* UI4X_COLOR_BUTTON_EDGE_BOTTOM */
    {
     .r = 0x3d,
     .g = 0x3d,
     .b = 0x3d,
     .a = 0xff,
     .mono_rgb = 0x00,
     .gray_rgb = 0x4A,
     },
    /* UI4X_COLOR_PIXEL_OFF */
    {
     .r = 0xcd,
     .g = 0xcd,
     .b = 0xcd,
     .a = 0xff,
     .mono_rgb = 0xff,
     .gray_rgb = 0xCD,
     },
    /* UI4X_COLOR_PIXEL_ON */
    {
     .r = 0x00,
     .g = 0x00,
     .b = 0x00,
     .a = 0xff,
     .mono_rgb = 0x00,
     .gray_rgb = 0x14,
     },
    /* UI4X_COLOR_BLACK_PIXEL_OFF */
    {
     .r = 0xf5,
     .g = 0xf5,
     .b = 0xf5,
     .a = 0xFF,
     .mono_rgb = 0xFF,
     .gray_rgb = 0xCD,
     },
    /* UI4X_COLOR_BLACK_PIXEL_ON */
    {
     .r = 0x0,
     .g = 0x0,
     .b = 0x0,
     .a = 0xFF,
     .mono_rgb = 0x0,
     .gray_rgb = 0x14,
     },
    /* UI4X_COLOR_LABEL */
    {
     .r = 0xff,
     .g = 0xff,
     .b = 0xff,
     .a = 0xff,
     .mono_rgb = 0xff,
     .gray_rgb = 0xff,
     },
    /* UI4X_COLOR_ALPHA */
    {
     .r = 0x67,
     .g = 0xc1,
     .b = 0x84,
     .a = 0xff,
     .mono_rgb = 0xff,
     .gray_rgb = 0xDC,
     },
    /* UI4X_COLOR_SHIFT_LEFT */
    {
     .r = 0x82,
     .g = 0xb1,
     .b = 0xff,
     .a = 0xff,
     .mono_rgb = 0xff,
     .gray_rgb = 0xDC,
     },
    /* UI4X_COLOR_SHIFT_RIGHT */
    {
     .r = 0xff,
     .g = 0x8a,
     .b = 0x80,
     .a = 0xff,
     .mono_rgb = 0xff,
     .gray_rgb = 0xA9,
     },
};

#define KB_LINE_HEIGHT 46
#define KB_COLUMN_WIDTH_6_KEYS 50
#define KB_COLUMN_WIDTH_5_KEYS 60

button_t buttons_49g[ NB_HP49_KEYS ] = {
    {.x = 0,
     .y = 0,
     .w = 36,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = "F1",
     .font_size = 0,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = "A",
     .left = "Y=",
     .highlight = false,
     .right = 0,
     .sub = 0                       },
    {.x = KB_COLUMN_WIDTH_6_KEYS,
     .y = 0,
     .w = 36,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = "F2",
     .font_size = 0,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = "B",
     .left = "WIN",
     .highlight = false,
     .right = 0,
     .sub = 0                       },
    {.x = 2 * KB_COLUMN_WIDTH_6_KEYS,
     .y = 0,
     .w = 36,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = "F3",
     .font_size = 0,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = "C",
     .left = "GRAPH",
     .highlight = false,
     .right = 0,
     .sub = 0                       },
    {.x = 3 * KB_COLUMN_WIDTH_6_KEYS,
     .y = 0,
     .w = 36,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = "F4",
     .font_size = 0,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = "D",
     .left = "2D/3D",
     .highlight = false,
     .right = 0,
     .sub = 0                       },
    {.x = 4 * KB_COLUMN_WIDTH_6_KEYS,
     .y = 0,
     .w = 36,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = "F5",
     .font_size = 0,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = "E",
     .left = "TBLSET",
     .highlight = false,
     .right = 0,
     .sub = 0                       },
    {.x = 5 * KB_COLUMN_WIDTH_6_KEYS,
     .y = 0,
     .w = 36,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = "F6",
     .font_size = 0,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = "F",
     .left = "TABLE",
     .highlight = false,
     .right = 0,
     .sub = 0                       },

    {.x = 0,
     .y = 1 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = "APPS",
     .font_size = 0,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = "G",
     .left = "FILES",
     .highlight = false,
     .right = "BEGIN",
     .sub = 0                       },
    {.x = KB_COLUMN_WIDTH_5_KEYS,
     .y = 1 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = "MODE",
     .font_size = 0,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = "H",
     .left = "CUSTOM",
     .highlight = false,
     .right = "END",
     .sub = 0                       },
    {.x = 2 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 1 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = "TOOL",
     .font_size = 0,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = "I",
     .left = "i",
     .highlight = false,
     .right = "I",
     .sub = 0                       },

    {.x = 0,
     .y = 2 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = "VAR",
     .font_size = 0,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = "J",
     .left = "UPDIR",
     .highlight = false,
     .right = "COPY",
     .sub = 0                       },
    {.x = KB_COLUMN_WIDTH_5_KEYS,
     .y = 2 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = "STO",
     .font_size = 0,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = "K",
     .left = "RCL",
     .highlight = false,
     .right = "CUT",
     .sub = 0                       },
    {.x = 2 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 2 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = "NXT",
     .font_size = 0,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = "L",
     .left = "PREV",
     .highlight = false,
     .right = "PASTE",
     .sub = 0                       },

    {.x = 3 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 1.4 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = 0,
     .font_size = 0,
     .label_graphic_w = left_width,
     .label_graphic_h = left_height,
     .label_graphic = left_bitmap,
     .letter = 0,
     .left = 0,
     .highlight = false,
     .right = 0,
     .sub = 0                       },
    {.x = 210,
     .y = 0.75 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = 0,
     .font_size = 0,
     .label_graphic_w = up_width,
     .label_graphic_h = up_height,
     .label_graphic = up_bitmap,
     .letter = 0,
     .left = 0,
     .highlight = false,
     .right = 0,
     .sub = 0                       },
    {.x = 4 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 1.4 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = 0,
     .font_size = 0,
     .label_graphic_w = right_width,
     .label_graphic_h = right_height,
     .label_graphic = right_bitmap,
     .letter = 0,
     .left = 0,
     .highlight = false,
     .right = 0,
     .sub = 0                       },
    {.x = 210,
     .y = 2 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = 0,
     .font_size = 0,
     .label_graphic_w = down_width,
     .label_graphic_h = down_height,
     .label_graphic = down_bitmap,
     .letter = 0,
     .left = 0,
     .highlight = false,
     .right = 0,
     .sub = 0                       },

    {.x = 0,
     .y = 3 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = "HIST",
     .font_size = 0,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = "M",
     .left = "CMD",
     .highlight = false,
     .right = "UNDO",
     .sub = 0                       },
    {.x = KB_COLUMN_WIDTH_5_KEYS,
     .y = 3 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = "CAT",
     .font_size = 0,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = "N",
     .left = "PRG",
     .highlight = false,
     .right = "CHARS",
     .sub = 0                       },
    {.x = 2 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 3 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = "EQW",
     .font_size = 0,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = "O",
     .left = "MTRW",
     .highlight = false,
     .right = "\x27",
     .sub = 0                       },
    {.x = 3 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 3 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = "SYMB",
     .font_size = 0,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = "P",
     .left = "MTH",
     .highlight = false,
     .right = "EVAL",
     .sub = 0                       },
    {.x = 4 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 3 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = 0,
     .font_size = 0,
     .label_graphic_w = bs_width,
     .label_graphic_h = bs_height,
     .label_graphic = bs_bitmap,
     .letter = 0,
     .left = "DEL",
     .highlight = false,
     .right = "CLEAR",
     .sub = 0                       },

    {.x = 0,
     .y = 4 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = 0,
     .font_size = 0,
     .label_graphic_w = power_width,
     .label_graphic_h = power_height,
     .label_graphic = power_bitmap,
     .letter = "Q",
     .left = "\x16",
     .highlight = false,
     .right = "LN",
     .sub = 0                       },
    {.x = KB_COLUMN_WIDTH_5_KEYS,
     .y = 4 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = 0,
     .font_size = 0,
     .label_graphic_w = sqrt_width,
     .label_graphic_h = sqrt_height,
     .label_graphic = sqrt_bitmap,
     .letter = "R",
     .left = "\x13",
     .highlight = false,
     .right = "\x14",
     .sub = 0                       },
    {.x = 2 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 4 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = "SIN",
     .font_size = 0,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = "S",
     .left = "ASIN",
     .highlight = false,
     .right = "\x09",
     .sub = 0                       },
    {.x = 3 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 4 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = "COS",
     .font_size = 2,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = "T",
     .left = "ACOS",
     .highlight = false,
     .right = "\x07",
     .sub = 0                       },
    {.x = 4 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 4 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = "TAN",
     .font_size = 0,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = "U",
     .left = "ATAN",
     .highlight = false,
     .right = "\x08",
     .sub = 0                       },

    {.x = 0,
     .y = 5 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = "EEX",
     .font_size = 0,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = "V",
     .left = "\x15",
     .highlight = false,
     .right = "LOG",
     .sub = 0                       },
    {.x = KB_COLUMN_WIDTH_5_KEYS,
     .y = 5 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = 0,
     .font_size = 0,
     .label_graphic_w = neg_width,
     .label_graphic_h = neg_height,
     .label_graphic = neg_bitmap,
     .letter = "W",
     .left = "\x60",
     .highlight = false,
     .right = "=",
     .sub = 0                       },
    {.x = 2 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 5 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = "X",
     .font_size = 0,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = "X",
     .left = "\x3b",
     .highlight = false,
     .right = "\x3c",
     .sub = 0                       },
    {.x = 3 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 5 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = 0,
     .font_size = 0,
     .label_graphic_w = inv_width,
     .label_graphic_h = inv_height,
     .label_graphic = inv_bitmap,
     .letter = "Y",
     .left = "\x3f",
     .highlight = false,
     .right = "\x3e",
     .sub = 0                       },
    {.x = 4 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 5 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = 0,
     .font_size = 0,
     .label_graphic_w = div_width,
     .label_graphic_h = div_height,
     .label_graphic = div_bitmap,
     .letter = "Z",
     .left = "ABS ",
     .highlight = false,
     .right = "ARG",
     .sub = 0                       },

    {.x = 0,
     .y = 6 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_ALPHA,
     .label_text = 0,
     .font_size = 0,
     .label_graphic_w = alpha_width,
     .label_graphic_h = alpha_height,
     .label_graphic = alpha_bitmap,
     .letter = 0,
     .left = "USER",
     .highlight = false,
     .right = "ENTRY",
     .sub = 0                       },
    {.x = KB_COLUMN_WIDTH_5_KEYS,
     .y = 6 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = "7",
     .font_size = 1,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = 0,
     .left = "S.SLV",
     .highlight = false,
     .right = "N.SLV",   //"NUM.SLV",
      .sub = 0},
    {.x = 2 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 6 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = "8",
     .font_size = 1,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = 0,
     .left = "EXP",    //"EXP&LN",
      .highlight = false,
     .right = "TRIG",
     .sub = 0                       },
    {.x = 3 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 6 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = "9",
     .font_size = 1,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = 0,
     .left = "FIN.",   //"FINANCE",
      .highlight = false,
     .right = "TIME",
     .sub = 0                       },
    {.x = 4 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 6 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = 0,
     .font_size = 0,
     .label_graphic_w = mul_width,
     .label_graphic_h = mul_height,
     .label_graphic = mul_bitmap,
     .letter = 0,
     .left = "\x19 ",
     .highlight = false,
     .right = "\x1c",
     .sub = 0                       },

    {.x = 0,
     .y = 7 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_SHIFT_LEFT,
     .label_text = 0,
     .font_size = 0,
     .label_graphic_w = shl_width,
     .label_graphic_h = shl_height,
     .label_graphic = shl_bitmap,
     .letter = 0,
     .left = 0,
     .highlight = false,
     .right = 0,
     .sub = 0                       },
    {.x = KB_COLUMN_WIDTH_5_KEYS,
     .y = 7 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = "4",
     .font_size = 1,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = 0,
     .left = "CALC",
     .highlight = false,
     .right = "ALG",
     .sub = 0                       },
    {.x = 2 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 7 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = "5",
     .font_size = 1,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = 0,
     .left = "MATR.",  //"MATRICES",
      .highlight = false,
     .right = "STAT",
     .sub = 0                       },
    {.x = 3 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 7 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = "6",
     .font_size = 1,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = 0,
     .left = "CNVRT",  //"CONVERT",
      .highlight = false,
     .right = "UNITS",
     .sub = 0                       },
    {.x = 4 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 7 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = 0,
     .font_size = 0,
     .label_graphic_w = minus_width,
     .label_graphic_h = minus_height,
     .label_graphic = minus_bitmap,
     .letter = 0,
     .left = "\x17 ",
     .highlight = false,
     .right = "\x1a",
     .sub = 0                       },

    {.x = 0,
     .y = 8 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_SHIFT_RIGHT,
     .label_text = 0,
     .font_size = 0,
     .label_graphic_w = shr_width,
     .label_graphic_h = shr_height,
     .label_graphic = shr_bitmap,
     .letter = 0,
     .left = " ",
     .highlight = false,
     .right = 0,
     .sub = 0                       },
    {.x = KB_COLUMN_WIDTH_5_KEYS,
     .y = 8 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = "1",
     .font_size = 1,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = 0,
     .left = "ARITH",
     .highlight = false,
     .right = "CMPLX",
     .sub = 0                       },
    {.x = 2 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 8 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = "2",
     .font_size = 1,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = 0,
     .left = "DEF",
     .highlight = false,
     .right = "LIB",
     .sub = 0                       },
    {.x = 3 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 8 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = "3",
     .font_size = 1,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = 0,
     .left = "\x23",
     .highlight = false,
     .right = "BASE",
     .sub = 0                       },
    {.x = 4 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 8 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = 0,
     .font_size = 0,
     .label_graphic_w = plus_width,
     .label_graphic_h = plus_height,
     .label_graphic = plus_bitmap,
     .letter = 0,
     .left = "\x1d ",
     .highlight = false,
     .right = "\x1b",
     .sub = 0                       },

    {.x = 0,
     .y = 9 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = "ON",
     .font_size = 0,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = 0,
     .left = "CONT",
     .highlight = false,
     .right = "OFF",
     .sub = "CANCEL"                },
    {.x = KB_COLUMN_WIDTH_5_KEYS,
     .y = 9 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = "0",
     .font_size = 1,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = 0,
     .left = "\x7e",
     .highlight = false,
     .right = "\x03",
     .sub = 0                       },
    {.x = 2 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 9 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = ".",
     .font_size = 1,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = 0,
     .left = "\x1e",
     .highlight = false,
     .right = "\x01",
     .sub = 0                       },
    {.x = 3 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 9 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = "SPC",
     .font_size = 0,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = 0,
     .left = "\x05 ",
     .highlight = false,
     .right = "\x02",
     .sub = 0                       },
    {.x = 4 * KB_COLUMN_WIDTH_5_KEYS,
     .y = 9 * KB_LINE_HEIGHT,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_LABEL,
     .label_text = "ENTER",
     .font_size = 0,
     .label_graphic_w = 0,
     .label_graphic_h = 0,
     .label_graphic = 0,
     .letter = 0,
     .left = "ANS ",
     .highlight = false,
     .right = "\x06NUM",
     .sub = 0                       },
};
