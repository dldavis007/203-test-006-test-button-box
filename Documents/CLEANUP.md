# Button-box cleanup

## Current baseline and scope

The current baseline is bdc732a, the camera-discovery extraction. The owner
reported operation after reloading the ImageCraft firmware; an apparent stop
around ReadCharMem was not reproduced after that reload and its cause remains
unconfirmed. This pass extracts device discovery while preserving the static
CAN buffer and existing public entry points. The owner's relative ImageCraft
paths and blank build-folder setting remain intact. The Node-ID-zero startup
regression and recovery are documented in STARTUP_NODE_ZERO.md.

The MCO integration is protected from general cleanup. mco.c, mcohw.c, user.c,
mco.h, mcohw.h, nodecfg.h and procimg.h match hardware-verified 65ced39, except
for the explicitly requested battery-only PDO removal in user.c described below. A test
checks their baseline content, allowing Windows line endings. A demonstrated
MCO defect or substantial advantage must be handled as a separate change;
removing a warning alone is not grounds to change its behavior. Node-ID zero
is intentional for this application and the C48 controller.

Preserve the EEPROM record format and offsets, variable storage order, startup
sequence, static PPAGE 0x3D, CAN identifiers and 1280-byte minimum stack reserve.
F1/F2 OFF continues to latch/transmit each function bit and light its own LED;
it disables the alternate navigation mapping and cross-pattern illumination.

## Shared-declaration header pass

ApplicationTypes.h now owns the existing bool, MatrixTbl, MenuStruct and
MenuStack definitions. ApplicationState.h groups shared declarations by button
data, SPI exchange and menu/mode state. Subroutines.h includes that header and
continues to expose the application functions and hardware constants.
Controller.h retains its initialization functions and watchdog declarations.
ReadCharMem and WriteCharMem are declared together in Subroutines.h.

Only headers and this document changed in this pass. No variable definitions
moved, no types/qualifiers/member order changed, and no source functions changed.
The new headers compile independently. All 20 host unit tests, both F1/F2
regression char modes and source syntax checks pass. Optimized host assembly
for all 11 C files matches the b79ca05 source snapshot after ignoring source-path
annotations. This is additional host verification, not a target compiler or
hardware result. The owner subsequently confirmed this pass working on hardware
at commit 8e46696.

## Hardware-configuration header pass

HardwareConfig.h now holds the board clock, GPIO initialization, ADC, SPI, PWM,
interrupt configuration values and display/alarm/watchdog pin assignments that
were mixed into Subroutines.h. The legacy alternate-clock notes are preserved.
Subroutines.h includes the new header for existing callers, while retaining
application constants such as menu sizes, display-buffer length and message flags.

Preprocessor macro definitions match 8e46696 exactly except for the new include
guard. Optimized host assembly for all 11 C sources also matches after ignoring
source annotations. All 20 unit tests, both F1/F2 regression char modes and source
syntax checks pass. No .c definitions, MCO files, EEPROM code, startup assembly,
linker configuration or build tools changed. The owner subsequently confirmed
this pass working on hardware at commit 21c7ebc.

## Application interface readability pass

Subroutines.h now groups its 51 existing function declarations by application
loop, saved settings, internal menu, device/camera control, battery diagnostics
and video display. Prototype spacing is consistent. All signatures and names
are preserved; the existing application constants are unchanged.

Subroutines.h is the only firmware file changed in this pass. Optimized host
assembly for all 11 C files matches 21c7ebc after ignoring source annotations.
All 20 unit tests, both F1/F2 regression char modes and source syntax checks pass.
No runtime implementation or storage declaration changed. The owner
subsequently confirmed this pass working on hardware at commit 6877736.

## Character-font data extraction

The 347-line CMDI definition moved from Subroutines.c to CharacterFontData.inc
in Embedded/Src. Subroutines.c includes it at the exact former definition
location. This is a data include, not another compilation unit. Symbol name,
const char type, initializer bytes and definition order are preserved. Existing
GCC and ImageCraft source lists require no extra compilation step.

Replacing the include with its original table text reconstructs the previous
Subroutines.c exactly. Optimized host assembly, including data, for all 11 C files
also matches 6877736 after ignoring source annotations. All 20 unit tests, both
F1/F2 regression char modes and source syntax checks pass. MCO, EEPROM layout,
startup and the stack reserve are unchanged. The owner subsequently confirmed
this pass working on hardware at commit 1f66f89.

