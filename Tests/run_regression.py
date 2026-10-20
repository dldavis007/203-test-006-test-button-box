"""Compile actual firmware scan/encoding code with injected keypad samples.
No PC_SIDE firmware, CAN simulator, or interactive bench is built.
"""
from pathlib import Path
import os
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / 'Embedded/Src'
INC = ROOT / 'Embedded/Include'
s = (SRC / 'Subroutines.c').read_text()

def function(name, source=None):
    s = source if source is not None else (SRC / "Subroutines.c").read_text()
    match = re.search(r'^(?:unsigned int|void|int) ' + name + r'\([^;]*?\)\s*\{', s, re.M)
    assert match, name
    start = match.start()
    pos = match.end()
    depth = 1
    while depth:
        depth += (s[pos] == '{') - (s[pos] == '}')
        pos += 1
    return s[start:pos]

begin = s.index('\tif ( !DeBounceTimer )', s.index('void doevents'))
end = s.index('\n\tif (ButtonBoxData & TeleData_Reset)', begin)
scan = s[begin:end]
# Replace only electrical input and asynchronous delay. Keep actual scan order,
# latching, settings gate, interlocks and all three output assignments.
condition = 'if ( (~PTT & Keypad_Matrix[Sw_Index].SW_Port_Data[0]) || (~PORTB & (Keypad_Matrix[Sw_Index].SW_Port_Data[1] & 0x07)) )'
assert scan.count(condition) == 1
scan = scan.replace(condition, 'if (pressed & (1UL << Sw_Index))')
scan = scan.replace('while (Timer1) { /* wait for interrupt */ }', '/* test clock */')
scan = scan.replace('unsigned int MaskBit = 0x8000;', '')
matrix = s[s.index('struct MatrixTbl Keypad_Matrix[18]'):s.index('void setDefaultMenu')]
macros = '\n'.join(line for line in (INC/'Interrupts.h').read_text().splitlines()
                   if line.startswith('#define TeleData_') or line.startswith('#define Gen_Flags_'))
