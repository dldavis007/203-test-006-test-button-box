/* Device discovery and menu population. Shared state stays in Subroutines.c. */
#include <string.h>
#include "DeviceDiscovery.h"
#include "Subroutines.h"
#include "Controller.h"
#include "Interrupts.h"
#include "mcohw.h"

/* Application access to the unchanged legacy MCO integration. */
extern int Device_Type_Response_ID[];
extern int Device_Type_Response_ID_ptr;
extern char Device_Menus[20][17];

void ActivateDeviceMenus(CAN_MSG *message)
{   //Activates Menus on all devices connected
    int i;
	for (i=0x201;i<0x280;i++)
	{
        message->ID = i;
        message->LEN = 1; 
        message->BUF[0] = 1;
        if (!MCOHW_PushMessage(message))
        {
            // failed to transmit
            MCOUSER_FatalError(0x8801);
        }
        //! Transmit this without using the TPDO
		Timer1 = RTI_One_Sec * .010;
		while (Timer1) { /* wait for interrupt */ }
	}
}

void ScanDevices(CAN_MSG *message)
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
				 
			     message->LEN = 2;
			 	 message->BUF[0] = 0x33;   //send message to randomize
				 message->BUF[1] = 0;
				 message->ID = 0;
				 resequenceNodes = true;
				 
				 if (!MCOHW_PushMessage(message))
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
				message->LEN = 3;
			 	message->BUF[0] = 0x33;  
				message->BUF[1] = (Device_Type_Response_ID[i] - 0x5E1) + 0x61;   //the node to receive the command
				message->BUF[2] = 0x10 + cntr;             //command to set new node ID 
				message->ID = 0;
				
				Device_Type_Response_ID[i] = 0x5E1 + cntr;
				cntr++;
				
				if(!MCOHW_PushMessage(message))
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
	        message->LEN = 4;
            message->BUF[0] = 0x34;  
            message->BUF[1] = 0;
            message->BUF[2] = (Device_Type_Response_ID[i] >> 8) & 0xff;
			message->BUF[3] = Device_Type_Response_ID[i] & 0xff;
            message->ID = 0;
            
            if(!MCOHW_PushMessage(message))
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
