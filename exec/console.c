#include "console.h"

static void _putchar(int c) {
    g_current_console.putchar(c);
}

static int _getchar() {
    return g_current_console.getchar();
}