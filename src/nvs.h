#ifndef NVS_H
#define NVS_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "bus.h"

#define NVS_SECTOR_SIZE 0x200

enum {
    NVS_REG_DATA = 0x00,
    NVS_REG_LBA  = 0x04,
    NVS_REG_CMD  = 0x08,
    NVS_REG_STAT = 0x0c
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
    NVS_STAT_PROBE  = 0x40000000,
    NVS_STAT_BUSY   = 0x80000000
};

typedef struct {
    uint32_t type;
    char model[128];
    char manufacturer[32];
    uint32_t sector_count;
    uint32_t sector_size;
} nvs_id;

typedef struct {
    FILE* file;

    uint32_t sector_size;
    uint32_t size;
    uint32_t status;
    uint32_t cmd;

    // RW operation
    uint32_t rw_lba;
    uint32_t rw_count;
    uint8_t* rw_buf;
    uint32_t rw_bytes_remaining;
    uint32_t rw_buf_idx;

    // ID
    nvs_id id;
} nvs_disk;

typedef struct {
    nvs_disk disk[4];
} nvs_t;

nvs_t* nvs_create();
void nvs_init(nvs_t*);
void nvs_open(nvs_t*, int, const char*);
void nvs_close(nvs_t*, int);
int nvs_query_access_cycles(void*);
uint32_t nvs_read32(uint32_t, void*);
uint32_t nvs_read16(uint32_t, void*);
uint32_t nvs_read8(uint32_t, void*);
void nvs_write32(uint32_t, uint32_t, void*);
void nvs_write16(uint32_t, uint32_t, void*);
void nvs_write8(uint32_t, uint32_t, void*);
void nvs_destroy(nvs_t*);
void nvs_init_bus_device(nvs_t*, bus_device_t*);

#endif