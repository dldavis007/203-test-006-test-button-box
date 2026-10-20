# 203-TEST-006 Button Box

The restored Rev 3.05 application is in `Rev 3.05/`. Development is on `dev`;
`main` retains the imported baseline.

## Independent function settings

In SETTINGS MENU, select F1 or F2 to toggle that function ON/OFF. Both default
ON. Exit the internal menu to save, as with the existing settings. The labels
identify function keys; descriptions of camera-menu and positioning modes remain
with the controlled equipment.

OFF disables that key's alternate navigation mode. F1/F2 still latch on each
press/release cycle, set their transmitted message bits and drive their LEDs.
With an OFF function selected, navigation and speed keys keep their ordinary
roles. ON retains the shared third-byte navigation encoding. The controller
Slow exception remains intact.

## Regression tests

Install Python 3 and a host GCC compiler available as `gcc` (or set `CC` to the
compiler executable). From PowerShell:

```powershell
.\run_regression.bat
```

Or run `python tests/run_regression.py`. The runner extracts the actual keypad
scan and output encoder from Subroutines.c, injects physical key samples, and
compiles/runs assertions. It also tests the actual function-settings codec.
No PC_SIDE application or interactive simulation is included.

Tests cover all four enable combinations; F1/F2 navigation output and release;
function replacement and toggling; held disabled keys; internal-menu routing;
normal travel/Slow/Stop; controller Slow preservation; erased and corrupt records;
and all four settings round trips. They run with signed and unsigned plain char.
Host unsigned int remains host-sized: these are not processor ABI tests.

## VS Code

Open the repository root (the folder containing `.vscode` and `Rev 3.05`).
Press **Ctrl+Shift+B** to run the default **Build-Run GCC** task, or choose
**Terminal > Run Task > Build-Run GCC**. It compiles and runs host regression
tests, not the embedded application. Python and host GCC must be on PATH.
A play button supplied by an extension may run the active C file instead of this
task; use the task commands above. Use **Build 9S12 GCC** or **Build + Load NoICE** for the embedded targets below.

## EEPROM

The legacy layout at 0x0800-0x087F is unchanged. A new aligned four-byte record
at 0x0880-0x0883 holds magic 0xA6, version 1, flags (bit 0 F1, bit 1 F2), and
flags XOR 0xFF. The source contains no other users of this range. The serial
number remains at 0x0902. Old/erased/invalid records enable both functions.
Defaults restore both ON. The legacy reset marker takes precedence over this
record. Existing EEPROM write timing delays separate the two write operations.

## Embedded builds and NoICE

The existing ImageCraft project is retained. The new GNU68HC12 build compiles
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

**Validation status:** host logic regression, legacy EEPROM codec, host syntax
checks of all GCC source paths, and build-layout tests passed. This environment
has no GNU68HC12 cross-compiler or NoICE, so no target binary or hardware session
has been validated here. The first Windows cross-build may expose toolchain or
library differences; send its full output if it fails. Host syntax checks do not
check HC12 assembly, ABI, linker execution or target library behavior.

Tracked DBG/S19/LST files under Rev 3.05 are imported baseline artifacts, not
builds of dev. Use only fresh build/9s12 artifacts for this GCC workflow.

See [docs/GCC_NOICE.md](docs/GCC_NOICE.md) for memory layout and bench checks.

The EEPROM word writer packs bytes as unsigned values. This prevents signed-char
extension from corrupting the F1/F2 flags/checksum record. Save on menu exit and
load at startup use the same paths as the other menu settings.
