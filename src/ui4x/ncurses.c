#include <ctype.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <locale.h>
#include <wchar.h>

#include <curses.h>

#include "config.h"
#include "emulator.h"
#include "common.h"
#include "inner.h"

#define COLORS ( config.model == MODEL_48GX ? colors_48gx : ( config.model == MODEL_49G ? colors_49g : colors_48sx ) )
#define BUTTONS ( config.model == MODEL_48GX ? buttons_48gx : ( config.model == MODEL_49G ? buttons_49g : buttons_48sx ) )

#define LCD_OFFSET_X 1
#define LCD_OFFSET_Y 1
#define LCD_BOTTOM LCD_OFFSET_Y + ( config.small ? ( LCD_HEIGHT / 2 ) : config.tiny ? ( LCD_HEIGHT / 4 ) : LCD_HEIGHT )
#define LCD_RIGHT LCD_OFFSET_X + ( ( config.small || config.tiny ) ? ( LCD_WIDTH / 2 ) + 1 : LCD_WIDTH )

#define LCD_COLOR_BG 48
#define LCD_COLOR_FG 49

#define LCD_PIXEL_ON 1
#define LCD_PIXEL_OFF 2
#define LCD_COLORS_PAIR 3

/*************/
/* variables */
/*************/
static config_t config;
static int lcd_pixels_buffer[ LCD_WIDTH * 80 ];
static int last_annunciators = -1;
// static int last_contrast = -1;

static bool keyboard_state[ NB_HP49_KEYS ];

/****************************/
/* functions implementation */
/****************************/
static inline wchar_t eight_bits_to_braille_char( bool b1, bool b2, bool b3, bool b4, bool b5, bool b6, bool b7, bool b8 )
{
    /*********/
    /* b1 b4 */
    /* b2 b5 */
    /* b3 b6 */
    /* b7 b8 */
    /*********/
    wchar_t chr = 0x2800;

    if ( b1 )
        chr |= 1; // 0b0000000000000001;
    if ( b2 )
        chr |= 2; // 0b0000000000000010;
    if ( b3 )
        chr |= 4; // 0b0000000000000100;
    if ( b4 )
        chr |= 8; // 0b0000000000001000;
    if ( b5 )
        chr |= 16; // 0b0000000000010000;
    if ( b6 )
        chr |= 32; // 0b0000000000100000;
    if ( b7 )
        chr |= 64; // 0b0000000001000000;
    if ( b8 )
        chr |= 128; // 0b0000000010000000;

    return chr;
}

static inline void ncurses_draw_lcd_tiny( void )
{
    bool b1, b2, b3, b4, b5, b6, b7, b8;
    int step_x = 2;
    int step_y = 4;

    wchar_t line[ 66 ]; /* ( LCD_WIDTH / step_x ) + 1 */

    if ( !config.mono && has_colors() )
        attron( COLOR_PAIR( LCD_COLORS_PAIR ) );

    for ( int y = 0; y < LCD_HEIGHT; y += step_y ) {
        wcscpy( line, L"" );

        for ( int x = 0; x < LCD_WIDTH; x += step_x ) {
            b1 = lcd_pixels_buffer[ ( y * LCD_WIDTH ) + x ];
            b4 = lcd_pixels_buffer[ ( y * LCD_WIDTH ) + x + 1 ];
            b2 = lcd_pixels_buffer[ ( ( y + 1 ) * LCD_WIDTH ) + x ];
            b5 = lcd_pixels_buffer[ ( ( y + 1 ) * LCD_WIDTH ) + x + 1 ];
            b3 = lcd_pixels_buffer[ ( ( y + 2 ) * LCD_WIDTH ) + x ];
            b6 = lcd_pixels_buffer[ ( ( y + 2 ) * LCD_WIDTH ) + x + 1 ];
            b7 = lcd_pixels_buffer[ ( ( y + 3 ) * LCD_WIDTH ) + x ];
            b8 = lcd_pixels_buffer[ ( ( y + 3 ) * LCD_WIDTH ) + x + 1 ];

            wchar_t pixels = eight_bits_to_braille_char( b1, b2, b3, b4, b5, b6, b7, b8 );
            wcsncat( line, &pixels, 1 );
        }
        mvaddwstr( LCD_OFFSET_Y + ( y / step_y ), LCD_OFFSET_X, line );
    }

    if ( !config.mono && has_colors() )
        attroff( COLOR_PAIR( LCD_COLORS_PAIR ) );

    wrefresh( stdscr );
}

