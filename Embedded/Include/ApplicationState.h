#ifndef APPLICATION_STATE_H
#define APPLICATION_STATE_H

#include "ApplicationTypes.h"

/* Defined in Subroutines.c; declarations here do not allocate storage. */

/* Button data and alternate-function LED selection. */
extern volatile unsigned int ButtonBoxData;
extern volatile unsigned char AlternateFunctionMask;
extern const struct MatrixTbl Keypad_Matrix[18];

/* Video-display SPI exchange, shared with its interrupt handler. */
extern volatile char SPI0InBuf[2], SPI0OutBuf[2];
extern volatile int SPI0InBufptr, SPI0OutBufptr;
extern bool CharsInMem;
extern bool firstWriteSPI;
extern char plusSymbol[54];
extern const char CMDI[];
extern char DispBuf[];
extern int DispBufptr;
extern char VCI_Flag2;

/* Menu state and operating mode. Preserve the legacy MCO qualifiers. */
extern struct MenuStruct Menuc[];
extern volatile int ReActivate_Timer;
extern volatile char UpdateMenu;
extern char UpdateDisp;
extern char MenuCheck;
extern bool controller_mode;
extern volatile bool legacy_mode;
extern bool resequenceNodes;
extern char deviceSubMenu[20][17];
extern int sub_Device_Type_Response_ID[20];

#endif
