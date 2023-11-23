#include "mmio.h"
#include "vmc.h"
#include "printf.h"
#include "console.h"
#include "uart.h"

void exit(int code) {
    mmio_write_32(VMC_EXIT, code);
}

int main() {
    kprintf("fmt: %s %x %d %u %i %p %%\n",
        "a string",
        0xbaadf00d,
        1234,
        4321,
        -1000,
        main
    );

    return 0xaa;
}

void __start() {
    uart_init();
    _init_console(uart_send_byte, uart_recv_byte);

    exit(main());
}