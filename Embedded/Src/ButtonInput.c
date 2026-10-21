/* Button scan and command encoding. Persistent state remains in Subroutines.c. */
#include "ButtonInput.h"
#include "Subroutines.h"
#include "Interrupts.h"
#include "mc9s12a128.h"
#include "mco.h"

extern UNSIGNED8 gProcImg[PROCIMG_SIZE];

/* Slow toggles in ordinary mode and acts as a momentary navigation key
   while an enabled F1/F2 mode is selected. Recompute from current scan data. */
static unsigned int SlowToggleMask(unsigned int data)
{
    return (EnabledFunctionData(data) & (TeleData_F1 | TeleData_F2))
        ? 0 : TeleData_Slow;
}

/* Function bits keep latching even when their alternate mode is disabled. */
static unsigned int LatchedButtonMask(unsigned int data)
{
    return TeleData_F1 | TeleData_F2 | TeleData_F3 | TeleData_Rev |
           TeleData_Fwd | TeleData_ExSlow | SlowToggleMask(data) | TeleData_Fast;
}

/* Called only when the existing debounce gate in doevents has expired. */
void ScanButtons(unsigned int *releasedSwitches)
{
	unsigned int SwitchData; 
	unsigned char Sw_Index;
	char StopFlag=0;
	
	DeBounceTimer = DeBounceTime;
	
	SwitchData = ButtonBoxData;
	
	//Keep all latched data
	SwitchData &= LatchedButtonMask(SwitchData);

	//Data Port T     Port B     Port P     Port T     Port J
	//	   Switch	  Switch	 LED  		LED	 	   LED
	//     0b01110000,0b00111111,0b00011111,0b00000110,0b11000000,		Mask bits
	//        RRR         CCCRRR      RCCC4        RR    RR      
	//        456         BCA123      11234        65    32             

	for(Sw_Index=0;Sw_Index<18;Sw_Index++)
	{	//See which button is pressed/not pressed and set SwitchData accordingly
		PORTB = (PORTB & ~0x38) | (~Keypad_Matrix[Sw_Index].SW_Port_Data[1] & 0x38);		 //Output Columns
		Timer1 = RTI_One_Sec * .001;
		while (Timer1) { /* wait for interrupt */ }
		if ( (~PTT & Keypad_Matrix[Sw_Index].SW_Port_Data[0]) || (~PORTB & (Keypad_Matrix[Sw_Index].SW_Port_Data[1] & 0x07)) )
		{   //Switch is pressed
			if ( Keypad_Matrix[Sw_Index].BB_Data & LatchedButtonMask(SwitchData) )
			{   //Switch is a toggle function 
				if ((*releasedSwitches) & Keypad_Matrix[Sw_Index].BB_Data) 
				{	//Switch is pressed and was previously released
					SwitchData = SwitchData ^ Keypad_Matrix[Sw_Index].BB_Data;      //exclusive OR to toggle bit
					(*releasedSwitches) = (*releasedSwitches) & ~Keypad_Matrix[Sw_Index].BB_Data;	//Clear previously released bit				
				}
				
				if (MenuCheck)
				    SwitchData &= ~EnabledFunctionData(TeleData_F1 | TeleData_F2);  // block enabled alternate modes while menu is present
			}
			else
			{   //Switch is not a toggle function 
				SwitchData = SwitchData | Keypad_Matrix[Sw_Index].BB_Data;      //Set bit
				if ( !Keypad_Matrix[Sw_Index].BB_Data )							//Stop button has a zero in BB_Data
				StopFlag = 1;
			}

		} 
		else
		{   //Switch is not pressed
			(*releasedSwitches)  = (*releasedSwitches) | Keypad_Matrix[Sw_Index].BB_Data;
		}
	}

	if ( SwitchData & TeleData_Data )
	    Disp_Menu_Port |= Disp_Menu_Bit;		// The Data bit is temporarily used for the option button	
	else
	    Disp_Menu_Port &= ~Disp_Menu_Bit;
	
	if ( SwitchData & TeleData_Up && !(SwitchData & TeleData_Down) )
	    Disp_Up_Port |= Disp_Up_Bit;
	else
	    Disp_Up_Port &= ~Disp_Up_Bit;
	
	if ( SwitchData & TeleData_Down && !(SwitchData & TeleData_Up) )
	    Disp_Down_Port |= Disp_Down_Bit;	
	else
	    Disp_Down_Port &= ~Disp_Down_Bit;

	if ( gProcImg[OUT_digi_12] & 0x34 )
	{
		CntrSwTimeOut = RTI_One_Sec * 0.125; 
		//C48 Controller Switches
		Cntrl_Slow_Flag = 0;

		if( gProcImg[OUT_digi_12] & 0x10 )      //forward is pressed
		{
			SwitchData |= (TeleData_Fwd | TeleData_Slow);
			SwitchData &= ~(TeleData_Rev | TeleData_ExSlow | TeleData_Fast);
			Cntrl_Slow_Flag = 1; // This will prevent the Slow button from being cleared below

		}
		if( gProcImg[OUT_digi_12] & 0x20 )      //reverse is pressed
		{
			SwitchData |= (TeleData_Rev | TeleData_Slow);
			SwitchData &= ~(TeleData_Fwd | TeleData_ExSlow | TeleData_Fast);
			Cntrl_Slow_Flag = 1; // This will prevent the Slow button from being cleared below				
		}
		if( gProcImg[OUT_digi_12] & 0x04 )      //inhibit is pressed
		{
			StopFlag = 1;
		}
		
		gProcImg[OUT_digi_12] = gProcImg[OUT_digi_12] & ~0x34;
	}
	else if ( CntrSwTimeOut == 0 )
	{
		CntrSwTimeOut = -1;
		StopFlag = 1;
	}
	
	if ( StopFlag )	   	  				 			   	 				//Stop pressed
	    SwitchData &= ~(TeleData_Fwd | TeleData_Rev);
		
	if ( !(SwitchData & TeleData_Rev) && !(SwitchData & TeleData_Fwd) ) //No Reverse or Forward
	    SwitchData &= ~(TeleData_ExSlow | SlowToggleMask(SwitchData) | TeleData_Fast);
	
	if ( SwitchData & TeleData_Fwd && (*releasedSwitches) & TeleData_Rev ) //FORWARD
	    SwitchData &= ~TeleData_Rev;
	
	if ( SwitchData & TeleData_Rev && (*releasedSwitches) & TeleData_Fwd ) //REVERSE
	    SwitchData &= ~TeleData_Fwd;

	if ( SwitchData & TeleData_F1 && (*releasedSwitches) & TeleData_F2 && (*releasedSwitches) & TeleData_F3 ) //F 1
		SwitchData &= ~(TeleData_F2 | TeleData_F3);
	if ( SwitchData & TeleData_F2 && (*releasedSwitches) & TeleData_F1 && (*releasedSwitches) & TeleData_F3 ) //F 2
		SwitchData &= ~(TeleData_F1 | TeleData_F3);
	if ( SwitchData & TeleData_F3 && (*releasedSwitches) & TeleData_F1 && (*releasedSwitches) & TeleData_F2 ) //F 3
		SwitchData &= ~(TeleData_F1 | TeleData_F2);

	SwitchData = SwitchData | 0x0400;
	ButtonBoxData = SwitchData;
 		if (ButtonBoxData & TeleData_Down && ButtonBoxData & TeleData_Up && controller_mode && !(EnabledFunctionData(TC0_RCVD_Data) & (TeleData_F1 | TeleData_F2)) ) //Don't output menu command when in "internal menu mode"
	{
		ButtonBoxData = ButtonBoxData & ~TeleData_Down & ~TeleData_Up;
		if (!(Gen_Flags & Gen_Flags_Menu_Active))
		    internal_menu = 1;
	}
}

