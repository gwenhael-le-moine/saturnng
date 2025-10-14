#ifndef _HDW_H
#  define _HDW_H 1

#  include "types.h"
#  define N_HDW_SIZE 256

typedef struct accel48_t {
    XAddress bs_address; /* Bank Switcher ext. address */
} accel48_t;

typedef struct accel49_t {
    XAddress view[ 2 ]; /* Base of Flash views */
} accel49_t;

typedef struct hdw_t {
    /*
      This structure contains the actual status of all peripheral modules of the
      Saturn CPU. The status of all modules is centralized to allow any device
      to easily access the status of other devices.

        struct HdwModule

  This substructure contains the status of all peripheral devices controlled
  by the hdw module.

  3.2: To support the HP49 hw configuration, the original BusStatus structure
  has been splitted in two:

  - (new) struct BusStatus: contains configuration-independent status
    information (.hdw), and configuration-dependent accelerators (.hdw.accel).
    The overall layout of the structure is the same for all configurations,
    and it is publicily accessible.

  - struct BusStatus_xx (corresponding to all fields of the original
    BusStatus structure except .hdw): contains the storage areas for
    ROM and RAM in configuration 'xx', and is private to the
    configuration-specific ROM/RAM emulation module.
     */
    Nibble registers[ N_HDW_SIZE ]; /* HDW registers */

    /* LCD driver */
    Address lcd_base_addr; /* LCD driver base address */
    int lcd_on;            /* LCD driver enable flag */
    int lcd_contrast;      /* LCD contrast value */
    int lcd_vlc;           /* LCD vertical line count */
    int lcd_offset;        /* LCD horizontal offset */
    int lcd_line_offset;   /* LCD line offset */
    Address lcd_menu_addr; /* LCD menu base address */
    int lcd_ann;           /* LCD annunciators status */

    /* Timers */
    Nibble t1_ctrl; /* Timer 1 control */

    Nibble t2_ctrl; /* Timer 2 control */

    Nibble t1_val; /* Timer 1 value */
    int32 t2_val;  /* Timer 2 value */

    /* 2.4: New member required to support Port emulation */
    Nibble card_status; /* Card status (hdw register 0x0F) */

    /* 2.4: Hw configuration-specific members used as accelerators;
       accel_valid is non-zero if the accelerators are valid
     */
    int accel_valid;

    union {
        accel48_t a48;
        accel49_t a49;
    } accel;

    /* 2.5: Serial port buffer registers */
    Byte serial_rbr;
    Byte serial_tbr;

    /* Misc */
    int16 crc; /* CRC */
} hdw_t;

/*---------------------------------------------------------------------------
  Global variables
  ---------------------------------------------------------------------------*/

extern hdw_t hdw;

extern void hdw_init( void );
extern void hdw_save( void );
extern Nibble hdw_read( Address );
extern void hdw_write( Address, Nibble );

#endif /*!_HDW_H*/