static inline wchar_t four_bits_to_quadrant_char( bool top_left, bool top_right, bool bottom_left, bool bottom_right )
{
    if ( top_left ) {
        if ( top_right ) {
            if ( bottom_left )
                return bottom_right ? L'█' : L'▛'; /* 0x2588 0x2598 */
            else
                return bottom_right ? L'▜' : L'▀'; /* 0x259C 0x2580 */
        } else {
            if ( bottom_left )
                return bottom_right ? L'▙' : L'▌';
            else
                return bottom_right ? L'▚' : L'▘';
        }
    } else {
        if ( top_right ) {
            if ( bottom_left )
                return bottom_right ? L'▟' : L'▞';
            else
                return bottom_right ? L'▐' : L'▝';
        } else {
            if ( bottom_left )
                return bottom_right ? L'▄' : L'▖';
            else
                return bottom_right ? L'▗' : L' ';
        }
    }
}

static inline void ncurses_draw_lcd_small( void )
{
    bool top_left, top_right, bottom_left, bottom_right;
    int step_x = 2;
    int step_y = 2;

    wchar_t line[ 66 ]; /* ( LCD_WIDTH / step_x ) + 1 */

    if ( !config.mono && has_colors() )
        attron( COLOR_PAIR( LCD_COLORS_PAIR ) );

    for ( int y = 0; y < LCD_HEIGHT; y += step_y ) {
        wcscpy( line, L"" );

        for ( int x = 0; x < LCD_WIDTH; x += step_x ) {
            top_left = lcd_pixels_buffer[ ( y * LCD_WIDTH ) + x ];
            top_right = lcd_pixels_buffer[ ( y * LCD_WIDTH ) + x + 1 ];
            bottom_left = lcd_pixels_buffer[ ( ( y + 1 ) * LCD_WIDTH ) + x ];
            bottom_right = lcd_pixels_buffer[ ( ( y + 1 ) * LCD_WIDTH ) + x + 1 ];

            wchar_t pixels = four_bits_to_quadrant_char( top_left, top_right, bottom_left, bottom_right );
            wcsncat( line, &pixels, 1 );
        }
        mvaddwstr( LCD_OFFSET_Y + ( y / step_y ), LCD_OFFSET_X, line );
    }

    if ( !config.mono && has_colors() )
        attroff( COLOR_PAIR( LCD_COLORS_PAIR ) );

    wrefresh( stdscr );
}

static inline void ncurses_draw_lcd_fullsize( void )
{
    bool bit;

    wchar_t line[ LCD_WIDTH ];

    if ( !config.mono && has_colors() )
        attron( COLOR_PAIR( LCD_COLORS_PAIR ) );

    for ( int y = 0; y < LCD_HEIGHT; ++y ) {
        wcscpy( line, L"" );

        for ( int x = 0; x < LCD_WIDTH; ++x ) {
            bit = lcd_pixels_buffer[ ( y * LCD_WIDTH ) + x ];

            wchar_t pixel = bit ? L'█' : L' ';
            wcsncat( line, &pixel, 1 );
        }
        mvaddwstr( LCD_OFFSET_Y + y, LCD_OFFSET_X, line );
    }

    if ( !config.mono && has_colors() )
        attroff( COLOR_PAIR( LCD_COLORS_PAIR ) );

    wrefresh( stdscr );
}

