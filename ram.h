#ifndef RAM_H
#define RAM_H

#include <stdint.h>
#include <stdlib.h>

#include "bus.h"

typedef struct {
    uint8_t* buf;
} ram_t;

ram_t* ram_create();
void ram_init(ram_t*, size_t);
int ram_query_access_cycles(void*);
uint32_t ram_read32(uint32_t, void*);
uint32_t ram_read16(uint32_t, void*);
uint32_t ram_read8(uint32_t, void*);
void ram_write32(uint32_t, uint32_t, void*);
void ram_write16(uint32_t, uint32_t, void*);
void ram_write8(uint32_t, uint32_t, void*);
void ram_destroy(ram_t*);
void ram_init_bus_device(ram_t*, bus_device_t*);

#endif