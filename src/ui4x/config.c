#include <assert.h>
#include <errno.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>

#include <getopt.h>

#include <lua.h>
#include <lauxlib.h>

#include "config.h"

#include "../debug.h"

#ifndef LUA_OK
#  define LUA_OK 0
#endif

static config_t config = {
    .progname = ( char* )"saturn4xxx",

    .model = MODEL_48GX,
    .throttle = false,
    .verbose = false,
    .shiftless = false,
    .big_screen = false,
    .black_lcd = false,

    .frontend = FRONTEND_SDL,

    .mono = false,
    .gray = false,

    .chromeless = false,
    .fullscreen = false,
    .scale = 1.0,

    .tiny = false,
    .small = false,

    /* from args.h */
    .reset = false,
    .monitor = false,
    .batchXfer = false,
    .state_dir_path = ".",

    .debug_level = DEBUG_C_NONE,
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
    switch ( config.model ) {
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
    }
    fprintf( stdout, "\" -- possible values: \"48gx\", \"48sx\", \"40g\", \"49g\"\n" );
    fprintf( stdout, "\n" );

    fprintf( stdout, "verbose = %s\n", config.verbose ? "true" : "false" );
    fprintf( stdout, "throttle = %s\n", config.throttle ? "true" : "false" );
    fprintf( stdout, "\n" );
    fprintf( stdout, "monitor = %s\n", config.monitor ? "true" : "false" );
    fprintf( stdout, "\n" );

    fprintf( stdout, "--------------------\n" );
    fprintf( stdout, "-- User Interface --\n" );
    fprintf( stdout, "--------------------\n" );
    fprintf( stdout, "frontend = \"" );
    switch ( config.frontend ) {
        case FRONTEND_SDL:
            fprintf( stdout, "sdl" );
            break;
        case FRONTEND_NCURSES:
            fprintf( stdout, "tui" );
            if ( config.small )
                fprintf( stdout, "-small" );
            else if ( config.tiny )
                fprintf( stdout, "-tiny" );
            break;
    }
    fprintf( stdout, "\" -- possible values: \"sdl\", \"tui\", \"tui-small\", \"tui-tiny\"\n" );
    fprintf( stdout, "\n" );

    fprintf( stdout, "gray = %s\n", config.gray ? "true" : "false" );
    fprintf( stdout, "mono = %s\n", config.mono ? "true" : "false" );

    fprintf( stdout, "\n" );
    fprintf( stdout, " -- Following options are specific to sdl frontend\n" );
    fprintf( stdout, "big_screen = %s\n", config.big_screen ? "true" : "false" );
    fprintf( stdout, "black_lcd = %s\n", config.black_lcd ? "true" : "false" );
    fprintf( stdout, "chromeless = %s\n", config.chromeless ? "true" : "false" );
    fprintf( stdout, "fullscreen = %s\n", config.fullscreen ? "true" : "false" );
    fprintf( stdout, "scale = %f\n", config.scale );
    fprintf( stdout, "shiftless = %s\n", config.shiftless ? "true" : "false" );

    fprintf( stdout, "\n" );
    fprintf( stdout, "--- End of saturnng configuration ----------------------------------------------\n" );
}

/* Path/name dynamic allocator */
static char* normalize_filename( char* path, char* name )
{
    char* s = malloc( strlen( path ) + strlen( name ) + 2 );

    strcpy( s, path );
    strcat( s, "/" );
    strcat( s, name );

    return s;
}

