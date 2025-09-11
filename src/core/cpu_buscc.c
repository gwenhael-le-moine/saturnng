#include "config.h"
#include "cpu.h"
#include "modules.h"
#include "chf_wrapper.h"

/* Instruction Group_80B */
// FIXME: 49g bugs here on display change
static void ExecGroup_80B0( void )
{
    Nibble n = FetchNibble( cpu_status.PC++ );

    switch ( n ) {
        case 0x0:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: RPL2 (preserve carry)" )
            break;
        case 0x3:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: FALSE" )
            break;
        case 0x4:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: DOFALSE" )
            break;
        case 0x5:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: BEEP2" )
            break;
        case 0x6:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: MOVEDOWN" )
            break;
        case 0x7:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: MOVEUP" )
            break;
        case 0x8:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: CREATETEMP" )
            break;
        case 0x9:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: RCKBp" )
            break;
        case 0xA:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: KEYDN" )
            break;
        case 0xB:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: doslow" )
            break;

        case 0x1:
        case 0x2:
        case 0xc:
        case 0xd:
        case 0xe:
        case 0xf:
            cpu_status.PC--;
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: < ?opcode? >" )
            break;

        default:
            /* Unknown opcode */
            ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, cpu_status.PC, n )
            break;
    }
}

static void ExecGroup_80B1( void )
{
    Nibble n = FetchNibble( cpu_status.PC++ );

    switch ( n ) {
        case 0x0: // simulate off function
            break;
        case 0x1:
            cpu_status.PC += 2;
            // do not do gettime, just skip the RTN after it to fall in the normal gettime function (only valid in untouched ROM)
            break;
        case 0x2:
            // do not do settime, fall in the normal settime function (only valid in untouched ROM)
            break;
        case 0x3:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: RESETOS" )
            break;
        case 0x4:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: AUTOTEST" )
            break;
        case 0x5:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: NATIVE?" )
            break;
        case 0x7:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: SERIAL" )
            break;

        case 0x6:
        case 0x8:
        case 0x9:
        case 0xa:
        case 0xb:
        case 0xc:
        case 0xd:
        case 0xe:
        case 0xf:
            cpu_status.PC--;
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: < ?opcode? >" )
            break;

        default:
            /* Unknown opcode */
            ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, cpu_status.PC, n )
            break;
    }
}

static void ExecGroup_80B2( void )
{
    Nibble n = FetchNibble( cpu_status.PC++ );

    switch ( n ) {
        case 0x8:
            // cpu_status.HST |= I[5]; // Emu48:apple.c:500
            cpu_status.PC += 1;
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: HST=1.x" )
            break;
        case 0x9: // screen height = 0x50 = 80 or 0x40 = 64
            cpu_status.A[ 4 ] = cpu_status.A[ 3 ] = cpu_status.A[ 2 ] = cpu_status.A[ 0 ] = 0;
            cpu_status.A[ 1 ] = config.model == MODEL_49G ? 5 : 4;
            break;
        case 0xA: // screen width = 0x83 = 131
            cpu_status.A[ 4 ] = cpu_status.A[ 3 ] = cpu_status.A[ 2 ] = 0;
            cpu_status.A[ 1 ] = 8;
            cpu_status.A[ 0 ] = 3;
            break;
        case 0xB: // it is medium apple
            cpu_status.carry = false;
            break;
        case 0xC: // it is big apple
            cpu_status.carry = config.model == MODEL_49G;
            break;
        case 0xE: // it is big apple V2
            cpu_status.carry = config.model == MODEL_49G;
            break;

        case 0x0:
        case 0x1:
        case 0x2:
        case 0x3:
        case 0x4:
        case 0x5:
        case 0x6:
        case 0x7:
        case 0xd:
        case 0xf:
            cpu_status.PC--;
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: < ?opcode? >" )
            break;

        default:
            /* Unknown opcode */
            ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, cpu_status.PC, n )
            break;
    }
}

