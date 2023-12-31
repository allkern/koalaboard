#include "hw/uart.h"
#include "hw/gpu.h"
#include "hw/vmc.h"
#include "hw/nvs.h"
#include "font/vga16.h"
#include "libc/stdio.h"
#include "libc/stdlib.h"
#include "sys/fs.h"

int main(void);

int main_return_code;

void vmc_exit_wrapper(void) {
    vmc_exit(main_return_code);
}

uint8_t header[0x1000];

void __start() {
    // Basic init
    gpu_init(g_font_vga16, 16);

    __libc_init_stdio(uart_recv_byte, gpu_putchar);

    printf("Booting KoalaOS...\n");

    printf("Initialized stdio\n");

    printf("Initializing stdlib... ");

    __libc_init_stdlib();

    printf("ok\n");

    printf("Initializing UART hardware... ");

    uart_init();

    printf("ok\n");

    nvs_read_sector(header, 0);

    if (!strncmp(&header[0x200], "KFS", 3)) {
        printf("Mounted disk0 at \"\\\"\n");
    } else {
        printf("Found NVS device at %x\n", NVS_PHYS_BASE);
        printf("Formatting drive...\n");

        fs_format(header);

        int foo = fs_create_directory("foo", 0);
        int bar = fs_create_directory("bar", foo);

        fs_create_file("myfile.txt", header, 0x1000, bar);

        printf("Mounted disk0 at \"\\\"\n");
    }

    printf("Calling main...\n");

    gpu_clear();

    // main call
    main_return_code = main();

    // Register vmc_exit call
    atexit(vmc_exit_wrapper);

    exit(EXIT_SUCCESS);
}