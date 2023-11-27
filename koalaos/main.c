#include "mmio.h"
#include "vmc.h"
#include "printf.h"
#include "tty.h"
#include "uart.h"
#include "gpu.h"
#include "font/vga16.h"

#ifdef _MSC_VER
#define __COMPILER__ "msvc"
#elif __GNUC__
#define __COMPILER__ "gcc"
#endif

#ifndef OS_INFO
#define OS_INFO unknown
#endif
#ifndef VERSION_TAG
#define VERSION_TAG latest
#endif
#ifndef COMMIT_HASH
#define COMMIT_HASH latest
#endif

#define STR1(m) #m
#define STR(m) STR1(m)

int strcmp(const char *s1, const char *s2) {
	while (*s1 == *s2++)
		if (*s1++ == '\0')
			return (0);
	return (*(const unsigned char *)s1 - *(const unsigned char *)(s2 - 1));
}

void exit(int code) {
    mmio_write_32(VMC_EXIT, code);
}

int xy = 0;

uint16_t get_char_uv(int c) {
#ifdef USE_FONT8
    return ((c & 0xf0) << 7) | ((c & 0xf) << 3);
#else
    return ((c & 0xf0) << 8) | ((c & 0xf) << 3);
#endif
}

void shell() {
    char buf[64];
    char* ptr = buf;

    for (int i = 0; i < 64; i++)
        buf[i] = 0;
    
    kprintf("\r> %s", buf);

    int c = tty_getchar();

    while (1) {
        kprintf("\r> %s", buf);

        while (c == -1)
            c = tty_getchar();

        if (c == '\r') {
            *ptr = 0;

            tty_putchar('\n');

            if (!strcmp(buf, "help")) {
                kprintf("Available commands:\n");
                kprintf("clear      - Clears the screen\n");
                kprintf("help       - Show a list of available commands\n");
                kprintf("ver        - Display KoalaOS' version information\n");
            } else if (!strcmp(buf, "ver")) {
                kprintf("KoalaOS 0.1-%s (%s %s %s)\n",
                    STR(COMMIT_HASH),
                    __COMPILER__,
                    __VERSION__,
                    STR(OS_INFO)
                );
            } else if (!strcmp(buf, "clear")) {
                xy = 0;

                mmio_write_32(GPU_GP0, 0x02010101);
                mmio_write_32(GPU_GP0, 0x00000000);
                mmio_write_32(GPU_GP0, 0x01e00280);
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

        kprintf("\r> %s", buf);

        c = tty_getchar();
    }
}

int main() {
    kprintf("Welcome to KoalaOS!\n\n");

    shell();

    return 0;
}

void gpu_putchar(int c) {
    if (c == '\n') {
        xy &= 0xffff0000;

#ifdef USE_FONT8
        xy += 0x80000;
#else
        xy += 0x100000;
#endif
        return;
    } else if (c == '\r') {
        xy &= 0xffff0000;

        return;
    }

    uint16_t uv = get_char_uv(c);

    mmio_write_32(GPU_GP0, 0x65000000);
    mmio_write_32(GPU_GP0, xy);
    mmio_write_32(GPU_GP0, 0x78000000 | uv);

#ifdef USE_FONT8
    mmio_write_32(GPU_GP0, 0x00080008);
#else
    mmio_write_32(GPU_GP0, 0x00100008);
#endif

    xy += 8;
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
    mmio_write_32(GPU_GP0, 0x02010101);
    mmio_write_32(GPU_GP0, 0x00000000);
    mmio_write_32(GPU_GP0, 0x01e00280);

    // Upload font to GPU
    mmio_write_32(GPU_GP0, 0xa0000000);
    mmio_write_32(GPU_GP0, 0x00000280);

#ifdef USE_FONT8
    mmio_write_32(GPU_GP0, 0x00400020);

    for (int i = 0; i < FONT8_SIZE; i++)
        mmio_write_32(GPU_GP0, g_font_vga8[i]);
#else
    mmio_write_32(GPU_GP0, 0x00800020);

    for (int i = 0; i < FONT16_SIZE; i++)
        mmio_write_32(GPU_GP0, g_font_vga16[i]);
#endif

    uint32_t clut[] = {
        0x63180421, 0x00000000,
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

    exit(main());
}

// DOS palette
// uint32_t g_palette[] = {
//     0x000000ff, 0x800000ff, 0x008000ff, 0x808000ff,
//     0x000080ff, 0x800080ff, 0x008080ff, 0xc0c0c0ff,
//     0x808080ff, 0xff0000ff, 0x00ff00ff, 0xffff00ff,
//     0x0000ffff, 0xff00ffff, 0x00ffffff, 0xffffffff
// };