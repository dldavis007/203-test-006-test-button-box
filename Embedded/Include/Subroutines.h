#ifndef Subroutines_H
#define Subroutines_H

#define Revision "3.05"
#define _SCI

extern volatile unsigned char AlternateFunctionMask;


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
void LoadMenu ( char Index[] );
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
void setDefaultMenu(void);

typedef enum { false = 0, true = !false } bool;

struct MatrixTbl
{
 	int BB_Data;
	char SW_Port_Data[2];
	char LED_Port_Data[3];
};

struct MenuStruct
{
char Index[4];
char Entry[12][21];           //menu text
char Pos[11];                 //position of variable
char *VarPntr[11];                  //pointer to variable
int (*FunctPtr[11])( void );        //pointer to function
};

extern struct MenuStruct Menuc[];   //holds menu data

struct MenuStack
{
char Index[4];
char CursorPos;
char FirstLine;
};

#define MenuSize 11
#define MenuStackSize 5

#define WIM_ID 0x310


//PLL OFF
//#define Tbus .543
//#define BusClk 1.8432

//PLL ON 22.118	 3.6864MHz Crystal
//PLL loop components, high stability = 3300pF, .033uf, 2K 
//#define REFDV_Init 0
//#define SYNR_Init 5
//#define Tbus .0452
//#define BusClk 22.118
//#define OscClk 3.6864

//PLL ON 24.576	 3.6864MHz Crystal
//PLL loop components, high stability = .01uf, .1uf, 2K 
//#define REFDV_Init 2
//#define SYNR_Init 19
//#define Tbus .0407
//#define BusClk 24.576
//#define OscClk 3.6864

//PLL ON 24.000	 4.0MHz Crystal
//PLL loop components, high stability = .01uf, .1uf, 2K 
//#define REFDV_Init 2
//#define SYNR_Init 17
//#define Tbus .0417
//#define BusClk 24.000
//#define OscClk 4.0

//PLL ON 24.000	 8.0MHz Crystal
//PLL loop components, high stability = .01uf, .1uf, 2K 
#define REFDV_Init 5
#define SYNR_Init 17
#define Tbus .0417
#define BusClk 24.000
#define OscClk 8.0


#define DDRA_Init 0xFF
#define DDRB_Init 0xB8
#define DDRE_Init 0xFC
#define DDRJ_Init 0xC0
#define DDRM_Init 0x08
#define DDRP_Init 0x1F
#define DDRS_Init 0x0F
#define DDRT_Init 0x8F


#define PORTA_Init 0x00
#define PORTB_Init 0x00
#define PORTE_Init 0x00
#define PTJ_Init 0xC0
#define PTM_Init 0x00
#define PTP_Init 0x11
#define PTS_Init 0x00
#define PTT_Init 0x8E


#define ATD0DIEN_Init 0x00
#define ATD0CTL2_Init 0x80
#define ATD0CTL3_Init 0x23
#define ATD0CTL4_Init 0x45
#define ATD0CTL5_Init 0x80


#define PUCR_Init 0x93
#define PPSJ_Init 0x00
#define PPSP_Init 0x00
#define PIEP_Init 0x00

#define PERM_Init 0x00
#define PPSM_Init 0x00

//#define PERS_Init 0x00
#define PERS_Init 0xff

//#define WOMS_Init 0x08
#define WOMS_Init 0x00




#define CRGINT_Init 0x80
#define RTICTL_Init 0x40

#define MODRR_Init 0x10

#define ReActivate_Time RTI_One_Sec * 0.5


//SPI Control Register 1
#define SPI0CR1_Init 0xd0
//SPI Control Register 2
#define SPI0CR2_Init 0x00
//SPI Baud Rate Register (24MHz / 2 = 12MHz)                               
#define SPI0BR_Init 0x00                  


#define SPISR_SPTEF 0x20
#define SPICR1_SPTIE 0x20
#define SPISR_SPIF 0X80

#define UpdateMainTime 250

#define PWME_Init 0xA0
#define PWMPOL_Init 0xff
#define PWMCLK_Init 0xff
#define PWMPRCLK_Init 0x11

#define PWMPER0_Init 100
#define PWMPER1_Init 100
#define PWMPER2_Init 100
#define PWMPER3_Init 100
#define PWMPER4_Init 100
#define PWMPER5_Init 100
#define PWMPER6_Init 100
#define PWMPER7_Init 100

#define PWMDTY0_Init 0
#define PWMDTY1_Init 0
#define PWMDTY2_Init 0
#define PWMDTY3_Init 0
#define PWMDTY4_Init 0
#define PWMDTY5_Init 40
#define PWMDTY6_Init 0
#define PWMDTY7_Init 0

#define PWMSCLA_Init 6
#define PWMSCLB_Init 6

#define DispBufLen  481

//VCI_Flag2
#define ICAM_MSG        0x01        //set when message is inspection camera position
#define NEW_MENU        0x02        //set menu is received
#define DISABLE_BB      0x04        //used to stop button box data to reduce interrupts on WIM, WCM when setting channels

#define Disp_Menu_Port 	PORTA
#define Disp_Menu_Bit	0x80			
#define Disp_Up_Port 	PORTA
#define Disp_Up_Bit		0x40
#define Disp_Down_Port 	PORTA
#define Disp_Down_Bit	0x20

#define Alarm_Port		PORTA
#define Alarm_Bit		0x10

//PORT-E define
#define HW_COP         0x40

#endif
