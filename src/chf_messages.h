#ifndef _CHF_MESSAGES_H
#define _CHF_MESSAGES_H 1

#include <stdlib.h>

#include <Chf.h>
// #include "../libChf/src/Chf.h"

/* Chf Module Identifiers:
   Each main module of the emulator has its own Chf Module Identifier; the
   values defined here must match those actually used in the message catalogs.
*/
#define CPU_CHF_MODULE_ID 11
#define DEBUG_CHF_MODULE_ID 30
#define DISK_IO_CHF_MODULE_ID 13
#define FLASH_CHF_MODULE_ID 16 /* 3.3 */
#define MOD_CHF_MODULE_ID 12
#define MAIN_CHF_MODULE_ID 10
#define SERIAL_CHF_MODULE_ID 15 /* 2.5 */
#define X_FUNC_CHF_MODULE_ID 18 /* 3.13 */

/*---------------------------------------------------------------------------
        Chf condition codes
  ---------------------------------------------------------------------------*/
// cpu.h
#define CPU_I_CALLED 101          /* Function %s called */
#define CPU_I_EXECUTING 102       /* Executing @PC %X */
#define CPU_I_SHUTDN 103          /* Shutdown */
#define CPU_I_WAKE 104            /* Wake */
#define CPU_I_INT 105             /* %s request accepted */
#define CPU_I_INT_PENDING 106     /* %s request pending */
#define CPU_I_RTI_LOOP 107        /* RTI loop to service %s */
#define CPU_I_RTI_END 108         /* RTI returning */
#define CPU_I_INTON 109           /* INTON servicing %s */
#define CPU_I_REVISION 110        /* CPU emulation revision: %s */
#define CPU_I_TIMER1_EX 111       /* Timer 1 expired; ctrl=%x */
#define CPU_I_TIMER2_EX 112       /* Timer 1 expired; ctrl=%x */
#define CPU_I_EMULATOR_INT 113    /* Emulator interrupt req. detected */
#define CPU_I_TIMER_ST 114        /* 3.1: Timer %s st: ctrl %x, val %x */
#define CPU_I_TIMER_EXP 115       /* 3.1: Timer %s expiration %d ms */
#define CPU_I_IDLE_X_LOOP 116     /* 3.1: Start idle loop, t/out %d ms */
#define CPU_I_ELAPSED 117         /* 3.1: Spent %d us in idle loop */
#define CPU_I_HALT 118            /* 3.13: CPU halted */
#define CPU_I_RUN 119             /* 3.13: CPU running */
#define CPU_W_RESETTING 201       /* Resetting CPU */
#define CPU_W_BAD_MONITOR_CMD 202 /* Bad monitor command: %s */
#define CPU_E_BAD_OPCODE 301      /* Bad opc. pc=%x, value=%x */
#define CPU_E_SAVE 302            /* Can't save CPU status */
#define CPU_E_NO_HALT 303         /* 3.13: Halt/Run not allowed */
#define CPU_F_INTERR 401          /* Internal error %s */
#define CPU_F_BAD_SHUTDN 402      /* Unexpected CPU shutdown */

// debug.h
#define DEBUG_W_NOT_SUPPORTED 201 /* Debug not supported */
#define DEBUG_W_BAD_CMD 202       /* Invalid command */

// disk_io.h
#define DISK_IO_S_OK 0        /* Function completed succesfully */
#define DISK_IO_I_CALLED 101  /* Function %s called */
#define DISK_IO_E_OPEN 401    /* Open file %s failed */
#define DISK_IO_E_GETC 402    /* getc() from file %s failed */
#define DISK_IO_E_PUTC 403    /* putc() to file %s failed */
#define DISK_IO_E_READ 404    /* fread() from file %s failed */
#define DISK_IO_E_WRITE 405   /* fwrite() to file %s failed */
#define DISK_IO_E_CLOSE 406   /* Close file %s failed */
#define DISK_IO_E_BAD_HDR 407 /* File %s has a bad header */
#define DISK_IO_E_SIZE 408    /* File %s too large */

// flash49.h
#define FLASH_I_READ 101        /* Read from address %x: %d */
#define FLASH_I_WRITE 102       /* Write address %x, datum %x */
#define FLASH_I_FSM 103         /* FSM from state %d, cycle %d */
#define FLASH_I_FSM_AD 104      /* FSM address %x, data %x */
#define FLASH_I_FSM_RESULT 105  /* FSM next state %d, result %x */
#define FLASH_I_FSM_OP 106      /* FSM operation %s */
#define FLASH_W_BAD_CMD 201     /* Bad cmd st%d, cycle%d, a%x, d%d */
#define FLASH_W_BAD_ADDRESS 202 /* Bad addr st%d, cycle%d, a%x, d%d */
#define FLASH_E_xxx 301
#define FLASH_F_xxx 401

