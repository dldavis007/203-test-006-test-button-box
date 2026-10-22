#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "Subroutines.h"
#include "Controller.h"
#include "mc9s12a128.h"
#include "Interrupts.h"
#include "Command.h"
#include "mcohw.h"
#include "mco.h"
#include "EEProm.h"

/* Application access to the legacy MCO integration; keep stack files unchanged. */
extern UNSIGNED8 gProcImg[PROCIMG_SIZE];

/* Button scan, transmitted data and alternate-function selection. */
unsigned int TC0_RCVD_Data;

volatile char UpdateMenu;

static CAN_MSG gTxMsg;

unsigned char ExSlowPushed;
unsigned char SlowPushed;
unsigned char FastPushed;

volatile unsigned int ButtonBoxData;
unsigned int LEDData;
volatile unsigned char AlternateFunctionMask = TeleData_F1 | TeleData_F2;
unsigned int tempData;
unsigned int Third_byte;
char Cntrl_Slow_Flag;

/* Display SPI exchange; the interrupt handler shares these buffers. */
volatile char SPI0InBuf[2];
volatile int SPI0InBufptr = 0;
volatile char SPI0OutBuf[2];
volatile int SPI0OutBufptr = 0;

int dispPBJTFlag = 0;

char DispBuf[DispBufLen];          //used to display keyboard input
int DispBufptr = 0;                //index for DispBuf[481]

volatile int ReActivate_Timer;

/* Startup mode and device-discovery state. */
char power_on = 1;
char Ext_Menu_xmitd;

bool resequenceNodes = false;
bool controller_mode = false;
volatile bool legacy_mode = false;
bool vid18_addr = false;
bool vid18_update = false;
bool F3pressed = false;

char deviceSubMenu[20][17];
int sub_Device_Type_Response_ID[20];

bool AuxDeviceLimitExceeded = false;
bool AuxDeviceMenuFull = false;

/* Received video messages and display assembly. */
char VCI_Flag2;     //more flags specific to VCI
char MsgIDtable[5];
char UpdateDisp;
char STX_count;            //count <STX>, 0x02, that indicates beginning of tag
char ETX_flag;             //count <ETX>, 0x03, that indicates end of tag
char XmitSPI_flag;
char RcvSPI_flag;
struct MsgStruct
{
	unsigned char Bufptr;
	char Buffer[30];
};

/* Internal menu and decoded key state. */
char MenuCheck;
char internal_menu;

signed char CursorDownFlag;
signed char CursorUpFlag;
signed char SelectFlag;
char Trigger_DeviceFlag;

/* Camera addresses and display tags. */
int cam_list[10];   //each elements holds a 2 byte camera address
char extCamTagList[10][11];
int cam_select;    //index of cam_list to select camera

/* Character-display initialization. */
bool firstWriteSPI = true;

bool CharsInMem = false;
bool goLow = false;
char plusSymbol[54];
#include "CharacterFontData.inc"

//The following Menu Variables are saved in EEPROM
char FirstVar[]       = "xxx";
char MessageRow[]	  = "  1";
char MessageCol[]	  = "  1";
char SerialNum[]      = "-----";
char F1Enable[4]      = " ON";
char F2Enable[4]      = " ON";
char NullVar[]        = "";

//The following Menu Variables are NOT saved in EEPROM

char Rev[]		      = Revision;

#include "MenuTables.inc"

unsigned char StackPointer = 0;

char Menu[12][21];

char VariableFlag = 0;

double increment = 1.0;          //used to increment and decrement variables
double uLimit = 25.0;            //numerical upper limit
double lLimit = 0.0;             //numerical lower limit
int nDigits = 2;

struct MsgStruct Messages[4];      //Proc:, Icam:, Warn:, Emsg:, messages

/* Button matrix and LED wiring. */

#include "KeypadMatrix.inc"


