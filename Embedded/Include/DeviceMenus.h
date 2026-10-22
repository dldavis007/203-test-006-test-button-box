#ifndef DEVICE_MENUS_H
#define DEVICE_MENUS_H


#include "FixedTypes.h"
#include "ApplicationState.h"

/* Definitions remain in Subroutines.c in their existing order. */
extern int16_t cam_list[10];
extern int16_t cam_select;

int16_t ExternalMenu(void);
int16_t ExternalSubMenu(void);
int16_t CameraMenu(void);
int16_t Trigger_Device(void);

#endif