config_t* config_init( int argc, char* argv[] )
{
    int option_index;
    int c = '?';

    char* config_file_name = "config.lua";
    char* mod_file_name = "mod";
    char* cpu_file_name = "cpu";
    char* hdw_file_name = "hdw";
    char* rom_file_name = "rom";
    char* ram_file_name = "ram";
    char* port_1_file_name = "port1";
    char* port_2_file_name = "port2";

    int print_config_and_exit = false;

    int clopt_model = -1;
    int clopt_verbose = -1;
    int clopt_big_screen = -1;
    int clopt_black_lcd = -1;
    int clopt_throttle = -1;
    int clopt_shiftless = -1;
    int clopt_frontend = -1;
    int clopt_mono = -1;
    int clopt_gray = -1;
    int clopt_chromeless = -1;
    int clopt_fullscreen = -1;
    double clopt_scale = -1.0;

    int clopt_tiny = -1;
    int clopt_small = -1;

    int clopt_reset = -1;
    int clopt_monitor = -1;
    /* int clopt_batchXfer = -1; */
    char* clopt_state_dir_path = ".";

    const char* optstring = "h";
    struct option long_options[] = {
        {"help",                 no_argument,       NULL,                   'h'             },
        {"verbose",              no_argument,       &clopt_verbose,         true            },
        {"print-config",         no_argument,       &print_config_and_exit, true            },

        {"throttle",             no_argument,       &clopt_throttle,        true            },
        {"big-screen",           no_argument,       &clopt_big_screen,      true            },
        {"black-lcd",            no_argument,       &clopt_black_lcd,       true            },

        {"48sx",                 no_argument,       &clopt_model,           MODEL_48SX      },
        {"48gx",                 no_argument,       &clopt_model,           MODEL_48GX      },
        {"40g",                  no_argument,       &clopt_model,           MODEL_40G       },
        {"49g",                  no_argument,       &clopt_model,           MODEL_49G       },

        {"reset",                no_argument,       &clopt_reset,           true            },
        {"monitor",              no_argument,       &clopt_monitor,         true            },
        /* {"batchXfer",  no_argument,       &clopt_batchXfer,  true            }, */
        {"state-dir",            required_argument, NULL,                   8999            },

        {"shiftless",            no_argument,       &clopt_shiftless,       true            },

        {"gui",                  no_argument,       &clopt_frontend,        FRONTEND_SDL    },
        {"chromeless",           no_argument,       &clopt_chromeless,      true            },
        {"fullscreen",           no_argument,       &clopt_fullscreen,      true            },
        {"scale",                required_argument, NULL,                   7110            },

        {"tui",                  no_argument,       &clopt_frontend,        FRONTEND_NCURSES},
        {"tui-small",            no_argument,       NULL,                   6110            },
        {"tui-tiny",             no_argument,       NULL,                   6120            },

        {"mono",                 no_argument,       &clopt_mono,            true            },
        {"gray",                 no_argument,       &clopt_gray,            true            },

        {"debug-opcodes",        no_argument,       NULL,                   38601           },
        {"debug-revision",       no_argument,       NULL,                   38602           },
        {"debug-x-func",         no_argument,       NULL,                   38603           },
        {"debug-flash",          no_argument,       NULL,                   38604           },
        {"debug-implementation", no_argument,       NULL,                   38605           },
        {"debug-mod-cache",      no_argument,       NULL,                   38606           },
        {"debug-serial",         no_argument,       NULL,                   38607           },
        {"debug-timers",         no_argument,       NULL,                   38608           },
        {"debug-interruptions",  no_argument,       NULL,                   38609           },
        {"debug-display",        no_argument,       NULL,                   38610           },
        {"debug-modules",        no_argument,       NULL,                   38611           },
        {"debug-trace",          no_argument,       NULL,                   38612           },

        {0,                      0,                 0,                      0               }
    };

    const char* help_text = "usage: %s [options]\n"
                            "options:\n"
                            "  -h --help         what you are reading\n"
                            "     --print-config output current configuration to stdout and exit (in config.lua formatting)\n"
                            "     --verbose      display more informations\n"
                            "     --throttle     throttle CPU speed\n"
                            "     --big-screen   131×80 screen (default: false)\n"
                            "     --black-lcd    (default: false)\n"
                            "     --48gx         emulate a HP 48GX\n"
                            "     --48sx         emulate a HP 48SX\n"
                            "     --40g          emulate a HP 40G\n"
                            "     --49g          emulate a HP 49G\n"
                            "     --state-dir=<path> use a different data directory "
                            "(default: ~/.config/saturnMODEL/)\n"
                            "     --gui          graphical (SDL2) front-end (default: true)\n"
                            "     --tui          text front-end (default: false)\n"
                            "     --tui-small    text small front-end (2×2 pixels per character) (default: "
                            "false)\n"
                            "     --tui-tiny     text tiny front-end (2×4 pixels per character) (default: "
                            "false)\n"
                            "     --chromeless   only show display (default: "
                            "false)\n"
                            "     --fullscreen   make the UI fullscreen "
                            "(default: false)\n"
                            "     --scale=<n>    make the UI scale <n> times "
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
                            "     --debug-revision       enables debugging revision (default: no)\n"
                            "     --debug-x-func         enables debugging extended functions (default: no)\n"
                            "     --debug-flash          enables debugging flash (default: no)\n"
                            "     --debug-implementation enables debugging implementation (default: no)\n"
                            "     --debug-mod-cache      enables debugging mod cache (default: no)\n"
                            "     --debug-serial         enables debugging serial (default: no)\n"
                            "     --debug-timers         enables debugging timers (default: no)\n"
                            "     --debug-interruptions  enables debugging interruptions (default: no)\n"
                            "     --debug-display        enables debugging display (default: no)\n"
                            "     --debug-modules        enables debugging modules (default: no)\n"
                            "     --debug-trace          enables debugging trace (default: no)\n";

    while ( c != EOF ) {
        c = getopt_long( argc, argv, optstring, long_options, &option_index );

        switch ( c ) {
            case 'h':
                fprintf( stderr, help_text, config.progname );
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
                clopt_scale = atof( optarg );
                break;
            case 8999:
                clopt_state_dir_path = optarg;
                break;

            case 38601:
                config.debug_level |= DEBUG_C_OPCODES;
                break;
            case 38602:
                config.debug_level |= DEBUG_C_REVISION;
                break;
            case 38603:
                config.debug_level |= DEBUG_C_X_FUNC;
                break;
            case 38604:
                config.debug_level |= DEBUG_C_FLASH;
                break;
            case 38605:
                config.debug_level |= DEBUG_C_IMPLEMENTATION;
                break;
            case 38606:
                config.debug_level |= DEBUG_C_MOD_CACHE;
                break;
            case 38607:
                config.debug_level |= DEBUG_C_SERIAL;
                break;
            case 38608:
                config.debug_level |= DEBUG_C_TIMERS;
                break;
            case 38609:
                config.debug_level |= DEBUG_C_INT;
                break;
            case 38610:
                config.debug_level |= DEBUG_C_DISPLAY;
                break;
            case 38611:
                config.debug_level |= DEBUG_C_MODULES;
                break;
            case 38612:
                config.debug_level |= DEBUG_C_TRACE;
                break;

            default:
                break;
        }
    }

    if ( clopt_state_dir_path != NULL )
        config.state_dir_path = strdup( clopt_state_dir_path );

    config.config_file_name = normalize_filename( config.state_dir_path, config_file_name );
    config.mod_file_name = normalize_filename( config.state_dir_path, mod_file_name );
    config.cpu_file_name = normalize_filename( config.state_dir_path, cpu_file_name );
    config.hdw_file_name = normalize_filename( config.state_dir_path, hdw_file_name );
    config.rom_file_name = normalize_filename( config.state_dir_path, rom_file_name );
    config.ram_file_name = normalize_filename( config.state_dir_path, ram_file_name );
    config.port_1_file_name = normalize_filename( config.state_dir_path, port_1_file_name );
    config.port_2_file_name = normalize_filename( config.state_dir_path, port_2_file_name );

    /**********************/
    /* 1. read config.lua */
    /**********************/
    bool haz_config_file = config_read( config.config_file_name );
    if ( haz_config_file ) {
        lua_getglobal( config_lua_values, "verbose" );
        config.verbose = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "throttle" );
        config.throttle = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "big_screen" );
        config.big_screen = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "black_lcd" );
        config.black_lcd = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "chromeless" );
        config.chromeless = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "fullscreen" );
        config.fullscreen = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "gray" );
        config.gray = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "mono" );
        config.mono = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "shiftless" );
        config.shiftless = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "monitor" );
        config.monitor = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "model" );
        const char* svalue_model = luaL_optstring( config_lua_values, -1, "49g" );
        if ( svalue_model != NULL ) {
            if ( strcmp( svalue_model, "49g" ) == 0 )
                config.model = MODEL_49G;
            if ( strcmp( svalue_model, "40g" ) == 0 )
                config.model = MODEL_40G;
            if ( strcmp( svalue_model, "48gx" ) == 0 )
                config.model = MODEL_48GX;
            if ( strcmp( svalue_model, "48sx" ) == 0 )
                config.model = MODEL_48SX;
        }

        lua_getglobal( config_lua_values, "frontend" );
        const char* svalue = luaL_optstring( config_lua_values, -1, "sdl" );
        if ( svalue != NULL ) {
            if ( strcmp( svalue, "sdl" ) == 0 )
                config.frontend = FRONTEND_SDL;
            if ( strcmp( svalue, "tui" ) == 0 ) {
                config.frontend = FRONTEND_NCURSES;
                config.small = false;
                config.tiny = false;
            }
            if ( strcmp( svalue, "tui-small" ) == 0 ) {
                config.frontend = FRONTEND_NCURSES;
                config.small = true;
                config.tiny = false;
            }
            if ( strcmp( svalue, "tui-tiny" ) == 0 ) {
                config.frontend = FRONTEND_NCURSES;
                config.small = false;
                config.tiny = true;
            }
        }

        lua_getglobal( config_lua_values, "scale" );
        config.scale = luaL_optnumber( config_lua_values, -1, 1.0 );
    }

    /****************************************************/
    /* 2. treat command-line params which have priority */
    /****************************************************/
    if ( clopt_verbose != -1 )
        config.verbose = clopt_verbose == true;
    if ( clopt_model != -1 )
        config.model = clopt_model;
    if ( clopt_throttle != -1 )
        config.throttle = clopt_throttle == true;
    if ( clopt_big_screen != -1 )
        config.big_screen = clopt_big_screen == true;
    if ( clopt_black_lcd != -1 )
        config.black_lcd = clopt_black_lcd == true;
    if ( clopt_frontend != -1 )
        config.frontend = clopt_frontend;
    if ( clopt_chromeless != -1 )
        config.chromeless = clopt_chromeless == true;
    if ( clopt_fullscreen != -1 )
        config.fullscreen = clopt_fullscreen == true;
    if ( clopt_scale > 0.0 )
        config.scale = clopt_scale;
    if ( clopt_mono != -1 )
        config.mono = clopt_mono == true;
    if ( clopt_small != -1 )
        config.small = clopt_small == true;
    if ( clopt_tiny != -1 )
        config.tiny = clopt_tiny == true;
    if ( clopt_gray != -1 )
        config.gray = clopt_gray == true;
    if ( clopt_shiftless != -1 )
        config.shiftless = clopt_shiftless == true;

    if ( clopt_reset != -1 )
        config.reset = clopt_reset;
    if ( clopt_monitor != -1 )
        config.monitor = clopt_monitor;
    /* if ( clopt_batchXfer != -1 ) */
    /*     config.batchXfer = clopt_batchXfer; */

    if ( config.model == MODEL_49G )
        config.black_lcd = true;

    config.progname = basename( strdup( argv[ 0 ] ) );
    switch ( config.model ) {
        case MODEL_48GX:
            strcat( config.progname, "48gx" );
            break;
        case MODEL_48SX:
            strcat( config.progname, "48sx" );
            break;
        case MODEL_49G:
            strcat( config.progname, "49g" );
            break;
        case MODEL_40G:
            strcat( config.progname, "40g" );
            break;
    }

    if ( config.verbose ) {
        if ( !print_config_and_exit )
            print_config();

        if ( optind < argc ) {
            fprintf( stderr, "%i invalid arguments : ", argc - optind );
            while ( optind < argc )
                fprintf( stderr, "%s\n", argv[ optind++ ] );
            fprintf( stderr, "\n" );
        }
    }

    if ( print_config_and_exit ) {
        print_config();
        exit( EXIT_SUCCESS );
    }

    if ( !haz_config_file ) {
        fprintf( stdout, "\nConfiguration file %s doesn't seem to exist or is invalid!\n", config.config_file_name );

        fprintf( stdout, "You can solve this by running `mkdir -p %s && %s --print-config >> %s`\n\n", config.state_dir_path,
                 config.progname, config.config_file_name );
    }

    return &config;
}
