# 203-TEST-006 Button Box

The Rev 3.05 application is in `Embedded/Src` with headers in `Embedded/Include`.
`dev` retains the hardware-tested baseline at `775691e`; `cleanup` contains the
folder reorganization and separately committed corrections. `main` retains the
imported baseline.

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
| `Legacy/ImageCraft` | Unmodified imported project files and build artifacts |
| `build/9s12` | Generated output, excluded from Git |

## Independent function settings

In SETTINGS MENU, select F1 or F2 to toggle that function ON/OFF. Both default
ON. Exit the internal menu to save, as with the existing settings. The labels
identify function keys; descriptions of camera-menu and positioning modes remain
with the controlled equipment.

OFF disables that key's alternate navigation mode. F1/F2 still latch on each
press/release cycle, set their transmitted message bits and drive their LEDs.
With an OFF function selected, navigation and speed keys keep their ordinary
roles, and the five navigation LEDs are not forced into the cross pattern. ON retains the shared third-byte navigation encoding. The controller
Slow exception remains intact.

## Regression tests

Install Python 3 and a host GCC compiler available as `gcc` (or set `CC` to the
compiler executable). From PowerShell:

```powershell
.\run_regression.bat
```

Or run `python Tests/run_regression.py`. The runner extracts the actual keypad
scan and output encoder from Subroutines.c, injects physical key samples, and
compiles/runs assertions. It also tests the actual function-settings codec.
No PC_SIDE application or interactive simulation is included.

Tests cover all four enable combinations; F1/F2 navigation output and release;
function replacement and toggling; OFF-mode key routing and LED cross gating; internal-menu routing;
normal travel/Slow/Stop; controller Slow preservation; erased and invalid saved strings;
and all four settings round trips. They run with signed and unsigned plain char.
Host unsigned int remains host-sized: these are not processor ABI tests.

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

Open `Embedded/ImageCraft/BUTTONBOX.prj` for the relocated ImageCraft project;
regenerate its make/dependency files in the IDE. The original project is archived
under `Legacy/ImageCraft`. The new GNU68HC12 build compiles
all application sources; it is separate from host regression testing.

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
checks pass. Shared declarations are organized by owning header, and the host
source check treats warnings as errors (excluding target-only address-width casts). The user verified the pre-cleanup GCC build/load, F1/F2 settings,
EEPROM persistence and mode LEDs on hardware. The reorganized cleanup branch
still needs a Windows GNU68HC12 build/load and hardware check. ImageCraft is
not installed here, so its relocated IDE project needs a local build check.

Tracked DBG/S19/LST files under `Legacy/ImageCraft` are imported baseline
artifacts. Use fresh `build/9s12` artifacts for GCC debugging.

See [Documents/GCC_NOICE.md](Documents/GCC_NOICE.md) for memory layout and bench
checks, and [Documents/CLEANUP.md](Documents/CLEANUP.md) for cleanup scope.
