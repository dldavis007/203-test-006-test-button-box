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
#include "string.h"






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

volatile char SPI0InBuf[2];
volatile int SPI0InBufptr = 0;
volatile char SPI0OutBuf[2];
volatile int SPI0OutBufptr = 0;

int dispPBJTFlag = 0;

char DispBuf[DispBufLen];          //used to display keyboard input
int DispBufptr = 0;                //index for DispBuf[481]

volatile int ReActivate_Timer;

char power_on = 1;
char Ext_Menu_xmitd;



bool resequenceNodes = false;
volatile bool controller_mode = false;
volatile bool legacy_mode = false;
bool vid18_addr = false;
bool vid18_update = false;
bool F3pressed = false;

char deviceSubMenu[20][17];
int sub_Device_Type_Response_ID[20];

bool AuxDeviceLimitExceeded = false;
bool AuxDeviceMenuFull = false;


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

char MenuCheck;
char internal_menu;



signed char CursorDownFlag;
signed char CursorUpFlag;
signed char SelectFlag;
char Trigger_DeviceFlag;

int cam_list[10];   //each elements holds a 2 byte camera address
char extCamTagList[10][11];
int cam_select;    //index of cam_list to select camera

bool firstWriteSPI = true;

bool CharsInMem = false;
bool goLow = false;
char plusSymbol[54];
const char CMDI[] = {
85,85,85,
85,85,85,
85,85,85,
85,85,85,
65,85,85,
40,85,85,
42,21,85,
74,133,85,
82,161,85,
84,168,85,
85,42,21,
85,74,133,
85,82,161,
85,84,168,
85,85,40,
85,85,65,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,64,1,
85,74,161,
85,74,161,
85,74,1,
85,74,21,
85,74,21,
85,74,21,
85,74,21,
85,74,21,
85,74,21,
85,74,21,
85,74,1,
85,74,161,
85,74,161,
85,64,1,
85,85,85,
85,85,85,
85,85,85,
64,1,85,
74,161,85,
74,161,85,
64,161,85,
84,161,85,
84,161,85,
84,161,85,
84,161,85,
84,161,85,
84,161,85,
84,161,85,
64,161,85,
74,161,85,
74,161,85,
64,1,85,
85,85,85,
85,85,85,
85,0,85,
84,170,21,
82,170,133,
82,130,133,
82,130,133,
82,170,133,
84,170,21,
84,168,85,
82,170,1,
74,138,40,
74,18,136,
40,82,129,
42,8,161,
42,170,40,
74,170,40,
80,0,65,
85,85,85,
85,85,85,
85,69,85,
85,33,85,
84,168,85,
82,170,21,
74,138,133,
42,18,161,
40,84,161,
65,85,5,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,65,85,
85,40,85,
85,40,85,
80,40,5,
74,170,161,
74,170,161,
80,40,5,
85,40,85,
85,40,85,
85,65,85,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
84,0,5,
82,170,161,
82,170,161,
84,0,5,
85,85,85,
85,85,85,
84,0,5,
82,170,161,
82,170,161,
84,0,5,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
64,0,1,
42,170,168,
42,170,168,
64,0,1,
85,85,85,
85,85,85,
85,85,85,
85,84,21,
85,82,133,
85,82,21,
85,82,21,
85,82,21,
85,82,21,
85,82,21,
85,72,85,
85,82,21,
85,82,21,
85,82,21,
85,82,21,
85,82,21,
85,82,133,
85,84,21,
85,85,85,
85,85,85,
85,85,85,
84,21,85,
82,133,85,
84,133,85,
84,133,85,
84,133,85,
84,133,85,
84,133,85,
85,33,85,
84,133,85,
84,133,85,
84,133,85,
84,133,85,
84,133,85,
82,133,85,
84,21,85,
85,85,85,
85,85,85,
85,69,85,
84,32,85,
82,34,21,
80,168,21,
74,170,133,
80,168,21,
82,34,21,
84,32,85,
85,69,85,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,85,85,
85,5,85,
84,161,85,
84,161,85,
84,161,85,
84,161,85,
84,161,85,
84,161,85,
84,161,85,
84,161,85,
84,161,85,
85,5,85,
85,85,85,
85,5,85,
84,161,85,
84,161,85,
85,5,85,
85,85,85,
85,85,85,
64,85,85,
42,21,85,
34,21,65,
42,21,40,
64,84,168,
85,82,161,
85,74,133,
85,42,21,
84,168,85,
82,161,85,
74,133,85,
42,21,1,
40,84,168,
65,84,136,
85,84,168,
85,85,1,
85,65,85,
84,40,5,
82,170,161,
74,170,168,
42,40,40,
40,40,65,
40,40,85,
42,40,21,
74,170,133,
82,170,161,
84,40,168,
85,40,40,
65,40,40,
40,40,168,
42,170,161,
74,170,133,
80,40,21,
85,65,85,
85,85,85,
85,85,85,
85,21,69,
84,133,33,
84,133,33,
64,128,33,
42,170,168,
66,85,129,
82,85,133,
82,85,133,
73,86,1,
42,170,168,
72,2,1,
72,82,21,
33,72,85,
33,72,85,
33,72,85,
69,81,85,
85,65,85,
85,40,85,
85,40,85,
85,40,85,
85,40,85,
85,40,85,
85,40,85,
85,40,85,
85,40,85,
85,40,85,
85,40,85,
85,40,85,
85,40,85,
85,40,85,
85,40,85,
85,40,85,
85,40,85,
85,65,85,

85,85,85,
85,65,85,
85,33,85,
85,40,85,
85,40,85,
84,138,21,
85,42,21,
85,34,21,
69,10,17,
33,42,8,
69,40,81,
85,40,85,
84,161,85,
84,5,85,
65,84,21,
40,82,133,
65,84,21,
85,85,85,

85,81,85,
85,72,85,
85,72,85,
85,42,21,
85,42,21,
85,136,149,
69,42,17,
33,8,8,
69,34,17,
85,42,21,
85,42,21,
85,42,21,
85,72,85,
85,72,85,
80,82,5,
74,20,161,
80,85,5,
85,85,85,

85,85,85,
85,65,85,
85,72,85,
85,40,85,
85,40,85,
84,162,21,
84,168,85,
84,136,85,
68,160,81,
32,168,72,
69,40,81,
85,40,85,
85,74,21,
85,80,21,
84,21,65,
82,133,40,
84,21,65,
85,85,85
};

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
char BattSerNum[] 	  = "  -  -   ";
char BattSOC[] 		  = "---";
char BattSOH[] 		  = "---";
char BattErr[] 		  = "   OK";
char BatteryTemps[]   = "B:---,C1:---,C2:---";


