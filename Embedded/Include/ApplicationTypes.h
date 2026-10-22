#ifndef APPLICATION_TYPES_H
#define APPLICATION_TYPES_H


#include "FixedTypes.h"
/* Shared application types. Member order and widths are part of the legacy ABI. */
typedef enum { false = 0, true = !false } bool;

struct MatrixTbl
{
    uint16_t BB_Data;
    uint8_t SW_Port_Data[2];
    uint8_t LED_Port_Data[3];
};

struct MenuStruct
{
    uint8_t Index[4];
    char Entry[12][21];        /* Menu text. */
    uint8_t Pos[11];    /* Variable display columns. */
    char *VarPntr[11];
    int16_t (*FunctPtr[11])(void);
};

struct MenuStack
{
    uint8_t Index[4];
    uint8_t CursorPos;
    uint8_t FirstLine;
};

#endif
