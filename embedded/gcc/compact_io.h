#ifndef BB_COMPACT_IO_H
#define BB_COMPACT_IO_H
/* Formats used by this firmware: %s, %c, %d/%u/%x/%X, %f (0..2 decimals),
   field width including *, zero padding, # and %%. No heap or stdio stream. */
int BB_sprintf(char *destination, const char *format, ...);
int BB_printf(const char *format, ...);
double BB_atof(const char *text);
#define sprintf BB_sprintf
#define printf BB_printf
#define atof BB_atof
#endif
