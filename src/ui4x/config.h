#ifndef _UI4x_CONFIG_H
#define _UI4x_CONFIG_H 1

#include <stdbool.h>

#define FRONTEND_SDL 1
#define FRONTEND_NCURSES 2

#define MODEL_48SX 485
#define MODEL_48GX 486
#define MODEL_40G 406
#define MODEL_49G 496

typedef struct {
    char* progname;

    int model;
    bool throttle;
    bool verbose;
    bool shiftless;
    bool big_screen;
    bool black_lcd;

    int frontend;
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
    bool reset;     /* 2.1: Force emulator reset */
    bool monitor;   /* 2.1: Call monitor() on startup */
    bool batchXfer; /* 3.15: Non-interactive file transfers */
    char* state_dir_path;
    char* config_file_name;

    char* mod_file_name;
    char* cpu_file_name;
    char* hdw_file_name;
    char* rom_file_name;
    char* ram_file_name;
    char* port_1_file_name;
    char* port_2_file_name;

    int debug_level;
    bool implement_BUSCC;
} config_t;

/*************/
/* functions */
/*************/
extern config_t* config_init( int argc, char* argv[] );

#endif /* !_UI4x_CONFIG_H */
