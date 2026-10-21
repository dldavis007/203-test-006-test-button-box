/* Battery menu diagnostics. Existing storage remains in Subroutines.c. */
#include <stdio.h>
#include "BatteryDiagnostics.h"
#include "Subroutines.h"
#include "Interrupts.h"
#include "mcohw.h"
#include "mco.h"

/* Application access to the unchanged legacy MCO integration. */
extern UNSIGNED8 gCANFilter;
extern UNSIGNED8 gProcImg[PROCIMG_SIZE];

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