## Menu and keypad table extraction

MenuTables.inc holds the default menu templates and runtime menu arrays in
exactly their previous order. KeypadMatrix.inc holds the 18-entry button/port/LED
mapping. Both are included by Subroutines.c at their original definition
locations. They are not compiled as separate source files. EEPROM variables
remain in Subroutines.c in their original order.

Expanding these two data includes reconstructs the previous Subroutines.c
exactly. All types, array dimensions, contents, qualifiers and symbol scope are
preserved. The regression harness reads the real extracted tables before
selecting its test fragments, retaining the existing F1/F2 menu and scan checks.

Optimized host assembly, including data, for all 11 C files matches 1f66f89 after
ignoring source annotations. All 20 unit tests, both F1/F2 regression char modes
and source syntax checks pass. MCO files, startup, build tools, EEPROM layout
and the 1280-byte stack reserve are unchanged. The target build/hardware check
for this pass is pending.

## Housekeeping and ownership review

After 61e4613, the application sources received blank-line cleanup, removal of
the duplicate string.h include and two unused local extern declarations, and
replacement/removal of stale comments. Comments identify the existing state
groups without reordering their definitions. The protected EEPROM declaration
sequence is byte-for-byte unchanged. All seven MCO baseline files remain intact.

VARIABLE_OWNERSHIP.md records current owners, shared interrupt/MCO dependencies,
future module boundaries and eight unreferenced-storage candidates. Those
candidates retain their definitions and RAM placement in this pass; their
removal is a separate decision from this ownership review. No new volatile or
static qualifiers were introduced.

Optimized host assembly for all 11 C files matches the pre-pass 61e4613 snapshot
after ignoring source annotations. All 20 unit tests, both F1/F2 regression
char modes and source syntax checks pass.
The latest table extraction and this pass still require target/hardware checks;
1f66f89 remains the last explicitly hardware-confirmed revision in this record.

## Settings compilation-unit extraction

Settings.c and Settings.h now contain the persistence interface and seven
unchanged function bodies: NormalizeFunctionSettings, EncodeLegacySettings,
DecodeLegacySettings, Load_Variables, Save_Variables, Load_Serial_Num and
Save_Serial_Num. Settings.h declares the saved variables, whose actual definitions
remain together in Subroutines.c in their original order. Menu callbacks and
button-mode selection remain in Subroutines.c.

Both the GCC source list and ImageCraft ButtonBox.src include Settings.c. The
host syntax checker also includes it. The F1/F2 regression now compiles and links
the real Settings.c instead of extracting copies of its codec/normalization
functions. It exercises RAM buffers; physical EEPROM load/save paths are not
executed on the host. The harness rejects unexpected EEPROM writes. A native
GCC page-zero warning for the unused 0x0800 hardware-load path is disabled only
in that host test build; production compiler warning flags are unchanged.

All 20 unit tests and both F1/F2 regression char modes pass. GCC application
syntax is checked with warnings as errors, and the new module's ImageCraft
conditional path also parses under host GNU89. These are not target builds.
The seven moved function bodies and saved-variable declaration block compare
exactly with c263b2a. The host Subroutines object preserves mutable data/BSS
section order, sizes and initialized bytes. Settings.c adds no mutable global
or static storage. All MCO files, the EEPROM driver, startup assembly and target
configuration are unchanged; the minimum stack reserve remains 1280 bytes.

Unlike the preceding textual data includes, this is a separate compilation
unit. Compiler optimization, code placement and the final target map may change.
The target memory validator is unchanged and must pass a fresh 9S12 build.
The owner subsequently confirmed settings save/load working on hardware at
855bfd6. ImageCraft compilation is explicitly deferred to the final cleanup
verification. Its source list will remain updated in the meantime. The GCC
record remains 28 bytes at 0x0800; the ImageCraft contiguous save remains 128
bytes; the separate serial-number address remains 0x0902.

## Display/SPI compilation-unit extraction

Ten unchanged functions now live in Display.c, exposed through Display.h:
Xmit_Rcv_SPI, WriteCharMem, ReadCharMem, resetTitler, showMenu, DispTranslate,
DisplayMsg, showMessage, ClearTitler and DisplayTitler. Subroutines.h includes
Display.h for existing callers. Both compiler source lists and the host syntax
checker include the new module.

