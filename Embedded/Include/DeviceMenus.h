#ifndef DEVICE_MENUS_H
#define DEVICE_MENUS_H

#include "ApplicationState.h"

/* Definitions remain in Subroutines.c in their existing order. */
extern int cam_list[10];
extern int cam_select;

int ExternalMenu(void);
int ExternalSubMenu(void);
int CameraMenu(void);
int Trigger_Device(void);

#endif
