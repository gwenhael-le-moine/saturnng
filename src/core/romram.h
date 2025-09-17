#ifndef _ROMRAM_H
#  define _ROMRAM_H 1

#  include "cpu.h"
#  include "hdw.h"
#  include "modules.h"

extern void RomInit( void );
extern void RamInit( void );
extern void Ce1Init( void );
extern void Ce2Init( void );
extern void NCe3Init( void );

extern void RomSave( void );
extern void RamSave( void );
extern void Ce1Save( void );
extern void Ce2Save( void );
extern void NCe3Save( void );

extern Nibble RomRead( Address );
extern Nibble RamRead( Address );
extern Nibble Ce1Read( Address );
extern Nibble Ce2Read( Address );
extern Nibble NCe3Read( Address );

extern void RomWrite( Address, Nibble );
extern void RamWrite( Address, Nibble );
extern void Ce1Write( Address, Nibble );
extern void Ce2Write( Address, Nibble );
extern void NCe3Write( Address, Nibble );

#endif /*!_ROMRAM_H*/
