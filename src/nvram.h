#ifndef NVRAM_H
#define NVRAM_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "bus.h"

typedef struct {
    size_t size;
    uint8_t* buf;
    const char* path;
} nvram_t;

nvram_t* nvram_create();
void nvram_init(nvram_t*, size_t, const char*);
void nvram_load(nvram_t*, const char*);
void nvram_flush(nvram_t*, const char*);
int nvram_query_access_cycles(void*);
uint32_t nvram_read32(uint32_t, void*);
uint32_t nvram_read16(uint32_t, void*);
uint32_t nvram_read8(uint32_t, void*);
void nvram_write32(uint32_t, uint32_t, void*);
void nvram_write16(uint32_t, uint32_t, void*);
void nvram_write8(uint32_t, uint32_t, void*);
void nvram_destroy(nvram_t*);
void nvram_init_bus_device(nvram_t*, bus_device_t*);

#endif