// modules.h
#define MOD_I_CALLED 101            /* Function %s called */
#define MOD_I_INITIALIZING 102      /* Initializing module %s */
#define MOD_I_RESETTING 103         /* Resetting module %s */
#define MOD_I_GET_ID 106            /* ModGetID returning %x */
#define MOD_I_CONFIG 107            /* ModConfig %s %x %x completed */
#define MOD_I_UNCONFIG 108          /* ModUnconfig %s %x %x completed */
#define MOD_I_SAVING 109            /* Saving status of module %s */
#define MOD_I_NOT_IMPLEMENTED 110   /* Function %s not implemented */
#define MOD_I_REVISION 111          /* Modules revision: %s */
#define MOD_I_BS_ADDRESS 112        /* 2.4: Bank Switcher address: %x */
#define MOD_I_PORT_1_WP 113         /* 2.4: Port 1 is write protected */
#define MOD_I_PORT_2_WP 114         /* 2.4: Port 2 is write protected */
#define MOD_I_PERF_CTR 115          /* 2.7: Value of PerfCtr %s is %d */
#define MOD_I_CACHED_UNCONFIG 116   /* 2.7: Cached ModUnconfig completed */
#define MOD_I_CACHED_CONFIG 117     /* 2.7: Cached ModConfig %x comp. */
#define MOD_I_UNCONFIG_L_HIT 118    /* 2.7: Late unconfig hit */
#define MOD_I_UNCONFIG_L_MISS 119   /* 2.7: Late unconfig miss */
#define MOD_W_BAD_CONFIG 202        /* Bad ModConfig %x ignored */
#define MOD_W_BAD_UNCONFIG 203      /* Bad ModUnconfig %x ignored */
#define MOD_W_HDW_WRITE 204         /* Bad HdwWrite %x, %x */
#define MOD_W_HDW_READ 205          /* Bad HdwRead %x */
#define MOD_W_RESETTING_ALL 206     /* Resetting all modules */
#define MOD_W_RAM_INIT 207          /* Can't initialize internal RAM */
#define MOD_W_HDW_INIT 208          /* Can't initialize HDW */
#define MOD_W_BAD_KEY 209           /* 2.1: Bad key %s ignored */
#define MOD_W_BAD_OUT_BIT 210       /* 2.1: Bad out_bit %x ignored */
#define MOD_W_PORT_1_INIT 211       /* 2.4: Can't initialize Port 1 */
#define MOD_W_PORT_2_INIT 212       /* 2.4: Can't initialize Port 2 */
#define MOD_W_NO_VICTIM 213         /* 2.7: No cache victim; flush/retry */
#define MOD_E_BAD_READ 301          /* Read unmapped addr %x */
#define MOD_E_BAD_WRITE 302         /* Write unmapped addr %x datum %x */
#define MOD_E_ROM_WRITE 303         /* Write into ROM addr %x datum %x */
#define MOD_E_RAM_SAVE 304          /* Can't save internal RAM status */
#define MOD_E_HDW_SAVE 305          /* Can't save HDW status */
#define MOD_E_PORT_1_SAVE 306       /* 2.4: Can't save Port 1 status */
#define MOD_E_CE1_WRITE 307         /* 2.4: Ce1Write addr %x datum %x */
#define MOD_E_PORT_2_SAVE 308       /* 2.4: Can't save Port 2 status */
#define MOD_E_NCE3_READ 309         /* 2.4: Read from NCE3 addr %x */
#define MOD_E_NCE3_WRITE 310        /* 2.4: Wr. to NCE3 addr %x datum %x */
#define MOD_E_NO_MATCH 311          /* 3.2: Hw desription %s not found */
#define MOD_E_ROM_SAVE 312          /* 3.3: Can't save Flash ROM */
#define MOD_F_MAP_SAVE 401          /* Can't save mod_map information */
#define MOD_F_ROM_INIT 402          /* Can't initialize internal ROM */
#define MOD_F_MAP_ALLOC 403         /* Dynamic map allocation failed */
#define MOD_F_BAD_ALLOC_C 404       /* 2.7: Bad alloc_c %d aft FlushCache*/
#define MOD_F_CHAIN_CORRUPTED 405   /* 2.7: ModMap chain corrupted */
#define MOD_F_NO_VICTIM 406         /* 2.7: No cache victim after flush */
#define MOD_F_MOD_STATUS_ALLOC 407  /* 3.2: ModStatus_xx alloc failed %d */
#define MOD_F_NO_DESCRIPTION 408    /* 3.2: No module description */
#define MOD_M_NOT_MAPPED 501        /* Address %x not mapped */
#define MOD_M_MAPPED 502            /* Address %x mapped to %s:%x */
#define MOD_M_MAP_TABLE_TITLE 503   /* */
#define MOD_M_MAP_TABLE_ROW 504     /* %s %x %x %s */
#define MOD_M_MAP_CONFIGURED 505    /* Configured */
#define MOD_M_MAP_SZ_CONFIGURED 506 /* Size configured */
#define MOD_M_MAP_UNCONFIGURED 507  /* Unconfigured */

