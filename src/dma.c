#include "dma.h"
#include "log.h"

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

psx_dma_t* psx_dma_create() {
    return (psx_dma_t*)malloc(sizeof(psx_dma_t));
}

const uint32_t g_psx_dma_ctrl_hw_1_table[] = {
    0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000,
    0x00000002
};

const uint32_t g_psx_dma_ctrl_hw_0_table[] = {
    0x71770703, 0x71770703, 0x71770703,
    0x71770703, 0x71770703, 0x71770703,
    0x50000002
};

const psx_dma_do_fn_t g_psx_dma_do_table[] = {
    NULL,
    NULL,
    psx_dma_do_gpu,
    NULL,
    NULL,
    NULL,
    psx_dma_do_otc
};

#define CR(c, r) *((&dma->mdec_in.madr) + (c * 3) + r)

void psx_dma_init(psx_dma_t* dma, bus_t* bus, psx_ic_t* ic) {
    memset(dma, 0, sizeof(psx_dma_t));

    dma->io_base = PSX_DMAR_BEGIN;
    dma->io_size = PSX_DMAR_SIZE;

    dma->bus = bus;
    dma->ic = ic;

    dma->dpcr = 0x07654321;
}

uint32_t psx_dma_read32(psx_dma_t* dma, uint32_t offset) {
    if (offset < 0x70) {
        int channel = (offset >> 4) & 0x7;
        int reg = (offset >> 2) & 0x3;
        uint32_t cr = CR(channel, reg);

        if (reg == 2) {
            cr |= g_psx_dma_ctrl_hw_1_table[channel];
            cr &= g_psx_dma_ctrl_hw_0_table[channel];
        }

        if (channel == 4) {
            log_set_quiet(0);
            log_error("DMA channel %u register %u (%08x) read %08x", channel, reg, PSX_DMAR_BEGIN + offset, cr);
            log_set_quiet(1);
        }

        log_error("DMA channel %u register %u (%08x) read %08x", channel, reg, PSX_DMAR_BEGIN + offset, cr);
        
        return cr;
    } else {
        switch (offset) {
            case 0x70: log_error("DMA control read %08x", dma->dpcr); return dma->dpcr;
            case 0x74: log_error("DMA irqc    read %08x", dma->dicr); return dma->dicr;

            default: {
                log_error("Unhandled 32-bit DMA read at offset %08x", offset);

                return 0x0;
            }
        }
    }
}

uint16_t psx_dma_read16(psx_dma_t* dma, uint32_t offset) {
    switch (offset) {
        case 0x70: return dma->dpcr;
        case 0x74: return dma->dicr;

        default: {
            log_error("Unhandled 16-bit DMA read at offset %08x", offset);

            return 0x0;
        }
    }
}

uint8_t psx_dma_read8(psx_dma_t* dma, uint32_t offset) {
    switch (offset) {
        case 0x70: return dma->dpcr;
        case 0x74: return dma->dicr;

        default: {
            log_error("Unhandled 8-bit DMA read at offset %08x", offset);

            return 0x0;
        }
    }
}

void psx_dma_write32(psx_dma_t* dma, uint32_t offset, uint32_t value) {
    if (offset < 0x70) {
        int channel = (offset >> 4) & 0x7;
        int reg = (offset >> 2) & 0x3;

        CR(channel, reg) = value;

        if (channel == 4) {
            log_set_quiet(0);
            log_fatal("DMA channel %u register %u write (%08x) %08x", channel, reg, PSX_DMAR_BEGIN + offset, value);
            log_set_quiet(1);
        }

        log_error("DMA channel %u register %u write (%08x) %08x", channel, reg, PSX_DMAR_BEGIN + offset, value);

        if (reg == 2)
            g_psx_dma_do_table[channel](dma);
    } else {
        switch (offset) {
            case 0x70: log_error("DMA control write %08x", value); dma->dpcr = value; break;
            case 0x74: {
                uint32_t old_value = value;

                // IRQ signal is read-only
                value &= ~DICR_IRQSI;

                // Reset flags
                dma->dicr &= ~(value & DICR_FLAGS);

                // Write other fields
                uint32_t flags = dma->dicr & DICR_FLAGS;

                dma->dicr &= ~DICR_FLAGS;
                dma->dicr |= value & (~DICR_FLAGS);
                dma->dicr |= flags;
            } break;

            default: {
                log_error("Unhandled 32-bit DMA write at offset %08x (%08x)", offset, value);
            } break;
        }
    }
}

void psx_dma_write16(psx_dma_t* dma, uint32_t offset, uint16_t value) {
    switch (offset) {
        default: {
            log_fatal("Unhandled 16-bit DMA write at offset %08x (%04x)", offset, value);

            //exit(1);
        } break;
    }
}

void psx_dma_write8(psx_dma_t* dma, uint32_t offset, uint8_t value) {
    switch (offset) {
        default: {
            log_fatal("Unhandled 8-bit DMA write at offset %08x (%02x)", offset, value);

            //exit(1);
        } break;
    }
}

const char* g_psx_dma_sync_type_name_table[] = {
    "burst",
    "request",
    "linked",
    "reserved"
};