Definitions of all SPI buffers/indices, display buffers, flags, character data
and startup state remain in Subroutines.c and its data includes. ApplicationState.h
adds matching extern declarations for the renderer. SPI0_Int_Handler is unchanged.
CAN message assembly (bldMessages) remains in Subroutines.c with its existing
function-local static cursor. This avoids moving that persistent storage along
with the renderer. No storage definitions or qualifiers were changed.

All ten moved function bodies compare exactly with 855bfd6. The host Subroutines
object retains mutable data/BSS section order, sizes and initialized contents;
Display.c adds no mutable global/static storage. All 21 unit tests, both F1/F2
regression char modes and source syntax checks pass. The new native test links
the real Display.c and actual glyph table, checking digits, alphabet, punctuation,
control/custom glyphs and extended-byte passthrough in both char modes. Physical
SPI and display routines are linked but not executed by the host test.

MCO files, the EEPROM driver/settings implementation, interrupt source, startup,
font/menu/keypad data and target configuration are unchanged. The required stack
reserve is still 1280 bytes. A new target build/map validation and hardware check
are required because separate compilation can change code placement and size.
Check startup, readable menu/message text, menu navigation, F1/F2 LEDs and modes,
and saved settings. ImageCraft build verification remains deferred by agreement.

## Internal-menu function extraction

Menu.c now owns 19 existing functions for menu loading, cursor movement, numeric
editing, selection, F1/F2 setting callbacks, menu exit and restoring defaults.
Menu.h declares their interface and existing menu constants. Function bodies
are unchanged. The GCC and ImageCraft source lists include the new module.

All menu arrays, cursor/editing state and saved variables remain defined in
Subroutines.c and MenuTables.inc in their original order. Host object comparison
confirms mutable data/BSS section order, sizes and initialized bytes are unchanged.
The const menu template has identical bytes and symbolic pointer relocations;
its host section name changes because callbacks are now external to that source.
Menu.c adds no mutable global/static storage.

All 22 unit tests, both F1/F2 regression char modes and application syntax checks
pass. The new test links the actual Menu.c and menu tables, covering navigation,
numeric wrapping, F1/F2 callbacks, serial-save requests, submenu exit, full-menu
save and restore-default requests. Persistence and display calls are stubbed in
this test; physical EEPROM, SPI and target execution still need hardware checks.

MCO files, Settings.c, Display.c, interrupts, startup, EEPROM variable declarations,
menu/keypad/font data and target linker configuration are unchanged. The minimum
stack reserve remains 1280 bytes. Target build/map validation and hardware testing
are pending for this extraction. Check menu entry, cursor movement, numeric
editing, F1/F2 ON/OFF, full exit and saved values after restart. Actual ImageCraft
compiler verification remains deferred by agreement.

## ImageCraft verification and obsolete project removal

The owner confirmed the menu extraction at 54ad773 working on hardware, then
built/linked all 12 application sources using ImageCraft 7.08 and reported that
it operates. The linker reported "Device 90% full". Detailed ImageCraft memory
and complete functional verification were not supplied.

Use Embedded/ImageCraft/BUTTONBOX.prj, leave its build-folder setting blank, and
use ..\Include;C:\iccv712\include\ as the include paths. A separate output
folder caused the IDE to generate incorrect source paths. Keep crt12initrm.o;
runtime.c, compact_io.c and startup.S are GCC-only and excluded from ImageCraft.

The root Legacy folder, containing a duplicate old IDE project and imported
build outputs, was removed because it led to opening the wrong project.
Those files remain recoverable from Git history. Documents/Legacy retains the
original reference text used by the active project. Generated ImageCraft outputs
beside the active project are already ignored. No firmware source, startup,
EEPROM format, MCO integration or memory configuration changed in this removal.

## External-device menu command extraction

DeviceMenus.c now owns ExternalMenu, ExternalSubMenu, CameraMenu and
Trigger_Device. The four function bodies are copied unchanged from 7915b16.
DeviceMenus.h exposes their existing signatures and the required camera-state
externs. All camera arrays, indices and other state remain in Subroutines.c in
exactly their previous order. Both GCC and ImageCraft source lists include the
new module; the owner's working ImageCraft project options are unchanged.

Discovery functions remain in Subroutines.c because scan_cameras, addressCamera,
Scan and ext_menu share its existing static CAN message buffer with other
application code. Moving these requires a separate decision about that interface;
this pass does not relocate the buffer or change its linkage. Camera-selection
shortcuts in doevents also remain intact.

