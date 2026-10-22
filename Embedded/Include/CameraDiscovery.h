#ifndef CAMERA_DISCOVERY_H
#define CAMERA_DISCOVERY_H

#include "ApplicationState.h"
#include "mco.h"

/* Existing definitions stay in Subroutines.c in their current order. */
extern char extCamTagList[10][11];
extern bool vid18_addr, vid18_update;

void scan_cameras(void);
void addressCamera(void);
int findCameras(void);
int duplicate_add(void);

/* Bridges receive Subroutines.c's existing static CAN buffer by address. */
void ScanCameraAddresses(CAN_MSG *message);
void RandomizeCameraAddresses(CAN_MSG *message);

#endif
