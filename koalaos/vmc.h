#ifndef VMC_H
#define VMC_H

#define VMC_EXIT    0x9f800000
#define VMC_PUTCHAR 0x9f800001

void vmc_exit(int code);
void vmc_putchar(int c);

#endif