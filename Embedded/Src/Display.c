/* SPI transport and video rendering. Storage remains in Subroutines.c. */
#include <string.h>
#include <stdlib.h>

#include "Display.h"
#include "Controller.h"
#include "mc9s12a128.h"
#include "Interrupts.h"
#include "Settings.h"
#include "mco.h"
#include "LegacyCanInterface.h"

void Xmit_Rcv_SPI ( void )
{   
    int i;
	
	if(firstWriteSPI) //dummy write
	{
	    SPI0OutBufptr = 1;
	    PTM &= ~0x08;
		for(i = 0; i < 5; i++);	
	    SPI0DR = 0x01; 
		SPI0CR1 |= SPICR1_SPTIE;
    	while (SPI0CR1 & SPICR1_SPTIE) { /* wait for interrupt */ }
		PTM |= 0x08;
	    firstWriteSPI = false;    
	}
		
 	PTM &= ~0x08;
	for(i = 0; i < 5; i++);	
	SPI0DR = SPI0OutBuf[0];
    SPI0CR1 |= SPICR1_SPTIE;
    while (SPI0CR1 & SPICR1_SPTIE) { /* wait for interrupt */ }
	PTM |= 0x08;
}

void WriteCharMem ( void )
{
    unsigned char CMM, CMAH, CMAL;
	
    SPI0OutBufptr = 0;
   	SPI0OutBuf[0] = 0x00;	   //Disable OSD
   	SPI0OutBuf[1] = 0x00;
   	Xmit_Rcv_SPI();

	Timer1 = .1 * RTI_One_Sec;
	while (Timer1) { /* wait for interrupt */ }

	CMAH = 0x00;
	CMM = 0xa0;
	
    for (CMAH = 0; CMAH < 19; CMAH++)
	{
        CMAL = 0x00;
	    SPI0OutBufptr = 0;
    	SPI0OutBuf[0] = 0x09;	       //Character memory high address (write)
    	SPI0OutBuf[1] = CMAH + 0x4d;   //4d is the location after the @ symbol
    	Xmit_Rcv_SPI();
		
		for (CMAL = 0; CMAL < 54; CMAL++)
		{
		    SPI0OutBufptr = 0;
    		SPI0OutBuf[0] = 0x0a;	   //Character memory low address (write)
    		SPI0OutBuf[1] = CMAL;      //0-53 
    		Xmit_Rcv_SPI();
			
		    SPI0OutBufptr = 0;
			SPI0InBufptr = 0;
    		SPI0OutBuf[0] = 0x0b;	                  //Character memory data in (write)
    		SPI0OutBuf[1] = CMDI[CMAH*54 + CMAL];     //writes data from custom Character Array
    		Xmit_Rcv_SPI();
		}
		
	    SPI0OutBufptr = 0;
    	SPI0OutBuf[0] = 0x08;	       //Character memory mode register (write)
    	SPI0OutBuf[1] = CMM;           //Writes to NVM array from shadow RAM
    	Xmit_Rcv_SPI();
		
		Timer1 = .1 * RTI_One_Sec;
		while (Timer1) { /* wait for interrupt */ }

    }
	Timer1 = .2 * RTI_One_Sec;
	while (Timer1) { /* wait for interrupt */ }
}

