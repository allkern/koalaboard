#ifndef STDLIB_H
#define STDLIB_H

#include <stddef.h>

void* malloc(size_t size);
void* calloc(size_t num, size_t size);
void* realloc(void* ptr, size_t new_size);
void free(void* ptr);

#define ALLOC_BLOCK_SIZE 0x1000
#define ALLOC_BLOCK_SHIFT 12

extern int __ram_start;
extern int __ram_end;

typedef struct {
    int free;
} __alloc_block;

static __alloc_block* __alloc_blocks;
static int __alloc_blocks_size;

#endif