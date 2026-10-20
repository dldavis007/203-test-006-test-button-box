"""Host syntax check of GCC paths only; not a target assembler/linker check."""
from pathlib import Path
import os
import subprocess
ROOT = Path(__file__).resolve().parents[1]
source = ROOT/'Rev 3.05'
files = [source/(name+'.c') for name in
         ['Controller', 'Flash', 'Command', 'EEProm', 'mco', 'user', 'mcohw', 'Interrupts', 'Subroutines']]
files += [ROOT/'embedded/gcc/runtime.c']
subprocess.run([os.environ.get('CC','gcc'), '-std=gnu89', '-fsyntax-only',
                '-DBUTTONBOX_GCC', '-DBUTTONBOX_HOST_SYNTAX',
                '-include', str(ROOT/'tests/stdio_macro_probe.h'),
                '-Wno-unknown-pragmas', '-w', '-I', str(source)] +
               [str(p) for p in files], check=True)
print('PASS: GCC conditional source paths parse (host syntax only)')
