# Application fixed-width type migration

## Scope

Application numeric declarations, matching externs, parameters, return types,
callback signatures, structures and casts now use int8_t/uint8_t and
int16_t/uint16_t. FixedTypes.h also provides int32_t/uint32_t. Strings, character
processing, text-menu variables and generic char-based memory interfaces retain
char. Existing double values and the existing bool abstraction remain intact.
No variable definition was moved, reordered, removed or added.

For the older CPU12 compilers, the compatibility header maps 8-bit types to
signed/unsigned char, 16-bit types to signed/unsigned int, and 32-bit types to
signed/unsigned long. Compile-time checks reject the wrong widths and integer
signedness. Host checks use stdint.h. They now exercise the converted 16-bit
objects, but still do not reproduce target integer promotions, pointers, enum
layout, compiler code generation or stack use.

The owner's ICC12 7.08B limits.h confirms CHAR_BIT=8, CHAR_MIN=UCHAR_MIN=0 and
CHAR_MAX=UCHAR_MAX=255. The GCC build retains its existing -fsigned-char flag.
Numeric bytes and hardware masks now have explicit unsigned types; cursor flags
and Trigger_DeviceFlag explicitly use int8_t. The latter corrects the plain-char
-1 release comparison under ImageCraft's unsigned-char default. Text continues
to use the compiler's normal char type.

## Preserved interfaces

The unmodified MCO implementation contains native declarations for Timer1,
Gen_Flags, UpdateDisp and sub_Device_Type_Response_ID. Their application
definitions/declarations retain those native types to preserve compatibility.
Device_Type_Response_ID, its cursor and the process-image types stay as declared
by MCO. The underlying int width is 16 bits on both target compilers.

Compiler/library ABI hooks main, getchar, putchar, _textmode, the GCC compact I/O
and runtime support, startup code and hardware-register headers retain their
required native declarations. MCO source/headers and application PDO setup are
unchanged. These are deliberate boundary exceptions, rather than unfinished
application conversions. No global switch changes plain-char signedness.

## Layout and behavior

FirstVar through NullVar and their saved text strings are byte-for-byte unchanged.
EEPROM codecs retain their existing field offsets and sizes. All global/static
state remains at its original definition location, including retained unused
storage. Matrix field widths and order are preserved; masks/port bytes are now
explicitly unsigned. Initialization values, CAN identifiers/payloads and scan
logic (including the IN58 exclusion) remain unchanged.

For a source-level layout comparison, a temporary copy substituted the target's
native type spellings for fixed-width names. All 13 application units retained
the old host data/BSS section order, sizes, initialized bytes and symbolic
relocations in that comparison. This checks declaration width intent and storage
order; it does not establish an actual CPU12 memory map. The real target build
must still validate RAM/Flash and the minimum 1280-byte stack reserve.

## Validation and acceptance

All 32 host unit tests, both plain-char modes of the F1/F2 regressions, and strict
application syntax checks pass. Tests now use matching fixed-width application
fixtures while retaining native MCO boundary types. Additional tests check
widths/ranges, signed sentinel behavior, all 65,536 high/low byte combinations,
16-bit hex-parser results and trigger release rearming in both char modes.
Legacy MCO diagnostics remain visible; its pinned baseline check passes.

Target GCC/NoICE and ImageCraft compilation and hardware acceptance are pending.
Save/close ImageCraft before pulling so it does not overwrite the updated header
list. Rebuild and load matching firmware/debug data for each compiler, then check:

- Cold startup, watchdog operation, reset and controller/legacy modes.
- Menu entry, held-key repeats, device trigger release/repress and variable editing.
- All four F1/F2 settings, raw transmitted function bits and LED behavior.
- Travel/stop/controller Slow, camera discovery and first/last camera selection.
- Device discovery, CNT-031 handling and IN58 exclusion where equipment is present.
- Settings save/load after power cycle, including existing stored settings.
- The target build's memory validation and stack reserve.
