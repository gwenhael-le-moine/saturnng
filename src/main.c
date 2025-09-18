#include <locale.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "options.h"

#include "ui4x/api.h"
#include "ui4x/common.h"

#include "core/chf_wrapper.h"
#include "core/emulator.h"
#include "core/monitor.h"
#include "core/x_func.h"

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
                ui_get_event();
            }

            ui_update_display();

            nb_refreshes_since_last_checking_events++;
            break;

        case SIGPIPE:
            ui_stop();
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

    /* Chf initialization with msgcat subsystem
     */
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
    setup_ui( &config );
    ui_start( &config );

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
