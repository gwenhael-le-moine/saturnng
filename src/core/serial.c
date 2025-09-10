/* -------------------------------------------------------------------------
   saturn - A poor-man's emulator of some HP calculators
   Copyright (C) 1998-2000 Ivan Cibrario Bertolotti

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the documentation of this program; if not, write to
   the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

   For more information, please contact the author, preferably by email,
   at the following address:

   Ivan Cibrario Bertolotti
   IRITI - National Research Council
   c/o IEN "Galileo Ferraris"
   Strada delle Cacce, 91
   10135 - Torino (ITALY)

   email: cibrario@iriti.cnr.it
   ------------------------------------------------------------------------- */

/* .+
.author       : Ivan Cibrario B.

.creation     :	13-Sep-2000

.description  :

    SASM.DOC by HP  (HORN disk 4)
    Guide to the Saturn Processor Rev. 0.00f and 1.0b by Matthew Mastracci
    entries.srt by Mika Heiskanen  (mheiskan@vipunen.hut.fi)
    x48 source code by Eddie C. Dost  (ecd@dressler.de)
    Emu48 source code by Sebastien Cariler

.notes        :
  $Log: serial.c,v $
  Revision 4.1  2000/12/11 09:54:19  cibrario
  Public release.

  Revision 3.17  2000/11/23 17:01:45  cibrario
  Implemented sutil library and assorted bug fixes:
  - Fixed UpdateTCS macro; it gave wrong results when trb was full
  - in SerialInit(), ensure that the pty is fully transparent by default
  - in SerialInit(), slave pty must have O_NONBLOCK set, otherwise
    SerialClose() could hang (rare)
  - in HandleSerial(), transmit ring buffer must be emptied even
    when !IOC_SON

  Revision 3.16  2000/11/21 16:41:08  cibrario
  Ultrix/IRIX support:
  - Added sgi/IRIX support (USE_STREAMSPTY)
  - Added fallback, dummy pty implementation

  Revision 3.10  2000/10/24 16:14:58  cibrario
  Added/Replaced GPL header

  Revision 3.5  2000/10/02 09:51:21  cibrario
  Linux support:
  - libc6 >= 2.0 has openpty()

  Revision 3.2  2000/09/22 14:34:55  cibrario
  Implemented preliminary support of HP49 hw architecture:
  - simplified handling of
    RCS_RBZ and RCS_RBF bits of RCS register.
  - disabled local ECHO on master
    pty when USE_OPENPTY is in effect; this avoid spurious rx
    when no process is connected to the slave pty yet. Apparently,
    USE_STREAMSPTY does not suffer from this.
  - removed warning message
    when reading from an empty RRB.

  Revision 2.6  2000/09/15  09:23:02  cibrario
  - Implemented USE_STREAMSPTY (needed to build saturn on Solaris)
  - Avoided name clash on ADDRESS_MASK when including pty headers
    (Digital UNIX platform)
  - Enhanced documentation of public functions

  Revision 2.5  2000/09/14  15:44:08  cibrario
  *** empty log message ***
.- */

#include <stdlib.h>
#include <errno.h>
#include "../libChf/src/Chf.h"

#include "config.h"
#include "machdep.h"
#include "cpu.h"
#include "serial.h" /* 2.5: Serial port emulation module */
#include "debug.h"

/*---------------------------------------------------------------------------
        Determine pty implementation

  Currently, the following two implementations are supported:

  - openpty(); available on Digital UNIX and Linux
  - STREAMS pseudo-tty driver (/dev/ptmx); available on Solaris 2.6

  SERIAL_FORCE_OPENPTY and SERIAL_FORCE_STREAMSPTY can be defined in config.h
  to force this module to use a particular implementation; otherwise,
  a (hopefully) appropriate implementation will be automatically selected
  by the cpp code that follows, depending on the build platform.

  At the end of the cpp code below, exactly one of the macros
  USE_OPENPTY and USE_STREAMSPTY will be defined; they should be used
  to conditionally select the implementation-dependent portions of code.

  ---------------------------------------------------------------------------*/

#ifdef SERIAL_FORCE_OPENPTY
#  define USE_OPENPTY
#  define FORCED
#endif

#ifdef SERIAL_FORCE_STREAMSPTY
#  define USE_STREAMSPTY
#  define FORCED
#endif

/* 3.5: Added linux support (__linux__ cpp macro) */
#if ( defined( __osf__ ) || defined( __linux__ ) ) && !defined( FORCED )
#  define USE_OPENPTY
#endif

/* 3.16: Added sgi support (__sgi cpp macro) */
#if ( defined( __sun__ ) || defined( __sgi ) ) && !defined( FORCED )
#  define USE_STREAMSPTY
#endif

