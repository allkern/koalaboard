#include <stddef.h>
#include <stdint.h>

#include "stdlib.h"

/*
    Very bad but simple dynamic memory implementation.
    Fixed block size (4 KiB), find first 
    - 
*/
void* malloc(size_t size) {
    int i;

    for (i = 0; i < __alloc_blocks_size; i++)
        if (__alloc_blocks[i].free)
            break;

    __alloc_blocks[i].free = 0;

    return __ram_start + (i << ALLOC_BLOCK_SHIFT);
}

void* calloc(size_t num, size_t size) {
    size_t s = num * size;

    void* ptr = malloc(s);

    for (int i = 0; i < s; i++)
        *(unsigned char*)ptr = 0;

    return ptr;
}

void* realloc(void* ptr, size_t new_size) {
    return ptr;
}

void free(void* ptr) {
    __alloc_blocks[((uintptr_t)ptr) >> ALLOC_BLOCK_SHIFT].free = 0;
}