const struct MenuStruct defaultMenuc[MenuSize] = {
    {
        {0, 0, 0, 0},
        {"     BUTTON BOX     ", " SETTINGS           ", " DIAGNOSTICS        ", " CAMERAS            ", " STATUS             ", " AUXILIARY EQUIP    ", " DEFAULTS           ", " EXIT               ", "                    ", "                    ", "                    ", "                    "},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar},
        {&NullFunction, &NullFunction, &NullFunction, &NullFunction, &NullFunction, &ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu}
    },
    {
        {0, 0, 0, 1},
        {"    SETTINGS MENU   ", " MESSAGE ROW        ", " MESSAGE COL        ", " F1                 ", " F2                 ", " EXIT               ", "                    ", "                    ", "                    ", "                    ", "                    ", "                    "},
        {15, 15, 15, 15, 0, 0, 0, 0, 0, 0, 0},
        {MessageRow, MessageCol, F1Enable, F2Enable, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar},
        {&Var1to13Funct, &Var1to30Funct, &ToggleF1Enable, &ToggleF2Enable, &ExitMenu, &NullFunction, &NullFunction, &NullFunction, &NullFunction, &NullFunction, &NullFunction}
    },
    {
        {0, 0, 0, 2},
        {"   DIAGNOSTIC MENU  ", " EXIT               ", "                    ", "                    ", "                    ", "                    ", "                    ", "                    ", "                    ", "                    ", "                    ", "                    "},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar},
        {&ExitMenu, &NullFunction, &NullFunction, &NullFunction, &NullFunction, &NullFunction, &NullFunction, &NullFunction, &NullFunction, &NullFunction, &NullFunction}
    },
    {
        {0, 0, 0, 3},
        {"      CAMERAS       ", " FIND CAMERAS       ", " EXIT               ", "                    ", "                    ", "                    ", "                    ", "                    ", "                    ", "                    ", "                    ", "                    "},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar},
        {&findCameras, &ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu}
    },
    {
        {0, 0, 0, 4},
        {"     STATUS MENU    ", " SOFTWARE REV       ", " SERIAL NUM         ", " BATT #             ", " STATE OF CHARGE    ", " STATE OF HEALTH    ", " B:---,C1:---,C2:---", " BATT STATUS        ", " CLEAR BATT ERROR   ", " VIEW NEXT BATT     ", " EXIT               ", "                    "},
        {16, 15, 11, 17, 17, 1, 15, 0, 0, 0, 0},
        {Rev, SerialNum, BattSerNum, BattSOC, BattSOH, BatteryTemps, BattErr, NullVar, NullVar, NullVar, NullVar},
        {&NullFunction, &setSerial, &NullFunction, &NullFunction, &NullFunction, &NullFunction, &NullFunction, &ClrBattErr, &NextBatt, &ExitMenu, &NullFunction}
    },
    {
        {0, 0, 0, 5},
        {"   AUXILIARY EQUIP  ", " EXIT               ", "                    ", "                    ", "                    ", "                    ", "                    ", "                    ", "                    ", "                    ", "                    ", "                    "},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar},
        {&ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu}
    },
    {
        {0, 0, 0, 6},
        {"     DEFAULTS       ", " EXIT               ", " RESTORE DEFAULTS   ", "                    ", "      WARNING       ", "YOU WILL LOSE ALL   ", "OF THE CURRENT      ", "SETTINGS WHEN       ", "RESTORING DEFAULTS  ", "                    ", "                    ", "                    "},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar},
        {&ExitMenu, &RestoreDefaults, &NullFunction, &NullFunction, &NullFunction, &NullFunction, &NullFunction, &NullFunction, &NullFunction, &NullFunction, &NullFunction}
    }
};

struct MenuStruct Menuc[MenuSize];

struct MenuStack defaultMenuStackc[MenuStackSize] = {
    {
        {0, 0, 0, 0},
        1,
        0
    }
};
struct MenuStack MenuStackc[MenuStackSize];

unsigned char StackPointer = 0;

char Menu[12][21];

char VariableFlag = 0;

double increment = 1.0;          //used to increment and decrement variables
double uLimit = 25.0;            //numerical upper limit
double lLimit = 0.0;             //numerical lower limit
int nDigits = 2;

#define SER_NUM 8
//specify off for 0, and 1 for on
#define OFF_ON  9


struct MsgStruct Messages[4];      //Proc:, Icam:, Warn:, Emsg:, messages


/***************************************************************************/ 

// external declaration for the process image array

/**************************************************************************/


struct MatrixTbl Keypad_Matrix[18] = {
    {
        0x0100,
        {0x00, 0x0C},
        {0x18, 0x00, 0x00}
    },
    {
        0x0080,
        {0x00, 0x0A},
        {0x08, 0x00, 0x40}
    },
    {
        0x8000,
        {0x00, 0x09},
        {0x08, 0x00, 0x80}
    },
    {
        0x2000,
        {0x40, 0x08},
        {0x09, 0x00, 0x00}
    },
    {
        0x0001,
        {0x20, 0x08},
        {0x08, 0x02, 0x00}
    },
    {
        0x0004,
        {0x10, 0x08},
        {0x08, 0x04, 0x00}
    },
    {
        0x0400,
        {0x00, 0x24},
        {0x14, 0x00, 0x00}
    },
    {
        0x0040,
        {0x00, 0x22},
        {0x04, 0x00, 0x40}
    },
    {
        0x0000,
        {0x00, 0x21},
        {0x04, 0x00, 0x80}
    },
    {
        0x1000,
        {0x40, 0x20},
        {0x05, 0x00, 0x00}
    },
    {
        0x0003,
        {0x20, 0x20},
        {0x04, 0x02, 0x00}
    },
    {
        0x0008,
        {0x10, 0x20},
        {0x04, 0x04, 0x00}
    },
    {
        0x0200,
        {0x00, 0x14},
        {0x12, 0x00, 0x00}
    },
    {
        0x0020,
        {0x00, 0x12},
        {0x02, 0x00, 0x40}
    },
    {
        0x4000,
        {0x00, 0x11},
        {0x02, 0x00, 0x80}
    },
    {
        0x0800,
        {0x40, 0x10},
        {0x03, 0x00, 0x00}
    },
    {
        0x0002,
        {0x20, 0x10},
        {0x02, 0x02, 0x00}
    },
    {
        0x0010,
        {0x10, 0x10},
        {0x02, 0x04, 0x00}
    }
};


void setDefaultMenu(void)
{
    int i;
	
	for (i = 0; i < MenuSize; i++)
	{
	    Menuc[i] = defaultMenuc[i];
    }
	
	memcpy(MenuStackc, defaultMenuStackc, sizeof(defaultMenuStackc));
	StackPointer = 0;
	VariableFlag = 0;    
}

	 
char BattNum[20][4];
unsigned char ptrDispBatt = 0;