/* 3.16: If no appropriate pty implementation has been found,
   throw a dummy implementation in.
*/
#if !defined( USE_OPENPTY ) && !defined( USE_STREAMSPTY )
#  define USE_NOPTY
#endif

#undef FORCED

/*---------------------------------------------------------------------------
        Include pty implementation-specific headers and definitions
  ---------------------------------------------------------------------------*/

#ifdef USE_OPENPTY
#  undef ADDRESS_MASK  /* 2.6: Avoid name clash 8-( */
#  include <fcntl.h>   /* fcntl() */
#  include <unistd.h>  /* ttyname() */
#  include <pty.h>     /* openpty() */
#  include <termios.h> /* tcgetattr()/tcsetattr() */
#  include <sys/termios.h>
#  include <sys/ioctl.h>
#endif

#ifdef USE_STREAMSPTY
#  undef ADDRESS_MASK /* 2.6: Avoid name clash 8-( */
/* stdlib.h already included */
#  include <fcntl.h> /* open(), fcntl() */
#  include <unistd.h>
#  include <termios.h>           /* tcgetattr()/tcsetattr() */
#  include <stropts.h>           /* ioctl() */
#  define PTY_MASTER "/dev/ptmx" /* Master cloning device */
#endif

/*---------------------------------------------------------------------------
        Private implementation of ring buffers

  The following data type definitions, macros, and functions together
  implement the ring buffers used to hold serial data being transmitted
  and received.  Multibyte pushes and pops are efficiently supported:
  the whole ring buffer can be filled/emptied completely with only
  two, non-overlapping memcpy(), read() or write() operations.

  ---------------------------------------------------------------------------*/

#define RB_SIZE 1024 /* Buffer size (# of characters) */

/* Hello, I am a RingBuffer... 8-} */
struct RingBuffer {
    int n;                /* Number of full slots: 0 <= n <= RB_SIZE */
    int8 *rp, *wp;        /* Read/Write pointers */
    int8* ep;             /* Pointer to the end of .data[] */
    int8 data[ RB_SIZE ]; /* Buffer storage */
};

/* Basic macros:

   Min(a, b)		returns the minimum (as told by '<') between a and b;
                        warning: evaluates a and b twice.

   ReadPointer(rb)	returns the read pointer of a given buffer; it
                        points to ContFullSlots() full buffer slots.

   FullSlots(rb)	returns the number of full slots of a given buffer;
                        the slots are not necessarily contiguous.

   ContFullSlots(rb)	returns the number of *contiguous* full slots of a
                        given buffer, starting at the current ReadPointer;
                        this macro is guaranteed to return a strictly
                        positive value if the buffer is not empty.

   EmptySlots(rb)	returns the number of empty slots of a given buffer;
                        the slots are not necessarily contiguous.

   ContEmptySlots(rb)	returns the number of *contiguous* empty slots of a
                        given buffer, starting at the current WritePointer;
                        this macro is guaranteed to return a strictly
                        positive value if the buffer is not full.

   UpdateReadPointer(rb, n)
                        moves the read pointer of ring buffer rb n slots
                        forward

   UpdateWritePointer(rb, n)
                        moves the write pointer of ring buffer rb n slots
                        forward

   Push(rb, c)		pushes character c into ring buffer rb; this macro
                        must be invoked only if EmptySlots(rb) is strictly
                        positive.

   Pull(rb, cp)		pulls a character from ring buffer rb and stores it
                        into *cp; this macro must be invoked only if
                        EmptySlots(rb) is strictly positive.

   InitRingBuffer(rb)	initializes ring buffer rb; it must be called
                        before using the ring buffer in any way.
*/
#define Min( a, b ) ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )
#define ReadPointer( rb ) ( ( rb ).rp )
#define FullSlots( rb ) ( ( rb ).n )
#define ContFullSlots( rb ) ( Min( FullSlots( rb ), ( rb ).ep - ReadPointer( rb ) ) )
#define WritePointer( rb ) ( ( rb ).wp )
#define EmptySlots( rb ) ( RB_SIZE - FullSlots( rb ) )
#define ContEmptySlots( rb ) ( Min( EmptySlots( rb ), ( rb ).ep - WritePointer( rb ) ) )

#define UpdateReadPointer( rb, n )                                                                                                         \
    {                                                                                                                                      \
        FullSlots( rb ) -= ( n );                                                                                                          \
        ReadPointer( rb ) += ( n );                                                                                                        \
        if ( ReadPointer( rb ) >= ( rb ).ep )                                                                                              \
            ReadPointer( rb ) -= RB_SIZE;                                                                                                  \
    }

