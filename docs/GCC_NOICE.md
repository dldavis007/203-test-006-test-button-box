# GCC / NoICE migration findings

Reference inspected: https://github.com/dldavis007/dg128_gcc_motion

That project has `build_embedded.bat`, `build_and_debug_noice.bat`,
`Tools/stabs_to_noice.py`, startup assembly, vectors and a fixed-memory linker
script. It uses GCC 3.3.6-m68hc1x-20060122 via m6811-elf-gcc, with -m68hc12,
-mshort, -fshort-double and -mlong-calls. The recorded Windows tool directory
is C:\Toolchains\GNU68HC12\bin. NoICE12.exe is configured under
C:\Program Files (x86)\NoICE\bin.

## Why the reference cannot simply be copied

- Its linker script allows 16 KB text at 0x4000. This button box's ImageCraft
  layout spans 0x4000-0xFFFF and includes a page window. A firmware map/size and
  page strategy must be established before creating a correct GCC linker script.
- This application needs its actual CAN, timer, SCI and SPI interrupt vectors
  and GCC interrupt attributes; ImageCraft pragmas do not implement GCC ISRs.
- Flash.c and Command.c contain ImageCraft assembly, symbol naming and calling
  conventions. Flash routines must execute safely during programming.
- EEPROM settings rely on contiguous declaration layout between FirstVar and
  NullVar. A GCC port must explicitly serialize legacy settings rather than
  assume linker ordering. Keep compatibility with installed EEPROM data.
- _bss_end is used as a reset-persistent controller-mode string. GCC startup and
  linker code must reserve and preserve it without overlapping stack or data.
- Host whole-file syntax checking already exposes an ImageCraft putchar(char)
  declaration conflict with standard stdio, and command.h versus Command.h
  case differences. These are not problems tested by the focused logic harness.
- The reference's NoICE source-map conversion is for its motion application;
  the generated session and watch list must be adapted to this application's
  source files, not pointed at the reference ELF.

## Next implementation stage

1. Establish the button-box GCC startup/linker/ISR and legacy-storage layout.
2. Compile all application files with the actual 9S12 GCC distribution; resolve
   ABI, library and assembly differences and inspect map/vectors/sections.
3. Generate ELF, S19 and NoICE source mappings for this firmware.
4. Add build-and-debug launcher that stops on any build failure and loads only
   freshly built button-box artifacts.
5. Validate boot, watchdogs, interrupts, keypad/LED scan, CAN, EEPROM and F1/F2
   on a bench unit.

No embedded GCC compiler or NoICE executable is available in this workspace.
No GCC firmware build, upload or debugger session has been claimed or performed.
A copied smoke-project launcher would not constitute button-box compatibility,
so none is provided as a working upload command in this first change.
