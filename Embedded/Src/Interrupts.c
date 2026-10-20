#include "Interrupts.h"
#include "mc9s12a128.h"
#include "Controller.h"
#include "mco.h"
#include "mcohw.h"
#include "nodecfg.h"
#include "procimg.h"
#include "Subroutines.h"
#include <math.h>

static char Seconds = 60;

static int Tick = RTI_One_Sec;


volatile char Gen_Flags;

volatile unsigned int Timer1;
volatile unsigned int Timer2;
volatile int CntrSwTimeOut;
volatile unsigned int DeBounceTimer;
volatile unsigned int MenuTimer;
volatile unsigned int ToggleTimer;
volatile unsigned int WD_Timer;

char SIN0Buf[SIN0BufLen] = "\0";
volatile int SIN0Bufptr = 0;
char SOUT0Buf[SOUT0BufLen] = "\0";
volatile int SOUT0Bufptr = 0;

static unsigned char TC0_State;
static unsigned char TC0_BitCounter;
static unsigned char QBit;
static unsigned int TC0_Data_Bit;



void DUMMY_ENTRY ( void )
{

}

void CANRxISR ( void )
{
    /* Reception belongs to MCOHW_PullMessage in the main loop. If RX IRQ is
       unexpectedly enabled, mask it without clearing RXF or losing the frame.
       No software receive queue exists for interrupt-driven reception. */
    CANRIER &= ~0x01;
}

void IRQ_Int_Handler ( void )
{

}

void TC0_Int_Handler ( void )
{
 	 TFLG1 = 0x01;	   		//clear interrupt
	 TC0 = TC0 + TC_170us;
	 if ( TC0_BitCounter < 28 )
	 {
	     PTT |= 0x01;
		 QBit = 0;
		 TC0_Data_Bit = 0x8000;
	 }
	 else
	 {
	     if ( QBit < 2 )
		     PTT &= ~0x01;
		 if ( QBit == 2 )
		     PTT |= 0x01;
		 if ( QBit == 3 )
		 {
		     if ( TC0_Data_Bit & ButtonBoxData )
			     PTT &= ~0x01;
			 TC0_Data_Bit = TC0_Data_Bit >> 1; 		 
		 }
		 
		 if ( ++QBit == 4 )
		     QBit = 0;
	 }
	 
	 if ( ++TC0_BitCounter == 96 )
	     TC0_BitCounter = 0;
	 
	 if ( ++TC0_State == 4 )
	     TC0_State = 0;
	 
}

void TC3_Int_Handler ( void )
{
    static unsigned char Sw_Index;
    
    TFLG1 = 0x08;	   		//clear interrupt
    TC3 = TCNT + TC_1ms;
    if ( !legacy_mode )
    {
        MCOHW_TimerISR();
	}

	if ((ButtonBoxData & AlternateFunctionMask) && (Sw_Index == 4 || Sw_Index == 9 || Sw_Index == 10 || Sw_Index == 11 || Sw_Index == 16))
	{
        // LEDs
        PTP = (PTP & ~0x0E) | (Keypad_Matrix[Sw_Index].LED_Port_Data[0] & 0x0E); //Output a High on the Column
        
        PTP = (PTP | 0x11) & ~(Keypad_Matrix[Sw_Index].LED_Port_Data[0] & 0x11);
        PTT = (PTT | 0x06) & ~Keypad_Matrix[Sw_Index].LED_Port_Data[1];
        PTJ = (PTJ | 0xC0) & ~Keypad_Matrix[Sw_Index].LED_Port_Data[2];
	
	}
    else if ( Keypad_Matrix[Sw_Index].BB_Data & ButtonBoxData || ( Sw_Index == 8 && !(ButtonBoxData & (TeleData_Rev | TeleData_Fwd) ) ) )
    {
        // LEDs
        PTP = (PTP & ~0x0E) | (Keypad_Matrix[Sw_Index].LED_Port_Data[0] & 0x0E); //Output a High on the Column
        
        PTP = (PTP | 0x11) & ~(Keypad_Matrix[Sw_Index].LED_Port_Data[0] & 0x11);
        PTT = (PTT | 0x06) & ~Keypad_Matrix[Sw_Index].LED_Port_Data[1];
        PTJ = (PTJ | 0xC0) & ~Keypad_Matrix[Sw_Index].LED_Port_Data[2];
    }
	else
	{
	    PTP = (PTP & ~0x0E); //Output a Low on the Columns
	}
	
	if (++Sw_Index >= 18)
	    Sw_Index=0;
	
	if (Sw_Index == 6)
	    Sw_Index++;
}


