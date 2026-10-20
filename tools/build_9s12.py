"""Build the complete button-box application with GNU68HC12, not host GCC."""
from pathlib import Path
import os
import json
import re
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'Rev 3.05'
STATIC_PAGE = json.loads((ROOT/'embedded/gcc/target.json').read_text())['static_ppage']
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
    """Pack near-call code into fixed low, static-page window and fixed high.

    Startup selects one page for the whole run; there are no banked calls.
    Flash configuration bytes and vector space remain excluded.
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
    remaining = [0x4000 - startup_size - 4, 0x4000 - 4, 0x3F00 - data_size - 4]
    banks = [[], [], []]
    for name, size, align in sorted(executable, key=lambda x: x[1], reverse=True):
        cost = size + align - 1
        fits = [i for i in (0, 1, 2) if remaining[i] >= cost]
        if not fits:
            raise RuntimeError('Firmware does not fit static-page Flash; no firmware/session generated. '
                               'Needs further footprint reduction or a multi-page banked-memory port. '
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
  window (rx) : ORIGIN = 0x8000, LENGTH = 0x4000
  high (rx) : ORIGIN = 0xC000, LENGTH = 0x3F00
  vectors (rx) : ORIGIN = 0xFF80, LENGTH = 0x80
}
SECTIONS {
  .text_low : {
    KEEP(merged.o(.startup))
%s
  } > low
  .text_window : {
%s
  } > window
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
''' % (entries(banks[0]), entries(banks[1]), entries(banks[2]))


def pruning_linker_script(info):
    """A diagnostic full link permits GC on old ld without combining it with -r.

    Keep output section names so the retained set can be mapped back to merged.o.
    Its temporary contiguous Flash map is never used for the delivered firmware.
    """
    allocated = [(n, z, a, f) for n, z, a, f in info if z and 'ALLOC' in f and not n.startswith(('.note', '.eh_frame'))]
    code = [n for n, z, a, f in allocated if n.startswith(('.text', '.rodata', '.tramp'))]
    data = [n for n, z, a, f in allocated if n.startswith(('.data', '.sdata'))]
    bss = [n for n, z, a, f in allocated if n.startswith(('.bss', '.sbss', '.softregs'))]
    known = set(code + data + bss + ['.startup', '.vectors', '.noinit'])
    unknown = [n for n, z, a, f in allocated if n not in known]
    if unknown:
        raise RuntimeError('Unmapped allocated sections in pruning pass: ' + ', '.join(unknown))
    lines = ['OUTPUT_FORMAT("elf32-m68hc12")', 'OUTPUT_ARCH(m68hc12)',
             'ENTRY(_start)', 'SECTIONS {',
             ' .startup 0x4000 : { KEEP(merged.o(.startup)) }']
    lines += [' %s : { merged.o(%s) }' % (name, name) for name in code]
    lines += [' . = ALIGN(2); __data_load = .;',
              ' .noinit 0x2000 (NOLOAD) : { KEEP(merged.o(.noinit)) }',
              ' . = ALIGN(2); __data_start = .;']
    # This image is never executed: initialized data can have LMA == RAM VMA.
    # In old ld, AT(ADDR(section)) may crash when GC removes that section.
    lines += [' %s : { merged.o(%s) }' % (name, name) for name in data]
    lines += [' . = ALIGN(2); __data_size = . - __data_start;',
              ' __bss_start = .;']
    lines += [' %s (NOLOAD) : { merged.o(%s) }' % (name, name) for name in bss]
    lines += [' . = ALIGN(2); __bss_size = . - __bss_start;',
              ' .vectors 0xFF80 : { KEEP(merged.o(.vectors)) }',
              ' .stab 0 : { *(.stab) }', ' .stabstr 0 : { *(.stabstr) }',
              ' /DISCARD/ : { *(.comment) *(.note*) *(.eh_frame*) }',
              '}']
    return '\n'.join(lines) + '\n'


def check_near_assembly(text):
    # Old GNU68HC12 accepts -mlong-calls but not its negative spelling.
    # Verify the default instead of passing an unsupported option.
    instructions = '\n'.join(line.split(';', 1)[0] for line in text.splitlines())
    if re.search(r'^\s*(?:call|rtc)\b', instructions, re.M | re.I):
        raise RuntimeError('Compiler defaults to far calls; fixed-bank runtime requires JSR/RTS.')
    if not re.search(r'^\s*rts\b', instructions, re.M | re.I):
        raise RuntimeError('Could not verify near-call return convention in compiler probe.')
    if not re.search(r'^\s*(?:jsr|bsr|lbsr)\b', instructions, re.M | re.I):
        raise RuntimeError('Could not verify near-call instruction in compiler probe.')


def unresolved_runtime_symbols(text):
    # These addresses/sizes are deliberately supplied by the final linker script.
    linker_defined = {'__bss_size', '__bss_start', '__data_load',
                      '__data_size', '__data_start'}
    return '\n'.join(line for line in text.splitlines()
                     if line.split() and line.split()[-1] not in linker_defined)


def main():
    BUILD.mkdir(parents=True, exist_ok=True)
    # Remove stale deliverables before any compiler/compile/link failure.
    for pattern in ('*.elf', '*.s19', '*.noi', '*.map', '*.o', '*.ld', '*.probe'):
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
    flags = ['-m68hc12', '-mshort', '-fshort-double',
             '-std=gnu89', '-ffreestanding', '-Os', '-gstabs+', '-fsigned-char', '-fno-common',
             '-fno-strict-aliasing', '-ffunction-sections', '-fdata-sections',
             '-DBUTTONBOX_GCC', '-DBUTTONBOX_STATIC_PAGE=0x%02X' % STATIC_PAGE, '-Wall', '-W', '-I', str(SOURCE)]
    probe = BUILD/'call_abi_probe.c'
    probe.write_text('extern unsigned int bb_external(unsigned int);\n'
                     'unsigned int bb_probe(unsigned int x) { return bb_external(x) + 1; }\n')
    print(run([gcc] + flags + ['-S', probe, '-o', BUILD/'call_abi_probe.s']))
    check_near_assembly((BUILD/'call_abi_probe.s').read_text())
    objects = []
    files = [SOURCE/(name+'.c') for name in SOURCES] + [ROOT/'embedded/gcc/runtime.c', ROOT/'embedded/gcc/compact_io.c', ROOT/'embedded/gcc/startup.S']
    for source in files:
        obj = BUILD/(source.stem+'.o')
        compile_flags = list(flags)
        if source.name == 'mco.c':
            # GCC 3.3.6 HC12 spills fail in optimized MCO_ProcessStack
            # (umulqihi3/D_OR_S_REGS). Keep the workaround local to this TU.
            compile_flags.append('-O0')
            print('Using -O0 for mco.c to avoid the GCC 3.3.6 register-spill bug.')
        print(run([gcc] + compile_flags + ['-c', source.name, '-o', obj], cwd=source.parent))
        objects.append(obj)
    # Resolve libc/libgcc into a relocatable object so their real section sizes
    # participate in fixed-bank layout (rather than guessing library footprint).
    print(run([gcc] + flags + ['-nostdlib', '-Wl,-r,-d', '-Wl,-Map,merged.map', '-o', 'merged.o'] +
              objects + ['-Wl,--start-group', '-lc', '-lgcc', '-Wl,--end-group'], cwd=BUILD))
    undefined = unresolved_runtime_symbols(run([tool('nm'), '-u', BUILD/'merged.o'])).strip()
    if undefined:
        raise RuntimeError('Unresolved symbols before final link:\n' + undefined)
    section_output = run([tool('objdump'), '-h', BUILD/'merged.o'])
    (BUILD/'sections.txt').write_text(section_output)
    info = sections(section_output)
    (BUILD/'pruning.ld').write_text(pruning_linker_script(info))
    print(run([gcc] + flags + ['-nostdlib', '-Wl,-T,pruning.ld', '-Wl,--gc-sections',
                              '-Wl,-Map,pruning.map', 'merged.o', '-o', 'pruning.probe'], cwd=BUILD))
    retained_output = run([tool('objdump'), '-h', BUILD/'pruning.probe'])
    (BUILD/'retained_sections.txt').write_text(retained_output)
    retained = {name for name, size, align, attrs in sections(retained_output)
                if size and 'ALLOC' in attrs}
    info = [entry for entry in info if entry[0] in retained]
    (BUILD/'pruning.probe').unlink()
    rom_bytes = sum(size for name, size, align, attrs in info
                    if 'ALLOC' in attrs and name.startswith(('.text', '.rodata', '.data', '.sdata', '.startup', '.vectors')))
    print('Post-pruning code/constants/data/vector bytes:', rom_bytes)
    print('Section report: build/9s12/sections.txt; library map: build/9s12/merged.map')
    (BUILD/'buttonbox.ld').write_text(linker_script(layout(info)))
    print(run([gcc] + flags + ['-nostdlib', '-Wl,-T,buttonbox.ld', '-Wl,--gc-sections', '-Wl,-Map,buttonbox.map',
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
        for pattern in ('*.elf', '*.s19', '*.noi', '*.probe'):
            for file in BUILD.glob(pattern):
                file.unlink()
        print('[ERROR]', exc, file=sys.stderr)
        sys.exit(1)
