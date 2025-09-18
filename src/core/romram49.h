#ifndef _ROMRAM49_H
#  define _ROMRAM49_H 1

#  include "cpu.h"
#  include "hdw.h"
#  include "modules.h"

/* External storage */
extern struct ModStatus_49* mod_status_49;

extern void RomInit49( void );
extern void RamInit49( void );
extern void Ce1Init49( void );
extern void Ce2Init49( void );
extern void NCe3Init49( void );

extern void RomSave49( void );
extern void RamSave49( void );
extern void Ce1Save49( void );
extern void Ce2Save49( void );
extern void NCe3Save49( void );

extern Nibble RomRead49( Address );
extern Nibble RamRead49( Address );
extern Nibble Ce1Read49( Address );
extern Nibble Ce2Read49( Address );
extern Nibble NCe3Read49( Address );

extern void RomWrite49( Address, Nibble );
extern void RamWrite49( Address, Nibble );
extern void Ce1Write49( Address, Nibble );
extern void Ce2Write49( Address, Nibble );
extern void NCe3Write49( Address, Nibble );

#endif /*!_ROMRAM49_H*/
