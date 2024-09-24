/* $Id: test06.c,v 2.1 2000/05/29 13:10:55 cibrario Rel $
   Chf test program.
   Structured condition handling - single and multithreaded

   $Log: test06.c,v $
   Revision 2.1  2000/05/29 13:10:55  cibrario
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

#define H_STACK_SIZE 10
#define C_STACK_SIZE 30

void* task( void* arg )
{
    volatile int phase = 0;

    ChfTry
    {
        phase = 1;
        ChfCondition 20, CHF_SUCCESS ChfEnd;
        ChfSignal();

        phase = 2;
        ChfCondition 20, CHF_INFO ChfEnd;
        ChfSignal();

        phase = 3;
        ChfCondition 20, CHF_WARNING ChfEnd;
        ChfSignal();

        phase = 4;
        ChfCondition 20, CHF_ERROR ChfEnd;
        ChfSignal();

        phase = 5;
        ChfCondition 20, CHF_FATAL ChfEnd;
        ChfSignal();

        /* Should not be reached */
        return ( void* )EXIT_FAILURE;
    }
    ChfCatch
    {
        /* Catched an exception; check descriptor */
        const ChfDescriptor* d = ChfGetTopCondition();
        if ( d == NULL || ChfGetNextDescriptor( d ) != NULL || ChfGetModuleId( d ) != CHF_MODULE_ID || ChfGetConditionCode( d ) != 20 )
            return ( void* )EXIT_FAILURE;
    }
    ChfEndTry;

    /* Check that the condition stack actually is empty after catch */
    ChfTry { const volatile ChfDescriptor* e = ChfGetTopCondition(); }
    ChfCatch
    {
        const ChfDescriptor* d = ChfGetTopCondition();
        if ( d == NULL || ChfGetNextDescriptor( d ) != NULL || ChfGetModuleId( d ) != CHF_SET ||
             ChfGetConditionCode( d ) != CHF_F_BAD_STATE )
            return ( void* )EXIT_FAILURE;
    }
    ChfEndTry;

    return ( void* )EXIT_SUCCESS;
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

    puts( "test06" );

    /* Initialization */
    if ( st = ChfMsgcatInit( argv[ 0 ], CHF_DEFAULT, "./test01.cat", C_STACK_SIZE, H_STACK_SIZE, EXIT_FAILURE ) )
        exit( st );

#ifdef _REENTRANT
    /* Create */
    for ( i = 0; i < N_THREADS; i++ )
        if ( pthread_create( &( t[ i ] ), NULL, task, ( void* )i ) ) {
            perror( "pthread_create" );
            exit( EXIT_FAILURE );
        }

    /* Join */
    for ( i = 0; i < N_THREADS; i++ ) {
        if ( pthread_join( t[ i ], &ret ) ) {
            perror( "pthread_join" );
            exit( EXIT_FAILURE );
        } else if ( ( int )ret != EXIT_SUCCESS )
            exit( ( int )ret );
    }

    st = EXIT_SUCCESS;
#else
    st = ( int )task( ( void* )0 );
#endif

    /* Exit Chf */
    ChfExit();
    exit( st );
}
