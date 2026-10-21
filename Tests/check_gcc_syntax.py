"""Host syntax check of GCC paths only; not a target assembler/linker check."""
from pathlib import Path
import os
import subprocess
ROOT = Path(__file__).resolve().parents[1]
source = ROOT/'Embedded/Src'
files = [source/(name+'.c') for name in
         ['Controller', 'Flash', 'Command', 'EEProm', 'mco', 'user', 'mcohw', 'Interrupts', 'Subroutines', 'Settings']]
files += [ROOT/'Embedded/Src/runtime.c', ROOT/'Embedded/Src/compact_io.c']
legacy_names = {'mco.c', 'user.c', 'mcohw.c'}
common = [os.environ.get('CC', 'gcc'), '-std=gnu89', '-ffreestanding', '-fsyntax-only',
          '-Wall', '-Wextra', '-Wno-pointer-to-int-cast', '-Wno-int-to-pointer-cast',
          '-DBUTTONBOX_GCC', '-DBUTTONBOX_HOST_SYNTAX',
          '-include', str(ROOT/'Tests/stdio_macro_probe.h'),
          '-Wno-unknown-pragmas', '-I', str(ROOT/'Embedded/Include')]
# Keep legacy integration diagnostics visible without rewriting stack semantics
# to satisfy -Werror. Application cleanup still gets strict warning checks.
subprocess.run(common + ['-Werror'] + [str(p) for p in files if p.name not in legacy_names], check=True)
subprocess.run(common + [str(p) for p in files if p.name in legacy_names], check=True)
print('PASS: strict application syntax; legacy MCO syntax checked with warnings visible')
