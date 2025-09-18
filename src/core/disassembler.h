#ifndef _DIS_H
#  define _DIS_H 1

#  include "types.h"

#  define DISASSEMBLE_OB_SIZE 128

Address Disassemble( Address pc, char ob[ DISASSEMBLE_OB_SIZE ] );

#endif /*!_DIS_H*/
