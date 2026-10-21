# Button-box cleanup

## Current baseline and scope

The owner confirmed b79ca05 on cleanup-preserve-mco working on hardware.
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
hardware result. The target build/hardware check for this pass remains pending.

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
