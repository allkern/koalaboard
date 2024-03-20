#include "libc/stdio.h"

#include "sys/dmem.h"
#include "usr/shell.h"

void irq_handler(void) {
    shell_exec("color 2A");
    printf("\nirq_handler: Hello, world!\n");
    shell_exec("color");
}

void syscall(uint32_t num) {
    asm("syscall");
}

int usr_test(int argc, const char* argv[]) {
    printf("kernel: Installing IRQ handler at %0p... ", irq_handler);

    syscall(irq_handler);

    printf("done\nkernel: Enabling interrupts... ", irq_handler);

    asm volatile (
        ".set noat\n"
        "la      $at, 0x40ff03\n"
        "mtc0    $at, $12\n"
        ".set at\n"
    );

    puts("done\n");

    // Crash system
    // *((int*)0) = 0;

    return EXIT_SUCCESS;
}