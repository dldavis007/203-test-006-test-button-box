"""Host syntax check of GCC paths only; not a target assembler/linker check."""
from pathlib import Path
import os
import subprocess
ROOT = Path(__file__).resolve().parents[1]
source = ROOT/'Embedded/Src'
files = [source/(name+'.c') for name in
         ['Controller', 'Flash', 'Command', 'EEProm', 'mco', 'user', 'mcohw', 'Interrupts', 'Subroutines']]
files += [ROOT/'Embedded/Src/runtime.c', ROOT/'Embedded/Src/compact_io.c']
subprocess.run([os.environ.get('CC','gcc'), '-std=gnu89', '-ffreestanding', '-fsyntax-only',
                '-Wall', '-Wextra', '-Werror',
                # Host pointers are 64-bit; firmware int/pointers are both 16-bit.
                # Target width assertions remain enabled in the real GCC build.
                '-Wno-pointer-to-int-cast', '-Wno-int-to-pointer-cast',
                '-DBUTTONBOX_GCC', '-DBUTTONBOX_HOST_SYNTAX',
                '-include', str(ROOT/'Tests/stdio_macro_probe.h'),
                '-Wno-unknown-pragmas', '-I', str(ROOT/'Embedded/Include')] +
               [str(p) for p in files], check=True)
print('PASS: GCC source paths pass strict host warning checks (target pointer-width casts excluded)')
