#include "Controller.h"
#include "mc9s12a128.h"

/* Reserved by the linker and deliberately not cleared at reset. */
char _bss_end[20] __attribute__((section(".noinit")));

/* GNU embedded libc variants use write/_write or outbyte for stdio. */
int write(int fd, const char *buffer, int length)
{
    int i;
    (void)fd;
    for (i = 0; i < length; ++i) putchar((unsigned char)buffer[i]);
    return length;
}
int _write(int fd, const char *buffer, int length)
{
    return write(fd, buffer, length);
}
void outbyte(char c) { putchar((unsigned char)c); }
int read(int fd, char *buffer, int length)
{
    (void)fd;
    if (length <= 0) return 0;
    buffer[0] = (char)getchar();
    return 1;
}
int _read(int fd, char *buffer, int length) { return read(fd, buffer, length); }
int close(int fd) { (void)fd; return 0; }
int _close(int fd) { return close(fd); }
int isatty(int fd) { (void)fd; return 1; }
int _isatty(int fd) { return isatty(fd); }
long lseek(int fd, long offset, int origin)
{ (void)fd; (void)offset; (void)origin; return 0; }
long _lseek(int fd, long offset, int origin) { return lseek(fd, offset, origin); }
/* No dynamic heap: do not let libc allocate into the hardware stack. */
void *sbrk(int increment) { (void)increment; return (void *)-1; }
void *_sbrk(int increment) { return sbrk(increment); }

#ifndef BUTTONBOX_HOST_SYNTAX
typedef char assert_int16[(sizeof(int) == 2) ? 1 : -1];
typedef char assert_long32[(sizeof(long) == 4) ? 1 : -1];
typedef char assert_pointer16[(sizeof(void *) == 2) ? 1 : -1];
typedef char assert_double32[(sizeof(double) == 4) ? 1 : -1];
#endif