All 23 host unit tests, both F1/F2 regression char modes and strict application
syntax checks pass. The new test links actual DeviceMenus.c, captures transmitted
CAN IDs/lengths/payload bytes, and checks selected-camera process-image bytes,
menu flags, reactivation timing, title clearing, deselection and transmit-error
notifications in both char modes. It stubs physical CAN/display and fatal-error
handling; it does not validate bus timing or legacy recovery behavior.

The remaining Subroutines.c text is exact after deleting the moved block. Host
objects retain data/BSS section order, sizes, initialized bytes and symbolic
pointer relocations. DeviceMenus.c adds no mutable global/static storage. MCO,
EEPROM layout/settings, startup, interrupt code, existing menu/display modules,
constant tables, target memory configuration and the 1280-byte stack requirement
are unchanged. Separate compilation can change target code size and placement.

Target GCC/NoICE and ImageCraft builds/hardware checks are pending for this pass.
Check camera selection, camera-menu entry, connected-device main and auxiliary
menus, trigger commands, return to the internal menu and F1/F2 modes/settings.

## Battery diagnostic function extraction

BatteryDiagnostics.c now owns Scan_Batteries, ClrBattErr and NextBatt. Their
bodies are unchanged from 6754a2a. BatteryDiagnostics.h declares the existing
functions and battery display/state variables. All storage remains defined in
Subroutines.c in its previous order, including the battery menu pointers. GCC
and ImageCraft source lists include the new module; ImageCraft project options
are unchanged.

All 24 host unit tests, both F1/F2 regression char modes and strict application
syntax checks pass. The new test links the actual battery module and compact
formatter, supplies scripted CAN process-image replies and replaces asynchronous
timeouts in the stack stub. It checks battery selection and wrapping, stale
selection fallback, serial/status/temperature formatting, alarm text, clear-error
requests, menu refresh and the existing three RPDO changes/filter handling.
It runs in both char modes. No live CAN stack, bus timing or battery hardware
is exercised by the host test, and no CAN stack code changes.

The remaining Subroutines.c is exact after removing the moved function block.
Host data/BSS section order, sizes, initialized bytes and symbolic pointer
relocations match the previous baseline. BatteryDiagnostics.c adds no mutable
global/static storage. Existing EEPROM records and declarations, MCO integration,
startup, interrupts, prior menu/display modules, constant tables, linker settings
and the 1280-byte stack requirement are unchanged.

GCC/NoICE and ImageCraft target build/hardware verification are pending for this
pass. Check startup, ordinary buttons and menus, plus battery status, NEXT and
CLEAR ERROR with battery hardware available. Target code size/placement can
change with separate compilation, so retain the target memory validation.

## Battery feature retirement (supersedes its extraction)

At the owner's request, remove the unused battery feature completely from the
active application: BatteryDiagnostics.c/.h, its host diagnostic test, the
controller-mode startup scan, seven battery-only arrays/variables, declarations,
STATUS rows and callbacks, and both compiler source lists. STATUS retains
SOFTWARE REV, SERIAL NUM and EXIT. Its menu index and table dimensions remain
unchanged, as do the other menus.

The only intentional change to the protected application CAN integration is
removing four battery-only registrations from user.c:

| Registration | CAN ID | Former use |
| --- | --- | --- |
| RPDO 5 | 0x024 | Battery serial replies |
| RPDO 6 | 0x007 | Battery status replies |
| TPDO 3 | 0x110 | Battery supervisor request |
| TPDO 4 | 0x120 | Battery-specific commands |

Keep RPDO 7 / 0x500: camera discovery uses its OUT_digi_13 buffer too. Remaining
PDO numbers, offsets, process-image size and capacity constants stay unchanged;
unused slots are not repurposed. No core mco.c/mcohw.c changes, Node-ID-zero
changes or unrelated protocol corrections are included. The baseline guard pins
the narrowly changed user.c explicitly; the other six integration files still
match 65ced39.

All 24 host unit tests, both F1/F2 regression char modes and strict application
syntax checks pass. The actual menu test verifies the shortened STATUS menu,
serial display/edit behavior, EXIT and cursor wrapping. A new native test runs
the actual ResetCommunication body against registration stubs, asserting the
five retained receive PDOs and two retained transmit PDOs, Node-ID zero, cold
process-image clearing and warm-reset preservation. The removed battery test
has been replaced rather than counted as continued coverage of dead code.

