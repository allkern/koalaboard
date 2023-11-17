#ifndef RAM_H
#define RAM_H

#include <stdint.h>

#define RAM_SIZE    0x1f000000
#define RAM_BEGIN   0x00000000
#define RAM_END     0x1effffff

typedef struct {
    uint32_t bus_delay;
    uint32_t io_base, io_size;

    uint8_t* buf;
} psx_ram_t;

psx_ram_t* psx_ram_create();
void psx_ram_init(psx_ram_t*, psx_mc2_t*);
uint32_t psx_ram_read32(psx_ram_t*, uint32_t);
uint16_t psx_ram_read16(psx_ram_t*, uint32_t);
uint8_t psx_ram_read8(psx_ram_t*, uint32_t);
void psx_ram_write32(psx_ram_t*, uint32_t, uint32_t);
void psx_ram_write16(psx_ram_t*, uint32_t, uint16_t);
void psx_ram_write8(psx_ram_t*, uint32_t, uint8_t);
void psx_ram_destroy(psx_ram_t*);

#endif