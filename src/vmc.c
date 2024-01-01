#include "vmc.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

vmc_t* vmc_create() {
    return (vmc_t*)malloc(sizeof(vmc_t));
}

void vmc_init(vmc_t* vmc) {
    memset(vmc, 0, sizeof(vmc_t));
}

int vmc_query_access_cycles(void* udata) {
    return 0;
}

uint32_t vmc_read32(uint32_t addr, void* udata) {
    vmc_t* vmc = (vmc_t*)udata;

    switch (addr) {
        case VMC_EXIT: {
            return vmc->exit_code;
        } break;

        case VMC_PUTCHAR: {
            return getchar();
        } break;

        case VMC_TIMEL: {
            return time(NULL) & 0xffffffff;
        } break;

        case VMC_TIMEH: {
            return time(NULL) >> 32;
        } break;
    }

    return 0xbaadf00d;
}

uint32_t vmc_read16(uint32_t addr, void* udata) {
    vmc_t* vmc = (vmc_t*)udata;

    return 0xf00d;
}

uint32_t vmc_read8(uint32_t addr, void* udata) {
    vmc_t* vmc = (vmc_t*)udata;

    return 0x0d;
}

void vmc_write32(uint32_t addr, uint32_t data, void* udata) {
    vmc_t* vmc = (vmc_t*)udata;

    switch (addr) {
        case VMC_EXIT: {
            vmc->exit_requested = 1;
            vmc->exit_code = data;
        } break;

        case VMC_PUTCHAR: {
            putchar(data);
        } break;

        case VMC_TIMEL: case VMC_TIMEH:
            break;
    }
}

void vmc_write16(uint32_t addr, uint32_t data, void* udata) {
    vmc_t* vmc = (vmc_t*)udata;

    switch (addr) {
        case VMC_EXIT: {
            vmc->exit_requested = 1;
            vmc->exit_code = data;
        } break;

        case VMC_PUTCHAR: {
            putchar(data);
        } break;

        case VMC_TIMEL: case VMC_TIMEH:
            break;
    }
}

void vmc_write8(uint32_t addr, uint32_t data, void* udata) {
    vmc_t* vmc = (vmc_t*)udata;

    switch (addr) {
        case VMC_EXIT: {
            vmc->exit_requested = 1;
            vmc->exit_code = data;
        } break;

        case VMC_PUTCHAR: {
            putchar(data);
        } break;

        case VMC_TIMEL: case VMC_TIMEH:
            break;
    }
}

void vmc_destroy(vmc_t* vmc) {
    free(vmc);
}

void vmc_init_bus_device(vmc_t* vmc, bus_device_t* dev) {
    dev->query_access_cycles = vmc_query_access_cycles;
    dev->read32 = vmc_read32;
    dev->read16 = vmc_read16;
    dev->read8 = vmc_read8;
    dev->write32 = vmc_write32;
    dev->write16 = vmc_write16;
    dev->write8 = vmc_write8;
    dev->udata = vmc;
}