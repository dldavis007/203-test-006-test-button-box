#ifndef Comand_H
#define Comand_H


#include "FixedTypes.h"
void command ( char CmdBuf[] );
uint8_t Checksum ( char *str );
void parse(char *pbuf, char *str);
void ucase (char *str);
void ltrim (char *c_ptr);
int16_t hextoi(char *string);

//#define CmdBufLen 64

//#define CommXmitngF  0b00000001
//#define CommRcvdF    0b00000010
//#define CommIntEnabF 0b00000100

//#define SINRcvd 0x02

#endif