void EncodeButtonCommands(void)
{

	tempData = ButtonBoxData;
	if (Gen_Flags & Gen_Flags_Menu_Active && !(Gen_Flags & Gen_Flags_Extern_Menu))  
	{
		tempData = ButtonBoxData & ~TeleData_Down & ~TeleData_Up;              //Don't output toggles when menu active
	}
	
	Third_byte = 0;
	if ( EnabledFunctionData(tempData) & (TeleData_F1 | TeleData_F2) )
	{
		if (Gen_Flags & Gen_Flags_Menu_Active)
		{   //exit menu if active
		    StackPointer = 0;
			DeSelect();
		}
		if((tempData & TeleData_Down) && (tempData & TeleData_Up)) // Menu
		    Third_byte |= 0x10;
		else if(tempData & TeleData_Down) // Left
		    Third_byte |= 0x04;
		else if(tempData & TeleData_Up) // Right
		    Third_byte |= 0x08;
			
		if(tempData & TeleData_Slow) // Up
		    Third_byte |= 0x01;
		if(tempData & TeleData_RotCW) // Dn
		    Third_byte |= 0x02;
			
		tempData &= ~(TeleData_Down | TeleData_Up | (Cntrl_Slow_Flag?0:TeleData_Slow) | TeleData_RotCW); // These keys are used for Menu navigation when F1 or F2 is selected
	}
	
	gProcImg[IN_digi_0] = tempData & 0xFF;
	gProcImg[IN_digi_1] = tempData >> 8 & 0xFF;
	gProcImg[IN_digi_2] = Third_byte;
}
