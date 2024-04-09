#include "bus.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

bus_device_t* bus_device_create() {
    return (bus_device_t*)malloc(sizeof(bus_device_t));
}

void bus_device_init(bus_device_t* dev, uint32_t io_start, uint32_t io_end) {
    memset(dev, 0, sizeof(bus_device_t));

    dev->io_start = io_start;
    dev->io_end = io_end;
}

void bus_device_destroy(bus_device_t* dev) {
    free(dev);
}

bus_t* bus_create() {
    return (bus_t*)malloc(sizeof(bus_t));
}

void bus_init(bus_t* bus) {
    memset(bus, 0, sizeof(bus_t));

    bus->devices = (bus_device_t**)0;
    bus->device_count = 0;
}

bus_device_t* bus_register_device(bus_t* bus, uint32_t io_start, uint32_t io_end) {
    bus_device_t* dev = bus_device_create();
    bus_device_init(dev, io_start, io_end);

    if (!bus->devices) {
        bus->devices = malloc(sizeof(bus_device_t*));
    } else {
        bus->devices = realloc(bus->devices, sizeof(bus_device_t*) * (1 + bus->device_count));
    }

    bus->devices[bus->device_count++] = dev;

    return dev;
}

bus_device_t* search_device(bus_t* bus, uint32_t addr) {
    for (int i = 0; i < bus->device_count; i++) {
        bus_device_t* dev = bus->devices[i];

        if ((addr >= dev->io_start) && (addr < dev->io_end))
            return dev;
    }

    return NULL;
}

void bus_destroy(bus_t* bus) {
    for (int i = 0; i < bus->device_count; i++) {
        bus_device_destroy(bus->devices[i]);
    
        free(bus->devices[i]);
    }

    free(bus->devices);
    free(bus);
}

uint32_t bus_read32(uint32_t addr, void* udata) {
    bus_t* bus = (bus_t*)udata;

    bus_device_t* dev = search_device(bus, addr);

    // Bus error
    if (!dev)
        return 0xdeadc0de;

    return dev->read32(addr - dev->io_start, dev->udata);
}

uint32_t bus_read16(uint32_t addr, void* udata) {
    bus_t* bus = (bus_t*)udata;

    bus_device_t* dev = search_device(bus, addr);

    // Bus error
    if (!dev)
        return 0xbaad;

    return dev->read16(addr - dev->io_start, dev->udata);
}

uint32_t bus_read8(uint32_t addr, void* udata) {
    bus_t* bus = (bus_t*)udata;

    bus_device_t* dev = search_device(bus, addr);

    // Bus error
    if (!dev)
        return 0xf0;

    return dev->read8(addr - dev->io_start, dev->udata);
}

uint32_t bus_write32(uint32_t addr, uint32_t data, void* udata) {
    bus_t* bus = (bus_t*)udata;

    bus_device_t* dev = search_device(bus, addr);

    // Bus error
    if (!dev)
        return 0xffffffff;

    dev->write32(addr - dev->io_start, data, dev->udata);
}

uint32_t bus_write16(uint32_t addr, uint32_t data, void* udata) {
    bus_t* bus = (bus_t*)udata;

    bus_device_t* dev = search_device(bus, addr);

    // Bus error
    if (!dev)
        return 0xffffffff;

    dev->write16(addr - dev->io_start, data, dev->udata);
}

uint32_t bus_write8(uint32_t addr, uint32_t data, void* udata) {
    bus_t* bus = (bus_t*)udata;

    bus_device_t* dev = search_device(bus, addr);

    // Bus error
    if (!dev)
        return 0xffffffff;

    dev->write8(addr - dev->io_start, data, dev->udata);
}

int bus_query_access_cycles(void* udata) {
    bus_t* bus = (bus_t*)udata;

    return bus->last_access_cycles;
}