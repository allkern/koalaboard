#include "libc/stdio.h"

#include "dmem.h"

struct tlb_entry tlb_read_entry(int index) {
    struct tlb_entry entry;

    asm (
        "mtc0 %2, $0\n"
        "tlbr\n"
        "mfc0 %0, $2\n"
        "mfc0 %1, $10\n"
        : "=r" (entry.lo), "=r" (entry.hi)
        : "r" (index)
    );

    return entry;
}