#ifndef _UI4x_CONFIG_H
#  define _UI4x_CONFIG_H 1

#  include <stdbool.h>

#  include "ui4x/src/api.h"

#  ifndef VERSION_MAJOR
#    define VERSION_MAJOR 0
#  endif
#  ifndef VERSION_MINOR
#    define VERSION_MINOR 0
#  endif
#  ifndef PATCHLEVEL
#    define PATCHLEVEL 0
#  endif

#  ifdef X50NG_DATADIR
#    define GLOBAL_DATADIR X50NG_DATADIR
#  else
#    define GLOBAL_DATADIR opt.progpath
#  endif

#  define CONFIG_FILE_NAME "config.lua"
#  define BUS_FILE_NAME "mod"
#  define CPU_FILE_NAME "cpu"
#  define HDW_FILE_NAME "hdw"
#  define ROM_FILE_NAME "rom"
#  define RAM_FILE_NAME "ram"
#  define PORT1_FILE_NAME "port1"
#  define PORT2_FILE_NAME "port2"

typedef struct config_t {
    /* duplicating ui4x_config_t here so that config_init can return one big struct */
    ui4x_model_t model;
    bool shiftless;
    bool black_lcd;
    bool newrpl_keyboard;

    ui4x_frontend_t frontend;
    bool mono;
    bool gray;

    bool chromeless;
    bool fullscreen;

    bool tiny;
    bool small;

    bool verbose;

    double zoom;
    bool netbook;
    int netbook_pivot_line;

    char* name;
    char* progname;
    char* progpath;
    char* wire_name;
    char* ir_name;

    char* datadir;
    char* style_filename;

    char* sd_dir;

    /* options below are specific to saturnng */
    bool throttle;

    bool reset;   /* 2.1: Force emulator reset */
    bool monitor; /* 2.1: Call monitor() on startup */
    int speed;

    int debug_level;
} config_t;

/*---------------------------------------------------------------------------
        Global variables
  ---------------------------------------------------------------------------*/
extern config_t config;

/*************/
/* functions */
/*************/
extern char* path_file_in_datadir( const char* filename );

extern config_t* config_init( int argc, char* argv[] );

#endif /* !_UI4x_CONFIG_H */
