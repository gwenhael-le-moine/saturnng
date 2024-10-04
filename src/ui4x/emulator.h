#ifndef _UI4x_EMULATOR_H
#define _UI4x_EMULATOR_H 1

#include <stdbool.h>

#include "config.h"

// HP 48{G,S}X Keys
#define HP48_KEY_A 0
#define HP48_KEY_B 1
#define HP48_KEY_C 2
#define HP48_KEY_D 3
#define HP48_KEY_E 4
#define HP48_KEY_F 5

#define HP48_KEY_MTH 6
#define HP48_KEY_PRG 7
#define HP48_KEY_CST 8
#define HP48_KEY_VAR 9
#define HP48_KEY_UP 10
#define HP48_KEY_NXT 11

#define HP48_KEY_QUOTE 12
#define HP48_KEY_STO 13
#define HP48_KEY_EVAL 14
#define HP48_KEY_LEFT 15
#define HP48_KEY_DOWN 16
#define HP48_KEY_RIGHT 17

#define HP48_KEY_SIN 18
#define HP48_KEY_COS 19
#define HP48_KEY_TAN 20
#define HP48_KEY_SQRT 21
#define HP48_KEY_POWER 22
#define HP48_KEY_INV 23

#define HP48_KEY_ENTER 24
#define HP48_KEY_NEG 25
#define HP48_KEY_EEX 26
#define HP48_KEY_DEL 27
#define HP48_KEY_BS 28

#define HP48_KEY_ALPHA 29
#define HP48_KEY_7 30
#define HP48_KEY_8 31
#define HP48_KEY_9 32
#define HP48_KEY_DIV 33

#define HP48_KEY_SHL 34
#define HP48_KEY_4 35
#define HP48_KEY_5 36
#define HP48_KEY_6 37
#define HP48_KEY_MUL 38

#define HP48_KEY_SHR 39
#define HP48_KEY_1 40
#define HP48_KEY_2 41
#define HP48_KEY_3 42
#define HP48_KEY_MINUS 43

#define HP48_KEY_ON 44
#define HP48_KEY_0 45
#define HP48_KEY_PERIOD 46
#define HP48_KEY_SPC 47
#define HP48_KEY_PLUS 48

#define FIRST_HP48_KEY HP48_KEY_A
#define LAST_HP48_KEY HP48_KEY_PLUS
#define NB_HP48_KEYS ( LAST_HP48_KEY + 1 )

// HP 4{0,9}G Keys
#define HP49_KEY_A 0
#define HP49_KEY_B 1
#define HP49_KEY_C 2
#define HP49_KEY_D 3
#define HP49_KEY_E 4
#define HP49_KEY_F 5

#define HP49_KEY_APPS 6
#define HP49_KEY_MODE 7
#define HP49_KEY_TOOL 8

#define HP49_KEY_VAR 9
#define HP49_KEY_STO 10
#define HP49_KEY_NXT 11

#define HP49_KEY_LEFT 12
#define HP49_KEY_UP 13
#define HP49_KEY_RIGHT 14
#define HP49_KEY_DOWN 15

#define HP49_KEY_HIST 16
#define HP49_KEY_CAT 17
#define HP49_KEY_EQW 18
#define HP49_KEY_SYMB 19
#define HP49_KEY_BS 20

#define HP49_KEY_POWER 21
#define HP49_KEY_SQRT 22
#define HP49_KEY_SIN 23
#define HP49_KEY_COS 24
#define HP49_KEY_TAN 25

#define HP49_KEY_EEX 26
#define HP49_KEY_NEG 27
#define HP49_KEY_X 28
#define HP49_KEY_INV 29
#define HP49_KEY_DIV 30

#define HP49_KEY_ALPHA 31
#define HP49_KEY_7 32
#define HP49_KEY_8 33
#define HP49_KEY_9 34
#define HP49_KEY_MUL 35

#define HP49_KEY_SHL 36
#define HP49_KEY_4 37
#define HP49_KEY_5 38
#define HP49_KEY_6 39
#define HP49_KEY_MINUS 40

#define HP49_KEY_SHR 41
#define HP49_KEY_1 42
#define HP49_KEY_2 43
#define HP49_KEY_3 44
#define HP49_KEY_PLUS 45

#define HP49_KEY_ON 46
#define HP49_KEY_0 47
#define HP49_KEY_PERIOD 48
#define HP49_KEY_SPC 49
#define HP49_KEY_ENTER 50

#define FIRST_HP49_KEY HP49_KEY_A
#define LAST_HP49_KEY HP49_KEY_ENTER
#define NB_HP49_KEYS ( LAST_HP49_KEY + 1 )

#define FIRST_HPKEY ( config.model == MODEL_48GX || config.model == MODEL_48SX ? FIRST_HP48_KEY : FIRST_HP49_KEY )
#define LAST_HPKEY ( config.model == MODEL_48GX || config.model == MODEL_48SX ? LAST_HP48_KEY : LAST_HP49_KEY )

#define KEYS_BUFFER_SIZE 9

// Annunciators
#define NB_ANNUNCIATORS 6

#define ANN_LEFT 0x81
#define ANN_RIGHT 0x82
#define ANN_ALPHA 0x84
#define ANN_BATTERY 0x88
#define ANN_BUSY 0x90
#define ANN_IO 0xa0

// LCD
#define NIBBLES_PER_ROW 34
#define LCD_WIDTH 131
#define LCD_HEIGHT ( config.big_screen ? 80 : 64 )

/*************************************************/
/* public API: if it's there it's used elsewhere */
/*************************************************/
extern void press_key( int hpkey );
extern void release_key( int hpkey );
extern bool is_key_pressed( int hpkey );

extern void init_emulator( config_t* conf );
extern void exit_emulator( void );

extern unsigned char get_annunciators( void );
extern bool get_display_state( void );
extern void get_lcd_buffer( int* target );
extern int get_contrast( void );

#endif /* !_UI4x_EMULATOR_H */
