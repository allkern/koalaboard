#include "mmio.h"
#include "vmc.h"
#include "printf.h"
#include "tty.h"
#include "uart.h"
#include "gpu.h"
#include "string.h"
#include "font/vga16.h"
#include "config.h"
#include "c8.h"

void gpu_putchar(int);

/* C:\Users\Lisandro\Downloads\Sierpinski [Sergey Naydenov, 2010].ch8 (12/5/2023 1:29:47 AM)
   StartOffset(h): 00000000, EndOffset(h): 00000208, Length(h): 00000209 */

unsigned char rawData[521] = {
	0x12, 0x05, 0x43, 0x38, 0x50, 0x60, 0x00, 0x85, 0x00, 0x60, 0x01, 0x81,
	0x50, 0xA3, 0xE6, 0xF1, 0x1E, 0xF0, 0x55, 0x60, 0x1F, 0x8A, 0x00, 0x60,
	0x00, 0x8B, 0x00, 0xA3, 0xC2, 0xF0, 0x65, 0xA3, 0xC2, 0xDA, 0xB1, 0x60,
	0x01, 0xA3, 0xC3, 0xF0, 0x55, 0x60, 0x1F, 0xA4, 0x06, 0xF0, 0x55, 0x60,
	0x01, 0xA3, 0xC4, 0xF0, 0x55, 0xA3, 0xC3, 0xF0, 0x65, 0x85, 0x00, 0x60,
	0x01, 0x81, 0x00, 0x80, 0x50, 0x80, 0x14, 0xA4, 0x07, 0xF0, 0x55, 0xA3,
	0xC4, 0xF0, 0x65, 0x85, 0x00, 0x60, 0x01, 0x81, 0x00, 0x80, 0x50, 0x80,
	0x15, 0xA3, 0xC5, 0xF0, 0x55, 0xA3, 0xC4, 0xF0, 0x65, 0x85, 0x00, 0xA3,
	0xC5, 0xF0, 0x65, 0xA3, 0xE6, 0xF0, 0x1E, 0xF0, 0x65, 0x86, 0x00, 0xA3,
	0xC4, 0xF0, 0x65, 0x87, 0x00, 0x60, 0x01, 0x81, 0x00, 0x80, 0x70, 0x80,
	0x14, 0xA3, 0xE6, 0xF0, 0x1E, 0xF0, 0x65, 0x81, 0x00, 0x80, 0x60, 0x80,
	0x13, 0x81, 0x50, 0xA3, 0xC6, 0xF1, 0x1E, 0xF0, 0x55, 0xA3, 0xC5, 0xF0,
	0x65, 0x85, 0x00, 0xA3, 0xC5, 0xF0, 0x65, 0xA3, 0xC6, 0xF0, 0x1E, 0xF0,
	0x65, 0x81, 0x50, 0xA3, 0xE6, 0xF1, 0x1E, 0xF0, 0x55, 0xA3, 0xC4, 0xF0,
	0x65, 0xA3, 0xC6, 0xF0, 0x1E, 0xF0, 0x65, 0x85, 0x00, 0x60, 0x01, 0x81,
	0x50, 0x50, 0x10, 0x6F, 0x01, 0x3F, 0x00, 0x12, 0xF9, 0xA3, 0xC4, 0xF0,
	0x65, 0x85, 0x00, 0x60, 0x1F, 0x81, 0x00, 0x80, 0x50, 0x80, 0x14, 0x8A,
	0x00, 0xA3, 0xC3, 0xF0, 0x65, 0x8B, 0x00, 0xA3, 0xC2, 0xF0, 0x65, 0xA3,
	0xC2, 0xDA, 0xB1, 0x60, 0x1F, 0x85, 0x00, 0xA3, 0xC4, 0xF0, 0x65, 0x81,
	0x00, 0x80, 0x50, 0x80, 0x15, 0x8A, 0x00, 0xA3, 0xC3, 0xF0, 0x65, 0x8B,
	0x00, 0xA3, 0xC2, 0xF0, 0x65, 0xA3, 0xC2, 0xDA, 0xB1, 0xA3, 0xC4, 0xF0,
	0x65, 0x85, 0x00, 0xA4, 0x07, 0xF0, 0x65, 0x81, 0x00, 0x80, 0x50, 0x82,
	0x10, 0x81, 0x05, 0x81, 0x20, 0x90, 0x10, 0x6F, 0x00, 0x3F, 0x01, 0x13,
	0x21, 0xA3, 0xC4, 0xF0, 0x65, 0x70, 0x01, 0xA3, 0xC4, 0xF0, 0x55, 0x12,
	0x47, 0xA3, 0xC3, 0xF0, 0x65, 0x85, 0x00, 0xA4, 0x06, 0xF0, 0x65, 0x81,
	0x00, 0x80, 0x50, 0x82, 0x10, 0x81, 0x05, 0x81, 0x20, 0x90, 0x10, 0x6F,
	0x00, 0x3F, 0x01, 0x13, 0x49, 0xA3, 0xC3, 0xF0, 0x65, 0x70, 0x01, 0xA3,
	0xC3, 0xF0, 0x55, 0x12, 0x2F, 0x13, 0x49, 0x81, 0x00, 0xA4, 0x08, 0x62,
	0x01, 0x8E, 0x25, 0xFE, 0x1E, 0xF0, 0x65, 0x00, 0xEE, 0x62, 0x01, 0x63,
	0x00, 0x83, 0x04, 0x81, 0x25, 0x31, 0x00, 0x13, 0x5D, 0x80, 0x30, 0x00,
	0xEE, 0xA4, 0x08, 0xFE, 0x1E, 0xF6, 0x55, 0x66, 0x00, 0x82, 0x00, 0x82,
	0x15, 0x3F, 0x01, 0x13, 0x95, 0x83, 0x00, 0x83, 0x06, 0x84, 0x10, 0x65,
	0x01, 0x82, 0x30, 0x82, 0x45, 0x3F, 0x01, 0x13, 0x8F, 0x84, 0x0E, 0x85,
	0x0E, 0x13, 0x81, 0x80, 0x45, 0x86, 0x54, 0x13, 0x71, 0xF5, 0x65, 0x80,
	0x60, 0x00, 0xEE, 0x82, 0x00, 0x80, 0x15, 0x3F, 0x00, 0x13, 0x9B, 0x80,
	0x20, 0x00, 0xEE, 0xA3, 0xBF, 0xF0, 0x33, 0xF2, 0x65, 0xF0, 0x29, 0xD3,
	0x45, 0x73, 0x06, 0xF1, 0x29, 0xD3, 0x45, 0x73, 0x06, 0xF2, 0x29, 0xD3,
	0x45, 0x00, 0xEE, 0x28, 0x63, 0x29, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00
};

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

