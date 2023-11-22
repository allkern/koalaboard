#include "ram.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

ram_t* ram_create() {
    return (ram_t*)malloc(sizeof(ram_t));
}

void ram_init(ram_t* ram, size_t size) {
    memset(ram, 0, sizeof(ram_t));

    ram->buf = malloc(size);
}

int ram_query_access_cycles(void* udata) {
    return 0;
}

#include <stdio.h>

uint32_t ram_read32(uint32_t addr, void* udata) {
    ram_t* ram = (ram_t*)udata;

    printf("RAM read32 [%08x] = %08x\n", addr, *(uint32_t*)(&ram->buf[addr]));

    return *(uint32_t*)(&ram->buf[addr]);
}

uint32_t ram_read16(uint32_t addr, void* udata) {
    ram_t* ram = (ram_t*)udata;

    return *(uint16_t*)(&ram->buf[addr]);
}

uint32_t ram_read8(uint32_t addr, void* udata) {
    ram_t* ram = (ram_t*)udata;

    return *(uint8_t*)(&ram->buf[addr]);
}

void ram_write32(uint32_t addr, uint32_t data, void* udata) {
    ram_t* ram = (ram_t*)udata;

    printf("RAM write32 [%08x] = %08x\n", addr, data);

    *(uint32_t*)(&ram->buf[addr]) = data;
}

void ram_write16(uint32_t addr, uint32_t data, void* udata) {
    ram_t* ram = (ram_t*)udata;

    *(uint16_t*)(&ram->buf[addr]) = data;
}

void ram_write8(uint32_t addr, uint32_t data, void* udata) {
    ram_t* ram = (ram_t*)udata;

    *(uint8_t*)(&ram->buf[addr]) = data;
}

void ram_destroy(ram_t* ram) {
    free(ram->buf);
    free(ram);
}

void ram_init_bus_device(ram_t* ram, bus_device_t* dev) {
    dev->query_access_cycles = ram_query_access_cycles;
    dev->read32 = ram_read32;
    dev->read16 = ram_read16;
    dev->read8 = ram_read8;
    dev->write32 = ram_write32;
    dev->write16 = ram_write16;
    dev->write8 = ram_write8;
    dev->udata = ram;
}