EEPROM saved-variable declarations, settings codecs, startup assembly, interrupt
source, working ImageCraft options and target memory configuration are unchanged.
Only the seven retired battery globals disappear from host data/BSS (125 bytes);
remaining data/BSS sections keep their order, sizes and initialized bytes, except
the intentionally updated STATUS menu template. Absolute RAM addresses may move;
target memory validation still requires at least 1280 stack bytes.

GCC/NoICE and ImageCraft target builds and hardware checks are pending. Check
controller-mode startup, camera discovery/selection (including HD camera tags),
external menus/triggers, F1/F2 operation, shortened STATUS menu, serial number and
saved settings after restart. Battery menu entries/requests must be absent.
Historical battery extraction notes above describe the intermediate commit,
not the current feature set.

## Button scan and command encoding extraction

ButtonInput.c now owns ScanButtons and EncodeButtonCommands. doevents retains
the original debounce gate and its function-local static SwitchUpData in the
same scope and order, passes its address to ScanButtons, then calls the encoder
every loop, including when debounce skips scanning. No persistent state moves
into the new module. ButtonInput.h declares these interfaces, the existing
EnabledFunctionData helper and required application globals. Both compiler
source lists include the module; working ImageCraft options are unchanged.

The scanner expressions and operation order are retained. Its only mechanical
changes are one indentation level and replacing SwitchUpData references with
parenthesized dereferences of the supplied state pointer. The encoder body is
unchanged. Reset handling, camera selection, internal-menu handling and the rest
of doevents remain in their prior order. Repeated F1/F2 expressions and other
logic simplification are deliberately left for a later hardware-verified pass.

All 24 host unit tests, both F1/F2 regression char modes and strict application
syntax checks pass. The existing regression harness now compiles the actual
new function bodies and actual debounce call-site fragment, replacing only
physical keypad input and the asynchronous scan delay as before. It retains
settings, latching/release, navigation, travel/stop, controller Slow, menu routing,
LED and EEPROM coverage. New assertions show debounce suppresses scanning while
third-byte encoding continues on each loop.

Host objects retain data/BSS section order, sizes, initialized bytes and symbolic
pointer relocations, including the release-history static in doevents.
ButtonInput.c adds no mutable global/static storage. Saved declarations/codecs,
MCO integration, startup, interrupts, existing modules, constant tables, ImageCraft
options and target memory settings are unchanged. The 1280-byte stack reserve
remains enforced; new function calls can affect target code placement and stack
use, so target build and hardware verification remain necessary.

GCC/NoICE and ImageCraft target checks are pending. Check key press/hold/release,
F1/F2 toggling and OFF behavior, alternate navigation and cross LEDs, travel/stop,
controller forward/reverse/Slow, internal/external menus, camera selection,
reset and saved settings after restart.

## Button latch-mask factoring

ButtonInput.c now names the existing expressions with two private helpers:
SlowToggleMask returns the Slow bit only in ordinary mode, and LatchedButtonMask
combines the fixed latched keys with that conditional bit. The repeated full
mask at scan start and toggle classification calls the second helper; the
no-travel speed clear calls the first. These expressions are still evaluated
at each original location using current SwitchData. Do not cache one mask at
scan start: pressing a function key earlier in the matrix can change how Slow
is handled later in the same scan. Raw function bits continue to latch even
when their alternate mode is OFF.

All 24 host unit tests, both signed/unsigned-char regressions and strict
application syntax checks pass. The regression also exhaustively compares both
helpers with the previous expressions across all 65536 input values and four
F1/F2 settings combinations. Existing press/release, command encoding, controller
Slow, travel/stop, menus, LEDs, debounce and EEPROM cases remain covered.

Only ButtonInput.c changes among Embedded files. Replacing its three helper
call sites with their original expressions reconstructs the previous source
exactly. Host data/BSS contents, order and pointer relocations are unchanged;
the module still adds no mutable storage. EEPROM, MCO, startup, interrupts,
project lists/options and the 1280-byte stack requirement are untouched.
Target GCC/ImageCraft build and hardware checks remain pending because compiler
inlining, code placement and call overhead can differ.