static inline void ncurses_draw_lcd( void )
{
    if ( config.tiny )
        ncurses_draw_lcd_tiny();
    else if ( config.small )
        ncurses_draw_lcd_small();
    else
        ncurses_draw_lcd_fullsize();
}

static void ui_init_LCD( void ) { memset( lcd_pixels_buffer, 0, sizeof( lcd_pixels_buffer ) ); }

static void ncurses_update_annunciators( void )
{
    const wchar_t* annunciators_icons[ 6 ] = { L"↰", L"↱", L"α", L"🪫", L"⌛", L"⇄" };
    const int annunciators_bits[ NB_ANNUNCIATORS ] = { ANN_LEFT, ANN_RIGHT, ANN_ALPHA, ANN_BATTERY, ANN_BUSY, ANN_IO };

    int annunciators = get_annunciators();

    if ( last_annunciators == annunciators )
        return;

    last_annunciators = annunciators;

    for ( int i = 0; i < NB_ANNUNCIATORS; i++ )
        mvaddwstr( 0, 4 + ( i * 4 ),
                   ( ( annunciators_bits[ i ] & annunciators ) == annunciators_bits[ i ] ) ? annunciators_icons[ i ] : L" " );
}

/**********/
/* public */
/**********/
void ui_update_display_ncurses( void )
{
    // apply_contrast();

    if ( get_display_state() )
        get_lcd_buffer( lcd_pixels_buffer );
    else
        ui_init_LCD();

    ncurses_update_annunciators();
    ncurses_draw_lcd();
}