void Scan_Batteries ( void )
{
    unsigned char i,ptrGetBatt=0;
	char BMS_Temp;
	
	//clear previous data
	for (i=0;i<20;i++)
	    BattNum[i][0] = BattNum[i][1] = BattNum[i][2] = BattNum[i][3] = 0;
	gProcImg[OUT_digi_13] = gProcImg[OUT_digi_13+1] = gProcImg[OUT_digi_13+2] = gProcImg[OUT_digi_13+3] = 0;
	gProcImg[OUT_digi_21] = gProcImg[OUT_digi_21+1] = gProcImg[OUT_digi_21+3] = gProcImg[OUT_digi_21+4] = gProcImg[OUT_digi_21+5] = 0;
	sprintf(BattErr,"   OK");

	
	//Send Supervisor_General_Request
	gProcImg[IN_digi_5] = 0x81;
	for (i=1;i<8;i++)
	    gProcImg[IN_digi_5+i] = 0x00;
    MCO_ProcessStack();

    Timer2 = RTI_One_Sec * .030;
	while ( Timer2 && !gProcImg[OUT_digi_13])
        MCO_ProcessStack();
	
	//Get BMS_ID_SERIAL_NUMBER_ANSWER(s) up to 20
	do
	{
    	BattNum[ptrGetBatt][0] = gProcImg[OUT_digi_13];
    	BattNum[ptrGetBatt][1] = gProcImg[OUT_digi_13+1];
    	BattNum[ptrGetBatt][2] = gProcImg[OUT_digi_13+2];
    	BattNum[ptrGetBatt][3] = gProcImg[OUT_digi_13+3];
    	
    	gProcImg[OUT_digi_13] = 0;
    	
        Timer2 = RTI_One_Sec * .030;
    	while ( Timer2 && !gProcImg[OUT_digi_13] )
            MCO_ProcessStack();
		ptrGetBatt++;
	}
	while (gProcImg[OUT_digi_13] && ptrGetBatt < 20); // Multiple batteries will send multiple serial numbers

	if (ptrDispBatt >= 20 || !BattNum[ptrDispBatt][0])
	    ptrDispBatt = 0;
		
	gProcImg[IN_digi_13] = BattNum[ptrDispBatt][0];
	gProcImg[IN_digi_13+1] = BattNum[ptrDispBatt][1];
	gProcImg[IN_digi_13+2] = BattNum[ptrDispBatt][2];
	gProcImg[IN_digi_13+3] = BattNum[ptrDispBatt][3];
	for (i=4;i<7;i++)
	    gProcImg[IN_digi_13+i] = 0x00;
	gProcImg[IN_digi_13+7] = 0x03;

    Timer2 = RTI_One_Sec * .030;
	while ( Timer2 && !gProcImg[OUT_digi_21+4])
        MCO_ProcessStack();

	sprintf (BattSerNum,"%02d-%02d-%03d",BattNum[ptrDispBatt][0],BattNum[ptrDispBatt][1],(unsigned int)((BattNum[ptrDispBatt][2]<<8)+BattNum[ptrDispBatt][3]));
	sprintf (BattSOH,"%3d", gProcImg[OUT_digi_21+4]);
	sprintf (BattSOC,"%3d", gProcImg[OUT_digi_21+5]);



	//Change RPDO 7 so we can get Alarm Status and Board Temperature
	gCANFilter--;  //This will probably only work on the last RPDO
	MCO_InitRPDO(6,0x020,8,OUT_digi_21);

	gProcImg[IN_digi_13] = BattNum[ptrDispBatt][0];
	gProcImg[IN_digi_13+1] = BattNum[ptrDispBatt][1];
	gProcImg[IN_digi_13+2] = BattNum[ptrDispBatt][2];
	gProcImg[IN_digi_13+3] = BattNum[ptrDispBatt][3];
	for (i=4;i<7;i++)
	    gProcImg[IN_digi_13+i] = 0x00;
	gProcImg[IN_digi_13+7] = 0x64;

    Timer2 = RTI_One_Sec * .030;
	while ( Timer2 )
        MCO_ProcessStack();

	if (gProcImg[OUT_digi_21] | gProcImg[OUT_digi_21+1] | gProcImg[OUT_digi_21+3])
	    sprintf(BattErr,"Error");
	BMS_Temp = gProcImg[OUT_digi_21+2];
	
	//Change RPDO 7 so we can get Cell1 and Cell2 Temperatures
	gCANFilter--;  //This will probably only work on the last RPDO
	MCO_InitRPDO(6,0x009,8,OUT_digi_21);

	gProcImg[IN_digi_13] = BattNum[ptrDispBatt][0];
	gProcImg[IN_digi_13+1] = BattNum[ptrDispBatt][1];
	gProcImg[IN_digi_13+2] = BattNum[ptrDispBatt][2];
	gProcImg[IN_digi_13+3] = BattNum[ptrDispBatt][3];
	for (i=4;i<7;i++)
	    gProcImg[IN_digi_13+i] = 0x00;
	gProcImg[IN_digi_13+7] = 0x03;

    Timer2 = RTI_One_Sec * .030;
	while ( Timer2 )
        MCO_ProcessStack();

	sprintf(BatteryTemps,"B:%3d,C1:%3d,C2:%3d",BMS_Temp-40,gProcImg[OUT_digi_21]-40,gProcImg[OUT_digi_21+1]-40);
	
	//Change RPDO 7 back so we can get Alarm Status SOH and SOC when going to the next battery
	gCANFilter--;
	MCO_InitRPDO(6,0x007,8,OUT_digi_21);
	
	
	gProcImg[IN_digi_5] = 0x00;//This will clear the data in TPDO 3 so it can be transmitted again if this function is called 
	gProcImg[IN_digi_13+7] = 0x00;
	
}

int NullFunction ( void )
{
    return 0;
}

int ClrBattErr ( void )
{
	gProcImg[IN_digi_13] = BattNum[ptrDispBatt][0];
	gProcImg[IN_digi_13+1] = BattNum[ptrDispBatt][1];
	gProcImg[IN_digi_13+2] = BattNum[ptrDispBatt][2];
	gProcImg[IN_digi_13+3] = BattNum[ptrDispBatt][3];
	gProcImg[IN_digi_13+4] = 0x02;
	gProcImg[IN_digi_13+5] = 0x3c;
	gProcImg[IN_digi_13+6] = 0x00;
	gProcImg[IN_digi_13+7] = 0x07;

    Timer2 = RTI_One_Sec * .030;
	while ( Timer2 )
        MCO_ProcessStack();

	Scan_Batteries();
	UpdateMenu = 1;
	return 0;
}

int NextBatt ( void )
{
    if (++ptrDispBatt >= 20)
    {
        ptrDispBatt = 0;
    }
	Scan_Batteries();
	UpdateMenu = 1;
	return 0;
}

int setSerial ( void )
{

    if ( VariableFlag >= 5 )
    {
        VariableFlag = 0;
       	LoadMenu ( MenuStackc[StackPointer].Index );
       	InsertCursor ();
        Save_Serial_Num();
	}

    if( SerialNum[4] == '-' || SerialNum[3] == '-' || SerialNum[2] == '-' 
        || SerialNum[1] == '-' || SerialNum[0] == '-' )
    {
        if ( VariableFlag )
        {
            VariableFlag++;
        }
        else
        {
            VariableFlag = 1;
            increment = 1.0;    //numerical amount to increment or decrement
            lLimit = 0.0;      //numerical lower limit
            uLimit = 9.0;     //numerical upper limit
            nDigits = SER_NUM;    //number of digits to display including decimal x10

            LoadMenu ( MenuStackc[StackPointer].Index );
        }
    }
    btnDisplayTitler ();
    return 0;
}

/* Select active alternate-key modes without changing latched/output bits. */
unsigned int EnabledFunctionData(unsigned int data)
{
    if (strcmp(F1Enable, " ON")) data &= ~TeleData_F1;
    if (strcmp(F2Enable, " ON")) data &= ~TeleData_F2;
    return data;
}

void NormalizeFunctionSettings(void)
{
    /* Old EEPROM has no valid strings in these newly added fields. */
    F1Enable[3] = F2Enable[3] = 0;
    if (strcmp(F1Enable, " ON") && strcmp(F1Enable, "OFF"))
        strcpy(F1Enable, " ON");
    if (strcmp(F2Enable, " ON") && strcmp(F2Enable, "OFF"))
        strcpy(F2Enable, " ON");
    AlternateFunctionMask = (!strcmp(F1Enable, " ON") ? TeleData_F1 : 0) |
                            (!strcmp(F2Enable, " ON") ? TeleData_F2 : 0);
}