void RTI_Int_Handler ( void )
{
 	 CRGFLG = 0x80;	   	//clear interrupt

	if ( MenuTimer )
	{
	    MenuTimer--;
	}

	if ( CntrSwTimeOut>0 )
	{
	    CntrSwTimeOut--;
	}

	if ( ReActivate_Timer && controller_mode )
	{ 
	    ReActivate_Timer--;
		if ( !ReActivate_Timer )
		{
		   Gen_Flags |= Gen_Flags_Menu_Active;
		   Gen_Flags &= ~Gen_Flags_Extern_Menu;
		   UpdateMenu = 1;
		}
	}
    
    if ( ToggleTimer )   //slows changing of cameras to a reasonable rate
    {
	    --ToggleTimer;
    }

    if ( DeBounceTimer )
    {
        --DeBounceTimer;
	}

    if ( Timer1 )
    {
        if ( !--Timer1 )
            Gen_Flags &= ~Gen_Flags_Timer1;
    }
    
    if ( Timer2 )
    {
        if ( !--Timer2 )
            Gen_Flags &= ~Gen_Flags_Timer2;
    }
	
	if (wdStarted)
	{
        if (WD_Timer)
           --WD_Timer;   
        else
        {
            if (wd_flag)
            {
                PORTE |= HW_COP;
                PORTE &= ~HW_COP;
        
                if(!wdInit)
                    wd_flag = 0;
    			
    			WD_Timer = HW_Watchdog_Time;	
            }
        }	
	}
    
    if ( !(Tick--) )
    {
        Tick = RTI_One_Sec;
        if ( !(Seconds--) )
        {
            Seconds = 60;
        
        }
    }	 
	
}

void SCI0_Int_Handler ( void ) 
{
 	 char tmpchar;
	 
 	 if ( SCI0SR1 & SCI0SR1_RDRF )
	 {
    	 tmpchar = SCI0DRL;
	     if ( !(Gen_Flags & Gen_Flags_SIN0Rcvd) )
		 {
			if ( tmpchar != '\n' )
			{
			    SIN0Buf [SIN0Bufptr++] = tmpchar;
    			if ( SIN0Bufptr == SIN0BufLen-1 )
    			{
    		        Gen_Flags |= Gen_Flags_SIN0Rcvd;
    		   		SIN0Buf [SIN0Bufptr] = '\0';
    			}
    			if ( tmpchar == '\r' )
    			{
    		        Gen_Flags |= Gen_Flags_SIN0Rcvd;
    		   		SIN0Buf [SIN0Bufptr] = '\0';
   			   		SIN0Bufptr = 0;
    			}
			}
		  } 
	 }
	 if ( SCI0SR1 & SCI0SR1_TDRE )
	 {
		   
	 }
}


void SCI1_Int_Handler ( void ) 
{

}

//This routine is currently designed to only transmit or receive 2 bytes max.
void SPI0_Int_Handler ( void ) 
{
    //receive
    if (SPI0SR & SPISR_SPIF)
	{
    	SPI0InBuf[SPI0InBufptr] = SPI0DR;
        SPI0InBufptr++;
    }		
	if (SPI0InBufptr == 2)
		SPI0InBufptr = 1;
		
    //transmit
    if (SPI0SR & SPISR_SPTEF)
    {
		if (SPI0OutBufptr == 0)
		{ 
		    SPI0OutBufptr++;
    	    SPI0DR = SPI0OutBuf[SPI0OutBufptr];
        }	
		   
        SPI0CR1 &= ~SPICR1_SPTIE;
    }
	

}


#include "vectors.h"
