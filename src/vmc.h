#ifndef VMC_H
#define VMC_H

#include <stdint.h>
#include <stdlib.h>

#include "bus.h"

typedef struct {
    int exit_code;
    int exit_requested;
} vmc_t;

vmc_t* vmc_create();
void vmc_init(vmc_t*);
int vmc_query_access_cycles(void*);
uint32_t vmc_read32(uint32_t, void*);
uint32_t vmc_read16(uint32_t, void*);
uint32_t vmc_read8(uint32_t, void*);
void vmc_write32(uint32_t, uint32_t, void*);
void vmc_write16(uint32_t, uint32_t, void*);
void vmc_write8(uint32_t, uint32_t, void*);
void vmc_destroy(vmc_t*);
void vmc_init_bus_device(vmc_t*, bus_device_t*);

#endif