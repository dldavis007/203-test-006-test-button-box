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
    def test_expected_linker_symbols_are_not_missing_runtime_hooks(self):
        symbols = " U __bss_size\n U __bss_start\n U __data_load\n U __data_size\n U __data_start\n U fstat\n"
        self.assertEqual(build.unresolved_runtime_symbols(symbols), " U fstat")
        self.assertEqual(build.unresolved_runtime_symbols(" U __bss_start\n"), "")
        self.assertIn('unexpected', build.unresolved_runtime_symbols(' U unexpected'))

    def test_default_near_call_probe(self):
        build.check_near_assembly("bb_probe:\n jsr bb_external\n addd #1\n rts\n")
        with self.assertRaises(RuntimeError):
            build.check_near_assembly(" call bb_external,0\n rtc\n")
        with self.assertRaises(RuntimeError):
            build.check_near_assembly(" rts\n")

    def test_sections_parser(self):
        info = build.sections('''  0 .startup 00000020 00000000 00000000 00000034 2**1
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  1 .text.foo 00000010 00000000 00000000 00000054 2**0
                  CONTENTS, ALLOC, LOAD, READONLY, CODE''')
        self.assertEqual(info[0][:3], ('.startup', 32, 2))
        self.assertEqual(info[1][:3], ('.text.foo', 16, 1))

    def test_fixed_banks_and_static_window(self):
        flags = 'CONTENTS, ALLOC, LOAD, READONLY, CODE'
        info = [('.startup', 32, 2, flags)] + [('.text.fn%d'%i, 4000, 2, flags) for i in range(6)]
        banks = build.layout(info)
        self.assertEqual(sum(map(len, banks)), 6)
        script = build.linker_script(banks)
        self.assertIn('ORIGIN = 0x4000', script)
        self.assertIn('ORIGIN = 0xC000', script)
        self.assertIn('ORIGIN = 0x8000', script)
        self.assertNotIn('ASSERT(', script)

    def test_reported_41kb_image_can_use_static_window(self):
        flags = 'ALLOC, LOAD, READONLY, CODE'
        info = [('.startup', 52, 2, flags)] + [('.text.fn%d' % i, 1000, 2, flags) for i in range(41)]
        banks = build.layout(info)
        self.assertEqual(len(banks), 3)
        self.assertEqual(sum(map(len, banks)), 41)
        for bank in banks:
            self.assertLessEqual(len(bank), 16)

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

class FinalMemoryTests(unittest.TestCase):
    def headers(self, **changes):
        rows = {'.text_low': (0x2000,0x4000,0x4000),
                '.text_window': (0x2000,0x8000,0x8000),
                '.text_high': (0x2000,0xC000,0xC000),
                '.noinit': (20,0x2000,0x2000),
                '.data': (0x100,0x2014,0xE000),
                '.bss': (0x1700,0x2114,0x2114),
                '.vectors': (128,0xFF80,0xFF80)}
        rows.update(changes)
        return '\n'.join('%d %s %08x %08x %08x 00000000 2**0' %
                         (i,n,z,v,l) for i,(n,(z,v,l)) in enumerate(rows.items()))

    def test_valid_image(self):
        build.validate_final_image(self.headers())

    def test_reject_overflow_vectors_and_overlap(self):
        for change in [{'.text_window': (0x4001,0x8000,0x8000)},
                       {'.vectors': (126,0xFF80,0xFF80)},
                       {'.data': (0x100,0x2014,0xFE80)},
                       {'.bss': (0x2000,0x2114,0x2114)},
                       {'.data': (0x100,0x2000,0xE000)}]:
            with self.subTest(change=change), self.assertRaises(RuntimeError):
                build.validate_final_image(self.headers(**change))

class PruningLinkTests(unittest.TestCase):
    @unittest.skipUnless(build.shutil.which('ld') and build.shutil.which('objdump') and build.shutil.which('gcc'), 'GNU host tools required')
    def test_full_link_prunes_before_layout_without_relocatable_gc(self):
        import subprocess
        with tempfile.TemporaryDirectory() as d:
            folder = Path(d)
            (folder/'input.c').write_text(
                'volatile int sink; int dead_data = 9; int dead_bss; int used(void) { return 7; }\n'
                'int unused(void) { return 8; }\n'
                'void _start(void) __attribute__((section(".startup")));\n'
                'void _start(void) { sink = used(); }\n'
                'char vectors[128] __attribute__((section(".vectors")));\n'
                'char persistent[20] __attribute__((section(".noinit")));\n')
            subprocess.run(['gcc','-fno-pie','-fno-asynchronous-unwind-tables','-ffunction-sections',
                            '-fdata-sections','-c','input.c','-o','merged.o'],cwd=folder,check=True)
            info = build.sections(subprocess.check_output(['objdump','-h','merged.o'],cwd=folder,text=True))
            script = build.pruning_linker_script(info)
            self.assertNotIn('ADDR(', script)
            self.assertNotRegex(script, r'\bAT\s*\(')
            self.assertNotIn('ASSERT(', script)
            # Same section/GC script, using host machine format for this test.
            script = script.replace('OUTPUT_FORMAT("elf32-m68hc12")','').replace('OUTPUT_ARCH(m68hc12)','')
            (folder/'pruning.ld').write_text(script)
            subprocess.run(['ld','--gc-sections','-T','pruning.ld','merged.o','-o','pruning.probe'],cwd=folder,check=True)
            live = build.sections(subprocess.check_output(['objdump','-h','pruning.probe'],cwd=folder,text=True))
            names = {row[0] for row in live}
            self.assertIn('.startup', names)
            self.assertIn('.text.used', names)
            self.assertIn('.vectors', names)
            self.assertNotIn('.text.unused', names)
            self.assertNotIn('.data.dead_data', names)
            self.assertNotIn('.bss.dead_bss', names)

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
            self.assertIn('LOAD \"' + str(folder/'buttonbox.elf') + '\" 03DH:0000H', session)
            self.assertIn('R PG 03DH', session)
            self.assertEqual(noice.noice_hex(0x8123), '03DH:08123H')
            self.assertEqual(noice.noice_hex(0xC123), '0C123H')
            self.assertNotIn('\nGO', session)
            self.assertIn('W Third_byte', session)
            self.assertTrue((folder/'buttonbox_reconnect.noi').exists())

if __name__ == '__main__':
    unittest.main()
