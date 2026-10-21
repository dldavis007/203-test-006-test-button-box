#ifndef APPLICATION_TYPES_H
#define APPLICATION_TYPES_H

/* Shared application types. Member order and widths are part of the legacy ABI. */
typedef enum { false = 0, true = !false } bool;

struct MatrixTbl
{
    int BB_Data;
    char SW_Port_Data[2];
    char LED_Port_Data[3];
};

struct MenuStruct
{
    unsigned char Index[4];
    char Entry[12][21];        /* Menu text. */
    unsigned char Pos[11];    /* Variable display columns. */
    char *VarPntr[11];
    int (*FunctPtr[11])(void);
};

struct MenuStack
{
    unsigned char Index[4];
    unsigned char CursorPos;
    unsigned char FirstLine;
};

#endif
