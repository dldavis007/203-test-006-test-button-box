# 203-TEST-006 Button Box

The Rev 3.06 application is in `Embedded/Src` with headers in `Embedded/Include`.
The cleanup work is on `cleanup-preserve-mco`; `main` retains the imported
baseline. Structural cleanup is complete pending the combined target build and
hardware checks in [Documents/CLEANUP.md](Documents/CLEANUP.md).

## Project layout

| Folder | Contents |
| --- | --- |
| `Embedded/Src` | Application C, GCC runtime and startup assembly |
| `Embedded/Include` | Application and device headers |
| `Embedded/Linker` | Target memory configuration; linker scripts are generated |
| `Embedded/ImageCraft` | Current IDE project referencing the shared source |
| `Tools` | GCC build and NoICE mapping scripts |
| `Tests` | Host regressions and build-tool checks |
| `Documents` | Build instructions, cleanup notes and original reference text |
| `build/9s12` | Generated output, excluded from Git |

## Application modules

| Module | Responsibility |
| --- | --- |
| `Subroutines.c` | Existing state, event coordinator, menu-key/entry and camera/reset helpers, video-message assembly |
| `Settings.c` | Saved settings, defaults and EEPROM codec |
| `Display.c` | Titler SPI transport and rendering |
| `Menu.c` / `DeviceMenus.c` | Internal navigation/editing and connected-unit commands |
| `ButtonInput.c` | Keypad scan, latching and outgoing button encoding |
| `CameraDiscovery.c` / `DeviceDiscovery.c` | Camera/unit scans and menu population |
| `LegacyCanInterface.h` | Application declarations for unchanged legacy CAN state |

Global definitions retain their existing placement to protect saved-variable
ordering and RAM layout. MCO, startup/PPAGE and the 1280-byte stack requirement
remain protected. Known behavioral concerns are recorded separately from cleanup.

## Numeric types

Application numbers use fixed-width types from `FixedTypes.h`. Text remains
`char`; native MCO/compiler interfaces retain their existing declarations.
See [the type-migration notes](Documents/FIXED_WIDTH_TYPES.md) for compatibility,
layout protection and target acceptance checks.

## Independent function settings

In SETTINGS MENU, F1 CAM MODE and F2 CTRL MODE use the normal variable editor.
Press Select to move the cursor before the value, use Up/Down to change ON/OFF,
then press Select again to return to the entry label. Select itself does not
change the value. Both default ON. Exit the internal menu to save, as with the
existing settings. The labels
identify function keys; descriptions of camera-menu and positioning modes remain
with the controlled equipment.

OFF disables that key's alternate navigation mode. F1/F2 still latch on each
press/release cycle, set their transmitted message bits and drive their LEDs.
With an OFF function selected, navigation and speed keys keep their ordinary
roles, and the five navigation LEDs are not forced into the cross pattern. ON retains the shared third-byte navigation encoding. The controller
Slow exception remains intact.

## Retired battery feature

Battery diagnostics are no longer supported. STATUS now shows SOFTWARE REV,
SERIAL NUM and EXIT. Battery scanning, RAM/display state and battery-only CAN
requests/replies are removed. Camera discovery and its shared receive buffer
remain supported. Saved settings and EEPROM offsets are unchanged. This removal
was confirmed working by the owner in both GCC and ImageCraft at 032d18e.
See Documents/CLEANUP.md for scope and bench checks.

## Regression tests

Install Python 3 and a host GCC compiler available as `gcc` (or set `CC` to the
compiler executable). From PowerShell:

```powershell
.\run_regression.bat
```

Run the complete host checks with:

```powershell
python -m unittest discover -s Tests
python Tests/run_regression.py
python Tests/check_gcc_syntax.py
```

The runner extracts the actual scan/encoder functions from ButtonInput.c and
their debounce call site from Subroutines.c, injects physical key samples, and
compiles/runs assertions. It also tests the actual function-settings codec.
No PC_SIDE application or interactive simulation is included.

