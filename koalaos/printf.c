#include "console.h"
#include "printf.h"

#include <stdarg.h>

void print_string(const char* str) {
    while (*str)
        _putchar(*str++);
}

static char digits[] = "0123456789abcdef";

void print_int(int n, int base, int sign) {
    char buf[16];
    unsigned int x;

    if (sign && (sign = n < 0)) {
        x = -n;
    } else {
        x = n;
    }

    int i = 0;

    do {
        buf[i++] = digits[x % base];
    } while ((x /= base) != 0);

    if (sign)
        buf[i++] = '-';

    while(--i >= 0)
        _putchar(buf[i]);
}

void print_ptr(uintptr_t x) {
    for (int i = 0; i < (sizeof(uintptr_t) * 2); i++, x <<= 4)
        _putchar(digits[x >> (sizeof(uintptr_t) * 8 - 4)]);
}

int kprintf(const char* fmt, ...) {
    va_list args;

    __builtin_va_start(args, fmt);
 
    while (*fmt != '\0') {
        if (*fmt == '%') {
            char f = *(++fmt);

            if (f == 's') {
                const char* s = __builtin_va_arg(args, const char*);

                print_string(s);
            } else if (f == 'x') {
                unsigned int x = __builtin_va_arg(args, unsigned int);

                print_int(x, 16, 0);
            } else if (f == 'd') {
                int x = __builtin_va_arg(args, int);

                print_int(x, 10, 1);
            } else if (f == 'u') {
                unsigned int x = __builtin_va_arg(args, unsigned int);

                print_int(x, 10, 0);
            } else if (f == 'i') {
                int x = __builtin_va_arg(args, int);

                print_int(x, 10, 1);
            } else if (f == 'p') {
                uintptr_t p = __builtin_va_arg(args, uintptr_t);

                print_ptr(p);
            } else if (f == '%') {
                _putchar('%');
            } else {
                return 0;
            }
        } else {
            _putchar(*fmt);
        }

        ++fmt;
    }
 
    __builtin_va_end(args);

    return 1;
}