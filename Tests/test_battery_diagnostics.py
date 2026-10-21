"""Check the real battery module with scripted replies and no physical CAN."""
from pathlib import Path
import os
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class BatteryDiagnosticsTests(unittest.TestCase):
    def test_requests_status_and_selection(self):
        program = r'''
#include <assert.h>
#include <string.h>
#include "Subroutines.h"
#include "Interrupts.h"
#include "mcohw.h"

char BattNum[20][4], BattSerNum[] = "  -  -   ";
char BattSOC[] = "---", BattSOH[] = "---", BattErr[] = "   OK";
char BatteryTemps[] = "B:---,C1:---,C2:---";
unsigned char ptrDispBatt;
volatile char UpdateMenu;
volatile unsigned int Timer2;
UNSIGNED8 gCANFilter, gProcImg[PROCIMG_SIZE];
static unsigned int step, remaps, battery_count, expected_index, filter_start;
static int expect_clear, alarm;
static const unsigned char serials[2][4] = {{1,2,0,3},{4,5,0,6}};
static const unsigned int identifiers[3] = {0x020,0x009,0x007};

static void assert_selected(void)
{
    unsigned int i;
    for (i = 0; i < 4; ++i)
        assert(gProcImg[IN_digi_13+i] ==
               (battery_count ? serials[expected_index][i] : 0));
}
void MCO_InitRPDO(UNSIGNED8 number, UNSIGNED16 identifier,
                  UNSIGNED8 length, UNSIGNED8 offset)
{
    assert(number == 6 && length == 8 && offset == OUT_digi_21);
    assert(remaps < 3 && identifier == identifiers[remaps]);
    /* Preserve the application's filter decrement before each reconfiguration. */
    assert(gCANFilter == filter_start-1);
    ++gCANFilter; /* Model the stack allocating the replacement receive filter. */
    ++remaps;
}
UNSIGNED8 MCO_ProcessStack(void)
{
    unsigned int i;
    assert(++step < 30); /* Turn an unexpected wait into a deterministic failure. */
    if (expect_clear) {
        assert_selected();
        assert(gProcImg[IN_digi_13+4] == 2);
        assert(gProcImg[IN_digi_13+5] == 0x3c);
        assert(gProcImg[IN_digi_13+6] == 0);
        assert(gProcImg[IN_digi_13+7] == 7);
        expect_clear = 0;
        step = 0;
        Timer2 = 0;
        return 0;
    }
    if (remaps == 0 && step <= 3) {
        assert(gProcImg[IN_digi_5] == 0x81);
        for (i = 1; i < 8; ++i) assert(gProcImg[IN_digi_5+i] == 0);
        if (step <= battery_count)
            for (i = 0; i < 4; ++i) gProcImg[OUT_digi_13+i] = serials[step-1][i];
    } else if (remaps == 0) {
        assert_selected();
        assert(gProcImg[IN_digi_13+7] == 3);
        gProcImg[OUT_digi_21+4] = 90;
        gProcImg[OUT_digi_21+5] = 85;
    } else if (remaps == 1) {
        assert_selected();
        assert(gProcImg[IN_digi_13+7] == 0x64);
        gProcImg[OUT_digi_21] = alarm ? 1 : 0;
        gProcImg[OUT_digi_21+1] = gProcImg[OUT_digi_21+3] = 0;
        gProcImg[OUT_digi_21+2] = 65;
    } else if (remaps == 2) {
        assert_selected();
        assert(gProcImg[IN_digi_13+7] == 3);
        gProcImg[OUT_digi_21] = 60;
        gProcImg[OUT_digi_21+1] = 70;
    } else assert(0);
    Timer2 = 0; /* Replace interrupt-driven timeout, never alter firmware waits. */
    return 0;
}
static void reset(unsigned int batteries, unsigned int selected)
{
    memset(gProcImg, 0, sizeof gProcImg);
    memset(BattNum, 0x55, sizeof BattNum);
    ptrDispBatt = (unsigned char)selected;
    battery_count = batteries;
    expected_index = selected < batteries ? selected : 0;
    step = remaps = 0;
    filter_start = gCANFilter = 10;
    expect_clear = alarm = 0;
    UpdateMenu = 0;
}
static void check_scan(void)
{
    unsigned int i;
    assert(remaps == 3 && gCANFilter == filter_start);
    assert(ptrDispBatt == expected_index);
    assert(gProcImg[IN_digi_5] == 0 && gProcImg[IN_digi_13+7] == 0);
    assert(!strcmp(BattErr, alarm ? "Error" : "   OK"));
    assert(!strcmp(BatteryTemps, "B: 25,C1: 20,C2: 30"));
    assert(!strcmp(BattSOH, " 90") && !strcmp(BattSOC, " 85"));
    assert(!strcmp(BattSerNum, !battery_count ? "00-00-000" :
                   expected_index ? "04-05-006" : "01-02-003"));
    for (i = battery_count; i < 20; ++i)
        assert(BattNum[i][0] == 0 && BattNum[i][1] == 0 &&
               BattNum[i][2] == 0 && BattNum[i][3] == 0);
}
int main(void)
{
    reset(2,1);
    Scan_Batteries();
    check_scan();
    reset(2,19); /* Stale selection resets to first discovered battery. */
    Scan_Batteries();
    check_scan();
    reset(2,0);
    expected_index = 1;
    assert(NextBatt() == 0);
    check_scan();
    assert(UpdateMenu == 1);
    reset(2,19); /* Next wraps before rescanning. */
    assert(NextBatt() == 0);
    check_scan();
    assert(UpdateMenu == 1);
    reset(2,1);
    memcpy(BattNum[1], serials[1], 4);
    expect_clear = alarm = 1;
    assert(ClrBattErr() == 0);
    check_scan();
    assert(UpdateMenu == 1);
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
                                '-Werror', '-DBUTTONBOX_GCC', '-DBUTTONBOX_HOST_SYNTAX', mode,
                                '-I', str(ROOT/'Embedded/Include'), str(source),
                                str(ROOT/'Embedded/Src/BatteryDiagnostics.c'),
                                str(ROOT/'Embedded/Src/compact_io.c'), '-o', str(binary)]
                               + sanitizer, check=True)
                subprocess.run([str(binary)], check=True)


if __name__ == '__main__':
    unittest.main()
