#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ic.h"
#include "log.h"

psx_ic_t* psx_ic_create() {
    return (psx_ic_t*)malloc(sizeof(psx_ic_t));
}

void psx_ic_init(psx_ic_t* ic, r3000_t* cpu) {
    memset(ic, 0, sizeof(psx_ic_t));

    ic->io_base = PSX_IC_BEGIN;
    ic->io_size = PSX_IC_SIZE;

    ic->stat = 0x00000000;
    ic->mask = 0x00000000;

    ic->cpu = cpu;
}

uint32_t psx_ic_read32(psx_ic_t* ic, uint32_t offset) {
    switch (offset) {
        case 0x00: return ic->stat;
        case 0x04: return ic->mask;
    }

    log_warn("Unhandled 32-bit IC read at offset %08x", offset);

    return 0x0;
}

uint16_t psx_ic_read16(psx_ic_t* ic, uint32_t offset) {
    switch (offset) {
        case 0x00: return ic->stat;
        case 0x04: return ic->mask;
    }

    log_warn("Unhandled 16-bit IC read at offset %08x", offset);

    return 0x0;
}

uint8_t psx_ic_read8(psx_ic_t* ic, uint32_t offset) {
    log_warn("Unhandled 8-bit IC read at offset %08x", offset);

    return 0x0;
}

void psx_ic_write32(psx_ic_t* ic, uint32_t offset, uint32_t value) {
    switch (offset) {
        case 0x00: ic->stat &= value; break;
        case 0x04: ic->mask = value; break;

        default: {
            log_warn("Unhandled 32-bit IC write at offset %08x (%08x)", offset, value);
        } break;
    }

    // Emulate acknowledge
    if (!(ic->stat & ic->mask)) {
        ic->cpu->cop0_r[COP0_CAUSE] &= ~SR_IM2;
    }
}

void psx_ic_write16(psx_ic_t* ic, uint32_t offset, uint16_t value) {
    switch (offset) {
        case 0x00: ic->stat &= value; break;
        case 0x04: ic->mask = value; break;

        default: {
            log_warn("Unhandled 16-bit IC write at offset %08x (%08x)", offset, value);
        } break;
    }

    // Emulate acknowledge
    if (!(ic->stat & ic->mask)) {
        ic->cpu->cop0_r[COP0_CAUSE] &= ~SR_IM2;
    }
}

void psx_ic_write8(psx_ic_t* ic, uint32_t offset, uint8_t value) {
    log_warn("Unhandled 8-bit IC write at offset %08x (%02x)", offset, value);
}

void psx_ic_irq(psx_ic_t* ic, int id) {
    ic->stat |= id;

    if (ic->mask & ic->stat)
        r3000_set_irq_pending(ic->cpu);
}

void psx_ic_destroy(psx_ic_t* ic) {
    free(ic);
}

uint32_t bus_ic_read32(uint32_t addr, void* udata) {
    psx_ic_t* ic = (psx_ic_t*)udata;

    return psx_ic_read32(ic, addr);
}

uint32_t bus_ic_read16(uint32_t addr, void* udata) {
    psx_ic_t* ic = (psx_ic_t*)udata;

    return psx_ic_read16(ic, addr);
}

uint32_t bus_ic_read8(uint32_t addr, void* udata) {
    psx_ic_t* ic = (psx_ic_t*)udata;

    return psx_ic_read8(ic, addr);
}

void bus_ic_write32(uint32_t addr, uint32_t data, void* udata) {
    psx_ic_t* ic = (psx_ic_t*)udata;

    psx_ic_write32(ic, addr, data);
}

void bus_ic_write16(uint32_t addr, uint16_t data, void* udata) {
    psx_ic_t* ic = (psx_ic_t*)udata;

    psx_ic_write16(ic, addr, data);
}

void bus_ic_write8(uint32_t addr, uint8_t data, void* udata) {
    psx_ic_t* ic = (psx_ic_t*)udata;

    psx_ic_write8(ic, addr, data);
}
