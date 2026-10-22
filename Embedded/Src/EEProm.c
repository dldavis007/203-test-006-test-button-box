#include "EEProm.h"
#include "mc9s12a128.h"
#include "Subroutines.h"

volatile uint8_t EEWriteStatus;

void EEInit ( void )
{
 	 ECLKDIV = (uint8_t)(OscClk * (5 + Tbus));
	 INITEE = INITEE_Init;
}




/* EEPROM bytes must not sign-extend when char is signed under GCC. */
uint16_t EEPROMWord(uint8_t high, uint8_t low)
{
    return ((uint16_t)high << 8) | (uint16_t)low;
}

void EEWrite ( int16_t ArraySize, char WriteData[], int16_t *WriteAddr )
{
 	 //Erases and Writes an array of up to 128 char to EEPROM
	 
	 int16_t i, j, TmpData = 0;
	 char *tempAddr;
	 char tempArray[128];
     EEWriteStatus = 0;
     while (!(ESTAT & ESTAT_CCIF));
     ESTAT = ESTAT_ERRORS; /* Clear latched ACCERR/PVIOL before a new sequence. */
	 
     
	 
	 //Add up to 3 more data bytes to beginning of array
	 //Addr must be divisible by 4 for erase operation
	 
	 tempAddr = (char*)WriteAddr;
     tempAddr = (char*)((int16_t)tempAddr & ~0x03);
	 
	 j = 0;
	 if ( (int16_t)WriteAddr != (int16_t)tempAddr )
	 {
	  	for ( j=0;(int16_t)tempAddr != (int16_t)WriteAddr;tempAddr++, j++)
		{
			tempArray[j] = *tempAddr;
		}	
	 } 

	 //transfer the rest of the data to tempArray

     for (i=0 ;i < ArraySize;i++, j++)
     {
        tempArray[j] = WriteData[i];
     }

	 //pad end of array to make divisible by 4

     if ( (j & 0x03) )
     {
	 	tempAddr = (char*)WriteAddr;
     	tempAddr = (char*)((int16_t)tempAddr & ~0x03);
	  	for ( ;j & 0x3;j++ )
		{
         	tempArray[j] = *(tempAddr + j);
		}
     }

	 
	 //Erase EEprom
	 
	 tempAddr = (char*)WriteAddr;
	 WriteAddr = (int16_t*)((int16_t)WriteAddr & ~0x03);

     for (i=0; i<j; i += 4, WriteAddr +=2 )
     {
		 *(volatile uint16_t *)WriteAddr = (uint16_t)TmpData;
		 ECMD = SecErase;
		 ESTAT = ESTAT_CBEIF;
		 if (ESTAT & ESTAT_ERRORS)
         {
             EEWriteStatus = ESTAT & ESTAT_ERRORS;
             return;
         }
         while (!(ESTAT & ESTAT_CCIF));
         if (ESTAT & ESTAT_ERRORS)
         {
             EEWriteStatus = ESTAT & ESTAT_ERRORS;
             return;
         }
	 }
	 

	 //Write to EEPROM
	 
	 WriteAddr = (int16_t*)tempAddr;	 
	 WriteAddr = (int16_t*)((int16_t)WriteAddr & ~0x03);

     for (i=0;i<j;i+=2, WriteAddr++)
     {
	  	 TmpData = EEPROMWord(tempArray[i], tempArray[i+1]);
		 *(volatile uint16_t *)WriteAddr = (uint16_t)TmpData;
		 ECMD = WordPrg;
		 ESTAT = ESTAT_CBEIF;
		 if (ESTAT & ESTAT_ERRORS)
         {
             EEWriteStatus = ESTAT & ESTAT_ERRORS;
             return;
         }
         while (!(ESTAT & ESTAT_CCIF));
         if (ESTAT & ESTAT_ERRORS)
         {
             EEWriteStatus = ESTAT & ESTAT_ERRORS;
             return;
         }
     }


    /* Never report a successful save solely because the command buffer emptied. */
    for (i = 0; i < ArraySize; ++i)
    {
        if (((volatile uint8_t *)tempAddr)[i] != (uint8_t)WriteData[i])
        {
            EEWriteStatus = 1;
            return;
        }
    }

}
