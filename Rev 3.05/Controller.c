#include <stdio.h>

#include "Controller.h"
#include "Interrupts.h"
#include "mc9s12a128.h"
#include "Flash.h"
#include "EEProm.h"
#include "Subroutines.h"
#include "mco.h"



char wd_flag = 0, wdInit = 0, wdStarted = 0;

extern int SIN0Bufptr;
extern int SOUT0Bufptr;
extern char SOUT0Buf[SOUT0BufLen];
extern unsigned int WD_Timer;


void main (void)
{
	InitPorts ();
	InitInterrupts ();
	InitWatchdog(); 
	InitPLL ();
	InitSCI ();
	InitSPI ();
  	PWMInit ();
	AtoDInit ();
	
    EEInit ();
	FlashInit ();

  	// end of initialization, enable all interrupts
  	INTR_ON();
	
	InitCANopen ();

	ReadCharMem();
	
	if (!CharsInMem)
	    WriteCharMem();
	
    Load_Serial_Num();      //get serial number from EEPROM, it cannot be modified
    //important to Load_Serial_Num() before Load_Variables(), EEProm is not available for
    //brief period after save_variables at end of load_variables()
	Load_Variables ();
	
	setDefaultMenu();

    resetTitler();

	COPCTL = 0x47;				//enable COP
	
	
	while (1)
	{
	    COP_Trig();
	    doevents ();
	}
	 
}


//Hardware Watchdog 
void InitWatchdog(void)   
{
    PORTE |= HW_COP;
	PORTE &= ~HW_COP;
	wdInit = 1;
	wd_flag = 1;
	WD_Timer = HW_Watchdog_Time;
	wdStarted = 1;
}

void COP_Trig(void)  
{
   ARMCOP = 0x55;   //software wd   
   ARMCOP = 0xAA;
   
   if(!WD_Timer)    //hardware wd  
   {
       PORTE |= HW_COP;
	   PORTE &= ~HW_COP;
	   WD_Timer = HW_Watchdog_Time;
   }
   else
       wd_flag = 1;
   
  
}

void InitPorts ( void )
{


	DDRA = DDRA_Init;
    DDRB = DDRB_Init;
	DDRE = DDRE_Init;
	DDRJ = DDRJ_Init;
    DDRM = DDRM_Init;
    DDRP = DDRP_Init;
    DDRS = DDRS_Init;
    DDRT = DDRT_Init;
	
    ATD0DIEN = ATD0DIEN_Init;
    ATD0CTL2 = ATD0CTL2_Init;
    ATD0CTL3 = ATD0CTL3_Init;
    ATD0CTL5 = ATD0CTL5_Init;
    
    
    PUCR = PUCR_Init;
	PERM = PERM_Init;
    PPSM = PPSM_Init;
    
    PORTA = PORTA_Init;
    PORTB = PORTB_Init;
	PORTE = PORTE_Init;	
	PTJ = PTJ_Init;
    PTM = PTM_Init;
	PTP = PTP_Init;
	PTS = PTS_Init;
    PTT = PTT_Init;
	
	PPSJ = PPSJ_Init;  		 		//Port J pulldowns
	
	PERS = PERS_Init;				//Port S pulldowns
	WOMS = WOMS_Init;				//Port S bit 3 (TXD1) Wired-OR
}

void InitInterrupts ( void )
{	
	CRGINT = CRGINT_Init;	 		//enable RTI
	RTICTL = RTICTL_Init;
    
    PPSP = PPSP_Init;		  		//rising edge
    PIEP = PIEP_Init;		  		//enable KW interrupts 
    
	TSCR1 = TSCR1_Init;				//enable input capture
    TIOS = TIOS_Init;				//enable output compares
	TIE = TIE_Init;
	TSCR2 = TSCR2_Init;
	TCTL3 = TCTL3_Init;
	TCTL4 = TCTL4_Init;
	
}

void InitPLL ( void )
{	
	REFDV = REFDV_Init;
	SYNR = SYNR_Init;
	while ( !(CRGFLG & 0x08) );
	CLKSEL |= 0x80;
}



void InitSCI ( void )
{
	SCI0BD = (unsigned int)(BusClk / 16 / .0012);
			   
	SCI0CR2 = SCI0CR2_TE | SCI0CR2_RE;
	
	SIN0Bufptr = 0;
	
	SCI0CR2 |=  SCI0CR2_RIE;
	
	SOUT0Bufptr = 0;
	SOUT0Buf[0] = '\r';

}

void InitSPI ( void )
{
	MODRR = MODRR_Init;
	SPI0CR1 = SPI0CR1_Init;
	SPI0CR2 = SPI0CR2_Init;
	SPI0BR = SPI0BR_Init;
}

void PWMInit ( void )
{
	PWMPOL = PWMPOL_Init;
    PWMCLK = PWMCLK_Init;
    PWMPRCLK = PWMPRCLK_Init;
    
    PWMPER0 = PWMPER0_Init;
    PWMPER1 = PWMPER1_Init;
    PWMPER2 = PWMPER2_Init;
    PWMPER3 = PWMPER3_Init;
    PWMPER4 = PWMPER4_Init;
    PWMPER5 = PWMPER5_Init;
    PWMPER6 = PWMPER6_Init;
    PWMPER7 = PWMPER7_Init;
    
    PWMDTY0 = PWMDTY0_Init;
    PWMDTY1 = PWMDTY1_Init;
    PWMDTY2 = PWMDTY2_Init;
    PWMDTY3 = PWMDTY3_Init;
    PWMDTY4 = PWMDTY4_Init;
    PWMDTY5 = PWMDTY5_Init;
    PWMDTY6 = PWMDTY6_Init;
    PWMDTY7 = PWMDTY7_Init;

    
    PWMSCLA = PWMSCLA_Init;
    PWMSCLB = PWMSCLB_Init;
    PWME = PWME_Init;
    
}

void AtoDInit ( void )
{
    ATD0DIEN = ATD0DIEN_Init;	  	 //Analog to Digital Registers
    ATD0CTL2 = ATD0CTL2_Init;
    ATD0CTL3 = ATD0CTL3_Init;
    ATD0CTL4 = ATD0CTL4_Init;
    ATD0CTL5 = ATD0CTL5_Init;
}    

int getchar(void)
	{
	while ((SCI0SR1 & SCI0SR1_RDRF) == 0)
		;
	return SCI0DRL;
	}

extern int _textmode;

int putchar(char c)
	{
	if (_textmode && c == '\n')
		putchar('\r');
	while ((SCI0SR1 & SCI0SR1_TDRE) == 0)
		;
	SCI0DRL = c;
	return c;
	}

void InitCANopen ( void )
{
    Reset_Max33011();
  	// Reset/Initialize CANopen communication
  	MCOUSER_ResetCommunication();
  	//while ( !(CANCTL0 & 0x10) );
}

int ResetProc (void)
{
    COPCTL = 0x01;				//enable COP 
	while (1);					//wait for reset
	return 0;
}


