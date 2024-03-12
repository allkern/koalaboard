#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nvs.h"

void cmd_read_sector(nvs_t* nvs, int index) {
    fseek(nvs->disk[index].file, nvs->disk[index].rw_lba * NVS_SECTOR_SIZE, SEEK_SET);
    fread(nvs->disk[index].rw_buf, 1, NVS_SECTOR_SIZE, nvs->disk[index].file);

    nvs->disk[index].status &= ~NVS_STAT_BUSY;
    nvs->disk[index].status |= NVS_STAT_IODREQ;

    nvs->disk[index].rw_buf_idx = 0;
    nvs->disk[index].rw_lba++;
}

void cmd_ident(nvs_t* nvs, int index) {
    nvs->disk[index].status &= ~NVS_STAT_BUSY;
    nvs->disk[index].status |= NVS_STAT_IODREQ;
    nvs->disk[index].rw_buf_idx = 0;

    memcpy(nvs->disk[index].rw_buf, &nvs->disk[index].id, sizeof(nvs_id));
}

void cmd_write_sector(nvs_t* nvs, int index) {
    // printf("NVS: Writing sector at %08x (sector %u)\n",
    //     nvs->disk[index].rw_lba * NVS_SECTOR_SIZE,
    //     nvs->disk[index].rw_lba
    // );

    fseek(nvs->disk[index].file, nvs->disk[index].rw_lba * NVS_SECTOR_SIZE, SEEK_SET);
    fwrite(nvs->disk[index].rw_buf, 1, NVS_SECTOR_SIZE, nvs->disk[index].file);

    nvs->disk[index].status &= ~(NVS_STAT_BUSY | NVS_STAT_IODREQ);

    nvs->disk[index].rw_buf_idx = 0;
    nvs->disk[index].rw_lba++;
}

uint32_t iob_read_data(nvs_t* nvs, int index) {
    uint32_t data = *(uint32_t*)&nvs->disk[index].rw_buf[nvs->disk[index].rw_buf_idx];

    nvs->disk[index].rw_buf_idx += 4;

    if (nvs->disk[index].rw_buf_idx >= NVS_SECTOR_SIZE)
        nvs->disk[index].status &= ~NVS_STAT_IODREQ;

    nvs->disk[index].rw_buf_idx &= NVS_SECTOR_SIZE - 1;

    return data;
}

void iob_write_data(nvs_t* nvs, int index, uint32_t data) {
    *(uint32_t*)&nvs->disk[index].rw_buf[nvs->disk[index].rw_buf_idx] = data;

    nvs->disk[index].rw_buf_idx += 4;

    if (nvs->disk[index].rw_buf_idx >= NVS_SECTOR_SIZE) {
        if (nvs->disk[index].cmd == NVS_CMD_WRITE_SECTOR)
            cmd_write_sector(nvs, index);
    
        nvs->disk[index].status &= ~NVS_STAT_IODREQ;
    }
    
    nvs->disk[index].rw_buf_idx &= NVS_SECTOR_SIZE - 1;
}

nvs_t* nvs_create() {
    return (nvs_t*)malloc(sizeof(nvs_t));
}

void nvs_init(nvs_t* nvs) {
    for (int i = 0; i < 4; i++)
        memset(&nvs->disk[i], 0, sizeof(nvs_disk));
}

