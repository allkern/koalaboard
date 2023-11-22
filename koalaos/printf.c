#include "console.h"
#include "printf.h"

#include <stdarg.h>

void __printf_string(const char* str) {
    while (*str)
        _putchar(*str++);
}

int _printf(const char* fmt, ...) {
    va_list args;

    __builtin_va_start(args, fmt);
 
    while (*fmt != '\0') {
        if (*fmt == '%') {
            char f = *(++fmt);

            if (f == 's') {
                const char* s = __builtin_va_arg(args, const char*);

                __printf_string(s);
            }
        } else {
            _putchar(*fmt);
        }

        ++fmt;
    }
 
    __builtin_va_end(args);
}