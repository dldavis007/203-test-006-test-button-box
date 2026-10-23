# 203-TEST-006 Button Box - Software Test Procedure

Firmware revision: 3.06 | Procedure date: 2026-10-23 | Test cases: 86

This is a source-informed hardware verification procedure for the 203-TEST-006 Button Box, revision 3.06. It covers exposed user functions, protocol integration and selected service/fault paths. It is not a claim that hardware tests have been executed, proof of every software branch, a safety certification or a CANopen conformance certificate. Source baseline: 941f19479ca6eb76d124ee8571a7194bca9aa162 on cleanup-preserve-mco. Later revisions require a change-impact review.

## Test record

Unit serial: __________  Board revision: __________  Operator: __________  Date: __________
GCC build/hash: ____________________  ICC build/hash: ____________________
Connected equipment/firmware: __________________________________________
EEPROM backup: ____________________  CAN/video evidence folder: ____________________

Use P (pass), F (fail), B (blocked), NT (not tested), or NA (not applicable with reason/approval). Every case has separate GCC and ICC result spaces. CORE is normal acceptance; EQUIPMENT needs named fixtures; ENGINEERING needs controlled fault injection or an owner decision. A known defect is still a failure against the desired criterion, never an automatic NA/pass.

## Bench preparation

1. Use a secured bench with motion/actuators disabled or mechanically safe. Controller Reset broadcasts 81 00 to every connected node; scans may randomize camera/CNT-031 addresses. Restore unit addressing/settings afterward.
2. Equipment: button box/board revision under test, regulated supply, monitor and video source, supported C48 controller, at least one SD camera and one VID-018/HD camera, Repeater Cart Controller, primary/auxiliary test units, IN58 and CNT-031 if applicable.
3. Instrumentation: Windows PC with GCC 3.3.6/ImageCraft 7.08B and NoICE/BDM, CAN capture/injection interface at 125 kbit/s, correct termination, serial adapter matched to board levels, optional scope/logic analyzer and EEPROM backup tool.
4. Run against both compiler images and retain separate result records. Do not silently carry a GCC pass over to ImageCraft. Save compiler version, exact commit, S19/ELF/DBG hashes and programmer erase options.
5. Before loading, save/close ImageCraft so its in-memory .src list does not overwrite updates. ImageCraft uses crt12initrm.o; GCC uses startup.S/runtime support and static PPAGE 0x3D. Neither build may violate its memory map.
6. Back up EEPROM, record serial/address/tag configuration, and confirm the normal menu/control baseline. A status message or LED alone does not prove the transmitted payload: log CAN/raw state for encoding tests.

## Execution order and scope

Start with A, then B-E, followed by F-K as equipment permits. Perform ENGINEERING cases L only with the stated controls. Finish with J01/J03 and L07. Re-run impacted cases after a fix; do not merge firmware acceptance with an untested documentation revision.

## A. Build, loading and startup

Run separately for GNU68HC12 GCC and ImageCraft. Load matching firmware and symbols. Preserve an EEPROM backup before programming.

### A01 - Build and identity [CORE]

**Steps:** Record branch/commit, firmware hash, compiler version, hardware revision and serial. Rebuild all sources for each compiler; capture build logs and the memory map.

**Expected / evaluation:** Revision shown by firmware is 3.06. Each build succeeds with no new unresolved errors. Record all warnings; check real RAM/Flash usage and at least 1280 stack bytes for GCC. ImageCraft memory use is reviewed separately.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### A02 - Host regression suite [CORE]

**Steps:** Run python -m unittest discover -s Tests, python Tests/run_regression.py, and python Tests/check_gcc_syntax.py. Save complete output.

**Expected / evaluation:** Current baseline has 32 unit tests; all pass. Both char modes of F1/F2 regressions pass; strict application syntax passes. Legacy MCO warnings remain identified separately.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### A03 - Load, PPAGE and main [CORE]

**Steps:** GCC: run build_and_debug_noice.bat, confirm fresh ELF/S19/debug session, then Run from _start. ImageCraft: rebuild/load the matching output using its existing custom startup object.

**Expected / evaluation:** Program reaches main and operates without executing blank memory. GCC PPAGE is 0x3D before paged code runs. No startup reset loop; correct vectors and matching source stepping.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### A04 - Cold boot as follower [CORE]

**Steps:** With a known working controller connected, power down/up without holding Rotate CCW. Use a bench boot that does not retain a prior controller marker. Log traffic and startup time.

**Expected / evaluation:** Follower behavior agrees with the established system: buttons transmit, incoming menus display, and no unintended local-controller scan or motion occurs. Record actual mode flags and startup duration.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### A05 - Controller startup and soft reset [CORE]

**Steps:** Hold Rotate CCW during startup on the isolated bench, then release. Observe scan requests and selected camera. Perform a software reset and repeat.

**Expected / evaluation:** Controller mode starts and discovers equipment. Rotate CCW is not emitted as an unintended startup command. The Cntrl RAM marker retains controller mode through the supported soft-reset path; cold-boot retention is recorded rather than assumed.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### A06 - Legacy/no-two-wire startup [EQUIPMENT]

**Steps:** Using the approved legacy fixture, start with the CAN/two-wire presence detection unavailable, with and without Rotate CCW. Then restore the normal fixture.

**Expected / evaluation:** Document legacy_mode, controller_mode and Gen_Flags_No2Wire. Local controller menus remain usable in the supported legacy startup path. No repeated fatal reset. Do not equate legacy_mode with merely an empty CAN bus.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### A07 - Repeated startup and ordinary watchdog service [CORE]

**Steps:** Run at least ten cold starts and ten supported soft resets, then 30 minutes of menus/scanning/buttons. Record restart counts and supply conditions.

**Expected / evaluation:** No unexplained resets, hangs, missing characters or loss of saved settings. Software/hardware watchdog service remains active during ordinary operation and expected scan delays.

