# Startup regression: collect evidence before restoring cleanup

## Current state

`cleanup` remains at `65ced39`, the working tree reported good on hardware.
Its tree is identical to `a071ac6`. The two reverted commits remain in history:

- `962cccc`: declaration/type/warning changes, scan table capacity and bounds;
  target compilation/linking completed but the RAM reserve validation failed.
- `35b2044`: read-only keypad/default-stack/vector tables, menu-editor guards
  and additional RAM reporting. It built/loaded but was reported to crash
  immediately. No crash PC, failed ELF/map or readback was captured here.

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

An earlier uploaded ELF (before these cleanup commits, not the exact `65ced39`
image) ends BSS at `0x3AF2`: 1294 stack bytes, just 14 bytes above the reserve.
Growing the scan-ID array from 12 to 20 16-bit entries adds 16 bytes. This is
consistent with the RAM validation failure; it does not prove the cause without
the exact pair of maps, and it does not explain the later startup crash.

The collector's section/symbol/S-record reader was also checked against that
real earlier ELF: its reset vector is `0x4000` and the allocated Flash and data
initializer ranges are present in its generated S-record.

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
symbol addresses and startup disassembly separately. `report.txt`/`report.json`
show the stack margin, reset destination, startup/main bytes, initialization
symbols and table locations. S-record checks reject invalid checksums or
conflicting writes and flag missing allocated section data. These are file
checks, not proof that the device contains or executes those bytes.

Upload **`build/startup-diagnosis/startup-diagnostics.zip`** for the next analysis.
The archive includes only the selected reports/build artifacts, not the Git
worktree metadata or complete worktrees. If a build fails, the archive still
contains its log and available map/section files.

## Next decision

Compare the exact images first: reset/vector values, main and initialization
addresses, data/BSS/soft-register placement, RAM margin, and read-only table
locations. A valid generated image does not rule out a loader, interrupt,
compiler-code-generation or runtime problem. Do not attribute the crash to a
particular const/volatile/type change without evidence.

Once a candidate is supported, create one change from `65ced39` and perform one
hardware check with the failing PC/reset source and expected memory bytes
recorded. Keep the working firmware available throughout. EEPROM offsets and
the stack reserve remain fixed during isolation.
