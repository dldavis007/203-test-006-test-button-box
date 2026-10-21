# Preserve the working MCO integration during cleanup

## Identified startup regression

The button box calls `MCO_Init(125, 0x00, 0)` before configuring RPDOs and TPDOs
with explicit CAN IDs. Node-ID zero is intentional application behavior. The
C48 controller also operates as Node-ID zero, according to the project owner.
Do not replace these integration choices with assumptions from generic CANopen
comments or conventional Node-ID ranges.

962cccc changed the RPDO and TPDO parameter guards from `Node_ID < 0` to
`Node_ID == 0`. Node_ID is unsigned, so the old lower-bound check never rejected
zero. The new check rejected this application's configured value. Hardware
stepping located the failure in the first MCO_InitRPDO guard; the caller and
uploaded disassembly corroborate that path. MCOUSER_FatalError then restarts
CAN initialization recursively with the same inputs, consuming stack on each
retry. The zero rejection is established; the final crash PC was not captured.

## Current restoration test branch

The first correction, 93e182d on startup-fix-node-zero, removed the new zero
checks and was confirmed working on hardware. That branch is preserved.
Following the owner's direction, cleanup-preserve-mco now restores mco.c,
mcohw.c, user.c, mco.h and mcohw.h byte-for-byte from working commit 65ced39,
instead of including general MCO cleanup. nodecfg.h and procimg.h also match
that baseline. Tests pin all seven files to that baseline, normalizing Windows
line endings. The owner-approved baseline includes earlier hardware-verified
porting and driver changes; it is not an upstream vendor-library replacement.

Application declarations were adjusted to compile with those original headers.
The incidental volatile change to controller_mode was also undone to match the
original MCO declaration. The application scan's adjacent-entry guards retain
the existing 20-entry loop limit without exporting a new MCO capacity macro.
No new MCO behavior, scan buffer, response handler or fatal-error policy is
introduced. The test of the removed scan-response helper was removed with it.

The application syntax check remains strict. Legacy MCO warnings stay visible
and do not become errors in the host syntax check. Do not rewrite MCO behavior
merely to eliminate a warning. A demonstrated defect or substantial benefit
must be proposed and validated separately. Examples requiring separate review
include the existing 12-entry response-ID array versus application scans of
20 entries, unbounded response lookup, and the legacy SDO sentinel macro.
They have not been silently repaired as part of this cleanup.

The unit tests exercise the actual PDO initializers, accepting Node-ID 0..127
and rejecting 128..255, and check PDO number, TPDO length, timing and offsets.
The stack files also have a baseline-integrity test. Target compilation and
hardware confirmation are still required after this restoration.

EEPROM storage, saved-variable offsets, startup assembly, linker generation
and the required 1280-byte stack reserve are unchanged. cleanup remains at the
hardware-verified 65ced39. This branch retains the other application cleanup
for testing; it has not been merged into cleanup.

```powershell
git fetch github
git switch cleanup-preserve-mco
git pull --ff-only github cleanup-preserve-mco
.\build_and_debug_noice.bat
```

If the branch has not previously been checked out, use
`git switch --track github/cleanup-preserve-mco` instead of `git switch`.
