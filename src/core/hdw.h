#ifndef _HDW_H
#  define _HDW_H 1

#  include "cpu.h"

extern void HdwInit( void );
extern void HdwSave( void );
extern Nibble HdwRead( Address );
extern void HdwWrite( Address, Nibble );

#endif /*!_HDW_H*/
