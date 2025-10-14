#ifndef _EMULATOR_H
#  define _EMULATOR_H 1

/* 2.1: EmulatorExit() option */
typedef enum {
    IMMEDIATE_EXIT,
    SAVE_AND_EXIT
} exit_option_t;

void Emulator( void );
void EmulatorIntRequest( void );
void EmulatorInit( void );              /* 2.1 */
void EmulatorExit( exit_option_t opt ); /* 2.1 */

#endif /*!_EMULATOR_H*/
