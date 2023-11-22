#include "mmio.h"
#include "vmc.h"
#include "printf.h"
#include "console.h"
#include "uart.h"

void exit(int code) {
    mmio_write_32(VMC_EXIT, code);
}

int main() {
    _printf("Hello, world!\n");
}

typedef int (*func_t)(int);

void __start() {
    uart_init();
    _init_console(uart_send_byte, uart_recv_byte);

    _printf("Hello, world!\n");

    exit(1);
}