/* Select active alternate-key modes without changing latched/output bits. */
unsigned int EnabledFunctionData(unsigned int data)
{
    if (strcmp(F1Enable, " ON")) data &= ~TeleData_F1;
    if (strcmp(F2Enable, " ON")) data &= ~TeleData_F2;
    return data;
}

int ATDGetLevel ( char ATD_Num )
{
 	ATD0CTL5=ATD0CTL5_Init | ATD_Num;
	while ( !(ATD0STAT0 & 0x80) );
	
	return (ATD0DR0+ATD0DR1+ATD0DR2+ATD0DR3)/4; 				
}

/* Keep the public API and shared static buffer at their existing owner. */
void ext_menu(void)
{
    ActivateDeviceMenus(&gTxMsg);
}

void Scan(void)
{
    ScanDevices(&gTxMsg);
}

/* Preserve the existing shared CAN buffer and public discovery entry points. */
void scan_cameras(void)
{
    ScanCameraAddresses(&gTxMsg);
}

void addressCamera(void)
{
    RandomizeCameraAddresses(&gTxMsg);
}

void doevents ( void )
{

	if (TC0_RCVD_Data != (ButtonBoxData << 8 | ButtonBoxData))
	{
		TC0_RCVD_Data = ButtonBoxData;
	}

	//see if external menu is not active
	if (!(gProcImg[OUT_digi_0] & 0x01) && controller_mode)
	{
		//external menu is not active
		if ( Gen_Flags & Gen_Flags_Menu_Active )
		{
			if ( ( TC0_RCVD_Data & TeleData_Down ) && CursorDownFlag == 0 )
			    CursorDownFlag = 1;
			else if ( CursorDownFlag == -1 && !( TC0_RCVD_Data & TeleData_Down ) )
			    CursorDownFlag = 0;
			
			if ( ( TC0_RCVD_Data &  TeleData_Up ) && CursorUpFlag == 0 )
			    CursorUpFlag = 1;
			else if ( CursorUpFlag == -1 && !( TC0_RCVD_Data & TeleData_Up ) )
			    CursorUpFlag = 0;
			
			if ( ( TC0_RCVD_Data & TeleData_Select ) && SelectFlag == 0 )
			   SelectFlag = 1;
			else if ( SelectFlag == -1 && !( TC0_RCVD_Data & TeleData_Select ) )
			    SelectFlag = 0;
			if ( ( TC0_RCVD_Data & TeleData_Trig ) && Trigger_DeviceFlag == 0 )
			    Trigger_DeviceFlag = 1;
			else if ( Trigger_DeviceFlag == -1 && !( TC0_RCVD_Data & TeleData_Trig ) )
			    Trigger_DeviceFlag = 0;
			
			if ( !MenuTimer )
			{
				MenuTimer = MenuTime;
				
				if ( CursorDownFlag )
				{
					CursorDown ();
					CursorDownFlag = -1;
				}
				if ( CursorUpFlag )
				{
					CursorUp ();
					CursorUpFlag = - 1;
				}					
				
				if ( SelectFlag )
				{
					MenuTimer = MenuTime * 2; //briefly disable select after menu is selected
					Select ();          //select menu item at cursor position
					SelectFlag = - 1;
				}
				
				if ( Trigger_DeviceFlag )
				{
					Trigger_Device ();
					Trigger_DeviceFlag = - 1;
				}

				if ( UpdateMenu )
				{
					LoadMenu (MenuStackc[StackPointer].Index);
					if (!VariableFlag)
					    InsertCursor ();
					btnDisplayTitler ();
					UpdateMenu = 0;
				}
			}
		}
		else
		{   //Neither Internal nor External menu active
			if ( controller_mode && !(EnabledFunctionData(TC0_RCVD_Data) & (TeleData_F1 | TeleData_F2)) )  //no internal menu functions if F1 or F2 pressed
			{
				if (internal_menu)
				{
					StackPointer = 0;
					MenuStackc[StackPointer].Index[0] = 0;
					MenuStackc[StackPointer].Index[1] = 0;
					MenuStackc[StackPointer].Index[2] = 0;
					MenuStackc[StackPointer].Index[3] = 0;
					MenuStackc[StackPointer].CursorPos = 1;
					MenuStackc[StackPointer].FirstLine = 0;
					Gen_Flags |= Gen_Flags_Menu_Active;
					
					LoadMenu( MenuStackc[StackPointer].Index );
					InsertCursor();
					btnDisplayTitler();			
					TC0_RCVD_Data &= ~0x07;  //Make sure up/down/select not active
					CursorDownFlag = 0;
					CursorUpFlag = 0;
					SelectFlag = 0;
					internal_menu = 0;
				}
				else if (!ToggleTimer && (TC0_RCVD_Data & TeleData_Down) && !(TC0_RCVD_Data & TeleData_Select))
				{
					ToggleTimer = RTI_One_Sec * 0.75;						
					
					if(cam_select <= 0)
					{
						cam_select = 9;         //wrap around and check for camera present
						while(!cam_list[cam_select])    --cam_select;   //decrement until camera is present
					}
					else
					{
						--cam_select;    //index of cam_list to select next camera
					}

					gProcImg[IN_digi_3] = cam_list[cam_select];           //lsb 
					gProcImg[IN_digi_3 + 1] = cam_list[cam_select]>>8;    //msb
					MCO_TransmitPDO(0);
				}
				else if (!ToggleTimer && (TC0_RCVD_Data & TeleData_Up) && !(TC0_RCVD_Data & TeleData_Select))
				{
					ToggleTimer = RTI_One_Sec * 0.75;
					
					if(cam_select >= 9 || !cam_list[cam_select + 1])
					{
						cam_select = 0;         //wrap around and check for camera present
					}
					else
					{
						++cam_select;    //index of cam_list to select next camera
					}
					
					gProcImg[IN_digi_3] = cam_list[cam_select];           //lsb 
					gProcImg[IN_digi_3 + 1] = cam_list[cam_select] >> 8;    //msb
					MCO_TransmitPDO(0);
				}
			}

		}	
	}
	else if (controller_mode)
	{
		//External Menu Active
		ReActivate_Timer = ReActivate_Time;
		Gen_Flags |= Gen_Flags_Extern_Menu;		
	}

	if (UpdateDisp)
	{
	    if (Gen_Flags & Gen_Flags_Extern_Menu) 
		    DisplayTitler();            //put menu on video
		
		UpdateDisp = 0;
	}

	if ( Gen_Flags & Gen_Flags_SIN0Rcvd )
	{
		//Display ( SIN0Buf );
		command ( SIN0Buf );
		SIN0Bufptr = 0;
		Gen_Flags &= ~Gen_Flags_SIN0Rcvd;
	}

	if ( !DeBounceTimer )
	{
		static unsigned SwitchUpData;
		ScanButtons(&SwitchUpData);
	}
	EncodeButtonCommands();
	
	if (ButtonBoxData & TeleData_Reset) //Reset
	{
		ButtonBoxData = 0x400 | TeleData_Reset;   //Clear all latched buttons
		resetTitler();
		
		if (controller_mode)
		{
			gTxMsg.ID = 0x00;     //send reset over CAN
        	gTxMsg.LEN = 2;              		    
        	gTxMsg.BUF[0] = 0x81;              		
            gTxMsg.BUF[1] = 0x00;
            if (!MCOHW_PushMessage(&gTxMsg))
        	{   // failed to transmit
            	MCOUSER_FatalError(0x8801);
            }
			ResetProc();  
		}
	}
	
	if (ButtonBoxData & TeleData_F3 && !F3pressed) //F3
	{
	    //F3pressed = true;
	}
	
	if (ButtonBoxData & TeleData_F2) //F2
	{
	    //F3pressed = false;
	}
	
	if (ButtonBoxData & TeleData_F2 && ButtonBoxData & TeleData_F3 && !(Gen_Flags & Gen_Flags_Extern_Menu))
	{   
		static int i, j, b, br;
		static char beforeSp[31], beforeSpRev[31], temp[31], temp2[31];
		
		if (!dispPBJTFlag)
		{
		    i = 0;
		    b = 20;
			br = 20;
			memset(beforeSp, 0, sizeof(beforeSp));         
			memset(beforeSpRev, 0, sizeof(beforeSpRev));
			memset(temp, 0, sizeof(temp));
			memset(temp2, 0, sizeof(temp2));
			Gen_Flags &= ~Gen_Flags_Menu_Active;
		    ClearTitler();
		    sprintf(beforeSp, "%*s", b, "");
			sprintf(beforeSpRev, "%*s", br, "");
			goLow = false;
		}	
		dispPBJTFlag = 1; 
		
		if (i == 0)
		{
		    for (j = 1; j < 13; j+=2)
			{ 
			    sprintf(temp, "%s\x11", beforeSp); 
			    showMessage(temp, j);	
            }
		    for (j = 2; j < 14; j+=2)
			{ 
			    sprintf(temp2, "%s\x11", beforeSpRev); 
			    showMessage(temp2, j);	
            }
              
			Timer1 = RTI_One_Sec * .2;
			while (Timer1) { /* wait for interrupt */ }
		}
		else if (i == 1)
		{
		    for (j = 1; j < 13; j+=2)
			{
			    sprintf(temp, "%s\x12", beforeSp); 
			    showMessage(temp, j);	
            }
			for (j = 2; j < 14; j+=2)
			{ 
			    sprintf(temp2, "%s\x12", beforeSpRev); 
			    showMessage(temp2, j);	
            }

			Timer1 = RTI_One_Sec * .2;
			while (Timer1) { /* wait for interrupt */ }
		}
		else if (i == 2)
		{
		    for (j = 1; j < 13; j+=2)
			{
			    sprintf(temp, "%s\x13", beforeSp); 
			    showMessage(temp, j);	
            }
			for (j = 2; j < 14; j+=2)
			{ 
			    sprintf(temp2, "%s\x13", beforeSpRev); 
			    showMessage(temp2, j);	
            }

			Timer1 = RTI_One_Sec * .2;
			while (Timer1) { /* wait for interrupt */ }
		}
		i++;

		if (i == 3)
		    i = 0;
			
		if (b == 30)	
		   goLow = true;
		else if (b == 10)
		   goLow = false;   
		   
		if (goLow)
		{
		    b--;  
			br++;  
		}	
		else
		{
		    b++;     
			br--;  
		}	 
 
		sprintf(beforeSp, "%*s", b, "");	 	
		sprintf(beforeSpRev, "%*s", br, "");	
	}
	else 
	{ 
		if (dispPBJTFlag)
		{
			if (controller_mode)
			    Gen_Flags |= Gen_Flags_Menu_Active;
				
			ClearTitler();
			UpdateMenu = 1;
			dispPBJTFlag = 0;
		}	
	}	

	if ( ButtonBoxData & TeleData_Trig && ButtonBoxData & TeleData_F1 ) //AKR Record
	{
		Alarm_Port |= Alarm_Bit;
	}
	else
	{
		Alarm_Port &= ~Alarm_Bit;
	}
	
	//if the 2-Wire system is working, start updating the 2-Wire Stack
	if ( !legacy_mode )//!(Gen_Flags & Gen_Flags_No2Wire) )
	{
		//Operate on CANopen protocol stack
		MCO_ProcessStack();
		//End MicroCanOpen Stack
	}   
	
	//if the 2-Wire system is working, start updating the 2-Wire Stack
	if ( !(Gen_Flags & Gen_Flags_No2Wire) )
	{
		if ( power_on && ( ButtonBoxData & TeleData_RotCCW || !strncmp(_bss_end,"Cntrl",6)) )
		{
	        controller_mode = true;
			strcpy(_bss_end,"Cntrl");				
			Gen_Flags |= Gen_Flags_Menu_Active;


		    //code before Scan() is so that the RotateCCW doesn't get sent out the CAN bus causing the machines to rotate once the scan is complete.
			ButtonBoxData &= ~TeleData_RotCCW;
			gProcImg[IN_digi_0] = ButtonBoxData & 0xFF;
			gProcImg[IN_digi_1] = ButtonBoxData >> 8 & 0xFF;
			gProcImg[IN_digi_2] = 0;
	        ButtonBoxData |= TeleData_RotCCW;  //set the CCW bit back here so that the light comes on the RotateCCW button
			Scan();
						
			scan_cameras();
		}
		else if ( power_on )
		{
		    strcpy(_bss_end,"Not Cntrl");
		}
		
		power_on = 0;
		
		bldMessages();
		
	}
	else
	{
		if ( power_on && ButtonBoxData & TeleData_RotCCW )
		{
			controller_mode = true;
			Gen_Flags |= Gen_Flags_Menu_Active;
			UpdateMenu = 1;
		}
		if ( power_on )
		    legacy_mode = true;
		power_on = 0;
	}
	
	if(controller_mode && gProcImg[OUT_digi_9 + 2] == 0xFF)  //msg received from VID-018 to update camlist
	{     
	    gProcImg[OUT_digi_9 + 2] = 0;   
		vid18_update = true;  
		showMessage("Proc: Updating Camera List... ", 1);
        findCameras();
    } 
	
}

