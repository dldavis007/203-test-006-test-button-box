"""Exercise internal-menu entry state and display ordering."""
from pathlib import Path
import unittest
from test_legacy_controls import extract, run_c

ROOT = Path(__file__).resolve().parents[1]


class MenuEntryTests(unittest.TestCase):
    def test_entry_initialization_and_display_boundary(self):
        source = (ROOT/'Embedded/Src/Subroutines.c').read_text()
        helper = extract(source, 'EnterInternalMenu')
        events = extract(source, 'doevents')
        # The original controller/function-mode/request guard remains at the call site.
        self.assertIn('if ( controller_mode && !(EnabledFunctionData(TC0_RCVD_Data) & (TeleData_F1 | TeleData_F2)) )', events)
        self.assertIn('if (internal_menu)\n\t\t\t\t{\n\t\t\t\t\tEnterInternalMenu();\n\t\t\t\t}\n\t\t\t\telse\n\t\t\t\t{\n\t\t\t\t\tServiceCameraShortcuts();', events)
        self.assertEqual(events.count('EnterInternalMenu();'), 1)
        program = r'''
#include <assert.h>
#include <string.h>
#include "Subroutines.h"
#include "Controller.h"
uint16_t TC0_RCVD_Data;
volatile char Gen_Flags;
int8_t CursorDownFlag, CursorUpFlag, SelectFlag;
int8_t Trigger_DeviceFlag;
uint8_t internal_menu;
struct MenuStack MenuStackc[MenuStackSize];
uint8_t StackPointer;
static uint16_t saved_input;
static unsigned calls;
static void check_display_boundary(void)
{
    unsigned i;
    assert(StackPointer == 0);
    for (i = 0; i < 4; ++i) assert(MenuStackc[0].Index[i] == 0);
    assert(MenuStackc[0].CursorPos == 1 && MenuStackc[0].FirstLine == 0);
    assert((uint8_t)Gen_Flags == (uint8_t)(0x01 | Gen_Flags_Menu_Active));
    /* Input clearing and flag resets occur AFTER rendering, as before. */
    assert(TC0_RCVD_Data == saved_input && internal_menu == 1);
    assert(CursorDownFlag == -1 && CursorUpFlag == 1 && SelectFlag == -1);
}
void LoadMenu(uint8_t *index)
{
    check_display_boundary();
    assert(index == MenuStackc[0].Index && calls++ == 0);
}
void InsertCursor(void) { check_display_boundary(); assert(calls++ == 1); }
void btnDisplayTitler(void) { check_display_boundary(); assert(calls++ == 2); }
''' + helper + r'''
int main(void)
{
    uint32_t input;
    for (input = 0; input <= 0xFFFF; ++input) {
        struct MenuStack untouched;
        memset(MenuStackc, 0x55, sizeof(MenuStackc));
        untouched = MenuStackc[1];
        StackPointer = MenuStackSize - 1;
        TC0_RCVD_Data = saved_input = input;
        Gen_Flags = 0x01;
        CursorDownFlag = SelectFlag = -1; CursorUpFlag = 1;
        Trigger_DeviceFlag = 1; internal_menu = 1; calls = 0;
        EnterInternalMenu();
        assert(calls == 3 && internal_menu == 0);
        assert(TC0_RCVD_Data == (input & ~0x07U));
        assert(CursorDownFlag == 0 && CursorUpFlag == 0 && SelectFlag == 0);
        assert(Trigger_DeviceFlag == 1);
        assert(!memcmp(&untouched, &MenuStackc[1], sizeof(untouched)));
    }
    return 0;
}
'''
        program = program.replace('"Subroutines.h"', '"'+str(ROOT/'Embedded/Include/Subroutines.h')+'"')
        program = program.replace('"Controller.h"', '"'+str(ROOT/'Embedded/Include/Controller.h')+'"')
        run_c('#define BUTTONBOX_GCC\n#define BUTTONBOX_HOST_SYNTAX\n' + program)


if __name__ == '__main__':
    unittest.main()
