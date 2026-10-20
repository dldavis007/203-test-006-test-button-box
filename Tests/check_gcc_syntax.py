"""Host syntax check of GCC paths only; not a target assembler/linker check."""
from pathlib import Path
import os
import subprocess
ROOT = Path(__file__).resolve().parents[1]
source = ROOT/'Embedded/Src'
files = [source/(name+'.c') for name in
         ['Controller', 'Flash', 'Command', 'EEProm', 'mco', 'user', 'mcohw', 'Interrupts', 'Subroutines']]
files += [ROOT/'Embedded/Src/runtime.c', ROOT/'Embedded/Src/compact_io.c']
subprocess.run([os.environ.get('CC','gcc'), '-std=gnu89', '-fsyntax-only',
                '-DBUTTONBOX_GCC', '-DBUTTONBOX_HOST_SYNTAX',
                '-include', str(ROOT/'Tests/stdio_macro_probe.h'),
                '-Wno-unknown-pragmas', '-w', '-I', str(ROOT/'Embedded/Include')] +
               [str(p) for p in files], check=True)
print('PASS: GCC conditional source paths parse (host syntax only)')
