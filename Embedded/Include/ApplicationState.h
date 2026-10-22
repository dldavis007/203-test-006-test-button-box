#ifndef APPLICATION_STATE_H
#define APPLICATION_STATE_H

#include "FixedTypes.h"
#include "ApplicationTypes.h"

/* Defined in Subroutines.c; declarations here do not allocate storage. */

/* Button data and alternate-function LED selection. */
extern volatile uint16_t ButtonBoxData;
extern volatile uint8_t AlternateFunctionMask;
extern const struct MatrixTbl Keypad_Matrix[18];

/* Video-display SPI exchange, shared with its interrupt handler. */
extern volatile uint8_t SPI0InBuf[2], SPI0OutBuf[2];
extern volatile int16_t SPI0InBufptr, SPI0OutBufptr;
extern bool CharsInMem;
extern bool firstWriteSPI;
extern char plusSymbol[54];
extern const char CMDI[];
extern char DispBuf[];
extern int16_t DispBufptr;
extern uint8_t VCI_Flag2;

/* Menu state and operating mode. Preserve the legacy MCO qualifiers. */
extern struct MenuStruct Menuc[];
extern volatile int16_t ReActivate_Timer;
extern volatile uint8_t UpdateMenu;
extern char UpdateDisp; /* Native char declaration is also present in MCO. */
extern uint8_t MenuCheck;
extern bool controller_mode;
extern volatile bool legacy_mode;
extern bool resequenceNodes;
extern char deviceSubMenu[20][17];
extern int sub_Device_Type_Response_ID[20]; /* Native MCO ABI. */

#endif