int ToggleF1Enable(void)
{
    if (!strcmp(F1Enable, " ON")) strcpy(F1Enable, "OFF");
    else strcpy(F1Enable, " ON");
    NormalizeFunctionSettings();
    LoadMenu(MenuStackc[StackPointer].Index);
    InsertCursor();
    btnDisplayTitler();
    return 0;
}

int ToggleF2Enable(void)
{
    if (!strcmp(F2Enable, " ON")) strcpy(F2Enable, "OFF");
    else strcpy(F2Enable, " ON");
    NormalizeFunctionSettings();
    LoadMenu(MenuStackc[StackPointer].Index);
    InsertCursor();
    btnDisplayTitler();
    return 0;
}

#ifdef BUTTONBOX_GCC
void EncodeLegacySettings(unsigned char legacy[28])
{
    memcpy(legacy, FirstVar, 4);
    memcpy(legacy + 4, MessageRow, 4);
    memcpy(legacy + 8, MessageCol, 4);
    memcpy(legacy + 12, SerialNum, 6);
    memcpy(legacy + 18, F1Enable, 4);
    memcpy(legacy + 22, F2Enable, 4);
    legacy[26] = legacy[27] = 0;
}

void DecodeLegacySettings(const unsigned char legacy[28])
{
    memcpy(FirstVar, legacy, 4);
    memcpy(MessageRow, legacy + 4, 4);
    memcpy(MessageCol, legacy + 8, 4);
    memcpy(SerialNum, legacy + 12, 6);
    memcpy(F1Enable, legacy + 18, 4);
    memcpy(F2Enable, legacy + 22, 4);
    NormalizeFunctionSettings();
    FirstVar[3] = MessageRow[3] = MessageCol[3] = SerialNum[5] = 0;
}
#endif

void Load_Variables (void)
{
 	 char *VarEEPROMPntr;
#ifndef BUTTONBOX_GCC
     char *VarRAMPntr;
#endif
	 #ifndef BUTTONBOX_GCC
     VarRAMPntr = FirstVar;
#endif
	 VarEEPROMPntr = (char *)0x0800;
	 if ( (unsigned char)*VarEEPROMPntr != 0xff )
	 {
#ifdef BUTTONBOX_GCC
         /* Preserve the ImageCraft EEPROM offsets without linker-order assumptions. */
         DecodeLegacySettings((const unsigned char *)0x0800);
#else
         do
         {
             *VarRAMPntr++ = *VarEEPROMPntr++;
         }while ( VarRAMPntr != NullVar );
#endif
         NormalizeFunctionSettings();
	 }
	 else
     {
         strcpy(F1Enable, " ON");
         strcpy(F2Enable, " ON");
         NormalizeFunctionSettings();
         Save_Variables();
     }
}

void Save_Variables ( void )
{
#ifdef BUTTONBOX_GCC
    {
        unsigned char legacy[28];
        EncodeLegacySettings(legacy);
        EEWrite(28, (char *)legacy, (int *)0x0800);
    }
#else
    EEWrite ( 128, FirstVar, (int *)0x0800 );
#endif
    Timer2 = 0.05 * RTI_One_Sec;
    while (Timer2) { /* wait for interrupt */ }
}

//retrieve serial number from EEProm
void Load_Serial_Num ( void )
{
    char *VarEEPROMPntr2;
    char tempSerialNum[5];            //temporary serial number
    int k;
    VarEEPROMPntr2 = (char *)0x0902;
    
    tempSerialNum[0] = *VarEEPROMPntr2;
    tempSerialNum[1] = *(VarEEPROMPntr2 + 1);   
    tempSerialNum[2] = *(VarEEPROMPntr2 + 2);   
    tempSerialNum[3] = *(VarEEPROMPntr2 + 3);   
    tempSerialNum[4] = *(VarEEPROMPntr2 + 4);
    
    for(k=0; k<5 && tempSerialNum[k] >= '0' && tempSerialNum[k] <= '9' ; ++k);
    //if all serial characters are valid, k will = 5
    if( k == 5)
    {
        SerialNum[0] = tempSerialNum[0];
        SerialNum[1] = tempSerialNum[1];   
        SerialNum[2] = tempSerialNum[2];   
        SerialNum[3] = tempSerialNum[3];   
        SerialNum[4] = tempSerialNum[4];
    }
}

void Save_Serial_Num ( void )
{
    char *EEpromPtr;
    
    EEpromPtr = &SerialNum[0];

 	EEWrite ( 5, EEpromPtr, (int *)0x0902 );
    Timer2 = 0.05 * RTI_One_Sec;   //EEProm is not available for brief period after EEWrite
    while (Timer2) { /* wait for interrupt */ }
}

int Var1to13Funct ( void )
{
    if ( VariableFlag )
    {
        VariableFlag = 0;
        LoadMenu ( MenuStackc[StackPointer].Index );
        InsertCursor ();
    }
    else
    {
        VariableFlag = 1;
        increment = 1.0;    //numerical amount to increment or decrement
        lLimit = 1.0;      //numerical lower limit
        uLimit = 13.0;     //numerical upper limit
        nDigits = 30;    //number of digits to display including decimal x10

        LoadMenu ( MenuStackc[StackPointer].Index );
    }
    btnDisplayTitler ();
    return 0;
}

int Var1to30Funct ( void )
{
    if ( VariableFlag )
    {
        VariableFlag = 0;
        LoadMenu ( MenuStackc[StackPointer].Index );
        InsertCursor ();
    }
    else
    {
        VariableFlag = 1;
        increment = 1.0;    //numerical amount to increment or decrement
        lLimit = 1.0;      //numerical lower limit
        uLimit = 30.0;     //numerical upper limit
        nDigits = 30;    //number of digits to display including decimal x10

        LoadMenu ( MenuStackc[StackPointer].Index );
    }
    btnDisplayTitler ();
    return 0;
}

int ExitMenu ( void )
{
    DeSelect ();
    return 0;
}

void CursorUp( void )
{
    if ( VariableFlag )
    {
         IncVariable ();
    }
    else
    {
       MenuStackc[StackPointer].CursorPos--;
       LoadMenu ( MenuStackc[StackPointer].Index );
       InsertCursor ();
       btnDisplayTitler ();
    }
}

void CursorDown( void )
{
    if ( VariableFlag )
    {
         DecVariable ();
    }
    else
    {
       MenuStackc[StackPointer].CursorPos++;
       LoadMenu ( MenuStackc[StackPointer].Index );
       InsertCursor ();
       btnDisplayTitler ();
    }
}

