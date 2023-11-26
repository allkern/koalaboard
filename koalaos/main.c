#include "mmio.h"
#include "vmc.h"
#include "printf.h"
#include "tty.h"
#include "uart.h"
#include "gpu.h"

void exit(int code) {
    mmio_write_32(VMC_EXIT, code);
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

            kprintf("command=%s\n", buf);

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
    kprintf("Welcome to KoalaOS!\n");

    tty_putchar('>');

    shell();

    return 0xaa;
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
    tty_init(uart_send_byte, uart_recv_byte);

    // Reset GPU
    mmio_write_32(GPU_GP1, 0x00000000);

    // Set resolution to 640x480, NTSC, interlaced
    mmio_write_32(GPU_GP1, 0x08000027);

    // Set drawing area to 0,0-640,480
    mmio_write_32(GPU_GP0, 0xe3000000);
    mmio_write_32(GPU_GP0, 0xe4078280);

    // Draw a 640x480 blue rectangle (fast)
    mmio_write_32(GPU_GP0, 0x0200ff00);
    mmio_write_32(GPU_GP0, 0x00000000);
    mmio_write_32(GPU_GP0, 0x01e00280);

    // Send our texture to the GPU
    mmio_write_32(GPU_GP0, 0xa0000000);
    // mmio_write_32(GPU_GP0, 0x01e00280);
    // mmio_write_32(GPU_GP0, 0x00100010);

    // for (int i = 0; i < ((16 * 16) >> 1); i++)
    //     mmio_write_32(GPU_GP0, g_texture[i]);

    exit(main());
}