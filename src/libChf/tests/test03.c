/* $Id: test03.c,v 2.1 2000/05/29 13:10:29 cibrario Rel $
   Chf test program.
   Generation and signal - single and multithreaded

   $Log: test03.c,v $
   Revision 2.1  2000/05/29 13:10:29  cibrario
   *** empty log message ***

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
#include "../src/Chf.h"

void* task( void* arg )
{
    const char* msg;
    const ChfDescriptor *d, *e;

    /* The sleep() is here to increase contention between threads */
    sleep( 1 );

    printf( "\tThread %d\n", ( int )arg );

    /* Generate a condition group and signal it */
    CHF_Condition 6, CHF_INFO, ( int )arg ChfEnd;
    CHF_Condition 6, CHF_INFO, ( int )arg ChfEnd;
    CHF_Condition 6, CHF_INFO, ( int )arg ChfEnd;
    CHF_Condition 6, CHF_INFO, ( int )arg ChfEnd;
    CHF_Condition 7, CHF_INFO, ( int )arg ChfEnd;

    /* The sleep() is here to increase contention between threads */
    sleep( 1 );
    ChfSignal();

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

    puts( "test03" );

    /* Initialization */
    if ( st = ChfMsgcatInit( argv[ 0 ], CHF_DEFAULT, "./test01.cat", 50, 10, 1 ) )
        exit( st );

#ifdef _REENTRANT
    /* Create */
    for ( i = 0; i < N_THREADS; i++ )
        if ( pthread_create( &( t[ i ] ), NULL, task, ( void* )i ) ) {
            perror( "pthread_create" );
            exit( EXIT_FAILURE );
        }

    /* Join */
    for ( i = 0; i < N_THREADS; i++ )
        if ( pthread_join( t[ i ], &ret ) ) {
            perror( "pthread_join" );
            exit( EXIT_FAILURE );
        }
#else
    task( ( void* )0 );
#endif

    /* Exit Chf */
    ChfExit();
    exit( EXIT_SUCCESS );
}