#define UpdateWritePointer( rb, n )                                                                                                        \
    {                                                                                                                                      \
        FullSlots( rb ) += ( n );                                                                                                          \
        WritePointer( rb ) += ( n );                                                                                                       \
        if ( WritePointer( rb ) >= ( rb ).ep )                                                                                             \
            WritePointer( rb ) -= RB_SIZE;                                                                                                 \
    }

#define Push( rb, c )                                                                                                                      \
    {                                                                                                                                      \
        FullSlots( rb )++;                                                                                                                 \
        *( WritePointer( rb )++ ) = c;                                                                                                     \
        if ( WritePointer( rb ) >= ( rb ).ep )                                                                                             \
            WritePointer( rb ) -= RB_SIZE;                                                                                                 \
    }

#define Pull( rb, cp )                                                                                                                     \
    {                                                                                                                                      \
        FullSlots( rb )--;                                                                                                                 \
        *cp = *( ReadPointer( rb )++ );                                                                                                    \
        if ( ReadPointer( rb ) >= ( rb ).ep )                                                                                              \
            ReadPointer( rb ) -= RB_SIZE;                                                                                                  \
    }

#define InitRingBuffer( rb )                                                                                                               \
    {                                                                                                                                      \
        FullSlots( rb ) = 0;                                                                                                               \
        ReadPointer( rb ) = WritePointer( rb ) = ( rb ).data;                                                                              \
        ( rb ).ep = ( rb ).data + RB_SIZE;                                                                                                 \
    }

/* Push/Pull functions:

   PullAndWrite(rbp, fd)
   This function pulls as many characters as possible from the ring buffer
   pointed by rbp and write()s them into file descriptor fd.  Returns the
   number of characters actually written, or a negative integer if
   write() reported an error.

   ReadAndPush(rbp, fd)
   This function reads as many characters as possible from file descriptor fd
   and pushes them into the ring buffer pointed by rbp.  Returns the
   number of characters actually read, or a negative integer if
   read() reported an error.

*/
static int PullAndWrite( struct RingBuffer* rbp, int fd )
{
    int total = 0; /* Total # of chars written */

    while ( FullSlots( *rbp ) > 0 ) {
        int chunk = ContFullSlots( *rbp ); /* Chunk size */
        int result;                        /* # of chars written */

        /* write() takes its data from ReadPointer (full slots) */
        result = write( fd, ReadPointer( *rbp ), chunk );

        if ( result < 0 && errno != EAGAIN )
            /* write() failed; return an error indication */
            return -1;

        if ( result > 0 ) {
            /* write() wrote at least one character; update ReadPointer */
            total += result;
            UpdateReadPointer( *rbp, result );
        }

        if ( result != chunk )
            /* Partial success of write(); break loop for now */
            break;
    }

    return total;
}

static int ReadAndPush( struct RingBuffer* rbp, int fd )
{
    int total = 0; /* Total # of chars read */

    while ( EmptySlots( *rbp ) > 0 ) {
        int chunk = ContEmptySlots( *rbp ); /* Chunk size */
        int result;                         /* # of chars read */

        /* read() puts its data into WritePointer (empty slots) */
        result = read( fd, WritePointer( *rbp ), chunk );

        if ( result < 0 && errno != EAGAIN )
            /* read() failed; return an error indication */
            return -1;

        if ( result > 0 ) {
            /* read() read at least one character; update WritePointer */
            total += result;
            UpdateWritePointer( *rbp, result );
        }

        if ( result != chunk )
            /* Partial success of read(); break loop */
            break;
    }

    return total;
}

/*---------------------------------------------------------------------------
        Static variables, holding the status of the emulated port
  ---------------------------------------------------------------------------*/
typedef enum {
    IOC_SON = 0x08,  /* Serial port enable */
    IOC_ETBE = 0x04, /* Enable IRQ on TX buffer empty */
    IOC_ERBF = 0x02, /* Enable IRQ on RX buffer full */
    IOC_ERBZ = 0x01, /* Enable IRQ on RX buzy (sic) */
} ioc_t;
static ioc_t ioc = 0; /* I/O and interrupt control register */

typedef enum {
    RCS_UNUSED = 0x08, /* Unused (?) */
    RCS_RER = 0x04,    /* RX Error */
    RCS_RBZ = 0x02,    /* RX Buzy */
    RCS_RBF = 0x01,    /* RX Buffer full */
} rcs_t;
static rcs_t rcs = 0; /* RX control & status register */

