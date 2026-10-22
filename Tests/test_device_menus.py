"""Exercise actual external-menu command encoding without physical CAN or display."""
from pathlib import Path
import os
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class DeviceMenuTests(unittest.TestCase):
    def test_commands_and_menu_state(self):
        program = r'''
#include <assert.h>
#include <string.h>
#include "Subroutines.h"
#include "Interrupts.h"
#include "mcohw.h"

struct MenuStack MenuStackc[MenuStackSize];
uint8_t StackPointer;
volatile char Gen_Flags;
volatile int16_t ReActivate_Timer;
int Device_Type_Response_ID[20], sub_Device_Type_Response_ID[20];
int16_t cam_list[10], cam_select;
UNSIGNED8 gProcImg[PROCIMG_SIZE];
static CAN_MSG sent;
static uint16_t pushes, clears, deselects, failures;
static UNSIGNED8 accept_message = 1;

UNSIGNED8 MCOHW_PushMessage(CAN_MSG *message)
{
    uint16_t i;
    sent.ID = message->ID;
    sent.LEN = message->LEN;
    /* Only copy transmitted bytes: the firmware leaves unused bytes untouched. */
    for (i = 0; i < message->LEN; ++i) sent.BUF[i] = message->BUF[i];
    ++pushes;
    return accept_message;
}
void MCOUSER_FatalError(UNSIGNED16 error)
{
    assert(error == 0x8801);
    ++failures;
}
void ClearTitler(void) { ++clears; }
void DeSelect(void) { ++deselects; }

static void reset(void)
{
    memset(MenuStackc, 0, sizeof MenuStackc);
    memset(gProcImg, 0xA5, sizeof gProcImg);
    memset(&sent, 0, sizeof sent);
    StackPointer = 2;
    Gen_Flags = Gen_Flags_Menu_Active | Gen_Flags_SCI1Xmtng;
    ReActivate_Timer = 0;
    pushes = clears = deselects = failures = 0;
    accept_message = 1;
}
static void check_external_state(void)
{
    assert((uint8_t)Gen_Flags == (Gen_Flags_Extern_Menu | Gen_Flags_SCI1Xmtng));
    assert(ReActivate_Timer == (int16_t)ReActivate_Time);
    assert(pushes == 1 && clears == 1 && deselects == 0);
}
int main(void)
{
    uint16_t i;
    reset();
    MenuStackc[2].CursorPos = 4;
    MenuStackc[2].FirstLine = 5; /* Device entry index 2. */
    Device_Type_Response_ID[2] = 0x5E1;
    assert(ExternalMenu() == 0);
    assert(sent.ID == 0x261 && sent.LEN == 1 && sent.BUF[0] == 1);
    check_external_state();
    for (i = 0; i < sizeof gProcImg; ++i) assert(gProcImg[i] == 0xA5);

    reset();
    MenuStackc[2].CursorPos = 2;
    MenuStackc[2].FirstLine = 3; /* Auxiliary entry index 4. */
    sub_Device_Type_Response_ID[4] = 0x5EE;
    assert(ExternalSubMenu() == 0);
    assert(sent.ID == 0x26E && sent.LEN == 1 && sent.BUF[0] == 1);
    check_external_state();

    reset();
    MenuStackc[2].CursorPos = 3;
    MenuStackc[2].FirstLine = 4; /* Camera index 5. */
    cam_list[5] = 0xFEDC;
    assert(CameraMenu() == 0);
    assert(cam_select == 5);
    assert(gProcImg[IN_digi_3] == 0xDC && gProcImg[IN_digi_3+1] == 0xFE);
    assert(sent.ID == 0x521 && sent.LEN == 3);
    assert(sent.BUF[0] == 8 && sent.BUF[1] == 0xDC && sent.BUF[2] == 0xFE);
    for (i = 0; i < sizeof gProcImg; ++i)
        if (i != IN_digi_3 && i != IN_digi_3+1) assert(gProcImg[i] == 0xA5);
    check_external_state();

    reset();
    MenuStackc[2].CursorPos = 3;
    MenuStackc[2].FirstLine = 5; /* Device entry index 1. */
    Device_Type_Response_ID[1] = 0x5E5;
    assert(Trigger_Device() == 0);
    assert(sent.ID == 0x265 && sent.LEN == 1 && sent.BUF[0] == 2);
    assert(pushes == 1 && deselects == 1 && clears == 0);
    assert((uint8_t)Gen_Flags == (Gen_Flags_Menu_Active | Gen_Flags_SCI1Xmtng));
    assert(ReActivate_Timer == 0);

    reset();
    assert(Trigger_Device() == 0); /* Existing zero-position guard. */
    assert(pushes == 0 && deselects == 0);

    /* Preserve each command's existing transmit-failure notification. */
    for (i = 0; i < 4; ++i) {
        reset();
        accept_message = 0;
        MenuStackc[2].CursorPos = (i == 1) ? 1 : (i == 2) ? 2 : 7;
        Device_Type_Response_ID[0] = sub_Device_Type_Response_ID[0] = 0x5E1;
        cam_list[0] = 0x1234;
        if (i == 0) ExternalMenu();
        else if (i == 1) ExternalSubMenu();
        else if (i == 2) CameraMenu();
        else Trigger_Device();
        assert(pushes == 1 && failures == 1);
    }
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
                                str(ROOT/'Embedded/Src/DeviceMenus.c'), '-o', str(binary)]
                               + sanitizer, check=True)
                subprocess.run([str(binary)], check=True)


if __name__ == '__main__':
    unittest.main()
