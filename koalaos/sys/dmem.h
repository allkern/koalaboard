#ifndef DMEM_H
#define DMEM_H

#include "libc/stdint.h"

#define TLB_ENTRY_COUNT 64

struct tlb_entry {
    uint32_t lo;
    uint32_t hi;
};

struct tlb_entry tlb_read_entry(int index);

#endif