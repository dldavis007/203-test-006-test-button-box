# Startup regression: Node-ID zero rejected by warning cleanup

The button box's MCOUSER_ResetCommunication explicitly calls
`MCO_Init(125, 0x00, 0)` before configuring RPDOs and TPDOs with explicit CAN IDs.
MCO_Init stores that zero in gMCOConfig.Node_ID. This is existing application
behavior; this fix does not change the unit's Node-ID or bus identifiers.

Commit 962cccc changed two parameter guards, in MCO_InitRPDO and MCO_InitTPDO,
from `Node_ID < 0` to `Node_ID == 0`. Because Node_ID is unsigned, the old
comparison could never reject zero. The replacement newly rejected the
application's configured value. The changed guard remains in 35b2044.

Hardware stepping located the failure at the first MCO_InitRPDO parameter guard.
The caller and generated disassembly corroborate it: the caller initializes
Node-ID zero, while the regression adds an explicit zero branch to fatal error
0x8804. TPDO initialization has the same defect and would reject it with 0x8806.

MCOUSER_FatalError calls InitCANopen, which calls MCOUSER_ResetCommunication and
reaches the same failing initializer again. With unchanged inputs this recurses,
consuming stack on each retry. This explains a startup failure despite adequate
statically reserved stack. The precise final crash PC/stack exhaustion has not
been captured; the invalid zero rejection itself is established.

## Isolated correction

The `startup-fix-node-zero` branch is based on the exact failed 35b2044 commit.
The only firmware edits remove the zero rejection from both PDO guards and
explain the intentional zero configuration. Values above 127 remain rejected.
All other failed-commit cleanup remains present so this is a controlled test of
the identified cause. The hardware-verified `cleanup` branch remains at 65ced39.

There are no EEPROM codec, saved-variable, linker, startup, RAM allocation or
stack-reserve edits. The build still validates at least 1280 bytes for stack.
Addresses from the archived regression disassembly do not apply to a rebuild.

## Validation and hardware check

A host regression extracts the actual RPDO/TPDO initializers. It reproduces the
old failure for zero, then verifies the corrected code accepts every Node-ID
0..127, rejects 128..255, preserves explicit CAN IDs and rejects invalid PDO
numbers, TPDO lengths, offsets and timing parameters. It runs with signed and
unsigned plain char. All 20 unit tests and the F1/F2 regression suite pass.
No target compiler or hardware is available in the analysis environment.

On the Windows toolchain machine:

```powershell
git fetch github
git switch --track github/startup-fix-node-zero
.\build_and_debug_noice.bat
```

Run through initialization normally, then check button operation, F1/F2 enabled
and disabled modes, LEDs and saved settings. Report any remaining failure with
its PC, SP, PPAGE and instruction view. A successful hardware run validates this
specific recovery; it does not by itself validate every other cleanup behavior.
The fatal-error recovery policy is unchanged and should be reviewed separately.
