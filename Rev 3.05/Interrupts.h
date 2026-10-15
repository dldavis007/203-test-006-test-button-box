#ifndef Interrupts_H
#define Interrupts_H

#pragma interrupt_handler DUMMY_ENTRY
#pragma interrupt_handler SCI0_Int_Handler
#pragma interrupt_handler SCI1_Int_Handler
#pragma interrupt_handler RTI_Int_Handler
#pragma interrupt_handler IRQ_Int_Handler
#pragma interrupt_handler TC0_Int_Handler
#pragma interrupt_handler TC3_Int_Handler
#pragma interrupt_handler SPI0_Int_Handler
#pragma interrupt_handler CANRxISR

#define TC_50us (int)(50/Tbus/TSCR2_PreScale+0.5)
#define TC_104us (int)(104/Tbus/TSCR2_PreScale+0.5)
#define TC_100us (int)(100/Tbus/TSCR2_PreScale+0.5)
#define TC_170us (int)(170/Tbus/TSCR2_PreScale+0.5)
#define TC_208us (int)(208/Tbus/TSCR2_PreScale+0.5)
#define TC_250us (int)(250/Tbus/TSCR2_PreScale+0.5)
#define TC_340us (int)(340/Tbus/TSCR2_PreScale+0.5)
#define TC_416us (int)(416/Tbus/TSCR2_PreScale+0.5)
#define TC_500us (int)(500/Tbus/TSCR2_PreScale+0.5)
#define TC_1200us (int)(1200/Tbus/TSCR2_PreScale+0.5)
#define TC_1ms (int)(1000/Tbus/TSCR2_PreScale+0.5)
#define TC_2ms (int)(2000/Tbus/TSCR2_PreScale+0.5)
#define TC_3ms (int)(3000/Tbus/TSCR2_PreScale+0.5)
#define TC_25ms (int)(25000/Tbus/TSCR2_PreScale+0.5)
#define TC_5ms (int)(5000/Tbus/TSCR2_PreScale+0.5)


#define TSCR2_PreScale 32



#define TIOS_Init 0b00001001
#define TSCR1_Init 0b10000000
#define TCTL1_Init 0b00000000
#define TCTL2_Init 0b00000000
#define TCTL3_Init 0b00000000
#define TCTL4_Init 0b00000000
#define TIE_Init 0b00001001
#define TSCR2_Init 0x05





#define RTI_Div_Rate 8192
#define RTI_One_Sec (OscClk*1000000/RTI_Div_Rate)


#define Gen_Flags_SCI1Xmtng 0x01
#define Gen_Flags_SIN0Rcvd 0x02
#define Gen_Flags_SIN1Rcvd 0x04
#define Gen_Flags_Timer1 0x08
#define Gen_Flags_Timer2 0x10
#define Gen_Flags_No2Wire 0x20
#define Gen_Flags_Menu_Active 0x40
#define Gen_Flags_Extern_Menu 0x80


#define SCI0CR1_PE 0x02
#define SCI0CR1_PT 0x01
#define SCI0CR1_M 0x10
#define SCI0CR2_TE 0x08
#define SCI0CR2_RE 0x04
#define SCI0SR1_RDRF 0x20
#define SCI0SR1_TC 0x40
#define SCI0SR1_TDRE 0x80
#define SCI0CR2_RIE 0x20
#define SCI0CR2_TCIE 0x40
#define SCI0CR2_TIE 0x80

#define TIE_C0I 0x01
#define TIE_C1I 0x02
#define TIE_C2I 0x04
#define TIE_C3I 0x08
#define TIE_C4I 0x10
#define TIE_C5I 0x20
#define TIE_C6I 0x40
#define TIE_C7I 0x80

#define SIN0BufLen 150
#define SOUT0BufLen 80
#define SIN1BufLen 80
#define SOUT1BufLen 150
#define PrintBufLen 80

#define MenuTime RTI_One_Sec/4

#define TeleData_Rev     0x8000
#define TeleData_Fwd     0x4000
#define TeleData_ExSlow  0x2000
#define TeleData_Slow    0x1000
#define TeleData_Fast    0x0800
#define TeleData_Data    0x0400
#define TeleData_Reset   0x0200
#define TeleData_Trig    0x0100
#define TeleData_F1      0x0080
#define TeleData_F2      0x0040
#define TeleData_F3      0x0020
#define TeleData_RotCCW  0x0010
#define TeleData_RotCW   0x0008
#define TeleData_Select  0x0004
#define TeleData_Up      0x0002
#define TeleData_Down    0x0001

#define DeBounceTime  RTI_One_Sec * .05;
#define HW_Watchdog_Time  RTI_One_Sec * 0.5

#endif