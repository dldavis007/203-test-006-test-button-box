"""Build the complete button-box application with GNU68HC12, not host GCC."""
from pathlib import Path
import os
import re
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'Rev 3.05'
BUILD = ROOT / 'build' / '9s12'
SOURCES = ['Controller', 'Flash', 'Command', 'EEProm', 'mco', 'user', 'mcohw', 'Interrupts', 'Subroutines']


def run(args, **kwargs):
    print('+', subprocess.list2cmdline([str(x) for x in args]), flush=True)
    return subprocess.check_output([str(x) for x in args], text=True, **kwargs)


def sections(text):
    """Parse objdump -h; retain sizes and next-line flags."""
    lines = text.splitlines()
    result = []
    for i, line in enumerate(lines):
        m = re.match(r'\s*\d+\s+(\S+)\s+([0-9a-fA-F]+)\s+[0-9a-fA-F]+\s+[0-9a-fA-F]+\s+[0-9a-fA-F]+\s+2\*\*(\d+)', line)
        if m:
            result.append((m[1], int(m[2], 16), 1 << int(m[3]), lines[i+1].strip()))
    if not result:
        raise RuntimeError('Could not parse object sections.')
    return result


def layout(info):
    """Pack near-call code/const sections into the two fixed Flash banks.

    Do not allocate anything in the PPAGE window (0x8000-0xBFFF), Flash
    configuration bytes (0xFF00-0xFF0F), or vector area (0xFF80-0xFFFF).
    """
    executable = []
    data_size = 0
    startup_size = 0
    for name, size, align, flags in info:
        if not size or 'ALLOC' not in flags:
            continue
        if name == '.startup':
            startup_size = size
        elif name == '.vectors' or name == '.noinit':
            continue
        elif name.startswith(('.text', '.rodata', '.tramp')):
            executable.append((name, size, align))
        elif name.startswith(('.data', '.sdata')):
            data_size += size + align - 1
        elif name.startswith(('.bss', '.sbss', '.softregs')):
            continue
        elif name == '.page0':
            raise RuntimeError('Unexpected page0 allocation would overlap device registers.')
        else:
            raise RuntimeError('Unmapped allocated section: ' + name)
    if not startup_size:
        raise RuntimeError('Startup section missing.')
    # Conservative alignment slack for data initializer placement.
    remaining = [0x4000 - startup_size - 4, 0x3F00 - data_size - 4]
    banks = [[], []]
    for name, size, align in sorted(executable, key=lambda x: x[1], reverse=True):
        cost = size + align - 1
        fits = [i for i in (0, 1) if remaining[i] >= cost]
        if not fits:
            raise RuntimeError('Firmware does not fit fixed Flash; no firmware/session generated. '
                               'Needs a reviewed banked-memory port or footprint reduction. '
                               'Section %s: %d bytes; remaining %s' % (name, size, remaining))
        bank = max(fits, key=lambda i: remaining[i])
        banks[bank].append(name)
        remaining[bank] -= cost
    return banks


def linker_script(banks):
    def entries(names):
        return '\n'.join('    merged.o(%s)' % n for n in names)
    return '''OUTPUT_FORMAT("elf32-m68hc12")
OUTPUT_ARCH(m68hc12)
ENTRY(_start)
MEMORY {
  ram (rw) : ORIGIN = 0x2000, LENGTH = 0x2000
  low (rx) : ORIGIN = 0x4000, LENGTH = 0x4000
  high (rx) : ORIGIN = 0xC000, LENGTH = 0x3F00
  vectors (rx) : ORIGIN = 0xFF80, LENGTH = 0x80
}
SECTIONS {
  .text_low : {
    KEEP(merged.o(.startup))
%s
  } > low
  .text_high : {
%s
    . = ALIGN(2);
    __data_load = .;
  } > high
  .noinit (NOLOAD) : { KEEP(*(.noinit)) . = ALIGN(2); } > ram
  .data : AT(__data_load) {
    __data_start = .;
    *(.data*) *(.sdata*)
    . = ALIGN(2);
    __data_end = .;
  } > ram
  __data_size = SIZEOF(.data);
  .bss (NOLOAD) : {
    __bss_start = .;
    *(.softregs*) *(.bss*) *(.sbss*) *(COMMON)
    . = ALIGN(2);
    __bss_end = .;
  } > ram
  __bss_size = SIZEOF(.bss);
  .vectors : { KEEP(*(.vectors)) } > vectors
  .stab 0 : { *(.stab) }
  .stabstr 0 : { *(.stabstr) }
  .debug_info 0 : { *(.debug_info) }
  .debug_abbrev 0 : { *(.debug_abbrev) }
  .debug_line 0 : { *(.debug_line) }
  .debug_frame 0 : { *(.debug_frame) }
  .debug_str 0 : { *(.debug_str) }
  .debug_loc 0 : { *(.debug_loc) }
  .debug_aranges 0 : { *(.debug_aranges) }
  /DISCARD/ : { *(.comment) *(.note*) *(.eh_frame*) }
  ASSERT(__data_load + __data_size <= 0xFF00, "Flash data initializer overflow")
  ASSERT(__bss_end <= 0x3B00, "Less than 1280 bytes remain for stack")
  ASSERT(SIZEOF(.vectors) == 128, "Incorrect interrupt vector table size")
}
''' % (entries(banks[0]), entries(banks[1]))


