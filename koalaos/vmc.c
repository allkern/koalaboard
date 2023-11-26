#include "vmc.h"
#include "mmio.h"

void vmc_exit(int code) {
    mmio_write_32(VMC_EXIT, code);
}

void vmc_putchar(int c) {
    mmio_write_8(VMC_PUTCHAR, c);
}