void IncVariable ( void )
{
    int k;
    int varIndex;       //index of variable
    double j;
    char *vPtr2;        //will also point to variable to be decremented

     //Find out which menu we are on
     for ( k=0;k<MenuSize;k++ )
     {
        if ( Menuc[k].Index[0] == MenuStackc[StackPointer].Index[0] && 
            Menuc[k].Index[1] == MenuStackc[StackPointer].Index[1] && 
            Menuc[k].Index[2] == MenuStackc[StackPointer].Index[2] && 
            Menuc[k].Index[3] == MenuStackc[StackPointer].Index[3] )
        {
            varIndex = MenuStackc[StackPointer].CursorPos+MenuStackc[StackPointer].FirstLine-1;
            vPtr2 = Menuc[k].VarPntr[varIndex];     //copy pointer to variable

            if( nDigits == OFF_ON ) {
                if ( !(strcmp ( vPtr2, "OFF" )) )
                    j = 1;      //switch to ON
                else
                    j = 0;      //switch to OFF
                break;     
            }

            if( nDigits == SER_NUM ) {              //limit to numerical serial numbers
                if( SerialNum[VariableFlag -1]  == '-' )
                    SerialNum[VariableFlag -1]  = '0';
                else
                {
                    if( SerialNum[VariableFlag -1] < '9' )     //ensure it's within lower limit
                        SerialNum[VariableFlag -1]++;         //decrement value
                    else
                        SerialNum[VariableFlag -1] = '0';         //wrap around to lower limit
                }
                break;
            }

            //determine if variable is a numeric value
            if( nDigits > OFF_ON ) {
                //convert text to numeric value
                j =  atof ( vPtr2 );

                if( (j + increment) <= uLimit )     //ensure it's within upper limit
                    j += increment;     //increment value
                else
                    j = lLimit;        //wrap around to lower limit           
                break;     
            }
        }
    }
    dispVariable(vPtr2, &j, nDigits);
    LoadMenu ( MenuStackc[StackPointer].Index );
    btnDisplayTitler ();
}

void DecVariable ( void )
{
    int k;
    int varIndex;       //index of variable
    double j;
    char *vPtr2;        //will also point to variable to be decremented

    //Find out which menu we are on
    for ( k=0;k<MenuSize;k++ )
    {
        if ( Menuc[k].Index[0] == MenuStackc[StackPointer].Index[0] && Menuc[k].Index[1] == 
            MenuStackc[StackPointer].Index[1] && Menuc[k].Index[2] == 
            MenuStackc[StackPointer].Index[2] && Menuc[k].Index[3] ==
            MenuStackc[StackPointer].Index[3] )
        {
            //VarPnt = MenuStackc[StackPointer].CursorPos+MenuStackc[StackPointer].FirstLine-1;
            varIndex = MenuStackc[StackPointer].CursorPos+MenuStackc[StackPointer].FirstLine-1;
            vPtr2 = Menuc[k].VarPntr[varIndex];     //copy pointer to variable
            
            if( nDigits == OFF_ON ) 
            {
                if ( !(strcmp ( vPtr2, "OFF" )) )
                    j = 1;      //switch to ON
                else
                    j = 0;      //switch to OFF
                break;     
            }
            if( nDigits == SER_NUM )               //limit to numerical serial numbers
            {
                if( SerialNum[VariableFlag -1]  == '-' )
                    SerialNum[VariableFlag -1]  = '0';
                else
                {
                    if( SerialNum[VariableFlag -1] > '0' )     //ensure it's within upper limit
                        SerialNum[VariableFlag -1]--;         //increment value
                    else
                        SerialNum[VariableFlag -1] = '9';         //wrap around to upper limit
                }
                break;
            }
            //determine if variable is a numeric value format for numeric values is greater than OFF_ON(6)
            if( nDigits > OFF_ON )
            {
                //convert text to numeric value
                j =  atof ( vPtr2 );
                j -= increment;         //decrement value
                if( j < lLimit )        //
                {
                    if( j < ( lLimit - (increment/2) ) )        //ensure it's past lLimit
                        j = uLimit;         //wrap around to upper limit
                    else
                        j = lLimit;
                }
                break;
            }
        }
    }
    dispVariable(vPtr2, &j, nDigits);
    LoadMenu ( MenuStackc[StackPointer].Index );
    btnDisplayTitler ();
}

/*****************************************
    display variable
//portable function to insert formatted variables into Menuc
//argument vPtr (variable pointer) points to the location of the variable
//argument value is the number to be displayed
//argument format, selects the format of the value
******************************************/
void dispVariable(char* vPtr, double *value, char format)
{
    switch( format ) {
        case 10:
            sprintf ( vPtr, "%1d", (int)*value );
            break;
        case 20:
            sprintf ( vPtr, "%2d", (int)*value );
            break;
        case 30:
            sprintf ( vPtr, "%3d", (int)*value );
            break;
        case 31:
            sprintf ( vPtr, "%#3.1f", *value );
            break;
        case 40:
            sprintf ( vPtr, "%4d", (int)*value );
            break;
        case 41:
            sprintf ( vPtr, "%#4.1f", *value );
            break;
        case 42:
            sprintf ( vPtr, "%#4.2f", *value );
            break;
        case 50:
            sprintf ( vPtr, "%5d", (int)*value );
            break;
        case 52:
            sprintf ( vPtr, "%#5.2f", *value );
            break;
        case SER_NUM:        // 8
            break;
        case OFF_ON:        // 9
            if( *value )     //on
                sprintf ( vPtr, " ON" );
            else            //off
                sprintf ( vPtr, "OFF" );
            break;
    }
}

void Select ( void )
{
     int k;
     
     //Save current menu pointers
     MenuStackc[StackPointer+1].Index[0]=MenuStackc[StackPointer].Index[1];
     MenuStackc[StackPointer+1].Index[1]=MenuStackc[StackPointer].Index[2];
     MenuStackc[StackPointer+1].Index[2]=MenuStackc[StackPointer].Index[3];
     MenuStackc[StackPointer+1].Index[3] = MenuStackc[StackPointer].CursorPos + MenuStackc[StackPointer].FirstLine;

     //look for selected menu 
     for ( k=0;k<MenuSize;k++ )
     {
        if ( Menuc[k].Index[0] == MenuStackc[StackPointer+1].Index[0] && 
            Menuc[k].Index[1] == MenuStackc[StackPointer+1].Index[1] && 
            Menuc[k].Index[2] == MenuStackc[StackPointer+1].Index[2] && 
            Menuc[k].Index[3] == MenuStackc[StackPointer+1].Index[3] )
        {
             //If found increment stackpointer and initialize cursor
             StackPointer++;
             //Cursor to menu entry 1
             MenuStackc[StackPointer].CursorPos = 1;
             MenuStackc[StackPointer].FirstLine = 0;
             LoadMenu ( MenuStackc[StackPointer].Index );
             InsertCursor ();
             btnDisplayTitler ();
             break;
        }
     } 

     if ( k >= MenuSize )
     {
         //Find out which menu we are on
         for ( k=0;k<MenuSize;k++ )
         {
            if ( Menuc[k].Index[0] == MenuStackc[StackPointer].Index[0] && Menuc[k].Index[1] == MenuStackc[StackPointer].Index[1] && 
                 Menuc[k].Index[2] == MenuStackc[StackPointer].Index[2] && Menuc[k].Index[3] == MenuStackc[StackPointer].Index[3] )
            {
                 //Execute function for that entry 
                 Menuc[k].FunctPtr[(MenuStackc[StackPointer].CursorPos + MenuStackc[StackPointer].FirstLine)-1]();//(MenuStackc[StackPointer].CursorPos + MenuStackc[StackPointer].FirstLine,0);
                 //LoadMenu ( MenuStackc[StackPointer].Index );
                 //DisplayMenu ();
                 break;
            }
         }
     }
}

