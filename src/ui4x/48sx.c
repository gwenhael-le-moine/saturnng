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
     .lb_w = menu_label_width,
     .lb_h = menu_label_height,
     .lb = menu_label_bitmap,
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
     .lb_w = menu_label_width,
     .lb_h = menu_label_height,
     .lb = menu_label_bitmap,
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
     .lb_w = menu_label_width,
     .lb_h = menu_label_height,
     .lb = menu_label_bitmap,
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
     .lb_w = menu_label_width,
     .lb_h = menu_label_height,
     .lb = menu_label_bitmap,
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
     .lb_w = menu_label_width,
     .lb_h = menu_label_height,
     .lb = menu_label_bitmap,
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
     .lb_w = menu_label_width,
     .lb_h = menu_label_height,
     .lb = menu_label_bitmap,
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
     .lb_w = 0,
     .lb_h = 0,
     .lb = 0,
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
     .lb_w = 0,
     .lb_h = 0,
     .lb = 0,
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
     .lb_w = 0,
     .lb_h = 0,
     .lb = 0,
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
     .lb_w = 0,
     .lb_h = 0,
     .lb = 0,
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
     .lb_w = up_width,
     .lb_h = up_height,
     .lb = up_bitmap,
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
     .lb_w = 0,
     .lb_h = 0,
     .lb = 0,
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
     .lb_w = quote_width,
     .lb_h = quote_height,
     .lb = quote_bitmap,
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
     .lb_w = 0,
     .lb_h = 0,
     .lb = 0,
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
     .lb_w = 0,
     .lb_h = 0,
     .lb = 0,
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
     .lb_w = left_width,
     .lb_h = left_height,
     .lb = left_bitmap,
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
     .lb_w = down_width,
     .lb_h = down_height,
     .lb = down_bitmap,
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
     .lb_w = right_width,
     .lb_h = right_height,
     .lb = right_bitmap,
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
     .lb_w = 0,
     .lb_h = 0,
     .lb = 0,
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
     .lb_w = 0,
     .lb_h = 0,
     .lb = 0,
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
     .lb_w = 0,
     .lb_h = 0,
     .lb = 0,
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
     .lb_w = sqrt_width,
     .lb_h = sqrt_height,
     .lb = sqrt_bitmap,
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
     .lb_w = power_width,
     .lb_h = power_height,
     .lb = power_bitmap,
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
     .lb_w = inv_width,
     .lb_h = inv_height,
     .lb = inv_bitmap,
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
     .lb_w = 0,
     .lb_h = 0,
     .lb = 0,
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
     .lb_w = neg_width,
     .lb_h = neg_height,
     .lb = neg_bitmap,
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
     .lb_w = 0,
     .lb_h = 0,
     .lb = 0,
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
     .lb_w = 0,
     .lb_h = 0,
     .lb = 0,
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
     .lb_w = bs_width,
     .lb_h = bs_height,
     .lb = bs_bitmap,
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
     .lb_w = alpha_width,
     .lb_h = alpha_height,
     .lb = alpha_bitmap,
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
     .lb_w = 0,
     .lb_h = 0,
     .lb = 0,
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
     .lb_w = 0,
     .lb_h = 0,
     .lb = 0,
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
     .lb_w = 0,
     .lb_h = 0,
     .lb = 0,
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
     .lb_w = div_width,
     .lb_h = div_height,
     .lb = div_bitmap,
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
     .lb_w = shl_width,
     .lb_h = shl_height,
     .lb = shl_bitmap,
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
     .lb_w = 0,
     .lb_h = 0,
     .lb = 0,
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
     .lb_w = 0,
     .lb_h = 0,
     .lb = 0,
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
     .lb_w = 0,
     .lb_h = 0,
     .lb = 0,
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
     .lb_w = mul_width,
     .lb_h = mul_height,
     .lb = mul_bitmap,
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
     .lb_w = shr_width,
     .lb_h = shr_height,
     .lb = shr_bitmap,
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
     .lb_w = 0,
     .lb_h = 0,
     .lb = 0,
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
     .lb_w = 0,
     .lb_h = 0,
     .lb = 0,
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
     .lb_w = 0,
     .lb_h = 0,
     .lb = 0,
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
     .lb_w = minus_width,
     .lb_h = minus_height,
     .lb = minus_bitmap,
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
     .lb_w = 0,
     .lb_h = 0,
     .lb = 0,
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
     .lb_w = 0,
     .lb_h = 0,
     .lb = 0,
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
     .lb_w = 0,
     .lb_h = 0,
     .lb = 0,
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
     .lb_w = 0,
     .lb_h = 0,
     .lb = 0,
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
     .lb_w = plus_width,
     .lb_h = plus_height,
     .lb = plus_bitmap,
     .letter = 0,
     .left = "{ }",
     .highlight = false,
     .right = ": :",
     .sub = 0     },
};
