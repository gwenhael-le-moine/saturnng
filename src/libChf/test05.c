/* $Id: test05.c,v 2.1 2000/05/29 13:10:46 cibrario Rel $
   Chf test program.
   Condition & Handler stacks oveflow checks - single and multithreaded

   $Log: test05.c,v $
   Revision 2.1  2000/05/29 13:10:46  cibrario
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
#include "Chf.h"

#define H_STACK_SIZE 10
#define C_STACK_SIZE 30

/* Dummy handler; pushed only to verify that the handler stack overflow
   checks are correct.
*/
ChfAction h1( const ChfDescriptor* c, const ChfState s, ChfPointer p ) { return CHF_RESIGNAL; }

/* Overflow check handler; it unwinds if the CHF_F_HDLR_STACK_FULL
   condition is signalled exactly after H_STACK_SIZE-2 invocations
   of ChfPushHandler(), it resignals a modified condition if the
   condition is signalled too early
*/
ChfAction h2( const ChfDescriptor* c, const ChfState s, ChfPointer p )
{
    int push_count = *( ( int* )p );
    ChfAction action;

    if ( s == CHF_SIGNALING ) {
        if ( ChfGetModuleId( c ) == CHF_SET && ChfGetConditionCode( c ) == CHF_F_HDLR_STACK_FULL ) {
            /* Handler stack is full; check correctness of the descriptor */
            if ( push_count == H_STACK_SIZE - 2 && ChfGetNextDescriptor( c ) == NULL && ChfGetSeverity( c ) == CHF_FATAL )
                action = CHF_UNWIND;
            else {
                ChfCondition 11, CHF_FATAL, push_count, H_STACK_SIZE - 2 ChfEnd;
                action = CHF_RESIGNAL;
            }
        }
    }

    else
        action = CHF_RESIGNAL;

    return action;
}

/* Overflow check handler; it unwinds if the CHF_F_COND_STACK_FULL
   condition is signalled exactly after C_STACK_SIZE invocations
   of ChfCondition, it resignals a modified condition if the
   condition is signalled too early
*/
ChfAction h3( const ChfDescriptor* c, const ChfState s, ChfPointer p )
{
    int push_count = *( ( int* )p );
    ChfAction action;

    if ( s == CHF_SIGNALING ) {
        if ( ChfGetModuleId( c ) == CHF_SET && ChfGetConditionCode( c ) == CHF_F_COND_STACK_FULL ) {
            /* Handler stack is full; check correctness of the descriptor */
            if ( push_count == C_STACK_SIZE && ChfGetNextDescriptor( c ) == NULL && ChfGetSeverity( c ) == CHF_FATAL )
                action = CHF_UNWIND;
            else {
                ChfCondition 12, CHF_FATAL, push_count, C_STACK_SIZE ChfEnd;
                action = CHF_RESIGNAL;
            }
        }
    }

    else
        action = CHF_RESIGNAL;

    return action;
}

void* task( void* arg )
{
    int push_count = 0;
    sigjmp_buf jb;

    /* The sleep() is here to increase contention between threads */
    sleep( 1 );

    printf( "\tThread %d\n", ( int )arg );

    /* Check handler stack overflow checks */
    if ( sigsetjmp( jb, 1 ) == 0 ) {
        int i;

        /* Push the handler */
        ChfPushHandler( h2, jb, ( ChfPointer )( &push_count ) );

        /* The sleep() is here to increase contention between threads */
        sleep( 1 );

        /* Push dummy handlers until an error should occur */
        for ( ; push_count < H_STACK_SIZE - 1; push_count++ )
            ChfPushHandler( h1, NULL, NULL );

        /* No error? Bad! */
        return ( void* )EXIT_FAILURE;
    }

    /* Flow control returns here if 'handler stack full' was signalled
       at the correct place.
       Check condition stack overflow checks
    */
    push_count = 0;
    if ( sigsetjmp( jb, 1 ) == 0 ) {
        int i;

        /* Push the handler */
        ChfPushHandler( h3, jb, ( ChfPointer )( &push_count ) );

        /* The sleep() is here to increase contention between threads */
        sleep( 1 );

        /* Push dummy conditions until an error should occur */
        for ( ; push_count <= C_STACK_SIZE; push_count++ )
            ChfCondition 1, CHF_INFO ChfEnd;

        /* No error? Bad! */
        return ( void* )EXIT_FAILURE;
    }

    /* Flow control returns here if 'condition stack full' was signalled
       at the correct place.
       Check condition stack overflow again, to ensure that no spurious
       conditions were left out in the previous check.
    */
    push_count = 0;
    if ( sigsetjmp( jb, 1 ) == 0 ) {
        int i;

        /* Push the handler */
        ChfPushHandler( h3, jb, ( ChfPointer )( &push_count ) );

        /* The sleep() is here to increase contention between threads */
        sleep( 1 );

        /* Push dummy conditions until an error should occur */
        for ( ; push_count <= C_STACK_SIZE; push_count++ )
            ChfCondition 1, CHF_INFO ChfEnd;

        /* No error? Bad! */
        return ( void* )EXIT_FAILURE;
    }

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

    puts( "test05" );

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
