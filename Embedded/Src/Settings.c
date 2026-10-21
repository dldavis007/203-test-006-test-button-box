/* Settings persistence. Saved storage remains in Subroutines.c in legacy order. */
#include <string.h>

#include "Settings.h"
#include "ApplicationState.h"
#include "HardwareConfig.h"
#include "Interrupts.h"
#include "EEProm.h"

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
