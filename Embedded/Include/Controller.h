#ifndef Controller_H
#define Controller_H


#include "FixedTypes.h"
#include "Subroutines.h"
#include "Interrupts.h"

#ifdef BUTTONBOX_GCC
/* GNU68HC12 stdio implements these as macros. Define real device hooks. */
#include <stdio.h>
#undef getchar
#undef putchar
#include <stdlib.h>
#include "compact_io.h"
#endif

#ifndef INTR_ON
#define INTR_ON()	asm("cli")
#define INTR_OFF()	asm("sei")
#endif

int16_t ResetProc (void);
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


/* Watchdog handshake shared with the timer interrupt. */
extern volatile uint8_t wd_flag, wdInit, wdStarted;
/* Startup-owned persistent marker (ImageCraft startup or GCC runtime). */
extern char _bss_end[20];

#endif
