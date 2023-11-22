#include "console.h"

void _init_console(putchar_t putchar, getchar_t getchar) {
    g_current_console.putchar = putchar;
    g_current_console.getchar = getchar;
}

void _putchar(int c) {
    g_current_console.putchar(c);
}

int _getchar() {
    return g_current_console.getchar();
}