typedef enum {
    TCS_BRK = 0x08, /* Unknown (?) */
    TCS_LPB = 0x04, /* Unknown (?) */
    TCS_TBZ = 0x02, /* TX Buzy */
    TCS_TBF = 0x01, /* TX Buffer full */
} tcs_t;
static tcs_t tcs = 0; /* TX control & status register */

static struct RingBuffer rrb; /* RX ring buffer */
static struct RingBuffer trb; /* TX ring buffer */

static char* pty_name; /* Name of pty's slave side */
static int master_pty; /* File descriptor of pty's master side */
static int slave_pty;  /* File descriptor of pty's slave side */

/*---------------------------------------------------------------------------
        Helper macros

  CheckIRQ	This macro checks the current status of ioc, rcs, and tcs,
                and posts an interrupt request if appropriate.  It should be
                called by all functions that modify the above-mentioned
                registers, after the modification has been made.
                Interrupt requests are posted only when IOC_SON is set.

  UpdateRCS	This macro updates the rcs register according to the
                current rrb FullSlots:
                - if the receiver ring buffer holds more than 1 character,
                  RCS_RBZ and RCS_RBF are both set
                - if the receiver ring buffer holds exactly 1 character,
                  RCS_RBZ is reset and RCS_RBF is set
                - if the receiver ring buffer is empty,
                  RCS_RBZ and RCS_RBF are both reset

                3.2: the RCS_RBZ bit is
                     always left clear and only RCS_RBF is updated;
                     this is simpler and works well on the 48, too.

  UpdateTCS	This macro updates the tcs register according to the
                current trb EmptySlots:
                - if the transmitter ring buffer has more than 1 empty slot,
                  TCS_TBZ and TCS_TBF are both reset
                - if the transmitter ring buffer has 1 empty slot,
                  TCS_TBZ is set and TCS_TBF is reset
                - if the transmitter ring buffer is full,
                  TCS_TBZ and TCS_TBF are both set.

  ---------------------------------------------------------------------------*/

#define CheckIRQ                                                                                                                           \
    if ( ( ioc & IOC_SON ) && ( ( ( ioc & IOC_ETBE ) && ( !( tcs & TCS_TBF ) ) ) || ( ( ioc & IOC_ERBF ) && ( rcs & RCS_RBF ) ) ||         \
                                ( ( ioc & IOC_ERBZ ) && ( rcs & RCS_RBZ ) ) ) )                                                            \
    CpuIntRequest( INT_REQUEST_IRQ )

#define UpdateRCS                                                                                                                          \
    if ( FullSlots( rrb ) > 0 ) {                                                                                                          \
        rcs |= RCS_RBF;                                                                                                                    \
    } else {                                                                                                                               \
        rcs &= ~( RCS_RBF );                                                                                                               \
    }

#define UpdateTCS                                                                                                                          \
    if ( EmptySlots( trb ) > 1 ) {                                                                                                         \
        tcs &= ~( TCS_TBF | TCS_TBZ );                                                                                                     \
    } else if ( EmptySlots( trb ) > 0 ) {                                                                                                  \
        tcs &= ~TCS_TBF;                                                                                                                   \
        tcs |= TCS_TBZ;                                                                                                                    \
    } else {                                                                                                                               \
        tcs |= ( TCS_TBF | TCS_TBZ );                                                                                                      \
    }

/*---------------------------------------------------------------------------
        Public functions
  ---------------------------------------------------------------------------*/

