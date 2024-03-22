#include "hw/uart.h"
#include "hw/gpu.h"
#include "hw/vmc.h"
#include "hw/nvs.h"
#include "font/vga16.h"
#include "libc/stdio.h"
#include "libc/stdlib.h"
#include "sys/fs.h"
#include "usr/dir.h"
#include "usr/shell.h"

int main(void);

int main_return_code;

void vmc_exit_wrapper(void) {
    vmc_exit(main_return_code);
}

uint8_t buf[0x1000];

void fat32_init() {
    // printf("Scanning NVS bus...\n");

    int i;

    for (i = 0; i < 4; i++) {
        int status = nvs_get_status(i);

        if (nvs_get_status(i) & NVS_STAT_PROBE) {
            nvs_read_ident(i, buf);

            nvs_id* id = (nvs_id*)buf;

            if (id->type) {
                // printf("  Mounting drive %u: %s %s... ", i, id->manufacturer, id->model);

                if (disk_mount(i)) {
                    // printf("ok\n");
                }
            }
        }
    }

    current_volume = volume_get_first();
}

void __start() {
    // Basic init
    gpu_init(g_font_vga16, 16);

    __libc_init_stdio(uart_recv_byte, uart_send_byte);

    // printf("Booting KoalaOS...\n");

    // printf("Initialized stdio\n");

    // printf("Initializing stdlib... ");

    __libc_init_stdlib();

    // printf("ok\n");

    // printf("Initializing UART hardware... ");

    uart_init();

    // printf("ok\n");

    fat32_init();

    // printf("Calling main...\n");

    gpu_clear();

    // main call
    main_return_code = main();

    // Register vmc_exit call
    atexit(vmc_exit_wrapper);

    exit(EXIT_SUCCESS);
}