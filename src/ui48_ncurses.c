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

#include "ui48_config.h"
#include "ui48_emulator.h"
#include "ui48_common.h"
#include "ui48_inner.h"

#define COLORS ( config.model == MODEL_48GX ? colors_gx : colors_sx )
#define BUTTONS ( config.model == MODEL_48GX ? buttons_gx : buttons_sx )

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
static int lcd_pixels_buffer[ LCD_WIDTH * LCD_HEIGHT ];
static int last_annunciators = -1;
// static int last_contrast = -1;

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

    if ( get_display_state() ) {
        get_lcd_buffer( lcd_pixels_buffer );
    } else
        ui_init_LCD();

    ncurses_update_annunciators();
    ncurses_draw_lcd();
}

void ui_get_event_ncurses( void )
{
    int hpkey = -1;
    uint32_t k;

    /* Start fresh and mark all keys as released */
    // release_all_keys();

    /* Iterate over all currently pressed keys and mark them as pressed */
    while ( ( k = getch() ) ) {
        if ( k == ( uint32_t )ERR )
            break;

        switch ( k ) {
            case '0':
                hpkey = HPKEY_0;
                break;
            case '1':
                hpkey = HPKEY_1;
                break;
            case '2':
                hpkey = HPKEY_2;
                break;
            case '3':
                hpkey = HPKEY_3;
                break;
            case '4':
                hpkey = HPKEY_4;
                break;
            case '5':
                hpkey = HPKEY_5;
                break;
            case '6':
                hpkey = HPKEY_6;
                break;
            case '7':
                hpkey = HPKEY_7;
                break;
            case '8':
                hpkey = HPKEY_8;
                break;
            case '9':
                hpkey = HPKEY_9;
                break;
            case 'a':
                hpkey = HPKEY_A;
                break;
            case 'b':
                hpkey = HPKEY_B;
                break;
            case 'c':
                hpkey = HPKEY_C;
                break;
            case 'd':
                hpkey = HPKEY_D;
                break;
            case 'e':
                hpkey = HPKEY_E;
                break;
            case 'f':
                hpkey = HPKEY_F;
                break;
            case 'g':
                hpkey = HPKEY_MTH;
                break;
            case 'h':
                hpkey = HPKEY_PRG;
                break;
            case 'i':
                hpkey = HPKEY_CST;
                break;
            case 'j':
                hpkey = HPKEY_VAR;
                break;
            case 'k':
            case KEY_UP:
                hpkey = HPKEY_UP;
                break;
            case 'l':
                hpkey = HPKEY_NXT;
                break;
            case 'm':
                hpkey = HPKEY_QUOTE;
                break;
            case 'n':
                hpkey = HPKEY_STO;
                break;
            case 'o':
                hpkey = HPKEY_EVAL;
                break;
            case 'p':
            case KEY_LEFT:
                hpkey = HPKEY_LEFT;
                break;
            case 'q':
            case KEY_DOWN:
                hpkey = HPKEY_DOWN;
                break;
            case 'r':
            case KEY_RIGHT:
                hpkey = HPKEY_RIGHT;
                break;
            case 's':
                hpkey = HPKEY_SIN;
                break;
            case 't':
                hpkey = HPKEY_COS;
                break;
            case 'u':
                hpkey = HPKEY_TAN;
                break;
            case 'v':
                hpkey = HPKEY_SQRT;
                break;
            case 'w':
                hpkey = HPKEY_POWER;
                break;
            case 'x':
                hpkey = HPKEY_INV;
                break;
            case 'y':
                hpkey = HPKEY_NEG;
                break;
            case 'z':
                hpkey = HPKEY_EEX;
                break;
            case ' ':
                hpkey = HPKEY_SPC;
                break;
            case KEY_DC:
                hpkey = HPKEY_DEL;
                break;
            case '.':
                hpkey = HPKEY_PERIOD;
                break;
            case '+':
                hpkey = HPKEY_PLUS;
                break;
            case '-':
                hpkey = HPKEY_MINUS;
                break;
            case '*':
                hpkey = HPKEY_MUL;
                break;
            case '/':
                hpkey = HPKEY_DIV;
                break;

            case KEY_F( 1 ):
            case KEY_ENTER:
            case '\n':
            case ',':
                hpkey = HPKEY_ENTER;
                break;
            case KEY_BACKSPACE:
            case 127:
            case '\b':
                hpkey = HPKEY_BS;
                break;
            case KEY_F( 2 ):
            case '[':
            case 339: /* PgUp */
                hpkey = HPKEY_SHL;
                break;
            case KEY_F( 3 ):
            case ']':
            case 338: /* PgDn */
                hpkey = HPKEY_SHR;
                break;
            case KEY_F( 4 ):
            case ';':
            case KEY_IC: /* Ins */
                hpkey = HPKEY_ALPHA;
                break;
            case KEY_F( 5 ):
            case '\\':
            case 27:  /* Esc */
            case 262: /* Home */
                hpkey = HPKEY_ON;
                break;

            case KEY_F( 7 ):
            case '|':      /* Shift+\ */
            case KEY_SEND: /* Shift+End */
            case KEY_F( 10 ):
                // please_exit = true;
                close_and_exit();
                break;
        }

        if ( !is_key_pressed( hpkey ) )
            press_key( hpkey );
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
    if ( config.verbose )
        fprintf( stderr, "UI is ncurses\n" );

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
            init_color( LCD_COLOR_BG, COLORS[ LCD ].gray_rgb, COLORS[ LCD ].gray_rgb, COLORS[ LCD ].gray_rgb );
            init_color( LCD_COLOR_FG, COLORS[ PIXEL ].gray_rgb, COLORS[ PIXEL ].gray_rgb, COLORS[ PIXEL ].gray_rgb );
        } else {
            init_color( LCD_COLOR_BG, COLORS[ LCD ].r, COLORS[ LCD ].g, COLORS[ LCD ].b );
            init_color( LCD_COLOR_FG, COLORS[ PIXEL ].r, COLORS[ PIXEL ].g, COLORS[ PIXEL ].b );
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