GCC: ____  ICC: ____  Evidence/defect: ____________________

## B. Keypad, LEDs and ordinary commands

Start with no menu active and F1/F2 alternate modes unselected. Use CAN capture and, where available, the two-wire analyzer. Exercise all 18 buttons listed in Appendix A.

### B01 - Every physical key and LED [CORE]

**Steps:** For each Appendix A row, press/release five times, then hold two seconds and release. Observe the matching LED, raw ButtonBoxData and transmitted bytes.

**Expected / evaluation:** Correct physical key is recognized; no adjacent key ghosting. Momentary keys release; toggle keys change once per press/release. Interpret speed/function/menu keys with their mode-specific tests, not as isolated unconditional bits.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### B02 - Debounce and held toggle [CORE]

**Steps:** Rapidly tap F1, F2, F3, Forward and Reverse, then hold each. Leave a clear release interval between deliberate presses.

**Expected / evaluation:** No repeated toggling during a held press. Five deliberate presses produce five transitions after release qualification. Startup with a toggle held is characterized; do not assume the initial static release history means an immediate toggle.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### B03 - Function replacement [CORE]

**Steps:** With other function keys released, select F1, then F2, then F3. Press the selected key again to clear it. Repeat with both settings OFF.

**Expected / evaluation:** Normally only the newly selected function remains latched; second press releases it. OFF does not remove raw function bits or their own LEDs. Simultaneous combinations are covered separately.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### B04 - Momentary direction/select/trigger [CORE]

**Steps:** With function modes unselected, exercise Up, Down, Select, Trigger, Rotate CW and Rotate CCW. Capture press and release bytes.

**Expected / evaluation:** Momentary command bits follow the physical presses and clear after release. Trigger is 0x0100, Select 0x0004, CW 0x0008, CCW 0x0010. Raw and encoded values are distinguished when a menu is active.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### B05 - Option/data baseline [CORE]

**Steps:** Observe idle frames, then press/release Option/data. Record Option LED and the board's display-menu GPIO if accessible.

**Expected / evaluation:** Bit 0x0400 is forced into ButtonBoxData by the existing scan and is therefore not a clean press-only indicator. Check the physical input/LED/GPIO transitions independently; record baseline rather than failing idle frames for this bit.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### B06 - Travel and Stop LEDs [CORE]

**Steps:** Observe idle, Forward, Stop, Reverse and Stop. Check all speed LEDs in the travel tests.

**Expected / evaluation:** Stop LED is lit when neither travel direction is latched. Direction LEDs track the selected direction. No flicker or unrelated LED changes beyond the defined alternate-mode cross pattern.

GCC: ____  ICC: ____  Evidence/defect: ____________________

## C. Travel, speed and C48 input

Disconnect or otherwise secure moving hardware. Use actual controller data at CAN ID 0x280, or an approved fixture, for the C48 cases.

### C01 - Forward/reverse interlock [CORE]

**Steps:** Latch Forward, then select Reverse after releasing Forward; reverse the order. Toggle the selected direction off.

**Expected / evaluation:** The newly selected direction replaces the other; neither ordinary sequential case leaves both directions set. Toggling the direction off leaves no travel command.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### C02 - Speed toggles [CORE]

**Steps:** While traveling, exercise Extra Slow, Slow and Fast individually and in combinations. Press each twice and inspect output bits/LEDs.

**Expected / evaluation:** Each is a toggle in ordinary mode. The code does not promise three-way speed exclusivity; capture simultaneous combinations and have the controlled-unit specification determine their meaning.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### C03 - Stop and no-travel cleanup [CORE]

**Steps:** Latch travel plus each speed, press Stop, then repeat by toggling the direction off.

**Expected / evaluation:** Forward and Reverse clear. Extra Slow and Fast clear when no direction remains. Ordinary Slow also clears; enabled alternate-mode Slow has the separate navigation exception.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### C04 - C48 Forward and Reverse [EQUIPMENT]

**Steps:** Send recurring 0x10 commands on 0x280, then recurring 0x20 commands, with each interval below the timeout. Observe release and direction changes.

**Expected / evaluation:** Forward/Slow or Reverse/Slow is set respectively. The opposing direction, Extra Slow and Fast clear. The controller Slow flag preserves the speed bit through alternate encoding.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### C05 - C48 inhibit and command consumption [EQUIPMENT]

**Steps:** While recurring controller travel is active, apply 0x04 inhibit. Observe gProcImg[OUT_digi_12] after consumption.

**Expected / evaluation:** Travel stops. The handled 0x34 mask is cleared from the process image so a stale command is not reapplied indefinitely. No unintended restart without a fresh valid command.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### C06 - Controller timeout [EQUIPMENT]

**Steps:** Establish controller travel, then cease valid switch messages. Capture last valid message and travel-bit removal; repeat in both directions.

**Expected / evaluation:** Travel clears after the nominal 125 ms timer expires, plus scan/scheduling latency. Measure actual delay; do not claim a tighter response limit than measured/approved. Timeout is not a substitute for a system safety interlock.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### C07 - Conflicting controller inputs [ENGINEERING]

**Steps:** Inject Forward+Reverse, then Forward+Reverse+inhibit in an isolated fixture. Record the output and receiving unit response.

**Expected / evaluation:** Characterize existing statement-order behavior; reverse is applied after forward and inhibit removes travel. Record any ambiguous or hazardous receiver action as a defect requiring a separate decision.

GCC: ____  ICC: ____  Evidence/defect: ____________________

## D. Internal menus and variable editing

Use controller mode with the ordinary local menu available. Return to a known settings baseline before each case.

### D01 - Menu entry and root navigation [CORE]

**Steps:** With enabled function modes inactive, press Menu, navigate all root entries and wrap above/below the ends. Exit and re-enter.

