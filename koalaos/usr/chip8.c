#include "libc/string.h"
#include "libc/stdio.h"
#include "sys/fs.h"
#include "hw/uart.h"
#include "hw/gpu.h"
#include "c8.h"

#include "util/mmio.h"
#include "shell.h"

int get_c8_key(int k) {
    if (k == '1') return 0x1;
    if (k == '2') return 0x2;
    if (k == '3') return 0x3;
    if (k == '4') return 0xc;
    if (k == 'q') return 0x4;
    if (k == 'w') return 0x5;
    if (k == 'e') return 0x6;
    if (k == 'r') return 0xd;
    if (k == 'a') return 0x7;
    if (k == 's') return 0x8;
    if (k == 'd') return 0x9;
    if (k == 'f') return 0xe;
    if (k == 'z') return 0xa;
    if (k == 'x') return 0x0;
    if (k == 'c') return 0xb;
    if (k == 'v') return 0xf;
    return -1;
}

chip8_t c8;
uint8_t screen[64 * 32];
uint8_t mem[0x1000];

int usr_chip8(int argc, const char* argv[]) {
    char* cwd = shell_get_cwd();

    const char* path;

    if (!argv[1]) {
        printf("Path \'%s\' does not point to a file\n", cwd);

        return 1;
    } else {
        path = argv[1];

        if (path[1] != ':') {
            char buf[128];

            char* p = cwd;
            char* q = buf;
            char* r = path;

            while (*p != '\0')
                *q++ = *p++;

            while (*r != '\0')
                *q++ = *r++;

            *q = '\0';

            path = buf;
        }
    }

    // Big buffer
    char buf[1024];

    struct file_s file;
    uint32_t status;

    if (fat_file_open(&file, path, strlen(path))) {
        printf("Could not open file \'%s\'\n", path);

        return 1;
    }

    if (fat_file_read(&file, buf, file.size, &status)) {
        printf("Could not read file \'%s\'\n", path);

        return 1;
    }

    gpu_clear();

    c8_init(&c8, screen, mem);
    c8_load_program(&c8, buf, file.size);

    int last_k = -1;

    while (1) {
        // Wait for GPU IRQ
        while (!(mmio_read_32(0x9f801070) & 1));

        // Acknowledge GPU IRQ
        mmio_write_32(0x9f801070, ~1ul);

        if (mmio_read_8(UART_LSR) & LSR_RX_READY) {
            int data = mmio_read_8(UART_RHR);

            // Exit emulator
            if (data == 0x1b) {
                gpu_clear();

                printf("Exited\n");

                return;
            }

            int k = get_c8_key(data);

            if (k != -1) {
                c8_keydown(&c8, k);

                last_k = k;
            }
        } else {
            if (last_k != -1)
                c8_keyup(&c8, last_k);

            last_k = -1;
        }

        int ips = 60;

        while (ips--)
            c8_execute_instruction(&c8);

        gpu_clear();

        for (int y = 0; y < 32; y++) {
            for (int x = 0; x < 64; x++) {
                mmio_write_32(GPU_GP0, 0x70000000 | (c8.screen[x + (y * 64)] ? 0x0000AAFF : 0x000044AA));
                mmio_write_32(GPU_GP0, ((y * 8) << 16) | (x * 8));
            }
        }
    }

    return EXIT_SUCCESS;
}