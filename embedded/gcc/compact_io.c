#include <stdarg.h>
/* Use the device function, not a libc stdio macro. */
extern int putchar(int c);

struct Output {
    char *destination;
    int count;
};
static void emit(struct Output *out, char c)
{
    if (out->destination) *out->destination++ = c;
    else putchar((unsigned char)c);
    ++out->count;
}
static void padding(struct Output *out, int width, char c)
{
    while (width-- > 0) emit(out, c);
}
static int digits(char *buffer, unsigned long value, unsigned int base, int upper)
{
    char reverse[11];
    int count = 0, i;
    do {
        unsigned int digit = (unsigned int)(value % base);
        reverse[count++] = (char)(digit < 10 ? '0' + digit : (upper ? 'A' : 'a') + digit - 10);
        value /= base;
    } while (value);
    for (i = 0; i < count; ++i) buffer[i] = reverse[count - i - 1];
    return count;
}
static int format_output(struct Output *out, const char *format, va_list args)
{
    while (*format) {
        char buffer[40], conversion;
        int width = 0, precision = -1, zero = 0, alternate = 0, left = 0;
        int count = 0, negative = 0, islong = 0, i;
        const char *text = buffer;
        if (*format != '%') { emit(out, *format++); continue; }
        ++format;
        while (*format == '0' || *format == '#' || *format == '-') {
            if (*format == '0') zero = 1;
            if (*format == '#') alternate = 1;
            if (*format == '-') left = 1;
            ++format;
        }
        if (*format == '*') {
            width = va_arg(args, int); ++format;
            if (width < 0) { left = 1; width = -width; }
        } else while (*format >= '0' && *format <= '9') width = width * 10 + *format++ - '0';
        if (*format == '.') {
            ++format; precision = 0;
            while (*format >= '0' && *format <= '9') precision = precision * 10 + *format++ - '0';
        }
        if (*format == 'l') { islong = 1; ++format; }
        conversion = *format;
        if (!conversion) break;
        ++format;
        if (conversion == 's') {
            text = va_arg(args, const char *);
            if (!text) text = "(null)";
            while (text[count] && (precision < 0 || count < precision)) ++count;
            zero = 0;
        } else if (conversion == 'c') {
            buffer[count++] = (char)va_arg(args, int);
        } else if (conversion == '%') {
            buffer[count++] = '%';
        } else if (conversion == 'd' || conversion == 'i' || conversion == 'u' || conversion == 'x' || conversion == 'X') {
            unsigned long value;
            unsigned int base = (conversion == 'x' || conversion == 'X') ? 16 : 10;
            if (conversion == 'd' || conversion == 'i') {
                long number = islong ? va_arg(args, long) : va_arg(args, int);
                negative = number < 0;
                value = negative ? 0UL - (unsigned long)number : (unsigned long)number;
            } else value = islong ? va_arg(args, unsigned long) : va_arg(args, unsigned int);
            count = digits(buffer, value, base, conversion == 'X');
        } else if (conversion == 'f') {
            double value = va_arg(args, double);
            unsigned long scaled, scale = 1, fraction;
            int whole_count;
            if (precision < 0) precision = 2;
            if (precision > 2) precision = 2; /* Firmware's maximum precision. */
            negative = value < 0;
            if (negative) value = -value;
            for (i = 0; i < precision; ++i) scale *= 10;
            /* All firmware menu limits are far below this bound. */
            if (!(value >= 0 && value <= 10000000.0)) {
                text = "?"; count = 1; negative = 0;
            } else {
                scaled = (unsigned long)(value * scale + 0.5);
                whole_count = digits(buffer, scaled / scale, 10, 0);
                count = whole_count;
                if (precision || alternate) buffer[count++] = '.';
                fraction = scaled % scale;
                for (i = precision; i > 0; --i) {
                    scale /= 10;
                    buffer[count++] = (char)('0' + fraction / scale);
                    fraction %= scale;
                }
            }
        } else {
            /* An unsupported format is visible, never silently consumes arguments. */
            if (out->destination) *out->destination = 0;
            return -1;
        }
        width -= count + negative;
        if (!left && !zero) padding(out, width, ' ');
        if (negative) emit(out, '-');
        if (!left && zero) padding(out, width, '0');
        for (i = 0; i < count; ++i) emit(out, text[i]);
        if (left) padding(out, width, ' ');
    }
    if (out->destination) *out->destination = 0;
    return out->count;
}
int BB_sprintf(char *destination, const char *format, ...)
{
    struct Output out;
    va_list args;
    int result;
    out.destination = destination; out.count = 0;
    va_start(args, format); result = format_output(&out, format, args); va_end(args);
    return result;
}
int BB_printf(const char *format, ...)
{
    struct Output out;
    va_list args;
    int result;
    out.destination = 0; out.count = 0;
    va_start(args, format); result = format_output(&out, format, args); va_end(args);
    return result;
}
double BB_atof(const char *text)
{
    double value = 0, factor = 0.1;
    int negative = 0;
    while (*text == ' ' || (*text >= '\t' && *text <= '\r')) ++text;
    if (*text == '-' || *text == '+') negative = *text++ == '-';
    while (*text >= '0' && *text <= '9') value = value * 10 + *text++ - '0';
    if (*text == '.') {
        ++text;
        while (*text >= '0' && *text <= '9') { value += (*text++ - '0') * factor; factor *= 0.1; }
    }
    return negative ? -value : value;
}
