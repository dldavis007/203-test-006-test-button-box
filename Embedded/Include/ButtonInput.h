#ifndef BUTTON_INPUT_H
#define BUTTON_INPUT_H

/* Existing button/encoder globals remain in Subroutines.c in their old order. */
extern unsigned int TC0_RCVD_Data, tempData, Third_byte;
extern char Cntrl_Slow_Flag, internal_menu;

unsigned int EnabledFunctionData(unsigned int data);
void ScanButtons(unsigned int *releasedSwitches);
void EncodeButtonCommands(void);

#endif
