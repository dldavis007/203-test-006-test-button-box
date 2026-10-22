#ifndef Flash_H
#define Flash_H


#include "FixedTypes.h"
void FlashInit ( void );
void FlashWrite ( int16_t ArraySize, char WriteData[], int16_t *WriteAddr );

#define FSTAT_CBEIF 0x80
#define FSTAT_CCIF 0x40

#endif
