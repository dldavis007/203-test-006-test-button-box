"""Exercise real parser helpers and timer LED scanning without target hardware."""
from pathlib import Path
import os
import re
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


def extract(source, name):
    match = re.search(r'^(?:static )?(?:void|int|char|unsigned int|UNSIGNED8) ' + name + r'\s*\([^;]*?\)\s*\{', source, re.M)
    start, pos, depth = match.start(), match.end(), 1
    while depth:
        depth += (source[pos] == '{') - (source[pos] == '}')
        pos += 1
    return source[start:pos]


def run_c(program):
    with tempfile.TemporaryDirectory() as directory:
        source = Path(directory)/'test.c'
        binary = Path(directory)/('test.exe' if os.name == 'nt' else 'test')
        source.write_text(program)
        sanitizer = [] if os.name == 'nt' else ['-fsanitize=undefined', '-fno-sanitize-recover=all']
        for mode in ['-fsigned-char', '-funsigned-char']:
            subprocess.run([os.environ.get('CC', 'gcc'), '-std=gnu89', '-Wall', '-Wextra',
                            '-Wno-parentheses', mode] + sanitizer + [str(source), '-o', str(binary)], check=True)
            subprocess.run([str(binary)], check=True)


class LegacyControlsTests(unittest.TestCase):
    def test_command_token_consumption(self):
        source = (ROOT/'Embedded/Src/Command.c').read_text()
        helpers = '\n'.join(extract(source, name) for name in ['ltrim', 'parse', 'ucase', 'hextoi'])
        run_c('#include <string.h>\n#include <assert.h>\n' + helpers + r'''
int main(void) {
    char command[64] = "   revision   go 4000\r\n";
    char token[64], spaces[] = "   ", empty[] = "";
    ucase(command);
    parse(token, command); assert(!strcmp(token, "REVISION"));
    parse(token, command); assert(!strcmp(token, "GO"));
    parse(token, command); assert(!strcmp(token, "4000"));
    assert(hextoi(token) == 0x4000 && !command[0]);
    ltrim(spaces); assert(!spaces[0]);
    ltrim(empty); assert(!empty[0]);
    parse(token, command); assert(!token[0]);
    assert(hextoi("1aFZ") == 0x1af);
    return 0;
}
''')

    def test_flash_word_preserves_all_byte_pairs(self):
        source = (ROOT/'Embedded/Src/Flash.c').read_text()
        run_c('#include <assert.h>\n' + extract(source, 'FlashWord') + r'''
int main(void) {
    unsigned int high, low;
    for (high = 0; high < 256; ++high)
        for (low = 0; low < 256; ++low) {
            unsigned int word = FlashWord((char)high, (char)low);
            assert(word == high * 256 + low);
        }
    return 0;
}
''')

    def test_polled_can_receive_and_irq_fallback(self):
        driver = (ROOT/'Embedded/Src/mcohw.c').read_text()
        interrupt = (ROOT/'Embedded/Src/Interrupts.c').read_text()
        run_c(r'''
#include <assert.h>
#include <string.h>
typedef unsigned char UNSIGNED8;
typedef unsigned long UNSIGNED32;
typedef struct { UNSIGNED32 ID; UNSIGNED8 LEN; UNSIGNED8 BUF[8]; } CAN_MSG;
unsigned char CANRFLG, CANRIER, CANRXIDR0, CANRXIDR1, CANRXDLR;
unsigned char registers[8];
#define CANRXDSR0 registers[0]
''' + extract(driver, 'MCOHW_PullMessage') + extract(interrupt, 'CANRxISR') + r'''
int main(void) {
    struct { unsigned long before; CAN_MSG message; unsigned long after; } guarded;
    int length, index;
    memset(&guarded, 0, sizeof guarded);
    guarded.before = 0x12345678; guarded.after = 0x87654321;
    CANRFLG = 0;
    assert(!MCOHW_PullMessage(&guarded.message));
    CANRXIDR0 = 0x64; CANRXIDR1 = 0x20; /* Standard ID 0x321 */
    for (index = 0; index < 8; ++index) registers[index] = 0x80 + index;
    for (length = 0; length < 16; ++length) {
        CANRFLG = 1; CANRXDLR = length;
        CANRIER = 0xa5;
        CANRxISR();
        assert(CANRIER == 0xa4 && CANRFLG == 1);
        assert(MCOHW_PullMessage(&guarded.message));
        assert(guarded.message.ID == 0x321);
        assert(guarded.message.LEN == (length < 8 ? length : 8));
        for (index = 0; index < guarded.message.LEN; ++index)
            assert(guarded.message.BUF[index] == 0x80 + index);
        assert(guarded.before == 0x12345678 && guarded.after == 0x87654321);
    }
    return 0;
}
''')

    def test_scan_replies_respect_table_and_payload_bounds(self):
        source = (ROOT/'Embedded/Src/mco.c').read_text()
        run_c(r'''
#include <assert.h>
#include <string.h>
typedef unsigned long UNSIGNED32;
typedef struct { UNSIGNED32 ID; unsigned char LEN; unsigned char BUF[8]; } CAN_MSG;
#define DEVICE_SCAN_CAPACITY 20
int Device_Type_Response_ID[DEVICE_SCAN_CAPACITY];
int Device_Type_Response_ID_ptr;
char Device_Menus[DEVICE_SCAN_CAPACITY][17];
''' + extract(source, 'RecordDeviceMenuResponse') + r'''
int main(void) {
    CAN_MSG message;
    int i, part;
    memset(&message, 0, sizeof message); message.LEN = 8;
    for (i = 0; i < 25; ++i) {
        message.ID = 0x580 + i;
        RecordDeviceMenuResponse(&message);
    }
    assert(Device_Type_Response_ID_ptr == DEVICE_SCAN_CAPACITY);
    for (i = 0; i < DEVICE_SCAN_CAPACITY; ++i)
        assert(Device_Type_Response_ID[i] == 0x580 + i);
    message.ID = 0x593;
    memcpy(message.BUF + 4, "TEST", 4);
    for (part = 1; part <= 4; ++part) {
        message.BUF[3] = part;
        RecordDeviceMenuResponse(&message);
    }
    assert(!strcmp(Device_Menus[19], "TESTTESTTESTTEST"));
    message.BUF[3] = 255; RecordDeviceMenuResponse(&message);
    assert(!strcmp(Device_Menus[19], "TESTTESTTESTTEST"));
    message.BUF[3] = 1; message.ID = 0x5ff;
    RecordDeviceMenuResponse(&message); /* Unknown ID cannot overrun the table. */
    message.ID = 0x593; message.LEN = 4;
    memcpy(message.BUF + 4, "FAIL", 4);
    RecordDeviceMenuResponse(&message);
    assert(!strcmp(Device_Menus[19], "TESTTESTTESTTEST"));
    return 0;
}
''')

    def test_variable_editors_reject_invalid_menu_state(self):
        source = (ROOT/'Embedded/Src/Subroutines.c').read_text()
        header = (ROOT/'Embedded/Include/Subroutines.h').as_posix()
        helpers = '\n'.join(extract(source, name) for name in ['dispVariable', 'IncVariable', 'DecVariable'])
        run_c('#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n#include <assert.h>\n#include "' + header + '"\n' + r'''
#define SER_NUM 8
#define OFF_ON 9
struct MenuStruct Menuc[MenuSize];
struct MenuStack MenuStackc[MenuStackSize];
unsigned char StackPointer;
char VariableFlag, SerialNum[6] = "12345";
double increment = 1, lLimit = 1, uLimit = 13;
int nDigits = 30, renders;
void LoadMenu(unsigned char index[]) { (void)index; }
void btnDisplayTitler(void) { ++renders; }
''' + helpers + r'''
int main(void) {
    char value[4] = "  1";
    Menuc[0].Index[0] = 1;
    MenuStackc[0].Index[0] = 1;
    MenuStackc[0].CursorPos = 1;
    Menuc[0].VarPntr[0] = value;
    IncVariable(); assert(!strcmp(value, "  2"));
    DecVariable(); assert(!strcmp(value, "  1"));
    DecVariable(); assert(!strcmp(value, " 13"));
    nDigits = OFF_ON; strcpy(value, "OFF");
    IncVariable(); assert(!strcmp(value, " ON"));
    DecVariable(); assert(!strcmp(value, "OFF"));
    renders = 0;
    MenuStackc[0].Index[0] = 99; IncVariable(); DecVariable();
    MenuStackc[0].Index[0] = 1;
    MenuStackc[0].CursorPos = 0; IncVariable(); DecVariable();
    MenuStackc[0].CursorPos = 12; IncVariable(); DecVariable();
    MenuStackc[0].CursorPos = 1; Menuc[0].VarPntr[0] = NULL;
    IncVariable(); DecVariable();
    Menuc[0].VarPntr[0] = SerialNum;
    nDigits = SER_NUM; VariableFlag = 0; IncVariable(); DecVariable();
    VariableFlag = 6; IncVariable(); DecVariable();
    StackPointer = MenuStackSize; IncVariable(); DecVariable();
    assert(!renders && !strcmp(SerialNum, "12345"));
    return 0;
}
''')

    def test_timer_scan_stays_inside_matrix(self):
        source = (ROOT/'Embedded/Src/Interrupts.c').read_text()
        handler = extract(source, 'TC3_Int_Handler')
        run_c(r'''
#include <assert.h>
#define TC_1ms 1
#define TeleData_Rev 0x8000
#define TeleData_Fwd 0x4000
struct Matrix { unsigned int BB_Data; unsigned char LED_Port_Data[3]; };
struct Matrix Keypad_Matrix[18];
unsigned int ButtonBoxData = 0xffff, TC3, TCNT;
unsigned char TFLG1, PTP, PTT, PTJ, AlternateFunctionMask = 0xc0;
int legacy_mode;
void MCOHW_TimerISR(void) {}
''' + handler + r'''
int main(void) {
    int i;
    for (i = 0; i < 18; ++i) Keypad_Matrix[i].BB_Data = 1;
    for (i = 0; i < 1000; ++i) TC3_Int_Handler();
    return 0;
}
''')


if __name__ == '__main__':
    unittest.main()