**Expected / evaluation:** Menu opens with the cursor on the expected first entry; no accidental selection from the entry press. Empty rows are skipped. Root contains Settings, Diagnostics, Cameras, Status, Auxiliary Equip, Defaults and Exit plus discovered units.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### D02 - Select/release and held keys [CORE]

**Steps:** Press/release Select repeatedly on menu entries; hold Up/Down. Check single presses and repeat after the menu delay.

**Expected / evaluation:** One deliberate Select action is not re-fired without release; repeat timing is controlled by MenuTimer. Up/Down navigate in the expected direction. No stuck -1 flags under ImageCraft or GCC.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### D03 - Message Row editor [CORE]

**Steps:** Select MESSAGE ROW; change upward/downward, test 1 and 13 boundaries, then Select again.

**Expected / evaluation:** Cursor moves immediately before the three-character value. Up increments, Down decrements; values wrap 1..13. Second Select returns to the row label without changing the value.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### D04 - Message Column editor [CORE]

**Steps:** Repeat on MESSAGE COL, including both ends of 1..30 and wrap in each direction.

**Expected / evaluation:** Same edit workflow; values stay three characters wide. Editor selection does not itself alter the value. Display placement near the right edge is separately characterized in I06.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### D05 - F1 CAM MODE editor [CORE]

**Steps:** Select F1 CAM MODE from both ON and OFF. Press Up and Down, Select to exit, then enter/exit again without changing it.

**Expected / evaluation:** Select only enters/exits VariableFlag mode. Up and Down each toggle ON/OFF; the cursor remains on the value while editing. Value starts at column 15; F2 and raw function latch bits remain unchanged.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### D06 - F2 CTRL MODE editor [CORE]

**Steps:** Repeat D05 for F2 CTRL MODE and verify F1 stays unchanged. Watch AlternateFunctionMask or the cross LEDs after changing it.

**Expected / evaluation:** Independent setting; Select does not toggle it. LED-mode mask updates immediately on an actual edit. Text is exactly three characters: leading-space ON or OFF.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### D07 - Exit/save boundary [CORE]

**Steps:** Change both function settings and a message position. Leave Settings to root, then exit the entire local menu. Capture EEPROM before/after each exit.

**Expected / evaluation:** Submenu exit does not invoke the full Save_Variables path; full menu exit does. No premature reset or corruption. On reboot, all edited values match J01.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### D08 - Status, diagnostics and serial [EQUIPMENT]

**Steps:** Visit Diagnostics and Status. Confirm software revision and serial. On a spare uncommissioned unit only, enter an unset serial with the existing five-digit editor; try editing an already valid serial.

**Expected / evaluation:** Diagnostics currently contains Exit, not a missing battery test. Status shows revision 3.06, serial and Exit. An assigned valid serial is not freely editable; unset digit entry follows the existing commissioning behavior and separate serial save.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### D09 - Restore defaults [CORE]

**Steps:** Back up settings. Set nondefault values, choose DEFAULTS / RESTORE DEFAULTS, allow reset, then re-enter Settings.

**Expected / evaluation:** F1/F2 return to ON; message positions return to firmware defaults (1/1) after reboot. Serial handling is checked against the recorded pretest serial. Confirm automatic reboot and normal control afterward.

GCC: ____  ICC: ____  Evidence/defect: ____________________

## E. F1/F2 enabled and disabled modes

Repeat the four combinations in Appendix B. Test each function independently with other function keys released. Use the real camera for F1 and Repeater Cart Controller for F2 where available.

### E01 - Four independent combinations [CORE]

**Steps:** Set ON/ON, ON/OFF, OFF/ON and OFF/OFF using the editor; fully exit. In each, latch F1 and then F2 separately.

**Expected / evaluation:** Own LED and transmitted function bit work in every combination. Alternate navigation and the five-key cross pattern apply only when that selected function is enabled. F3 remains an ordinary function selection.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### E02 - Enabled third-byte navigation [CORE]

**Steps:** For each enabled mode, exercise Slow, Rotate CW, Down, Up and Menu one at a time; release each. Compare with Appendix B.

**Expected / evaluation:** Third byte is 0x01, 0x02, 0x04, 0x08 or 0x10 respectively; release clears momentary navigation. The function bit remains in the first two bytes. Repurposed raw key bits are stripped, except controller Slow.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### E03 - Disabled-mode ordinary routing [CORE]

**Steps:** Turn the selected function OFF and repeat E02, once for each function. Exercise travel before checking ordinary Slow.

**Expected / evaluation:** No alternate third-byte navigation or forced cross LEDs. Momentary ordinary key bits and travel/speed rules apply. Function bit still latches/transmits. In controller mode Menu/Up/Down may invoke local menus or camera shortcuts rather than pass through untouched.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### E04 - Slow changes between modes [CORE]

**Steps:** With no travel, hold/release Slow in an enabled F1 mode and then enabled F2. Repeat OFF with travel and across switching functions.

**Expected / evaluation:** Enabled navigation Slow is momentary, not left latched. Ordinary Slow remains subject to travel/no-travel cleanup. Switching modes does not leave a stale third-byte Up command.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### E05 - C48 Slow exception [EQUIPMENT]

**Steps:** With an enabled function selected, send recurring C48 Forward or Reverse while exercising alternate navigation.

**Expected / evaluation:** The controller-requested Slow bit remains in the first two output bytes despite navigation remapping. Stop/timeout still clears travel as tested in C05/C06.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### E06 - Receiver behavior and mode text [EQUIPMENT]

**Steps:** Use F1 to open/control the selected camera menu. Use F2 to rotate and extend/retract the Repeater Cart Controller. Record the mapping of each navigation key from the receiver specification.

**Expected / evaluation:** Only the intended selected equipment acts. F1/F2 bits distinguish the shared navigation payload. Receiver-generated status such as Position Active displays correctly; button box does not invent a hardcoded mode message.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### E07 - Record/alarm output [EQUIPMENT]

