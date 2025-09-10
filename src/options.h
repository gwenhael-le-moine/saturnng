#ifndef _UI4x_CONFIG_H
#  define _UI4x_CONFIG_H 1

#  include <stdbool.h>

typedef enum { FRONTEND_SDL, FRONTEND_NCURSES, FRONTEND_GTK } frontend_t;

typedef enum { MODEL_48SX = 485, MODEL_48GX = 486, MODEL_40G = 406, MODEL_49G = 496, MODEL_50G = 506 } model_t;

typedef struct {
    char* progname;

    model_t model;
    bool throttle;
    bool verbose;
    bool shiftless;
    bool big_screen;
    bool black_lcd;

    frontend_t frontend;
    bool mono;
    bool gray;

    bool chromeless;
    bool fullscreen;
    double scale;

    bool tiny;
    bool small;

    char* wire_name;
    char* ir_name;

    /* from args.h */
    bool reset;   /* 2.1: Force emulator reset */
    bool monitor; /* 2.1: Call monitor() on startup */

    char* state_dir_path;

    char* config_path;
    char* mod_path;
    char* cpu_path;
    char* hdw_path;
    char* rom_path;
    char* ram_path;
    char* port1_path;
    char* port2_path;

    int debug_level;
    bool enable_BUSCC;
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