void ReadCharMem ( void )
{
    unsigned char CMM, CMAH, CMAL;
	int i;
	
    SPI0OutBufptr = 0;
   	SPI0OutBuf[0] = 0x00;	   //Disable OSD
   	SPI0OutBuf[1] = 0x00;
   	Xmit_Rcv_SPI();

	Timer1 = .1 * RTI_One_Sec;
	while (Timer1) { /* wait for interrupt */ }
	
	CMM = 0x50;
	CMAH = 5;           //determining if the "+" symbol is there is sufficient: CMAH = 5
    CMAL = 0x00;
    SPI0OutBufptr = 0;
    SPI0OutBuf[0] = 0x09;	          //Character memory high address (write)
    SPI0OutBuf[1] = CMAH + 0x4d;      //4d is the location after the @ symbol
    Xmit_Rcv_SPI();
		
	SPI0OutBufptr = 0;
    SPI0OutBuf[0] = 0x08;	       //Character memory mode register (write)
    SPI0OutBuf[1] = CMM;           //Reads from NVM array into shadow RAM
    Xmit_Rcv_SPI();
	
	Timer1 = .1 * RTI_One_Sec;
	while (Timer1) { /* wait for interrupt */ }

    for (CMAL = 0; CMAL < 54; CMAL++)
    {
        SPI0OutBufptr = 0;
     	SPI0OutBuf[0] = 0x0a;	      //Character memory low address (write)
     	SPI0OutBuf[1] = CMAL;         //0-53 
     	Xmit_Rcv_SPI();
   
        SPI0OutBufptr = 0;
     	SPI0OutBuf[0] = 0xc0;	             //Character memory data out (read only)
		Xmit_Rcv_SPI();		
     	plusSymbol[CMAL] = SPI0InBuf[1];     //reads data from Character Array on MAX7456 	
    }

	//compare arrays
	for (i = 0; i < 54; i++)
	{
	   if (plusSymbol[i] != CMDI[CMAH*54 + i])
	   {
	   	  CharsInMem = false;
		  return;
	   }  
	}
	
    CharsInMem = true;
}

void resetTitler(void)
{
	MenuCheck=0;
	
	SPI0OutBufptr = 0;
    SPI0OutBuf[0] = 0x00;  	   //Video Mode0, Reset
    SPI0OutBuf[1] = 0x02;
	Xmit_Rcv_SPI();
		
	Timer1 = RTI_One_Sec * 0.002;  //short delay to allow reset to finish
	while (Timer1) { /* wait for interrupt */ }
		
	SPI0OutBufptr = 0;
    SPI0OutBuf[0] = 0x00;  	   //Video Mode0, Enable display OSD
    SPI0OutBuf[1] = 0x08;
	Xmit_Rcv_SPI();
	
}

//send menu characters to titler via SPI
//puts Menu in proper location, leaves room for messages on top line
void showMenu ( void )
{
    int i, j;

	MenuCheck=1;
	
	for (i=0; i<=8; i++)
	{
	 	int DispAddr;
		
		DispAddr = ( i + 2 ) * 30 + 4;
		
		SPI0OutBufptr = 0;
        SPI0OutBuf[0] = 0x05;  	   //Display memory high address
        SPI0OutBuf[1] = DispAddr>>8;
    	Xmit_Rcv_SPI();
		
		SPI0OutBufptr = 0;
        SPI0OutBuf[0] = 0x06;	   //Display memory low address
        SPI0OutBuf[1] = DispAddr;
    	Xmit_Rcv_SPI();
		    
		SPI0OutBufptr = 0;
        SPI0OutBuf[0] = 0x04;	   //Display memory mode, 8 bit operation mode, auto increment
        SPI0OutBuf[1] = 0x41;
    	Xmit_Rcv_SPI();
					
	 	for ( j=0;j<=19;j++ )
	 	{
			SPI0OutBufptr = 1;
    	 	SPI0OutBuf[0] = DispTranslate ( &DispBuf[i*20+j+6] );
    	 	Xmit_Rcv_SPI();
		}
		
        SPI0OutBufptr = 1;
        SPI0OutBuf[0] = 0xff;
        Xmit_Rcv_SPI();
	}
}

