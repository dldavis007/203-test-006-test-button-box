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
extern int Device_Type_Response_ID[];
extern int Device_Type_Response_ID_ptr;
extern char Device_Menus[20][17];
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

void ext_menu ( void )
{   //Activates Menus on all devices connected
    int i;
	for (i=0x201;i<0x280;i++)
	{
        gTxMsg.ID = i;
        gTxMsg.LEN = 1; 
        gTxMsg.BUF[0] = 1;
        if (!MCOHW_PushMessage(&gTxMsg))
        {
            // failed to transmit
            MCOUSER_FatalError(0x8801);
        }
        //! Transmit this without using the TPDO
		Timer1 = RTI_One_Sec * .010;
		while (Timer1) { /* wait for interrupt */ }
	}
}

void Scan ( void )
{
	int i,j,k, cntr;
	
	Timer1 = RTI_One_Sec * 2.25;
	while ( Timer1 )
	{
	    COP_Trig();
	}
		
	for (j=0x610;j<=0x67f;j++)
	{
	    Timer1 = RTI_One_Sec * .015;      //necessary to allow menu to populate
	    while (Timer1) { /* wait for interrupt */ }
        SDO_Init_Upload (j,0x2100,0);
	    MCO_ProcessStack();
	    COP_Trig();
	}
	for (j=1;j<40;j++)
	{
	    Timer1 = RTI_One_Sec * .015;      //necessary to allow menu to populate
	    while (Timer1) { /* wait for interrupt */ }
	    MCO_ProcessStack();
	    COP_Trig();
	}

	i = 0;
    while (i < 20)
    {
	    if (Device_Type_Response_ID[i] >= 0x5E1 && Device_Type_Response_ID[i] <= 0x5E9) 
		{ 
		     if (i + 1 < 20 && Device_Type_Response_ID[i] == Device_Type_Response_ID[i+1])  //if duplicate detected
			 {
			     Device_Type_Response_ID[i] = 0;
				 Device_Type_Response_ID[i+1] = 0;
				 Device_Type_Response_ID_ptr = i;
				 
			     gTxMsg.LEN = 2;
			 	 gTxMsg.BUF[0] = 0x33;   //send message to randomize
				 gTxMsg.BUF[1] = 0;
				 gTxMsg.ID = 0;
				 resequenceNodes = true;
				 
				 if (!MCOHW_PushMessage(&gTxMsg))
                 {
          		  	MCOUSER_FatalError(0x8801);
      		     } 
				 
				 Timer1 = RTI_One_Sec * 0.25;
        	     while (Timer1) { /* wait for interrupt */ }   //wait here to receive bootup message after CNT-031's reset

				 for (j = 0x661; j < 0x66A; j++)
				 {
				     Timer1 = RTI_One_Sec * .015;       
	    			 while (Timer1) { /* wait for interrupt */ }
        			 SDO_Init_Upload (j,0x2100,0);
	    			 MCO_ProcessStack();
				 }
				 
                 Timer1 = RTI_One_Sec * .015;       
	    	     while (Timer1) { /* wait for interrupt */ }
			     MCO_ProcessStack();

				 i -= 1;
			 }	
		
	    }
	 	else if ( i + 1 < 20 && Device_Type_Response_ID[i] != 0 && Device_Type_Response_ID[i] == Device_Type_Response_ID[i+1])
		{
		    for(j=i; j<19; j++) {                   //move Type Responses up (remove duplicates, they should be consecutive)
			    Device_Type_Response_ID[j] = Device_Type_Response_ID[j+1];
			}
		}
 	    i++;
    }

	i = 0;
	cntr = 0;
	
    while (i < 20)
    {
	    COP_Trig();
		
	    if (Device_Type_Response_ID[i] != 0)
		{
		    if (resequenceNodes == true && Device_Type_Response_ID[i] >= 0x5E1 && Device_Type_Response_ID[i] <= 0x5E9)
			{
				gTxMsg.LEN = 3;
			 	gTxMsg.BUF[0] = 0x33;  
				gTxMsg.BUF[1] = (Device_Type_Response_ID[i] - 0x5E1) + 0x61;   //the node to receive the command
				gTxMsg.BUF[2] = 0x10 + cntr;             //command to set new node ID 
				gTxMsg.ID = 0;
				
				Device_Type_Response_ID[i] = 0x5E1 + cntr;
				cntr++;
				
				if(!MCOHW_PushMessage(&gTxMsg))
          		    MCOUSER_FatalError(0x8801);
				 
				Timer1 = RTI_One_Sec *.025;
        	    while (Timer1) { /* wait for interrupt */ }
			}
			
			MCO_ProcessStack();
			
	        for (j=1; j<=4; j++)
			{
                SDO_Init_Upload((0x600 + (Device_Type_Response_ID[i] - 0x580)), 0x2100, j);   //SDO Request to 0x600 + Node_ID to sub-indexes 1 - 4
                MCO_ProcessStack();
        	    Timer1 = RTI_One_Sec * .02;
        	    while (Timer1) { /* wait for interrupt */ }
			}
		}
		i++;
	}	

	for (j=1;j<40;j++)
	{
	    COP_Trig();
	    Timer1 = RTI_One_Sec * .05;
	    while (Timer1) { /* wait for interrupt */ }
	    MCO_ProcessStack();
	}
	
	//send Device_Type_Reponse_ID data to all CNT-031 nodes for recording in their array
	for (i = 0; i < 20; i++)
    {
		if (Device_Type_Response_ID[i] >= 0x5E1 && Device_Type_Response_ID[i] <= 0x5EA)
		{
	        gTxMsg.LEN = 4;
            gTxMsg.BUF[0] = 0x34;  
            gTxMsg.BUF[1] = 0;
            gTxMsg.BUF[2] = (Device_Type_Response_ID[i] >> 8) & 0xff;
			gTxMsg.BUF[3] = Device_Type_Response_ID[i] & 0xff;
            gTxMsg.ID = 0;
            
            if(!MCOHW_PushMessage(&gTxMsg))
                MCOUSER_FatalError(0x8801);
				
			Timer1 = RTI_One_Sec * .015;       
	    	while (Timer1) { /* wait for interrupt */ }	
			MCO_ProcessStack();
        }	
	}
		
	i = 0;
	j = 0;
    k = 0;
	
    while ( i < 20 )
    {
	    COP_Trig();
	    if ( Device_Type_Response_ID[i] != 0 ) 
		{
            //main menu items
            if( (unsigned char)Device_Menus[i][15] != 0xff && Device_Menus[i][15] != 0x00 )    //0xff indicates auxiliary menus, WIM = 0xA6                   
		    {  						   		   					   		  	    //0x32 indicates main menu items
    	    	Device_Menus[i][15] = 0x32;
				strcpy(Menuc[0].Entry[j+8], Menuc[0].Entry[j+7]);		
    			Menuc[0].FunctPtr[j+6] = &ExternalMenu;
    	    	strcpy(&Menuc[0].Entry[j+7][1], &Device_Menus[i][0]);
    	    	strcpy(&Menuc[0].Entry[j+7][16], "    ");
    			j++;
    		}
      	    //auxiliary menu items   
      	    if( (unsigned char)Device_Menus[i][15] == 0xff )                                   //0xff indicates auxiliary menus
            {
			    if (k < 10)  //supports max of 10 aux devices
				{
        	    	strcpy(Menuc[5].Entry[k+2],Menuc[5].Entry[k+1]);		
        			Menuc[5].FunctPtr[k+0] = &ExternalSubMenu;
        	    	strcpy(&Menuc[5].Entry[k+1][1],&Device_Menus[i][0]);
        	    	strcpy(&Menuc[5].Entry[k+1][16],"    ");
        			k++;
					
					if (k == 10)
					    AuxDeviceMenuFull = true;
				}
				else
				{
				    AuxDeviceLimitExceeded = true;
				}
    		}
        }
        i++;
	}
	
    menuSort();     //seperate sub-menu items from main menu items
	SDO_restore();
}

