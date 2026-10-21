# Button-box cleanup

## Current baseline and scope

The owner confirmed 6877736 on cleanup-preserve-mco working on hardware.
That is the baseline for subsequent application cleanup. Keep changes small
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
startup and the stack reserve are unchanged. The target build and hardware check
for this pass are pending.

## Structure

- Embedded/Src: firmware sources, startup and GCC support.
- Embedded/Include: application and device headers.
- Embedded/Linker: target memory configuration.
- Tools: target build and NoICE session generation.
- Tests: host regressions and syntax/build-layout checks.
- Documents: build instructions, cleanup decisions and diagnosis.
- Legacy/ImageCraft: imported legacy outputs.
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
