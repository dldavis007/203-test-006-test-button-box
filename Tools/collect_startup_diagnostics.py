"""Build the known-working and reverted firmware without starting NoICE.

Run on the Windows machine with the original GNU68HC12 toolchain. Separate
Git worktrees preserve the user's current sources and each generated image.
"""
from pathlib import Path
import argparse
import difflib
import json
import os
import re
import shutil
import subprocess
import sys
import zipfile

ROOT = Path(__file__).resolve().parents[1]
CASES = [('working', '65ced39d268d532984f195e218ece2bd5e5eb556'),
         ('regression', '35b2044cd5d68e3502f9189d742162aa5add7119')]
ARTIFACTS = ['buttonbox.elf', 'buttonbox.s19', 'buttonbox.map', 'merged.map',
             'final_sections.txt', 'sections.txt', 'retained_sections.txt',
             'buttonbox_debug.noi', 'pruning.map']


def capture(command, cwd):
    result = subprocess.run(list(map(str, command)), cwd=cwd, text=True,
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    if result.returncode:
        raise RuntimeError(result.stdout.strip())
    return result.stdout


def section_rows(text):
    lines = text.splitlines()
    rows = {}
    for i, line in enumerate(lines):
        match = re.match(r'\s*\d+\s+(\S+)\s+([\da-fA-F]+)\s+([\da-fA-F]+)\s+([\da-fA-F]+)\s+', line)
        if match:
            rows[match[1]] = dict(size=int(match[2], 16), vma=int(match[3], 16),
                                  lma=int(match[4], 16), flags=lines[i+1].strip() if i+1 < len(lines) else '')
    return rows


def srecords(text):
    """Decode physical load addresses and reject bad checksums/conflicting data."""
    memory = {}
    for number, line in enumerate(text.splitlines(), 1):
        line = line.strip()
        if not line:
            continue
        if len(line) < 4 or line[0] != 'S':
            raise ValueError('Invalid S-record at line %d' % number)
        raw = bytes.fromhex(line[2:])
        if not raw or raw[0] != len(raw)-1 or sum(raw) & 255 != 255:
            raise ValueError('S-record length/checksum at line %d' % number)
        width = {'1': 2, '2': 3, '3': 4}.get(line[1])
        if width is None:
            continue
        if len(raw) < width + 2:
            raise ValueError('Short S-record at line %d' % number)
        address = int.from_bytes(raw[1:1+width], 'big')
        for offset, value in enumerate(raw[1+width:-1]):
            here = address + offset
            if here in memory and memory[here] != value:
                raise ValueError('Conflicting data at 0x%X' % here)
            memory[here] = value
    return memory


def inspect_image(headers, symbols, records):
    rows = section_rows(headers)
    memory = srecords(records)
    names = {}
    for line in symbols.splitlines():
        match = re.match(r'^([\da-fA-F]+)\s+(?:[\da-fA-F]+\s+)?[A-Za-z]\s+(\S+)\s*$', line)
        if match:
            names[match[2]] = int(match[1], 16)
    result = dict(sections=rows, symbols={}, issues=[])
    if '.bss' in rows:
        end = rows['.bss']['vma'] + rows['.bss']['size']
        result['stack_bytes'] = 0x4000 - end
        result['stack_margin_bytes'] = result['stack_bytes'] - 1280
        if result['stack_bytes'] < 1280:
            result['issues'].append('Less than 1280 bytes remain for stack')
    if '.noinit' in rows and (rows['.noinit']['vma'], rows['.noinit']['size']) != (0x2000, 20):
        result['issues'].append('Persistent reset-marker placement changed')
    for name in ['_start', 'main', '__data_load', '__data_start', '__data_size',
                 '__bss_start', '__bss_size', '_bss_end', 'Keypad_Matrix',
                 'defaultMenuStackc', 'interrupt_vectors', 'F1Enable', 'F2Enable',
                 'TC3_Int_Handler', 'RTI_Int_Handler', '_.frame']:
        if name not in names:
            continue
        address = names[name]
        peek = bytes(memory[address+i] for i in range(16) if address+i in memory)
        result['symbols'][name] = dict(address=address, flash_bytes=peek.hex(' '))
    if 0xFFFE in memory and 0xFFFF in memory:
        result['reset_vector'] = memory[0xFFFE] * 256 + memory[0xFFFF]
        if result['reset_vector'] != names.get('_start'):
            result['issues'].append('Reset vector does not match _start')
    else:
        result['issues'].append('Reset vector missing from S-record')
    for name in ['.text_low', '.text_window', '.text_high', '.vectors', '.data']:
        if name in rows:
            row = rows[name]
            absent = sum(address not in memory for address in range(row['lma'], row['lma']+row['size']))
            if absent:
                result['issues'].append('%s has %d bytes absent from S-record' % (name, absent))
    if any(address > 0xFFFF for address in memory):
        result['issues'].append('S-record contains extended addresses; current session expects logical 16-bit addresses')
    return result


def tool(name):
    prefix = os.environ.get('GNU68HC12_PREFIX', 'm6811-elf-')
    filename = prefix + name + ('.exe' if os.name == 'nt' else '')
    path = Path(os.environ.get('GNU68HC12_BIN', r'C:\Toolchains\GNU68HC12\bin')) / filename
    return str(path) if path.is_file() else prefix + name


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--plan', action='store_true', help='show cases without creating worktrees or compiling')
    args = parser.parse_args(argv)
    if args.plan:
        for label, commit in CASES:
            print('%s: %s (build only; no NoICE or device programming)' % (label, commit))
        return 0
    output = ROOT/'build/startup-diagnosis'
    output.mkdir(parents=True, exist_ok=True)
    manifest = dict(cases={}, tool_versions={})
    for name in ['gcc', 'ld']:
        try:
            manifest['tool_versions'][name] = capture([tool(name), '--version'], ROOT)
        except (OSError, RuntimeError) as error:
            if name == 'gcc':
                raise
            manifest['tool_versions'][name] = 'Version command unavailable: %s' % error
    for label, commit in CASES:
        tree = output/'worktrees'/label
        artifacts = output/label
        artifacts.mkdir(parents=True, exist_ok=True)
        for filename in ['symbols.txt', 'startup_disassembly.txt']:
            stale = artifacts/filename
            if stale.exists():
                stale.unlink()
        if tree.exists():
            head = capture(['git', 'rev-parse', 'HEAD'], tree).strip()
            dirty = capture(['git', 'status', '--porcelain', '--untracked-files=no'], tree).strip()
            if head != commit or dirty:
                raise RuntimeError('Diagnostic worktree differs from the requested clean commit: %s' % tree)
        else:
            tree.parent.mkdir(parents=True, exist_ok=True)
            capture(['git', 'worktree', 'add', '--detach', str(tree), commit], ROOT)
        print('Building %s (%s); no target loading' % (label, commit[:7]), flush=True)
        with (artifacts/'build.log').open('w') as log:
            build = subprocess.run([sys.executable, str(tree/'Tools/build_9s12.py')], cwd=tree,
                                   stdout=log, stderr=subprocess.STDOUT)
        manifest['cases'][label] = dict(commit=commit, build_exit=build.returncode)
        for filename in ARTIFACTS:
            source = tree/'build/9s12'/filename
            destination = artifacts/filename
            if source.exists():
                shutil.copyfile(source, destination)
            elif destination.exists():
                destination.unlink()  # Only a previous diagnostic copy, never source/user files.
        if build.returncode == 0 and (artifacts/'buttonbox.elf').exists():
            elf = artifacts/'buttonbox.elf'
            headers = capture([tool('objdump'), '-h', elf], ROOT)
            symbols = capture([tool('nm'), '-n', '-S', elf], ROOT)
            (artifacts/'symbols.txt').write_text(symbols)
            (artifacts/'startup_disassembly.txt').write_text(capture(
                [tool('objdump'), '-d', '--start-address=0x4000', '--stop-address=0x4050', elf], ROOT))
            manifest['cases'][label]['image'] = inspect_image(headers, symbols, (artifacts/'buttonbox.s19').read_text())
    (output/'report.json').write_text(json.dumps(manifest, indent=2)+'\n')
    lines = ['Button-box startup comparison', 'No NoICE session or target programming was performed.',
             'Working: 65ced39; reverted regression: 35b2044.', '']
    for label, data in manifest['cases'].items():
        lines.append('%s: build exit %d' % (label, data['build_exit']))
        image = data.get('image')
        if not image:
            lines.append('  See %s/build.log; build did not produce a usable image.' % label)
            continue
        lines.append('  Stack bytes: %s; margin above unchanged 1280-byte reserve: %s' %
                     (image.get('stack_bytes'), image.get('stack_margin_bytes')))
        lines.append('  Reset vector: 0x%04X' % image.get('reset_vector', 0))
        for name, row in image['symbols'].items():
            lines.append('  %s = 0x%04X; S-record bytes: %s' % (name, row['address'], row['flash_bytes'] or '(RAM or linker size)'))
        lines.extend('  ISSUE: '+issue for issue in image['issues'])
        lines.append('')
    before, after = output/'working/startup_disassembly.txt', output/'regression/startup_disassembly.txt'
    if before.exists() and after.exists():
        lines.append('Startup disassembly differences (changed initialization addresses can be legitimate):')
        lines.extend(difflib.unified_diff(before.read_text().splitlines(), after.read_text().splitlines(),
                                          fromfile='working', tofile='regression', lineterm=''))
    (output/'report.txt').write_text('\n'.join(lines)+'\n')
    with zipfile.ZipFile(output/'startup-diagnostics.zip', 'w', zipfile.ZIP_DEFLATED) as archive:
        for filename in ['report.txt', 'report.json']:
            archive.write(output/filename, filename)
        for label, _ in CASES:
            for filename in ARTIFACTS + ['build.log', 'symbols.txt', 'startup_disassembly.txt']:
                path = output/label/filename
                if path.exists():
                    archive.write(path, label+'/'+filename)
    print('Collected: %s' % output)
    print('Share startup-diagnostics.zip for diagnosis; worktrees and Git metadata are excluded.')
    return 0 if all(case['build_exit'] == 0 and not case.get('image', {}).get('issues')
                    for case in manifest['cases'].values()) else 1


if __name__ == '__main__':
    try:
        sys.exit(main())
    except (OSError, RuntimeError, ValueError) as error:
        print('[ERROR] %s' % error, file=sys.stderr)
        sys.exit(1)