void ui_get_event_ncurses( void )
{
    bool new_keyboard_state[ NB_HP49_KEYS ];
    uint32_t k;

    for ( int key = FIRST_HPKEY; key <= LAST_HPKEY; key++ )
        new_keyboard_state[ key ] = false;

    /* Iterate over all currently pressed keys and mark them as pressed */
    while ( ( k = getch() ) ) {
        if ( k == ( uint32_t )ERR )
            break;

        switch ( k ) {
            case '0':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_0 : HP48_KEY_0 ) ] = true;
                break;
            case '1':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_1 : HP48_KEY_1 ) ] = true;
                break;
            case '2':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_2 : HP48_KEY_2 ) ] = true;
                break;
            case '3':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_3 : HP48_KEY_3 ) ] = true;
                break;
            case '4':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_4 : HP48_KEY_4 ) ] = true;
                break;
            case '5':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_5 : HP48_KEY_5 ) ] = true;
                break;
            case '6':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_6 : HP48_KEY_6 ) ] = true;
                break;
            case '7':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_7 : HP48_KEY_7 ) ] = true;
                break;
            case '8':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_8 : HP48_KEY_8 ) ] = true;
                break;
            case '9':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_9 : HP48_KEY_9 ) ] = true;
                break;
            case 'a':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_A : HP48_KEY_A ) ] = true;
                break;
            case 'b':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_B : HP48_KEY_B ) ] = true;
                break;
            case 'c':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_C : HP48_KEY_C ) ] = true;
                break;
            case 'd':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_D : HP48_KEY_D ) ] = true;
                break;
            case 'e':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_E : HP48_KEY_E ) ] = true;
                break;
            case 'f':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_F : HP48_KEY_F ) ] = true;
                break;
            case 'g':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_APPS : HP48_KEY_MTH ) ] = true;
                break;
            case 'h':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_MODE : HP48_KEY_PRG ) ] = true;
                break;
            case 'i':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_TOOL : HP48_KEY_CST ) ] = true;
                break;
            case 'j':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_VAR : HP48_KEY_VAR ) ] = true;
                break;
            case 'k':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_STO : HP48_KEY_UP ) ] = true;
                break;
            case KEY_UP:
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_UP : HP48_KEY_UP ) ] = true;
                break;
            case 'l':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_NXT : HP48_KEY_NXT ) ] = true;
                break;
            case 'm':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_HIST : HP48_KEY_QUOTE ) ] = true;
                break;
            case 'n':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_CAT : HP48_KEY_STO ) ] = true;
                break;
            case 'o':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_EQW : HP48_KEY_EVAL ) ] = true;
                break;
            case 'p':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_SYMB : HP48_KEY_LEFT ) ] = true;
                break;
            case KEY_LEFT:
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_LEFT : HP48_KEY_LEFT ) ] = true;
                break;
            case 'q':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_POWER : HP48_KEY_DOWN ) ] = true;
                break;
            case KEY_DOWN:
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_DOWN : HP48_KEY_DOWN ) ] = true;
                break;
            case 'r':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_SQRT : HP48_KEY_RIGHT ) ] = true;
                break;
            case KEY_RIGHT:
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_RIGHT : HP48_KEY_RIGHT ) ] = true;
                break;
            case 's':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_SIN : HP48_KEY_SIN ) ] = true;
                break;
            case 't':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_COS : HP48_KEY_COS ) ] = true;
                break;
            case 'u':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_TAN : HP48_KEY_TAN ) ] = true;
                break;
            case 'v':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_EEX : HP48_KEY_SQRT ) ] = true;
                break;
            case 'w':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_NEG : HP48_KEY_POWER ) ] = true;
                break;
            case 'x':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_X : HP48_KEY_INV ) ] = true;
                break;
            case 'y':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_INV : HP48_KEY_NEG ) ] = true;
                break;
            case 'z':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_DIV : HP48_KEY_EEX ) ] = true;
                break;
            case ' ':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_SPC : HP48_KEY_SPC ) ] = true;
                break;
            case KEY_DC:
                new_keyboard_state[ ( config.model == MODEL_49G ? -1 : HP48_KEY_DEL ) ] = true;
                break;
            case '.':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_PERIOD : HP48_KEY_PERIOD ) ] = true;
                break;
            case '+':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_PLUS : HP48_KEY_PLUS ) ] = true;
                break;
            case '-':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_MINUS : HP48_KEY_MINUS ) ] = true;
                break;
            case '*':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_MUL : HP48_KEY_MUL ) ] = true;
                break;
            case '/':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_DIV : HP48_KEY_DIV ) ] = true;
                break;

            case KEY_F( 1 ):
            case KEY_ENTER:
            case '\n':
            case ',':
            case 13:
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_ENTER : HP48_KEY_ENTER ) ] = true;
                break;
            case KEY_BACKSPACE:
            case 127:
            case '\b':
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_BS : HP48_KEY_BS ) ] = true;
                break;
            case KEY_F( 2 ):
            case '[':
            case 339: /* PgUp */
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_SHL : HP48_KEY_SHL ) ] = true;
                break;
            case KEY_F( 3 ):
            case ']':
            case 338: /* PgDn */
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_SHR : HP48_KEY_SHR ) ] = true;
                break;
            case KEY_F( 4 ):
            case ';':
            case KEY_IC: /* Ins */
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_ALPHA : HP48_KEY_ALPHA ) ] = true;
                break;
            case KEY_F( 5 ):
            case '\\':
            case 27:  /* Esc */
            case 262: /* Home */
                new_keyboard_state[ ( config.model == MODEL_49G ? HP49_KEY_ON : HP48_KEY_ON ) ] = true;
                break;

            case KEY_F( 7 ):
            case '|':      /* Shift+\ */
            case KEY_SEND: /* Shift+End */
            case KEY_F( 10 ):
                // please_exit = true;
                close_and_exit();
                break;

            default:
                break;
        }
    }

    for ( int key = FIRST_HPKEY; key <= LAST_HPKEY; key++ ) {
        if ( keyboard_state[ key ] == new_keyboard_state[ key ] )
            continue; /* key hasn't changed state */

        if ( !keyboard_state[ key ] && new_keyboard_state[ key ] && !is_key_pressed( key ) )
            press_key( key );
        else if ( keyboard_state[ key ] && !new_keyboard_state[ key ] && is_key_pressed( key ) )
            release_key( key );

        keyboard_state[ key ] = new_keyboard_state[ key ];
    }
}

