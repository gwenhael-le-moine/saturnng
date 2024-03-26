/* $Id: test02.c,v 2.1 2000/05/29 13:56:44 cibrario Rel $
   Chf test program.
   Simple initialization - multithreaded.


*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <setjmp.h>

#ifdef _REENTRANT
#  include <pthread.h>
#endif

#define CHF_MODULE_ID 255
#define CHF_EXTENDED_INFO
#include "Chf.h"

void* task( void* arg )
{
    const char* msg;
    const ChfDescriptor *d, *e;

    printf( "\tThread %d\n", ( int )arg );

    /*   message (CHF_MODULE_ID, 1) exists, (CHF_MODULE_ID, 2) does not */
    msg = ChfGetMessage( CHF_MODULE_ID, 1, "Default_1" );
    if ( strcmp( msg, "Set_255,Message_1" ) )
        exit( EXIT_FAILURE );
    msg = ChfGetMessage( CHF_MODULE_ID, 2, "Default_2" );
    if ( strcmp( msg, "Default_2" ) )
        exit( EXIT_FAILURE );

    /* Generate a condition and check descriptor; this is line 36 */
    ChfCondition 3, CHF_WARNING, 456 ChfEnd;

    if ( ( d = ChfGetTopCondition() ) == NULL )
        exit( EXIT_FAILURE );
    if ( d->module_id != CHF_MODULE_ID || d->condition_code != 3 || d->severity != CHF_WARNING || d->line_number != 36 ||
         strcmp( d->file_name, "test02.c" ) || strcmp( d->message, "Set_255,Arg_456,Message_3" ) || d->next != NULL )
        exit( EXIT_FAILURE );

    /* Generate another condition and check; this is line 50 */
    ChfCondition 4, CHF_INFO, "arg" ChfEnd;

    if ( ( e = ChfGetTopCondition() ) == NULL )
        exit( EXIT_FAILURE );
    if ( e->module_id != CHF_MODULE_ID || e->condition_code != 4 || e->severity != CHF_INFO || e->line_number != 50 ||
         strcmp( e->file_name, "test02.c" ) || strcmp( e->message, "Set_255,Arg_arg,Message_4" ) || e->next != d )
        exit( EXIT_FAILURE );

    /* Discard the previous condition group and create a new one */
    ChfDiscard();

    /* This is line 67 */
    ChfCondition 5, CHF_ERROR, 456, 789 ChfEnd;

    if ( ( d = ChfGetTopCondition() ) == NULL )
        exit( EXIT_FAILURE );
    if ( d->module_id != CHF_MODULE_ID || d->condition_code != 5 || d->severity != CHF_ERROR || d->line_number != 67 ||
         strcmp( d->file_name, "test02.c" ) || strcmp( d->message, "Set_255,Arg_456-789,Message_5" ) || d->next != NULL )
        exit( EXIT_FAILURE );

    return ( void* )0;
}

#define N_THREADS 50

int main( int argc, char* argv[] )
{
    int st;
    int i;
    void* ret;
#ifdef _REENTRANT
    pthread_t t[ N_THREADS ];
#endif

    puts( "test02" );

#ifdef _REENTRANT
    /* Initialization */
    if ( st = ChfMsgcatInit( argv[ 0 ], CHF_DEFAULT, "./test01.cat", 50, 10, 1 ) )
        exit( st );

    /* Create */
    for ( i = 0; i < N_THREADS; i++ )
        if ( st = pthread_create( &( t[ i ] ), NULL, task, ( void* )i ) ) {
            printf( "pthread_create: error %d", st );
            exit( EXIT_FAILURE );
        }

    /* Join */
    for ( i = 0; i < N_THREADS; i++ )
        if ( st = pthread_join( t[ i ], &ret ) ) {
            printf( "pthread_join: error %d", st );
            exit( EXIT_FAILURE );
        }

    /* Exit Chf */
    ChfExit();
#endif
    exit( EXIT_SUCCESS );
}
