#ifndef Subroutines_H
#define Subroutines_H

#include "ApplicationState.h"
#include "HardwareConfig.h"

#define Revision "3.05"
#define _SCI


void Scan_Batteries ( void );
int NullFunction ( void );
int ClrBattErr ( void );
int NextBatt ( void );
int VarSerNumFunct ( void );
void Load_Variables (void);
void Save_Variables ( void );
void Load_Serial_Num ( void );
void Save_Serial_Num ( void );
int setSerial ( void );
int ToggleF1Enable(void);
int ToggleF2Enable(void);
int Var1to13Funct ( void );
int Var1to30Funct ( void );
int ExitMenu ( void );
void CursorUp( void );
void CursorDown( void );
void IncVariable ( void );
void DecVariable ( void );
void dispVariable(char* vPtr, double *value, char format);
void Select ( void );
void DeSelect ( void );
void LoadMenu ( unsigned char Index[] );
void InsertCursor ( void );
void btnDisplayTitler ( void );
int RestoreDefaults ( void );
int ATDGetLevel ( char ATD_Num );
void ext_menu ( void );
void Scan ( void );
void menuSort( void );
void scan_cameras(void);
void addressCamera(void);
int findCameras(void);
int duplicate_add(void);
int ExternalMenu ( void );
int ExternalSubMenu ( void );
int CameraMenu ( void );
int Trigger_Device ( void );
void doevents ( void );
void Xmit_Rcv_SPI ( void );
void resetTitler(void);
void showMenu ( void );
char DispTranslate ( char *TranChar );
void bldMessages(void);
void DisplayMsg ( char Message[] );
void showMessage ( char Message[], int row );
void ClearTitler ( void );
void DisplayTitler ( void );
void WriteCharMem(void);
void ReadCharMem(void);
void setDefaultMenu(void);

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