void ui_stop_ncurses( void )
{
    nodelay( stdscr, FALSE );
    echo();
    endwin();
}

void ui_start_ncurses( config_t* conf )
{
    config = *conf;

    setlocale( LC_ALL, "" );
    initscr();              /* initialize the curses library */
    keypad( stdscr, TRUE ); /* enable keyboard mapping */
    nodelay( stdscr, TRUE );
    curs_set( 0 );
    cbreak(); /* take input chars one at a time, no wait for \n */
    noecho();
    nonl(); /* tell curses not to do NL->CR/NL on output */

    if ( !config.mono && has_colors() ) {
        start_color();

        if ( config.gray ) {
            init_color( LCD_COLOR_BG, COLORS[ UI4X_COLOR_PIXEL_OFF ].gray_rgb, COLORS[ UI4X_COLOR_PIXEL_OFF ].gray_rgb,
                        COLORS[ UI4X_COLOR_PIXEL_OFF ].gray_rgb );
            init_color( LCD_COLOR_FG, COLORS[ UI4X_COLOR_PIXEL_ON ].gray_rgb, COLORS[ UI4X_COLOR_PIXEL_ON ].gray_rgb,
                        COLORS[ UI4X_COLOR_PIXEL_ON ].gray_rgb );
        } else {
            init_color( LCD_COLOR_BG, ( COLORS[ UI4X_COLOR_PIXEL_OFF ].rgb >> 16 ) & 0xff,
                        ( COLORS[ UI4X_COLOR_PIXEL_OFF ].rgb >> 8 ) & 0xff, COLORS[ UI4X_COLOR_PIXEL_OFF ].rgb & 0xff );
            init_color( LCD_COLOR_BG, ( COLORS[ UI4X_COLOR_PIXEL_ON ].rgb >> 16 ) & 0xff, ( COLORS[ UI4X_COLOR_PIXEL_ON ].rgb >> 8 ) & 0xff,
                        COLORS[ UI4X_COLOR_PIXEL_ON ].rgb & 0xff );
        }

        init_pair( LCD_PIXEL_OFF, LCD_COLOR_BG, LCD_COLOR_BG );
        init_pair( LCD_PIXEL_ON, LCD_COLOR_FG, LCD_COLOR_FG );
        init_pair( LCD_COLORS_PAIR, LCD_COLOR_FG, LCD_COLOR_BG );
    }

    mvaddch( 0, 0, ACS_ULCORNER );
    mvaddch( LCD_BOTTOM, 0, ACS_LLCORNER );
    mvaddch( 0, LCD_RIGHT, ACS_URCORNER );
    mvaddch( LCD_BOTTOM, LCD_RIGHT, ACS_LRCORNER );
    mvhline( 0, 1, ACS_HLINE, LCD_RIGHT - 1 );
    mvhline( LCD_BOTTOM, 1, ACS_HLINE, LCD_RIGHT - 1 );
    mvvline( 1, 0, ACS_VLINE, LCD_BOTTOM - 1 );
    mvvline( 1, LCD_RIGHT, ACS_VLINE, LCD_BOTTOM - 1 );

    mvprintw( 0, 2, "[   |   |   |   |   |   ]" ); /* annunciators */
    mvprintw( 0, LCD_RIGHT - 18, "< %s v%i.%i.%i >", config.progname, VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL );

    mvprintw( LCD_BOTTOM, 2, "[ wire: %s ]-[ IR: %s ]-[ contrast: %i ]", config.wire_name, config.ir_name, get_contrast() );

    mvprintw( LCD_BOTTOM + 1, 0, "F1: Enter, F2: Left-Shift, F3: Right-Shift, F4: Alpha, F5: On, F7: Quit" );
}

void setup_frontend_ncurses( void )
{
    ui_get_event = ui_get_event_ncurses;
    ui_update_display = ui_update_display_ncurses;
    ui_start = ui_start_ncurses;
    ui_stop = ui_stop_ncurses;
}
