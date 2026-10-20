/**************************************************************************
MODULE:    PROCIMG
CONTAINS:  Process Image Configuration
COPYRIGHT: Embedded Systems Academy, Inc. 2002-2005.
           All rights reserved. www.microcanopen.com
           This software was written in accordance to the guidelines at
           www.esacademy.com/software/softwarestyleguide.pdf
DISCLAIM:  Read and understand our disclaimer before using this code!
           www.esacademy.com/disclaim.htm
LICENSE:   THIS IS THE EDUCATIONAL VERSION OF MICROCANOPEN
           See file license_educational.txt or
           www.microcanopen.com/license_educational.txt
           A commercial MicroCANopen license is available at
           www.CANopenStore.com
VERSION:   2.10, ESA 12-JAN-05
           $LastChangedDate: 2005-01-12 13:53:59 -0700 (Wed, 12 Jan 2005) $
           $LastChangedRevision: 48 $
***************************************************************************/ 

#ifndef _PROCIMG_H
#define _PROCIMG_H


/**************************************************************************
DEFINES: Definition of the process image
Modify these for your application
**************************************************************************/

// Define the size of the process image
#define PROCIMG_SIZE 58

// Define process variables: offsets into the process image 
// Button Box Data
#define IN_digi_0 0
// Button Box Data
#define IN_digi_1 1
// Button Box Data
#define IN_digi_2 2

// Camera
#define IN_digi_3 3
#define IN_digi_4 4

// Digital Inputs
#define IN_digi_5 5
#define IN_digi_6 6
#define IN_digi_7 7
#define IN_digi_8 8
#define IN_digi_9 9
#define IN_digi_10 10
#define IN_digi_11 11
#define IN_digi_12 12
#define IN_digi_13 13 
#define IN_digi_14 14
#define IN_digi_15 15
#define IN_digi_16 16
#define IN_digi_17 17
#define IN_digi_18 18
#define IN_digi_19 19
#define IN_digi_20 20

// Analog Inputs
#define IN_ana_1 21
#define IN_ana_2 23

// Digital Outputs
#define OUT_digi_0 25
#define OUT_digi_1 26
#define OUT_digi_2 27
#define OUT_digi_3 28
#define OUT_digi_4 29
#define OUT_digi_5 30
#define OUT_digi_6 31
#define OUT_digi_7 32
#define OUT_digi_8 33
#define OUT_digi_9 34
#define OUT_digi_10 35
#define OUT_digi_11 36
#define OUT_digi_12 37
#define OUT_digi_13 38
#define OUT_digi_14 39
#define OUT_digi_15 40
#define OUT_digi_16 41
#define OUT_digi_17 42
#define OUT_digi_18 43
#define OUT_digi_19 44
#define OUT_digi_20 45
#define OUT_digi_21 46
#define OUT_digi_22 47
#define OUT_digi_23 48
#define OUT_digi_24 49
#define OUT_digi_25 50
#define OUT_digi_26 51
#define OUT_digi_27 52
#define OUT_digi_28 53

// Analog Outputs
#define OUT_ana_0 54
#define OUT_ana_1 56


/* Process image storage is defined once in user.c. */
extern unsigned char gProcImg[PROCIMG_SIZE];

#endif
