#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "inner.h"
#include "sdl.h"
#include "ncurses.h"

void ( *ui_get_event )( void );
void ( *ui_update_display )( void );
void ( *ui_start )( config_t* conf );
void ( *ui_stop )( void );

void setup_ui( config_t* conf )
{
    switch ( conf->frontend ) {
        case FRONTEND_NCURSES:
            setup_frontend_ncurses();
            break;
        case FRONTEND_SDL:
        default:
            setup_frontend_sdl();
            break;
    }
}

void close_and_exit( void )
{
    exit_emulator();

    ui_stop();

    exit( 0 );
}
