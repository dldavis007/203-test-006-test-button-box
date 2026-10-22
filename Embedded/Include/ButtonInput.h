#ifndef BUTTON_INPUT_H
#define BUTTON_INPUT_H


#include "FixedTypes.h"
/* Existing button/encoder globals remain in Subroutines.c in their old order. */
extern uint16_t TC0_RCVD_Data, tempData, Third_byte;
extern uint8_t Cntrl_Slow_Flag, internal_menu;

uint16_t EnabledFunctionData(uint16_t data);
void ScanButtons(uint16_t *releasedSwitches);
void EncodeButtonCommands(void);

#endif
