"""Exercise camera discovery with scripted replies; no physical CAN or display."""
from pathlib import Path
import os
import re
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class CameraDiscoveryTests(unittest.TestCase):
    def test_discovery_commands_tags_menu_and_refresh(self):
        application = (ROOT/'Embedded/Src/Subroutines.c').read_text()
        start = application.index('char FirstVar[]')
        end = application.index(';', application.index('char Rev[]', start)) + 1
        variables = application[start:end]
        tables = (ROOT/'Embedded/Src/MenuTables.inc').read_text()
        callbacks = sorted(set(re.findall(r'&(\w+)', tables)) - {'findCameras'})
        stubs = '\n'.join('int '+name+'(void) { return 0; }' for name in callbacks)
        # Preserve all module code except the plain interrupt-driven delay used
        # after randomization. Stack-polling waits use the scripted replies below.
        module = (ROOT/'Embedded/Src/CameraDiscovery.c').read_text()
        wait = 'while (Timer2) { /* wait for interrupt */ }'
        self.assertEqual(module.count(wait), 1)
        module = module.replace(wait, 'Timer2 = 0; /* injected timer interrupt */')
        program = r'''
#include <assert.h>
#include <string.h>
#include "Subroutines.h"
#include "Interrupts.h"
#include "mcohw.h"

int cam_list[10], cam_select;
char extCamTagList[10][11];
bool vid18_addr, vid18_update;
volatile char UpdateMenu;
volatile unsigned int Timer2;
UNSIGNED8 gProcImg[PROCIMG_SIZE];
static CAN_MSG shared_message;
static unsigned int randomizations, scans, polls, failures, watchdog, displayed;
static unsigned char last_command;
static int with_cameras, accept_message = 1;
int CameraMenu(void) { return 0; }
void COP_Trig(void) { ++watchdog; }
void showMessage(char text[], int row)
{
    assert(row == 1 && !strcmp(text, "Proc:                         "));
    ++displayed;
}
void MCOUSER_FatalError(UNSIGNED16 code)
{
    assert(code == 0x8801);
    ++failures;
}
UNSIGNED8 MCOHW_PushMessage(CAN_MSG *message)
{
    assert(message == &shared_message); /* Retain the caller-owned buffer. */
    assert(message->ID == 0x521 && message->LEN == 1);
    last_command = message->BUF[0];
    if (last_command == 1) ++randomizations;
    else { assert(last_command == 2); ++scans; polls = 0; }
    return (UNSIGNED8)accept_message;
}
UNSIGNED8 MCO_ProcessStack(void)
{
    if (last_command == 1) return 0;
    assert(last_command == 2 && ++polls < 10);
    if (polls == 1) return 0; /* Call before the receive window starts. */
    if (with_cameras && polls == 2) {
        gProcImg[OUT_digi_9] = 0x34;
        gProcImg[OUT_digi_9+1] = 0x12;
    } else if (with_cameras && polls == 3) {
        gProcImg[OUT_digi_13] = 0x18;
        gProcImg[OUT_digi_13+1] = 0x45;
        gProcImg[OUT_digi_13+2] = 0x23;
        memcpy(&gProcImg[OUT_digi_13+3], "HD CA", 5);
    } else if (with_cameras && polls == 4) {
        gProcImg[OUT_digi_13] = 0x18;
        memcpy(&gProcImg[OUT_digi_13+1], "MERA  ", 6);
    } else Timer2 = 0;
    return 0;
}
/* Use the same bridge bodies as Subroutines.c, supplied below. */
''' + variables + '\n' + stubs + r'''
#include "MenuTables.inc"
'''
        from test_legacy_controls import extract
        for name in ['scan_cameras', 'addressCamera']:
            bridge = extract(application, name).replace('&gTxMsg', '&shared_message')
            program += '\n' + bridge
        program += r'''
static void reset(int cameras)
{
    memcpy(Menuc, defaultMenuc, sizeof Menuc);
    memset(cam_list, 0, sizeof cam_list);
    memset(extCamTagList, 0, sizeof extCamTagList);
    memset(gProcImg, 0, sizeof gProcImg);
    cam_select = 7;
    vid18_addr = vid18_update = false;
    UpdateMenu = 0;
    randomizations = scans = polls = failures = watchdog = displayed = 0;
    accept_message = 1;
    with_cameras = cameras;
    Timer2 = 0;
}
static void check_mixed_scan(void)
{
    unsigned int i;
    assert(scans == 1 && cam_select == 0);
    assert(cam_list[0] == 0x1234 && cam_list[1] == 0x2345);
    for (i = 2; i < 10; ++i) assert(cam_list[i] == 0);
    assert(gProcImg[IN_digi_3] == 0x34 && gProcImg[IN_digi_3+1] == 0x12);
    assert(gProcImg[OUT_digi_9] == 0 && gProcImg[OUT_digi_9+1] == 0);
    assert(gProcImg[OUT_digi_13] == 0 && vid18_addr == false);
    assert(!strncmp(&Menuc[3].Entry[2][1], "CAMERA 1", 8));
    assert(!strncmp(&Menuc[3].Entry[3][1], "HD CAMERA  ", 11));
    assert(!strncmp(Menuc[3].Entry[4], " EXIT", 5));
    assert(Menuc[3].FunctPtr[1] == CameraMenu);
    assert(Menuc[3].FunctPtr[2] == CameraMenu);
    assert(UpdateMenu == 1 && displayed == 1 && watchdog >= 4);
    for (i = 0; i < sizeof extCamTagList; ++i)
        assert(((char *)extCamTagList)[i] == 0);
}
int main(void)
{
    reset(1);
    scan_cameras();
    check_mixed_scan();
    assert(randomizations == 0);

    reset(1);
    assert(findCameras() == 1);
    check_mixed_scan();
    assert(randomizations == 1 && !vid18_update);

    reset(1);
    vid18_update = true;
    strcpy(Menuc[3].Entry[7], " OLD CAMERA ENTRY   ");
    Menuc[3].FunctPtr[6] = CameraMenu;
    assert(findCameras() == 1);
    check_mixed_scan();
    assert(randomizations == 0 && !vid18_update);
    assert(!strncmp(Menuc[3].Entry[7], "                    ", 20));
    assert(Menuc[3].FunctPtr[6] == ExitMenu);

    reset(0);
    vid18_update = true;
    assert(findCameras() == 0);
    assert(cam_select == 0 && cam_list[0] == 0);
    assert(gProcImg[IN_digi_3] == 0 && gProcImg[IN_digi_3+1] == 0);
    assert(UpdateMenu == 1 && displayed == 1);
    assert(!strncmp(Menuc[3].Entry[2], " EXIT", 5));

    reset(0);
    assert(duplicate_add() == 0); /* Zero slots are not duplicates. */
    cam_list[0] = 0x1234; cam_list[9] = 0x2345;
    assert(duplicate_add() == 0);
    cam_list[9] = cam_list[0];
    assert(duplicate_add() == 1);

    reset(0); accept_message = 0;
    addressCamera();
    assert(randomizations == 1 && failures == 1);
    scan_cameras();
    assert(scans == 1 && failures == 2);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            folder = Path(directory)
            test = folder/'test.c'
            module_file = folder/'CameraDiscovery.c'
            binary = folder/('test.exe' if os.name == 'nt' else 'test')
            test.write_text(program)
            module_file.write_text(module)
            sanitizer = [] if os.name == 'nt' else ['-fsanitize=undefined', '-fno-sanitize-recover=all']
            for mode in ['-fsigned-char', '-funsigned-char']:
                subprocess.run([os.environ.get('CC', 'gcc'), '-std=gnu89', '-Wall', '-Wextra',
                                '-Wno-missing-braces', '-DBUTTONBOX_GCC', '-DBUTTONBOX_HOST_SYNTAX', mode,
                                '-I', str(ROOT/'Embedded/Include'), '-I', str(ROOT/'Embedded/Src'),
                                str(test), str(module_file), str(ROOT/'Embedded/Src/compact_io.c'),
                                '-o', str(binary)] + sanitizer, check=True)
                subprocess.run([str(binary)], check=True)


if __name__ == '__main__':
    unittest.main()
