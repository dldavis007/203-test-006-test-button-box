"""Test diagnostic collection without a cross compiler, debugger or target."""
from pathlib import Path
from types import SimpleNamespace
import importlib.util
import tempfile
import unittest
import zipfile
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('diagnostics', ROOT/'Tools/collect_startup_diagnostics.py')
diag = importlib.util.module_from_spec(spec)
spec.loader.exec_module(diag)


def record(address, data):
    content = bytes([len(data)+3, address >> 8, address & 255]) + bytes(data)
    return 'S1' + (content + bytes([255-(sum(content) & 255)])).hex().upper()


def fixture():
    rows = [('.text_low', 4, 0x4000, 0x4000), ('.text_window', 0, 0x8000, 0x8000),
            ('.text_high', 0, 0xC000, 0xC000), ('.noinit', 20, 0x2000, 0x2000),
            ('.data', 4, 0x2014, 0xE000), ('.bss', 0x100, 0x2018, 0x2018),
            ('.vectors', 128, 0xFF80, 0xFF80)]
    headers = '\n'.join('%d %s %08x %08x %08x 00001000 2**0\nCONTENTS, ALLOC, LOAD' %
                        (i, *row) for i, row in enumerate(rows))
    symbols = '00004000 00000002 T _start\n00004002 00000002 T main\n00002118 A __bss_end\n'
    records = '\n'.join([record(0x4000, [0x10, 0x20, 0x30, 0x40]),
                         record(0xE000, [1, 2, 3, 4]), record(0xFF80, [0x40, 0] * 64)])
    return headers, symbols, records


class StartupDiagnosticsTests(unittest.TestCase):
    def test_image_report_checks_reset_and_initializer_bytes(self):
        headers, symbols, records = fixture()
        image = diag.inspect_image(headers, symbols, records)
        self.assertEqual(image['issues'], [])
        self.assertEqual(image['reset_vector'], 0x4000)
        self.assertEqual(image['stack_bytes'], 0x4000 - 0x2118)
        self.assertEqual(image['symbols']['main']['address'], 0x4002)
        self.assertEqual(image['symbols']['_start']['flash_bytes'], '10 20 30 40')
        wrong_reset = records.splitlines()[:2] + [record(0xFF80, [0xFF] * 128)]
        image = diag.inspect_image(headers, symbols, '\n'.join(wrong_reset))
        self.assertIn('Reset vector does not match _start', image['issues'])
        missing_data = '\n'.join([records.splitlines()[0], records.splitlines()[2]])
        self.assertTrue(any('.data' in issue for issue in diag.inspect_image(headers, symbols, missing_data)['issues']))

    def test_srecords_reject_bad_checksums_and_conflicting_writes(self):
        valid = record(0x4000, [1, 2, 3])
        with self.assertRaises(ValueError):
            diag.srecords(valid[:-2] + '00')
        with self.assertRaises(ValueError):
            diag.srecords(valid + '\n' + record(0x4000, [4]))

    def test_collector_only_builds_and_retains_separate_artifacts(self):
        headers, symbols, records = fixture()
        commands = []
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root/'build').mkdir()
            def capture(command, cwd):
                command = list(map(str, command))
                commands.append(command)
                if command[:3] == ['git', 'worktree', 'add']:
                    Path(command[-2]).mkdir(parents=True)
                    return ''
                if command[1:2] == ['--version']:
                    return 'test toolchain'
                if '-h' in command:
                    return headers
                if '-S' in command:
                    return symbols
                if '-d' in command:
                    return '4000: startup'
                raise AssertionError(command)
            def build(command, cwd, **kwargs):
                commands.append(list(map(str, command)))
                self.assertEqual(Path(command[1]).name, 'build_9s12.py')
                out = Path(cwd)/'build/9s12'
                out.mkdir(parents=True)
                (out/'buttonbox.elf').write_bytes(b'ELF fixture')
                (out/'buttonbox.s19').write_text(records)
                (out/'buttonbox.map').write_text('separate map')
                return SimpleNamespace(returncode=0)
            with patch.object(diag, 'ROOT', root), patch.object(diag, 'capture', side_effect=capture), \
                 patch.object(diag.subprocess, 'run', side_effect=build):
                self.assertEqual(diag.main([]), 0)
            out = root/'build/startup-diagnosis'
            for label, _ in diag.CASES:
                self.assertTrue((out/label/'buttonbox.elf').exists())
                self.assertTrue((out/label/'buttonbox.map').exists())
            self.assertTrue((out/'report.json').exists())
            with zipfile.ZipFile(out/'startup-diagnostics.zip') as archive:
                self.assertIn('working/buttonbox.elf', archive.namelist())
                self.assertIn('regression/buttonbox.elf', archive.namelist())
                self.assertFalse(any('worktrees/' in name or '.git' in name for name in archive.namelist()))
            self.assertFalse(any('NoICE12' in ' '.join(command) for command in commands))


if __name__ == '__main__':
    unittest.main()