void menuSort( void )       //seperate sub-menu items from primary menu items
{
    int index = 0;          //index for main menu array
    int i;                  //index used to advance main menu items
    int subIndex = 0;       //index for sub menu array

    for(index = 0; index < 19; index++) {
	    COP_Trig();
        if( (unsigned char)Device_Menus[index][15] == 0xff ) {    //0xff indicates sub menu of Auxiliary Equip
            sub_Device_Type_Response_ID[subIndex] = Device_Type_Response_ID[index];
            strcpy( &deviceSubMenu[subIndex++][0], &Device_Menus[index][0]);
            
            for(i=index; i<19; i++) {                   //move main menu items up
                Device_Type_Response_ID[i] = Device_Type_Response_ID[i+1];
                strcpy( &Device_Menus[i][0], &Device_Menus[i+1][0]);
                Device_Menus[i][15] = Device_Menus[i+1][15];   //copy sub-menu marker
            }
            --index;        //do NOT skip menu item that was moved up
        }
    }
}

//request addresses from camera(s)
void scan_cameras(void)
{
    unsigned int k, i, j;
    char tempstr[21];

    //send transmit address command 0x02 to RPDO4
	gTxMsg.LEN = 1;
	gTxMsg.BUF[0] = 0x02;
	gTxMsg.ID = 0x521;
 	if (!MCOHW_PushMessage(&gTxMsg))
    {
      // failed to transmit
      MCOUSER_FatalError(0x8801);
    }
    MCO_ProcessStack();

    //receive camera addresses from 
    i = 0; 
	j = 0;
	k = 0;
	Timer2 = RTI_One_Sec * 2.2;             //intialize timer
    do 
	{
	    COP_Trig();
        MCO_ProcessStack();
        if(gProcImg[OUT_digi_9] || gProcImg[OUT_digi_9+1])  //when address is received
		{         
            cam_list[i] = gProcImg[OUT_digi_9] + (gProcImg[OUT_digi_9+1]<<8);     //save camera address
            gProcImg[OUT_digi_9] = 0;       //reset to allow for new value
            gProcImg[OUT_digi_9+1] = 0;     //reset to allow for new value
            ++i;
        } 
		else if (gProcImg[OUT_digi_13] == 0x18)  //when address received from HD Cam
		{
		    if (!vid18_addr)
			{
		       cam_list[i] = gProcImg[OUT_digi_13 + 1] + (gProcImg[OUT_digi_13 + 2] << 8);     //save camera address
			   vid18_addr = true;
			   
               extCamTagList[i][0] = gProcImg[OUT_digi_13 + 3]; 
			   extCamTagList[i][1] = gProcImg[OUT_digi_13 + 4]; 
			   extCamTagList[i][2] = gProcImg[OUT_digi_13 + 5]; 
			   extCamTagList[i][3] = gProcImg[OUT_digi_13 + 6]; 
			   extCamTagList[i][4] = gProcImg[OUT_digi_13 + 7]; 
			}
			else
			{
			   extCamTagList[i][5] = gProcImg[OUT_digi_13 + 1];
			   extCamTagList[i][6] = gProcImg[OUT_digi_13 + 2];
			   extCamTagList[i][7] = gProcImg[OUT_digi_13 + 3];
			   extCamTagList[i][8] = gProcImg[OUT_digi_13 + 4];
			   extCamTagList[i][9] = gProcImg[OUT_digi_13 + 5];
			   extCamTagList[i][10] = gProcImg[OUT_digi_13 + 6];
			   
			   while(extCamTagList[i][j] == 0x20){
			       j++;
			   }  
    		   strncpy(extCamTagList[i], &extCamTagList[i][j], 11-j); 
			   memset(&extCamTagList[i][11-j], 0x20, j);
			   
			   i++;
			   vid18_addr = false;
			}
			gProcImg[OUT_digi_13] = 0;       //reset to allow for new value
			j = 0; 
		}
		   
    } while ( Timer2 && i < 10);       //allow time for all cameras to report

    cam_select = 0;    //select first camera found

    for(k = i; k < 9; k++) {
        cam_list[k] = 0;    //set unused camera array elements to zero
    }

    //populate camera menu
	i = 0;      //index of camera found
	j = 0;      //counts menu items added
    while ( i < 10 )
    {
        if ( (cam_list[i] != 0) )
	    {   //Find Cameras is at j = 1,
			strcpy (Menuc[3].Entry[j+3],Menuc[3].Entry[j+2]);   //move "Exit" down
			if (extCamTagList[i][0] == 0)
			    sprintf(tempstr,"CAMERA %d           ", i+1);
			else
			{
                memset(tempstr, 0x20, sizeof(tempstr));
				for (k=0; k<11; k++)
				{
				    tempstr[k] = extCamTagList[i][k];
				}
			}	
			strncpy ( &Menuc[3].Entry[j+2][1], tempstr, 20);          
            Menuc[3].FunctPtr[j+1] = &CameraMenu;               //Add function pointer
            j++;
		}
		i++;
	}	
    
    //initialize gProcImg settings for camera selection
    gProcImg[IN_digi_3] = cam_list[cam_select];    //lsb 
    gProcImg[IN_digi_3 + 1] = cam_list[cam_select] >> 8;       //msb 
	
	memset(extCamTagList, 0, 10*11*sizeof(char));
	
	UpdateMenu = 1;
	showMessage("Proc:                         ", 1); //clear top line
}

