#include "inner.h"

color_t colors_48sx[ NB_COLORS ] = {
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
     .g = 0xA6,
     .b = 0x0,
     .a = 0xFF,
     .mono_rgb = 0xFF,
     .gray_rgb = 0xE6,
     },
    /* UI4X_COLOR_SHIFT_RIGHT */
    {
     .r = 0x0,
     .g = 0xD2,
     .b = 0xFF,
     .a = 0xFF,
     .mono_rgb = 0xFF,
     .gray_rgb = 0xA9,
     },
    /* UI4X_COLOR_BUTTON_EDGE_TOP */
    {
     .r = 0x6D,
     .g = 0x5D,
     .b = 0x5D,
     .a = 0xFF,
     .mono_rgb = 0x0,
     .gray_rgb = 0x5B,
     },
    /* UI4X_COLOR_BUTTON */
    {
     .r = 0x5A,
     .g = 0x4D,
     .b = 0x4D,
     .a = 0xFF,
     .mono_rgb = 0x0,
     .gray_rgb = 0x51,
     },
    /* UI4X_COLOR_BUTTON_EDGE_BOTTOM */
    {
     .r = 0x4C,
     .g = 0x41,
     .b = 0x41,
     .a = 0xFF,
     .mono_rgb = 0x0,
     .gray_rgb = 0x45,
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
     .r = 0x6D,
     .g = 0x4E,
     .b = 0x4E,
     .a = 0xFF,
     .mono_rgb = 0x0,
     .gray_rgb = 0x58,
     },
    /* UI4X_COLOR_FACEPLATE */
    {
     .r = 0x5A,
     .g = 0x40,
     .b = 0x40,
     .a = 0xFF,
     .mono_rgb = 0x0,
     .gray_rgb = 0x49,
     },
    /* UI4X_COLOR_FACEPLATE_EDGE_BOTTOM */
    {
     .r = 0x4C,
     .g = 0x36,
     .b = 0x36,
     .a = 0xFF,
     .mono_rgb = 0x0,
     .gray_rgb = 0x3C,
     },
    /* UI4X_COLOR_UPPER_FACEPLATE_EDGE_TOP */
    {
     .r = 0x9B,
     .g = 0x76,
     .b = 0x54,
     .a = 0xFF,
     .mono_rgb = 0x0,
     .gray_rgb = 0x7C,
     },
    /* UI4X_COLOR_UPPER_FACEPLATE */
    {
     .r = 0x7C,
     .g = 0x5E,
     .b = 0x43,
     .a = 0xFF,
     .mono_rgb = 0x0,
     .gray_rgb = 0x63,
     },
    /* UI4X_COLOR_UPPER_FACEPLATE_EDGE_BOTTOM */
    {
     .r = 0x64,
     .g = 0x4B,
     .b = 0x35,
     .a = 0xFF,
     .mono_rgb = 0x0,
     .gray_rgb = 0x4F,
     },
    /* UI4X_COLOR_HP_LOGO */
    {
     .r = 0xCC,
     .g = 0xA9,
     .b = 0x6B,
     .a = 0xFF,
     .mono_rgb = 0xFF,
     .gray_rgb = 0xAC,
     },
    /* UI4X_COLOR_HP_LOGO_BG */
    {
     .r = 0x40,
     .g = 0x40,
     .b = 0x40,
     .a = 0xFF,
     .mono_rgb = 0x0,
     .gray_rgb = 0x41,
     },
    /* UI4X_COLOR_48GX_128K_RAM */
    {
     .r = 0xCA,
     .g = 0xB8,
     .b = 0x90,
     .a = 0xFF,
     .mono_rgb = 0xFF,
     .gray_rgb = 0xB9,
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
     .r = 0x3C,
     .g = 0x2A,
     .b = 0x2A,
     .a = 0xFF,
     .mono_rgb = 0x0,
     .gray_rgb = 0x30,
     },
};

