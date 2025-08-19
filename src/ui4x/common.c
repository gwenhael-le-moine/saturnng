#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "inner.h"
#if defined( HAS_SDL )
#  include "sdl.h"
#endif
#include "ncurses.h"

void ( *ui_get_event )( void );
void ( *ui_update_display )( void );
void ( *ui_start )( config_t* conf );
void ( *ui_stop )( void );

void setup_ui( config_t* conf )
{
    switch ( conf->frontend ) {
        case FRONTEND_NCURSES:
#if !defined( HAS_SDL )
        default:
#endif
            setup_frontend_ncurses();
            break;
#if defined( HAS_SDL )
        case FRONTEND_SDL:
        default:
            setup_frontend_sdl();
            break;
#endif
    }
}

void close_and_exit( void )
{
    exit_emulator();

    ui_stop();

    exit( 0 );
}
