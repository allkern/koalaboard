#include "libc/stdio.h"

#include "sys/dmem.h"

void irq_handler(void) {
    printf("Hello, world! (from IRQ handler)\n");
}

void syscall(uint32_t num) {
    asm("syscall");
}

int usr_test(int argc, const char* argv[]) {
    // printf("argc=%u argv=%p argv[0]=%p\n", argc, argv, argv[0]);

    // for (int i = 0; i < argc; i++) {
    //     printf("arg %u: \"%s\"\n", i, argv[i]);
    // }

    int index = 0;

    if (argv[1])
        index = atoi(argv[1]);

    struct tlb_entry entry;
    
    entry = tlb_read_entry(index);

    printf("TLB entry %u: lo=0x%08x hi=0x%08x\n\n",
        index, entry.lo, entry.hi
    );

    int free_blocks = 0;

    for (int i = 0; i < TLB_ENTRY_COUNT; i++) {
        entry = tlb_read_entry(i);

        if ((!entry.lo) && (!entry.hi))
            ++free_blocks;
    }

    printf("Free dynamic memory: %u KiB (%u TLB entries used)\n",
        free_blocks * 4,
        64 - free_blocks
    );

    syscall(irq_handler);

    // Crash system
    // *((int*)0) = 0;

    free_blocks = 0;

    return EXIT_SUCCESS;
}