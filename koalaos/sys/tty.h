#ifndef TTY_H
#define TTY_H

#include "libc/stdint.h"

typedef void (*putchar_t)(int);
typedef int (*getchar_t)();

typedef struct {
    void* udata;
    putchar_t putchar;
    getchar_t getchar;
} tty_t;

static tty_t g_current_tty;

void tty_init(putchar_t putchar, getchar_t getchar);
void tty_putchar(int c);
int tty_getchar();

#endif