static void ExecGroup_80B3( void )
{
    Nibble n = FetchNibble( cpu_status.PC++ );

    switch ( n ) {
        case 0x0:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: config_disp0 Ca:address 4K data" )
            break;
        case 0x1:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: unconfig_disp0 does the refresh" )
            break;
        case 0x2:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: refresh_disp0 force refresh" )
            break;
        case 0x3:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: set_lines_disp0 nb in Cb" )
            break;
        case 0x4:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: set_offset_disp0 offset to disp in disp0" )
            /* w.d0offset = Npack(w.C, 5); */
            /* w.d0offset &= 0x7FF; */
            break;
        case 0x5:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: native_get_line_disp0" )
            /* Nunpack(w.C, w.d0offset, 5); */
            break;
        case 0x8:
            // cpu_status.HST |= I[5]; // Emu48:apple.c:500
            cpu_status.PC += 3;
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: ?HST=1.x" )
            break;

        case 0x6:
        case 0x7:
        case 0x9:
        case 0xa:
        case 0xb:
        case 0xc:
        case 0xd:
        case 0xe:
        case 0xf:
            cpu_status.PC--;
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: < ?opcode? >" )
            break;

        default:
            /* Unknown opcode */
            ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, cpu_status.PC, n )
            break;
    }
}

static void ExecGroup_80B4( void )
{
    Nibble n = FetchNibble( cpu_status.PC++ );

    switch ( n ) {
        case 0x0:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: setup basic memory configuration" )
            break;
        case 0x1:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: erase Flash bank" )
            break;
        case 0x2:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: write byte into Flash bank" )
            break;
        case 0x3:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: format Flash bank" )
            break;

        case 0x4:
        case 0x5:
        case 0x6:
        case 0x7:
        case 0x8:
        case 0x9:
        case 0xa:
        case 0xb:
        case 0xc:
        case 0xd:
        case 0xe:
        case 0xf:
            cpu_status.PC--;
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: < ?opcode? >" )
            break;

        default:
            /* Unknown opcode */
            ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, cpu_status.PC, n )
            break;
    }
}

static void ExecGroup_80B5( void )
{
    Nibble n = FetchNibble( cpu_status.PC++ );
    switch ( n ) {
        case 0x0:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: REMON" )
            break;
        case 0x1:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: REMOFF" )
            break;
        case 0x6:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: OUTBYT" )
            break;
        case 0x7:
            cpu_status.D0 = cpu_status.D1 = 0;
            break;

        case 0x2:
        case 0x3:
        case 0x4:
        case 0x5:
        case 0x8:
        case 0x9:
        case 0xa:
        case 0xb:
        case 0xc:
        case 0xd:
        case 0xe:
        case 0xf:
            cpu_status.PC--;
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: < ?opcode? >" )
            break;

        default:
            /* Unknown opcode */
            ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, cpu_status.PC, n )
            break;
    }
}

static void ExecGroup_80B6( void )
{
    Nibble n = FetchNibble( cpu_status.PC++ );

    switch ( n ) {
        case 0x0:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: ACCESSSD" )
            break;
        case 0x1:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: PORTTAG?" )
            break;
        case 0x4:
            cpu_status.carry = false; /* There is no SD card present */
            break;
        case 0x6:
            cpu_status.carry = false; /* Could not format SD (non-existent) card */
            break;

        case 0x2:
        case 0x3:
        case 0x5:
        case 0x7:
        case 0x8:
        case 0x9:
        case 0xa:
        case 0xb:
        case 0xc:
        case 0xd:
        case 0xe:
        case 0xf:
            cpu_status.PC--;
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: < ?opcode? >" )
            break;

        default:
            /* Unknown opcode */
            ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, cpu_status.PC, n )
            break;
    }
}

static void ExecGroup_80B7( void )
{
    Nibble n = FetchNibble( cpu_status.PC++ );

    switch ( n ) {
        case 0xF:
            cpu_status.carry = false;
            cpu_status.PC++;
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: SETFLDn" )
            break;

        case 0x0:
        case 0x1:
        case 0x2:
        case 0x3:
        case 0x4:
        case 0x5:
        case 0x6:
        case 0x7:
        case 0x8:
        case 0x9:
        case 0xa:
        case 0xb:
        case 0xc:
        case 0xd:
        case 0xe:
            cpu_status.PC--;
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: < ?opcode? >" )
            break;

        default:
            /* Unknown opcode */
            ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, cpu_status.PC, n )
            break;
    }
}

