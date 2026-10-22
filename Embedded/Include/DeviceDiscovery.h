#ifndef DEVICE_DISCOVERY_H
#define DEVICE_DISCOVERY_H


#include "FixedTypes.h"
#include "ApplicationState.h"
#include "mco.h"

/* Existing definitions stay in Subroutines.c in their current order. */
extern bool AuxDeviceLimitExceeded, AuxDeviceMenuFull;

void ext_menu(void);
void Scan(void);
void menuSort(void);

/* Bridges use Subroutines.c's existing static CAN buffer by address. */
void ActivateDeviceMenus(CAN_MSG *message);
void ScanDevices(CAN_MSG *message);

#endif