void bldMessages(void)
{
    int i;
    char CANBuf[18];
    static int DispBufptr;
    ETX_flag = 0;
    if ( gProcImg[OUT_digi_1+1] )
	{
		memcpy ( CANBuf, &gProcImg[OUT_digi_1+1], 7 );
		CANBuf[7]=0;
		if( strchr( CANBuf, 0x03 ))
		{
            ETX_flag = 1;
        }

		if ( CANBuf[0] == 0x02 )	//<STX> is part of message
		{
            if ( !gProcImg[OUT_digi_1] )    //Menu messages here
			{
			    if ( !strcmp ( &CANBuf[1], "Clr:\x3" ) )
                {
			        ClearTitler ();
					Gen_Flags &= ~Gen_Flags_Extern_Menu;         //this clears the menu from C48 controller when BBox is not in controller mode
					UpdateDisp = 1;
                }

			    else if ( !strncmp( &CANBuf[1], "Home:",5 ) )
			    {
			        DispBufptr = 0;
                    UpdateDisp = 0;         //delay showing menu until it's populated
					Gen_Flags |= Gen_Flags_Extern_Menu;         //this displays menu from C48 controller when BBox is not in controller mode
                }
			    else if (  !strncmp( &CANBuf[1], "Menu:", 5 ))
			    {
                    DispBufptr = 0;                             //start at index 0
                    strcpy ( &DispBuf[DispBufptr],CANBuf );     //copy to DispBuf
                    DispBufptr = DispBufptr + strlen ( CANBuf );
			    }
		    }
			else        //Other messages here
			{
			    i = 0;
				while ( MsgIDtable[i] && MsgIDtable[i] != gProcImg[OUT_digi_1] && i++<3 );
				MsgIDtable[i] = gProcImg[OUT_digi_1];

				if ( Messages[i].Bufptr !=0 )  //Make sure last message went out
                {
                    Messages[i].Buffer[Messages[i].Bufptr] = 0x03;
                    Messages[i].Buffer[Messages[i].Bufptr+1] = 0x00;
//                    sout(1, Messages[i].Buffer );            //output it if didn't already go
					DisplayMsg ( Messages[i].Buffer );
                } 
				Messages[i].Bufptr=0;
				Messages[i].Buffer[0]=0;
				
                if ( !strcmp ( &CANBuf[1], "Clr:\x3" ) )
			        ClearTitler ();
			    if ( !strncmp( &CANBuf[1], "Home:",5 ) || !strncmp( &CANBuf[1], "Menu:", 5 ) 
			        ||  !strncmp( &CANBuf[1], "Warn:", 5 ) ||  !strncmp( &CANBuf[1], "Proc:", 5 )
                    ||  !strncmp( &CANBuf[1], "Icam:", 5 ) ||  !strncmp( &CANBuf[1], "Emsg:", 5 )
                    ||  !strncmp( &CANBuf[1], "Xmsg:", 5 ) )
			    {
			        if ( CANBuf[6] )
    			    {
    			        strcpy ( &Messages[i].Buffer[Messages[i].Bufptr],CANBuf );
    			        Messages[i].Bufptr = Messages[i].Bufptr + strlen ( CANBuf );
                        if ( Messages[i].Bufptr > 21 )
						{
                            ETX_flag = 1;       //buffer is full transmit message as is
                            Messages[i].Buffer[Messages[i].Bufptr] = 0x03;
                    		Messages[i].Buffer[Messages[i].Bufptr+1] = 0x00;
						}
					}
					if ( !strncmp( &CANBuf[1], "Icam:", 5 ) ||  !strncmp( &CANBuf[1], "Proc:", 5 ) 
					     ||  !strncmp( &CANBuf[1], "Warn:", 5 ))
						VCI_Flag2 |= ICAM_MSG;
                }
                if(ETX_flag)
                {
                    ETX_flag = 0;
                    MsgIDtable[i] = 0;
//                  sout(1, Messages[i].Buffer );            //send to ethernet
					DisplayMsg ( Messages[i].Buffer );
				    Messages[i].Bufptr=0;
				    Messages[i].Buffer[0]=0;
                }
			}
		}
		else			//<STX> is NOT part of message
		{
			if ( !gProcImg[OUT_digi_1] )        //Menu messages here
			{
    	        strcpy ( &DispBuf[DispBufptr],CANBuf );
                DispBufptr = DispBufptr + strlen ( CANBuf );
    			UpdateDisp = ETX_flag;
						
				if(ETX_flag)
				{
					VCI_Flag2 |= NEW_MENU;
				}	
			}
			else        //Other messages here
			{
			    i = 0;
				while ( MsgIDtable[i] != gProcImg[OUT_digi_1] && i++<3 );
    	        strcpy ( &Messages[i].Buffer[Messages[i].Bufptr],CANBuf );
    			Messages[i].Bufptr = Messages[i].Bufptr + strlen ( CANBuf );
                if ( Messages[i].Bufptr > 21 )
                {
					ETX_flag = 1;       //buffer is full transmit message as is
                    Messages[i].Buffer[Messages[i].Bufptr] = 0x03;
                    Messages[i].Buffer[Messages[i].Bufptr+1] = 0x00;
                }
                if(ETX_flag)
                {
                    ETX_flag = 0;
                    MsgIDtable[i] = 0;
//                  sout(1, Messages[i].Buffer );            //sent to ethernet
					DisplayMsg ( Messages[i].Buffer );
				    Messages[i].Bufptr=0;
				    Messages[i].Buffer[0]=0;
                }
				if ( !strncmp( &Messages[i].Buffer[6], "Stop BB", 7 ) )
                    VCI_Flag2 |= DISABLE_BB;        //dis-able button box
				if ( !strncmp( &Messages[i].Buffer[6], "Send BB", 7 ) )
                    VCI_Flag2 &= ~DISABLE_BB;       //enable button box
			}
		}
		gProcImg[OUT_digi_1+1] = 0;
	}

    COP_Trig();
}

