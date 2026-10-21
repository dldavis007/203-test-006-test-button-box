#ifndef Subroutines_H
#define Subroutines_H

#include "ApplicationState.h"
#include "HardwareConfig.h"
#include "Settings.h"
#include "Display.h"

#define Revision "3.05"
#define _SCI

/* Application loop. */
void doevents(void);

/* Saved settings and settings-menu actions. */
int setSerial(void);
int RestoreDefaults(void);
int ToggleF1Enable(void);
int ToggleF2Enable(void);
int Var1to13Funct(void);
int Var1to30Funct(void);
int VarSerNumFunct(void);

/* Internal menu navigation and variable editing. */
void setDefaultMenu(void);
void LoadMenu(unsigned char Index[]);
int ExitMenu(void);
void CursorUp(void);
void CursorDown(void);
void Select(void);
void DeSelect(void);
void InsertCursor(void);
void IncVariable(void);
void DecVariable(void);
void dispVariable(char* vPtr, double *value, char format);
int NullFunction(void);

/* Device discovery, camera selection and external menus. */
void Scan(void);
void menuSort(void);
void scan_cameras(void);
void addressCamera(void);
int findCameras(void);
int duplicate_add(void);
int ExternalMenu(void);
int ExternalSubMenu(void);
int CameraMenu(void);
int Trigger_Device(void);
void ext_menu(void);

/* Battery diagnostics and analog input. */
void Scan_Batteries(void);
int ClrBattErr(void);
int NextBatt(void);
int ATDGetLevel(char ATD_Num);

/* Application message assembly and menu rendering. */
void bldMessages(void);
void btnDisplayTitler(void);

/* Application sizes, timing and message flags. */
#define MenuSize 11
#define MenuStackSize 5

#define WIM_ID 0x310

#define ReActivate_Time RTI_One_Sec * 0.5
#define UpdateMainTime 250
#define DispBufLen  481

//VCI_Flag2
#define ICAM_MSG        0x01        //set when message is inspection camera position
#define NEW_MENU        0x02        //set menu is received
#define DISABLE_BB      0x04        //used to stop button box data to reduce interrupts on WIM, WCM when setting channels

#endif
