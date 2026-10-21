"""Exercise the real PDO initializers with the button box's Node-ID zero."""
from pathlib import Path
import unittest
from test_legacy_controls import extract, run_c

ROOT = Path(__file__).resolve().parents[1]


class MCOInitializationTests(unittest.TestCase):
    def test_node_zero_and_parameter_boundaries(self):
        source = (ROOT/'Embedded/Src/mco.c').read_text()
        initializers = '\n'.join(extract(source, name) for name in
                                 ['MCO_InitRPDO', 'MCO_InitTPDO'])
        run_c(r'''
#include <assert.h>
#include <setjmp.h>
#include <stdint.h>
#include <string.h>
typedef uint8_t UNSIGNED8;
typedef uint16_t UNSIGNED16;
#define CHECK_PARAMETERS
#define USE_EVENT_TIME
#define USE_INHIBIT_TIME
#define NR_OF_RPDOS 8
#define NR_OF_TPDOS 8
#define PROCIMG_SIZE 80
struct { UNSIGNED8 Node_ID; } gMCOConfig;
struct { UNSIGNED8 len, offset; UNSIGNED16 CANID; } gRPDOConfig[8];
struct { struct { UNSIGNED16 ID; UNSIGNED8 LEN; } CAN;
         UNSIGNED8 offset; UNSIGNED16 event_time, inhibit_time; } gTPDOConfig[8];
static jmp_buf fatal;
static volatile UNSIGNED16 fatal_code;
static UNSIGNED16 filter_id;
void MCOUSER_FatalError(UNSIGNED16 code) { fatal_code = code; longjmp(fatal, 1); }
int MCOHW_SetCANFilter(UNSIGNED16 id) { filter_id = id; return 1; }
''' + initializers + r'''
int main(void) {
    unsigned int node;
    /* This application intentionally uses Node-ID zero and explicit CAN IDs. */
    for (node = 0; node < 128; ++node) {
        gMCOConfig.Node_ID = node;
        fatal_code = 0;
        if (setjmp(fatal)) assert(0);
        MCO_InitRPDO(1, 0x200, 1, 0);
        assert(gRPDOConfig[0].CANID == 0x200 && filter_id == 0x200);
        assert(gRPDOConfig[0].len == 1 && gRPDOConfig[0].offset == 0);
        MCO_InitTPDO(1, 0x421, 0, 10, 2, 3);
        assert(gTPDOConfig[0].CAN.ID == 0x421 && gTPDOConfig[0].CAN.LEN == 2);
        assert(gTPDOConfig[0].offset == 3 && gTPDOConfig[0].event_time == 0);
        assert(gTPDOConfig[0].inhibit_time == 10 && fatal_code == 0);
    }
    /* Every other byte value for Node-ID must still fail validation. */
    for (node = 128; node < 256; ++node) {
        gMCOConfig.Node_ID = node;
        fatal_code = 0;
        if (!setjmp(fatal)) { MCO_InitRPDO(1,0x200,1,0); assert(0); }
        assert(fatal_code == 0x8804);
        if (!setjmp(fatal)) { MCO_InitTPDO(1,0x421,0,10,2,3); assert(0); }
        assert(fatal_code == 0x8806);
    }
    gMCOConfig.Node_ID = 0;
    if (!setjmp(fatal)) { MCO_InitRPDO(0,0x200,1,0); assert(0); }
    assert(fatal_code == 0x8804);
    if (!setjmp(fatal)) { MCO_InitRPDO(9,0x200,1,0); assert(0); }
    assert(fatal_code == 0x8804);
    if (!setjmp(fatal)) { MCO_InitRPDO(1,0x200,1,80); assert(0); }
    assert(fatal_code == 0x8904);
    if (!setjmp(fatal)) { MCO_InitTPDO(0,0x421,0,10,2,3); assert(0); }
    assert(fatal_code == 0x8806);
    if (!setjmp(fatal)) { MCO_InitTPDO(9,0x421,0,10,2,3); assert(0); }
    assert(fatal_code == 0x8806);
    if (!setjmp(fatal)) { MCO_InitTPDO(1,0x421,0,10,0,3); assert(0); }
    assert(fatal_code == 0x8806);
    if (!setjmp(fatal)) { MCO_InitTPDO(1,0x421,0,10,9,3); assert(0); }
    assert(fatal_code == 0x8806);
    if (!setjmp(fatal)) { MCO_InitTPDO(1,0x421,0,0,2,3); assert(0); }
    assert(fatal_code == 0x8806);
    if (!setjmp(fatal)) { MCO_InitTPDO(1,0x421,0,10,2,80); assert(0); }
    assert(fatal_code == 0x8906);
    return 0;
}
''')


if __name__ == '__main__':
    unittest.main()