**Steps:** With F1 latched, press/release Trigger and measure the alarm output; clear F1 and repeat. Repeat F1 setting ON and OFF.

**Expected / evaluation:** The existing alarm expression depends on raw F1+Trigger, not F1 enable: output asserts for that combination in either setting. Camera recording additionally depends on the camera's alarm configuration.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### E08 - F2+F3 display animation [EQUIPMENT]

**Steps:** On an isolated unit, hold the simultaneous combination needed to retain F2+F3. Observe glyph animation, then release/clear the combination.

**Expected / evaluation:** Existing three-frame animation and recovery are preserved. Sequential selection normally excludes the other function, so it is not sufficient to test this combination. No stuck animation/menu state after release.

GCC: ____  ICC: ____  Evidence/defect: ____________________

## F. Camera discovery and selection

Use SD cameras, VID-018/HD fixtures and a CAN logger. Record addresses/tags before scanning: FIND CAMERAS normally issues address randomization.

### F01 - Startup camera scan [EQUIPMENT]

**Steps:** Boot controller mode with one SD camera, then a small mixed SD/HD set. Record discovery traffic, list order, addresses and tags.

**Expected / evaluation:** 0x521 payload 02 requests addresses. Connected cameras appear once in the supported list with corresponding addresses/tags. Selected-address bytes are little-endian. No unsolicited rotation during startup.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### F02 - FIND CAMERAS [EQUIPMENT]

**Steps:** Choose FIND CAMERAS with two known cameras. Log traffic and compare IDs before/after.

**Expected / evaluation:** Normal manual find sends 0x521 payload 01 (randomize), then payload 02 (scan). List is rebuilt and first returned camera selected. Address changes are expected where receiver firmware implements randomization.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### F03 - Camera tag assembly [EQUIPMENT]

**Steps:** Use an HD camera with a distinctive ten-character tag, leading spaces, and a nonblank final character. Repeat with a shorter tag and an SD camera without a tag.

**Expected / evaluation:** Both tag fragments assemble into the intended label without losing the last character. Leading-space handling and trailing spaces are checked visually. Untagged cameras use the fallback CAMERA label.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### F04 - Camera menu selection [EQUIPMENT]

**Steps:** Select each listed camera from CAMERAS. Capture the command and selected-address process image.

**Expected / evaluation:** Command 0x521 carries 08, address low byte, address high byte. External-menu mode starts and only the addressed camera menu responds; selection PDO address matches.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### F05 - Up/Down shortcuts and wrap [EQUIPMENT]

**Steps:** Exit menus with enabled alternate modes unselected. Use Up/Down through a nonempty camera list, including first/last wrap. Hold each and repeat with Select held.

**Expected / evaluation:** Selection advances/wraps correctly with nominal 0.75 s repeat delay. 0x421 carries selected address low/high. Holding Select suppresses the shortcut. Function-enabled modes route keys to navigation instead.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### F06 - Single camera and rescan shrink [EQUIPMENT]

**Steps:** Repeat shortcuts with one camera. Scan three cameras, remove one and rescan; check list, Exit row and selection.

**Expected / evaluation:** Single camera remains selected and its tag can still be shown. Removed entries are absent; no stale Exit/action pointers. Capacity-edge stale-slot tests are in L03.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### F07 - VID-018 update notification [EQUIPMENT]

**Steps:** Change a camera tag or invoke its supported restore-defaults notification so 0x2A1 byte 2 is FF. Capture the following scan.

**Expected / evaluation:** Notification is consumed/cleared, Updating Camera List appears, and the list rebuilds. vid18_update prevents the normal manual-find randomization step; record that no unwanted 01 is sent for this path.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### F08 - No cameras and duplicate addresses [EQUIPMENT]

**Steps:** With no cameras, boot and open the Cameras list without immediately using shortcuts. Separately use a fixture with duplicate camera IDs, then manual FIND CAMERAS.

**Expected / evaluation:** Empty scan returns control with no invented camera. Duplicate/randomization outcome is recorded against receiver support. Do not assume duplicate_add() itself guarantees automatic repair; empty shortcuts are a controlled robustness case.

GCC: ____  ICC: ____  Evidence/defect: ____________________

## G. Unit discovery and CNT-031 handling

Use a small nominal network first; capture all SDO requests and replies. Specialized CNT-031 tests may change unit addresses and require an isolated bench.

### G01 - Discovery request range [EQUIPMENT]

**Steps:** Start controller mode and log initial device discovery.

**Expected / evaluation:** Requests cover CAN IDs 0x610..0x67F to object 0x2100/subindex 0. Reply IDs map as 0x580+Node-ID. Scan completes and the menu remains responsive; do not infer Node-ID filtering from a bare CAN-ID comparison.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### G02 - Primary and auxiliary menus [EQUIPMENT]

**Steps:** Connect one primary and two auxiliary units with distinctive names. Open root and Auxiliary Equip, including adjacent auxiliary replies.

**Expected / evaluation:** Primary/auxiliary classification and labels match the reply marker (FF for auxiliary). Entries retain the correct target after sorting; no skipped adjacent auxiliary row. Exit remains accessible.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### G03 - IN58 exclusion [EQUIPMENT]

**Steps:** Add the inclinometer with response ID 0x5BE before, between and after normal discovery replies in a controlled fixture.

**Expected / evaluation:** Initial discovery may query Node-ID 0x3E, but its 0x5BE response is excluded before menu-detail requests. Other unit entries remain correct. Repeated/end-of-full-array exclusions require separate boundary characterization.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### G04 - Ordinary duplicate reply [EQUIPMENT]

**Steps:** Inject adjacent duplicate replies for an ordinary non-CNT-031 unit, then scan normally.

**Expected / evaluation:** Nominal adjacent duplicate removal does not create duplicate menu entries or misroute the remaining targets. Record behavior for more than two duplicates rather than assuming arbitrary deduplication is implemented.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### G05 - CNT-031 duplicate/resequence [EQUIPMENT]

