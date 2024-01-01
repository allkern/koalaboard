#include "nvs.h"

#include "util/mmio.h"
#include "libc/stdio.h"

void nvs_zero_sector(uint32_t lba) {
    mmio_write_32(NVS_REG_LBA, lba);
    mmio_write_32(NVS_REG_CMD, NVS_CMD_WRITE_SECTOR);

    while (mmio_read_32(NVS_REG_STAT) & NVS_STAT_IODREQ)
        mmio_write_32(NVS_REG_DATA, 0);
}

void nvs_read_sector(void* buf, uint32_t lba) {
    uint32_t* ptr = (uint32_t*)buf;

    mmio_write_32(NVS_REG_LBA, lba);
    mmio_write_32(NVS_REG_CMD, NVS_CMD_READ_SECTOR);

    while (mmio_read_32(NVS_REG_STAT) & NVS_STAT_IODREQ)
        *ptr++ = mmio_read_32(NVS_REG_DATA);
}

void nvs_write_sector(void* buf, uint32_t lba) {
    uint32_t* ptr = (uint32_t*)buf;

    mmio_write_32(NVS_REG_LBA, lba);
    mmio_write_32(NVS_REG_CMD, NVS_CMD_WRITE_SECTOR);

    while (mmio_read_32(NVS_REG_STAT) & NVS_STAT_IODREQ)
        mmio_write_32(NVS_REG_DATA, *ptr++);
}