#include "Subroutines.h"

#ifndef Controller_H
#define Controller_H

#ifndef INTR_ON
#define INTR_ON()	asm("cli")
#define INTR_OFF()	asm("sei")
#endif

extern unsigned int Timer1;
extern void WriteCharMem(void);
extern void ReadCharMem(void);
extern bool CharsInMem;

int ResetProc (void);
void InitPorts ( void );
void InitInterrupts ( void );
void InitPLL ( void );
void InitSCI ( void );
void InitSPI ( void );
void PWMInit ( void );
void AtoDInit ( void );
int getchar(void);
int putchar(char c);
void InitCANopen ( void );
void InitWatchdog(void);
void COP_Trig(void); 


#endif