**Steps:** Use approved CNT-031 units/fixture at response IDs 0x5E1..0x5E9 with an adjacent duplicate. Capture the complete exchange.

**Expected / evaluation:** Existing proprietary randomize command 33 00 on CAN ID 0, rescan 0x661..0x669, resequence commands and 34 response-table broadcasts occur as expected by matching unit firmware. Unique units remain accessible afterward.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### G06 - CNT-031 rescan request [EQUIPMENT]

**Steps:** Have a CNT-031 fixture send the supported 34 01 notification on its low identifier in 0x061..0x06A.

**Expected / evaluation:** Application clears/rebuilds discovery/menu state through its existing path without stale target indices. This tests the installed private protocol, not CANopen conformance.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### G07 - No units and retired battery feature [CORE]

**Steps:** Boot with no discoverable units, then inspect menus and startup traffic. Compare with a normal connected run.

**Expected / evaluation:** Empty scan completes without fabricated unit entries. Battery diagnostics/requests do not return. Camera RPDO 0x500 remains functional; its retention is not a leftover battery fault.

GCC: ____  ICC: ____  Evidence/defect: ____________________

## H. External menus and device triggers

Use cooperative camera/unit firmware. Exercise actual receiver acknowledgments and close/clear behavior, not only transmitted command bytes.

### H01 - Primary unit menu [EQUIPMENT]

**Steps:** Select a discovered primary unit from root; navigate, Select, and exit its menu.

**Expected / evaluation:** Command to 0x200+Node-ID has one byte 01. Internal menu flag clears, external-menu flag sets, and only the selected unit opens. Return to local menu after the receiver releases external-menu control and the reactivation delay expires.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### H02 - Auxiliary unit menu [EQUIPMENT]

**Steps:** Select each auxiliary entry, including entries requiring scroll. Repeat with more than one auxiliary target.

**Expected / evaluation:** One-byte 01 goes to the correct 0x200+Node-ID derived from the auxiliary table. Labels, target addresses and return behavior stay aligned after sorting.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### H03 - Valid device trigger row [EQUIPMENT]

**Steps:** Highlight a discovered primary device row, press/release Trigger, then trigger again after returning to the appropriate row.

**Expected / evaluation:** One-byte 02 goes to the selected 0x200+Node-ID; existing DeSelect behavior follows. Release/repress rearms the signed trigger flag; no repeated unsolicited triggering after release.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### H04 - External-menu priority [EQUIPMENT]

**Steps:** While a unit reports external menu active (0x200 bit 0), operate local menu keys and receive display updates; then clear that state.

**Expected / evaluation:** External menu remains in control while active. Reactivation timer refreshes; local mode resumes after supported close/clear behavior. No local camera shortcut or enabled alternate-mode leakage into the wrong menu.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### H05 - Internal-menu key suppression [CORE]

**Steps:** Open the internal menu and exercise Up/Down/Menu while logging 0x180. Repeat with F1/F2 OFF and with enabled alternate modes prevented by menu state.

**Expected / evaluation:** Local navigation does not leak ordinary Up/Down as outgoing unit toggles. OFF function bits may still latch; enabled alternate keys obey MenuCheck blocking. Verify state-specific behavior rather than assuming all keys are globally suppressed.

GCC: ____  ICC: ____  Evidence/defect: ____________________

## I. Video messages, SPI and custom glyphs

Use a video monitor, a known video source and a controlled message sender. Appendix C provides a short 0x310 test message. Use protocol-valid lengths in normal tests.

### I01 - Fresh and existing font memory [EQUIPMENT]

**Steps:** On a spare display, test boot with the expected plus glyph already installed, then with a deliberately different plus glyph using an approved display tool.

**Expected / evaluation:** ReadCharMem compares glyph 0x52. Matching data skips rewriting; mismatch causes installation of the 19 stored glyphs at 0x4D..0x5F. Startup completes without watchdog resets. This is a plus-glyph check, not a full font checksum.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### I02 - Characters and animation frames [EQUIPMENT]

**Steps:** Display uppercase/lowercase, digits, supported punctuation, the plus/percent symbols and the three animation frames using valid message fixtures.

**Expected / evaluation:** Characters match CharacterFontData.inc and DispTranslate. Distinguish supported characters from unspecified translations; record glyph shape, clipping and source-video visibility.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### I03 - Short/fragmented status messages [EQUIPMENT]

**Steps:** Send the Appendix C Proc: example. Repeat Warn: and Icam: with valid framing, one frame at a time and back-to-back.

**Expected / evaluation:** TEST appears without prefix/control characters, fragmentation artifacts or missing final character. Releases/new messages clear the prior assembly state. Capture the actual display and CAN trace.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### I04 - Message categories and interleaving [EQUIPMENT]

**Steps:** Test Proc, Warn, Icam, Emsg, Xmsg, Home and Menu formats with their proper message-origin IDs. Interleave two then four status origins.

**Expected / evaluation:** Display/routing is recorded for each supported format; no cross-origin text mixing or memory corruption. Acceptance for Emsg/Xmsg is confirmed explicitly because display gating differs from the three status categories that set ICAM_MSG.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### I05 - Menu-frame assembly and clear [EQUIPMENT]

**Steps:** Use valid Home/Menu fragments and a Clr: plus ETX frame with the appropriate menu origin. Repeat with local and external menus active.

**Expected / evaluation:** No partial menu displayed before completion; correct rows appear after the end marker. Clear removes the expected display and releases supported menu state. Normal status traffic can resume.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### I06 - Row/column placement and edges [EQUIPMENT]

**Steps:** With no menu, test Message Row 1 and 13 and Message Col 1 and 30 using short text. Repeat while a local/external menu is active.