/* .+

.creation     : 13-Sep-2000
.description  :
  This function opens and initializes the pseudo-terminal that will
  be used to emulate the calculator's serial port.  When successful
  it returns to the caller the symbolic name of the slave side of
  the pseudo-terminal (that is, the side that will actually be used
  by file transfer programs such as kermit), otherwise it signals
  one or more status codes and returns NULL.

  Notice that the string returned by SerialInit() shall not be
  modified in any way.

.call         :
                slave_pty_name = SerialInit(void);
.input        :
                void
.output       :
                const char *slave_pty_name, name of slave pty or NULL
.status_codes :
                SERIAL_I_CALLED
                SERIAL_I_PTYNAME
                SERIAL_W_NOPTY
                SERIAL_F_OPENPTY
                SERIAL_F_FCNTL
                SERIAL_F_OPEN_MASTER
                SERIAL_F_GRANTPT
                SERIAL_F_UNLOCKPT
                SERIAL_F_OPEN_SLAVE
                SERIAL_F_PUSH
.notes        :
  2.5, 13-Sep-2000, creation
  2.6, 15-Sep-2000, update
    - implemented USE_STREAMSPTY
  3.2, 22-Sep-2000, update
    - disabled local ECHO on master
      pty when USE_OPENPTY is in effect; this avoid spurious rx
      when no process is connected to the slave pty yet.
  3.16, 16-Nov-2000, update
    - added dummy pty implementation
  3.17, 22-Nov-2000, bug fix
    - ensure that the pty is fully transparent by default
    - slave pty must have O_NONBLOCK set
.- */
const char* SerialInit( void )
{
    /* Initialize ring buffers */
    InitRingBuffer( rrb );
    InitRingBuffer( trb );

#ifndef USE_NOPTY
#  ifdef USE_OPENPTY
    /* Open pty master/slave pair; don't specify pty name, struct termios
       and struct winsize.
    */
    if ( openpty( &master_pty, &slave_pty, NULL, NULL, NULL ) ) {
        pty_name = ( char* )NULL;

        ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
        ChfGenerate( SERIAL_CHF_MODULE_ID, __FILE__, __LINE__, SERIAL_F_OPENPTY, CHF_FATAL );
        ChfSignal( SERIAL_CHF_MODULE_ID );
    } else {
        int cur_flags;

        pty_name = ttyname( slave_pty );

        /* Remember: close the pty before exiting */
        atexit( SerialClose );

        /* Set O_NONBLOCK on master_pty */
        if ( ( cur_flags = fcntl( master_pty, F_GETFL, 0 ) ) < 0 || fcntl( master_pty, F_SETFL, cur_flags | O_NONBLOCK ) < 0 ) {
            ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
            ChfGenerate( SERIAL_CHF_MODULE_ID, __FILE__, __LINE__, SERIAL_F_FCNTL, CHF_FATAL );
            ChfSignal( SERIAL_CHF_MODULE_ID );
        }
    }
#  endif

#  ifdef USE_STREAMSPTY
    /* Open master cloning device */
    if ( ( master_pty = open( PTY_MASTER, O_RDWR | O_NONBLOCK ) ) < 0 ) {
        pty_name = ( char* )NULL;

        ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
        ChfGenerate( SERIAL_CHF_MODULE_ID, __FILE__, __LINE__, SERIAL_F_OPEN_MASTER, CHF_FATAL, PTY_MASTER );
        ChfSignal( SERIAL_CHF_MODULE_ID );
    } else {
        /* Master side opened ok; change permissions and unlock slave side */

        if ( grantpt( master_pty ) < 0 ) {
            /* close() may modify errno; save it first */
            ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );

            ( void )close( master_pty );

            ChfGenerate( SERIAL_CHF_MODULE_ID, __FILE__, __LINE__, SERIAL_F_GRANTPT, CHF_FATAL );
            ChfSignal( SERIAL_CHF_MODULE_ID );
        }

        if ( unlockpt( master_pty ) < 0 ) {
            /* close() may modify errno; save it first */
            ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );

            ( void )close( master_pty );

            ChfGenerate( SERIAL_CHF_MODULE_ID, __FILE__, __LINE__, SERIAL_F_UNLOCKPT, CHF_FATAL );
            ChfSignal( SERIAL_CHF_MODULE_ID );
        }

        /* Get name of slave side; this must be done on the *master* side */
        pty_name = ptsname( master_pty );

        /* Open slave in nonblocking mode */
        if ( ( slave_pty = open( pty_name, O_RDWR | O_NONBLOCK ) ) < 0 ) {
            /* close() may modify errno; save it first */
            ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );

            ( void )close( master_pty );

            ChfGenerate( SERIAL_CHF_MODULE_ID, __FILE__, __LINE__, SERIAL_F_OPEN_SLAVE, CHF_FATAL, pty_name );
            ChfSignal( SERIAL_CHF_MODULE_ID );
        }

        /* Remember: close the pty before exiting */
        atexit( SerialClose );

        /* Push appropriate STREAMS modules on the slave side to support
           terminal emulation.  This way, the slave side should be
           indistinguishable from a real terminal.
        */
        if ( ioctl( slave_pty, I_PUSH, "ptem" ) == -1 ) {
            ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
            ChfGenerate( SERIAL_CHF_MODULE_ID, __FILE__, __LINE__, SERIAL_F_PUSH, CHF_FATAL, "ptem" );
            ChfSignal( SERIAL_CHF_MODULE_ID );
        }

        if ( ioctl( slave_pty, I_PUSH, "ldterm" ) == -1 ) {
            ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
            ChfGenerate( SERIAL_CHF_MODULE_ID, __FILE__, __LINE__, SERIAL_F_PUSH, CHF_FATAL, "ldterm" );
            ChfSignal( SERIAL_CHF_MODULE_ID );
        }
    }
