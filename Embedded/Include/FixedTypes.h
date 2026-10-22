#ifndef BUTTONBOX_FIXED_TYPES_H
#define BUTTONBOX_FIXED_TYPES_H

#include <limits.h>

/* Old target libraries need not provide C99 stdint.h. Preserve the 9S12
 * compiler's native ABI: char=8, int=16 and long=32 bits. Host checks use
 * stdint.h so converted numeric objects have target widths. Native-type MCO
 * interface objects are explicit exceptions; host integer promotions/ABI still
 * differ from the CPU12.
 */
#if defined(BUTTONBOX_HOST_SYNTAX) || (!defined(BUTTONBOX_GCC) && !defined(__ICC_VERSION))
#include <stdint.h>
#else
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed int int16_t;
typedef unsigned int uint16_t;
typedef signed long int32_t;
typedef unsigned long uint32_t;
#endif

typedef char bb_check_byte_width[(CHAR_BIT == 8) ? 1 : -1];
typedef char bb_check_int8[(sizeof(int8_t) == 1 && (int8_t)-1 < 0) ? 1 : -1];
typedef char bb_check_uint8[(sizeof(uint8_t) == 1 && (uint8_t)-1 == 255U) ? 1 : -1];
typedef char bb_check_int16[(sizeof(int16_t) == 2 && (int16_t)-1 < 0) ? 1 : -1];
typedef char bb_check_uint16[(sizeof(uint16_t) == 2 && (uint16_t)-1 == 65535U) ? 1 : -1];
typedef char bb_check_int32[(sizeof(int32_t) == 4) ? 1 : -1];
typedef char bb_check_uint32[(sizeof(uint32_t) == 4) ? 1 : -1];

#endif
