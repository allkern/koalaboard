#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <tgmath.h>
#include <time.h>

typedef struct __attribute__((__packed__)) {
    uint8_t reserved0[0x194];
    uint64_t time_stamp; // Time Stamp of when volume has changed.
    uint64_t data_size; // Size of Data Area in blocks
    uint64_t index_size; // Size of Index Area in bytes
    uint8_t magic[3]; // signature ‘SFS’ (0x534653)
    uint8_t version; // SFS version (0x10 = 1.0, 0x1A = 1.10)
    uint64_t total_blocks; // Total number of blocks in volume (including reserved area)
    uint32_t rsvd_blocks; // Number of reserved blocks
    uint8_t block_size; // log(x+7) of block size (x = 2 = 512)
    uint8_t crc; // Zero sum of bytes above
    uint8_t reserved1[0x42];
} sfs_super;

static uint32_t g_config_block_size = 0;
static uint32_t g_config_image_size = 0;

int main(void) {
    g_config_block_size = 0x1000; // 4 KiB
    g_config_image_size = 0x4000000; // 64 MiB

    FILE* file = fopen("disk.bin", "wb");

    if (!file) {
        printf("Could not open disk image \'%s\'\n", "disk.bin");

        exit(1);
    }

    uint8_t dummy = 0xff;

    // Make a file the size of the desired image size
    fseek(file, g_config_image_size, SEEK_SET);
    fwrite(&dummy, 1, 1, file);
    fseek(file, 0, SEEK_SET);

    int super_blocks = 1;
    int resv_blocks = 0;
    int total_blocks = g_config_image_size / g_config_block_size;
    int data_blocks = total_blocks - super_blocks - resv_blocks;

    uint64_t timestamp = ((uint64_t)time(NULL)) << 16;

    sfs_super* super = malloc(sizeof(sfs_super));

    memset(super, 0, sizeof(sfs_super));

    super->time_stamp   = ((uint64_t)time(NULL)) << 16;
    super->data_size    = 0;
    super->index_size   = 0;
    super->magic[0]     = 'S';
    super->magic[1]     = 'F';
    super->magic[2]     = 'S';
    super->version      = 0x10;
    super->total_blocks = g_config_image_size / g_config_block_size;
    super->rsvd_blocks  = super_blocks;
    super->block_size   = __builtin_ffs(g_config_block_size) - 8;
    super->crc          = 0;

    fwrite(super, sizeof(sfs_super), 1, file);

    return 0;
}