char DispTranslate ( char *TranChar )
{
 	char RetChar;
	
    if ( *TranChar == '0' )
        RetChar = 0x0a;
    else if ( *TranChar >= '1' && *TranChar <= '9' )
        RetChar = *TranChar - 0x30;
    else if ( *TranChar >= 'A' && *TranChar <= 'Z' )
        RetChar = *TranChar - 0x36;
    else if ( *TranChar >= 'a' && *TranChar <= 'z' )
        RetChar = *TranChar - 0x3c;
    else if ( *TranChar == ' ' )
        RetChar = 0x00;
    else if ( *TranChar == 'v' )
        RetChar = 0x3a;
    else if ( *TranChar == '(' )
        RetChar = 0x3f;
    else if ( *TranChar == ')' )
        RetChar = 0x40;
    else if ( *TranChar == '.' )
        RetChar = 0x41;
    else if ( *TranChar == '?' )
        RetChar = 0x42;
    else if ( *TranChar == ';' )
        RetChar = 0x43;
    else if ( *TranChar == ':' )
        RetChar = 0x44;
    else if ( *TranChar == ',' )
        RetChar = 0x45;
    else if ( *TranChar == 0x27 ) // '
        RetChar = 0x46;
    else if ( *TranChar == '/' )
        RetChar = 0x47;
    else if ( *TranChar == 0x22 ) // "
        RetChar = 0x48;
    else if ( *TranChar == '-' )
        RetChar = 0x49;
    else if ( *TranChar == '<' )
        RetChar = 0x4a;
    else if ( *TranChar == '>' )
        RetChar = 0x4b;
    else if ( *TranChar == '@' )
        RetChar = 0x4c;
    else if ( *TranChar == 0x5c )     // '\' 
        RetChar = 0x4D;
    else if ( *TranChar == '[' )
        RetChar = 0x4E;
    else if ( *TranChar == ']' )
        RetChar = 0x4F;
    else if ( *TranChar == '&' )
        RetChar = 0x50;
    else if ( *TranChar == '^' )
        RetChar = 0x51;
    else if ( *TranChar == '+' )
        RetChar = 0x52;
    else if ( *TranChar == '=' )
        RetChar = 0x53;
    else if ( *TranChar == '_' )
        RetChar = 0x54;
    else if ( *TranChar == '{' )
        RetChar = 0x55;
    else if ( *TranChar == '}' )
        RetChar = 0x56;
    else if ( *TranChar == '*' )
        RetChar = 0x57;
    else if ( *TranChar == '!' )
        RetChar = 0x58;
    else if ( *TranChar == '%' )
        RetChar = 0x59;
    else if ( *TranChar == '$' )
        RetChar = 0x5A;
    else if ( *TranChar == '#' )
        RetChar = 0x5B;
	else if ( *TranChar == '|' )
        RetChar = 0x5C;	
		
	else if ( *TranChar == '\x11' )
        RetChar = 0x5D;
    else if ( *TranChar == '\x12' )
        RetChar = 0x5E;
	else if ( *TranChar == '\x13' )
        RetChar = 0x5F;	
		
	else if ( *TranChar == 0x03 )   //<ETX>
        RetChar = 0x00;
	else if ( *TranChar == 0x02 )   //<STX>
        RetChar = 0x00;

    else if ( *TranChar == 0x04 ) // v
        RetChar = 0x3a;
    else if ( *TranChar == 0x06 ) // >
        RetChar = 0x4b;
    else if ( *TranChar == 0x01 ) // space
        RetChar = 0x00;
        //RetChar = 0x20;
    else if ( *TranChar == 0x3d ) // =
        RetChar = 0x49;
        
    else
	    RetChar = *TranChar;

    return RetChar;
}
/***
numbers 1-9	  x - 0x30
number 0	  x -0x26
letters A-Z	  z - 0x36
letters a-z	  z - 0x3c
symbols		  (  )  .  ?  ;  :  ,  '  /  "  -  <  >  @
		      3f 40 41 42 43 44 45 46 47 48 49 4a 4b 4c
		      28 29 2e 3f 3b 3a 2c 27 2f 22 2d 3c 3e 40
***/

void DisplayMsg ( char Message[] )
{
    if( VCI_Flag2 & ICAM_MSG )
        showMessage( Message, atoi(MessageRow) );              //puts Proc:, Warn:, etc messages on video top line
}

