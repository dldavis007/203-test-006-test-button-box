#ifndef INTERNAL_MENU_H
#define INTERNAL_MENU_H


#include "FixedTypes.h"
#include "ApplicationState.h"

#define MenuSize 11
#define MenuStackSize 5
#define SER_NUM 8
/* Numeric editor format: zero for OFF and one for ON. */
#define OFF_ON 9

/* Definitions remain in Subroutines.c and its menu data include, in old order. */
extern const struct MenuStruct defaultMenuc[MenuSize];
extern const struct MenuStack defaultMenuStackc[MenuStackSize];
extern struct MenuStack MenuStackc[MenuStackSize];
extern uint8_t StackPointer;
extern char Menu[12][21];
extern uint8_t VariableFlag;
extern double increment, uLimit, lLimit;
extern int16_t nDigits;

/* Initialization, navigation, rendering and variable editing. */
void setDefaultMenu(void);
void LoadMenu(uint8_t Index[]);
int16_t ExitMenu(void);
void CursorUp(void);
void CursorDown(void);
void Select(void);
void DeSelect(void);
void InsertCursor(void);
void IncVariable(void);
void DecVariable(void);
void dispVariable(char* vPtr, double *value, uint8_t format);
void btnDisplayTitler(void);

/* Internal menu callbacks. */
int16_t NullFunction(void);
int16_t setSerial(void);
int16_t RestoreDefaults(void);
int16_t VarOffOnFunct(void);
int16_t Var1to13Funct(void);
int16_t Var1to30Funct(void);

#endif
