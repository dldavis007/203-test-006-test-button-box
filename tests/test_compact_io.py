"""Compare firmware-used formats against host libc for relevant menu ranges."""
from pathlib import Path
import os
import subprocess
import tempfile
import unittest
ROOT = Path(__file__).resolve().parents[1]

class CompactIOTests(unittest.TestCase):
    def test_firmware_formats(self):
        source = r'''
#include <stdio.h>
#include <string.h>
#include <assert.h>
int BB_sprintf(char *, const char *, ...);
int BB_printf(const char *, ...);
double BB_atof(const char *);
char serial[256]; int serial_count;
int putchar(int c) { serial[serial_count++] = (char)c; return c; }
int main(void) {
    char actual[128], expected[128];
    int i, j;
    const char *decimal[] = {"%1d", "%2d", "%3d", "%4d", "%5d", "%02d"};
    const char *floating[] = {"%#3.1f", "%#4.1f", "%#4.2f", "%#5.2f"};
    for (i = -40; i <= 100; ++i) {
        for (j = 0; j < 6; ++j) {
            assert(BB_sprintf(actual, decimal[j], i) == sprintf(expected, decimal[j], i));
            assert(!strcmp(actual, expected));
        }
        for (j = 0; j < 4; ++j) {
            assert(BB_sprintf(actual, floating[j], (double)i) == sprintf(expected, floating[j], (double)i));
            assert(!strcmp(actual, expected));
        }
    }
    BB_sprintf(actual, "%02d-%02d-%03d", 12, 34, 567);
    assert(!strcmp(actual, "12-34-567"));
    BB_sprintf(actual, "B:%3d,C1:%3d,C2:%3d", -10, 24, 35);
    assert(!strcmp(actual, "B:-10,C1: 24,C2: 35"));
    for (i = 0; i <= 30; ++i) {
        BB_sprintf(actual, "%*s", i, ""); sprintf(expected, "%*s", i, "");
        assert(!strcmp(actual, expected));
    }
    BB_sprintf(actual, "%s\x11", "test"); assert(!strcmp(actual, "test\x11"));
    BB_sprintf(actual, "CAMERA %d           ", 10);
    assert(!strcmp(actual, "CAMERA 10           "));
    BB_sprintf(actual, "%d", -32768); assert(!strcmp(actual, "-32768"));
    BB_sprintf(actual, "%04X", 65535U); assert(!strcmp(actual, "FFFF"));
    assert(BB_atof("  -12.25") == -12.25);
    assert(BB_atof(" 30") == 30);
    assert(BB_atof("OFF") == 0);
    BB_printf("Revision %s\r\n", "3.05"); serial[serial_count] = 0;
    assert(!strcmp(serial, "Revision 3.05\r\n"));
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as d:
            folder = Path(d)
            (folder/'test.c').write_text(source)
            binary = folder/('test.exe' if os.name == 'nt' else 'test')
            subprocess.run([os.environ.get('CC','gcc'), '-std=c89', '-Wall', '-Wextra',
                            str(folder/'test.c'), str(ROOT/'embedded/gcc/compact_io.c'),
                            '-o', str(binary)], check=True)
            subprocess.run([str(binary)], check=True)

if __name__ == '__main__':
    unittest.main()