**Expected / evaluation:** Configured placement is correct when no menu is active; messages are routed to the reserved top row during menus. Check right-edge behavior explicitly; do not assume long messages clip safely at column 30.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### I07 - Repeated/long messages and Holiday symptom [EQUIPMENT]

**Steps:** Exercise valid 21/22-character status payloads, repeated same-origin messages, and the actual Holiday producer sequence. Record at least ten repetitions.

**Expected / evaluation:** No incomplete message, stale tail or unexplained disappearance. Buffer-capacity/long-frame defects are recorded with exact bytes and timing; the previously noted Holiday persistence issue is not automatically considered fixed.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### I08 - Video/SPI recovery [EQUIPMENT]

**Steps:** Repeat resets, menu clears, camera switches and video-source reconnects for 15 minutes. Observe the physical SPI pins if a fault occurs.

**Expected / evaluation:** Display recovers to a readable state; no stuck SPTIE wait or missing SPI completion. Capture PC, SPI0CR1/SPI0SR and buffer indices for a hang rather than treating a slow debugger step as proof of failure.

GCC: ____  ICC: ____  Evidence/defect: ____________________

## J. EEPROM compatibility and resets

Back up the EEPROM first. Use normal save/exit for acceptance; controlled corruption tests belong on a spare unit.

### J01 - Save/power-cycle all combinations [CORE]

**Steps:** For each F1/F2 combination, set a different row/column, exit the full menu, wait for save completion, power-cycle and read back.

**Expected / evaluation:** Both functions and both positions return exactly as saved. F1/F2 remain independent, and saved strings have terminators. With the debugger, EEWriteStatus is zero after verified writes.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### J02 - Existing EEPROM migration [EQUIPMENT]

**Steps:** Load a backup representing pre-F1/F2 settings on a spare unit, preserving valid row/column/serial data but old/invalid function fields. Boot normally.

**Expected / evaluation:** Old valid fields remain usable; invalid F1/F2 strings default independently to ON. No unrelated EEPROM location is overwritten by a new ad-hoc record.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### J03 - Cross-compiler settings [CORE]

**Steps:** Save distinctive settings with ImageCraft, program GCC while preserving EEPROM, then boot/read and save a second combination. Reload ImageCraft preserving EEPROM and read back.

**Expected / evaluation:** Both builds agree on row/column/serial and F1/F2 fields. GCC offsets 0x0812..0x0815 and 0x0816..0x0819 match the existing record. Record programmer erase options and any serial commissioning differences.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### J04 - Erased/invalid fields [ENGINEERING]

**Steps:** On a spare, test erased settings marker and separately corrupt only one function field to a non-ON/OFF string. Retain backups.

**Expected / evaluation:** Erased marker selects default function settings and the existing save path. Each invalid function string normalizes without altering a valid other function. Restore the production image/settings afterward.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### J05 - Reset key in both modes [CORE]

**Steps:** In follower mode and controller mode, latch some commands, press Reset and log display/reset traffic. Allow equipment reset time.

**Expected / evaluation:** ButtonBoxData clears to the existing reset/data pattern; titler resets first. Controller mode sends CAN ID 0, DLC 2, data 81 00 then resets locally. Follower mode does not assume the same CAN broadcast/local-reset sequence.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### J06 - Serial protection and defaults [CORE]

**Steps:** Record serial from Status and EEPROM 0x0902..0x0906. Save other settings, restore defaults and reset through each supported route.

**Expected / evaluation:** Serial remains consistent with the existing separate serial policy. Message settings/enable fields do not overlap that record. Document any difference caused by programming/erasing rather than attributing it to menu save.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### J07 - Interrupted save characterization [ENGINEERING]

**Steps:** Only on a spare, use an approved timed power-interruption setup during a save; repeat at several offsets and restore from backup.

**Expected / evaluation:** Record recovery and actual field values. Current format has no atomic transaction/checksum guarantee; do not promise recovery of all old or all new values. Any release requirement beyond current behavior needs a separate design decision.

GCC: ____  ICC: ____  Evidence/defect: ____________________

## K. Communications and service interfaces

CAN runs at the code-configured 125 kbit/s with intentional Node-ID zero. Use an isolated, correctly terminated bench. Confirm serial electrical levels before connecting a PC adapter.

### K01 - Periodic button PDO and selection PDO [EQUIPMENT]

**Steps:** Capture idle and button activity in operational state, then change camera selection.

**Expected / evaluation:** 0x180 carries three bytes: command low, command high, alternate navigation. Nominal event time is 95 ms. 0x421 carries two camera-address bytes with configured 10 ms inhibit. Record actual timing/jitter under scan/menu load.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### K02 - Receive mapping [EQUIPMENT]

**Steps:** Inject valid fixtures on 0x200 (1 byte), 0x310 (8), 0x2A1 (3), 0x280 (1), and 0x500 (8). Verify the intended handler/state for each.

**Expected / evaluation:** External menu, display messages, camera update, C48 switches and HD camera replies reach the correct process-image regions. Battery removal has not disabled shared camera reception.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### K03 - NMT/SDO regression [EQUIPMENT]

**Steps:** On the isolated network, exercise supported NMT start, stop, preoperational, reset-node and reset-communication commands and read known OD entries. Return to normal operational state.

**Expected / evaluation:** NMT state/heartbeat and applicable PDO activity match the existing MCO implementation; communication-reset preserves valid configuration. Known OD responses including 0x1000/0x1018/0x2100 match the compiled values. This is regression coverage, not a conformance certificate.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### K04 - Two-wire waveform [EQUIPMENT]

**Steps:** With an approved receiver/analyzer, exercise individual command bits, alternate modes, Stop, Reset and a held key. Compare captured waveform/decoded value with raw ButtonBoxData.

**Expected / evaluation:** Receiver decodes the intended raw button word. Do not expect the two-wire waveform to contain the third navigation byte used by the CAN PDO. Check idle levels and timing against the established board/receiver specification.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### K05 - Serial commands and line handling [EQUIPMENT]

