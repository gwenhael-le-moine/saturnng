#ifndef _UI4x_CONFIG_H
#  define _UI4x_CONFIG_H 1

#  include <stdbool.h>

#  include "ui4x/api.h"

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
    char* progname;

    ui4x_model_t model;
    bool shiftless;
    bool big_screen;
    bool black_lcd;

    ui4x_frontend_t frontend;
    bool mono;
    bool gray;

    bool chromeless;
    bool fullscreen;
    double zoom;

    bool tiny;
    bool small;

    char* wire_name;
    char* ir_name;

    char* style_filename;

    bool verbose;

    /* options below are specific to saturnng */
    bool throttle;

    bool reset;   /* 2.1: Force emulator reset */
    bool monitor; /* 2.1: Call monitor() on startup */
    int speed;

    int debug_level;

    char* datadir;
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
