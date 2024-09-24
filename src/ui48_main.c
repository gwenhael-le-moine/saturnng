#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "ui48_config.h"
#include "ui48_emulator.h"
#include "ui48_common.h"

#include "config.h"
#include "cpu.h"
#include "monitor.h"

#define SPEED_HZ_UI 64

/* Chf condition codes (main program only) */

#define CHF_MODULE_ID MAIN_CHF_MODULE_ID
#include <Chf.h>

#define MAIN_M_COPYRIGHT 501
#define MAIN_M_LICENSE 502

/*---------------------------------------------------------------------------
   Chf parameters - Do not change.
  ---------------------------------------------------------------------------*/

#define CONDITION_STACK_SIZE 16
#define HANDLER_STACK_SIZE 8

/* Conditional prefix and mandatory suffix to make a message catalog
   name from argv[0]
*/
static const char cat_prefix[] = "./";
static const char cat_suffix[] = ".cat";

#define CAT_PREFIX_LEN ( sizeof( cat_prefix ) + 1 )
#define CAT_SUFFIX_LEN ( sizeof( cat_suffix ) + 1 )

static void adjust_setlocale( void )
{
    fprintf( stderr, "saturn-W-locale probably bad; reverting to C locale\n" );

    putenv( "LC_ALL=C" );
    putenv( "LC_COLLATE=C" );
    putenv( "LC_CTYPE=C" );
    putenv( "LC_MESSAGES=C" );
    putenv( "LC_MONETARY=C" );
    putenv( "LC_NUMERIC=C" );
    putenv( "LC_TIME=C" );
    putenv( "LANG=C" );
}

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
    char* cat_name;
    int st;
    int retry = 0;

    if ( ( cat_name = malloc( strlen( argv[ 0 ] ) + CAT_PREFIX_LEN + CAT_SUFFIX_LEN + 1 ) ) == NULL ) {
        fprintf( stderr, "saturn-E-cat_name initialization failed\n" );
        exit( EXIT_FAILURE );
    }

    /* Generate catalog name, without optional prefix */
    strcpy( cat_name, argv[ 0 ] );
    strcat( cat_name, cat_suffix );

    /* 3.15: Retry the initialization steps below two times; before trying
       the second time, adjust the setlocale() environment variables
       with adjust_setlocale()
    */
    while ( retry < 2 ) {
        /* Chf initialization with msgcat subsystem; notice that on
           some systems (e.g. Digital UNIX) catopen() can succeed even
           if it was not able to open the right message catalog;
           better try it now.
        */
        if ( ( st = ChfMsgcatInit( argv[ 0 ],            /* Application's name */
                                   CHF_DEFAULT,          /* Options */
                                   cat_name,             /* Name of the message catalog */
                                   CONDITION_STACK_SIZE, /* Size of the condition stack */
                                   HANDLER_STACK_SIZE,   /* Size of the handler stack */
                                   EXIT_FAILURE          /* Abnormal exit code */
                                   ) ) != CHF_S_OK ||
             ChfGetMessage( CHF_MODULE_ID, MAIN_M_COPYRIGHT, NULL ) == NULL )
            fprintf( stderr, "saturn-E-Primary Chf initialization failed (%d)\n", st );
        else
            break;

        /* if ( ( st = ChfStaticInit( argv[ 0 ],            /\* Application's name *\/ */
        /*                            CHF_DEFAULT,          /\* Options *\/ */
        /*                            [],             /\* Name of the message catalog *\/ */
        /*                            CONDITION_STACK_SIZE, /\* Size of the condition stack *\/ */
        /*                            HANDLER_STACK_SIZE,   /\* Size of the handler stack *\/ */
        /*                            EXIT_FAILURE          /\* Abnormal exit code *\/ */
        /*                            ) ) != CHF_S_OK || */
        /*      ChfGetMessage( CHF_MODULE_ID, MAIN_M_COPYRIGHT, NULL ) == NULL ) */
        /*     fprintf( stderr, "saturn-E-Primary Chf initialization failed (%d)\n", st ); */
        /* else */
        /*     break; */

        /* Bring down Chf before initializing it again */
        if ( st == CHF_S_OK )
            ChfExit();

        /* Try alternate message catalog name (with prefix) */
        strcpy( cat_name, cat_prefix );
        strcat( cat_name, argv[ 0 ] );
        strcat( cat_name, cat_suffix );

        if ( ( st = ChfMsgcatInit( argv[ 0 ],            /* Application's name */
                                   CHF_DEFAULT,          /* Options */
                                   cat_name,             /* Name of the message catalog */
                                   CONDITION_STACK_SIZE, /* Size of the condition stack */
                                   HANDLER_STACK_SIZE,   /* Size of the handler stack */
                                   EXIT_FAILURE          /* Abnormal exit code */
                                   ) ) != CHF_S_OK ||
             ChfGetMessage( CHF_MODULE_ID, MAIN_M_COPYRIGHT, NULL ) == NULL )
            fprintf( stderr, "saturn-E-Alternate Chf initialization failed (%d)\n", st );
        else
            break;

        /* Bring down Chf before initializing it again */
        if ( st == CHF_S_OK )
            ChfExit();

        /* Attempt to adjust setlocale() environment variables */
        if ( retry++ == 0 )
            adjust_setlocale();
    }

    if ( retry == 2 ) {
        fprintf( stderr, "saturn-F-Application aborted\n" );
        exit( EXIT_FAILURE );
    }

    /* cat_name no longer needed */
    free( cat_name );

    /* 3.9: Print out MAIN_M_COPYRIGHT and MAIN_M_LICENSE on stdout now */
    fprintf( stdout, ChfGetMessage( CHF_MODULE_ID, MAIN_M_COPYRIGHT, "" ), "$Revision: 4.1 $" );
    /* fprintf( stdout, ChfGetMessage( CHF_MODULE_ID, MAIN_M_LICENSE, "" ) ); */

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
    it.it_interval.tv_usec = 1000000 / SPEED_HZ_UI;
    it.it_value.tv_sec = 0;
    it.it_value.tv_usec = 1000000 / SPEED_HZ_UI;
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
