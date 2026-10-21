"""Verify active CAN registration after retiring the battery feature."""
from pathlib import Path
import os
import subprocess
import tempfile
import unittest
from test_legacy_controls import extract

ROOT = Path(__file__).resolve().parents[1]


class RetiredBatteryTests(unittest.TestCase):
    def test_reset_registers_only_camera_button_and_menu_pdos(self):
        reset = extract((ROOT/'Embedded/Src/user.c').read_text(), 'MCOUSER_ResetCommunication')
        program = r'''
#include <assert.h>
#include <string.h>
#include "mco.h"
#define INTR_OFF() ((void)0)
#define INTR_ON() ((void)0)
UNSIGNED8 gProcImg[PROCIMG_SIZE], setFilters[8];
char clearProc = 1;
static unsigned int receives, transmits, initialized;
static const unsigned int rx[][4] = {
    {1,0x200,1,OUT_digi_0}, {2,0x310,8,OUT_digi_1},
    {3,0x2A1,3,OUT_digi_9}, {4,0x280,1,OUT_digi_12},
    {7,0x500,8,OUT_digi_13}
};
static const unsigned int tx[][6] = {
    {1,0x421,0,10,2,IN_digi_3}, {2,0x180,95,0,3,IN_digi_0}
};
void MCO_Init(UNSIGNED16 baud, UNSIGNED8 node, UNSIGNED16 heartbeat)
{
    assert(baud == 125 && node == 0 && heartbeat == 0);
    receives = transmits = 0;
    ++initialized;
}
void MCO_InitRPDO(UNSIGNED8 n, UNSIGNED16 id, UNSIGNED8 length, UNSIGNED8 offset)
{
    const unsigned int *expected;
    assert(receives < sizeof rx/sizeof rx[0]);
    expected = rx[receives++];
    assert(n == expected[0] && id == expected[1]);
    assert(length == expected[2] && offset == expected[3]);
}
void MCO_InitTPDO(UNSIGNED8 n, UNSIGNED16 id, UNSIGNED16 event,
                   UNSIGNED16 inhibit, UNSIGNED8 length, UNSIGNED8 offset)
{
    const unsigned int *expected;
    assert(transmits < sizeof tx/sizeof tx[0]);
    expected = tx[transmits++];
    assert(n == expected[0] && id == expected[1]);
    assert(event == expected[2] && inhibit == expected[3]);
    assert(length == expected[4] && offset == expected[5]);
}
''' + reset + r'''
int main(void)
{
    unsigned int i;
    memset(gProcImg, 0xA5, sizeof gProcImg);
    MCOUSER_ResetCommunication();
    assert(initialized == 1 && receives == 5 && transmits == 2);
    assert(clearProc == 0);
    for (i = 0; i < PROCIMG_SIZE; ++i) assert(gProcImg[i] == 0);
    for (i = 0; i < 8; ++i) assert(setFilters[i] == 0x80);
    memset(gProcImg, 0xA5, sizeof gProcImg);
    MCOUSER_ResetCommunication();
    assert(initialized == 2 && receives == 5 && transmits == 2);
    for (i = 0; i < PROCIMG_SIZE; ++i) assert(gProcImg[i] == 0xA5);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory)/'test.c'
            binary = Path(directory)/('test.exe' if os.name == 'nt' else 'test')
            source.write_text(program)
            sanitizer = [] if os.name == 'nt' else ['-fsanitize=undefined', '-fno-sanitize-recover=all']
            for mode in ['-fsigned-char', '-funsigned-char']:
                subprocess.run([os.environ.get('CC', 'gcc'), '-std=gnu89', '-Wall', '-Wextra',
                                '-Werror', '-DBUTTONBOX_GCC', '-DBUTTONBOX_HOST_SYNTAX', mode,
                                '-I', str(ROOT/'Embedded/Include'), str(source), '-o', str(binary)]
                               + sanitizer, check=True)
                subprocess.run([str(binary)], check=True)


if __name__ == '__main__':
    unittest.main()
