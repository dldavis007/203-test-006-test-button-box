#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

/* Board clock, peripheral initialization and pin assignments.
   Values and legacy alternate-clock notes are preserved from Subroutines.h. */

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

//SPI Control Register 1
#define SPI0CR1_Init 0xd0
//SPI Control Register 2
#define SPI0CR2_Init 0x00
//SPI Baud Rate Register (24MHz / 2 = 12MHz)                               
#define SPI0BR_Init 0x00                  


#define SPISR_SPTEF 0x20
#define SPICR1_SPTIE 0x20
#define SPISR_SPIF 0X80

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
