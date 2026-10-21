# Startup regression: collect evidence before restoring cleanup

## Current state

`cleanup` remains at `65ced39`, the working tree reported good on hardware.
Its tree is identical to `a071ac6`. The two reverted commits remain in history:

- `962cccc`: declaration/type/warning changes, scan table capacity and bounds;
  target compilation/linking completed but the RAM reserve validation failed.
- `35b2044`: read-only keypad/default-stack/vector tables, menu-editor guards
  and additional RAM reporting. It built/loaded but was reported to crash
  immediately. Exact working/failed ELF, S-record, maps and startup disassembly
  have now been collected; a crash PC and target readback are still missing.

The immediate startup crash has **not** been diagnosed. Passing host tests did
not establish target correctness. Do not reapply the warning pass as a group.

This diagnostic branch adds only a build collector, tests and this document.
Application source, startup assembly, linker generation, EEPROM layout and the
1280-byte stack reserve match `65ced39` exactly.

## Established evidence and its limits

The working and reverted trees have identical startup assembly and an identical
FirstVar-to-NullVar saved-variable declaration block. Linker address generation
was unchanged; the follow-up merely printed additional RAM figures. However,
changes in retained code/data and read-only placement can change the generated
addresses even with the same linker script.

The uploaded `startup-diagnostics.zip` contains successful builds of both exact
commits on GCC 3.3.6. Comparing their generated images establishes:

| Item | Working 65ced39 | Reverted 35b2044 |
| --- | --- | --- |
| Reset / startup | 0x4000 | 0x4000 |
| Main | 0x7357 | 0xEFA6 |
| Data initializer in Flash | 0xF0F2 | 0xF272 |
| Initialized RAM | 0x2014..0x241F | 0x2014..0x2387 |
| BSS / software registers | 0x2420..0x3AF5 | 0x2388..0x3A6D |
| Available stack below 0x4000 | 1290 bytes | 1426 bytes |
| Margin above required 1280 bytes | 10 bytes | 146 bytes |
| Keypad table | RAM 0x23A0 | Flash 0xED75 |
| Default menu stack table | RAM 0x2372 | Flash 0xF201 |
| RTI interrupt handler | 0x69BA | PPAGE 0x3D:0xAA9A |

The startup instruction sequence is unchanged. It masks interrupts, maps RAM,
sets SP to 0x4000 and PPAGE to 0x3D, copies the exact initialized-data length,
clears BSS/software registers, and calls the main address shown above. Its
address/length operands agree with each image's section map. Neither image has
missing allocated Flash/data-initializer bytes or conflicting S-record writes.

The 18 keypad entries and default menu-stack bytes match exactly, including
when the working RAM data is read from its Flash initializer. The 11 default
menu records also match after resolving their variable/function pointers by
symbol name. All 64 vector slots resolve to the same handler names in both
images, including the reset vector. FirstVar, MessageRow, MessageCol, SerialNum,
F1Enable and F2Enable have identical default bytes and relative offsets.
These checks compare generated files, not device memory or EEPROM readback.

The failed image passes the static stack-reserve check with *more* available
RAM. This does not prove runtime stack use is safe, or explain the earlier
962cccc reserve failure (that intermediate commit was not collected). The
literal startup sequence, corrupt table initializers and an obvious vector
mismatch are not supported as causes by this evidence. Interrupt code generation,
runtime behavior, and actual Flash/page loading remain unverified. In particular,
the moved RTI handler requires the static PPAGE selection to remain correct;
its new address alone is not evidence of a bug.

## Collect on the Windows toolchain machine

From the repository root, with the same GNU68HC12 environment used for builds:

```powershell
python Tools/collect_startup_diagnostics.py
```

This creates detached Git worktrees for the exact working and failed commits
under `build/startup-diagnosis/worktrees`. It runs each worktree's existing
`Tools/build_9s12.py` directly. It does not call `build_and_debug_noice.bat`,
start NoICE, program the target, change EEPROM, switch the current checkout,
modify either case's firmware, or relax memory validation.

The original GCC 3.3.6 toolchain and local Git history containing those commits
are required. Separate worktrees/artifacts are retained for reproduction. A
rerun requires those diagnostic worktrees to remain at their exact clean
commits. This check ignores generated/untracked build output.

For a dry listing without creating worktrees or compiling:

```powershell
python Tools/collect_startup_diagnostics.py --plan
```

The collector keeps build logs, ELF, S-record, map files, section listings,
symbol addresses, startup disassembly and full disassembly separately. `report.txt`/`report.json`
show the stack margin, reset destination, startup/main bytes, initialization
symbols and table locations. S-record checks reject invalid checksums or
conflicting writes and flag missing allocated section data. These are file
checks, not proof that the device contains or executes those bytes.

Upload **`build/startup-diagnosis/startup-diagnostics.zip`** for the next analysis.
The archive includes only the selected reports/build artifacts, not the Git
worktree metadata or complete worktrees. If a build fails, the archive still
contains its log and available map/section files.

## Next diagnostic step: use the already-built ELFs

From `startup-diagnosis`, update the diagnostic tools and run:

```powershell
git fetch github
git switch startup-diagnosis
git pull --ff-only github startup-diagnosis
python Tools/collect_startup_diagnostics.py --disassemble-existing
```

This validates the recorded commit identities, runs objdump on the two collected
ELFs, and adds `working/disassembly.txt` and `regression/disassembly.txt` to
`build/startup-diagnosis/startup-diagnostics.zip`. It does not rebuild, start
NoICE, load firmware, change EEPROM, or touch the detached diagnostic worktrees.
Upload the updated archive. The original collector now includes full disassembly
as well, for future collections.

If the existing failing image is tested again in NoICE, capture the point of
failure before resetting or loading another image. Set a hardware breakpoint at
its `main` (0xEFA6); confirm PPAGE reads 0x3D. Then use the source view to step
through initialization, especially the first interrupt enable, CAN initialization,
ReadCharMem, Load_Variables and setDefaultMenu. Record the last completed call
and the PC, SP, PPAGE and disassembly at the first failure. A hang in a wait loop,
a watchdog reset and execution from erased Flash require different fixes.
Do not assume a displayed source line alone identifies the executed instruction.

No cleanup has been reapplied. Once a candidate is supported, create one change
from 65ced39 and perform one hardware check. Keep the working image available.
EEPROM record offsets and the 1280-byte reserve remain fixed during isolation.

## Full-disassembly review (2026-10-21)

The second archive includes full objdump output for the exact working and
reverted ELFs. Main retains the same initialization call order and interrupt
enable position. Both sets of ISR prologues save _.frame, _.tmp, _.z and _.xy;
their epilogues restore them before RTI. Keypad addressing in the failed TC3
handler uses Flash address 0xED75 with the expected seven-byte entry stride.
Default-menu copying still uses 311-byte records and the expected 30-byte
default menu-stack copy. These observations identify no definite compiler or
startup fault; they do not certify all generated code or execution.

The next step is an execution checkpoint on the already-built failed image,
not another collection script or a speculative cleanup patch. Its existing
NoICE debug session is:

`build/startup-diagnosis/worktrees/regression/build/9s12/buttonbox_debug.noi`

That session loads the failed 35b2044 image and leaves execution halted at
_start. Close another NoICE instance before opening it. Keep the known-good
image/session available for restoration. Do not use build_and_debug_noice.bat
for this test: that rebuilds the current checkout instead of the failed case.

Set one hardware breakpoint at a time, remove it when reached, and proceed
from the current halt to the next checkpoint:

| Failed-image address | Meaning when reached |
| --- | --- |
| 0xEFA6 | main entry: startup copy/clear and transfer to main completed |
| 0xEFCC | immediately before CLI: peripheral initialization returned |
| 0xEFD1 | InitCANopen returned, with interrupts enabled |
| 0xEFED | initialization including EEPROM load/menu setup reached the main loop |

These are fixed-region addresses; no page qualifier is needed for these
breakpoints. PPAGE must remain 0x3D. If a checkpoint is not reached, halt before
resetting/reloading and capture PC, SP, PPAGE, the instruction view, and the last
checkpoint reached. Record whether the failure is a hang, reset or unexpected
execution, and whether it happens during a debugger pause. A watchdog reset
caused by a long pause must be distinguished from the original free-running
failure. Addresses apply only to the archived regression ELF, not a rebuild or
the currently working image.

No firmware edits have been made as a result of this review. The EEPROM codec,
record offsets, reset-marker allocation and 1280-byte reserve remain unchanged.
