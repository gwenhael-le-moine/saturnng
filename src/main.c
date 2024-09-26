#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "ui4x_config.h"
#include "ui4x_emulator.h"
#include "ui4x_common.h"

#include "config.h"
#include "cpu.h"
#include "monitor.h"
#include "chf_messages.h"

#define UI_REFRESH_RATE_Hz 64

#define CONDITION_STACK_SIZE 16
#define HANDLER_STACK_SIZE 8

void signal_handler( int sig )
{
    switch ( sig ) {
        /* case SIGINT: /\* Ctrl-C *\/ */
        /*     enter_debugger |= USER_INTERRUPT; */
        /*     break; */
        case SIGALRM:
            ui_get_event();
            ui_update_display();
            break;
        case SIGPIPE:
            ui_stop();
            exit_emulator();
            exit( 0 );
        default:
            break;
    }
}

int main( int argc, char** argv )
{
    config = *config_init( argc, argv );

    /* Chf initialization with msgcat subsystem; notice that on
           some systems (e.g. Digital UNIX) catopen() can succeed even
           if it was not able to open the right message catalog;
           better try it now.
        */
    if ( ( ChfStaticInit( argv[ 0 ],            /* Application's name */
                          CHF_DEFAULT,          /* Options */
                          message_table,        /* Name of the message catalog */
                          message_table_size,   /* message catalog size */
                          CONDITION_STACK_SIZE, /* Size of the condition stack */
                          HANDLER_STACK_SIZE,   /* Size of the handler stack */
                          EXIT_FAILURE          /* Abnormal exit code */
                          ) ) != CHF_S_OK ) {
        fprintf( stderr, "saturn-E-Primary Chf initialization failed\n" );
        exit( EXIT_FAILURE );
    }

    /* 3.9: Print out MAIN_M_COPYRIGHT and MAIN_M_LICENSE on stdout now */
    fprintf( stdout,
             "saturn %i.%i.%i - A poor-man's emulator of HP48GX, HP49, HP39/40\nCopyright (C) 1998-2000 Ivan Cibrario "
             "Bertolotti\nCopyright (C) Gwenhael Le Moine\n",
             VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL );
    fprintf( stdout,
             "This program is free software, and comes with ABSOLUTELY NO WARRANTY;\nfor details see the accompanying documentation.\n\n" );

    init_emulator( &config );

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