When updating locally, save and close ImageCraft before pulling. Its in-memory
project source list is written back on exit and can overwrite a freshly pulled
ButtonBox.src. Reopen Embedded/ImageCraft/BUTTONBOX.prj after the pull. If an
update was pulled while the IDE was open, preserve the updated source list
before closing and restore it afterward, then reopen the project.

## Camera discovery extraction

CameraDiscovery.c owns the camera address scan, address randomization,
findCameras and duplicate_add. The existing scan_cameras and addressCamera
entry points remain as small wrappers in Subroutines.c, passing its original
static gTxMsg to ScanCameraAddresses and RandomizeCameraAddresses. The buffer
keeps its storage, linkage and initialization, and is still shared with device
discovery/reset commands. The moved scan/randomization bodies differ only in
their names/parameter and field access through that pointer. findCameras and
duplicate_add are copied unchanged. All camera arrays, flags and indices stay
in Subroutines.c at their original definition locations. New externs/prototypes
are in CameraDiscovery.h. Device Scan/menuSort/ext_menu and camera-selection
shortcuts in doevents remain for later cleanup.

GCC and ImageCraft source lists include CameraDiscovery.c. The owner's 8860562
ImageCraft project options remain byte-identical: relative paths, blank build
folder and outputs beside Embedded/ImageCraft. Close ImageCraft before pulling
this change, then reopen the project so its source list includes the module.

All 25 host unit tests, both F1/F2 regression char modes and strict application
syntax checks pass. The new test uses actual discovery code, real menu templates
and wrapper bodies with scripted stack replies. It covers ordinary SD and HD
addresses, HD tag assembly, camera menu callbacks/EXIT, normal randomization,
HD refresh skipping randomization, empty discovery, duplicate addresses and
transmit failure notifications. Only the interrupt-only randomization delay is
replaced in the host copy; production waits are unchanged. Live CAN traffic,
reply timing and target integer/pointer widths require target verification.

Host data/BSS section order, sizes, initialized bytes and symbolic pointer
relocations match 8860562; the new module allocates no mutable global/static
storage. MCO, EEPROM definitions/codecs, startup, interrupts, existing modules,
constant tables, working project options, linker settings and the 1280-byte
stack requirement are unchanged. Additional function calls can affect target
code placement and stack use, so GCC/NoICE and ImageCraft builds and hardware
checks are pending. Exercise startup discovery, FIND CAMERAS, SD/HD tags,
camera selection and camera menus, plus ordinary F1/F2 and saved settings.

Existing discovery edge cases are deliberately not repaired in this extraction:
the unused-address clear loop stops before index 9, the maximum camera count
can exceed available menu rows once FIND/EXIT/header are included, and tag
trimming uses overlapping strncpy. The new fixtures cover normal mixed-camera
and freshly empty scans, not those boundary cases. Review fixes separately.

## Device discovery extraction

DeviceDiscovery.c now owns the device scan, auxiliary menu sorting and external
menu activation loop. Public Scan and ext_menu remain small wrappers in
Subroutines.c and pass its original static gTxMsg to ScanDevices and
ActivateDeviceMenus. The moved bodies differ only in the names/parameter and
buffer access; menuSort is copied unchanged. All response handling, SDO requests,
duplicate-node randomization/resequencing, callback assignment and final
SDO_restore retain their original order. Three existing extern declarations for
legacy response state move with the functions. Application-owned device arrays,
flags and the shared CAN buffer remain in their existing storage order.

DeviceDiscovery.h exposes the interfaces and existing auxiliary-state externs.
Both compiler source lists include the new module. The ImageCraft project
options are byte-identical to bdc732a. Save and close the IDE before pulling,
then reopen Embedded/ImageCraft/BUTTONBOX.prj so it loads the new source list.

All 26 host unit tests, both signed/unsigned-char F1/F2 regression modes and
strict application syntax checks pass. The new test uses actual discovery
functions, wrapper bodies and menu templates, with stack/watchdog boundaries
stubbed and interrupt-only delays replaced in a host copy. It checks all 127
external-menu activation destinations, transmit-failure notification, the 112
initial discovery requests, empty scans, primary/auxiliary menu population,
adjacent auxiliary sorting and the existing duplicate-node randomization,
resequence and response-table broadcast payloads. It verifies the caller-owned
CAN buffer is used throughout. Physical CAN timing and stack integration are
not exercised. Fixtures isolate the application's existing 20-slot scan logic;
they do not validate the separately documented legacy response-array capacity.
The existing primary-menu capacity issue is also unchanged.

