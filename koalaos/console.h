#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>

typedef void (*putchar_t)(int);
typedef int (*getchar_t)();

typedef struct {
    void* udata;
    putchar_t putchar;
    getchar_t getchar;
} console_t;

static console_t g_current_console;

void _init_console(putchar_t putchar, getchar_t getchar);
void _putchar(int c);
int _getchar();

#endif