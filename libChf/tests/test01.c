/* $Id: test01.c,v 2.1 2000/05/29 13:55:50 cibrario Rel $
   Chf test program.
   Simple initialization.


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

int main( int argc, char* argv[] )
{
    int st;
    const char* msg;
    const ChfDescriptor *d, *e;

    puts( "test01" );

    system( "gencat test01.cat test01.msf" );
    system( "gencat test01.cat chf.msf" );

    /* Initialization */
    if ( st = ChfMsgcatInit( argv[ 0 ], CHF_DEFAULT, "./test01.cat", 50, 10, 1 ) )
        exit( st );

    /* ChfGetMessage:
       message (CHF_MODULE_ID, 1) exists, (CHF_MODULE_ID, 2) does not
     */
    msg = ChfGetMessage( CHF_MODULE_ID, 1, "Default_1" );
    if ( strcmp( msg, "Set_255,Message_1" ) )
        exit( EXIT_FAILURE );
    msg = ChfGetMessage( CHF_MODULE_ID, 2, "Default_2" );
    if ( strcmp( msg, "Default_2" ) )
        exit( EXIT_FAILURE );

    /* Generate a condition and check descriptor; this is line 46 */
    ChfCondition 3, CHF_WARNING, 456 ChfEnd;

    if ( ( d = ChfGetTopCondition() ) == NULL )
        exit( EXIT_FAILURE );
    if ( d->module_id != CHF_MODULE_ID || d->condition_code != 3 || d->severity != CHF_WARNING || d->line_number != 46 ||
         strcmp( d->file_name, "test01.c" ) || strcmp( d->message, "Set_255,Arg_456,Message_3" ) || d->next != NULL )
        exit( EXIT_FAILURE );

    /* Generate another condition and check; this is line 60 */
    ChfCondition 4, CHF_INFO, "arg" ChfEnd;

    if ( ( e = ChfGetTopCondition() ) == NULL )
        exit( EXIT_FAILURE );
    if ( e->module_id != CHF_MODULE_ID || e->condition_code != 4 || e->severity != CHF_INFO || e->line_number != 60 ||
         strcmp( e->file_name, "test01.c" ) || strcmp( e->message, "Set_255,Arg_arg,Message_4" ) || e->next != d )
        exit( EXIT_FAILURE );

    /* Discard the previous condition group and create a new one */
    ChfDiscard();

    /* This is line 77 */
    ChfCondition 5, CHF_ERROR, 456, 789 ChfEnd;

    if ( ( d = ChfGetTopCondition() ) == NULL )
        exit( EXIT_FAILURE );
    if ( d->module_id != CHF_MODULE_ID || d->condition_code != 5 || d->severity != CHF_ERROR || d->line_number != 77 ||
         strcmp( d->file_name, "test01.c" ) || strcmp( d->message, "Set_255,Arg_456-789,Message_5" ) || d->next != NULL )
        exit( EXIT_FAILURE );

    /* Exit Chf */
    ChfExit();
    exit( EXIT_SUCCESS );
}
