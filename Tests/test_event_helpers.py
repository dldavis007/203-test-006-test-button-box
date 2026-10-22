"""Check camera shortcuts and reset ordering using the real event helpers."""
from pathlib import Path
import unittest
from test_legacy_controls import extract, run_c

ROOT = Path(__file__).resolve().parents[1]


def run_helper(program):
    for name in ['Subroutines.h', 'Interrupts.h', 'LegacyCanInterface.h', 'mcohw.h']:
        program = program.replace('"'+name+'"', '"'+str(ROOT/'Embedded/Include'/name)+'"')
    run_c('#define BUTTONBOX_GCC\n#define BUTTONBOX_HOST_SYNTAX\n' + program)


class EventHelperTests(unittest.TestCase):
    def test_camera_shortcuts(self):
        source = (ROOT/'Embedded/Src/Subroutines.c').read_text()
        helper = extract(source, 'ServiceCameraShortcuts')
        program = r'''
#include <assert.h>
#include <string.h>
#include "Subroutines.h"
#include "Interrupts.h"
#include "LegacyCanInterface.h"
uint16_t TC0_RCVD_Data;
volatile uint16_t ToggleTimer;
int16_t cam_list[10], cam_select;
UNSIGNED8 gProcImg[PROCIMG_SIZE];
static unsigned sends, expected;
void MCO_TransmitPDO(UNSIGNED8 number)
{
    assert(number == 0 && cam_select == (int16_t)expected);
    assert(gProcImg[IN_digi_3] == (cam_list[expected] & 0xFF));
    assert(gProcImg[IN_digi_3 + 1] == ((cam_list[expected] >> 8) & 0xFF));
    assert(ToggleTimer == (uint16_t)(RTI_One_Sec * 0.75));
    ++sends;
}
''' + helper + r'''
int main(void)
{
    unsigned length, selected, keys, timer, i;
    for (length = 1; length <= 10; ++length) {
        memset(cam_list, 0, sizeof(cam_list));
        for (i = 0; i < length; ++i) cam_list[i] = 0x1200 + i;
        for (selected = 0; selected < length; ++selected)
        for (keys = 0; keys < 8; ++keys)
        for (timer = 0; timer < 2; ++timer) {
            int16_t active = !timer && !(keys & 4) && (keys & 3);
            cam_select = selected; sends = 0; ToggleTimer = timer;
            TC0_RCVD_Data = ((keys & 1) ? TeleData_Down : 0)
                           | ((keys & 2) ? TeleData_Up : 0)
                           | ((keys & 4) ? TeleData_Select : 0);
            memset(gProcImg, 0xAA, sizeof(gProcImg));
            expected = selected;
            if (active) {
                /* Down takes precedence when both direction buttons are set. */
                if (keys & 1) expected = selected ? selected - 1 : length - 1;
                else expected = (selected + 1 == length) ? 0 : selected + 1;
            }
            ServiceCameraShortcuts();
            assert(cam_select == (int16_t)expected && sends == (unsigned)!!active);
            for (i = 0; i < sizeof(gProcImg); ++i)
                if (!active || (i != IN_digi_3 && i != IN_digi_3 + 1))
                    assert(gProcImg[i] == 0xAA);
            if (!active) assert(ToggleTimer == timer);
        }
    }
    return 0;
}
'''
        run_helper(program)

    def test_reset_message_and_callback_order(self):
        source = (ROOT/'Embedded/Src/Subroutines.c').read_text()
        events = extract(source, 'doevents')
        self.assertLess(events.index('EncodeButtonCommands();'), events.index('HandleResetButton();'))
        self.assertIn('if (ButtonBoxData & TeleData_Reset) //Reset\n\t{\n\t\tHandleResetButton();', events)
        program = r'''
#include <assert.h>
#include <string.h>
#include "Subroutines.h"
#include "LegacyCanInterface.h"
#include "mcohw.h"
#include "Interrupts.h"
volatile uint16_t ButtonBoxData;
bool controller_mode;
static CAN_MSG gTxMsg;
static unsigned step, failures;
static int16_t fail_send;
void resetTitler(void)
{
    assert(step++ == 0 && ButtonBoxData == (0x400 | TeleData_Reset));
}
UNSIGNED8 MCOHW_PushMessage(CAN_MSG *message)
{
    assert(controller_mode && step++ == 1 && message == &gTxMsg);
    assert(message->ID == 0 && message->LEN == 2);
    assert(message->BUF[0] == 0x81 && message->BUF[1] == 0);
    return !fail_send;
}
void MCOUSER_FatalError(UNSIGNED16 code)
{
    assert(fail_send && step++ == 2 && code == 0x8801);
    ++failures; /* Allow return only to check the existing continuation. */
}
int16_t ResetProc(void)
{
    assert(controller_mode && step++ == (fail_send ? 3U : 2U));
    return 0;
}
''' + extract(source, 'HandleResetButton') + r'''
int main(void)
{
    unsigned mode, failed;
    for (mode = 0; mode < 2; ++mode)
    for (failed = 0; failed < 2; ++failed) {
        controller_mode = mode ? true : false;
        fail_send = failed; step = failures = 0;
        memset(&gTxMsg, 0x55, sizeof(gTxMsg));
        ButtonBoxData = 0xFFFF;
        HandleResetButton();
        assert(step == (mode ? (failed ? 4U : 3U) : 1U));
        assert(failures == (mode && failed));
        assert(ButtonBoxData == (0x400 | TeleData_Reset));
        if (!mode) assert(gTxMsg.BUF[0] == 0x55);
    }
    return 0;
}
'''
        run_helper(program)


if __name__ == '__main__':
    unittest.main()
