#ifndef FS_H
#define FS_H

#include "libc/stdint.h"
#include "libc/stddef.h"
#include "libc/string.h"
#include "libc/stdio.h"

#include "hw/nvs.h"
#include "hw/vmc.h"

typedef struct {
    char mbr_shell[436];
    char mbr_udid[10];
    char mbr_ptable[64];
    char mbr_signature[2];
    char magic[4];
    uint32_t version;
    uint32_t hdr_size;
    uint32_t resv_area_size;
    uint32_t user_area_size;
    uint32_t dtab_area_size;
    uint32_t total_disk_size;
} fs_hdr;

typedef int directory_index;

#define FS_ROOT 0
#define FS_META_ENTRY_SIZE 1024
#define FS_META_ENTRIES_PER_SECTOR (NVS_SECTOR_SIZE / FS_META_ENTRY_SIZE)
#define FS_META_MAX_NAME (FS_META_ENTRY_SIZE - 24)

typedef union {
    uint64_t u64;
    uint32_t u32[2];
} fs_timestamp;

enum {
    MT_END = 0,
    MT_FREE = 1,
    MT_FILE = 2,
    MT_DIRECTORY = 4
};

typedef struct {
    uint32_t type;
    uint32_t parent;
    uint32_t size;
    uint32_t lba;
    fs_timestamp ts;
    char name[FS_META_ENTRY_SIZE-24];
} fs_meta;

void fs_format(void* buf);
void fs_create_file(const char* name, void* data, size_t size, int parent);
int fs_create_directory(const char* name, int parent);
int fs_search_directory(const char* path, int parent);
void fs_list_directory(const char* path);

#endif