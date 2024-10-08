/* $Id: test04.c,v 2.1 2000/05/29 13:10:38 cibrario Rel $
   Chf test program.
   General condition handling - single and multithreaded

   $Log: test04.c,v $
   Revision 2.1  2000/05/29 13:10:38  cibrario
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

struct tdata_s {
    const ChfDescriptor *d, *e;
    int phase;
};

ChfAction h1( const ChfDescriptor* c, const ChfState s, ChfPointer p )
{
    struct tdata_s* tdata_p = ( struct tdata_s* )p;
    ChfAction action;

    if ( c != tdata_p->e || ChfGetNextDescriptor( c ) != tdata_p->d ) {
        CHF_Condition 10, CHF_FATAL ChfEnd;
        action = CHF_RESIGNAL;
    }

    else
        action = CHF_CONTINUE;

    return action;
}

ChfAction h2( const ChfDescriptor* c, const ChfState s, ChfPointer p )
{
    struct tdata_s* tdata_p = ( struct tdata_s* )p;
    ChfAction action;

    switch ( s ) {
        case CHF_SIGNALING:
            {
                if ( c != tdata_p->e || ChfGetNextDescriptor( c ) != tdata_p->d || ( tdata_p->phase != 2 && tdata_p->phase != 4 ) ) {
                    CHF_Condition 10, CHF_FATAL ChfEnd;
                    action = CHF_RESIGNAL;
                }

                else {
                    action = ( ChfGetConditionCode( c ) != 8 ? CHF_CONTINUE : CHF_UNWIND );
                }
                break;
            }
        case CHF_UNWINDING:
            {
                if ( tdata_p->phase != 4 )
                    exit( EXIT_FAILURE );
                tdata_p->phase = 5;
                action = CHF_CONTINUE;
                break;
            }
        default:
            {
                exit( EXIT_FAILURE );
            }
    }
    return action;
}

ChfAction h3( const ChfDescriptor* c, const ChfState s, ChfPointer p )
{
    struct tdata_s* tdata_p = ( struct tdata_s* )p;
    ChfAction action;

    /* This handler must be invoked only during the first signal */
    if ( tdata_p->phase != 3 )
        exit( EXIT_FAILURE );

    switch ( s ) {
        case CHF_SIGNALING:
            {
                if ( ChfGetConditionCode( c ) != 9 || ChfGetNextDescriptor( c ) != NULL ) {
                    exit( EXIT_FAILURE );
                }

                else {
                    tdata_p->phase = 4;
                    action = CHF_CONTINUE;
                }
                break;
            }
        default:
            {
                exit( EXIT_FAILURE );
            }
    }
    return action;
}

ChfAction h4( const ChfDescriptor* c, const ChfState s, ChfPointer p )
{
    struct tdata_s* tdata_p = ( struct tdata_s* )p;
    ChfAction action;

    /* This handler must be invoked only during the first signal */
    if ( tdata_p->phase != 2 )
        exit( EXIT_FAILURE );

    switch ( s ) {
        case CHF_SIGNALING:
            {
                if ( c != tdata_p->e || ChfGetNextDescriptor( c ) != tdata_p->d ) {
                    CHF_Condition 10, CHF_FATAL ChfEnd;
                    action = CHF_RESIGNAL;
                }

                else {
                    /* This generates a new group and signals it */
                    tdata_p->phase = 3;
                    CHF_Condition 9, CHF_INFO ChfEnd;
                    ChfSignal();

                    if ( tdata_p->phase != 4 )
                        exit( EXIT_FAILURE );
                    tdata_p->phase = 5;

                    if ( c != tdata_p->e || ChfGetNextDescriptor( c ) != tdata_p->d ) {
                        CHF_Condition 10, CHF_FATAL ChfEnd;
                        action = CHF_RESIGNAL;
                    } else
                        action = CHF_CONTINUE;
                }
                break;
            }
        default:
            {
                exit( EXIT_FAILURE );
            }
    }
    return action;
}

void* task( void* arg )
{
    volatile struct tdata_s tdata;

    /* The sleep() is here to increase contention between threads */
    sleep( 1 );

    printf( "\tThread %d\n", ( int )arg );

    /* Push the handler */
    ChfPushHandler( h1, NULL, ( ChfPointer )( &tdata ) );

    /* Generate a condition group and signal it */
    CHF_Condition 6, CHF_INFO, ( int )arg ChfEnd;
    tdata.d = ChfGetTopCondition();
    CHF_Condition 7, CHF_INFO, ( int )arg ChfEnd;
    tdata.e = ChfGetTopCondition();

    /* The sleep() is here to increase contention between threads */
    sleep( 1 );
    ChfSignal();

    /* Pop the handler */
    ChfPopHandler();

    /* Generate a new condition group with (apparently) wrong linkage
       and signal it; this checks that the handler has actually been
       removed.
    */
    CHF_Condition 6, CHF_INFO, ( int )arg ChfEnd;
    tdata.d = NULL;
    CHF_Condition 7, CHF_INFO, ( int )arg ChfEnd;
    tdata.e = NULL;
    ChfSignal();

    /* Conditional unwind test */
    {
        sigjmp_buf jb;

        tdata.phase = 0;
        if ( setjmp( jb ) == 0 ) {
            ChfPushHandler( h2, jb, ( ChfPointer )( &tdata ) );

            /* Generate a condition group and signal it */
            tdata.phase = 1;
            CHF_Condition 6, CHF_INFO, ( int )arg ChfEnd;
            tdata.d = ChfGetTopCondition();
            CHF_Condition 7, CHF_INFO, ( int )arg ChfEnd;
            tdata.e = ChfGetTopCondition();

            /* This does not trigger an unwind */
            tdata.phase = 2;
            ChfSignal();

            tdata.phase = 3;
            CHF_Condition 6, CHF_INFO, ( int )arg ChfEnd;
            tdata.d = ChfGetTopCondition();
            CHF_Condition 8, CHF_INFO, ( int )arg ChfEnd;
            tdata.e = ChfGetTopCondition();

            /* This MUST trigger an unwind */
            tdata.phase = 4;
            ChfSignal();

            exit( EXIT_FAILURE );

        } else {
            /* Unwind */
            if ( tdata.phase != 5 )
                exit( EXIT_FAILURE );

            ChfPopHandler();
        }
    }

    /* Condition generation and signal while a signal is in progress;
       this requires two handlers.
    */
    {
        tdata.phase = 0;

        ChfPushHandler( h3, NULL, ( ChfPointer )&tdata );
        ChfPushHandler( h4, NULL, ( ChfPointer )&tdata );

        tdata.phase = 1;
        CHF_Condition 6, CHF_INFO, ( int )arg ChfEnd;
        tdata.d = ChfGetTopCondition();
        CHF_Condition 7, CHF_INFO, ( int )arg ChfEnd;
        tdata.e = ChfGetTopCondition();

        tdata.phase = 2;
        ChfSignal();

        if ( tdata.phase != 5 )
            exit( EXIT_FAILURE );

        ChfPopHandler();
        ChfPopHandler();
    }

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

    puts( "test04" );

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
