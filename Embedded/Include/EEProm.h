#ifndef EEProm_H
#define EEProm_H



#include "FixedTypes.h"
extern volatile uint8_t EEWriteStatus; /* 0=verified; 0x30=hardware errors; 1=verify failed */
void EEInit ( void );
void EEWrite ( int16_t ArraySize, char WriteData[], int16_t *WriteAddr );

#define ESTAT_CBEIF 0x80
#define ESTAT_CCIF 0x40
#define ESTAT_ERRORS 0x30

#define INITEE_Init 0x09


#define WordPrg 0x20
#define SecErase 0x40
#define EE_Begin 0x800

#endif
