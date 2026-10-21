# Button-box cleanup

## Current baseline and scope

The owner confirmed normal menu rendering/navigation working on hardware at
651f2ee on cleanup-preserve-mco, after confirming settings save/load at 855bfd6.
651f2ee is the starting point for the internal-menu extraction. Keep changes small
and separately testable. The Node-ID-zero startup regression and recovery are
documented in STARTUP_NODE_ZERO.md.

The MCO integration is protected from general cleanup. mco.c, mcohw.c, user.c,
mco.h, mcohw.h, nodecfg.h and procimg.h match hardware-verified 65ced39. A test
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
