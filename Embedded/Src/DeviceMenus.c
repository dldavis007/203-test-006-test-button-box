/* Commands for menus on connected cameras/devices. State stays in Subroutines.c. */
#include "DeviceMenus.h"
#include "Subroutines.h"
#include "Menu.h"
#include "Display.h"
#include "Interrupts.h"
#include "mcohw.h"
#include "mco.h"
#include "LegacyCanInterface.h"

int16_t ExternalMenu ( void )
{
    int16_t i;
	CAN_MSG gTxMsg;

	i = MenuStackc[StackPointer].CursorPos + MenuStackc[StackPointer].FirstLine - 7;

	//activate external menu
	gTxMsg.LEN = 1;
	gTxMsg.BUF[0] = 1;
	gTxMsg.BUF[1] = 0;
	gTxMsg.BUF[2] = 0;
	gTxMsg.BUF[3] = 0;
	gTxMsg.ID = 0x200 + (Device_Type_Response_ID[i] - 0x580);
 	if (!MCOHW_PushMessage(&gTxMsg))
    {
      // failed to transmit
      MCOUSER_FatalError(0x8801);
    }

	Gen_Flags &= ~Gen_Flags_Menu_Active;        //clear active menu flag
	Gen_Flags |= Gen_Flags_Extern_Menu;         //set external menu flag

    ClearTitler ();
	ReActivate_Timer = ReActivate_Time;

	//DeSelect ();
	return 0;
}

int16_t ExternalSubMenu ( void )
{
    int16_t i;
	CAN_MSG gTxMsg;

	i = MenuStackc[StackPointer].CursorPos + MenuStackc[StackPointer].FirstLine - 1;
	
	//activate external menu
	gTxMsg.LEN = 1;
	gTxMsg.BUF[0] = 1;
	gTxMsg.BUF[1] = 0;
	gTxMsg.BUF[2] = 0;
	gTxMsg.BUF[3] = 0;
	gTxMsg.ID = 0x200 + (sub_Device_Type_Response_ID[i] - 0x580);
 	if (!MCOHW_PushMessage(&gTxMsg))
    {
      // failed to transmit
      MCOUSER_FatalError(0x8801);
    }

	Gen_Flags &= ~Gen_Flags_Menu_Active;        //clear active menu flag
	Gen_Flags |= Gen_Flags_Extern_Menu;         //set external menu flag

    ClearTitler ();
	ReActivate_Timer = ReActivate_Time;

	return 0;
}

int16_t CameraMenu ( void )
{
    CAN_MSG gTxMsg;

	//menu is offset from cam_list
    cam_select = MenuStackc[StackPointer].CursorPos + MenuStackc[StackPointer].FirstLine - 2;

    //select camera
    gProcImg[IN_digi_3] = cam_list[cam_select];    //lsb 
    gProcImg[IN_digi_3 + 1] = cam_list[cam_select] >> 8;       //msb 

    //activate external menu
	gTxMsg.LEN = 3;
	gTxMsg.BUF[0] = 0x08;
	gTxMsg.BUF[1] = cam_list[cam_select];
	gTxMsg.BUF[2] = cam_list[cam_select] >> 8;
	gTxMsg.ID = 0x500 + 0x21;
 	if (!MCOHW_PushMessage(&gTxMsg))
    {
      // failed to transmit
      MCOUSER_FatalError(0x8801);
    }

	Gen_Flags &= ~Gen_Flags_Menu_Active;    //turn off main menu
	Gen_Flags |= Gen_Flags_Extern_Menu;     //activate external menu
	ClearTitler();
	ReActivate_Timer = ReActivate_Time;
	
	return 0;
}

int16_t Trigger_Device ( void )
{
    int16_t i;
	CAN_MSG gTxMsg;

    if ( MenuStackc[StackPointer].CursorPos+MenuStackc[StackPointer].FirstLine >=1 )
    {
        i = MenuStackc[StackPointer].CursorPos + MenuStackc[StackPointer].FirstLine - 7;
   
        gTxMsg.LEN = 1;
        gTxMsg.BUF[0] = 2;
        gTxMsg.BUF[1] = 0;
        gTxMsg.BUF[2] = 0;
        gTxMsg.BUF[3] = 0;
        gTxMsg.ID = 0x200 + (Device_Type_Response_ID[i] - 0x580);
        if (!MCOHW_PushMessage(&gTxMsg))
        {
            // failed to transmit
            MCOUSER_FatalError(0x8801);
        }

        DeSelect ();
	}
	
	return 0;
}
