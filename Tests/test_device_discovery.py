"""Exercise device discovery and menu population without a live CAN stack."""
from pathlib import Path
import os
import re
import subprocess
import tempfile
import unittest
from test_legacy_controls import extract

ROOT = Path(__file__).resolve().parents[1]


class DeviceDiscoveryTests(unittest.TestCase):
    def test_scan_sort_and_resequence(self):
        application = (ROOT/'Embedded/Src/Subroutines.c').read_text()
        start = application.index('char FirstVar[]')
        end = application.index(';', application.index('char Rev[]', start)) + 1
        variables = application[start:end]
        tables = (ROOT/'Embedded/Src/MenuTables.inc').read_text()
        callbacks = sorted(set(re.findall(r'&(\w+)', tables)))
        stubs = '\n'.join('int '+name+'(void) { return 0; }' for name in callbacks)
        module = (ROOT/'Embedded/Src/DeviceDiscovery.c').read_text()
        wait = 'while (Timer1) { /* wait for interrupt */ }'
        self.assertGreater(module.count(wait), 0)
        module = module.replace(wait, 'Timer1 = 0; /* injected timer interrupt */')
        program = r'''
#include <assert.h>
#include <string.h>
#include "Subroutines.h"
#include "Interrupts.h"
#include "mcohw.h"

/* These fixtures isolate the application's existing 20-slot scan logic;
   the separately documented legacy stack capacity mismatch is not repaired. */
int Device_Type_Response_ID[20], Device_Type_Response_ID_ptr;
char Device_Menus[20][17], deviceSubMenu[20][17];
int sub_Device_Type_Response_ID[20];
bool resequenceNodes, AuxDeviceLimitExceeded, AuxDeviceMenuFull;
volatile unsigned int Timer1;
static CAN_MSG shared_message, sent[128];
static unsigned int sends, initial_requests, detail_requests, restores, watchdog;
static unsigned int failures, polls;
static int mode, duplicate_pending, fail_next;
int ExternalMenu(void) { return 0; }
int ExternalSubMenu(void) { return 0; }
void COP_Trig(void)
{
    assert(++watchdog < 1000);
    Timer1 = 0; /* Initial watchdog-serviced wait gets its timer interrupt here. */
}
UNSIGNED8 MCO_ProcessStack(void) { assert(++polls < 1000); return 0; }
void SDO_restore(void) { ++restores; }
void MCOUSER_FatalError(UNSIGNED16 code)
{
    assert(code == 0x8801);
    ++failures;
}
UNSIGNED8 MCOHW_PushMessage(CAN_MSG *message)
{
    unsigned int i;
    assert(message == &shared_message && sends < 128);
    sent[sends].ID = message->ID;
    sent[sends].LEN = message->LEN;
    for (i = 0; i < message->LEN; ++i) sent[sends].BUF[i] = message->BUF[i];
    ++sends;
    if (message->LEN == 2 && message->BUF[0] == 0x33) duplicate_pending = 1;
    if (fail_next) { fail_next = 0; return 0; }
    return 1;
}
void SDO_Init_Upload(int id, int index, char subindex)
{
    assert(index == 0x2100);
    if (subindex == 0) {
        if (initial_requests < 112) assert(id == 0x610 + (int)initial_requests);
        else assert(mode == 2 && id == 0x661 + (int)(initial_requests-112));
        ++initial_requests;
        if (duplicate_pending && id == 0x661) {
            Device_Type_Response_ID[0] = 0x5E3;
            Device_Type_Response_ID[1] = 0x5E7;
            duplicate_pending = 0;
        }
    } else {
        assert(mode != 0 && detail_requests < 8);
        assert(subindex == (char)(detail_requests % 4 + 1));
        assert(id == (mode == 2 ? 0x661 : 0x621) + (int)(detail_requests/4));
        ++detail_requests;
    }
}
''' + variables + '\n' + stubs + r'''
#include "MenuTables.inc"
'''
        for name in ['ext_menu', 'Scan']:
            program += '\n' + extract(application, name).replace('&gTxMsg', '&shared_message')
        program += r'''
static void label(unsigned int slot, const char *name, int auxiliary)
{
    memset(Device_Menus[slot], ' ', 15);
    memcpy(Device_Menus[slot], name, strlen(name));
    Device_Menus[slot][15] = auxiliary ? (char)0xff : 0x32;
    Device_Menus[slot][16] = 0;
}
static void reset(int scenario)
{
    memcpy(Menuc, defaultMenuc, sizeof Menuc);
    memset(Device_Type_Response_ID, 0, sizeof Device_Type_Response_ID);
    memset(Device_Menus, 0, sizeof Device_Menus);
    memset(deviceSubMenu, 0, sizeof deviceSubMenu);
    memset(sub_Device_Type_Response_ID, 0, sizeof sub_Device_Type_Response_ID);
    memset(sent, 0, sizeof sent);
    mode = scenario;
    sends = initial_requests = detail_requests = restores = watchdog = polls = failures = 0;
    duplicate_pending = fail_next = 0;
    Device_Type_Response_ID_ptr = 0;
    resequenceNodes = AuxDeviceLimitExceeded = AuxDeviceMenuFull = false;
    Timer1 = 0;
    if (mode) {
        Device_Type_Response_ID[0] = mode == 2 ? 0x5E1 : 0x5A1;
        Device_Type_Response_ID[1] = mode == 2 ? 0x5E1 : 0x5A2;
        label(0, "MAIN UNIT", 0);
        label(1, "AUX UNIT", 1);
    }
}
static void check_populated(int expected_main, int expected_aux)
{
    assert(restores == 1 && detail_requests == 8);
    assert(Device_Type_Response_ID[0] == expected_main && Device_Type_Response_ID[1] == 0);
    assert(sub_Device_Type_Response_ID[0] == expected_aux);
    assert(!strncmp(deviceSubMenu[0], "AUX UNIT", 8));
    assert(!strncmp(&Menuc[0].Entry[7][1], "MAIN UNIT", 9));
    assert(!strncmp(Menuc[0].Entry[8], " EXIT", 5));
    assert(Menuc[0].FunctPtr[6] == ExternalMenu);
    assert(!strncmp(&Menuc[5].Entry[1][1], "AUX UNIT", 8));
    assert(!strncmp(Menuc[5].Entry[2], " EXIT", 5));
    assert(Menuc[5].FunctPtr[0] == ExternalSubMenu);
    assert(!AuxDeviceLimitExceeded && !AuxDeviceMenuFull);
}
int main(void)
{
    unsigned int i;
    reset(0);
    ext_menu();
    assert(sends == 127 && initial_requests == 0);
    for (i = 0; i < sends; ++i) {
        assert(sent[i].ID == 0x201+i);
        assert(sent[i].LEN == 1 && sent[i].BUF[0] == 1);
    }
    reset(0); fail_next = 1;
    ext_menu();
    assert(sends == 127 && failures == 1);

    reset(0);
    Scan();
    assert(initial_requests == 112 && detail_requests == 0 && sends == 0);
    assert(restores == 1 && !strncmp(Menuc[0].Entry[7], " EXIT", 5));

    reset(1);
    Scan();
    check_populated(0x5A1, 0x5A2);
    assert(initial_requests == 112 && sends == 0 && !resequenceNodes);

    /* Ignore a single IN58 reply before, between or after ordinary units.
       Initial discovery still queries it; no menu-detail requests follow. */
    for (i = 0; i < 3; ++i) {
        unsigned int slot, ordinary = 0;
        reset(1);
        for (slot = 0; slot < 3; ++slot)
            Device_Type_Response_ID[slot] = slot == i ? 0x5BE : 0x5A1 + ordinary++;
        Scan();
        check_populated(0x5A1, 0x5A2);
        assert(initial_requests == 112 && sends == 0 && !resequenceNodes);
    }

    reset(2);
    Scan();
    check_populated(0x5E1, 0x5E2);
    assert(initial_requests == 121 && sends == 5 && resequenceNodes);
    assert(sent[0].ID == 0 && sent[0].LEN == 2);
    assert(sent[0].BUF[0] == 0x33 && sent[0].BUF[1] == 0);
    for (i = 1; i <= 2; ++i) {
        assert(sent[i].ID == 0 && sent[i].LEN == 3);
        assert(sent[i].BUF[0] == 0x33);
        assert(sent[i].BUF[1] == (i == 1 ? 0x63 : 0x67));
        assert(sent[i].BUF[2] == 0x0f+i);
        assert(sent[i+2].ID == 0 && sent[i+2].LEN == 4);
        assert(sent[i+2].BUF[0] == 0x34 && sent[i+2].BUF[1] == 0);
        assert(sent[i+2].BUF[2] == 5 && sent[i+2].BUF[3] == 0xE0+i);
    }

    reset(0);
    for (i = 0; i < 4; ++i) Device_Type_Response_ID[i] = 0x5A1+i;
    label(0, "MAIN ONE", 0); label(1, "AUX ONE", 1);
    label(2, "AUX TWO", 1); label(3, "MAIN TWO", 0);
    menuSort();
    assert(sub_Device_Type_Response_ID[0] == 0x5A2);
    assert(sub_Device_Type_Response_ID[1] == 0x5A3);
    assert(!strncmp(deviceSubMenu[0], "AUX ONE", 7));
    assert(!strncmp(deviceSubMenu[1], "AUX TWO", 7));
    assert(Device_Type_Response_ID[0] == 0x5A1 && Device_Type_Response_ID[1] == 0x5A4);
    assert(!strncmp(Device_Menus[1], "MAIN TWO", 8));
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            folder = Path(directory)
            test = folder/'test.c'
            module_file = folder/'DeviceDiscovery.c'
            binary = folder/('test.exe' if os.name == 'nt' else 'test')
            test.write_text(program)
            module_file.write_text(module)
            sanitizer = [] if os.name == 'nt' else ['-fsanitize=undefined', '-fno-sanitize-recover=all']
            for mode in ['-fsigned-char', '-funsigned-char']:
                subprocess.run([os.environ.get('CC', 'gcc'), '-std=gnu89', '-Wall', '-Wextra',
                                '-Wno-missing-braces', '-DBUTTONBOX_GCC', '-DBUTTONBOX_HOST_SYNTAX', mode,
                                '-I', str(ROOT/'Embedded/Include'), '-I', str(ROOT/'Embedded/Src'),
                                str(test), str(module_file), '-o', str(binary)] + sanitizer, check=True)
                subprocess.run([str(binary)], check=True)


if __name__ == '__main__':
    unittest.main()