void nvs_open(nvs_t* nvs, int index, const char* path) {
    nvs->disk[index].rw_buf = malloc(NVS_SECTOR_SIZE);
    nvs->disk[index].sector_size = NVS_SECTOR_SIZE;
    nvs->disk[index].status = 0;
    nvs->disk[index].cmd = 0;
    nvs->disk[index].rw_buf_idx = 0;
    nvs->disk[index].file = fopen(path, "r+b");

    if (!nvs->disk[index].file) {
        printf("Could not open disk image \'%s\'\n", path);

        exit(1);
    }

    fseek(nvs->disk[index].file, 0, SEEK_END);

    nvs->disk[index].size = ftell(nvs->disk[index].file);

    fseek(nvs->disk[index].file, 0, SEEK_SET);

    if (nvs->disk[index].size & (NVS_SECTOR_SIZE - 1))
        printf("Disk image size not a multiple of sector size\n");

    // Initialize nvs_id structure
    nvs->disk[index].id.type = 0x0001;
    strncpy(nvs->disk[index].id.model, "Virtual Disk Controller", 128);
    strncpy(nvs->disk[index].id.manufacturer, "NVS", 32);
    nvs->disk[index].id.sector_count = nvs->disk[index].size >> __builtin_ffs(NVS_SECTOR_SIZE);
    nvs->disk[index].id.sector_size = NVS_SECTOR_SIZE;

    // Set PROBE bit (drive is connected)
    nvs->disk[index].status |= NVS_STAT_PROBE;
}

void nvs_close(nvs_t* nvs, int index) {
    if (nvs->disk[index].file)
        fclose(nvs->disk[index].file);

    nvs->disk[index].file = NULL;
}

int nvs_query_access_cycles(void* udata) {
    return 1;
}

uint32_t nvs_read32(uint32_t addr, void* udata) {
    nvs_t* nvs = (nvs_t*)udata;

    int index = (addr >> 4) & 3;

    switch (addr & 0xf) {
        case NVS_REG_DATA: {
            return iob_read_data(nvs, index);
        } break;

        case NVS_REG_LBA: {
            return nvs->disk[index].rw_lba;
        } break;

        case NVS_REG_CMD: {
            return nvs->disk[index].cmd;
        } break;

        case NVS_REG_STAT: {
            return nvs->disk[index].status;
        } break;
    }
}

uint32_t nvs_read16(uint32_t addr, void* udata) {
    nvs_t* nvs = (nvs_t*)udata;
}

uint32_t nvs_read8(uint32_t addr, void* udata) {
    nvs_t* nvs = (nvs_t*)udata;
}

void nvs_write32(uint32_t addr, uint32_t data, void* udata) {
    nvs_t* nvs = (nvs_t*)udata;

    int index = (addr >> 4) & 3;

    switch (addr & 0xf) {
        case NVS_REG_DATA: {
            iob_write_data(nvs, index, data);
        } break;

        case NVS_REG_LBA: {
            nvs->disk[index].rw_lba = data;
        } break;

        case NVS_REG_CMD: {
            nvs->disk[index].cmd = data;

            if (data == NVS_CMD_READ_SECTOR) {
                cmd_read_sector(nvs, index);
            } else if (data == NVS_CMD_WRITE_SECTOR) {
                nvs->disk[index].status |= NVS_STAT_IODREQ;
            } else if (data == NVS_CMD_IDENT) {
                cmd_ident(nvs, index);
            }
        } break;

        case NVS_REG_STAT: {
            // Nothing
        } break;
    }
}

void nvs_write16(uint32_t addr, uint32_t data, void* udata) {
    nvs_t* nvs = (nvs_t*)udata;
}

void nvs_write8(uint32_t addr, uint32_t data, void* udata) {
    nvs_t* nvs = (nvs_t*)udata;
}

void nvs_destroy(nvs_t* nvs) {
    for (int i = 0; i < 4; i++) {
        nvs_close(nvs, i);

        free(nvs->disk[i].rw_buf);
    }
    
    free(nvs);
}

void nvs_init_bus_device(nvs_t* nvs, bus_device_t* bdev) {
    bdev->query_access_cycles = nvs_query_access_cycles;
    bdev->read32 = nvs_read32;
    bdev->read16 = nvs_read16;
    bdev->read8 = nvs_read8;
    bdev->write32 = nvs_write32;
    bdev->write16 = nvs_write16;
    bdev->write8 = nvs_write8;
    bdev->udata = nvs;
}