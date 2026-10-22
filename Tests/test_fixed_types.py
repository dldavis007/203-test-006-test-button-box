"""Check fixed-width ranges, signed sentinels and mask/byte preservation."""
from pathlib import Path
import unittest
from test_legacy_controls import extract, run_c

ROOT = Path(__file__).resolve().parents[1]


class FixedTypeTests(unittest.TestCase):
    def test_widths_sentinels_and_high_bits(self):
        run_c(r'''
#include <assert.h>
int main(void)
{
    uint32_t value;
    int8_t released = -1;
    uint16_t mask = 0xFFFF;
    assert(sizeof(int8_t) == 1 && sizeof(uint8_t) == 1);
    assert(sizeof(int16_t) == 2 && sizeof(uint16_t) == 2);
    assert(sizeof(int32_t) == 4 && sizeof(uint32_t) == 4);
    assert(released == -1 && (int16_t)-1 < 0);
    assert(mask == 65535U && (mask & 0x8000U));
    for (value = 0; value <= 65535U; ++value) {
        uint8_t low = (uint8_t)value;
        uint8_t high = (uint8_t)(value >> 8);
        uint16_t combined = (uint16_t)(((uint16_t)high << 8) | low);
        assert(combined == value);
    }
    return 0;
}
''')

    def test_hex_parser_retains_signed_target_result(self):
        source = (ROOT/'Embedded/Src/Command.c').read_text()
        run_c('#include <assert.h>\n' + extract(source, 'hextoi') + r'''
int main(void)
{
    assert(hextoi("7FFF") == 32767);
    assert(hextoi("8000") == (int16_t)-32768);
    assert(hextoi("FFFF") == (int16_t)-1);
    return 0;
}
''')


if __name__ == '__main__':
    unittest.main()