#  endif

    /* 3.17: Ensure that the pty is fully trasparent by default.
       This allows to use most non-terminal-aware applications (such as od)
       on the pty directly.
    */
    {
        struct termios tios;

        if ( tcgetattr( slave_pty, &tios ) ) {
            ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
            ChfGenerate( SERIAL_CHF_MODULE_ID, __FILE__, __LINE__, SERIAL_F_TCGETATTR, CHF_FATAL );
            ChfSignal( SERIAL_CHF_MODULE_ID );
        }

        tios.c_iflag &= ~( BRKINT | IGNPAR | PARMRK | INPCK | ISTRIP | INLCR | IGNCR | ICRNL | IUCLC | IXON | IXANY | IXOFF | IMAXBEL );

        tios.c_iflag |= IGNBRK;

        tios.c_oflag &=
            ~( OPOST | OLCUC | ONLCR | OCRNL | ONOCR | ONLRET | OFILL | OFDEL | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY );

        tios.c_cflag &= ~( CSIZE | CSTOPB | PARENB | PARODD );

        tios.c_cflag |= CS8 | CREAD | HUPCL | CLOCAL;

        tios.c_lflag &= ~( ISIG | ICANON | ECHO | ECHONL | IEXTEN );

        /* read()s are satisfed when at least 1 character is available;
           intercharacter/read timer disabled.
        */
        tios.c_cc[ VMIN ] = 1;
        tios.c_cc[ VTIME ] = 0;

        if ( tcsetattr( slave_pty, TCSANOW, &tios ) ) {
            ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
            ChfGenerate( SERIAL_CHF_MODULE_ID, __FILE__, __LINE__, SERIAL_F_TCSETATTR, CHF_FATAL );
            ChfSignal( SERIAL_CHF_MODULE_ID );
        }
    }

    /* Publish pty name */
    if ( config.verbose ) {
        ChfGenerate( SERIAL_CHF_MODULE_ID, __FILE__, __LINE__, SERIAL_I_PTY_NAME, CHF_INFO, pty_name );
        ChfSignal( SERIAL_CHF_MODULE_ID );
    }

#else
    /* Dummy implementation; do nothing */
    pty_name = "";

    ChfGenerate( SERIAL_CHF_MODULE_ID, __FILE__, __LINE__, SERIAL_W_NOPTY, CHF_WARNING );
    ChfSignal( SERIAL_CHF_MODULE_ID );
#endif

    return pty_name;
}

/* .+

.creation     : 13-Sep-2000
.description  :
  This function closes the pseudo-terminal opened by SerialInit(), if
  SerialInit() succeeded, it does nothing otherwise.

  Notice that is is normally unnecessary to call this function directly,
  because SerialInit() automatically registers it with atexit() upon
  successful completion.

.call         :
                SerialClose();
.input        :
                void
.output       :
                void
.status_codes :
                SERIAL_I_CALLED
                SERIAL_E_PTY_CLOSE
.notes        :
  2.5, 13-Sep-2000, creation
  2.6, 15-Sep-2000, update
    - updated documentation

.- */
void SerialClose( void )
{
    bool err = close( slave_pty ) || close( master_pty );
    if ( err ) {
        ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
        ChfGenerate( SERIAL_CHF_MODULE_ID, __FILE__, __LINE__, SERIAL_E_PTY_CLOSE, CHF_ERROR );
        ChfSignal( SERIAL_CHF_MODULE_ID );
    }
}

/* .+

.creation     : 13-Sep-2000
.description  :
  This function returns to the caller the name of the slave size of
  the pseudo-terminal previously opened by SerialInit() if the
  latter function succeeded, otherwise NULL.

  Notice that the string returned by SerialPtyName() shall not be
  modified in any way.

.call         :
                slave_pty_name = SerialPtyName();
.input        :
                void
.output       :
                const char *slave_pty_name, name of slave pty opened by
                  SerialInit()
.status_codes :
                SERIAL_I_CALLED
.notes        :
  2.5, 13-Sep-2000, creation

.- */
const char* SerialPtyName( void ) { return pty_name; }

/* .+

.creation     : 13-Sep-2000
.description  :
  This function emulates a read of the serial I/O and interrupt control
  register and returns its current value to the caller.

  Reading IOC returns the current value of the ioc emulation register
  and does not trigger any ancillary action.

.call         :
                n = Serial_IOC_Read();
.input        :
                void
.output       :
                Nibble n, current value of emulated register
.status_codes :
                SERIAL_I_CALLED
                SERIAL_I_READ
.notes        :
  2.5, 13-Sep-2000, creation

.- */
Nibble Serial_IOC_Read( void )
{

    debug2( SERIAL_CHF_MODULE_ID, DEBUG_C_SERIAL, SERIAL_I_READ, "IOC", ioc );

    return ioc;
}

