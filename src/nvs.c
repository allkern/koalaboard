#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "nvs.h"

void cmd_read_sector(nvs_t* nvs) {
    fseek(nvs->file, nvs->rw_lba * NVS_SECTOR_SIZE, SEEK_SET);
    fread(nvs->rw_buf, 1, NVS_SECTOR_SIZE, nvs->file);

    nvs->status &= ~NVS_STAT_BUSY;
    nvs->status |= NVS_STAT_IODREQ;

    nvs->rw_buf_idx = 0;
    nvs->rw_lba++;
}

void cmd_write_sector(nvs_t* nvs) {
    fseek(nvs->file, nvs->rw_lba * NVS_SECTOR_SIZE, SEEK_SET);
    fwrite(nvs->rw_buf, 1, NVS_SECTOR_SIZE, nvs->file);

    nvs->status &= ~(NVS_STAT_BUSY | NVS_STAT_IODREQ);

    nvs->rw_buf_idx = 0;
    nvs->rw_lba++;
}

uint32_t iob_read_data(nvs_t* nvs) {
    uint32_t data = *(uint32_t*)&nvs->rw_buf[nvs->rw_buf_idx];

    nvs->rw_buf_idx += 4;

    if (nvs->rw_buf_idx >= NVS_SECTOR_SIZE)
        nvs->status &= ~NVS_STAT_IODREQ;

    nvs->rw_buf_idx &= NVS_SECTOR_SIZE - 1;

    return data;
}

void iob_write_data(nvs_t* nvs, uint32_t data) {
    *(uint32_t*)&nvs->rw_buf[nvs->rw_buf_idx] = data;

    nvs->rw_buf_idx += 4;

    if (nvs->rw_buf_idx >= NVS_SECTOR_SIZE) {
        if (nvs->cmd == NVS_CMD_WRITE_SECTOR)
            cmd_write_sector(nvs);
    
        nvs->status &= ~NVS_STAT_IODREQ;
    }
    
    nvs->rw_buf_idx &= NVS_SECTOR_SIZE - 1;
}

nvs_t* nvs_create() {
    return (nvs_t*)malloc(sizeof(nvs_t));
}

void nvs_init(nvs_t* nvs) {
    memset(nvs, 0, sizeof(nvs));

    nvs->file = NULL;
    nvs->sector_size = NVS_SECTOR_SIZE;
    nvs->rw_buf = malloc(NVS_SECTOR_SIZE);
}

void nvs_open(nvs_t* nvs, const char* path) {
    nvs->file = fopen(path, "rwb");

    if (!nvs->file) {
        printf("Could not open disk image \'%s\'\n", path);

        exit(1);
    }

    fseek(nvs->file, 0, SEEK_END);

    nvs->size = ftell(nvs->file);

    fseek(nvs->file, 0, SEEK_SET);

    if (nvs->size & (NVS_SECTOR_SIZE - 1))
        printf("Disk image size not a multiple of sector size\n");
}

void nvs_close(nvs_t* nvs) {
    if (nvs->file)
        fclose(nvs->file);

    nvs->file = NULL;
}

int nvs_query_access_cycles(void* udata) {
    return 1;
}

uint32_t nvs_read32(uint32_t addr, void* udata) {
    nvs_t* nvs = (nvs_t*)udata;

    switch (addr) {
        case NVS_REG_DATA: {
            return iob_read_data(nvs);
        } break;

        case NVS_REG_LBA: {
            return nvs->rw_lba;
        } break;

        case NVS_REG_CMD: {
            return nvs->cmd;
        } break;

        case NVS_REG_STAT: {
            return nvs->status;
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

    switch (addr) {
        case NVS_REG_DATA: {
            iob_write_data(nvs, data);
        } break;

        case NVS_REG_LBA: {
            nvs->rw_lba = data;
        } break;

        case NVS_REG_CMD: {
            nvs->cmd = data;

            if (data == NVS_CMD_READ_SECTOR)
                cmd_read_sector(nvs);
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
    nvs_close(nvs);

    free(nvs->rw_buf);
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