def main():
    BUILD.mkdir(parents=True, exist_ok=True)
    # Remove stale deliverables before any compiler/compile/link failure.
    for pattern in ('*.elf', '*.s19', '*.noi', '*.map', '*.o', '*.ld'):
        for file in BUILD.glob(pattern):
            file.unlink()
    toolbin = os.environ.get('GNU68HC12_BIN', r'C:\Toolchains\GNU68HC12\bin')
    exe = '.exe' if os.name == 'nt' else ''
    prefix = os.environ.get('GNU68HC12_PREFIX', 'm6811-elf-')
    def tool(name):
        file = Path(toolbin)/(prefix + name + exe)
        return str(file) if file.is_file() else prefix + name
    gcc = tool('gcc')
    if not shutil.which(gcc):
        raise RuntimeError('9S12 compiler not found: %s. Set GNU68HC12_BIN or PATH.' % gcc)
    print(run([gcc, '--version']))
    flags = ['-m68hc12', '-mshort', '-fshort-double', '-mno-long-calls',
             '-std=gnu89', '-ffreestanding', '-Os', '-gstabs+', '-fsigned-char', '-fno-common',
             '-fno-strict-aliasing', '-ffunction-sections', '-fdata-sections',
             '-DBUTTONBOX_GCC', '-Wall', '-W', '-I', str(SOURCE)]
    objects = []
    files = [SOURCE/(name+'.c') for name in SOURCES] + [ROOT/'embedded/gcc/runtime.c', ROOT/'embedded/gcc/startup.S']
    for source in files:
        obj = BUILD/(source.stem+'.o')
        print(run([gcc] + flags + ['-c', source.name, '-o', obj], cwd=source.parent))
        objects.append(obj)
    # Resolve libc/libgcc into a relocatable object so their real section sizes
    # participate in fixed-bank layout (rather than guessing library footprint).
    print(run([gcc] + flags + ['-nostdlib', '-Wl,-r', '-o', 'merged.o'] +
              objects + ['-Wl,--start-group', '-lc', '-lgcc', '-Wl,--end-group'], cwd=BUILD))
    undefined = run([tool('nm'), '-u', BUILD/'merged.o']).strip()
    if undefined:
        raise RuntimeError('Unresolved symbols before final link:\n' + undefined)
    info = sections(run([tool('objdump'), '-h', BUILD/'merged.o']))
    (BUILD/'buttonbox.ld').write_text(linker_script(layout(info)))
    print(run([gcc] + flags + ['-nostdlib', '-Wl,-T,buttonbox.ld', '-Wl,-Map,buttonbox.map',
                              'merged.o', '-o', 'buttonbox.elf'], cwd=BUILD))
    print(run([tool('objcopy'), '-O', 'srec', BUILD/'buttonbox.elf', BUILD/'buttonbox.s19']))
    print(run([tool('size'), '-A', BUILD/'buttonbox.elf']))
    print(run([tool('objdump'), '-h', '-f', BUILD/'buttonbox.elf']))
    print(run([sys.executable, ROOT/'tools/stabs_to_noice.py', tool('objdump'),
               BUILD/'buttonbox.elf', SOURCE/'Controller.c', BUILD/'buttonbox.noi',
               BUILD/'buttonbox_debug.noi']))
    print('9S12 build complete. Bench validation is required before field use.')
    return 0

if __name__ == '__main__':
    try:
        sys.exit(main())
    except (OSError, subprocess.CalledProcessError, RuntimeError) as exc:
        # Never leave loadable artifacts after any failed build stage.
        for pattern in ('*.elf', '*.s19', '*.noi'):
            for file in BUILD.glob(pattern):
                file.unlink()
        print('[ERROR]', exc, file=sys.stderr)
        sys.exit(1)
