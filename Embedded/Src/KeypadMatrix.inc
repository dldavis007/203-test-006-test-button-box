/* Button-to-port and LED keypad mapping. Included only by Subroutines.c
 * at the original definition location. Entries stay in hardware scan order.
 * Binary comments show raw masks, not active-high/active-low button states.
 * Switch Port B: bits 5:3 select columns; bits 2:0 sense rows.
 * Switch Port T contains the other row masks. LED arrays map to P, T and J.
 * Keep hex constants: GNU68HC12 GCC 3.3.6 does not accept 0b literals.
 */
const struct MatrixTbl Keypad_Matrix[18] = {

    /* [00] Trigger / record */
    {
        0x0100,                  /* Button message: TeleData_Trig */
        /* Message mask bits:    0000000100000000 */

        /* Switch inputs:        Port T      Port B */
        /* Binary masks:         00000000    00001100 */
        {                       0x00,       0x0C },
        /* Message mask bits:    0000000000000000 */

        /* LED outputs:          Port P      Port T      Port J */
        /* Binary masks:         00011000    00000000    00000000 */
        {                       0x18,       0x00,       0x00 }
        /* Message mask bits:    0000000000011000 */
    },

    /* [01] F1 */
    {
        0x0080,                  /* Button message: TeleData_F1 */
        /* Message mask bits:    0000000010000000 */

        /* Switch inputs:        Port T      Port B */
        /* Binary masks:         00000000    00001010 */
        {                       0x00,       0x0A },
        /* Message mask bits:    0000000000000000 */

        /* LED outputs:          Port P      Port T      Port J */
        /* Binary masks:         00001000    00000000    01000000 */
        {                       0x08,       0x00,       0x40 }
        /* Message mask bits:    0000000000001000 */
    },

    /* [02] Reverse */
    {
        0x8000,                  /* Button message: TeleData_Rev */
        /* Message mask bits:    1000000000000000 */

        /* Switch inputs:        Port T      Port B */
        /* Binary masks:         00000000    00001001 */
        {                       0x00,       0x09 },
        /* Message mask bits:    0000000000000000 */

        /* LED outputs:          Port P      Port T      Port J */
        /* Binary masks:         00001000    00000000    10000000 */
        {                       0x08,       0x00,       0x80 }
        /* Message mask bits:    0000000000001000 */
    },

    /* [03] Extra slow */
    {
        0x2000,                  /* Button message: TeleData_ExSlow */
        /* Message mask bits:    0010000000000000 */

        /* Switch inputs:        Port T      Port B */
        /* Binary masks:         01000000    00001000 */
        {                       0x40,       0x08 },
        /* Message mask bits:    0000000001000000 */

        /* LED outputs:          Port P      Port T      Port J */
        /* Binary masks:         00001001    00000000    00000000 */
        {                       0x09,       0x00,       0x00 }
        /* Message mask bits:    0000000000001001 */
    },

    /* [04] Down */
    {
        0x0001,                  /* Button message: TeleData_Down */
        /* Message mask bits:    0000000000000001 */

        /* Switch inputs:        Port T      Port B */
        /* Binary masks:         00100000    00001000 */
        {                       0x20,       0x08 },
        /* Message mask bits:    0000000000100000 */

        /* LED outputs:          Port P      Port T      Port J */
        /* Binary masks:         00001000    00000010    00000000 */
        {                       0x08,       0x02,       0x00 }
        /* Message mask bits:    0000000000001000 */
    },

    /* [05] Select */
    {
        0x0004,                  /* Button message: TeleData_Select */
        /* Message mask bits:    0000000000000100 */

        /* Switch inputs:        Port T      Port B */
        /* Binary masks:         00010000    00001000 */
        {                       0x10,       0x08 },
        /* Message mask bits:    0000000000010000 */

        /* LED outputs:          Port P      Port T      Port J */
        /* Binary masks:         00001000    00000100    00000000 */
        {                       0x08,       0x04,       0x00 }
        /* Message mask bits:    0000000000001000 */
    },

    /* [06] Option / data */
    {
        0x0400,                  /* Button message: TeleData_Data */
        /* Message mask bits:    0000010000000000 */

        /* Switch inputs:        Port T      Port B */
        /* Binary masks:         00000000    00100100 */
        {                       0x00,       0x24 },
        /* Message mask bits:    0000000000000000 */

        /* LED outputs:          Port P      Port T      Port J */
        /* Binary masks:         00010100    00000000    00000000 */
        {                       0x14,       0x00,       0x00 }
        /* Message mask bits:    0000000000010100 */
    },

    /* [07] F2 */
    {
        0x0040,                  /* Button message: TeleData_F2 */
        /* Message mask bits:    0000000001000000 */

        /* Switch inputs:        Port T      Port B */
        /* Binary masks:         00000000    00100010 */
        {                       0x00,       0x22 },
        /* Message mask bits:    0000000000000000 */

        /* LED outputs:          Port P      Port T      Port J */
        /* Binary masks:         00000100    00000000    01000000 */
        {                       0x04,       0x00,       0x40 }
        /* Message mask bits:    0000000000000100 */
    },

    /* [08] Stop */
    {
        0x0000,                  /* Button message: zero: Stop is handled separately */
        /* Message mask bits:    0000000000000000 */

        /* Switch inputs:        Port T      Port B */
        /* Binary masks:         00000000    00100001 */
        {                       0x00,       0x21 },
        /* Message mask bits:    0000000000000000 */

        /* LED outputs:          Port P      Port T      Port J */
        /* Binary masks:         00000100    00000000    10000000 */
        {                       0x04,       0x00,       0x80 }
        /* Message mask bits:    0000000000000100 */
    },

    /* [09] Slow */
    {
        0x1000,                  /* Button message: TeleData_Slow */
        /* Message mask bits:    0001000000000000 */

        /* Switch inputs:        Port T      Port B */
        /* Binary masks:         01000000    00100000 */
        {                       0x40,       0x20 },
        /* Message mask bits:    0000000001000000 */

        /* LED outputs:          Port P      Port T      Port J */
        /* Binary masks:         00000101    00000000    00000000 */
        {                       0x05,       0x00,       0x00 }
        /* Message mask bits:    0000000000000101 */
    },

    /* [10] Menu (Down + Up) */
    {
        0x0003,                  /* Button message: TeleData_Down | TeleData_Up */
        /* Message mask bits:    0000000000000011 */

        /* Switch inputs:        Port T      Port B */
        /* Binary masks:         00100000    00100000 */
        {                       0x20,       0x20 },
        /* Message mask bits:    0000000000100000 */

        /* LED outputs:          Port P      Port T      Port J */
        /* Binary masks:         00000100    00000010    00000000 */
        {                       0x04,       0x02,       0x00 }
        /* Message mask bits:    0000000000000100 */
    },

    /* [11] Rotate clockwise */
    {
        0x0008,                  /* Button message: TeleData_RotCW */
        /* Message mask bits:    0000000000001000 */

        /* Switch inputs:        Port T      Port B */
        /* Binary masks:         00010000    00100000 */
        {                       0x10,       0x20 },
        /* Message mask bits:    0000000000010000 */

        /* LED outputs:          Port P      Port T      Port J */
        /* Binary masks:         00000100    00000100    00000000 */
        {                       0x04,       0x04,       0x00 }
        /* Message mask bits:    0000000000000100 */
    },

    /* [12] Reset */
    {
        0x0200,                  /* Button message: TeleData_Reset */
        /* Message mask bits:    0000001000000000 */

        /* Switch inputs:        Port T      Port B */
        /* Binary masks:         00000000    00010100 */
        {                       0x00,       0x14 },
        /* Message mask bits:    0000000000000000 */

        /* LED outputs:          Port P      Port T      Port J */
        /* Binary masks:         00010010    00000000    00000000 */
        {                       0x12,       0x00,       0x00 }
        /* Message mask bits:    0000000000010010 */
    },

    /* [13] F3 */
    {
        0x0020,                  /* Button message: TeleData_F3 */
        /* Message mask bits:    0000000000100000 */

        /* Switch inputs:        Port T      Port B */
        /* Binary masks:         00000000    00010010 */
        {                       0x00,       0x12 },
        /* Message mask bits:    0000000000000000 */

        /* LED outputs:          Port P      Port T      Port J */
        /* Binary masks:         00000010    00000000    01000000 */
        {                       0x02,       0x00,       0x40 }
        /* Message mask bits:    0000000000000010 */
    },

    /* [14] Forward */
    {
        0x4000,                  /* Button message: TeleData_Fwd */
        /* Message mask bits:    0100000000000000 */

        /* Switch inputs:        Port T      Port B */
        /* Binary masks:         00000000    00010001 */
        {                       0x00,       0x11 },
        /* Message mask bits:    0000000000000000 */

        /* LED outputs:          Port P      Port T      Port J */
        /* Binary masks:         00000010    00000000    10000000 */
        {                       0x02,       0x00,       0x80 }
        /* Message mask bits:    0000000000000010 */
    },

    /* [15] Fast */
    {
        0x0800,                  /* Button message: TeleData_Fast */
        /* Message mask bits:    0000100000000000 */

        /* Switch inputs:        Port T      Port B */
        /* Binary masks:         01000000    00010000 */
        {                       0x40,       0x10 },
        /* Message mask bits:    0000000001000000 */

        /* LED outputs:          Port P      Port T      Port J */
        /* Binary masks:         00000011    00000000    00000000 */
        {                       0x03,       0x00,       0x00 }
        /* Message mask bits:    0000000000000011 */
    },

    /* [16] Up */
    {
        0x0002,                  /* Button message: TeleData_Up */
        /* Message mask bits:    0000000000000010 */

        /* Switch inputs:        Port T      Port B */
        /* Binary masks:         00100000    00010000 */
        {                       0x20,       0x10 },
        /* Message mask bits:    0000000000100000 */

        /* LED outputs:          Port P      Port T      Port J */
        /* Binary masks:         00000010    00000010    00000000 */
        {                       0x02,       0x02,       0x00 }
        /* Message mask bits:    0000000000000010 */
    },

    /* [17] Rotate counterclockwise */
    {
        0x0010,                  /* Button message: TeleData_RotCCW */
        /* Message mask bits:    0000000000010000 */

        /* Switch inputs:        Port T      Port B */
        /* Binary masks:         00010000    00010000 */
        {                       0x10,       0x10 },
        /* Message mask bits:    0000000000010000 */

        /* LED outputs:          Port P      Port T      Port J */
        /* Binary masks:         00000010    00000100    00000000 */
        {                       0x02,       0x04,       0x00 }
        /* Message mask bits:    0000000000000010 */
    }
};
