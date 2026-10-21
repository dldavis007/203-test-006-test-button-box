#ifndef INTERNAL_MENU_H
#define INTERNAL_MENU_H

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
extern unsigned char StackPointer;
extern char Menu[12][21];
extern char VariableFlag;
extern double increment, uLimit, lLimit;
extern int nDigits;

/* Initialization, navigation, rendering and variable editing. */
void setDefaultMenu(void);
void LoadMenu(unsigned char Index[]);
int ExitMenu(void);
void CursorUp(void);
void CursorDown(void);
void Select(void);
void DeSelect(void);
void InsertCursor(void);
void IncVariable(void);
void DecVariable(void);
void dispVariable(char* vPtr, double *value, char format);
void btnDisplayTitler(void);

/* Internal menu callbacks. */
int NullFunction(void);
int setSerial(void);
int RestoreDefaults(void);
int ToggleF1Enable(void);
int ToggleF2Enable(void);
int Var1to13Funct(void);
int Var1to30Funct(void);

#endif
