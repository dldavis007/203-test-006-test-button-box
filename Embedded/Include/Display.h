#ifndef DISPLAY_H
#define DISPLAY_H


#include "FixedTypes.h"
/* SPI transport, character memory and video rendering. */
void Xmit_Rcv_SPI(void);
void WriteCharMem(void);
void ReadCharMem(void);
void resetTitler(void);
void showMenu(void);
uint8_t DispTranslate(char *TranChar);
void DisplayMsg(char Message[]);
void showMessage(char Message[], int16_t row);
void ClearTitler(void);
void DisplayTitler(void);

#endif