void DeSelect ( void )
{
     if (StackPointer )
     {
        StackPointer--;
        LoadMenu ( MenuStackc[StackPointer].Index );
        InsertCursor ();
        btnDisplayTitler ();
     }
     else
     {
        Gen_Flags &= ~Gen_Flags_Menu_Active;
        ClearTitler ();
        Save_Variables ();
     }
	 VariableFlag = 0;
}

void LoadMenu ( unsigned char Index[] )
{
    int i, j, k;

    //find menu
    for ( k=0;k<MenuSize;k++ )
    {
        if ( Menuc[k].Index[0] == Index[0] && Menuc[k].Index[1] == Index[1] && 
             Menuc[k].Index[2] == Index[2] && Menuc[k].Index[3] == Index[3] )
             break;
    } 
	
	
    //Get Menu Title
    for ( j=0;j<=20;j++ )
	{
        Menu[0][j] = Menuc[k].Entry[0][j];
	}

    //See if this entry is a valid menu entry, if not start cursor back at top
    if ( MenuStackc[StackPointer].CursorPos > 0 && Menuc[k].Entry[MenuStackc[StackPointer].CursorPos + MenuStackc[StackPointer].FirstLine][1] == ' ' )
    {
         MenuStackc[StackPointer].CursorPos = 1;
         MenuStackc[StackPointer].FirstLine = 0;
    }
        
    //Allow a maximum of 12 entries, if past 12, start cursor back at top
    if ( MenuStackc[StackPointer].CursorPos + MenuStackc[StackPointer].FirstLine >= 12 )
    {
         MenuStackc[StackPointer].CursorPos = 1;
         MenuStackc[StackPointer].FirstLine = 0;
    }
        
    //If cursor is too far down, check to see if there is another entry, if so, show it and set cursor to bottom entry.
    if ( MenuStackc[StackPointer].CursorPos == 9 )
    {
         if ( Menuc[k].Entry[9+MenuStackc[StackPointer].FirstLine][1] != ' ' )
         {
            MenuStackc[StackPointer].FirstLine++;
            MenuStackc[StackPointer].CursorPos = 8;
         }
         else
         {
             MenuStackc[StackPointer].CursorPos = 1;
             MenuStackc[StackPointer].FirstLine = 0;
         }
    }

    //If cursor is too far up, and the first entry is not the first line then move firstline up and set cursor to top entry
    if ( MenuStackc[StackPointer].CursorPos == 0 )
    {
         if ( MenuStackc[StackPointer].FirstLine )
         {
            MenuStackc[StackPointer].FirstLine--;
            MenuStackc[StackPointer].CursorPos = 1;
         }
         else
         {
             //Find last entry
             for ( i=11;i>0;i-- )
             {
                 if ( Menuc[k].Entry[i][1] != ' ' && Menuc[k].Entry[i][0] == ' ' )
                      break;
             }
             //If more than 8 entries, cursor will be at the bottom (8) and first line will be adjusted
             if ( i > 8 )
             {
                 MenuStackc[StackPointer].CursorPos = 8;
                 MenuStackc[StackPointer].FirstLine = i - 8;
             }
             //Otherwise the cursor will be on that entry with the firstline at 0
             else
             {
                 MenuStackc[StackPointer].CursorPos = i;
                 MenuStackc[StackPointer].FirstLine = 0;
             }
         }
    }

    //Load appropriate menu entries into Menu[i][j]
    for ( i=1;i<=8;i++ )
	{
	    for ( j=0;j<=20;j++ )
		{
            Menu[i][j] = Menuc[k].Entry[i+MenuStackc[StackPointer].FirstLine][j];
		}
	}

	//Load any variables
    for ( i=1;i<=8;i++ )
	{
        if ( Menuc[k].Pos[i+MenuStackc[StackPointer].FirstLine-1] )
        {
             if ( VariableFlag && i == MenuStackc[StackPointer].CursorPos )
                  Menu[i][Menuc[k].Pos[i+MenuStackc[StackPointer].FirstLine-1]-1] = '>';
             for ( j=0;j<=20;j++ )
             {
                 if ( *(Menuc[k].VarPntr[i+MenuStackc[StackPointer].FirstLine-1]+j) )
                 {
                    Menu[i][Menuc[k].Pos[i+MenuStackc[StackPointer].FirstLine-1]+j] = *(Menuc[k].VarPntr[i+MenuStackc[StackPointer].FirstLine-1]+j);
                 }
                 else
                 {
                     break;
                 }
             }
        }
	}
}

void InsertCursor ( void )
{
	//Display Cursor Pointer
    Menu[MenuStackc[StackPointer].CursorPos][0] = '>';
}

void btnDisplayTitler ( void )
{
	int i, j, k;
	
	for ( i=0;i<=8;i++ )
	{
	    for ( j=0;j<=15;j+=5 )
		{
		 	for ( k=0;k<=4;k++ )
			{
				DispBuf[i*20+j+k+6] = Menu[i][j+k];  //the menu is treated as recieve over CAN
									  				 //the first 6 char are (stx)MENU:
													 //strcpy() would work better here
			}
    	    COP_Trig();
		}
	}
	showMenu ();
}

