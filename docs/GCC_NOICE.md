# Button-box GNU68HC12 / NoICE workflow

Reference: https://github.com/dldavis007/dg128_gcc_motion
The source-map converter was adapted from that project's Tools/stabs_to_noice.py.
The application, runtime, vectors, settings and watches here are for the button
box, not the motion project's smoke application.

## Commands

From PowerShell at the repository root:

```powershell
.\build_embedded.bat
.\build_and_debug_noice.bat
```

VS Code also has **Build 9S12 GCC** and **Build + Load NoICE** tasks. The default
Ctrl+Shift+B task remains the host regression suite.

If necessary:

```powershell
$env:GNU68HC12_BIN = 'C:\Toolchains\GNU68HC12\bin'
$env:NOICE_EXE = 'C:\Program Files (x86)\NoICE\bin\NoICE12.exe'
```

Build + Load rebuilds before launching, then loads the fresh ELF and source
mappings. The generated session halts, maps RAM/EEPROM, loads the ELF and sets
PC to _start. It does not automatically run the target. Press Run in NoICE.
Configure the correct MC9S12A128 BDM target, Flash burner and hardware breakpoints
in NoICE. Keep the NoICE Virtual UART disabled: this firmware does not reserve
the motion project's UART mailbox addresses.

Official debugger setup:
https://www.noicedebugger.com/help/bdm12.htm
https://www.noicedebugger.com/help/cmdlist.htm

## Build and ABI

GCC 3.3.6-m68hc1x-20060122 is the intended distribution. Defaults look under
C:\Toolchains\GNU68HC12\bin or PATH for m6811-elf-* tools. GNU68HC12_PREFIX
can override the executable prefix. Do not point this at MinGW or ARM GCC.

The flags use HC12, 16-bit int/pointers, 32-bit double, signed plain char,
default near calls (verified by an assembly probe), size optimization, function/data sections and STABS mappings.
Compile-time assertions verify widths. Near calls deliberately differ from the
motion project's far-call build: this port uses only fixed Flash, with no PPAGE
switching. The RAM Flash command wrapper also follows the near-call ABI.

GCC option reference:
https://gcc.gnu.org/onlinedocs/gcc-4.1.2/gcc/M68hc1x-Options.html

## Memory

- RAM: 0x2000-0x3FFF. Twenty reset-persistent bytes are reserved in .noinit for
  _bss_end, then initialized data/BSS. At least 1280 bytes are reserved for stack.
- Fixed Flash: 0x4000-0x7FFF and 0xC000-0xFEFF.
- Page window: 0x8000-0xBFFF is not used. No PPAGE assumption is required.
- Flash configuration/security bytes at 0xFF00-0xFF0F are not emitted.
- Vectors: 0xFF80-0xFFFF, exactly 128 bytes, using the existing vector assignments.

All application and needed library objects are combined relocatably. Their
actual section sizes are packed into the two fixed Flash banks. A generated
linker script places startup at 0x4000, copies initialized RAM data, clears BSS,
preserves .noinit and enforces Flash/vector/stack limits. This fixed-bank build
may not fit: the imported ImageCraft image is about 43 KB and spans the page
window. GCC size optimization may reduce that, but only a real link can decide.
If it cannot fit, the build stops with no loadable output. A banked-memory port
or further footprint work would then be required; this script never silently
places code in an unconfigured page window.

## Source compatibility changes

GCC interrupt declarations and vector placement are conditional, preserving
ImageCraft pragmas. Flash.c and Command.c have GCC-compatible calls. Stdio has
the standard GCC putchar signature and runtime hooks. Interrupt-updated timers,
flags and buffer indexes are volatile. Menu cursor flags are explicitly signed.
The legacy erased-EEPROM check casts to unsigned char.

GCC explicitly serializes the legacy settings: FirstVar at EEPROM +0, MessageRow
+4, MessageCol +8, SerialNum +12. These offsets were checked against BUTTONBOX.mp
from the imported ImageCraft build. It writes 20 aligned bytes, preserving unused
bytes above that range. ImageCraft's existing contiguous storage path is retained.
The separate F1/F2 record remains at 0x0880. Both codecs have host regression checks.

## Validation and next bench checks

Host syntax checks of all GCC C paths, logic regressions (signed/unsigned char),
legacy settings round trips and layout tests pass. This is not a verified target
build: GNU68HC12/NoICE are not installed in this execution environment.

On the Windows machine, first run build_embedded.bat and review any compile/link
output. A successful build must show no unresolved symbols and pass all layout
assertions. Then use a bench unit to check boot/watchdogs, SCI output, timer and
SPI interrupts, keypad/LED scanning, CAN send/receive, settings save/restart,
defaults and F1/F2 against the Repeater Cart Controller. Confirm stop/release
behavior and settings persistence before field use.

Existing CAN receive uses polling through MCOHW_PullMessage. The inherited
CANRxISR has an uninitialized pointer and must not be enabled without a separate
fix. This port retains its existing vector assignment and does not enable CAN
receive interrupts. A host test does not validate an interrupt path.

GCC 3.3.6 rejects `-mno-long-calls`; the build omits that option and verifies
JSR/BSR and RTS in generated probe assembly before compiling the application.

The Windows GCC 3.3.6 cross-build hit an internal D_OR_S_REGS/umulqihi3
register-spill error in optimized MCO_ProcessStack. The build overrides -Os
with -O0 for mco.c only. Other modules remain size-optimized. This may increase
Flash use; the existing link-layout checks still enforce all bounds. The next
Windows build must establish whether this compiler workaround is sufficient.
