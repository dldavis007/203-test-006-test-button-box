"""Link the real renderer and check its MAX7456 character encoding.

Physical SPI routines are linked but not executed by these host tests.
"""
from pathlib import Path
import os
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class DisplayModuleTests(unittest.TestCase):
    def test_character_encoding_in_actual_module(self):
        program = r'''
#include <assert.h>
#include "Subroutines.h"
#include "Interrupts.h"
#include "mco.h"

/* Supply the module's existing application-owned state without target I/O. */
volatile uint8_t SPI0InBuf[2], SPI0OutBuf[2];
volatile int16_t SPI0InBufptr, SPI0OutBufptr;
volatile unsigned int Timer1;
volatile char Gen_Flags;
volatile bool legacy_mode;
bool CharsInMem, firstWriteSPI = true;
char plusSymbol[54], DispBuf[DispBufLen];
uint8_t MenuCheck, VCI_Flag2;
int16_t DispBufptr;
char MessageRow[4] = "  1", MessageCol[4] = "  1";
UNSIGNED8 gProcImg[PROCIMG_SIZE];
UNSIGNED8 MCO_ProcessStack(void) { assert(0); return 0; }
#include "CharacterFontData.inc"

static uint8_t translate(uint16_t input)
{
    char character = (char)input;
    return (uint8_t)DispTranslate(&character);
}
int main(void)
{
    uint16_t i;
    static const uint8_t symbols[][2] = {
        {' ',0x00}, {'(',0x3f}, {')',0x40}, {'.',0x41}, {'?',0x42},
        {';',0x43}, {':',0x44}, {',',0x45}, {0x27,0x46}, {'/',0x47},
        {0x22,0x48}, {'-',0x49}, {'<',0x4a}, {'>',0x4b}, {'@',0x4c},
        {0x5c,0x4d}, {'[',0x4e}, {']',0x4f}, {'&',0x50}, {'^',0x51},
        {'+',0x52}, {'=',0x53}, {'_',0x54}, {'{',0x55}, {'}',0x56},
        {'*',0x57}, {'!',0x58}, {'%',0x59}, {'$',0x5a}, {'#',0x5b},
        {'|',0x5c}, {0x11,0x5d}, {0x12,0x5e}, {0x13,0x5f},
        {0x03,0x00}, {0x02,0x00}, {0x04,0x3a}, {0x06,0x4b}, {0x01,0x00}
    };
    assert(translate('0') == 0x0a);
    for (i = 1; i <= 9; ++i) assert(translate('0'+i) == i);
    for (i = 0; i < 26; ++i) {
        assert(translate('A'+i) == 0x0b+i);
        assert(translate('a'+i) == 0x25+i);
    }
    for (i = 0; i < sizeof symbols/sizeof symbols[0]; ++i)
        assert(translate(symbols[i][0]) == symbols[i][1]);
    for (i = 128; i < 256; ++i) assert(translate(i) == i);
    assert(translate(0) == 0 && translate(5) == 5 && translate(0x7f) == 0x7f);
    assert(sizeof CMDI == 19*54);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            folder = Path(directory)
            source = folder/'test.c'
            binary = folder/('test.exe' if os.name == 'nt' else 'test')
            source.write_text(program)
            sanitizer = [] if os.name == 'nt' else ['-fsanitize=undefined', '-fno-sanitize-recover=all']
            for mode in ['-fsigned-char', '-funsigned-char']:
                subprocess.run([os.environ.get('CC', 'gcc'), '-std=gnu89', '-Wall', '-Wextra',
                                '-Wno-parentheses', '-DBUTTONBOX_GCC', '-DBUTTONBOX_HOST_SYNTAX', mode,
                                '-I', str(ROOT/'Embedded/Include'), '-I', str(ROOT/'Embedded/Src'),
                                str(source), str(ROOT/'Embedded/Src/Display.c'),
                                '-o', str(binary)] + sanitizer, check=True)
                subprocess.run([str(binary)], check=True)


if __name__ == '__main__':
    unittest.main()
