#include "hw/uart.h"
#include "hw/gpu.h"
#include "hw/vmc.h"
#include "hw/nvs.h"
#include "font/vga8.h"
#include "libc/stdio.h"
#include "libc/stdlib.h"
#include "sys/ext2.h"
#include "usr/dir.h"
#include "usr/shell.h"
#include "sys/user.h"

int main(void);

int main_return_code;

void vmc_exit_wrapper(void) {
    vmc_exit(main_return_code);
}

void __start() {
    gpu_init(g_font_vga8, 8);

    __libc_init_stdio(uart_recv_byte, gpu_putchar);
    __libc_init_stdlib();

    // Register vmc_exit call
    atexit(vmc_exit_wrapper);

    uart_init();

    if (ext2_init())
        exit(EXIT_FAILURE);

    if (user_init())
        exit(EXIT_FAILURE);

    gpu_clear();

    while (1)
        main();

    exit(EXIT_SUCCESS);
}