button_t buttons_48sx[ NB_HP48_KEYS ] = {
    {.x = 0,
     .y = 0,
     .w = 36,
     .h = 23,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = 0,
     .font_size = 0,
     .label_bits_w = menu_label_width,
     .label_bits_h = menu_label_height,
     .label_bits = menu_label_bitmap,
     .letter = "A",
     .left = 0,
     .highlight = false,
     .right = 0,
     .sub = 0     },
    {.x = 50,
     .y = 0,
     .w = 36,
     .h = 23,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = 0,
     .font_size = 0,
     .label_bits_w = menu_label_width,
     .label_bits_h = menu_label_height,
     .label_bits = menu_label_bitmap,
     .letter = "B",
     .left = 0,
     .highlight = false,
     .right = 0,
     .sub = 0     },
    {.x = 100,
     .y = 0,
     .w = 36,
     .h = 23,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = 0,
     .font_size = 0,
     .label_bits_w = menu_label_width,
     .label_bits_h = menu_label_height,
     .label_bits = menu_label_bitmap,
     .letter = "C",
     .left = 0,
     .highlight = false,
     .right = 0,
     .sub = 0     },
    {.x = 150,
     .y = 0,
     .w = 36,
     .h = 23,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = 0,
     .font_size = 0,
     .label_bits_w = menu_label_width,
     .label_bits_h = menu_label_height,
     .label_bits = menu_label_bitmap,
     .letter = "D",
     .left = 0,
     .highlight = false,
     .right = 0,
     .sub = 0     },
    {.x = 200,
     .y = 0,
     .w = 36,
     .h = 23,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = 0,
     .font_size = 0,
     .label_bits_w = menu_label_width,
     .label_bits_h = menu_label_height,
     .label_bits = menu_label_bitmap,
     .letter = "E",
     .left = 0,
     .highlight = false,
     .right = 0,
     .sub = 0     },
    {.x = 250,
     .y = 0,
     .w = 36,
     .h = 23,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = 0,
     .font_size = 0,
     .label_bits_w = menu_label_width,
     .label_bits_h = menu_label_height,
     .label_bits = menu_label_bitmap,
     .letter = "F",
     .left = 0,
     .highlight = false,
     .right = 0,
     .sub = 0     },

    {.x = 0,
     .y = 50,
     .w = 36,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = "MTH",
     .font_size = 0,
     .label_bits_w = 0,
     .label_bits_h = 0,
     .label_bits = 0,
     .letter = "G",
     .left = "PRINT",
     .highlight = true,
     .right = 0,
     .sub = 0     },
    {.x = 50,
     .y = 50,
     .w = 36,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = "PRG",
     .font_size = 0,
     .label_bits_w = 0,
     .label_bits_h = 0,
     .label_bits = 0,
     .letter = "H",
     .left = "I/O",
     .highlight = true,
     .right = 0,
     .sub = 0     },
    {.x = 100,
     .y = 50,
     .w = 36,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = "CST",
     .font_size = 0,
     .label_bits_w = 0,
     .label_bits_h = 0,
     .label_bits = 0,
     .letter = "I",
     .left = "MODES",
     .highlight = true,
     .right = 0,
     .sub = 0     },
    {.x = 150,
     .y = 50,
     .w = 36,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = "VAR",
     .font_size = 0,
     .label_bits_w = 0,
     .label_bits_h = 0,
     .label_bits = 0,
     .letter = "J",
     .left = "MEMORY",
     .highlight = true,
     .right = 0,
     .sub = 0     },
    {.x = 200,
     .y = 50,
     .w = 36,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = 0,
     .font_size = 0,
     .label_bits_w = up_width,
     .label_bits_h = up_height,
     .label_bits = up_bitmap,
     .letter = "K",
     .left = "LIBRARY",
     .highlight = true,
     .right = 0,
     .sub = 0     },
    {.x = 250,
     .y = 50,
     .w = 36,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = "NXT",
     .font_size = 0,
     .label_bits_w = 0,
     .label_bits_h = 0,
     .label_bits = 0,
     .letter = "L",
     .left = "PREV",
     .highlight = false,
     .right = 0,
     .sub = 0     },

    {.x = 0,
     .y = 100,
     .w = 36,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = 0,
     .font_size = 0,
     .label_bits_w = quote_width,
     .label_bits_h = quote_height,
     .label_bits = quote_bitmap,
     .letter = "M",
     .left = "UP",
     .highlight = false,
     .right = "HOME",
     .sub = 0     },
    {.x = 50,
     .y = 100,
     .w = 36,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = "STO",
     .font_size = 0,
     .label_bits_w = 0,
     .label_bits_h = 0,
     .label_bits = 0,
     .letter = "N",
     .left = "DEF",
     .highlight = false,
     .right = "RCL",
     .sub = 0     },
    {.x = 100,
     .y = 100,
     .w = 36,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = "EVAL",
     .font_size = 0,
     .label_bits_w = 0,
     .label_bits_h = 0,
     .label_bits = 0,
     .letter = "O",
     .left = "\x80Q",
     .highlight = false,
     .right = "\x80NUM",
     .sub = 0     },
    {.x = 150,
     .y = 100,
     .w = 36,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = 0,
     .font_size = 0,
     .label_bits_w = left_width,
     .label_bits_h = left_height,
     .label_bits = left_bitmap,
     .letter = "P",
     .left = "GRAPH",
     .highlight = false,
     .right = 0,
     .sub = 0     },
    {.x = 200,
     .y = 100,
     .w = 36,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = 0,
     .font_size = 0,
     .label_bits_w = down_width,
     .label_bits_h = down_height,
     .label_bits = down_bitmap,
     .letter = "Q",
     .left = "REVIEW",
     .highlight = false,
     .right = 0,
     .sub = 0     },
    {.x = 250,
     .y = 100,
     .w = 36,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = 0,
     .font_size = 0,
     .label_bits_w = right_width,
     .label_bits_h = right_height,
     .label_bits = right_bitmap,
     .letter = "R",
     .left = "SWAP",
     .highlight = false,
     .right = 0,
     .sub = 0     },

    {.x = 0,
     .y = 150,
     .w = 36,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = "SIN",
     .font_size = 0,
     .label_bits_w = 0,
     .label_bits_h = 0,
     .label_bits = 0,
     .letter = "S",
     .left = "ASIN",
     .highlight = false,
     .right = "\x07",
     .sub = 0     },
    {.x = 50,
     .y = 150,
     .w = 36,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = "COS",
     .font_size = 0,
     .label_bits_w = 0,
     .label_bits_h = 0,
     .label_bits = 0,
     .letter = "T",
     .left = "ACOS",
     .highlight = false,
     .right = "\x08",
     .sub = 0     },
    {.x = 100,
     .y = 150,
     .w = 36,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = "TAN",
     .font_size = 0,
     .label_bits_w = 0,
     .label_bits_h = 0,
     .label_bits = 0,
     .letter = "U",
     .left = "ATAN",
     .highlight = false,
     .right = "\x09",
     .sub = 0     },
    {.x = 150,
     .y = 150,
     .w = 36,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = 0,
     .font_size = 0,
     .label_bits_w = sqrt_width,
     .label_bits_h = sqrt_height,
     .label_bits = sqrt_bitmap,
     .letter = "V",
     .left = "\x0a",
     .highlight = false,
     .right = "\x0b",
     .sub = 0     },
    {.x = 200,
     .y = 150,
     .w = 36,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = 0,
     .font_size = 0,
     .label_bits_w = power_width,
     .label_bits_h = power_height,
     .label_bits = power_bitmap,
     .letter = "W",
     .left = "\x0c",
     .highlight = false,
     .right = "LOG",
     .sub = 0     },
    {.x = 250,
     .y = 150,
     .w = 36,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = 0,
     .font_size = 0,
     .label_bits_w = inv_width,
     .label_bits_h = inv_height,
     .label_bits = inv_bitmap,
     .letter = "X",
     .left = "\x0d",
     .highlight = false,
     .right = "LN",
     .sub = 0     },

    {.x = 0,
     .y = 200,
     .w = 86,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = "ENTER",
     .font_size = 2,
     .label_bits_w = 0,
     .label_bits_h = 0,
     .label_bits = 0,
     .letter = 0,
     .left = "EQUATION",
     .highlight = false,
     .right = "MATRIX",
     .sub = 0     },
    {.x = 100,
     .y = 200,
     .w = 36,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = 0,
     .font_size = 0,
     .label_bits_w = neg_width,
     .label_bits_h = neg_height,
     .label_bits = neg_bitmap,
     .letter = "Y",
     .left = "EDIT",
     .highlight = false,
     .right = "VISIT",
     .sub = 0     },
    {.x = 150,
     .y = 200,
     .w = 36,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = "EEX",
     .font_size = 0,
     .label_bits_w = 0,
     .label_bits_h = 0,
     .label_bits = 0,
     .letter = "Z",
     .left = "2D",
     .highlight = false,
     .right = "3D",
     .sub = 0     },
    {.x = 200,
     .y = 200,
     .w = 36,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = "DEL",
     .font_size = 0,
     .label_bits_w = 0,
     .label_bits_h = 0,
     .label_bits = 0,
     .letter = 0,
     .left = "PURGE",
     .highlight = false,
     .right = 0,
     .sub = 0     },
    {.x = 250,
     .y = 200,
     .w = 36,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = 0,
     .font_size = 0,
     .label_bits_w = bs_width,
     .label_bits_h = bs_height,
     .label_bits = bs_bitmap,
     .letter = 0,
     .left = "DROP",
     .highlight = false,
     .right = "CLR",
     .sub = 0     },

    {.x = 0,
     .y = 250,
     .w = 36,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = 0,
     .font_size = 0,
     .label_bits_w = alpha_width,
     .label_bits_h = alpha_height,
     .label_bits = alpha_bitmap,
     .letter = 0,
     .left = "USR",
     .highlight = false,
     .right = "ENTRY",
     .sub = 0     },
    {.x = 60,
     .y = 250,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = "7",
     .font_size = 1,
     .label_bits_w = 0,
     .label_bits_h = 0,
     .label_bits = 0,
     .letter = 0,
     .left = "SOLVE",
     .highlight = true,
     .right = 0,
     .sub = 0     },
    {.x = 120,
     .y = 250,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = "8",
     .font_size = 1,
     .label_bits_w = 0,
     .label_bits_h = 0,
     .label_bits = 0,
     .letter = 0,
     .left = "PLOT",
     .highlight = true,
     .right = 0,
     .sub = 0     },
    {.x = 180,
     .y = 250,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = "9",
     .font_size = 1,
     .label_bits_w = 0,
     .label_bits_h = 0,
     .label_bits = 0,
     .letter = 0,
     .left = "ALGEBRA",
     .highlight = true,
     .right = 0,
     .sub = 0     },
    {.x = 240,
     .y = 250,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = 0,
     .font_size = 0,
     .label_bits_w = div_width,
     .label_bits_h = div_height,
     .label_bits = div_bitmap,
     .letter = 0,
     .left = "( )",
     .highlight = false,
     .right = "#",
     .sub = 0     },

    {.x = 0,
     .y = 300,
     .w = 36,
     .h = 26,
     .label_color = UI4X_COLOR_SHIFT_LEFT,
     .label_text = 0,
     .font_size = 0,
     .label_bits_w = shl_width,
     .label_bits_h = shl_height,
     .label_bits = shl_bitmap,
     .letter = 0,
     .left = 0,
     .highlight = false,
     .right = 0,
     .sub = 0     },
    {.x = 60,
     .y = 300,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = "4",
     .font_size = 1,
     .label_bits_w = 0,
     .label_bits_h = 0,
     .label_bits = 0,
     .letter = 0,
     .left = "TIME",
     .highlight = true,
     .right = 0,
     .sub = 0     },
    {.x = 120,
     .y = 300,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = "5",
     .font_size = 1,
     .label_bits_w = 0,
     .label_bits_h = 0,
     .label_bits = 0,
     .letter = 0,
     .left = "STAT",
     .highlight = true,
     .right = 0,
     .sub = 0     },
    {.x = 180,
     .y = 300,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = "6",
     .font_size = 1,
     .label_bits_w = 0,
     .label_bits_h = 0,
     .label_bits = 0,
     .letter = 0,
     .left = "UNITS",
     .highlight = true,
     .right = 0,
     .sub = 0     },
    {.x = 240,
     .y = 300,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = 0,
     .font_size = 0,
     .label_bits_w = mul_width,
     .label_bits_h = mul_height,
     .label_bits = mul_bitmap,
     .letter = 0,
     .left = "[ ]",
     .highlight = false,
     .right = "_",
     .sub = 0     },

    {.x = 0,
     .y = 350,
     .w = 36,
     .h = 26,
     .label_color = UI4X_COLOR_SHIFT_RIGHT,
     .label_text = 0,
     .font_size = 0,
     .label_bits_w = shr_width,
     .label_bits_h = shr_height,
     .label_bits = shr_bitmap,
     .letter = 0,
     .left = 0,
     .highlight = false,
     .right = 0,
     .sub = 0     },
    {.x = 60,
     .y = 350,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = "1",
     .font_size = 1,
     .label_bits_w = 0,
     .label_bits_h = 0,
     .label_bits = 0,
     .letter = 0,
     .left = "RAD",
     .highlight = false,
     .right = "POLAR",
     .sub = 0     },
    {.x = 120,
     .y = 350,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = "2",
     .font_size = 1,
     .label_bits_w = 0,
     .label_bits_h = 0,
     .label_bits = 0,
     .letter = 0,
     .left = "STACK",
     .highlight = false,
     .right = "ARG",
     .sub = 0     },
    {.x = 180,
     .y = 350,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = "3",
     .font_size = 1,
     .label_bits_w = 0,
     .label_bits_h = 0,
     .label_bits = 0,
     .letter = 0,
     .left = "CMD",
     .highlight = false,
     .right = "MENU",
     .sub = 0     },
    {.x = 240,
     .y = 350,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = 0,
     .font_size = 0,
     .label_bits_w = minus_width,
     .label_bits_h = minus_height,
     .label_bits = minus_bitmap,
     .letter = 0,
     .left = "\x0e",
     .highlight = false,
     .right = "\x0f",
     .sub = 0     },

    {.x = 0,
     .y = 400,
     .w = 36,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = "ON",
     .font_size = 0,
     .label_bits_w = 0,
     .label_bits_h = 0,
     .label_bits = 0,
     .letter = 0,
     .left = "CONT",
     .highlight = false,
     .right = "OFF",
     .sub = "ATTN"},
    {.x = 60,
     .y = 400,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = "0",
     .font_size = 1,
     .label_bits_w = 0,
     .label_bits_h = 0,
     .label_bits = 0,
     .letter = 0,
     .left = "= ",
     .highlight = false,
     .right = " \x80",
     .sub = 0     },
    {.x = 120,
     .y = 400,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = ".",
     .font_size = 1,
     .label_bits_w = 0,
     .label_bits_h = 0,
     .label_bits = 0,
     .letter = 0,
     .left = ",",
     .highlight = false,
     .right = " \x10",
     .sub = 0     },
    {.x = 180,
     .y = 400,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = "SPC",
     .font_size = 0,
     .label_bits_w = 0,
     .label_bits_h = 0,
     .label_bits = 0,
     .letter = 0,
     .left = "\x11 ",
     .highlight = false,
     .right = " \x12",
     .sub = 0     },
    {.x = 240,
     .y = 400,
     .w = 46,
     .h = 26,
     .label_color = UI4X_COLOR_WHITE,
     .label_text = 0,
     .font_size = 0,
     .label_bits_w = plus_width,
     .label_bits_h = plus_height,
     .label_bits = plus_bitmap,
     .letter = 0,
     .left = "{ }",
     .highlight = false,
     .right = ": :",
     .sub = 0     },
};
