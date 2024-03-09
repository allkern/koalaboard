#ifndef NVS_H
#define NVS_H

#include "libc/stdint.h"

#define NVS_SECTOR_SIZE 0x1000
#define NVS_PHYS_BASE 0x9fa00000

enum {
    NVS_REG_DATA = NVS_PHYS_BASE + 0x00,
    NVS_REG_LBA  = NVS_PHYS_BASE + 0x04,
    NVS_REG_CMD  = NVS_PHYS_BASE + 0x08,
    NVS_REG_STAT = NVS_PHYS_BASE + 0x0c
};

enum {
    NVS_CMD_NOP           = 0x00,
    NVS_CMD_READ_SECTOR   = 0x01,
    NVS_CMD_READ_SECTORS  = 0x11,
    NVS_CMD_WRITE_SECTOR  = 0x02,
    NVS_CMD_WRITE_SECTORS = 0x12,
    NVS_CMD_IDENT         = 0x03
};

enum {
    NVS_STAT_IODREQ = 0x00000001,
    NVS_STAT_BUSY   = 0x80000000
};

unsigned int nvs_get_status();
void nvs_read_sector(void* buf, uint32_t lba);
void nvs_write_sector(void* buf, uint32_t lba);
void nvs_zero_sector(uint32_t lba);

#endif