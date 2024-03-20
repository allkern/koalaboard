#include "nvram.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

nvram_t* nvram_create() {
    return (nvram_t*)malloc(sizeof(nvram_t));
}

void nvram_init(nvram_t* nvram, size_t size, const char* path) {
    memset(nvram, 0, sizeof(nvram_t));

    nvram->buf = malloc(size);
    nvram->path = path;
    nvram->size = size;

    FILE* file = fopen(path, "rb");

    if (!file)
        return;

    fread(nvram->buf, 1, size, file);
    fclose(file);
}

int nvram_query_access_cycles(void* udata) {
    return 0;
}

uint32_t nvram_read32(uint32_t addr, void* udata) {
    nvram_t* nvram = (nvram_t*)udata;

    return *(uint32_t*)(&nvram->buf[addr]);
}

uint32_t nvram_read16(uint32_t addr, void* udata) {
    nvram_t* nvram = (nvram_t*)udata;

    return *(uint16_t*)(&nvram->buf[addr]);
}

uint32_t nvram_read8(uint32_t addr, void* udata) {
    nvram_t* nvram = (nvram_t*)udata;

    return *(uint8_t*)(&nvram->buf[addr]);
}

void nvram_write32(uint32_t addr, uint32_t data, void* udata) {
    nvram_t* nvram = (nvram_t*)udata;

    *(uint32_t*)(&nvram->buf[addr]) = data;
}

void nvram_write16(uint32_t addr, uint32_t data, void* udata) {
    nvram_t* nvram = (nvram_t*)udata;

    *(uint16_t*)(&nvram->buf[addr]) = data;
}

void nvram_write8(uint32_t addr, uint32_t data, void* udata) {
    nvram_t* nvram = (nvram_t*)udata;

    *(uint8_t*)(&nvram->buf[addr]) = data;
}

void nvram_destroy(nvram_t* nvram) {
    FILE* file = fopen(nvram->path, "wb");

    if (!file) {
        printf("Failed to flush NVRAM contents to \'%s\'\n", nvram->path);

        return;
    }
    
    fseek(file, 0, SEEK_SET);
    fwrite(nvram->buf, 1, nvram->size, file);
    fclose(file);

    free(nvram->buf);
    free(nvram);
}

void nvram_init_bus_device(nvram_t* nvram, bus_device_t* dev) {
    dev->query_access_cycles = nvram_query_access_cycles;
    dev->read32 = nvram_read32;
    dev->read16 = nvram_read16;
    dev->read8 = nvram_read8;
    dev->write32 = nvram_write32;
    dev->write16 = nvram_write16;
    dev->write8 = nvram_write8;
    dev->udata = nvram;
}