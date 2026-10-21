#ifndef SETTINGS_H
#define SETTINGS_H

/* Storage remains defined in Subroutines.c, in the legacy EEPROM sequence. */
extern char FirstVar[4];
extern char MessageRow[4], MessageCol[4];
extern char SerialNum[6];
extern char F1Enable[4], F2Enable[4];
extern char NullVar[];

void NormalizeFunctionSettings(void);
void Load_Variables(void);
void Save_Variables(void);
void Load_Serial_Num(void);
void Save_Serial_Num(void);

#ifdef BUTTONBOX_GCC
/* Explicit codec for the existing 28-byte ImageCraft-compatible EEPROM record. */
void EncodeLegacySettings(unsigned char legacy[28]);
void DecodeLegacySettings(const unsigned char legacy[28]);
#endif

#endif