chip8_t c8;
uint8_t screen[64 * 32];
uint8_t mem[0x1000];

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

void chip8() {
    mmio_write_32(GPU_GP0, 0x02080808);
    mmio_write_32(GPU_GP0, 0x00000000);
    mmio_write_32(GPU_GP0, 0x01e00280);

    c8_init(&c8, screen, mem);
    c8_load_program(&c8, rawData, sizeof(rawData));

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
                xy = 0;

                mmio_write_32(GPU_GP0, 0x02080808);
                mmio_write_32(GPU_GP0, 0x00000000);
                mmio_write_32(GPU_GP0, 0x01e00280);

                kprintf("Exited\n");

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

        int ips = 60 * 8;

        while (ips--)
            c8_execute_instruction(&c8);

        mmio_write_32(GPU_GP0, 0x02080808);
        mmio_write_32(GPU_GP0, 0x00000000);
        mmio_write_32(GPU_GP0, 0x01e00280);

        for (int y = 0; y < 32; y++) {
            for (int x = 0; x < 64; x++) {
                mmio_write_32(GPU_GP0, 0x70000000 | (c8.screen[x + (y * 64)] ? 0x0000AAFF : 0x000044AA));
                mmio_write_32(GPU_GP0, ((y * 8) << 16) | (x * 8));
            }
        }
    }
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
                kprintf("chip8      - Launch CHIP-8 emulator\n");
                kprintf("help       - Show a list of available commands\n");
                kprintf("ver        - Display KoalaOS' version information\n");
            } else if (!strcmp(buf, "ver")) {
                kprintf("KoalaOS 0.1-%s (%s %s %s %s)\n",
                    STR(COMMIT_HASH),
                    __COMPILER__,
                    __ARCH__,
                    __VERSION__,
                    STR(OS_INFO)
                );
            } else if (!strcmp(buf, "clear")) {
                xy = 0;

                mmio_write_32(GPU_GP0, 0x02080808);
                mmio_write_32(GPU_GP0, 0x00000000);
                mmio_write_32(GPU_GP0, 0x01e00280);
            } else if (!strcmp(buf, "chip8")) {
                chip8();
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
    mmio_write_32(GPU_GP0, 0x02080808);
    mmio_write_32(GPU_GP0, 0x00000000);
    mmio_write_32(GPU_GP0, 0x01e00280);

    // Upload font to GPU
    mmio_write_32(GPU_GP0, 0xa0000000);
    mmio_write_32(GPU_GP0, 0x00000280);

#ifdef USE_FONT8
    mmio_write_32(GPU_GP0, 0x00800020);

    for (int i = 0; i < FONT8_SIZE; i++)
        mmio_write_32(GPU_GP0, g_font_vga8[i]);
#else
    mmio_write_32(GPU_GP0, 0x01000020);

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