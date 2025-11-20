#include <assert.h>
#include <errno.h>
#include <libgen.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <getopt.h>

#include <lauxlib.h>
#include <lua.h>

#include "core/chf_wrapper.h"
#include "options.h"

#ifndef LUA_OK
#  define LUA_OK 0
#endif

static config_t __config = {
    .progname = ( char* )"saturn4xxx",

    .model = MODEL_48GX,
    .throttle = false,
    .verbose = false,

    .shiftless = false,
    .big_screen = false,
    .black_lcd = false,

#if defined( HAS_SDL )
    .frontend = FRONTEND_SDL,
#elif defined( HAS_GTK )
    .frontend = FRONTEND_GTK,
#else
    .frontend = FRONTEND_NCURSES,
#endif

    .mono = false,
    .gray = false,

    .chromeless = false,
    .fullscreen = false,
    .zoom = 1.0,

    .tiny = false,
    .small = false,

    .reset = false,
    .monitor = false,

    .speed = 0,

    .debug_level = DEBUG_C_NONE,

    .datadir = ( char* )".",
};

lua_State* config_lua_values;

static inline bool config_read( const char* filename )
{
    int rc;

    assert( filename != NULL );

    /*---------------------------------------------------
    ; Create the Lua state, which includes NO predefined
    ; functions or values.  This is literally an empty
    ; slate.
    ;----------------------------------------------------*/
    config_lua_values = luaL_newstate();
    if ( config_lua_values == NULL ) {
        fprintf( stderr, "cannot create Lua state\n" );
        return false;
    }

    /*-----------------------------------------------------
    ; For the truly paranoid about sandboxing, enable the
    ; following code, which removes the string library,
    ; which some people find problematic to leave un-sand-
    ; boxed. But in my opinion, if you are worried about
    ; such attacks in a configuration file, you have bigger
    ; security issues to worry about than this.
    ;------------------------------------------------------*/
#ifdef PARANOID
    lua_pushliteral( config_lua_values, "x" );
    lua_pushnil( config_lua_values );
    lua_setmetatable( config_lua_values, -2 );
    lua_pop( config_lua_values, 1 );
#endif

    /*-----------------------------------------------------
    ; Lua 5.2+ can restrict scripts to being text only,
    ; to avoid a potential problem with loading pre-compiled
    ; Lua scripts that may have malformed Lua VM code that
    ; could possibly lead to an exploit, but again, if you
    ; have to worry about that, you have bigger security
    ; issues to worry about.  But in any case, here I'm
    ; restricting the file to "text" only.
    ;------------------------------------------------------*/
    rc = luaL_loadfile( config_lua_values, filename );
    if ( rc != LUA_OK ) {
        fprintf( stderr, "Lua error: (%d) %s\n", rc, lua_tostring( config_lua_values, -1 ) );
        return false;
    }

    rc = lua_pcall( config_lua_values, 0, 0, 0 );
    if ( rc != LUA_OK ) {
        fprintf( stderr, "Lua error: (%d) %s\n", rc, lua_tostring( config_lua_values, -1 ) );
        return false;
    }

    return true;
}

