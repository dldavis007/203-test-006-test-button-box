# Cleanup branch

Baseline: dev commit 775691e, reported working on hardware on 2026-10-20.
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

Host checks pass (14 unit tests, both scan regression char modes and syntax).
Windows build/load and hardware regression remain outstanding for this branch.

## Remaining review

- CANRxISR writes through an uninitialized pointer. CAN reception currently
  polls in mcohw.c; repair the ownership/queue design before enabling RX IRQs.
- FlashWrite uses an uninitialized erase dummy and signed-byte arithmetic;
  review it separately with Flash/character-table hardware validation.
- mco.c needs -O0 for the old GCC backend. Review char-range warnings against
  protocol types before changing signedness globally.
- Serial parsing APIs lack destination sizes; adding bounds changes API and
  needs separate malformed-input tests.
- Audit ISR/main shared data and compiler soft-register reentrancy before
  moving more code into interrupts.
- Reduce unused locals and clarify boolean masks in focused changes. Avoid
  whole-file formatting during logic repairs so reviews remain manageable.
