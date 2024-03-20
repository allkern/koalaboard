#include "rom.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

rom_t* rom_create() {
    return (rom_t*)malloc(sizeof(rom_t));
}

void rom_init(rom_t* rom, const char* path) {
    memset(rom, 0, sizeof(rom_t));

    FILE* file = fopen(path, "rb");

    if (!file) {
        printf("Couldn't open ROM file \"%s\"\n", path);

        exit(1);
    }

    fseek(file, 0, SEEK_END);
    rom->size = ftell(file);
    fseek(file, 0, SEEK_SET);

    rom->buf = malloc(rom->size);

    fread(rom->buf, 1, rom->size, file);
    fclose(file);
}

int rom_query_access_cycles(void* udata) {
    return 0;
}

uint32_t rom_read32(uint32_t addr, void* udata) {
    rom_t* rom = (rom_t*)udata;

    return *(uint32_t*)(&rom->buf[addr]);
}

uint32_t rom_read16(uint32_t addr, void* udata) {
    rom_t* rom = (rom_t*)udata;

    return *(uint16_t*)(&rom->buf[addr]);
}

uint32_t rom_read8(uint32_t addr, void* udata) {
    rom_t* rom = (rom_t*)udata;

    return *(uint8_t*)(&rom->buf[addr]);
}

void rom_write32(uint32_t addr, uint32_t data, void* udata) {
    rom_t* rom = (rom_t*)udata;

    *(uint32_t*)(&rom->buf[addr]) = data;
}

void rom_write16(uint32_t addr, uint32_t data, void* udata) {
    rom_t* rom = (rom_t*)udata;

    *(uint16_t*)(&rom->buf[addr]) = data;
}

void rom_write8(uint32_t addr, uint32_t data, void* udata) {
    rom_t* rom = (rom_t*)udata;

    *(uint8_t*)(&rom->buf[addr]) = data;
}

void rom_destroy(rom_t* rom) {
    free(rom->buf);
    free(rom);
}

void rom_init_bus_device(rom_t* rom, bus_device_t* dev) {
    dev->query_access_cycles = rom_query_access_cycles;
    dev->read32 = rom_read32;
    dev->read16 = rom_read16;
    dev->read8 = rom_read8;
    dev->write32 = rom_write32;
    dev->write16 = rom_write16;
    dev->write8 = rom_write8;
    dev->udata = rom;
}