int RestoreDefaults ( void )
{
    strcpy(F1Enable, " ON");
    strcpy(F2Enable, " ON");
    NormalizeFunctionSettings();
    FirstVar[0] = 0xFF;    //indicates data needs to be replaced with defaults
    Save_Variables();         
    ResetProc ();
    return 0;
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
		     if (i + 1 < DEVICE_SCAN_CAPACITY && Device_Type_Response_ID[i] == Device_Type_Response_ID[i+1])  //if duplicate detected
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
	 	else if ( i + 1 < DEVICE_SCAN_CAPACITY && Device_Type_Response_ID[i] != 0 && Device_Type_Response_ID[i] == Device_Type_Response_ID[i+1])
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


int ExternalMenu ( void )
{
    int i;
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

int ExternalSubMenu ( void )
{
    int i;
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

int CameraMenu ( void )
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

int Trigger_Device ( void )
{
    int i;
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
		//static unsigned char Prev_Cntrl_Sw;
		
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
			//Prev_Cntrl_Sw = 1;
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
			//Prev_Cntrl_Sw = 0;
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

			Scan_Batteries();

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

void Xmit_Rcv_SPI ( void )
{   
    int i;
	
	if(firstWriteSPI) //dummy write
	{
	    SPI0OutBufptr = 1;
	    PTM &= ~0x08;
		for(i = 0; i < 5; i++);	
	    SPI0DR = 0x01; 
		SPI0CR1 |= SPICR1_SPTIE;
    	while (SPI0CR1 & SPICR1_SPTIE) { /* wait for interrupt */ }
		PTM |= 0x08;
	    firstWriteSPI = false;    
	}
		
 	PTM &= ~0x08;
	for(i = 0; i < 5; i++);	
	SPI0DR = SPI0OutBuf[0];
    SPI0CR1 |= SPICR1_SPTIE;
    while (SPI0CR1 & SPICR1_SPTIE) { /* wait for interrupt */ }
	PTM |= 0x08;
}


void WriteCharMem ( void )
{
    unsigned char CMM, CMAH, CMAL;
	
    SPI0OutBufptr = 0;
   	SPI0OutBuf[0] = 0x00;	   //Disable OSD
   	SPI0OutBuf[1] = 0x00;
   	Xmit_Rcv_SPI();

	Timer1 = .1 * RTI_One_Sec;
	while (Timer1) { /* wait for interrupt */ }

	CMAH = 0x00;
	CMM = 0xa0;
	
    for (CMAH = 0; CMAH < 19; CMAH++)
	{
        CMAL = 0x00;
	    SPI0OutBufptr = 0;
    	SPI0OutBuf[0] = 0x09;	       //Character memory high address (write)
    	SPI0OutBuf[1] = CMAH + 0x4d;   //4d is the location after the @ symbol
    	Xmit_Rcv_SPI();
		
		for (CMAL = 0; CMAL < 54; CMAL++)
		{
		    SPI0OutBufptr = 0;
    		SPI0OutBuf[0] = 0x0a;	   //Character memory low address (write)
    		SPI0OutBuf[1] = CMAL;      //0-53 
    		Xmit_Rcv_SPI();
			
		    SPI0OutBufptr = 0;
			SPI0InBufptr = 0;
    		SPI0OutBuf[0] = 0x0b;	                  //Character memory data in (write)
    		SPI0OutBuf[1] = CMDI[CMAH*54 + CMAL];     //writes data from custom Character Array
    		Xmit_Rcv_SPI();
		}
		
	    SPI0OutBufptr = 0;
    	SPI0OutBuf[0] = 0x08;	       //Character memory mode register (write)
    	SPI0OutBuf[1] = CMM;           //Writes to NVM array from shadow RAM
    	Xmit_Rcv_SPI();
		
		Timer1 = .1 * RTI_One_Sec;
		while (Timer1) { /* wait for interrupt */ }

    }
	Timer1 = .2 * RTI_One_Sec;
	while (Timer1) { /* wait for interrupt */ }
}

void ReadCharMem ( void )
{
    unsigned char CMM, CMAH, CMAL;
	int i;
	
    SPI0OutBufptr = 0;
   	SPI0OutBuf[0] = 0x00;	   //Disable OSD
   	SPI0OutBuf[1] = 0x00;
   	Xmit_Rcv_SPI();

	Timer1 = .1 * RTI_One_Sec;
	while (Timer1) { /* wait for interrupt */ }
	
	CMM = 0x50;
	CMAH = 5;           //determining if the "+" symbol is there is sufficient: CMAH = 5
    CMAL = 0x00;
    SPI0OutBufptr = 0;
    SPI0OutBuf[0] = 0x09;	          //Character memory high address (write)
    SPI0OutBuf[1] = CMAH + 0x4d;      //4d is the location after the @ symbol
    Xmit_Rcv_SPI();
		
	SPI0OutBufptr = 0;
    SPI0OutBuf[0] = 0x08;	       //Character memory mode register (write)
    SPI0OutBuf[1] = CMM;           //Reads from NVM array into shadow RAM
    Xmit_Rcv_SPI();
	
	Timer1 = .1 * RTI_One_Sec;
	while (Timer1) { /* wait for interrupt */ }


    for (CMAL = 0; CMAL < 54; CMAL++)
    {
        SPI0OutBufptr = 0;
     	SPI0OutBuf[0] = 0x0a;	      //Character memory low address (write)
     	SPI0OutBuf[1] = CMAL;         //0-53 
     	Xmit_Rcv_SPI();
   
        SPI0OutBufptr = 0;
     	SPI0OutBuf[0] = 0xc0;	             //Character memory data out (read only)
		Xmit_Rcv_SPI();		
     	plusSymbol[CMAL] = SPI0InBuf[1];     //reads data from Character Array on MAX7456 	
    }

	//compare arrays
	for (i = 0; i < 54; i++)
	{
	   if (plusSymbol[i] != CMDI[CMAH*54 + i])
	   {
	   	  CharsInMem = false;
		  return;
	   }  
	}
	
    CharsInMem = true;
}

void resetTitler(void)
{
	MenuCheck=0;
	
	SPI0OutBufptr = 0;
    SPI0OutBuf[0] = 0x00;  	   //Video Mode0, Reset
    SPI0OutBuf[1] = 0x02;
	Xmit_Rcv_SPI();
		
	Timer1 = RTI_One_Sec * 0.002;  //short delay to allow reset to finish
	while (Timer1) { /* wait for interrupt */ }
		
	SPI0OutBufptr = 0;
    SPI0OutBuf[0] = 0x00;  	   //Video Mode0, Enable display OSD
    SPI0OutBuf[1] = 0x08;
	Xmit_Rcv_SPI();
	
}

//send menu characters to titler via SPI
//puts Menu in proper location, leaves room for messages on top line
void showMenu ( void )
{
    int i, j;

	MenuCheck=1;
	
	for (i=0; i<=8; i++)
	{
	 	int DispAddr;
		
		DispAddr = ( i + 2 ) * 30 + 4;
		
		SPI0OutBufptr = 0;
        SPI0OutBuf[0] = 0x05;  	   //Display memory high address
        SPI0OutBuf[1] = DispAddr>>8;
    	Xmit_Rcv_SPI();
		
		SPI0OutBufptr = 0;
        SPI0OutBuf[0] = 0x06;	   //Display memory low address
        SPI0OutBuf[1] = DispAddr;
    	Xmit_Rcv_SPI();
		    
		SPI0OutBufptr = 0;
        SPI0OutBuf[0] = 0x04;	   //Display memory mode, 8 bit operation mode, auto increment
        SPI0OutBuf[1] = 0x41;
    	Xmit_Rcv_SPI();
					
	 	for ( j=0;j<=19;j++ )
	 	{
			SPI0OutBufptr = 1;
    	 	SPI0OutBuf[0] = DispTranslate ( &DispBuf[i*20+j+6] );
    	 	Xmit_Rcv_SPI();
		}
		
        SPI0OutBufptr = 1;
        SPI0OutBuf[0] = 0xff;
        Xmit_Rcv_SPI();
	}
}

char DispTranslate ( char *TranChar )
{
 	char RetChar;
	
    if ( *TranChar == '0' )
        RetChar = 0x0a;
    else if ( *TranChar >= '1' && *TranChar <= '9' )
        RetChar = *TranChar - 0x30;
    else if ( *TranChar >= 'A' && *TranChar <= 'Z' )
        RetChar = *TranChar - 0x36;
    else if ( *TranChar >= 'a' && *TranChar <= 'z' )
        RetChar = *TranChar - 0x3c;
    else if ( *TranChar == ' ' )
        RetChar = 0x00;
    else if ( *TranChar == 'v' )
        RetChar = 0x3a;
    else if ( *TranChar == '(' )
        RetChar = 0x3f;
    else if ( *TranChar == ')' )
        RetChar = 0x40;
    else if ( *TranChar == '.' )
        RetChar = 0x41;
    else if ( *TranChar == '?' )
        RetChar = 0x42;
    else if ( *TranChar == ';' )
        RetChar = 0x43;
    else if ( *TranChar == ':' )
        RetChar = 0x44;
    else if ( *TranChar == ',' )
        RetChar = 0x45;
    else if ( *TranChar == 0x27 ) // '
        RetChar = 0x46;
    else if ( *TranChar == '/' )
        RetChar = 0x47;
    else if ( *TranChar == 0x22 ) // "
        RetChar = 0x48;
    else if ( *TranChar == '-' )
        RetChar = 0x49;
    else if ( *TranChar == '<' )
        RetChar = 0x4a;
    else if ( *TranChar == '>' )
        RetChar = 0x4b;
    else if ( *TranChar == '@' )
        RetChar = 0x4c;
    else if ( *TranChar == 0x5c )     // '\' 
        RetChar = 0x4D;
    else if ( *TranChar == '[' )
        RetChar = 0x4E;
    else if ( *TranChar == ']' )
        RetChar = 0x4F;
    else if ( *TranChar == '&' )
        RetChar = 0x50;
    else if ( *TranChar == '^' )
        RetChar = 0x51;
    else if ( *TranChar == '+' )
        RetChar = 0x52;
    else if ( *TranChar == '=' )
        RetChar = 0x53;
    else if ( *TranChar == '_' )
        RetChar = 0x54;
    else if ( *TranChar == '{' )
        RetChar = 0x55;
    else if ( *TranChar == '}' )
        RetChar = 0x56;
    else if ( *TranChar == '*' )
        RetChar = 0x57;
    else if ( *TranChar == '!' )
        RetChar = 0x58;
    else if ( *TranChar == '%' )
        RetChar = 0x59;
    else if ( *TranChar == '$' )
        RetChar = 0x5A;
    else if ( *TranChar == '#' )
        RetChar = 0x5B;
	else if ( *TranChar == '|' )
        RetChar = 0x5C;	
		
	else if ( *TranChar == '\x11' )
        RetChar = 0x5D;
    else if ( *TranChar == '\x12' )
        RetChar = 0x5E;
	else if ( *TranChar == '\x13' )
        RetChar = 0x5F;	
		
	else if ( *TranChar == 0x03 )   //<ETX>
        RetChar = 0x00;
	else if ( *TranChar == 0x02 )   //<STX>
        RetChar = 0x00;

    else if ( *TranChar == 0x04 ) // v
        RetChar = 0x3a;
    else if ( *TranChar == 0x06 ) // >
        RetChar = 0x4b;
    else if ( *TranChar == 0x01 ) // space
        RetChar = 0x00;
        //RetChar = 0x20;
    else if ( *TranChar == 0x3d ) // =
        RetChar = 0x49;
        
    else
	    RetChar = *TranChar;

    return RetChar;
}
/***
numbers 1-9	  x - 0x30
number 0	  x -0x26
letters A-Z	  z - 0x36
letters a-z	  z - 0x3c
symbols		  (  )  .  ?  ;  :  ,  '  /  "  -  <  >  @
		      3f 40 41 42 43 44 45 46 47 48 49 4a 4b 4c
		      28 29 2e 3f 3b 3a 2c 27 2f 22 2d 3c 3e 40
***/

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


void DisplayMsg ( char Message[] )
{
    if( VCI_Flag2 & ICAM_MSG )
        showMessage( Message, atoi(MessageRow) );              //puts Proc:, Warn:, etc messages on video top line
}

void showMessage ( char Message[], int row )       //row = position on video screen, skip row 0
{
    char endofmsg = 0; 
    int column;     //position on video screen
    int index = 0;
    int DispAddr;

    //see if external menu is not active and internal menu is not active
	if ( (gProcImg[OUT_digi_0] & 0x01) || (( Gen_Flags & Gen_Flags_Menu_Active ) && MenuCheck==0))
	{
	    //some menu is active, display on top or bottom row, column selection is okay
    	//if ( row == 1 || row >= 6 )
    	    //DispAddr = 12 * 30 + atoi(MessageCol);
    	//else
    		DispAddr = 1 * 30 + atoi(MessageCol);
	
	}
	else if(MenuCheck==1)
	{
	 	 DispAddr = 1 * 30 + atoi(MessageCol);
	}
	else
	{
	    //no menu is active
        DispAddr = row * 30 + atoi(MessageCol);
	}
    
    SPI0OutBufptr = 0;
    SPI0OutBuf[0] = 0x05;  	   //Display memory high address
    SPI0OutBuf[1] = DispAddr>>8;
    Xmit_Rcv_SPI();
    
    SPI0OutBufptr = 0;
    SPI0OutBuf[0] = 0x06;	   //Display memory low address
    SPI0OutBuf[1] = DispAddr;
    Xmit_Rcv_SPI();
    
    SPI0OutBufptr = 0;
    SPI0OutBuf[0] = 0x04;	   //Display memory mode, 8 bit, auto increment
    SPI0OutBuf[1] = 0x41;
    Xmit_Rcv_SPI();
    
    for ( column=0;column<=24;column++ )   
    {
        SPI0OutBufptr = 1;
		if ( !endofmsg )
		{
		    SPI0OutBuf[0] = DispTranslate ( &Message[6+index++] );
            if ( ( Message[6+index] ) == 0x03 )
			    endofmsg = 1;
		}
		else
		{
		    SPI0OutBuf[0] = 0;
        }
		Xmit_Rcv_SPI();
    }
    SPI0OutBufptr = 1;
    SPI0OutBuf[0] = 0xff;
    Xmit_Rcv_SPI();

    if( VCI_Flag2 |= ICAM_MSG )      //if its a camera position
        VCI_Flag2 &= ~ICAM_MSG;      //clear flag
}

void ClearTitler ( void )
{
//    TCPClearTitler();
    DispBufptr = 0;
	if ( !legacy_mode )//!(Gen_Flags & Gen_Flags_No2Wire) )
	    MCO_ProcessStack();
    Timer1 = RTI_One_Sec * .05;
	while (Timer1) { /* wait for interrupt */ }
	resetTitler();
}

//send characters to titler via SPI
void DisplayTitler ( void )
{
    int i,j;
	int DispAddr;

    //if( VCI_Flag & TCP_PRESENT )
    //TCPDisplayTitler();
    //else if( Gen_Flags & Gen_Flags_Extern_Menu )
    if( Gen_Flags & Gen_Flags_Extern_Menu )
        showMenu();     //only for menus over CAN

    else        //used for VCI menu only
    {
        for ( i=0;i<=13;i++)
		{
            DispAddr =  ( i + 1) * 30 + 1;
        
			SPI0OutBufptr = 0;
			SPI0OutBuf[0] = 0x05;  	   //Display memory high address
			SPI0OutBuf[1] = DispAddr>>8;
			Xmit_Rcv_SPI();
    
			SPI0OutBufptr = 0;
			SPI0OutBuf[0] = 0x06;	   //Display memory low address
			SPI0OutBuf[1] = DispAddr;
			Xmit_Rcv_SPI();
    
			SPI0OutBufptr = 0;
			SPI0OutBuf[0] = 0x04;	   //Display memory mode, 8 bit, auto increment
			SPI0OutBuf[1] = 0x41;
			Xmit_Rcv_SPI();
	
			for ( j=0;j<=27;j++ )
			{
				SPI0OutBufptr = 1;
                SPI0OutBuf[0] = DispTranslate ( &DispBuf[i*28+j] );
				Xmit_Rcv_SPI();
			}

			SPI0OutBufptr = 1;
			SPI0OutBuf[0] = 0xff;
			Xmit_Rcv_SPI();
    	}

    }
}
