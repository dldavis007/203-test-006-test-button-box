# Application variable ownership review

This review accompanies the housekeeping pass after 61e4613. It identifies
ownership before extracting functions into modules. It does not change any
variable definition, qualifier, initializer, linkage, array dimension or order.
The initial housekeeping pass retained identical optimized host assembly.
The Settings.c, Display.c, Menu.c and DeviceMenus.c splits are separate compilation-unit changes that
require target memory checks; their moved bodies and existing storage remain intact.

## Current owners and intended boundaries

| Current definition owner | State | Consumers and intended responsibility |
| --- | --- | --- |
| Controller.c | wd_flag, wdInit, wdStarted | Startup/main-loop watchdog handshake with the timer ISR; declarations stay in Controller.h. |
| Interrupts.c | Gen_Flags, timers, SIN0Buf/SOUT0Buf and their indices | Interrupt-owned timing/serial services, used by the application through Interrupts.h. Preserve current qualifiers. |
| Interrupts.c | Seconds, Tick, TC0_State, TC0_BitCounter, QBit, TC0_Data_Bit, handler-local Sw_Index | Existing private ISR state. Keep its current linkage and initialization. |
| Subroutines.c | ButtonBoxData, AlternateFunctionMask, SPI0 buffers/indices, ReActivate_Timer, UpdateMenu, legacy_mode | Shared with interrupts; grouped declarations are in ApplicationState.h. |
| Subroutines.c | controller_mode, resequenceNodes, deviceSubMenu, sub_Device_Type_Response_ID, UpdateDisp | Also consumed by the legacy MCO integration. Preserve names, types, qualifiers and external linkage. |
| Subroutines.c | CharsInMem | Display-startup result read by Controller.c; declared in ApplicationState.h. |
| Subroutines.c | FirstVar, MessageRow, MessageCol, SerialNum, F1Enable, F2Enable, NullVar | Protected saved-variable sequence. Settings.c owns persistence/normalization; Settings.h exposes declarations. Menu tables hold pointers into the same arrays. |
| MenuTables.inc, included by Subroutines.c | defaultMenuc, Menuc, defaultMenuStackc, MenuStackc | Menu templates/runtime arrays. The include preserves their translation unit and definition order. Menuc is shared through ApplicationState.h. |
| Subroutines.c | StackPointer, Menu, VariableFlag, CursorDownFlag, CursorUpFlag, SelectFlag, increment, uLimit, lLimit, nDigits | Menu.c owns navigation/editing functions; storage stays in Subroutines.c. Menu.h exposes the required declarations. |
| Subroutines.c | TC0_RCVD_Data, tempData, Third_byte, Cntrl_Slow_Flag, MenuCheck, internal_menu, Trigger_DeviceFlag | Button decoding, alternate-mode handling and outgoing command assembly. Separate functions before changing visibility. |
| Subroutines.c | cam_list, extCamTagList, cam_select, vid18_addr, vid18_update, F3pressed, deviceSubMenu, auxiliary-limit flags | Discovery and camera shortcuts remain in Subroutines.c. DeviceMenus.c owns four external-menu/trigger commands; camera-state declarations are in DeviceMenus.h. |
| Subroutines.c | firstWriteSPI, goLow, plusSymbol, DispBuf/DispBufptr, message buffers/flags, MsgIDtable | Display.c now owns SPI/rendering functions; storage and bldMessages remain in Subroutines.c. The assembler retains its separate local static cursor. |
| Subroutines.c | BattNum, ptrDispBatt and battery display strings | Battery discovery/diagnostics and menu display. |
| EEProm.c | EEWriteStatus | EEPROM write/readback result exposed for diagnostics; preserve the debugger-visible symbol. |
| runtime.c, or ImageCraft startup | _bss_end[20] | Persistent startup marker. Preserve its GCC .noinit placement and existing startup ownership. |
| Legacy MCO files | CAN buffers/configuration, process image, response tables and filter state | Protected integration, not application cleanup. Application declarations remain local to Subroutines.c. |

The groups above describe responsibilities; they do not authorize moving
storage into those future modules. Settings/menus/display currently share
pointers, and moving definitions can affect linker packing and legacy saves.

## Unreferenced-storage candidates

These eight Subroutines.c globals have no identifier references beyond their
definitions in the current Embedded .c/.h/.inc/.S source tree:

- ExSlowPushed
- SlowPushed
- FastPushed
- LEDData
- Ext_Menu_xmitd
- STX_count
- XmitSPI_flag
- RcvSPI_flag

Their definitions are retained. A source reference search does not establish
whether external debugger workflows or legacy tooling expect their symbols.
Removing them would also change RAM placement. Any later removal should be a
separate change with target map/stack validation and hardware confirmation.

The unused local declarations of gTxSDO and gRxCAN were removed from
Subroutines.c. Their actual definitions and all MCO files are untouched.

## Extraction sequence

1. Settings/EEPROM functions have been extracted to Settings.c. Saved storage
   remains in Subroutines.c and explicit GCC EEPROM offsets are unchanged.
   Menu callbacks/pointers remain valid; the owner confirmed settings save/load
   on hardware at 855bfd6.
2. SPI/rendering functions are extracted to Display.c. Interrupt interfaces and
   original storage remain intact. CAN message assembly stays in Subroutines.c;
   normal menu rendering/navigation was confirmed on hardware at 651f2ee.
3. Internal-menu functions are extracted to Menu.c with settings/display calls
   and original storage intact. Host checks pass; target/hardware verification
   was confirmed by the owner, including a successful ImageCraft build.
4. Four camera/device menu commands are extracted to DeviceMenus.c. Target
   GCC/ImageCraft build and hardware verification are pending. Discovery and
   selection shortcuts remain in Subroutines.c; review its shared static CAN
   message buffer before extracting those functions. The MCO stack is unchanged.
5. Separate button scanning and command encoding from doevents.

For each real compilation-unit extraction, update both compiler project paths
as needed, run host regressions and a target build, compare memory use and
confirm hardware behavior. Identical host assembly alone does not establish
that the 9S12 compiler/linker produced an equivalent image.
