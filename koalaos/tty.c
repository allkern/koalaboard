#include "tty.h"

void tty_init(putchar_t putchar, getchar_t getchar) {
    g_current_tty.putchar = putchar;
    g_current_tty.getchar = getchar;
}

void tty_putchar(int c) {
    g_current_tty.putchar(c);
}

int tty_getchar() {
    return g_current_tty.getchar();
}