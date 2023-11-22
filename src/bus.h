#ifndef BUS_H
#define BUS_H

#include <stdint.h>

typedef int (*dev_query_access_cycles_t)(void* udata);
typedef uint32_t (*dev_read32_t)(uint32_t addr, void* udata);
typedef uint32_t (*dev_read16_t)(uint32_t addr, void* udata);
typedef uint32_t (*dev_read8_t)(uint32_t addr, void* udata);
typedef void (*dev_write32_t)(uint32_t addr, uint32_t data, void* udata);
typedef void (*dev_write16_t)(uint32_t addr, uint32_t data, void* udata);
typedef void (*dev_write8_t)(uint32_t addr, uint32_t data, void* udata);

typedef struct {
    uint32_t io_start, io_end;

    dev_query_access_cycles_t query_access_cycles;
    dev_read32_t read32;
    dev_read16_t read16;
    dev_read8_t read8;
    dev_write32_t write32;
    dev_write16_t write16;
    dev_write8_t write8;
    void* udata;
} bus_device_t;

bus_device_t* bus_device_create();
void bus_device_init(bus_device_t*, uint32_t, uint32_t);
void bus_device_destroy(bus_device_t*);

typedef struct {
    bus_device_t** devices;
    unsigned       device_count;
    int            last_access_cycles;
} bus_t;

bus_t* bus_create();
void bus_init(bus_t* bus);
bus_device_t* bus_register_device(bus_t* bus, uint32_t, uint32_t);
void bus_destroy(bus_t* bus);
uint32_t bus_read32(uint32_t addr, void* udata);
uint32_t bus_read16(uint32_t addr, void* udata);
uint32_t bus_read8(uint32_t addr, void* udata);
uint32_t bus_write32(uint32_t addr, uint32_t data, void* udata);
uint32_t bus_write16(uint32_t addr, uint32_t data, void* udata);
uint32_t bus_write8(uint32_t addr, uint32_t data, void* udata);
int bus_query_access_cycles(void* udata);

#endif