**Steps:** Use the board's SCI0 interface at the configured rate (1200 baud from the current divisor), 8 data bits/no parity/1 stop bit after confirming hardware. Send HELP, ?, REVISION, lowercase commands, spaces, CR/CRLF and an unknown short command.

**Expected / evaluation:** HELP/? and revision output are recognizable; revision is 3.06. Commands are case-normalized and serial framing is handled without overflow/stuck receive state. Unknown input does not jump or reset unexpectedly.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### K06 - Service RESET, LOAD and GO [ENGINEERING]

**Steps:** On a recoverable spare only, use RESET/#RESET. Test LOAD with approved S1/S9 records targeting a reserved bench RAM area, and GO only to a known return-safe test routine. Keep BDM recovery ready.

**Expected / evaluation:** RESET restarts. Valid record bytes arrive at the approved location; GO executes only the approved routine and returns/ends as designed. No production Flash/EEPROM/stack writes. These legacy commands lack a general safety/validation guarantee.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### K07 - Stop BB / Send BB characterization [ENGINEERING]

**Steps:** Feed the valid producer message sequence containing Stop BB and Send BB. Observe VCI_Flag2, CAN output and two-wire output independently.

**Expected / evaluation:** The parser sets/clears DISABLE_BB in its supported continuation path. No consuming transmitter gate was found in this source review; do not claim actual button suppression unless the bench proves it. Record this as characterization/open requirement.

GCC: ____  ICC: ____  Evidence/defect: ____________________

## L. Robustness, fault injection and final acceptance

These are controlled engineering tests, not instructions to exercise every malformed case on moving equipment. Use backups, current limits, canaries/debugger and an approved fault fixture.

### L01 - CAN disconnect and recovery [ENGINEERING]

**Steps:** During idle, menu display and controller travel, disconnect/reconnect the appropriate bench link. Test transmit failure/bus-off with approved analyzer support.

**Expected / evaluation:** Record local state, controller timeout and recovery. No unexplained corruption. Existing MCO fatal-error recovery may reset/re-enter; do not assume nonblocking graceful recovery or compliance behavior without evidence.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### L02 - Discovery and menu capacity [ENGINEERING]

**Steps:** With debugger/canaries, increase primary units, auxiliary units and reply count around each real allocation. Include more than twelve response IDs only in an instrumented fixture.

**Expected / evaluation:** Detect/report any out-of-bounds access or invalid menu pointer. Legacy response storage is 12 IDs while application loops assume 20; primary menu space is also limited. Passing a small network does not qualify maximum capacities.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### L03 - Empty/full camera list [ENGINEERING]

**Steps:** Instrument empty-list Up/Down shortcuts, nine/ten cameras, then rescan fewer cameras after a full list. Use canaries before executing risky paths.

**Expected / evaluation:** No out-of-range cam_select, stale final slot or menu overwrite is the desired robustness criterion. Existing code has empty-list decrement, final-slot clearing and full-menu capacity risks; failures remain open defects, not waived passes.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### L04 - Trigger outside a device row [ENGINEERING]

**Steps:** On an isolated instrumented unit, press Trigger on root built-in entries and inside Settings/Status; observe target-index calculation before allowing a transmit.

**Expected / evaluation:** No negative/out-of-range device lookup or unintended command is the desired result. Trigger_Device computes a device index from row minus seven without full context checks, so halt/report an invalid index rather than commanding live equipment.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### L05 - Watchdog fault coverage [ENGINEERING]

**Steps:** Use an approved fault-injection harness to stall the main loop with interrupts running and, separately, to stop interrupt progress. Do not equate debugger halt with a software stall if debug freeze is enabled.

**Expected / evaluation:** Record which watchdog resets each failure and measured delay. wdInit is set at startup and no later clear was found; therefore a main-loop-only stall is not assumed covered by the external-watchdog handshake. No forced pass of an unproven watchdog requirement.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### L06 - ADC and generic memory primitives [ENGINEERING]

**Steps:** If those callable service routines are required for release, use a calibrated ADC fixture and reserved RAM/Flash/EEPROM test regions on a spare to invoke ATDGetLevel, FlashWord/EEPROMWord and approved write paths.

**Expected / evaluation:** ADC averages agree with calibration; byte packing covers 00/7F/80/FF; writes/readback preserve neighboring data. ATDGetLevel and generic FlashWrite lack a normal user-menu path: mark not applicable with owner approval if unused.

GCC: ____  ICC: ____  Evidence/defect: ____________________

### L07 - Soak and release reconciliation [CORE]

**Steps:** Run at least two hours of mixed menus, camera changes, messages, function-mode edits and stop/travel commands on the secured bench. Repeat essential acceptance on the second compiler, then restore intended settings.

**Expected / evaluation:** No unexplained reset, memory drift, lost settings or stuck control. Every applicable CORE/EQUIPMENT test has a result, build identity and evidence. ENGINEERING failures receive explicit disposition; unresolved safety/functional defects block unconditional acceptance.

GCC: ____  ICC: ____  Evidence/defect: ____________________

## Appendix A - Physical-button checklist

Run B01 once for each row; record P/F/B/NT/NA independently for each compiler. Values are hexadecimal masks before contextual encoding.

