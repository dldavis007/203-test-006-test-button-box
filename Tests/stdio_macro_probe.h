/* Model the old GNU68HC12 stdio macros during host syntax checking. */
#include <stdio.h>
#define getchar() BB_FAKE_GETCHAR_MACRO()
#define putchar(c) BB_FAKE_PUTCHAR_MACRO(c)