static void print_config( void )
{
    fprintf( stdout, "--------------------------------------------------------------------------------\n" );
    fprintf( stdout, "-- Configuration file for saturnng\n" );
    fprintf( stdout, "-- This is a comment\n" );

    fprintf( stdout, "model = \"" );
    switch ( __config.model ) {
        case MODEL_48GX:
            fprintf( stdout, "48gx" );
            break;
        case MODEL_48SX:
            fprintf( stdout, "48sx" );
            break;
        case MODEL_40G:
            fprintf( stdout, "40g" );
            break;
        case MODEL_49G:
            fprintf( stdout, "49g" );
            break;
        case MODEL_50G:
            fprintf( stdout, "50g" );
            break;
    }
    fprintf( stdout, "\" -- possible values: \"48gx\", \"48sx\", \"40g\", \"49g\"\n" );
    fprintf( stdout, "\n" );

    fprintf( stdout, "verbose = %s\n", __config.verbose ? "true" : "false" );
    fprintf( stdout, "throttle = %s\n", __config.throttle ? "true" : "false" );
    fprintf( stdout, "speed = %i\n", __config.speed );
    fprintf( stdout, "\n" );
    fprintf( stdout, "monitor = %s\n", __config.monitor ? "true" : "false" );
    fprintf( stdout, "\n" );

    fprintf( stdout, "--------------------\n" );
    fprintf( stdout, "-- User Interface --\n" );
    fprintf( stdout, "--------------------\n" );
    fprintf( stdout, "frontend = \"" );
    switch ( __config.frontend ) {
        case FRONTEND_GTK:
            fprintf( stdout, "gtk" );
            break;
        case FRONTEND_SDL:
            fprintf( stdout, "sdl" );
            break;
        case FRONTEND_NCURSES:
            fprintf( stdout, "tui" );
            if ( __config.small )
                fprintf( stdout, "-small" );
            else if ( __config.tiny )
                fprintf( stdout, "-tiny" );
            break;
    }
    fprintf( stdout, "\" -- possible values: \"sdl\", \"tui\", \"tui-small\", \"tui-tiny\"\n" );
    fprintf( stdout, "\n" );

    fprintf( stdout, "gray = %s\n", __config.gray ? "true" : "false" );
    fprintf( stdout, "mono = %s\n", __config.mono ? "true" : "false" );

    fprintf( stdout, "\n" );
    fprintf( stdout, " -- Following options are specific to sdl frontend\n" );
    fprintf( stdout, "black_lcd = %s\n", __config.black_lcd ? "true" : "false" );
    fprintf( stdout, "chromeless = %s\n", __config.chromeless ? "true" : "false" );
    fprintf( stdout, "fullscreen = %s\n", __config.fullscreen ? "true" : "false" );
    fprintf( stdout, "zoom = %f\n", __config.zoom );
    fprintf( stdout, "shiftless = %s\n", __config.shiftless ? "true" : "false" );

    fprintf( stdout, "\n" );
    fprintf( stdout, "--- End of saturnng configuration ----------------------------------------------\n" );
}

/* Path/name dynamic allocator */
static char* normalize_filename( char* path, const char* name )
{
    char* s = malloc( strlen( path ) + strlen( name ) + 2 );

    strcpy( s, path );
    strcat( s, "/" );
    strcat( s, name );

    return s;
}

char* path_file_in_datadir( const char* filename ) { return normalize_filename( __config.datadir, filename ); }

