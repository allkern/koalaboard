#include "mmio.h"
#include "vmc.h"

static char digits[] = "0123456789abcdef";

int putchar(char c) {
    mmio_write_8(VMC_PUTCHAR, c);

    return c;
}

static void printint(int xx, int base, int sign) {
    char buf[16];
    int i;
    unsigned int x;

    if(sign && (sign = xx < 0))
        x = -xx;
    else
        x = xx;

    i = 0;
    do {
        buf[i++] = digits[x % base];
    } while((x /= base) != 0);

    if(sign)
        buf[i++] = '-';

    while(--i >= 0)
        putchar(buf[i]);
}

int puts(char* str) {
    while (*str)
        putchar(*str++);

    putchar('\n');

    return 1;
}

void exit(int code) {
    mmio_write_32(VMC_EXIT, code);

    return;
}

int main() {
    puts("Hello, world!");
}

void __start() {
    main();

    exit(1);
}