// main.c
#define MAIN_M_COPYRIGHT 501
#define MAIN_M_LICENSE 502

// serial.h
#define SERIAL_I_CALLED 101      /* Function %s called */
#define SERIAL_I_REVISION 102    /* Serial port emulation rev. %s */
#define SERIAL_I_READ 103        /* Read %s -> %x */
#define SERIAL_I_WRITE 104       /* Write %s %x -> %x */
#define SERIAL_I_RBR 105         /* Read RBR -> %x */
#define SERIAL_I_TBR 106         /* Write TBR <- %x */
#define SERIAL_I_PTY_NAME 107    /* pty name is %s */
#define SERIAL_W_EMPTY_RRB 201   /* Read from empty RX buffer, rcs=%x */
#define SERIAL_W_FULL_TRB 202    /* Write into full TX buffer, tcs=%x */
#define SERIAL_W_NOPTY 203       /* 3.16: Pty support not available */
#define SERIAL_E_TRB_DRAIN 301   /* Error draining TX buffer */
#define SERIAL_E_RRB_CHARGE 302  /* Error charging RX buffer */
#define SERIAL_E_PTY_CLOSE 303   /* Error closing pty */
#define SERIAL_F_OPENPTY 401     /* openpty() failed on master pty */
#define SERIAL_F_FCNTL 402       /* fcntl() failed on master pty */
#define SERIAL_F_OPEN_MASTER 403 /* Can't open pty master %s */
#define SERIAL_F_GRANTPT 404     /* grantpt() failed on master pty */
#define SERIAL_F_UNLOCKPT 405    /* unlockpt() failed on master pty */
#define SERIAL_F_OPEN_SLAVE 406  /* Can't open pty slave %s */
#define SERIAL_F_PUSH 407        /* ioctl(I_PUSH,%s) failed on slave */
#define SERIAL_F_TCGETATTR 408   /* tcgetattr() failed on master */
#define SERIAL_F_TCSETATTR 409   /* tcsetattr() failed on master */

// x_func.h
#define X_FUNC_I_CALLED 101     /* Function %s called */
#define X_FUNC_I_CODE 102       /* Function code %d */
#define X_FUNC_I_SET_SPEED 103  /* Speed set to %dMhz (%d mult.) */
#define X_FUNC_I_MAX_SPEED 104  /* Emulator at max speed */
#define X_FUNC_I_FILE_NAME 105  /* Transferring file name %s */
#define X_FUNC_I_KGET 106       /* Kget start:%x end:%x hdr:%s */
#define X_FUNC_I_SEND 107       /* Send start:%x end:%x hdr:%s */
#define X_FUNC_W_BAD_CODE 201   /* Bad function code %d ignored */
#define X_FUNC_W_ABORTED 202    /* Aborted by user */
#define X_FUNC_W_FAILED 203     /* Operation failed */
#define X_FUNC_E_NO_HALT 301    /* Cpu halt not allowed */
#define X_FUNC_E_NO_SPEED 302   /* No speed control available */
#define X_FUNC_E_NO_BIN_HDR 303 /* Can't determine hdr for hw %s */
#define X_FUNC_F_xxx 401
#define X_FUNC_M_KGET 501 /* FSB title for Kget function */
#define X_FUNC_M_SEND 502 /* FSB title for Send function */

extern ChfTable message_table[];
extern size_t message_table_size;

#endif /*!_CHF_MESSAGES_H*/
