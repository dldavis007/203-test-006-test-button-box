#ifndef LEGACY_CAN_INTERFACE_H
#define LEGACY_CAN_INTERFACE_H

#include "mco.h"
#include "procimg.h"

/* Application declarations for existing legacy integration storage.
 * Keep stack definitions, capacities and Node-ID-zero behavior unchanged.
 * The response-ID array is intentionally unsized here: the legacy stack has
 * 12 entries while some application scans assume 20 (a separate bounds issue).
 */
extern UNSIGNED8 gProcImg[PROCIMG_SIZE];
extern int Device_Type_Response_ID[];
extern int Device_Type_Response_ID_ptr;
extern char Device_Menus[20][17];

#endif