/* .+

.creation     : 13-Sep-2000
.description  :
  This function emulates a read of the serial receiver control & status
  register and returns its current value to the caller.

  Reading RCS returns the current value of the rcs emulation register
  and does not trigger any ancillary action.

.call         :
                n = Serial_RCS_Read();
.input        :
                void
.output       :
                Nibble n, current value of emulated register
.status_codes :
                SERIAL_I_CALLED
                SERIAL_I_READ
.notes        :
  2.5, 13-Sep-2000, creation

.- */
Nibble Serial_RCS_Read( void )
{

    debug2( SERIAL_CHF_MODULE_ID, DEBUG_C_SERIAL, SERIAL_I_READ, "RCS", rcs );

    return rcs;
}

/* .+

.creation     : 13-Sep-2000
.description  :
  This function emulates a read of the serial transmitter control & status
  register and returns its current value to the caller.

  Reading TCS returns the current value of the tcs emulation register
  and does not trigger any ancillary action.

.call         :
                n = Serial_TCS_Read();
.input        :
                void
.output       :
                Nibble n, current value of emulated register
.status_codes :
                SERIAL_I_CALLED
                SERIAL_I_READ
.notes        :
  2.5, 13-Sep-2000, creation

.- */
Nibble Serial_TCS_Read( void )
{

    debug2( SERIAL_CHF_MODULE_ID, DEBUG_C_SERIAL, SERIAL_I_READ, "TCS", tcs );

    return tcs;
}

/* .+

.creation     : 13-Sep-2000
.description  :
  This function emulates a read of the serial receiver buffer
  register and returns its current value to the caller.

  Reading RBR triggers the following additional actions:
  - pulls one character from receiver ring buffer
  - UpdateRCS
  - CheckIRQ

.call         :
                d = Serial_RBR_Read();
.input        :
                void
.output       :
                int8 d, current value of emulated register
.status_codes :
                SERIAL_I_CALLED
                SERIAL_I_RBR
                SERIAL_W_EMPTY_RRB
.notes        :
  2.5, 13-Sep-2000, creation
  3.2, 22-Sep-2000, update
    - removed warning message
      when reading from an empty RRB.
.- */
int8 Serial_RBR_Read( void )
{
    int8 rx;

    /* Pull one character from rbr, if not empty */
    if ( FullSlots( rrb ) > 0 ) {
        Pull( rrb, &rx );
    } else {
        /* rrb is empty */

        rx = ( int8 )0xFF;
    }

    /* Update receiver status */
    UpdateRCS;

    /* Post a new IRQ if necessary */
    CheckIRQ;

    debug1( SERIAL_CHF_MODULE_ID, DEBUG_C_SERIAL, SERIAL_I_RBR, rx );

    return rx;
}

/* .+

.creation     : 13-Sep-2000
.description  :
  This function emulates a write into the serial I/O and interrupt control
  register.

  Writing IOC triggers the following additional actions:

  - CheckIRQ is executed

.call         :
                Serial_IOC_Write(n);
.input        :
                Nibble n, value to be written into the emulated register
.output       :
                void
.status_codes :
                SERIAL_I_CALLED
                SERIAL_I_WRITE
.notes        :
  2.5, 13-Sep-2000, creation

.- */
void Serial_IOC_Write( Nibble n )
{

    debug3( SERIAL_CHF_MODULE_ID, DEBUG_C_SERIAL, SERIAL_I_WRITE, "IOC", ioc, n );

    ioc = n;

    CheckIRQ;
}

/* .+

.creation     : 13-Sep-2000
.description  :
  This function emulates a write into the serial receiver control & status
  register.

  The status is updated and no additional actions are taken; it is not
  so clear when a direct write into RCS could be useful.

.call         :
                Serial_RCS_Write(n);
.input        :
                Nibble n, value to be written into the emulated register
.output       :
                void
.status_codes :
                SERIAL_I_CALLED
                SERIAL_I_WRITE
.notes        :
  2.5, 13-Sep-2000, creation

.- */
void Serial_RCS_Write( Nibble n )
{

    debug3( SERIAL_CHF_MODULE_ID, DEBUG_C_SERIAL, SERIAL_I_WRITE, "RCS", rcs, n );

    rcs = n;
}

/* .+

.creation     : 13-Sep-2000
.description  :
  This function emulates a write into the serial transmitter control & status
  register.

  The status is updated and no additional actions are taken; it is not
  so clear when a direct write into TCS could be useful.

.call         :
                Serial_TCS_Write(n);
.input        :
                Nibble n, value to be written into the emulated register
.output       :
                void
.status_codes :
                SERIAL_I_CALLED
                SERIAL_I_WRITE
.notes        :
  2.5, 13-Sep-2000, creation

.- */
void Serial_TCS_Write( Nibble n )
{

    debug3( SERIAL_CHF_MODULE_ID, DEBUG_C_SERIAL, SERIAL_I_WRITE, "TCS", tcs, n );

    tcs = n;
}

