# Cleanup branch

Baseline: dev commit 775691e, reported working on hardware on 2026-10-20.
The user also reported the reorganized branch fdbdf24 working on hardware.
The cleanup branch starts with directory changes only. Subsequent source
corrections are separate commits. No CAN IDs, EEPROM offsets, F1/F2 semantics,
startup address or PPAGE setting are intentionally changed by the relocation.

## Structure

The Embedded/Src, Embedded/Include, Embedded/Linker and Tools split follows
`dg128_gcc_motion`. There is no PC simulation or empty Common/PC hierarchy.
Documents and Tests contain this project's documentation and regressions.
Legacy/ImageCraft retains imported outputs and project files byte-for-byte.
The relocated ImageCraft project references shared sources and headers; let the
IDE regenerate dependency/make files. Check its local compiler include/library
paths and the custom crt12initrm.o startup availability.

## Verification

Run `run_regression.bat`, then `python -m unittest discover -s Tests`, then
`python Tests/check_gcc_syntax.py`. On Windows, run `build_and_debug_noice.bat`.
Check fresh NoICE source paths, startup to main, all four F1/F2 settings,
function toggles, cross LEDs, normal navigation/travel/stop, and EEPROM after
power cycling. Also build the relocated ImageCraft project before merging.

## First source corrections

- The timer LED scanner accessed entry 18 of an 18-entry table. It now wraps
  before that invalid index and uses an unsigned index. A host test executes
  1000 real handler calls with undefined-behavior bounds checks on Linux.
- Serial parser shifts used overlapping strcpy, which has undefined behavior.
  They now use memmove, with tests for chained commands, leading spaces,
  line endings and empty input under both signed and unsigned char.
- Character terminators use '\0' rather than pointer NULL; serial S-record
  checks use logical AND; unused parser locals were removed.

The first 14-test cleanup pass was reported working on hardware. The subsequent
Flash/CAN pass has 16 host unit tests; both scan regression char modes and source
syntax also pass. Its Windows build/load and hardware regression are pending.

## Remaining review

- CAN reception remains polled in mcohw.c. The old uninitialized-pointer ISR
  was replaced by an RX-interrupt mask fallback that leaves RXF/frame data for
  the poller. Initialization explicitly masks RX IRQ. Receive copies are bounded
  to eight bytes and RXF is released once, avoiding a second clear that could
  discard a newly arrived frame. Tests cover DLC 0-15, frame ID/data and the
  fallback. Interrupt-driven reception would require a separate queue design.
- FlashWrite now initializes the erase dummy, packs unsigned bytes, uses
  volatile command-address stores and rejects lengths outside 1-512 bytes.
  All 65,536 byte pairs are tested under both char modes. The Flash command
  sequence remains unchanged; no active application caller currently uses this
  helper. A hardware test is needed before using it for new Flash writes.
- mco.c needs -O0 for the old GCC backend. Review char-range warnings against
  protocol types before changing signedness globally.
- Serial parsing APIs lack destination sizes; adding bounds changes API and
  needs separate malformed-input tests.
- Audit ISR/main shared data and compiler soft-register reentrancy before
  moving more code into interrupts.
- Reduce unused locals and clarify boolean masks in focused changes. Avoid
  whole-file formatting during logic repairs so reviews remain manageable.

## Shared declarations and warnings pass

The user reported a071ac6 working on hardware before this pass. Shared state
now has one declaration in its owner header; source files include the owner
header instead of repeating externs. Private ISR counters and the application's
private transmit buffer have internal linkage. ISR-shared watchdog flags, SPI
buffers, mode flags, menu update flag and button bits use matching volatile
declarations/definitions. The FirstVar-to-NullVar block retains its field order,
widths and stored values. Menu index types remain one byte, explicitly unsigned.

The scan audit found an actual 12-entry ID table used by 20-entry callers and
a mismatched 127-row extern for a 20-row menu table. Both now use the shared
20-row capacity. Scan replies are bounded by table capacity, ID membership,
subindex 1-4 and payload length; adjacent duplicate checks guard their final
index. Tests cover excess replies, the last row, unknown IDs and invalid input.
The battery selection wraps before its 20-entry table boundary.

Flat aggregate initializers now have explicit field braces. Confirmed unused
locals, signed marker comparisons and ambiguous boolean grouping were corrected.
The SDO end marker is explicit bytes rather than an invalid length passed into
the response macro. Protocol data entries and CAN IDs are unchanged.

`Tests/check_gcc_syntax.py` now uses -Wall -Wextra -Werror rather than -w. Only
the two pointer/int width diagnostics are excluded for this host-only check,
because host pointers are wider than the HC12's 16-bit addresses. The embedded
build retains -Wall -W and its target width assertions; no target warning class
is disabled. A Windows GCC 3.3.6 build is still required to confirm that compiler's
output, code size and ABI. Host checks cannot establish a warning-free target
build. Current unit test count: 17, plus the signed/unsigned keypad regressions.

Remaining work includes size-aware serial parser APIs, menu-building capacity
checks, broader interrupt reentrancy review, and splitting Subroutines.c by
responsibility. This pass does not claim the whole firmware has been audited.

## Follow-up to the GCC 3.3.6 target build

The target build of 962cccc compiled and linked but failed the unchanged 1280-byte
stack reserve check. Keypad_Matrix and defaultMenuStackc are read-only templates
and now const, keeping them in Flash instead of consuming initialized RAM. The
vector pointer table is also const so its section does not request writable
attributes. The target assembler must confirm whether its attribute warning is
resolved. The memory report now prints noinit/data/BSS ranges and actual stack
bytes before validation; the minimum remains 1280.

IncVariable/DecVariable now initialize their pointer/value and reject missing
menus, invalid stack/row positions, null variable pointers and invalid serial
character positions. Tests cover these cases and normal numeric/ON-OFF edits.
All 19 host tests, keypad regressions and strict source checks pass. A new target
build is required to verify the RAM headroom and remaining compiler warnings.