static void ExecGroup_80B8( void )
{
    Nibble n = FetchNibble( cpu_status.PC++ );

    switch ( n ) {
        case 0x0:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: r=s" )
            break;
        case 0x1:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: r=r+s" )
            break;
        case 0x2:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: r=r-s" )
            break;
        case 0x3:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: r=r*s" )
            break;
        case 0x4:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: r=r/s" )
            break;
        case 0x5:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: r=r%s" )
            break;
        case 0x6:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: r=-r-1" )
            break;
        case 0x7:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: r=-r" )
            break;
        case 0x8:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: r=r<s" )
            break;
        case 0x9:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: r=r>s" )
            break;
        case 0xA:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: r=r^s" )
            break;

        case 0xb:
        case 0xc:
        case 0xd:
        case 0xe:
        case 0xf:
            cpu_status.PC--;
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: < ?opcode? >" )
            break;

        default:
            /* Unknown opcode */
            ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, cpu_status.PC, n )
            break;
    }
}

static void ExecGroup_80B9( void )
{
    Nibble n = FetchNibble( cpu_status.PC++ );

    switch ( n ) {
        case 0x0:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: Data streamer" )
            break;

        case 0x1:
        case 0x2:
        case 0x3:
        case 0x4:
        case 0x5:
        case 0x6:
        case 0x7:
        case 0x8:
        case 0x9:
        case 0xa:
        case 0xb:
        case 0xc:
        case 0xd:
        case 0xe:
        case 0xf:
            cpu_status.PC--;
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: < ?opcode? >" )
            break;

        default:
            /* Unknown opcode */
            ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, cpu_status.PC, n )
            break;
    }
}

static void ExecGroup_80BE( void )
{
    Nibble n = FetchNibble( cpu_status.PC++ );

    switch ( n ) {
        case 0xE:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: ARMFLUSH" )
            break;
        case 0xF:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: ARMSYS" )
            break;

        case 0x0:
        case 0x1:
        case 0x2:
        case 0x3:
        case 0x4:
        case 0x5:
        case 0x6:
        case 0x7:
        case 0x8:
        case 0x9:
        case 0xa:
        case 0xb:
        case 0xc:
        case 0xd:
            cpu_status.PC--;
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: < ?opcode? >" )
            break;

        default:
            /* Unknown opcode */
            ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, cpu_status.PC, n )
            break;
    }
}

static void ExecGroup_80BF( void )
{
    Nibble n = FetchNibble( cpu_status.PC++ );

    switch ( n ) {
        case 0xF:
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: ARMSAT" )
            break;

        case 0x0:
        case 0x1:
        case 0x2:
        case 0x3:
        case 0x4:
        case 0x5:
        case 0x6:
        case 0x7:
        case 0x8:
        case 0x9:
        case 0xa:
        case 0xb:
        case 0xc:
        case 0xd:
        case 0xe:
            cpu_status.PC--;
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: < ?opcode? >" )
            break;

        default:
            /* Unknown opcode */
            ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, cpu_status.PC, n )
            break;
    }
}

void ExecGroup_80B( void )
{
    Nibble n = FetchNibble( cpu_status.PC++ );

    switch ( n ) {
        case 0x0:
            ExecGroup_80B0();
            break;
        case 0x1:
            ExecGroup_80B1();
            break;
        case 0x2:
            ExecGroup_80B2();
            break;
        case 0x3:
            ExecGroup_80B3();
            break;
        case 0x4:
            ExecGroup_80B4();
            break;
        case 0x5:
            ExecGroup_80B5();
            break;
        case 0x6:
            ExecGroup_80B6();
            break;
        case 0x7:
            ExecGroup_80B7();
            break;
        case 0x8:
            ExecGroup_80B8();
            break;
        case 0x9:
            ExecGroup_80B9();
            break;
        case 0xE:
            ExecGroup_80BE();
            break;
        case 0xF:
            ExecGroup_80BF();
            break;

        case 0xA:
        case 0xB:
        case 0xC:
        case 0xD:
            cpu_status.PC--;
            DEBUG( CPU_CHF_MODULE_ID, DEBUG_C_IMPLEMENTATION, CPU_I_CALLED, "Not implemented: < 80B%i >", n )
            break;

        default:
            /* Unknown opcode */
            ERROR( CPU_CHF_MODULE_ID, CPU_E_BAD_OPCODE, cpu_status.PC, n )
            break;
    }
}
