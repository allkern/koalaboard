#ifndef ROM_H
#define ROM_H

#include <stdint.h>
#include <stdlib.h>

#include "bus.h"

typedef struct {
    uint8_t* buf;
    int size;
} rom_t;

rom_t* rom_create();
void rom_init(rom_t*, const char*);
int rom_query_access_cycles(void*);
uint32_t rom_read32(uint32_t, void*);
uint32_t rom_read16(uint32_t, void*);
uint32_t rom_read8(uint32_t, void*);
void rom_write32(uint32_t, uint32_t, void*);
void rom_write16(uint32_t, uint32_t, void*);
void rom_write8(uint32_t, uint32_t, void*);
void rom_destroy(rom_t*);
void rom_init_bus_device(rom_t*, bus_device_t*);

#endif