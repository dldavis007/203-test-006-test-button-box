/* Camera discovery. State and the shared CAN buffer stay in Subroutines.c. */
#include <stdio.h>
#include <string.h>
#include "CameraDiscovery.h"
#include "Subroutines.h"
#include "Controller.h"
#include "Interrupts.h"
#include "mcohw.h"

extern UNSIGNED8 gProcImg[PROCIMG_SIZE];

//request addresses from camera(s)
void ScanCameraAddresses(CAN_MSG *message)
{
    unsigned int k, i, j;
    char tempstr[21];

    //send transmit address command 0x02 to RPDO4
	message->LEN = 1;
	message->BUF[0] = 0x02;
	message->ID = 0x521;
 	if (!MCOHW_PushMessage(message))
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
void RandomizeCameraAddresses(CAN_MSG *message)
{
    //send generate random address command 0x01 to RPDO4
	message->LEN = 1;
	message->BUF[0] = 0x01;
	message->ID = 0x521;
 	if (!MCOHW_PushMessage(message))
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
