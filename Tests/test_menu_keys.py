"""Check active-menu key flags, repeat timing and callback order."""
from pathlib import Path
import unittest
from test_legacy_controls import extract, run_c

ROOT = Path(__file__).resolve().parents[1]


class MenuKeyTests(unittest.TestCase):
    def test_key_events_and_render_order(self):
        source = (ROOT/'Embedded/Src/Subroutines.c').read_text()
        helper = extract(source, 'ServiceInternalMenuKeys')
        events = extract(source, 'doevents')
        self.assertEqual(events.count('ServiceInternalMenuKeys();'), 1)
        self.assertLess(events.index('ServiceInternalMenuKeys();'), events.index('ScanButtons('))
        # Include the real key masks and timer constants, with target operations unused.
        program = r'''
#include <assert.h>
#include "Subroutines.h"
#include "Interrupts.h"
unsigned int TC0_RCVD_Data;
signed char CursorDownFlag, CursorUpFlag, SelectFlag;
char Trigger_DeviceFlag, VariableFlag;
volatile char UpdateMenu;
volatile unsigned int MenuTimer;
struct MenuStack MenuStackc[MenuStackSize];
unsigned char StackPointer;
static char calls[16];
static unsigned count;
static void record(char value) { assert(count < sizeof(calls)); calls[count++] = value; }
void CursorDown(void) { record('D'); }
void CursorUp(void) { record('U'); }
void Select(void) { record('S'); }
int Trigger_Device(void) { record('T'); return 0; }
void LoadMenu(unsigned char *index) { assert(index == MenuStackc[StackPointer].Index); record('L'); }
void InsertCursor(void) { record('I'); }
void btnDisplayTitler(void) { record('B'); }
''' + helper + r'''
int main(void)
{
    unsigned mask;
    for (mask = 0; mask < 16; ++mask) {
        TC0_RCVD_Data = ((mask & 1) ? TeleData_Down : 0)
                       | ((mask & 2) ? TeleData_Up : 0)
                       | ((mask & 4) ? TeleData_Select : 0)
                       | ((mask & 8) ? TeleData_Trig : 0);
        CursorDownFlag = CursorUpFlag = SelectFlag = Trigger_DeviceFlag = 0;
        MenuTimer = 1; UpdateMenu = 1; VariableFlag = 0; count = 0;
        ServiceInternalMenuKeys();
        assert(count == 0 && UpdateMenu == 1 && MenuTimer == 1);
        assert(CursorDownFlag == !!(mask & 1));
        assert(CursorUpFlag == !!(mask & 2));
        assert(SelectFlag == !!(mask & 4));
        assert(Trigger_DeviceFlag == !!(mask & 8));
        MenuTimer = 0;
        ServiceInternalMenuKeys();
        assert(count == 3U + (unsigned int)(!!(mask & 1) + !!(mask & 2) + !!(mask & 4) + !!(mask & 8)));
        count = 0;
        if (mask & 1) assert(calls[count++] == 'D');
        if (mask & 2) assert(calls[count++] == 'U');
        if (mask & 4) assert(calls[count++] == 'S');
        if (mask & 8) assert(calls[count++] == 'T');
        assert(calls[count++] == 'L' && calls[count++] == 'I' && calls[count++] == 'B');
        assert(UpdateMenu == 0);
        assert(MenuTimer == (unsigned int)((mask & 4) ? MenuTime * 2 : MenuTime));
        /* Existing nonzero flags repeat when the menu timer expires. */
        count = 0; MenuTimer = 0;
        ServiceInternalMenuKeys();
        assert(count == (unsigned int)(!!(mask & 1) + !!(mask & 2) + !!(mask & 4) + !!(mask & 8)));
        /* Releasing signed navigation flags rearms them before the next tick. */
        TC0_RCVD_Data = 0; MenuTimer = 1;
        ServiceInternalMenuKeys();
        assert(CursorDownFlag == 0 && CursorUpFlag == 0 && SelectFlag == 0);
    }
    CursorDownFlag = CursorUpFlag = SelectFlag = Trigger_DeviceFlag = 0;
    MenuTimer = 0; UpdateMenu = 1; VariableFlag = 1; count = 0;
    ServiceInternalMenuKeys();
    assert(count == 2 && calls[0] == 'L' && calls[1] == 'B');
    return 0;
}
'''
        # run_c intentionally accepts only standalone sources; use absolute includes.
        for header in ['Subroutines.h', 'Interrupts.h']:
            program = program.replace('"'+header+'"', '"'+str(ROOT/'Embedded/Include'/header)+'"')
        program = '#define BUTTONBOX_GCC\n#define BUTTONBOX_HOST_SYNTAX\n' + program
        run_c(program)


if __name__ == '__main__':
    unittest.main()