Host data/BSS section order, sizes, initialized bytes and symbolic pointer
relocations match bdc732a. DeviceDiscovery.c adds no mutable global/static
storage. Core MCO files and application PDO setup, EEPROM declarations/codecs,
startup, interrupts, display/SPI functions including ReadCharMem, other extracted
modules, constant tables, project options and target memory settings are unchanged.
The 1280-byte stack requirement remains enforced. Additional calls can change
target code placement and stack use, so GCC/NoICE and ImageCraft builds and
hardware checks are pending. Check controller-mode startup/device discovery,
primary and auxiliary device menu entries, their external menu/trigger commands,
camera discovery/selection, F1/F2 and saved settings.

## Active internal-menu key handling

The device-discovery extraction in 01aea39 was confirmed working in hardware.
The next pass gives active-menu key handling a private
ServiceInternalMenuKeys() helper in Subroutines.c. Its original guarded call
position in doevents(), flag types, repeat behavior, timer assignments, menu
refresh behavior and callback order are preserved. Camera shortcuts, menu entry,
reset handling and CAN behavior are unchanged. No source-list updates are needed.

All 27 host tests pass. The new key-handling test covers all 16 input combinations,
timer gating, repeats, release rearming of signed navigation flags, callback
order and cursor suppression while editing. Both signed/unsigned-char F1/F2
regressions and strict application syntax checks pass. Host data/BSS section
order, sizes, initialized bytes and symbolic relocations match the prior source.
No globals, EEPROM declarations, startup, MCO files or memory limits changed;
the build still enforces the 1280-byte stack reserve. Target code placement and
call overhead require GCC/NoICE and ImageCraft hardware verification. Check menu
navigation, select delay, held-key repeats, device triggers from the menu,
variable editing, exit/save, camera shortcuts and all F1/F2 setting combinations.

## Internal-menu entry handling

The active-menu key extraction in f3f1752 was confirmed working in hardware.
This pass moves only the internal-menu entry statements to a private
EnterInternalMenu() helper in Subroutines.c. The existing controller/F1/F2 and
pending-entry guards remain in doevents(), including the else-if relationship
with camera shortcuts. Initialization, rendering, input clearing and flag reset
retain their original order. No globals, headers or source lists change.

All 28 host tests pass, along with both signed/unsigned-char F1/F2 regressions
and strict application syntax checks. The new test checks entry across all
65,536 input masks, display callback ordering, unrelated input/flag preservation,
and untouched non-root stack entries. Host data/BSS section order, sizes,
initialized bytes and symbolic relocations match f3f1752. EEPROM layout, startup,
MCO and the 1280-byte stack requirement remain unchanged. Target builds and
hardware checks are pending: menu entry/exit, initial cursor position, no stray
selection on entry, camera shortcuts, F1/F2 gating and saved settings.

## Structure

- Embedded/Src: firmware sources, startup and GCC support.
- Embedded/Include: application and device headers.
- Embedded/Linker: target memory configuration.
- Tools: target build and NoICE session generation.
- Tests: host regressions and syntax/build-layout checks.
- Documents: build instructions, cleanup decisions and diagnosis.
- build: generated and ignored artifacts.

The ImageCraft project remains under Embedded/ImageCraft. Its compiler paths
and custom crt12initrm.o must be available for an ImageCraft build. There is no
PC-side simulator in this project.

## Validation

Run the host checks from the repository root:

```powershell
python -m unittest discover -s Tests
python Tests/run_regression.py
python Tests/check_gcc_syntax.py
```

Application syntax checks treat warnings as errors; legacy MCO warnings remain
visible without changing that implementation. Real GCC builds retain warnings.

On the Windows GNU68HC12 machine, run build_and_debug_noice.bat. Confirm startup,
button toggles, travel/stop, all four F1/F2 setting combinations, cross LEDs,
menu navigation and saved settings after restart. The target build must still
pass its original memory validation. Build ImageCraft before claiming that
compiler path is verified.

## Deferred work

Consider smaller application modules only after this declaration pass is
confirmed. Preserve definition order and memory placement when extracting code;
line count alone is not justification to split a module.

The legacy MCO scan-ID capacity/lookup and SDO sentinel concerns are recorded
for separate review in STARTUP_NODE_ZERO.md. They are not silently repaired in
this cleanup. Likewise, changing the recursive fatal-error recovery policy is
a separate behavioral change.