Tests cover all four enable combinations; F1/F2 navigation output and release;
function replacement and toggling; OFF-mode key routing and LED cross gating; internal-menu routing;
normal travel/Slow/Stop; controller Slow preservation; erased and invalid saved strings;
and all four settings round trips. They run with signed and unsigned plain char.
Converted application numbers have target widths in host tests; MCO boundary
objects, integer promotions and pointer/enum layout still follow the host ABI.
These are not processor ABI tests.

## VS Code

Open the repository root (the folder containing `.vscode` and `Embedded`).
Press **Ctrl+Shift+B** to run the default **Build-Run GCC** task, or choose
**Terminal > Run Task > Build-Run GCC**. It compiles and runs host regression
tests, not the embedded application. Python and host GCC must be on PATH.
A play button supplied by an extension may run the active C file instead of this
task; use the task commands above. Use **Build 9S12 GCC** or **Build + Load NoICE** for the embedded targets below.

## EEPROM

F1Enable and F2Enable are in the original saved RAM declaration block, between
FirstVar and NullVar, after SerialNum. ImageCraft uses the existing contiguous
save/load mechanism. GCC explicitly serializes the same fields because linker
placement does not guarantee contiguous C globals. The first 18 EEPROM bytes
are unchanged; F1Enable occupies 0x0812-0x0815, F2Enable 0x0816-0x0819.
Both are stored as ON/OFF strings with terminators. Invalid/old fields default
independently to ON. There is no separate record or checksum at 0x0880.
Exit the entire internal menu to save, then power-cycle to verify persistence.

## Embedded builds and NoICE

Save and close ImageCraft before pulling updates: it writes its in-memory
source list back to ButtonBox.src on exit and can overwrite changes just pulled.
Reopen the project after updating, so newly added source files are compiled.

Open `Embedded/ImageCraft/BUTTONBOX.prj` for the relocated ImageCraft project;
regenerate its make/dependency files in the IDE. Keep the ImageCraft build-folder
setting blank and use `..\Include;C:\iccv712\include\` for the include paths.
A separate build folder caused incorrect generated source paths in the owner's IDE.
Exclude GCC-only runtime.c, compact_io.c and startup.S; retain the custom
crt12initrm.o startup object. Generated files beside the project are ignored.
The GNU68HC12 build compiles all application sources separately from host tests.

From PowerShell in the repository root:

```powershell
.\build_embedded.bat
.\build_and_debug_noice.bat
```

The second command rebuilds, launches NoICE and loads fresh firmware/debug data.
It leaves the target halted at `_start`; use NoICE **Run** to start execution.
Configure NoICE for the actual MC9S12A128 BDM target with **Use PPAGE**, its Flash burner and
hardware breakpoints before loading. Do not reuse a different processor's Flash
programming configuration.

Default compiler location: `C:\Toolchains\GNU68HC12\bin`.
Default NoICE: `C:\Program Files (x86)\NoICE\bin\NoICE12.exe`.
Override with `GNU68HC12_BIN` and `NOICE_EXE` environment variables if needed.
The build emits `build/9s12/buttonbox.elf`, `.s19`, `.map`, and NoICE `.noi` files.
It stops before launching NoICE on any compiler, unresolved-symbol, memory-layout
or debug-map failure and removes stale loadable artifacts.

**Validation status:** host regression, settings codec, source syntax and build-tool
checks pass. The owner confirmed the cleanup through menu extraction at 54ad773
working on hardware, then successfully compiled/linked all 12 application sources
with ImageCraft 7.08 and reported operation. The ImageCraft linker reported
"Device 90% full"; detailed RAM/stack and complete functional checks were not
provided. ImageCraft is not installed in the development environment.

The obsolete root Legacy folder was removed to prevent opening the wrong IDE
project. Its former files remain available in Git history. Original reference
documents remain under Documents/Legacy. Use fresh build artifacts for debugging.

See [Documents/GCC_NOICE.md](Documents/GCC_NOICE.md) for memory layout and bench
checks, and [Documents/CLEANUP.md](Documents/CLEANUP.md) for cleanup scope.

## Hardware software verification

See [the revision 3.06 software test procedure](Documents/SOFTWARE_TEST_PROCEDURE.md)
for the complete source-informed bench checklist, separate GCC/ImageCraft result
records, equipment requirements and controlled engineering tests for known limits.
