#include <assert.h>
#include <errno.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <getopt.h>

#include "ui4x_config.h"

static config_t config = {
    .progname = ( char* )"ui4x",

    .model = MODEL_48GX,
    .verbose = false,
    .shiftless = false,

    .frontend = FRONTEND_SDL,

    .mono = false,
    .gray = false,

    .chromeless = false,
    .fullscreen = false,
    .scale = 1.0,

    .tiny = false,
    .small = false,

    /* .wire_name = ( char* )"/dev/wire", */
    /* .ir_name = ( char* )"/dev/ir", */

    /* from args.h */
    .reset = false,
    .monitor = false,
    .batchXfer = false,
    .state_dir_path = ".",
    /* .mod_file_name, */
    /* .cpu_file_name, */
    /* .hdw_file_name, */
    /* .rom_file_name, */
    /* .ram_file_name, */
    /* .port_1_file_name, */
    /* .port_2_file_name, */
    /* .hw = ( char* )"hp48", /\* 2.1: Hardware configuration (unused) *\/ */
};

static void print_config( void )
{
    fprintf( stderr, "*** config\n" );
    fprintf( stderr, "  .progname = %s\n", config.progname );

    fprintf( stderr, "  .model = %i\n", config.model );
    fprintf( stderr, "  .verbose = %s\n", config.verbose ? "true" : "false" );
    fprintf( stderr, "  .shiftless = %s\n", config.shiftless ? "true" : "false" );

    fprintf( stderr, "  .frontend = %i\n", config.frontend );

    fprintf( stderr, "  .mono = %s\n", config.mono ? "true" : "false" );
    fprintf( stderr, "  .gray = %s\n", config.gray ? "true" : "false" );

    fprintf( stderr, "  .chromeless = %s\n", config.chromeless ? "true" : "false" );
    fprintf( stderr, "  .fullscreen = %s\n", config.fullscreen ? "true" : "false" );
    fprintf( stderr, "  .scale = %f\n", config.scale );

    fprintf( stderr, "  .tiny = %s\n", config.tiny ? "true" : "false" );
    fprintf( stderr, "  .small = %s\n", config.small ? "true" : "false" );

    fprintf( stderr, "  .wire_name = %s\n", config.wire_name );
    fprintf( stderr, "  .ir_name = %s\n", config.ir_name );

    fprintf( stderr, "  .reset = %s\n", config.reset ? "true" : "false" );
    fprintf( stderr, "  .monitor = %s\n", config.monitor ? "true" : "false" );
    fprintf( stderr, "  .batchXfer = %s\n", config.batchXfer ? "true" : "false" );
    fprintf( stderr, "  .state_dir_path = %s\n", config.state_dir_path );
    fprintf( stderr, "  .mod_file_name = %s\n", config.mod_file_name );
    fprintf( stderr, "  .cpu_file_name = %s\n", config.cpu_file_name );
    fprintf( stderr, "  .hdw_file_name = %s\n", config.hdw_file_name );
    fprintf( stderr, "  .rom_file_name = %s\n", config.rom_file_name );
    fprintf( stderr, "  .ram_file_name = %s\n", config.ram_file_name );
    fprintf( stderr, "  .port_1_file_name = %s\n", config.port_1_file_name );
    fprintf( stderr, "  .port_2_file_name = %s\n", config.port_2_file_name );
    fprintf( stderr, "  .hw = %s\n", config.hw );
    fprintf( stderr, "*** /config\n" );
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

    int clopt_model = -1;
    int clopt_verbose = -1;
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
    char* clopt_mod_file_name = "mod";
    char* clopt_cpu_file_name = "cpu";
    char* clopt_hdw_file_name = "hdw";
    char* clopt_rom_file_name = "rom";
    char* clopt_ram_file_name = "ram";
    char* clopt_port_1_file_name = "port1";
    char* clopt_port_2_file_name = "port2";

    const char* optstring = "h";
    struct option long_options[] = {
        {"help",       no_argument,       NULL,              'h'             },
        {"verbose",    no_argument,       &clopt_verbose,    true            },

        {"48sx",       no_argument,       &clopt_model,      MODEL_48SX      },
        {"48gx",       no_argument,       &clopt_model,      MODEL_48GX      },
        {"40g",        no_argument,       &clopt_model,      MODEL_40G       },
        {"49g",        no_argument,       &clopt_model,      MODEL_49G       },

        {"reset",      no_argument,       &clopt_reset,      true            },
        {"monitor",    no_argument,       &clopt_monitor,    true            },
        /* {"batchXfer",  no_argument,       &clopt_batchXfer,  true            }, */
        {"state-dir",  required_argument, NULL,              8999            },
        /* {"mod",        required_argument, NULL,              8000            }, */
        /* {"cpu",        required_argument, NULL,              8010            }, */
        /* {"hdw",        required_argument, NULL,              8020            }, */
        /* {"rom",        required_argument, NULL,              8030            }, */
        /* {"ram",        required_argument, NULL,              8040            }, */
        /* {"port1",      required_argument, NULL,              8050            }, */
        /* {"port2",      required_argument, NULL,              8060            }, */

        {"shiftless",  no_argument,       &clopt_shiftless,  true            },

        {"gui",        no_argument,       &clopt_frontend,   FRONTEND_SDL    },
        {"chromeless", no_argument,       &clopt_chromeless, true            },
        {"fullscreen", no_argument,       &clopt_fullscreen, true            },
        {"scale",      required_argument, NULL,              7110            },

        {"tui",        no_argument,       &clopt_frontend,   FRONTEND_NCURSES},
        {"tui-small",  no_argument,       NULL,              6110            },
        {"tui-tiny",   no_argument,       NULL,              6120            },

        {"mono",       no_argument,       &clopt_mono,       true            },
        {"gray",       no_argument,       &clopt_gray,       true            },

        {0,            0,                 0,                 0               }
    };

    const char* help_text = "usage: %s [options]\n"
                            "options:\n"
                            "  -h --help       what you are reading\n"
                            "     --gui        graphical (SDL2) front-end (default: true)\n"
                            "     --tui        text front-end (default: false)\n"
                            "     --tui-small  text small front-end (2×2 pixels per character) (default: "
                            "false)\n"
                            "     --tui-tiny   text tiny front-end (2×4 pixels per character) (default: "
                            "false)\n"
                            "     --chromeless only show display (default: "
                            "false)\n"
                            "     --fullscreen make the UI fullscreen "
                            "(default: false)\n"
                            "     --scale=<n>  make the UI scale <n> times "
                            "(default: 1.0)\n"
                            "     --mono       make the UI monochrome (default: "
                            "false)\n"
                            "     --gray       make the UI grayscale (default: "
                            "false)\n"
                            "     --48gx       make the GUI looks like a HP 48GX (default: "
                            "auto)\n"
                            "     --48sx       make the GUI looks like a HP 48SX (default: "
                            "auto)\n"
                            "     --shiftless  don't map the shift keys to let them free for numbers (default: "
                            "false)\n";

    while ( c != EOF ) {
        c = getopt_long( argc, argv, optstring, long_options, &option_index );

        switch ( c ) {
            case 'h':
                fprintf( stderr, help_text, config.progname );
                exit( 0 );
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
                /* case 8000: */
                /*     clopt_mod_file_name = optarg; */
                /*     break; */
                /* case 8010: */
                /*     clopt_cpu_file_name = optarg; */
                /*     break; */
                /* case 8020: */
                /*     clopt_hdw_file_name = optarg; */
                /*     break; */
                /* case 8030: */
                /*     clopt_rom_file_name = optarg; */
                /*     break; */
                /* case 8040: */
                /*     clopt_ram_file_name = optarg; */
                /*     break; */
                /* case 8050: */
                /*     clopt_port_1_file_name = optarg; */
                /*     break; */
                /* case 8060: */
                /*     clopt_port_2_file_name = optarg; */
                /*     break; */

            default:
                break;
        }
    }

    if ( optind < argc ) {
        fprintf( stderr, "Invalid arguments : " );
        while ( optind < argc )
            fprintf( stderr, "%s\n", argv[ optind++ ] );
        fprintf( stderr, "\n" );
    }

    /****************************************************/
    /* 2. treat command-line params which have priority */
    /****************************************************/
    if ( clopt_verbose != -1 )
        config.verbose = clopt_verbose == true;
    if ( clopt_model != -1 )
        config.model = clopt_model;
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
    if ( clopt_state_dir_path != NULL )
        config.state_dir_path = strdup( clopt_state_dir_path );
    /* if ( clopt_mod_file_name != NULL ) */
    /*     config.mod_file_name = strdup( clopt_mod_file_name ); */
    /* if ( clopt_cpu_file_name != NULL ) */
    /*     config.cpu_file_name = strdup( clopt_cpu_file_name ); */
    /* if ( clopt_hdw_file_name != NULL ) */
    /*     config.hdw_file_name = strdup( clopt_hdw_file_name ); */
    /* if ( clopt_rom_file_name != NULL ) */
    /*     config.rom_file_name = strdup( clopt_rom_file_name ); */
    /* if ( clopt_ram_file_name != NULL ) */
    /*     config.ram_file_name = strdup( clopt_ram_file_name ); */
    /* if ( clopt_port_1_file_name != NULL ) */
    /*     config.port_1_file_name = strdup( clopt_port_1_file_name ); */
    /* if ( clopt_port_2_file_name != NULL ) */
    /*     config.port_2_file_name = strdup( clopt_port_2_file_name ); */

    config.mod_file_name = normalize_filename( config.state_dir_path, clopt_mod_file_name );
    config.cpu_file_name = normalize_filename( config.state_dir_path, clopt_cpu_file_name );
    config.hdw_file_name = normalize_filename( config.state_dir_path, clopt_hdw_file_name );
    config.rom_file_name = normalize_filename( config.state_dir_path, clopt_rom_file_name );
    config.ram_file_name = normalize_filename( config.state_dir_path, clopt_ram_file_name );
    config.port_1_file_name = normalize_filename( config.state_dir_path, clopt_port_1_file_name );
    config.port_2_file_name = normalize_filename( config.state_dir_path, clopt_port_2_file_name );

    switch ( clopt_model ) {
        case MODEL_40G:
            config.hw = "hp40";
            break;
        case MODEL_49G:
            config.hw = "hp49";
            break;
        case MODEL_48SX:
        case MODEL_48GX:
        default:
            config.hw = "hp48";
            break;
    }

    config.progname = strdup( argv[ 0 ] );

    print_config();

    return &config;
}
