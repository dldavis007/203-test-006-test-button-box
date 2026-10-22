#include "Flash.h"
#include "mc9s12a128.h"
#include "Subroutines.h"

//Caution Flash.c should be the first file in the files section under the project browser


void FlashInit ( void )
{
 	 FCLKDIV = (uint8_t)(OscClk * (5 + Tbus));
}

uint8_t FlashCmd[] = {0x7b,0x01,0x06,0xc6,0x80,0x7b,0x01,0x05,
	 			   0x1f,0x01,0x05,0x40,0xfb,0x3d};
//FlashCmd   7B0106         STAB    0106
//       	 C680           LDAB    #80
//       	 7B0105         STAB    0105
//       	 1F010540FB     BRCLR   0105 #40 1009
//       	 3D             RTS     




//  char TmpStr2[] = "Hello World!\r\n";
//  int *intptr;

//	intptr = (int*)0xc1f0;
//	FlashWrite ( sizeof (TmpStr2), TmpStr2, intptr );	


#ifdef BUTTONBOX_GCC
/* Called normally (JSR/RTS); this assembly wrapper calls the existing RAM code. */
extern void BBFlashCommand(uint8_t command);
#endif

/* Preserve both byte values when plain char is signed. */
uint16_t FlashWord(uint8_t high, uint8_t low)
{
    return ((uint16_t)high << 8) | (uint16_t)low;
}

void FlashWrite ( int16_t ArraySize, char WriteData[], int16_t *WriteAddr )
{
 	 //Erases and Writes an array of up to 512 char to Flash
	 
	 int16_t i, j;
     uint16_t TmpData = 0;
	 char *tempAddr;
	 char tempArray[1024];

     if (ArraySize <= 0 || ArraySize > 512)
     {
         return;
     }
	 
     
	 
	 //Add up to 511 more data bytes to beginning of array
	 //Addr must be divisible by 512 for erase operation
	 
	 tempAddr = (char*)WriteAddr;
     tempAddr = (char*)((int16_t)tempAddr & ~0x1ff);
	 
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

	 //pad end of array to make divisible by 512

     if ( (j & 0x1ff) )
     {
	 	tempAddr = (char*)WriteAddr;
     	tempAddr = (char*)((int16_t)tempAddr & ~0x1ff);
	  	for ( ;j & 0x1ff;j++ )
		{
         	tempArray[j] = *(tempAddr + j);
		}
     }

	 
	 //Erase Flash
	 
	 tempAddr = (char*)WriteAddr;
	 WriteAddr = (int16_t*)((int16_t)WriteAddr & ~0x1ff);

     for (i=0; i<j; i += 512, WriteAddr +=256 )
     {
		 *(volatile uint16_t *)WriteAddr = TmpData;
#ifdef BUTTONBOX_GCC
        BBFlashCommand(0x40);
#else
		 asm ("ldab #0x40");
		 asm ("jsr _FlashCmd");
#endif
//		 FCMD = 0x40;
//		 FSTAT = FSTAT_CCIF;
//		 while (!(FSTAT & FSTAT_CCIF) );
	 }
	 

	 //Write to Flash
	 
	 WriteAddr = (int16_t*)tempAddr;	 
	 WriteAddr = (int16_t*)((int16_t)WriteAddr & ~0x1ff);

     for (i=0;i<j;i+=2, WriteAddr++)
     {
	  	 TmpData = FlashWord(tempArray[i], tempArray[i+1]);
		 *(volatile uint16_t *)WriteAddr = TmpData;
#ifdef BUTTONBOX_GCC
        BBFlashCommand(0x20);
#else
		 asm ("ldab #0x20");
		 asm ("jsr _FlashCmd");
#endif
//		 FCMD = 0x20;
//		 FSTAT = FSTAT_CCIF;
//		 while (!(FSTAT & FSTAT_CCIF) );
     }


}
