#include "mmio.h"
#include "vmc.h"
#include "printf.h"
#include "tty.h"
#include "uart.h"
#include "gpu.h"
#include "font.h"

int strcmp(const char *s1, const char *s2) {
	while (*s1 == *s2++)
		if (*s1++ == '\0')
			return (0);
	return (*(const unsigned char *)s1 - *(const unsigned char *)(s2 - 1));
}

void exit(int code) {
    mmio_write_32(VMC_EXIT, code);
}

int x = 0, y = 0;

uint16_t get_char_uv(int c) {
    return ((c & 0xf0) << 7) | ((c & 0xf) << 3);
}

void shell() {
    char buf[64];
    char* ptr = buf;

    for (int i = 0; i < 64; i++)
        buf[i] = 0;

    int c = tty_getchar();

    while (1) {
        while (c == -1)
            c = tty_getchar();

        if (c == '\r') {
            *ptr = 0;

            tty_putchar('\n');

            if (!strcmp(buf, "help")) {
                kprintf("Available commands:\n");
                kprintf("help - Show a list of available commands\n");
            } else {
                kprintf("Unrecognized command \'%s\'\n", buf);
            }

            tty_putchar('\n');

            for (int i = 0; i < 64; i++)
                buf[i] = 0;

            ptr = buf;

            goto next;
        } else if (c == '\b') {
            *--ptr = 0;

            goto next;
        }

        *ptr++ = c;
        
        kprintf("\r> %s", buf);

        next:

        c = tty_getchar();
    }
}

int main() {
    kprintf("Welcome to KoalaOS!\n\n");

    tty_putchar('>');

    shell();

    return 0xaa;
}

void gpu_putchar(int c) {
    if (c == '\n') {
        x = 0;
        y += 0x80000;

        return;
    } else if (c == '\r') {
        x = 0;

        return;
    }

    uint16_t uv = get_char_uv(c);

    mmio_write_32(GPU_GP0, 0x75000000);
    mmio_write_32(GPU_GP0, y | x);
    mmio_write_32(GPU_GP0, 0x78000000 | uv);

    x += 8;
}

/*
GP1(08h) - Display mode

  0-1   Horizontal Resolution 1     (0=256, 1=320, 2=512, 3=640) ;GPUSTAT.17-18
  2     Vertical Resolution         (0=240, 1=480, when Bit5=1)  ;GPUSTAT.19
  3     Video Mode                  (0=NTSC/60Hz, 1=PAL/50Hz)    ;GPUSTAT.20
  4     Display Area Color Depth    (0=15bit, 1=24bit)           ;GPUSTAT.21
  5     Vertical Interlace          (0=Off, 1=On)                ;GPUSTAT.22
  6     Horizontal Resolution 2     (0=256/320/512/640, 1=368)   ;GPUSTAT.16
  7     "Reverseflag"               (0=Normal, 1=Distorted)      ;GPUSTAT.14
  8-23  Not used (zero)
*/

void __start() {
    uart_init();
    tty_init(gpu_putchar, uart_recv_byte);

    // Reset GPU
    mmio_write_32(GPU_GP1, 0x00000000);

    // Set resolution to 640x480, NTSC, interlaced
    mmio_write_32(GPU_GP1, 0x08000027);

    // Set drawing area to 0,0-640,480
    mmio_write_32(GPU_GP0, 0xe3000000);
    mmio_write_32(GPU_GP0, 0xe4078280);

    // Clear the screen (fast)
    mmio_write_32(GPU_GP0, 0x02000000);
    mmio_write_32(GPU_GP0, 0x00000000);
    mmio_write_32(GPU_GP0, 0x01e00280);

    // Upload font to GPU
    mmio_write_32(GPU_GP0, 0xa0000000);
    mmio_write_32(GPU_GP0, 0x00000280);
    mmio_write_32(GPU_GP0, 0x00400020);

    for (int i = 0; i < FONT_SIZE; i++)
        mmio_write_32(GPU_GP0, g_font[i]);

    uint32_t clut[] = {
        0xffff0000, 0x00000000,
        0x00000000, 0x00000000,
        0x00000000, 0x00000000,
        0x00000000, 0x00000000
    };

    mmio_write_32(GPU_GP0, 0xa0000000);
    mmio_write_32(GPU_GP0, 0x01e00000);
    mmio_write_32(GPU_GP0, 0x00010010);

    for (int i = 0; i < 8; i++)
        mmio_write_32(GPU_GP0, clut[i]);

    // Set up texpage (x=640, y=0, enable drawing)
    mmio_write_32(GPU_GP0, 0xe100040a);

    kprintf("[%s] GPU init done\n", __FUNCTION__);

    exit(main());
}