void psx_dma_do_gpu_linked(psx_dma_t* dma) {
    uint32_t hdr = bus_read32(dma->gpu.madr, dma->bus);
    uint32_t size = hdr >> 24;
    uint32_t addr = dma->gpu.madr;

    while (1) {
        while (size--) {
            addr = (addr + (CHCR_STEP(gpu) ? -4 : 4)) & 0x1ffffc;

            // Get command from linked list
            uint32_t cmd = bus_read32(addr, dma->bus);

            // Write to GP0
            bus_write32(0x1f801810, cmd, dma->bus);

            dma->gpu_irq_delay++;
        }

        addr = hdr & 0xffffff;

        if (addr == 0xffffff) break;

        hdr = bus_read32(addr, dma->bus);
        size = hdr >> 24;
    }
}

void psx_dma_do_gpu_request(psx_dma_t* dma) {
    if (!CHCR_BUSY(gpu))
        return;

    uint32_t size = BCR_SIZE(gpu) * BCR_BCNT(gpu);

    if (CHCR_TDIR(gpu)) {
        for (int i = 0; i < size; i++) {
            uint32_t data = bus_read32(dma->gpu.madr, dma->bus);

            bus_write32(0x1f801810, data, dma->bus);

            dma->gpu.madr += CHCR_STEP(gpu) ? -4 : 4;
        }
    } else {
        for (int i = 0; i < size; i++) {
            uint32_t data = bus_read32(0x1f801810, dma->bus);

            bus_write32(dma->gpu.madr, data, dma->bus);

            dma->gpu.madr += CHCR_STEP(gpu) ? -4 : 4;
        }
    }

    dma->gpu_irq_delay = size;
}

void psx_dma_do_gpu_burst(psx_dma_t* dma) {
    log_fatal("GPU DMA burst sync mode unimplemented");

    exit(1);
}

psx_dma_do_fn_t g_psx_dma_gpu_table[] = {
    psx_dma_do_gpu_burst,
    psx_dma_do_gpu_request,
    psx_dma_do_gpu_linked
};

void psx_dma_do_gpu(psx_dma_t* dma) {
    if (!CHCR_BUSY(gpu))
        return;

    g_psx_dma_gpu_table[CHCR_SYNC(gpu)](dma);

    // Clear BCR and CHCR trigger and busy bits
    dma->gpu.chcr &= ~(CHCR_BUSY_MASK | CHCR_TRIG_MASK);
    dma->gpu.bcr = 0;
}

void psx_dma_do_otc(psx_dma_t* dma) {
    if ((!(dma->dpcr & DPCR_DMA6EN)) || (!CHCR_TRIG(otc)) || (!CHCR_BUSY(otc)))
        return;

    uint32_t size = BCR_SIZE(otc);

    if (!size) size = 0x10000;

    for (int i = size; i > 0; i--) {
        uint32_t addr = (i != 1) ? (dma->otc.madr - 4) : 0xffffff;

        bus_write32(dma->otc.madr, addr & 0xffffff, dma->bus);

        dma->otc.madr -= 4;
    }

    dma->otc_irq_delay = size;

    // Clear BCR and CHCR trigger and busy bits
    dma->otc.chcr = 0;
    //dma->otc.chcr &= ~(CHCR_BUSY_MASK | CHCR_TRIG_MASK);
    dma->otc.bcr = 0;
}

void psx_dma_update(psx_dma_t* dma, int cyc) {
    if (dma->gpu_irq_delay) {
        dma->gpu_irq_delay -= cyc;

        if (!dma->gpu_irq_delay)
            if (dma->dicr & DICR_DMA2EN)
                dma->dicr |= DICR_DMA2FL;
    }

    if (dma->otc_irq_delay) {
        dma->otc_irq_delay -= cyc;

        if (!dma->otc_irq_delay)
            if (dma->dicr & DICR_DMA6EN)
                dma->dicr |= DICR_DMA6FL;
    }

    int prev_irq_signal = (dma->dicr & DICR_IRQSI) != 0;
    int irq_master_en = (dma->dicr & DICR_IRQEN) != 0;
    int irq_on_flags = (dma->dicr & DICR_FLGEN) >> 16;
    int force_irq = (dma->dicr & DICR_FORCE) != 0;
    int irq = (dma->dicr & DICR_FLAGS) >> 24;

    int irq_signal = force_irq || (irq_master_en && ((irq & irq_on_flags) != 0));

    if (irq_signal && !prev_irq_signal)
        psx_ic_irq(dma->ic, IC_DMA);

    dma->dicr &= ~DICR_IRQSI;
    dma->dicr |= irq_signal << 31;
}

void psx_dma_destroy(psx_dma_t* dma) {
    free(dma);
}

uint32_t bus_dma_read32(uint32_t addr, void* udata) {
    psx_dma_t* dma = (psx_dma_t*)udata;

    return psx_dma_read32(dma, addr);
}

uint32_t bus_dma_read16(uint32_t addr, void* udata) {
    psx_dma_t* dma = (psx_dma_t*)udata;

    return psx_dma_read16(dma, addr);
}

uint32_t bus_dma_read8(uint32_t addr, void* udata) {
    psx_dma_t* dma = (psx_dma_t*)udata;

    return psx_dma_read8(dma, addr);
}

void bus_dma_write32(uint32_t addr, uint32_t data, void* udata) {
    psx_dma_t* dma = (psx_dma_t*)udata;
}

void bus_dma_write16(uint32_t addr, uint32_t data, void* udata) {
    psx_dma_t* dma = (psx_dma_t*)udata;
}

void bus_dma_write8(uint32_t addr, uint32_t data, void* udata) {
    psx_dma_t* dma = (psx_dma_t*)udata;
}


#undef CR