#ifndef Subroutines_H
#define Subroutines_H

#include "ApplicationState.h"
#include "HardwareConfig.h"
#include "Settings.h"
#include "Display.h"
#include "Menu.h"
#include "DeviceMenus.h"
#include "ButtonInput.h"
#include "CameraDiscovery.h"
#include "DeviceDiscovery.h"

#define Revision "3.05"
#define _SCI

/* Application loop. */
void doevents(void);

/* Analog input. */
int ATDGetLevel(char ATD_Num);

/* Application message assembly and menu rendering. */
void bldMessages(void);

/* Application sizes, timing and message flags. */

#define WIM_ID 0x310

#define ReActivate_Time RTI_One_Sec * 0.5
#define UpdateMainTime 250
#define DispBufLen  481

//VCI_Flag2
#define ICAM_MSG        0x01        //set when message is inspection camera position
#define NEW_MENU        0x02        //set menu is received
#define DISABLE_BB      0x04        //used to stop button box data to reduce interrupts on WIM, WCM when setting channels

#endif
