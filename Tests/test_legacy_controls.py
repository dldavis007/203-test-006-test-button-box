"""Exercise real parser helpers and timer LED scanning without target hardware."""
from pathlib import Path
import os
import re
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


def extract(source, name):
    match = re.search(r'^(?:void|int|char) ' + name + r'\s*\([^;]*?\)\s*\{', source, re.M)
    start, pos, depth = match.start(), match.end(), 1
    while depth:
        depth += (source[pos] == '{') - (source[pos] == '}')
        pos += 1
    return source[start:pos]


def run_c(program):
    with tempfile.TemporaryDirectory() as directory:
        source = Path(directory)/'test.c'
        binary = Path(directory)/('test.exe' if os.name == 'nt' else 'test')
        source.write_text(program)
        sanitizer = [] if os.name == 'nt' else ['-fsanitize=undefined', '-fno-sanitize-recover=all']
        for mode in ['-fsigned-char', '-funsigned-char']:
            subprocess.run([os.environ.get('CC', 'gcc'), '-std=gnu89', '-Wall', '-Wextra',
                            '-Wno-parentheses', mode] + sanitizer + [str(source), '-o', str(binary)], check=True)
            subprocess.run([str(binary)], check=True)


class LegacyControlsTests(unittest.TestCase):
    def test_command_token_consumption(self):
        source = (ROOT/'Embedded/Src/Command.c').read_text()
        helpers = '\n'.join(extract(source, name) for name in ['ltrim', 'parse', 'ucase', 'hextoi'])
        run_c('#include <string.h>\n#include <assert.h>\n' + helpers + r'''
int main(void) {
    char command[64] = "   revision   go 4000\r\n";
    char token[64], spaces[] = "   ", empty[] = "";
    ucase(command);
    parse(token, command); assert(!strcmp(token, "REVISION"));
    parse(token, command); assert(!strcmp(token, "GO"));
    parse(token, command); assert(!strcmp(token, "4000"));
    assert(hextoi(token) == 0x4000 && !command[0]);
    ltrim(spaces); assert(!spaces[0]);
    ltrim(empty); assert(!empty[0]);
    parse(token, command); assert(!token[0]);
    assert(hextoi("1aFZ") == 0x1af);
    return 0;
}
''')

    def test_timer_scan_stays_inside_matrix(self):
        source = (ROOT/'Embedded/Src/Interrupts.c').read_text()
        handler = extract(source, 'TC3_Int_Handler')
        run_c(r'''
#include <assert.h>
#define TC_1ms 1
#define TeleData_Rev 0x8000
#define TeleData_Fwd 0x4000
struct Matrix { unsigned int BB_Data; unsigned char LED_Port_Data[3]; };
struct Matrix Keypad_Matrix[18];
unsigned int ButtonBoxData = 0xffff, TC3, TCNT;
unsigned char TFLG1, PTP, PTT, PTJ, AlternateFunctionMask = 0xc0;
int legacy_mode;
void MCOHW_TimerISR(void) {}
''' + handler + r'''
int main(void) {
    int i;
    for (i = 0; i < 18; ++i) Keypad_Matrix[i].BB_Data = 1;
    for (i = 0; i < 1000; ++i) TC3_Int_Handler();
    return 0;
}
''')


if __name__ == '__main__':
    unittest.main()
