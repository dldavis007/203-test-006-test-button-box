"""Build-layout checks and fail-closed behavior without a cross-compiler."""
import importlib.util
from pathlib import Path
import unittest
from unittest.mock import patch
import tempfile

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('build_9s12', ROOT/'tools/build_9s12.py')
build = importlib.util.module_from_spec(spec)
spec.loader.exec_module(build)

class LayoutTests(unittest.TestCase):
    def test_sections_parser(self):
        info = build.sections('''  0 .startup 00000020 00000000 00000000 00000034 2**1
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  1 .text.foo 00000010 00000000 00000000 00000054 2**0
                  CONTENTS, ALLOC, LOAD, READONLY, CODE''')
        self.assertEqual(info[0][:3], ('.startup', 32, 2))
        self.assertEqual(info[1][:3], ('.text.foo', 16, 1))

    def test_fixed_banks_only(self):
        flags = 'CONTENTS, ALLOC, LOAD, READONLY, CODE'
        info = [('.startup', 32, 2, flags)] + [('.text.fn%d'%i, 4000, 2, flags) for i in range(6)]
        banks = build.layout(info)
        self.assertEqual(sum(map(len, banks)), 6)
        script = build.linker_script(banks)
        self.assertIn('ORIGIN = 0x4000', script)
        self.assertIn('ORIGIN = 0xC000', script)
        self.assertNotIn('ORIGIN = 0x8000', script)
        self.assertIn('__data_load + __data_size <= 0xFF00', script)
        self.assertIn('__bss_end <= 0x3B00', script)
        self.assertIn('SIZEOF(.vectors) == 128', script)

    def test_reject_overflow_or_unknown_sections(self):
        flags = 'ALLOC, LOAD, CODE'
        base = [('.startup', 32, 2, flags)]
        with self.assertRaises(RuntimeError):
            build.layout(base + [('.text.huge', 0x8000, 2, flags)])
        with self.assertRaises(RuntimeError):
            build.layout(base + [('.unexpected', 10, 2, flags)])
        with self.assertRaises(RuntimeError):
            build.layout(base + [('.page0', 10, 2, flags)])
        with self.assertRaises(RuntimeError):
            build.layout([('.text.foo', 10, 2, flags)])

class FailureTests(unittest.TestCase):
    def test_missing_compiler_removes_stale_outputs(self):
        with tempfile.TemporaryDirectory() as d:
            directory = Path(d)
            for name in ['buttonbox.elf', 'buttonbox.s19', 'buttonbox_debug.noi']:
                (directory/name).write_text('old build')
            with patch.object(build, 'BUILD', directory), patch.object(build.shutil, 'which', return_value=None):
                with self.assertRaisesRegex(RuntimeError, 'compiler not found'):
                    build.main()
            self.assertEqual(list(directory.iterdir()), [])

class NoICEMapTests(unittest.TestCase):
    def test_session_loads_current_elf_and_stays_halted(self):
        spec = importlib.util.spec_from_file_location('noice', ROOT/'tools/stabs_to_noice.py')
        noice = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(noice)
        stabs = ("0 SO 0 0 00000000 0 Controller.c\n"
                 "1 FUN 0 0 00004020 0 main:F1\n"
                 "2 SLINE 0 25 00000000 0 \n"
                 "3 FUN 0 0 00000010 0 \n")
        symbols = "\n".join("00002000 B " + name for name, _ in noice.ESSENTIAL_WATCHES)
        with tempfile.TemporaryDirectory() as d:
            folder = Path(d)
            args = ['stabs_to_noice.py', 'm6811-elf-objdump', str(folder/'buttonbox.elf'),
                    str(ROOT/'Rev 3.05/Controller.c'), str(folder/'buttonbox.noi'),
                    str(folder/'buttonbox_debug.noi')]
            with patch.object(noice.sys, 'argv', args), patch.object(noice.subprocess, 'check_output', side_effect=[stabs, symbols]):
                self.assertEqual(noice.main(), 0)
            session = (folder/'buttonbox_debug.noi').read_text()
            self.assertIn('HALT', session)
            self.assertIn('LOAD "' + str(folder/'buttonbox.elf') + '"', session)
            self.assertIn('R PC _start', session)
            self.assertNotIn('\nGO', session)
            self.assertIn('W Third_byte', session)
            self.assertTrue((folder/'buttonbox_reconnect.noi').exists())

if __name__ == '__main__':
    unittest.main()