config_t* config_init( int argc, char* argv[] )
{
    int option_index;
    int c = '?';

    int print_config_and_exit = false;

    int clopt_model = -1;
    int clopt_verbose = -1;
    int clopt_black_lcd = -1;
    int clopt_throttle = -1;
    int clopt_shiftless = -1;
    int clopt_frontend = -1;
    int clopt_mono = -1;
    int clopt_gray = -1;
    int clopt_chromeless = -1;
    int clopt_fullscreen = -1;
    double clopt_zoom = -1.0;

    int clopt_tiny = -1;
    int clopt_small = -1;

    int clopt_reset = -1;
    int clopt_monitor = -1;

    int clopt_speed = -1;

    char* clopt_datadir = ( char* )".";

    const char* optstring = "h";
    struct option long_options[] = {
        {"help",                 no_argument,       NULL,                   'h'             },
        {"verbose",              no_argument,       &clopt_verbose,         true            },
        {"print-config",         no_argument,       &print_config_and_exit, true            },

        {"throttle",             no_argument,       &clopt_throttle,        true            },

        {"speed",                required_argument, NULL,                   7111            },

        {"48sx",                 no_argument,       &clopt_model,           MODEL_48SX      },
        {"48gx",                 no_argument,       &clopt_model,           MODEL_48GX      },
        {"40g",                  no_argument,       &clopt_model,           MODEL_40G       },
        {"49g",                  no_argument,       &clopt_model,           MODEL_49G       },

        {"reset",                no_argument,       &clopt_reset,           true            },
        {"monitor",              no_argument,       &clopt_monitor,         true            },

        {"datadir",            required_argument, NULL,                   8999            },
        {"state-dir",            required_argument, NULL,                   8999            }, /* DEPRECATED */

        {"shiftless",            no_argument,       &clopt_shiftless,       true            },

#if defined( HAS_GTK )
        {"gtk",                  no_argument,       &clopt_frontend,        FRONTEND_GTK    },
#endif
#if defined( HAS_SDL )
        {"sdl",                  no_argument,       &clopt_frontend,        FRONTEND_SDL    },
        {"gui",                  no_argument,       &clopt_frontend,        FRONTEND_SDL    }, /* DEPRECATED */
#endif
        {"chromeless",           no_argument,       &clopt_chromeless,      true            },
        {"fullscreen",           no_argument,       &clopt_fullscreen,      true            },
        {"zoom",                required_argument, NULL,                   7110            },
        {"scale",                required_argument, NULL,                   7110            }, /* DEPRECATED */
        {"black-lcd",            no_argument,       &clopt_black_lcd,       true            },

        {"tui",                  no_argument,       &clopt_frontend,        FRONTEND_NCURSES},
        {"tui-small",            no_argument,       NULL,                   6110            },
        {"tui-tiny",             no_argument,       NULL,                   6120            },

        {"mono",                 no_argument,       &clopt_mono,            true            },
        {"gray",                 no_argument,       &clopt_gray,            true            },

        {"debug-opcodes",        no_argument,       NULL,                   38601           },
        {"debug-flash",          no_argument,       NULL,                   38604           },
        {"debug-implementation", no_argument,       NULL,                   38605           },
        {"debug-bus-cache",      no_argument,       NULL,                   38606           },
        {"debug-serial",         no_argument,       NULL,                   38607           },
        {"debug-timers",         no_argument,       NULL,                   38608           },
        {"debug-interruptions",  no_argument,       NULL,                   38609           },
        {"debug-bus",            no_argument,       NULL,                   38611           },

        {0,                      0,                 0,                      0               }
    };

    const char* help_text = "usage: %s [options]\n"
                            "options:\n"
                            "  -h --help         what you are reading\n"
                            "     --print-config output current configuration to stdout and exit (in config.lua formatting)\n"
                            "     --verbose      display more informations\n"
                            "     --throttle     throttle CPU speed\n"
                            "     --speed=<n>    set cpu's speed to <n> MHz "
                            "(default: 1.0)\n"
                            "     --black-lcd    (default: false)\n"
                            "     --48gx         emulate a HP 48GX\n"
                            "     --48sx         emulate a HP 48SX\n"
                            "     --40g          emulate a HP 40G\n"
                            "     --49g          emulate a HP 49G\n"
                            "     --datadir=<path> use a different data directory "
                            "(default: ~/.config/saturnMODEL/)\n"
#if defined( HAS_SDL )
                            "     --sdl          graphical (SDL2) front-end (default: true)\n"
#endif
#if defined( HAS_GTK )
                            "     --gtk          graphical (gtk4) front-end (default: false)\n"
#endif
                            "     --tui          text front-end (default: false)\n"
                            "     --tui-small    text small front-end (2×2 pixels per character) (default: "
                            "false)\n"
                            "     --tui-tiny     text tiny front-end (2×4 pixels per character) (default: "
                            "false)\n"
                            "     --chromeless   only show display (default: "
                            "false)\n"
                            "     --fullscreen   make the UI fullscreen "
                            "(default: false)\n"
                            "     --zoom=<n>    make the UI zoom <n> times "
                            "(default: 1.0)\n"
                            "     --mono         make the UI monochrome (default: "
                            "false)\n"
                            "     --gray         make the UI grayscale (default: "
                            "false)\n"
                            "     --shiftless    don't map the shift keys to let them free for numbers (default: "
                            "false)\n"
                            "     --reset        force a reset\n"
                            "     --monitor      start with monitor (default: no)\n"
                            "\n"
                            "     --debug-opcodes        enables debugging opcodes (default: no)\n"
                            "     --debug-flash          enables debugging flash (default: no)\n"
                            "     --debug-implementation enables debugging implementation (default: no)\n"
                            "     --debug-bus-cache      enables debugging bus cache (default: no)\n"
                            "     --debug-serial         enables debugging serial (default: no)\n"
                            "     --debug-timers         enables debugging timers (default: no)\n"
                            "     --debug-interruptions  enables debugging interruptions (default: no)\n"
                            "     --debug-bus        enables debugging bus (default: no)\n";

    while ( c != EOF ) {
        c = getopt_long( argc, argv, optstring, long_options, &option_index );

        switch ( c ) {
            case 'h':
                fprintf( stdout, help_text, __config.progname );
                exit( EXIT_SUCCESS );
                break;
            case 6110:
                clopt_frontend = FRONTEND_NCURSES;
                clopt_small = true;
                break;
            case 6120:
                clopt_frontend = FRONTEND_NCURSES;
                clopt_tiny = true;
                break;
            case 7110:
                clopt_zoom = atof( optarg );
                break;
            case 7111:
                clopt_speed = atof( optarg );
                break;
            case 8999:
                clopt_datadir = optarg;
                break;

            case 38601:
                __config.debug_level |= DEBUG_C_OPCODES;
                break;
            case 38604:
                __config.debug_level |= DEBUG_C_FLASH;
                break;
            case 38605:
                __config.debug_level |= DEBUG_C_IMPLEMENTATION;
                break;
            case 38606:
                __config.debug_level |= DEBUG_C_BUS_CACHE;
                break;
            case 38607:
                __config.debug_level |= DEBUG_C_SERIAL;
                break;
            case 38608:
                __config.debug_level |= DEBUG_C_TIMERS;
                break;
            case 38609:
                __config.debug_level |= DEBUG_C_INT;
                break;
            case 38611:
                __config.debug_level |= DEBUG_C_BUS;
                break;

            default:
                break;
        }
    }

    if ( clopt_datadir != NULL )
        __config.datadir = strdup( clopt_datadir );

    /**********************/
    /* 1. read config.lua */
    /**********************/
    bool haz_config_file = config_read( path_file_in_datadir( CONFIG_FILE_NAME ) );
    if ( haz_config_file ) {
        lua_getglobal( config_lua_values, "verbose" );
        __config.verbose = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "throttle" );
        __config.throttle = lua_toboolean( config_lua_values, -1 );

        /* lua_getglobal( config_lua_values, "big_screen" ); */
        /* __config.big_screen = lua_toboolean( config_lua_values, -1 ); */

        lua_getglobal( config_lua_values, "black_lcd" );
        __config.black_lcd = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "chromeless" );
        __config.chromeless = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "fullscreen" );
        __config.fullscreen = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "gray" );
        __config.gray = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "mono" );
        __config.mono = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "shiftless" );
        __config.shiftless = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "monitor" );
        __config.monitor = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "model" );
        const char* svalue_model = luaL_optstring( config_lua_values, -1, "49g" );
        if ( svalue_model != NULL ) {
            if ( strcmp( svalue_model, "49g" ) == 0 )
                __config.model = MODEL_49G;
            if ( strcmp( svalue_model, "40g" ) == 0 )
                __config.model = MODEL_40G;
            if ( strcmp( svalue_model, "48gx" ) == 0 )
                __config.model = MODEL_48GX;
            if ( strcmp( svalue_model, "48sx" ) == 0 )
                __config.model = MODEL_48SX;
        }

        lua_getglobal( config_lua_values, "frontend" );
        const char* svalue = luaL_optstring( config_lua_values, -1, "sdl" );
        if ( svalue != NULL ) {
            if ( strcmp( svalue, "gtk" ) == 0 )
                __config.frontend = FRONTEND_GTK;
            if ( strcmp( svalue, "sdl" ) == 0 )
                __config.frontend = FRONTEND_SDL;
            if ( strcmp( svalue, "tui" ) == 0 ) {
                __config.frontend = FRONTEND_NCURSES;
                __config.small = false;
                __config.tiny = false;
            }
            if ( strcmp( svalue, "tui-small" ) == 0 ) {
                __config.frontend = FRONTEND_NCURSES;
                __config.small = true;
                __config.tiny = false;
            }
            if ( strcmp( svalue, "tui-tiny" ) == 0 ) {
                __config.frontend = FRONTEND_NCURSES;
                __config.small = false;
                __config.tiny = true;
            }
        }

        lua_getglobal( config_lua_values, "zoom" );
        __config.zoom = luaL_optnumber( config_lua_values, -1, 1.0 );

        /* DEPRECATED */
        lua_getglobal( config_lua_values, "scale" );
        __config.zoom = luaL_optnumber( config_lua_values, -1, 1.0 );

        lua_getglobal( config_lua_values, "speed" );
        __config.speed = luaL_optnumber( config_lua_values, -1, 1 );
    }

    /****************************************************/
    /* 2. treat command-line params which have priority */
    /****************************************************/
    if ( clopt_verbose != -1 )
        __config.verbose = clopt_verbose == true;
    if ( clopt_model != -1 )
        __config.model = clopt_model;
    if ( clopt_throttle != -1 )
        __config.throttle = clopt_throttle == true;
    if ( clopt_black_lcd != -1 )
        __config.black_lcd = clopt_black_lcd == true;
    if ( clopt_frontend != -1 )
        __config.frontend = clopt_frontend;
    if ( clopt_chromeless != -1 )
        __config.chromeless = clopt_chromeless == true;
    if ( clopt_fullscreen != -1 )
        __config.fullscreen = clopt_fullscreen == true;
    if ( clopt_zoom > 0.0 )
        __config.zoom = clopt_zoom;
    if ( clopt_mono != -1 )
        __config.mono = clopt_mono == true;
    if ( clopt_small != -1 )
        __config.small = clopt_small == true;
    if ( clopt_tiny != -1 )
        __config.tiny = clopt_tiny == true;
    if ( clopt_gray != -1 )
        __config.gray = clopt_gray == true;
    if ( clopt_shiftless != -1 )
        __config.shiftless = clopt_shiftless == true;

    if ( clopt_reset != -1 )
        __config.reset = clopt_reset;
    if ( clopt_monitor != -1 )
        __config.monitor = clopt_monitor;

    if ( clopt_speed > 0 )
        __config.speed = clopt_speed;

    __config.progname = basename( strdup( argv[ 0 ] ) );
    switch ( __config.model ) {
        case MODEL_48GX:
            strcat( __config.progname, "48gx" );
            break;
        case MODEL_48SX:
            strcat( __config.progname, "48sx" );
            break;
        case MODEL_49G:
            strcat( __config.progname, "49g" );
            break;
        case MODEL_40G:
            strcat( __config.progname, "40g" );
            break;
        case MODEL_50G:
            strcat( __config.progname, "50g" );
            /* __config.model = MODEL_49G; */
            __config.big_screen = true;
            break;
    }

    if ( __config.model == MODEL_49G )
        __config.black_lcd = true;

    if ( __config.verbose ) {
        if ( !print_config_and_exit )
            print_config();

        if ( optind < argc ) {
            fprintf( stderr, "%i invalid arguments : ", argc - optind );
            while ( optind < argc ) {
                fprintf( stderr, "%s\n", argv[ optind ] );
                optind++;
            }
            fprintf( stderr, "\n" );
        }
    }

    if ( print_config_and_exit ) {
        print_config();
        exit( EXIT_SUCCESS );
    }

    if ( !haz_config_file ) {
        fprintf( stdout, "\nConfiguration file %s doesn't seem to exist or is invalid!\n", path_file_in_datadir( CONFIG_FILE_NAME ) );

        fprintf( stdout, "You can solve this by running `mkdir -p %s && %s --print-config >> %s`\n\n", __config.datadir, __config.progname,
                 path_file_in_datadir( CONFIG_FILE_NAME ) );
    }

    return &__config;
}
