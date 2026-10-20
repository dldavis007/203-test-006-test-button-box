# 203-TEST-006 Button Box

The restored Rev 3.05 application is in `Rev 3.05/`. Development is on `dev`;
`main` retains the imported baseline.

## Independent function settings

In SETTINGS MENU, select F1 or F2 to toggle that function ON/OFF. Both default
ON. Exit the internal menu to save, as with the existing settings. The labels
identify function keys; descriptions of camera-menu and positioning modes remain
with the controlled equipment.

Disabling a function clears its latched selection and rejects its physical key.
A disabled press is consumed, requiring release before a subsequent enabled
press can latch it. F3 and ordinary travel controls retain their existing logic.
The three existing Slow masks and the final Cntrl_Slow_Flag exception remain intact.

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
task; use the task commands above. There is no embedded GCC/NoICE launch task yet.

## EEPROM

The legacy layout at 0x0800-0x087F is unchanged. A new aligned four-byte record
at 0x0880-0x0883 holds magic 0xA6, version 1, flags (bit 0 F1, bit 1 F2), and
flags XOR 0xFF. The source contains no other users of this range. The serial
number remains at 0x0902. Old/erased/invalid records enable both functions.
Defaults restore both ON. The legacy reset marker takes precedence over this
record. Existing EEPROM write timing delays separate the two write operations.

## Embedded validation status

The ImageCraft project remains the embedded build target. The host regression
checks do not compile or validate the whole embedded firmware. Rebuild it with
ImageCraft and check both settings on real hardware: save/restart, defaults,
held keys, LEDs, CAN outputs, F1 camera menu and F2 Repeater Cart positioning.
Tracked DBG/S19/LST files are imported baseline artifacts, not builds of dev.

GCC/NoICE migration findings and remaining work are in
[docs/GCC_NOICE.md](docs/GCC_NOICE.md).
