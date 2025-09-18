#include <stdio.h>

#include "options.h"

#include "core/emulator.h"
#include "core/keyboard.h"
#include "core/modules.h"
#include "core/serial.h"

#include "ui4x/api.h"

#define KEYBOARD ( __config.model == MODEL_48GX || __config.model == MODEL_48SX ? keyboard48 : keyboard49 )

typedef struct hpkey_t {
    int code;
    bool pressed;
} hpkey_t;

static hpkey_t keyboard48[ NB_HP48_KEYS ] = {
    /* From top left to bottom right */
    {0x14,   false},
    {0x84,   false},
    {0x83,   false},
    {0x82,   false},
    {0x81,   false},
    {0x80,   false},

    {0x24,   false},
    {0x74,   false},
    {0x73,   false},
    {0x72,   false},
    {0x71,   false},
    {0x70,   false},

    {0x04,   false},
    {0x64,   false},
    {0x63,   false},
    {0x62,   false},
    {0x61,   false},
    {0x60,   false},

    {0x34,   false},
    {0x54,   false},
    {0x53,   false},
    {0x52,   false},
    {0x51,   false},
    {0x50,   false},

    {0x44,   false},
    {0x43,   false},
    {0x42,   false},
    {0x41,   false},
    {0x40,   false},

    {0x35,   false},
    {0x33,   false},
    {0x32,   false},
    {0x31,   false},
    {0x30,   false},

    {0x25,   false},
    {0x23,   false},
    {0x22,   false},
    {0x21,   false},
    {0x20,   false},

    {0x15,   false},
    {0x13,   false},
    {0x12,   false},
    {0x11,   false},
    {0x10,   false},

    {0x8000, false},
    {0x03,   false},
    {0x02,   false},
    {0x01,   false},
    {0x00,   false},
};

static hpkey_t keyboard49[ NB_HP49_KEYS ] = {
    /* From top left to bottom right */
    {0x50,   false},
    {0x51,   false},
    {0x52,   false},
    {0x53,   false},
    {0x54,   false},
    {0x55,   false},

    {0x57,   false},
    {0x47,   false},
    {0x37,   false},

    {0x27,   false},
    {0x17,   false},
    {0x07,   false},

    {0x62,   false},
    {0x63,   false},
    {0x60,   false},
    {0x61,   false},

    {0x46,   false},
    {0x36,   false},
    {0x26,   false},
    {0x16,   false},
    {0x06,   false},

    {0x45,   false},
    {0x35,   false},
    {0x25,   false},
    {0x15,   false},
    {0x05,   false},

    {0x44,   false},
    {0x34,   false},
    {0x24,   false},
    {0x14,   false},
    {0x04,   false},

    {0x73,   false},
    {0x33,   false},
    {0x23,   false},
    {0x13,   false},
    {0x03,   false},

    {0x72,   false},
    {0x32,   false},
    {0x22,   false},
    {0x12,   false},
    {0x02,   false},

    {0x71,   false},
    {0x31,   false},
    {0x21,   false},
    {0x11,   false},
    {0x01,   false},

    {0x8000, false},
    {0x30,   false},
    {0x20,   false},
    {0x10,   false},
    {0x00,   false},
};

static config_t __config;

void press_key( int hpkey )
{
    if ( hpkey < 0 || hpkey > NB_KEYS )
        return;
    // Check not already pressed (may be important: avoids a useless do_kbd_int)
    if ( KEYBOARD[ hpkey ].pressed )
        return;

    KEYBOARD[ hpkey ].pressed = true;

    KeybPress( KEYBOARD[ hpkey ].code );
}

void release_key( int hpkey )
{
    if ( hpkey < 0 || hpkey > NB_KEYS )
        return;
    // Check not already released (not critical)
    if ( !KEYBOARD[ hpkey ].pressed )
        return;

    KEYBOARD[ hpkey ].pressed = false;

    KeybRelease( KEYBOARD[ hpkey ].code );
}

bool is_key_pressed( int hpkey )
{
    if ( hpkey < 0 || hpkey > NB_KEYS )
        return false;

    return KEYBOARD[ hpkey ].pressed;
}

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

            // split nibble
            for ( int nx = 0; nx < 4; nx++ )
                target[ ( y * LCD_WIDTH ) + ( x * 4 ) + nx ] = ( v & ( 1 << ( nx & 3 ) ) ) > 0 ? 1 : 0;
        }

        addr += mod_status.hdw.lcd_line_offset;
    }

    /* Scan menu display rows */
    addr = mod_status.hdw.lcd_menu_addr;
    for ( ; y < LCD_HEIGHT; y++ ) {
        /* Scan columns */
        for ( x = 0; x < NIBBLES_PER_ROW; x++ ) {
            v = FetchNibble( addr++ );

            // split nibble
            for ( int nx = 0; nx < 4; nx++ )
                target[ ( y * LCD_WIDTH ) + ( x * 4 ) + nx ] = ( v & ( 1 << ( nx & 3 ) ) ) > 0 ? 1 : 0;
        }
    }
}

int get_contrast( void ) { return mod_status.hdw.lcd_contrast - 6; }

void init_emulator( config_t* conf )
{
    __config = *conf;

    EmulatorInit();

    conf->wire_name = ( char* )SerialInit();
}

void exit_emulator( void ) { EmulatorExit( SAVE_AND_EXIT ); }
