#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ui48_config.h"
#include "ui48_emulator.h"

#include "config.h"
#include "keyb.h"
#include "modules.h"

config_t config;

typedef struct hpkey_t {
    int code;
    bool pressed;
    char* inOut;
} hpkey_t;

static hpkey_t keyboard48[ NB_KEYS ] = {
    /* From top left to bottom right */
    {0x14,   false, "1/10"},
    {0x84,   false, "8/10"},
    {0x83,   false, "8/08"},
    {0x82,   false, "8/04"},
    {0x81,   false, "8/02"},
    {0x80,   false, "8/01"},

    {0x24,   false, "2/10"},
    {0x74,   false, "7/10"},
    {0x73,   false, "7/08"},
    {0x72,   false, "7/04"},
    {0x71,   false, "7/02"},
    {0x70,   false, "7/01"},

    {0x04,   false, "0/10"},
    {0x64,   false, "6/10"},
    {0x63,   false, "6/08"},
    {0x62,   false, "6/04"},
    {0x61,   false, "6/02"},
    {0x60,   false, "6/01"},

    {0x34,   false, "3/10"},
    {0x54,   false, "5/10"},
    {0x53,   false, "5/08"},
    {0x52,   false, "5/04"},
    {0x51,   false, "5/02"},
    {0x50,   false, "5/01"},

    {0x44,   false, "4/10"},
    {0x43,   false, "4/08"},
    {0x42,   false, "4/04"},
    {0x41,   false, "4/02"},
    {0x40,   false, "4/01"},

    {0x35,   false, "3/20"},
    {0x33,   false, "3/08"},
    {0x32,   false, "3/04"},
    {0x31,   false, "3/02"},
    {0x30,   false, "3/01"},

    {0x25,   false, "2/20"},
    {0x23,   false, "2/08"},
    {0x22,   false, "2/04"},
    {0x21,   false, "2/02"},
    {0x20,   false, "2/01"},

    {0x15,   false, "1/20"},
    {0x13,   false, "1/08"},
    {0x12,   false, "1/04"},
    {0x11,   false, "1/02"},
    {0x10,   false, "1/01"},

    {0x8000, false, "*"   },
    {0x03,   false, "0/08"},
    {0x02,   false, "0/04"},
    {0x01,   false, "0/02"},
    {0x00,   false, "0/01"},
};

void press_key( int hpkey )
{
    // Check not already pressed (may be important: avoids a useless do_kbd_int)
    if ( keyboard48[ hpkey ].pressed )
        return;

    keyboard48[ hpkey ].pressed = true;

    KeybPress( keyboard48[ hpkey ].inOut );
}

void release_key( int hpkey )
{
    // Check not already released (not critical)
    if ( !keyboard48[ hpkey ].pressed )
        return;

    keyboard48[ hpkey ].pressed = false;

    KeybRelease( keyboard48[ hpkey ].inOut );
}

bool is_key_pressed( int hpkey ) { return keyboard48[ hpkey ].pressed; }

unsigned char get_annunciators( void ) { return mod_status.hdw.lcd_ann; }

bool get_display_state( void ) { return mod_status.hdw.lcd_on; }

void get_lcd_buffer( int* target )
{
    Address addr = mod_status.hdw.lcd_base_addr;
    int x, y;
    Nibble v;

    /* Scan active display rows */
    for ( y = 0; y <= mod_status.hdw.lcd_vlc; y++ ) {
        /* Scan columns */
        for ( x = 0; x < NIBBLES_PER_ROW; x++ ) {
            v = FetchNibble( addr++ );

            for ( int nx = 0; nx < 4; nx++ )
                target[ ( y * 131 ) + ( x * 4 ) + nx ] = v & ( 1 << ( nx & 3 ) );
        }

        addr += mod_status.hdw.lcd_line_offset;
    }

    /* Scan menu display rows */
    addr = mod_status.hdw.lcd_menu_addr;
#define MAX_ROWS 64
    for ( ; y < MAX_ROWS; y++ ) {
        /* Scan columns */
        for ( x = 0; x < NIBBLES_PER_ROW; x++ ) {
            v = FetchNibble( addr++ );

            for ( int nx = 0; nx < 4; nx++ )
                target[ ( y * 131 ) + ( x * 4 ) + nx ] = v & ( 1 << ( nx & 3 ) );
        }
    }
}

int get_contrast( void ) { return mod_status.hdw.lcd_contrast; }

void init_emulator( config_t* conf )
{
    config = *conf;

    EmulatorInit();
}

void exit_emulator( void ) { EmulatorExit( SAVE_AND_EXIT ); }
