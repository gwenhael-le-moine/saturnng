#ifndef _UI4x_CONFIG_H
#  define _UI4x_CONFIG_H 1

#  include <stdbool.h>

#  include "ui4x/api.h"

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
    double scale;

    bool tiny;
    bool small;

    char* wire_name;
    char* ir_name;

    bool verbose;

    /* options below are specific to saturnng */
    bool throttle;

    bool reset;   /* 2.1: Force emulator reset */
    bool monitor; /* 2.1: Call monitor() on startup */
    int speed;

    int debug_level;

    char* datadir;

    char* config_path;
    char* bus_path;
    char* cpu_path;
    char* hdw_path;
    char* rom_path;
    char* ram_path;
    char* port1_path;
    char* port2_path;
} config_t;

/*---------------------------------------------------------------------------
        Global variables
  ---------------------------------------------------------------------------*/
extern config_t config;

/*************/
/* functions */
/*************/
extern config_t* config_init( int argc, char* argv[] );

#endif /* !_UI4x_CONFIG_H */
