#include "Subroutines.h"

#ifndef Controller_H
#define Controller_H

#ifdef BUTTONBOX_GCC
/* GNU68HC12 stdio implements these as macros. Define real device hooks. */
#include <stdio.h>
#undef getchar
#undef putchar
#include <stdlib.h>
#include "../embedded/gcc/compact_io.h"
#endif

#ifndef INTR_ON
#define INTR_ON()	asm("cli")
#define INTR_OFF()	asm("sei")
#endif

extern volatile unsigned int Timer1;
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
#ifdef BUTTONBOX_GCC
int putchar(int c);
#else
int putchar(char c);
#endif
void InitCANopen ( void );
void InitWatchdog(void);
void COP_Trig(void); 


#endif
