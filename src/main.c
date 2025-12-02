#include <locale.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "emulator_api.h"
#include "options.h"

#include "ui4x/api.h"

#include "core/chf_wrapper.h"
#include "core/cpu.h"
#include "core/emulator.h"
#include "core/monitor.h"

#define UI_REFRESH_RATE_Hz 64

#define CONDITION_STACK_SIZE 16
#define HANDLER_STACK_SIZE 8

#define QUERY_EVENTS_EVERY_X_FRAME 4

config_t config;

void signal_handler( int sig )
{
    static int nb_refreshes_since_last_checking_events = 0;

    switch ( sig ) {
        /* case SIGINT: /\* Ctrl-C *\/ */
        /*     enter_debugger |= USER_INTERRUPT; */
        /*     break; */
        case SIGALRM:
            if ( nb_refreshes_since_last_checking_events > QUERY_EVENTS_EVERY_X_FRAME ) {
                nb_refreshes_since_last_checking_events = 0;
                ui_handle_pending_inputs();
            }

            ui_refresh_output();

            nb_refreshes_since_last_checking_events++;
            break;

        case SIGPIPE:
            exit_ui();
            exit_emulator();
            exit( EXIT_SUCCESS );
        default:
            break;
    }
}

int main( int argc, char** argv )
{
    setlocale( LC_ALL, "C" );

    config = *config_init( argc, argv );

    /* Chf initialization */
    int ret = ChfStaticInit( MAIN_CHF_MODULE_ID, config.progname, CHF_DEFAULT, message_table, message_table_size, CONDITION_STACK_SIZE,
                             HANDLER_STACK_SIZE, EXIT_FAILURE );
    if ( ret != CHF_S_OK ) {
        fprintf( stderr, "saturn-E-Primary Chf initialization failed\n" );
        exit( EXIT_FAILURE );
    }

    /* 3.9: Print out MAIN_M_COPYRIGHT and MAIN_M_LICENSE on stdout now */
    fprintf( stdout,
             "%s %i.%i.%i - A poor-man's emulator of HP48GX/SX, HP49, HP39/40\nCopyright (C) 1998-2000 Ivan Cibrario "
             "Bertolotti\nCopyright (C) Gwenhael Le Moine\n",
             config.progname, VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL );
    fprintf( stdout,
             "This program is free software, and comes with ABSOLUTELY NO WARRANTY;\nfor details see the accompanying documentation.\n\n" );

    init_emulator( &config );
    set_speed( config.speed );

    /* (G)UI */
    ui4x_config_t config_ui = {
        .model = config.model,
        .shiftless = config.shiftless,
        .black_lcd = config.black_lcd,
        .newrpl_keyboard = false,

        .frontend = config.frontend,

        .mono = config.mono,
        .gray = config.gray,

        .chromeless = config.chromeless,
        .fullscreen = config.fullscreen,

        .tiny = config.tiny,
        .small = config.small,

        .verbose = config.verbose,

        .zoom = config.zoom,
        .netbook = false, /* FIXME */
        .netbook_pivot_line = 3,

        .name = config.progname,
        .progname = config.progname,
        .progpath = NULL,

        .wire_name = config.wire_name,
        .ir_name = config.ir_name,

        .datadir = NULL,
        .style_filename = config.style_filename == NULL ? NULL : path_file_in_datadir( config.style_filename ),

        .sd_dir = NULL,
    };

    ui4x_emulator_api_t emulator_api = {
        .press_key = press_key,
        .release_key = release_key,
        .is_key_pressed = is_key_pressed,
        .is_display_on = get_display_state,
        .get_annunciators = get_annunciators,
        .get_lcd_buffer = get_lcd_buffer,
        .get_contrast = get_contrast,
        .do_mount_sd = NULL,
        .do_unmount_sd = NULL,
        .is_sd_mounted = NULL,
        .get_sd_path = NULL,
        .do_reset = NULL,
        .do_stop = exit_emulator,
        .do_sleep = NULL,
        .do_wake = NULL,
        .do_debug = NULL,
    };
    init_ui( &config_ui, &emulator_api );

    sigset_t set;
    struct sigaction sa;
    sigemptyset( &set );
    sigaddset( &set, SIGALRM );
    sa.sa_handler = signal_handler;
    sa.sa_mask = set;
#ifdef SA_RESTART
    sa.sa_flags = SA_RESTART;
#endif
    sigaction( SIGALRM, &sa, ( struct sigaction* )0 );

    /************************************/
    /* set the real time interval timer */
    /************************************/
    /*
      Every <interval>µs setitimer will trigger a SIGALRM
      which will getUI events and refresh UI in signal_handler
     */
    struct itimerval it;
    it.it_interval.tv_sec = 0;
    it.it_interval.tv_usec = 1000000 / UI_REFRESH_RATE_Hz;
    it.it_value.tv_sec = it.it_interval.tv_sec;
    it.it_value.tv_usec = it.it_interval.tv_usec;
    setitimer( ITIMER_REAL, &it, ( struct itimerval* )0 );

    if ( config.monitor )
        /* Invoke Monitor */
        Monitor();
    else
        /* Call Emulator directly */
        Emulator();

    /* never reached */
    return EXIT_SUCCESS;
}
