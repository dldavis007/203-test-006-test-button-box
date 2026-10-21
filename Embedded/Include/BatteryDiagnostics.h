#ifndef BATTERY_DIAGNOSTICS_H
#define BATTERY_DIAGNOSTICS_H

/* Definitions remain in Subroutines.c in their existing order. */
extern char BattNum[20][4];
extern unsigned char ptrDispBatt;
extern char BattSerNum[], BattSOC[], BattSOH[], BattErr[], BatteryTemps[];

void Scan_Batteries(void);
int ClrBattErr(void);
int NextBatt(void);

#endif
