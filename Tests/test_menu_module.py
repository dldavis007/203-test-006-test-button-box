"""Exercise the real internal menu module with display/persistence boundaries stubbed."""
from pathlib import Path
import os
import subprocess
import tempfile
import unittest
from test_legacy_controls import extract

ROOT = Path(__file__).resolve().parents[1]


class MenuModuleTests(unittest.TestCase):
    def test_navigation_editing_callbacks_and_save_boundary(self):
        source = (ROOT/'Embedded/Src/Subroutines.c').read_text()
        start = source.index('char FirstVar[]')
        end = source.index(';', source.index('char Rev[]', start)) + 1
        # Reuse actual default variable and menu definitions, not copied fixtures.
        variables = source[start:end]
        program = r'''
#include <assert.h>
#include <string.h>
#include "Subroutines.h"
#include "Interrupts.h"

volatile char Gen_Flags;
char DispBuf[DispBufLen];
uint8_t StackPointer;
char Menu[12][21];
uint8_t VariableFlag;
double increment = 1.0, uLimit = 25.0, lLimit = 0.0;
int16_t nDigits = 2;
static int16_t rendered, cleared, saved, serial_saved, normalized, resets, watchdog;
volatile uint8_t AlternateFunctionMask;
volatile uint16_t ButtonBoxData;
uint16_t TC0_RCVD_Data;
void ApplyFunctionSettings(void);
void showMenu(void) { ++rendered; }
void ClearTitler(void) { ++cleared; }
void COP_Trig(void) { ++watchdog; }
void Save_Variables(void) { ++saved; }
void Save_Serial_Num(void) { ++serial_saved; }
void NormalizeFunctionSettings(void) { ++normalized; ApplyFunctionSettings(); }
int16_t ResetProc(void) { ++resets; return 0; }
int16_t findCameras(void) { return 0; }
''' + variables + '\n' + extract((ROOT/'Embedded/Src/Settings.c').read_text(), 'NormalizeFunctionSettings').replace('NormalizeFunctionSettings', 'ApplyFunctionSettings', 1) + r'''
#include "MenuTables.inc"

int main(void)
{
    int16_t count;
    setDefaultMenu();
    assert(StackPointer == 0 && VariableFlag == 0);
    assert(MenuStackc[0].CursorPos == 1 && MenuStackc[0].FirstLine == 0);
    LoadMenu(MenuStackc[0].Index);
    InsertCursor();
    btnDisplayTitler();
    assert(Menu[1][0] == '>' && rendered == 1 && watchdog == 36);
    assert(!strncmp(&DispBuf[6], Menu[0], 20));

    /* Root SETTINGS entry opens the actual settings menu. */
    Select();
    assert(StackPointer == 1 && MenuStackc[1].Index[3] == 1);
    assert(MenuStackc[1].CursorPos == 1 && saved == 0);
    assert(!strncmp(Menu[0], "    SETTINGS MENU", 17));

    /* Enter/leave numeric editing; preserve current Up/Down direction. */
    Select();
    assert(VariableFlag == 1 && nDigits == 30 && lLimit == 1 && uLimit == 13);
    CursorUp(); assert(!strcmp(MessageRow, "  2"));
    CursorDown(); assert(!strcmp(MessageRow, "  1"));
    CursorDown(); assert(!strcmp(MessageRow, " 13"));
    Select(); assert(VariableFlag == 0 && saved == 0);

    /* F1/F2 use the same Select/edit/Select interaction as numeric rows. */
    ButtonBoxData = TC0_RCVD_Data = TeleData_F1 | TeleData_F2;
    NormalizeFunctionSettings();
    for (count = 3; count <= 4; ++count) {
        char *setting = count == 3 ? F1Enable : F2Enable;
        char *other = count == 3 ? F2Enable : F1Enable;
        uint8_t bit = count == 3 ? TeleData_F1 : TeleData_F2;
        int16_t changes = normalized;
        MenuStackc[1].CursorPos = count;
        MenuStackc[1].FirstLine = 0;
        LoadMenu(MenuStackc[1].Index); InsertCursor();
        assert(Menu[count][0] == '>' && Menu[count][14] == ' ');
        assert(!strncmp(&Menu[count][1], count == 3 ? "F1 CAM MODE" : "F2 CTRL MODE", count == 3 ? 11 : 12));
        assert(!strncmp(&Menu[count][15], " ON", 3));
        Select();
        assert(VariableFlag == 1 && nDigits == OFF_ON);
        assert(!strcmp(setting, " ON") && normalized == changes && saved == 0);
        assert(Menu[count][0] == ' ' && Menu[count][14] == '>');
        /* Up changes the value, without moving to a different entry. */
        CursorUp();
        assert(!strcmp(setting, "OFF") && !strcmp(other, " ON"));
        assert(MenuStackc[1].CursorPos == count && VariableFlag == 1);
        assert(!strncmp(&Menu[count][15], "OFF", 3) && Menu[count][14] == '>');
        assert(!(AlternateFunctionMask & bit) && normalized == changes+1);
        CursorDown();
        assert(!strcmp(setting, " ON") && (AlternateFunctionMask & bit));
        CursorDown();
        assert(!strcmp(setting, "OFF") && !(AlternateFunctionMask & bit));
        Select();
        assert(VariableFlag == 0 && !strcmp(setting, "OFF") && saved == 0);
        assert(Menu[count][0] == '>' && Menu[count][14] == ' ');
        assert(normalized == changes+3);
        /* Select alone never changes a value, including OFF on re-entry. */
        Select(); assert(VariableFlag == 1 && !strcmp(setting, "OFF"));
        Select(); assert(VariableFlag == 0 && !strcmp(setting, "OFF"));
        /* Restore with the opposite direction to exercise both transitions. */
        Select(); CursorUp(); Select();
        assert(!strcmp(setting, " ON") && !strcmp(other, " ON"));
        assert(ButtonBoxData == (TeleData_F1 | TeleData_F2));
        assert(TC0_RCVD_Data == ButtonBoxData && saved == 0);
    }

    /* Wrap over empty trailing entries. */
    MenuStackc[1].CursorPos = 1;
    CursorUp(); assert(MenuStackc[1].CursorPos == 5);
    CursorDown(); assert(MenuStackc[1].CursorPos == 1);

    /* STATUS retains revision/serial, with EXIT immediately after them. */
    MenuStackc[1].Index[3] = 4;
    MenuStackc[1].CursorPos = 1;
    MenuStackc[1].FirstLine = 0;
    LoadMenu(MenuStackc[1].Index);
    assert(!strncmp(Menu[1], " SOFTWARE REV", 13));
    assert(!strncmp(Menu[2], " SERIAL NUM", 11));
    assert(!strncmp(Menu[3], " EXIT", 5));
    assert(Menuc[4].VarPntr[0] == Rev && Menuc[4].VarPntr[1] == SerialNum);
    assert(Menuc[4].FunctPtr[2] == ExitMenu);
    CursorUp(); assert(MenuStackc[1].CursorPos == 3);
    CursorDown(); assert(MenuStackc[1].CursorPos == 1);

    /* Leaving serial entry after digit five requests its separate save. */
    MenuStackc[1].CursorPos = 2;
    MenuStackc[1].FirstLine = 0;
    strcpy(SerialNum, "12345");
    VariableFlag = 5;
    setSerial();
    assert(serial_saved == 1 && VariableFlag == 0);

    /* Returning to root does not save; fully exiting does, exactly once. */
    DeSelect();
    assert(StackPointer == 0 && saved == 0 && cleared == 0);
    Gen_Flags |= Gen_Flags_Menu_Active;
    VariableFlag = 1;
    DeSelect();
    assert(saved == 1 && cleared == 1 && VariableFlag == 0);
    assert(!(Gen_Flags & Gen_Flags_Menu_Active));

    count = normalized;
    RestoreDefaults();
    assert(!strcmp(F1Enable, " ON") && !strcmp(F2Enable, " ON"));
    assert((uint8_t)FirstVar[0] == 0xff && normalized == count+1);
    assert(saved == 2 && resets == 1);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            folder = Path(directory)
            test = folder/'menu_test.c'
            binary = folder/('test.exe' if os.name == 'nt' else 'test')
            test.write_text(program)
            sanitizer = [] if os.name == 'nt' else ['-fsanitize=undefined', '-fno-sanitize-recover=all']
            for mode in ['-fsigned-char', '-funsigned-char']:
                subprocess.run([os.environ.get('CC', 'gcc'), '-std=gnu89', '-Wall', '-Wextra',
                                '-Wno-parentheses', '-Wno-missing-braces',
                                '-DBUTTONBOX_GCC', '-DBUTTONBOX_HOST_SYNTAX', mode,
                                '-I', str(ROOT/'Embedded/Include'), '-I', str(ROOT/'Embedded/Src'),
                                str(test), str(ROOT/'Embedded/Src/Menu.c'),
                                str(ROOT/'Embedded/Src/compact_io.c'), '-o', str(binary)] + sanitizer,
                               check=True)
                subprocess.run([str(binary)], check=True)


if __name__ == '__main__':
    unittest.main()
