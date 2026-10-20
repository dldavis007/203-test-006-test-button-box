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
Configure the correct MC9S12A128 BDM target, enable Use PPAGE, Flash burner and hardware breakpoints
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
motion project's far-call build. Startup fixes PPAGE to 0x3D for the entire run,
allowing ordinary 16-bit code/data pointers into 0x8000-0xBFFF. The firmware does
not switch pages or use far CALL/RTC. The RAM Flash wrapper follows the near ABI.

GCC option reference:
https://gcc.gnu.org/onlinedocs/gcc-4.1.2/gcc/M68hc1x-Options.html

## Memory

- RAM: 0x2000-0x3FFF. Twenty reset-persistent bytes are reserved in .noinit for
  _bss_end, then initialized data/BSS. At least 1280 bytes are reserved for stack.
- Fixed Flash: 0x4000-0x7FFF and 0xC000-0xFEFF.
- Static window: 0x8000-0xBFFF maps PPAGE 0x3D. This is distinct from the
  fixed low/high aliases (pages 0x3E/0x3F). Startup at fixed 0x4000 selects it
  before any window access and before interrupts are enabled.
- Flash configuration/security bytes at 0xFF00-0xFF0F are not emitted.
- Vectors: 0xFF80-0xFFFF, exactly 128 bytes, using existing vector assignments.

The reviewed section report retained 41,253 bytes after pruning, so the original
32 KB fixed-bank restriction was insufficient. Layout now packs code/constants
into all three 16 KB address ranges. Startup and RAM-data initialization remain
in fixed Flash. The generated script enforces each range and vector/stack limits.
Images that exceed this static 48 KB arrangement still fail; it is not a general
128 KB bank-switching port.

NoICE must have Use PPAGE enabled. The generated load command explicitly supplies
the static page: `LOAD "buttonbox.elf" 03DH:0000H`. NoICE documents page-qualified
LOAD offsets; outside the paged window the fixed/RAM addresses remain unpaged.
The session sets PG to 0x3D before and after loading, and generated function/line
addresses inside the window explicitly use `03DH:08xxxH` notation. At reset the
fixed startup also selects 0x3D. The .s19 file has logical 16-bit addresses; any
separate programmer must be configured to map its window to page 0x3D.

NoICE paging reference: https://www.noicedebugger.com/help/2bitmmu.htm

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

## Footprint diagnosis from the first cross-build

The user's section report showed 60,308 bytes of .text (including a 34,317-byte
plain .text library block), 6,420 bytes of read-only constants and 1,630 bytes of
initialized data before garbage collection. Application functions already had
separate .text.NAME sections; they were not merged into the large plain block.
This exceeds even a 48 KB flat layout before pruning.

The GCC build now uses compact_io.c for this firmware's printf/sprintf formats
and decimal menu parsing, avoiding the general-purpose stdio/strtod dependency.
The supported contract is the formats currently present in the firmware, with
floating menu output limited to two fractional digits and normal small menu
values. It is not a complete standard printf/atof replacement (no exponent/NaN
parsing, locale or high precision). Integer/menu output, battery strings,
variable widths and serial output are compared against host libc in tests.
ImageCraft retains its original library functions.

The relocatable link resolves libraries without garbage collection (the old
linker rejects combining --gc-sections with -r). A separate diagnostic full link
prunes unused sections with startup/vector roots and preserves their names.
Only those retained sections enter fixed-bank size packing. The delivered final
link also enables garbage collection. The temporary diagnostic image is never
loaded by NoICE and is deleted before generating the final firmware.
The build saves sections.txt and merged.map for actual target-footprint review.
The Windows cross-build must still establish the resulting size; the subsequent static-page layout change is described above.

The legacy linker also crashed at ldexp.c:745 during the diagnostic link.
The pruning script no longer uses per-section AT(ADDR(section)) expressions:
GC can eliminate their referenced output sections. Its temporary data uses
RAM addresses for both VMA/LMA because that image is never run. Diagnostic
ASSERT expressions were removed too, avoiding an old ASSERT/-Map interaction.
The actual firmware still uses its checked Flash-to-RAM initialization image
and retains the final memory/vector assertions.
Reference: https://ecos.sourceware.org/ml/binutils/2005-08/msg00361.html

The same ldexp.c:745 crash then occurred in the final link. Final-script ASSERT
commands are now replaced by post-link Python checks of the actual ELF section
addresses, load addresses and sizes. These reject Flash/window overflow, vector
size/address errors, RAM overlap, misplaced reset-persistent storage, insufficient
stack reserve and initializer overlap with Flash configuration bytes. Validation
runs before S19 or NoICE session generation; failure removes the ELF too.
