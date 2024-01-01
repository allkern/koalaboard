#include "stdint.h"
#include "stdlib.h"

void* __ram_start = 0;
void* __ram_end = 0;

static __alloc_block* __alloc_blocks;
static int __alloc_blocks_size;

#define ATEXIT_MAX 32

void (*__atexit_func_array[ATEXIT_MAX])(void);
static int __atexit_func_index;

void __libc_init_stdlib(void) {
    __alloc_blocks = (__alloc_block*)__ram_start;
}

int atexit(void (*func)(void)) {
    __atexit_func_array[__atexit_func_index++] = func;
}

void exit(int code) {
    for (int i = 0; i < __atexit_func_index && i < ATEXIT_MAX; i++)
        __atexit_func_array[i]();
}

/*
    Very bad but simple dynamic memory implementation.
    Fixed block size (4 KiB)
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