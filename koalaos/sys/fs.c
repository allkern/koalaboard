#include "fs.h"

fs_hdr* g_hdr = NULL;
int g_meta_entry_count = 0;

char sector_buf[NVS_SECTOR_SIZE];

void fs_format(void* buf) {
    g_hdr = (fs_hdr*)buf;

    memset(buf, 0, NVS_SECTOR_SIZE);

    // Zero all sectors (clear disk)
    for (int i = 0; i < (0x4000000 >> 12); i++)
        nvs_zero_sector(i);

    g_hdr->mbr_signature[0] = 0xaa;
    g_hdr->mbr_signature[1] = 0x55;

    strncpy(g_hdr->magic, "KFS", 4);

    g_hdr->version = 1;
    g_hdr->hdr_size = sizeof(fs_hdr);
    g_hdr->resv_area_size = 0;
    g_hdr->user_area_size = 0;
    g_hdr->dtab_area_size = 0;
    g_hdr->total_disk_size = 0x4000000 >> 12;

    nvs_write_sector(buf, 0);
}

int search_free_meta_entry() {
    int entry = 1;
    int lba = g_hdr->total_disk_size - 1;

    while (1) {
        nvs_read_sector(sector_buf, lba--);

        for (int i = 0; i < FS_META_ENTRIES_PER_SECTOR; i++) {
            fs_meta* meta = (fs_meta*)&sector_buf[i * FS_META_ENTRY_SIZE];

            if (meta->type == MT_FREE || meta->type == MT_END)
                return entry;

            ++entry;
        }
    }
}

void write_meta_entry(int entry, fs_meta* data) {
    // Physical entry location is entry number - 1
    --entry;

    int lba = g_hdr->total_disk_size - 1 - (entry / FS_META_ENTRIES_PER_SECTOR);

    nvs_read_sector(sector_buf, lba);

    fs_meta* dst = (fs_meta*)&sector_buf[(entry & 3) * FS_META_ENTRY_SIZE];

    memcpy(dst, data, sizeof(fs_meta));

    nvs_write_sector(sector_buf, lba);
}

void read_meta_entry(int entry, fs_meta* data) {
    --entry;

    int lba = g_hdr->total_disk_size - 1 - (entry / FS_META_ENTRIES_PER_SECTOR);

    nvs_read_sector(sector_buf, lba);

    fs_meta* src = (fs_meta*)&sector_buf[(entry & 3) * FS_META_ENTRY_SIZE];

    memcpy(data, src, sizeof(fs_meta));
}

void fs_create_file(const char* name, void* data, size_t size, int parent) {
    size += sizeof(fs_meta);

    vmc_time_t time = vmc_get_time();

    fs_timestamp ts;

    ts.u32[0] = time.u32[0];
    ts.u32[1] = time.u32[1];

    fs_meta meta;

    strncpy(meta.name, name, FS_META_MAX_NAME);

    meta.parent = parent;
    meta.size = (size >> 12) + 1;
    meta.ts = ts;
    meta.type = MT_FILE;
    meta.lba = 0;

    uint32_t lba = g_hdr->resv_area_size + 1;

    memcpy(sector_buf, &meta, FS_META_ENTRY_SIZE);

    nvs_write_sector(sector_buf, lba);

    // int sector_space = NVS_SECTOR_SIZE;
    // int sector_off = 0;

    // size -= sizeof(fs_meta);
    // sector_space -= sizeof(fs_meta);

    // memcpy(&sector_buf[sector_off], &meta, FS_META_ENTRY_SIZE);

    // sector_off += sizeof(fs_meta);

    // while (size) {
    //     if (size >= sector_space) {
    //         size -= sector_space;

    //         memcpy(&sector_buf[sector_off], data, )
    //     }
    // }
    // memcpy()
}

int fs_create_directory(const char* name, int parent) {
    vmc_time_t time = vmc_get_time();

    fs_timestamp ts;

    ts.u32[0] = time.u32[0];
    ts.u32[1] = time.u32[1];

    fs_meta meta;

    strncpy(meta.name, name, FS_META_MAX_NAME);

    meta.parent = parent;
    meta.size = 0;
    meta.ts = ts;
    meta.type = MT_DIRECTORY;
    meta.lba = 0;

    int entry = search_free_meta_entry();

    write_meta_entry(entry, &meta);
}

int fs_search_directory(const char* path, int parent) {
    char* end = strchr(path, '\0');
    char* dir = strchr(path, '/');

    return 0;
}

void fs_list_directory(const char* path) {
    fs_meta meta;

    int i = 1;
    
    read_meta_entry(i++, &meta);

    while (meta.type != MT_END) {
        if (meta.parent == 0)
            printf("%s %x\n", meta.name, meta.ts.u64);
        
        read_meta_entry(i++, &meta);
    }
}