void showMessage ( char Message[], int row )       //row = position on video screen, skip row 0
{
    char endofmsg = 0; 
    int column;     //position on video screen
    int index = 0;
    int DispAddr;

    //see if external menu is not active and internal menu is not active
	if ( (gProcImg[OUT_digi_0] & 0x01) || (( Gen_Flags & Gen_Flags_Menu_Active ) && MenuCheck==0))
	{
	    //some menu is active, display on top or bottom row, column selection is okay
    	//if ( row == 1 || row >= 6 )
    	    //DispAddr = 12 * 30 + atoi(MessageCol);
    	//else
    		DispAddr = 1 * 30 + atoi(MessageCol);
	
	}
	else if(MenuCheck==1)
	{
	 	 DispAddr = 1 * 30 + atoi(MessageCol);
	}
	else
	{
	    //no menu is active
        DispAddr = row * 30 + atoi(MessageCol);
	}
    
    SPI0OutBufptr = 0;
    SPI0OutBuf[0] = 0x05;  	   //Display memory high address
    SPI0OutBuf[1] = DispAddr>>8;
    Xmit_Rcv_SPI();
    
    SPI0OutBufptr = 0;
    SPI0OutBuf[0] = 0x06;	   //Display memory low address
    SPI0OutBuf[1] = DispAddr;
    Xmit_Rcv_SPI();
    
    SPI0OutBufptr = 0;
    SPI0OutBuf[0] = 0x04;	   //Display memory mode, 8 bit, auto increment
    SPI0OutBuf[1] = 0x41;
    Xmit_Rcv_SPI();
    
    for ( column=0;column<=24;column++ )   
    {
        SPI0OutBufptr = 1;
		if ( !endofmsg )
		{
		    SPI0OutBuf[0] = DispTranslate ( &Message[6+index++] );
            if ( ( Message[6+index] ) == 0x03 )
			    endofmsg = 1;
		}
		else
		{
		    SPI0OutBuf[0] = 0;
        }
		Xmit_Rcv_SPI();
    }
    SPI0OutBufptr = 1;
    SPI0OutBuf[0] = 0xff;
    Xmit_Rcv_SPI();

    if( VCI_Flag2 |= ICAM_MSG )      //if its a camera position
        VCI_Flag2 &= ~ICAM_MSG;      //clear flag
}

void ClearTitler ( void )
{
//    TCPClearTitler();
    DispBufptr = 0;
	if ( !legacy_mode )//!(Gen_Flags & Gen_Flags_No2Wire) )
	    MCO_ProcessStack();
    Timer1 = RTI_One_Sec * .05;
	while (Timer1) { /* wait for interrupt */ }
	resetTitler();
}

//send characters to titler via SPI
void DisplayTitler ( void )
{
    int i,j;
	int DispAddr;

    //if( VCI_Flag & TCP_PRESENT )
    //TCPDisplayTitler();
    //else if( Gen_Flags & Gen_Flags_Extern_Menu )
    if( Gen_Flags & Gen_Flags_Extern_Menu )
        showMenu();     //only for menus over CAN

    else        //used for VCI menu only
    {
        for ( i=0;i<=13;i++)
		{
            DispAddr =  ( i + 1) * 30 + 1;
        
			SPI0OutBufptr = 0;
			SPI0OutBuf[0] = 0x05;  	   //Display memory high address
			SPI0OutBuf[1] = DispAddr>>8;
			Xmit_Rcv_SPI();
    
			SPI0OutBufptr = 0;
			SPI0OutBuf[0] = 0x06;	   //Display memory low address
			SPI0OutBuf[1] = DispAddr;
			Xmit_Rcv_SPI();
    
			SPI0OutBufptr = 0;
			SPI0OutBuf[0] = 0x04;	   //Display memory mode, 8 bit, auto increment
			SPI0OutBuf[1] = 0x41;
			Xmit_Rcv_SPI();
	
			for ( j=0;j<=27;j++ )
			{
				SPI0OutBufptr = 1;
                SPI0OutBuf[0] = DispTranslate ( &DispBuf[i*28+j] );
				Xmit_Rcv_SPI();
			}

			SPI0OutBufptr = 1;
			SPI0OutBuf[0] = 0xff;
			Xmit_Rcv_SPI();
    	}

    }
}