//command camera boards to generate new addresses
void addressCamera(void)
{
    //send generate random address command 0x01 to RPDO4
	gTxMsg.LEN = 1;
	gTxMsg.BUF[0] = 0x01;
	gTxMsg.ID = 0x521;
 	if (!MCOHW_PushMessage(&gTxMsg))
    {
      // failed to transmit
      MCOUSER_FatalError(0x8801);
    }
    MCO_ProcessStack();
	Timer2 = RTI_One_Sec * 0.25;         //intialize timer
    while (Timer2) { /* wait for interrupt */ }       //allow time for to create addresses 
}

int findCameras(void)
{
    int i;

	if (!vid18_update)
        addressCamera();
		
    //restore menu without external cameras prior to scanning
    for(i = 1; i < 10; ++i)
    {
        Menuc[3].FunctPtr[i] = &ExitMenu;
        if(i > 1 )
            strcpy (Menuc[3].Entry[i],"                    ");   //move Exit down
    }
   	strcpy (Menuc[3].Entry[2]," EXIT               ");   //move Exit down

    scan_cameras();

    //put camera address on ProcImg to select camera
    cam_select = 0;
    gProcImg[IN_digi_3] = cam_list[cam_select];    //lsb 
    gProcImg[IN_digi_3 + 1] = cam_list[cam_select]>>8;       //msb 
    
	vid18_update = false;
	
    if(cam_list[0] > 0)
	    return 1;    //indicates cameras present
		
    return 0;        //indicates cameras not found
}

