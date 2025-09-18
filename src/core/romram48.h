#ifndef _ROMRAM_H
#  define _ROMRAM_H 1

#  include "types.h"

extern void RomInit48( void );
extern void RamInit48( void );
extern void Ce1Init48( void );
extern void Ce2Init48( void );
extern void NCe3Init48( void );

extern void RomSave48( void );
extern void RamSave48( void );
extern void Ce1Save48( void );
extern void Ce2Save48( void );
extern void NCe3Save48( void );

extern Nibble RomRead48( Address );
extern Nibble RamRead48( Address );
extern Nibble Ce1Read48( Address );
extern Nibble Ce2Read48( Address );
extern Nibble NCe3Read48( Address );

extern void RomWrite48( Address, Nibble );
extern void RamWrite48( Address, Nibble );
extern void Ce1Write48( Address, Nibble );
extern void Ce2Write48( Address, Nibble );
extern void NCe3Write48( Address, Nibble );

#endif /*!_ROMRAM_H*/