/* .+

.creation     : 13-Sep-2000
.description  :
  This function emulates a write into the serial 'clear receiver error'
  register.

  The value written is ignored, and the RCS_RER bit is cleared.

.call         :
                Serial_CRER_Write(n);
.input        :
                Nibble n, value to be written into the emulated register
.output       :
                void
.status_codes :
                SERIAL_I_CALLED
                SERIAL_I_WRITE
.notes        :
  2.5, 13-Sep-2000, creation

.- */
void Serial_CRER_Write( Nibble n )
{

    debug3( SERIAL_CHF_MODULE_ID, DEBUG_C_SERIAL, SERIAL_I_WRITE, "CRER", 0, n );

    rcs &= ~RCS_RER;
}

/* .+

.creation     : 13-Sep-2000
.description  :
  This function emulates a write into the serial transmitter buffer
  register.

  Writing RBR triggers the following additional actions:
  - pushes one character into transmitter ring buffer
  - UpdateTCS
  - CheckIRQ

.call         :
                Serial_TBR_Write(d);
.input        :
                int8 d, value to be written into the emulated register
.output       :
                void
.status_codes :
                SERIAL_I_CALLED
                SERIAL_I_TBR
                SERIAL_W_FULL_TRB
.notes        :
  2.5, 13-Sep-2000, creation

.- */
void Serial_TBR_Write( int8 d )
{

    debug1( SERIAL_CHF_MODULE_ID, DEBUG_C_SERIAL, SERIAL_I_TBR, d );

    /* Pull one character from rbr, if not empty */
    if ( EmptySlots( trb ) > 0 ) {
        Push( trb, d );
    } else {
        /* trb is full; discard character */
        ChfGenerate( SERIAL_CHF_MODULE_ID, __FILE__, __LINE__, SERIAL_W_FULL_TRB, CHF_WARNING, tcs );
        ChfSignal( SERIAL_CHF_MODULE_ID );
    }

    /* Update transmitter status */
    UpdateTCS;

    /* Post a new IRQ if necessary */
    CheckIRQ;
}

/* .+

.creation     : 14-Sep-2000
.description  :
  This function is called by the emulator loop every 1/16s, and performs
  the following actions:

  - attempt to pull as many characters as possible from the transmitter
    ring buffer and to write them on master_pty (PullAndWrite)
  - UpdateTCS

  - attempt to read as many characters are possible from master_pty and
    to push them into the receiver ring buffer (ReadAndPush)
  - UpdateRCS

  - CheckIRQ

.call         :
                HandleSerial();
.input        :
                void
.output       :
                void
.status_codes :
                SERIAL_I_CALLED
                SERIAL_E_TRB_DRAIN
                SERIAL_E_RRB_CHARGE
.notes        :
  2.5, 14-Sep-2000, creation
  2.6, 15-Sep-2000, update
    - enhanced documentation of status_codes
  3.16, 16-Nov-2000, update
    - added dummy pty implementation
  3.17, 22-Nov-2000, bug fix
    - transmit ring buffer must be emptied even when !IOC_SON
.- */
void HandleSerial( void )
{
#ifndef USE_NOPTY
    /* Attempt to drain transmitter buffer even if serial port is closed */
    int result = PullAndWrite( &trb, master_pty );

    /* Signal a condition upon failure */
    if ( result < 0 ) {
        ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
        ChfGenerate( SERIAL_CHF_MODULE_ID, __FILE__, __LINE__, SERIAL_E_TRB_DRAIN, CHF_ERROR );
        ChfSignal( SERIAL_CHF_MODULE_ID );
    }

    /* Update tcs */
    UpdateTCS;

    if ( ioc & IOC_SON ) {
        /* Attempt to charge receiver buffer */
        result = ReadAndPush( &rrb, master_pty );

        /* Signal a condition upon failure */
        if ( result < 0 ) {
            ChfGenerate( CHF_ERRNO_SET, __FILE__, __LINE__, errno, CHF_ERROR );
            ChfGenerate( SERIAL_CHF_MODULE_ID, __FILE__, __LINE__, SERIAL_E_RRB_CHARGE, CHF_ERROR );
            ChfSignal( SERIAL_CHF_MODULE_ID );
        }

        /* Update receiver status */
        UpdateRCS;

        /* Post an IRQ if necessary */
        CheckIRQ;
    }
#endif
}