//used to check for duplicate camera addresses
int duplicate_add(void)
{
    int i, j;
    
    for(i = 0; i<10; i++)
    {
        for(j = 0; j<10; j++)
        {
            if( i != j && cam_list[i] && cam_list[j] ) 
            {
                if( cam_list[i] == cam_list[j] )   return 1;   //duplicate found
            }
        }
    }
    return 0;   //no duplicate addresses
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
		unsigned int SwitchData; 
		static unsigned SwitchUpData;
		unsigned char Sw_Index;
		char StopFlag=0;
		
		DeBounceTimer = DeBounceTime;
		
		SwitchData = ButtonBoxData;
		
		//Keep all latched data
		SwitchData &= ( TeleData_F1 | TeleData_F2 | TeleData_F3 | TeleData_Rev | TeleData_Fwd | TeleData_ExSlow | ((EnabledFunctionData(SwitchData) & (TeleData_F1 | TeleData_F2))?0:TeleData_Slow) | TeleData_Fast );

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
				if ( Keypad_Matrix[Sw_Index].BB_Data & ( TeleData_F1 | TeleData_F2 | TeleData_F3 | TeleData_Rev | TeleData_Fwd | TeleData_ExSlow | ((EnabledFunctionData(SwitchData) & (TeleData_F1 | TeleData_F2))?0:TeleData_Slow) | TeleData_Fast ) )
				{   //Switch is a toggle function 
					if (SwitchUpData & Keypad_Matrix[Sw_Index].BB_Data) 
					{	//Switch is pressed and was previously released
						SwitchData = SwitchData ^ Keypad_Matrix[Sw_Index].BB_Data;      //exclusive OR to toggle bit
						SwitchUpData = SwitchUpData & ~Keypad_Matrix[Sw_Index].BB_Data;	//Clear previously released bit				
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
				SwitchUpData  = SwitchUpData | Keypad_Matrix[Sw_Index].BB_Data;
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
		    SwitchData &= ~(TeleData_ExSlow | ((EnabledFunctionData(SwitchData) & (TeleData_F1 | TeleData_F2))?0:TeleData_Slow) | TeleData_Fast);
		
		if ( SwitchData & TeleData_Fwd && SwitchUpData & TeleData_Rev ) //FORWARD
		    SwitchData &= ~TeleData_Rev;
		
		if ( SwitchData & TeleData_Rev && SwitchUpData & TeleData_Fwd ) //REVERSE
		    SwitchData &= ~TeleData_Fwd;

		if ( SwitchData & TeleData_F1 && SwitchUpData & TeleData_F2 && SwitchUpData & TeleData_F3 ) //F 1
			SwitchData &= ~(TeleData_F2 | TeleData_F3);
		if ( SwitchData & TeleData_F2 && SwitchUpData & TeleData_F1 && SwitchUpData & TeleData_F3 ) //F 2
			SwitchData &= ~(TeleData_F1 | TeleData_F3);
		if ( SwitchData & TeleData_F3 && SwitchUpData & TeleData_F1 && SwitchUpData & TeleData_F2 ) //F 3
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