menu = s[s.index('char FirstVar[]'):s.index('struct MenuStruct Menuc[MenuSize];')]
callbacks = sorted(set(re.findall(r'&(\w+)', menu)) - {'ToggleF1Enable', 'ToggleF2Enable'})
menu_stubs = '\n'.join('int ' + name + '(void) { return 0; }' for name in callbacks)
helpers = '\n'.join(function(n) for n in ['EnabledFunctionData', 'NormalizeFunctionSettings', 'ToggleF1Enable', 'ToggleF2Enable', 'EncodeLegacySettings', 'DecodeLegacySettings'])
helpers += '\n' + function('EEPROMWord', (SRC/'EEProm.c').read_text())
led_condition = re.search(r'if \((\(ButtonBoxData & AlternateFunctionMask\).*?)\)\n', (SRC/'Interrupts.c').read_text()).group(1)
program = r'''
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "Subroutines.h"
#include "procimg.h"
#define RTI_One_Sec 1000
#define DeBounceTime 10
''' + macros + r'''
volatile unsigned int ButtonBoxData;
unsigned int TC0_RCVD_Data, tempData, Third_byte;
volatile unsigned char AlternateFunctionMask = TeleData_F1 | TeleData_F2;
unsigned int DeBounceTimer, Timer1;
int CntrSwTimeOut = -1;
unsigned char PORTB, PORTA;
char Cntrl_Slow_Flag, MenuCheck, Gen_Flags, internal_menu;
unsigned char StackPointer;
volatile bool controller_mode;
unsigned char gProcImg[PROCIMG_SIZE];
''' + menu + menu_stubs + r'''

struct MenuStack MenuStackc[MenuStackSize];
void LoadMenu(unsigned char index[]) { (void)index; }
void InsertCursor(void) {}
void btnDisplayTitler(void) {}
void DeSelect(void) { Gen_Flags &= ~Gen_Flags_Menu_Active; }
''' + helpers + '\nint navigation_led(int Sw_Index) { return ' + led_condition + '; }\n' + matrix + '\nvoid scan(unsigned long pressed) {\n' + scan + r'''
}
void tick(unsigned long pressed)
{
    TC0_RCVD_Data = ButtonBoxData;
    DeBounceTimer = 0;
    scan(pressed);
    assert(gProcImg[IN_digi_0] == (tempData & 255));
    assert(gProcImg[IN_digi_1] == ((tempData >> 8) & 255));
    assert(gProcImg[IN_digi_2] == Third_byte);
}
void clear(void)
{
    ButtonBoxData = TC0_RCVD_Data = 0;
    MenuCheck = Gen_Flags = internal_menu = 0;
    Cntrl_Slow_Flag = controller_mode = 0;
    CntrSwTimeOut = -1;
    memset(gProcImg, 0, sizeof(gProcImg));
    strcpy(F1Enable, " ON"); strcpy(F2Enable, " ON");
    tick(0); /* release every key, including after a previous scenario */
}
#define KEY(i) (1UL << (i))
int main(void)
{
    unsigned int mode;
    unsigned char legacy[28], flags;
    int f1, f2, key;
    const int nav_keys[5] = {4, 16, 9, 11, 10};
    const unsigned int nav_bits[5] = {4, 8, 1, 2, 16};
    assert(!strcmp(defaultMenuc[1].Entry[3], " F1                 "));
    assert(!strcmp(defaultMenuc[1].Entry[4], " F2                 "));
    assert(defaultMenuc[1].VarPntr[2] == F1Enable);
    assert(defaultMenuc[1].VarPntr[3] == F2Enable);
    assert(defaultMenuc[1].FunctPtr[2] == ToggleF1Enable);
    assert(defaultMenuc[1].FunctPtr[3] == ToggleF2Enable);
    assert(defaultMenuc[1].FunctPtr[4] == ExitMenu);
    strcpy(MessageRow, " 12"); strcpy(MessageCol, " 23"); strcpy(SerialNum, "12345");
    EncodeLegacySettings(legacy);
    assert(!memcmp(legacy, "xxx", 4));
    assert(!memcmp(legacy + 4, " 12", 4));
    assert(!memcmp(legacy + 8, " 23", 4));
    assert(!memcmp(legacy + 12, "12345", 6));
    strcpy(MessageRow, "  1"); strcpy(MessageCol, "  1"); strcpy(SerialNum, "-----");
    DecodeLegacySettings(legacy);
    assert(!strcmp(MessageRow, " 12") && !strcmp(MessageCol, " 23"));
    assert(!strcmp(SerialNum, "12345"));
    clear(); ButtonBoxData = TC0_RCVD_Data = TeleData_F1 | TeleData_F2;
    ToggleF1Enable();
    assert(!strcmp(F1Enable, "OFF") && !strcmp(F2Enable, " ON"));
    assert(ButtonBoxData == (TeleData_F1 | TeleData_F2) && TC0_RCVD_Data == ButtonBoxData);
    ToggleF2Enable(); assert(ButtonBoxData == (TeleData_F1 | TeleData_F2) && TC0_RCVD_Data == ButtonBoxData);
    ToggleF1Enable(); ToggleF2Enable();
    assert(!strcmp(F1Enable, " ON") && !strcmp(F2Enable, " ON"));
    for (f1 = 0; f1 <= 1; ++f1) for (f2 = 0; f2 <= 1; ++f2) {
        clear();
        strcpy(F1Enable, f1 ? " ON" : "OFF");
        strcpy(F2Enable, f2 ? " ON" : "OFF");
        tick(KEY(1)); assert(ButtonBoxData & TeleData_F1);
        tick(0); tick(KEY(7));
        assert(ButtonBoxData & TeleData_F2);
        assert(!(ButtonBoxData & TeleData_F1));
        tick(0); tick(KEY(4));
        assert(Third_byte == (f2 ? 4 : 0));
        assert(tempData & TeleData_F2);
        if (!f2) assert(tempData & TeleData_Down);
    }
    for (mode = TeleData_F2; mode <= TeleData_F1; mode *= 2) {
        clear();
        strcpy(F1Enable, mode == TeleData_F1 ? "OFF" : " ON");
        strcpy(F2Enable, mode == TeleData_F2 ? "OFF" : " ON");
        ButtonBoxData = mode;
        tick(KEY(4)); assert(Third_byte == 0);
        assert((tempData & (mode | TeleData_Down)) == (mode | TeleData_Down));
        tick(0); tick(KEY(11)); assert(Third_byte == 0);
        assert(tempData & TeleData_RotCW);
        clear(); strcpy(F1Enable, "OFF"); strcpy(F2Enable, "OFF");
        MenuCheck = 1; tick(mode == TeleData_F1 ? KEY(1) : KEY(7));
        assert(ButtonBoxData & mode); /* OFF still latches with menu present. */
    }
    for (mode = TeleData_F2; mode <= TeleData_F1; mode *= 2) {
        for (key = 0; key < 5; ++key) {
            clear(); ButtonBoxData = mode;
            tick(KEY(nav_keys[key]));
            assert(Third_byte == nav_bits[key]);
            assert(tempData & mode);
            assert(!(tempData & (TeleData_Down | TeleData_Up | TeleData_Slow | TeleData_RotCW)));
            tick(0); assert(Third_byte == 0);
        }
        clear(); ButtonBoxData = mode; controller_mode = 1;
        tick(KEY(10)); assert(Third_byte == 16 && !internal_menu);
        clear(); ButtonBoxData = mode;
        gProcImg[OUT_digi_12] = 0x10;
        tick(0); assert(tempData & TeleData_Fwd);
        assert(tempData & TeleData_Slow); /* controller exception */
    }
    clear(); controller_mode = 1;
    tick(KEY(10)); assert(internal_menu && Third_byte == 0);
    clear(); tick(KEY(14)); tick(0); tick(KEY(9)); tick(0);
    assert(ButtonBoxData & TeleData_Fwd);
    assert(ButtonBoxData & TeleData_Slow); /* normal speed stays latched */
    tick(KEY(8)); assert(!(ButtonBoxData & (TeleData_Fwd | TeleData_Rev | TeleData_Slow)));
    clear(); tick(KEY(1)); tick(0); tick(KEY(1));
    assert(!(ButtonBoxData & TeleData_F1)); /* second press toggles off */
    clear(); tick(KEY(1)); tick(0); tick(KEY(7));
    assert((ButtonBoxData & (TeleData_F1 | TeleData_F2)) == TeleData_F2);
    tick(0); tick(KEY(13)); assert(ButtonBoxData & TeleData_F3);
    assert(!(ButtonBoxData & (TeleData_F1 | TeleData_F2)));
    clear(); MenuCheck = 1; tick(KEY(7)); assert(!(ButtonBoxData & TeleData_F2));
    clear(); strcpy(F1Enable, "OFF"); strcpy(F2Enable, "OFF");
    tick(KEY(1)); assert(ButtonBoxData & TeleData_F1);
    tick(0); tick(KEY(1)); assert(!(ButtonBoxData & TeleData_F1));
    tick(0); tick(KEY(7)); assert(ButtonBoxData & TeleData_F2);
    tick(0); tick(KEY(7)); assert(!(ButtonBoxData & TeleData_F2));
    for (flags = 0; flags < 4; ++flags) {
        strcpy(F1Enable, (flags & 1) ? " ON" : "OFF");
        strcpy(F2Enable, (flags & 2) ? " ON" : "OFF");
        NormalizeFunctionSettings();
        assert(AlternateFunctionMask == ((flags & 1 ? TeleData_F1 : 0) | (flags & 2 ? TeleData_F2 : 0)));
        ButtonBoxData = TeleData_F1;
        assert(!!navigation_led(4) == !!(flags & 1));
        ButtonBoxData = TeleData_F2;
        assert(!!navigation_led(16) == !!(flags & 2));
        ButtonBoxData = TeleData_F1 | TeleData_F2;
        assert(!navigation_led(1)); /* F1 LED remains in the ordinary latch path. */
        EncodeLegacySettings(legacy);
        { /* Round-trip through the actual EEPROM word packing. */
            int n;
            for (n = 0; n < 28; n += 2) {
                unsigned int word = EEPROMWord(legacy[n], legacy[n + 1]);
                legacy[n] = word >> 8; legacy[n + 1] = word & 255;
            }
        }
        strcpy(F1Enable, " ON"); strcpy(F2Enable, " ON");
        DecodeLegacySettings(legacy);
        assert(!strcmp(F1Enable, (flags & 1) ? " ON" : "OFF"));
        assert(!strcmp(F2Enable, (flags & 2) ? " ON" : "OFF"));
    }
    memset(legacy + 18, 255, 8); DecodeLegacySettings(legacy);
    assert(!strcmp(F1Enable, " ON") && !strcmp(F2Enable, " ON"));
    memcpy(legacy + 18, "OFF", 4); memcpy(legacy + 22, "BAD", 4);
    DecodeLegacySettings(legacy);
    assert(!strcmp(F1Enable, "OFF") && !strcmp(F2Enable, " ON"));
    puts("PASS: settings combinations, F1/F2 commands, release/latching, travel/stop, menu routing, EEPROM codec");
    return 0;
}
'''
with tempfile.TemporaryDirectory() as d:
    source = Path(d)/'regression.c'
    binary = Path(d)/('regression.exe' if os.name == 'nt' else 'regression')
    source.write_text(program)
    for char_mode in ['-fsigned-char', '-funsigned-char']:
        subprocess.run([os.environ.get('CC', 'gcc'), '-std=gnu89', '-Wall', '-Wextra',
                        '-Wno-parentheses', '-Wno-missing-braces', char_mode,
                        '-I', str(INC), str(source), '-o', str(binary)], check=True)
        subprocess.run([str(binary)], check=True)