| Index | Button | Mask | Behavior / exception | GCC | ICC |
| --- | --- | --- | --- | --- | --- |
| 00 | Trigger / record | 0x0100 | Momentary; raw F1+Trigger also drives alarm | | |
| 01 | F1 | 0x0080 | Toggle; alternate behavior depends on F1 setting | | |
| 02 | Reverse | 0x8000 | Toggle with direction/stop rules | | |
| 03 | Extra Slow | 0x2000 | Toggle; cleared without travel | | |
| 04 | Down | 0x0001 | Momentary; menu/alternate routing may consume | | |
| 05 | Select | 0x0004 | Momentary; Select/edit/Select workflow | | |
| 06 | Option / data | 0x0400 | Raw word forces this bit; inspect physical input separately | | |
| 07 | F2 | 0x0040 | Toggle; alternate behavior depends on F2 setting | | |
| 08 | Stop | 0x0000 | Special zero entry clears travel; no standalone Stop bit | | |
| 09 | Slow | 0x1000 | Ordinary toggle; enabled alternate Up is momentary | | |
| 10 | Menu | 0x0003 | Combined Down+Up; local or alternate menu routing | | |
| 11 | Rotate CW | 0x0008 | Momentary; alternate Down when enabled | | |
| 12 | Reset | 0x0200 | Reset/display sequence; controller broadcast differs | | |
| 13 | F3 | 0x0020 | Toggle; F2+F3 simultaneous animation remains | | |
| 14 | Forward | 0x4000 | Toggle with direction/stop rules | | |
| 15 | Fast | 0x0800 | Toggle; cleared without travel | | |
| 16 | Up | 0x0002 | Momentary; menu/alternate routing may consume | | |
| 17 | Rotate CCW | 0x0010 | Momentary; startup mode selection when held | | |

## Appendix B - Function-mode matrices

The selected function bit is preserved in the command word in every setting. Clear other function selections before each row.

| F1 | F2 | Expected alternate behavior | GCC | ICC |
| --- | --- | --- | --- | --- |
| ON | ON | F1 and F2 both enable alternate mapping | | |
| ON | OFF | Only F1 enables alternate mapping | | |
| OFF | ON | Only F2 enables alternate mapping | | |
| OFF | OFF | Neither enables alternate mapping; both still latch/transmit | | |

With the selected function enabled:

| Physical key | Third byte | Shared navigation meaning |
| --- | --- | --- |
| Slow | 0x01 | Up |
| Rotate CW | 0x02 | Down |
| Down | 0x04 | Left |
| Up | 0x08 | Right |
| Menu / Down+Up | 0x10 | Menu / Select |

The Repeater Cart Controller determines what these directions mean for rotation/extension. Do not infer actuator direction from the common encoding. With OFF and no other enabled function selected, third-byte navigation is zero; local menu/camera shortcuts still apply in controller mode.

## Appendix C - CAN/debug reference

- Button word = 0x180 data byte 0 OR (byte 1 shifted left 8); byte 2 is alternate navigation.
- Camera address = 0x421 byte 0 OR (byte 1 shifted left 8).
- Example valid status fixture: CAN ID 0x310, DLC 8, data `01 02 50 72 6F 63 3A 54`, followed after normal processing by `01 45 53 54 03 00 00 00`. This is origin 01, STX + Proc: + TEST + ETX. Observe TEST without the prefix. Use no more than the supported concurrent origins.
- Useful watches: ButtonBoxData, TC0_RCVD_Data, tempData, Third_byte, AlternateFunctionMask, F1Enable, F2Enable, VariableFlag, nDigits, StackPointer, MenuStackc, Gen_Flags, controller_mode, legacy_mode, MenuCheck, MenuTimer, ToggleTimer, CntrSwTimeOut, cam_select, cam_list, CharsInMem, EEWriteStatus, SPI0InBufptr, SPI0OutBufptr and watchdog state.
- GCC settings record starts at 0x0800: FirstVar 4 bytes; MessageRow 4; MessageCol 4; SerialNum 6; F1Enable 4 at 0x0812; F2Enable 4 at 0x0816; two trailing padding bytes. Separate commissioning serial is five bytes at 0x0902. ImageCraft retains the original contiguous save mechanism.
- For an unexplained fault save PC, SP, PPAGE, reset cause, CAN trace, settings backup, exact input sequence and compiler image identity before reloading.

## Appendix D - Coverage and limits

| Function area | Procedure/source mapping |
| --- | --- |
| Build/ABI/startup | A01-A07; J03; FixedTypes.h, Controller.c, startup/runtime |
| All 18 keys and LEDs | B01-B06; Appendix A; KeypadMatrix.inc, Interrupts.c |
| Travel and C48 | C01-C07; ButtonInput.c, user.c |
| Menus/editors/serial/defaults | D01-D09; J01-J07; Menu.c, MenuTables.inc, Settings.c |
| F1/F2, recording, animation | E01-E08; ButtonInput.c, Subroutines.c, CharacterFontData.inc |
| Cameras and units | F01-F08; G01-G07; discovery modules, MCO integration |
| External menus and triggers | H01-H05; DeviceMenus.c, menu-state logic |
| Video/message transport | I01-I08; Display.c, bldMessages, SPI ISR |
| CAN/two-wire/SCI/service | K01-K07; legacy MCO, Interrupts.c, Command.c |
| Known limits/faults/endurance | L01-L07; instrumented engineering verification |

Known source-review concerns are intentionally visible in this procedure: response-array size versus scan loop, device/camera menu capacity, empty-camera shortcuts, stale final camera slot/tag copying, trigger indexing outside a valid device row, message-category/persistence behavior, and watchdog/DISABLE_BB assumptions. They are not represented as repaired by the cleanup. Generic ADC/Flash/service routines have no ordinary operator path; release applicability must be explicit.

## Disposition and sign-off

GCC: P ___ F ___ B ___ NT ___ NA ___   ICC: P ___ F ___ B ___ NT ___ NA ___
Open defect IDs and owner-approved exclusions: __________________________________
Restored production settings/addresses verified by: __________________ Date: __________
Tester: __________________ Reviewer/engineer: __________________ Date: __________
Acceptance: [ ] Accepted  [ ] Accepted with listed exclusions  [ ] Rejected / incomplete

Unconditional acceptance requires every applicable case to pass and all engineering findings to have a documented disposition. A missing fixture is blocked/not tested, not passed.
