#include <stdint.h>

#include "cpu.h"

uint8_t program[] = {
    0xad, 0xde, 0x02, 0x3c, 0xef, 0xbe, 0x42, 0x34,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

int bus_query(void* udata) {
    return 0;
}

void bus_write(uint32_t addr, uint32_t data, void* udata) {
    return;
}

uint32_t bus_read(uint32_t addr, void* udata) {
    if (addr >= 0xbfc00000)
        return *(uint32_t*)(&program[addr - 0xbfc00000]);

    return 0x00000000;
}

int main() {
    r3000_bus_t bus = {
        .query_access_cycles = bus_query,
        .read32 = bus_read,
        .read16 = bus_read,
        .read8 = bus_read,
        .write32 = bus_write,
        .write16 = bus_write,
        .write8 = bus_write
    };

    r3000_t* cpu = r3000_create();
    r3000_init(cpu, &bus);
    r3000_cycle(cpu);
    r3000_cycle(cpu);
    r3000_cycle(cpu);
    r3000_cycle(cpu);
    r3000_cycle(cpu);
    r3000_cycle(cpu);
    r3000_cycle(cpu);
    r3000_cycle(cpu);

    printf("r0=%08x at=%08x v0=%08x v1=%08x\n", cpu->r[0] , cpu->r[1] , cpu->r[2] , cpu->r[3] );
    printf("a0=%08x a1=%08x a2=%08x a3=%08x\n", cpu->r[4] , cpu->r[5] , cpu->r[6] , cpu->r[7] );
    printf("t0=%08x t1=%08x t2=%08x t3=%08x\n", cpu->r[8] , cpu->r[9] , cpu->r[10], cpu->r[11]);
    printf("t4=%08x t5=%08x t6=%08x t7=%08x\n", cpu->r[12], cpu->r[13], cpu->r[14], cpu->r[15]);
    printf("s0=%08x s1=%08x s2=%08x s3=%08x\n", cpu->r[16], cpu->r[17], cpu->r[18], cpu->r[19]);
    printf("s4=%08x s5=%08x s6=%08x s7=%08x\n", cpu->r[20], cpu->r[21], cpu->r[22], cpu->r[23]);
    printf("t8=%08x t9=%08x k0=%08x k1=%08x\n", cpu->r[24], cpu->r[25], cpu->r[26], cpu->r[27]);
    printf("gp=%08x sp=%08x fp=%08x ra=%08x\n", cpu->r[28], cpu->r[29], cpu->r[30], cpu->r[31]);
    printf("pc=%08x hi=%08x lo=%08x\n", cpu->pc, cpu->hi, cpu->lo);

    r3000_destroy(cpu);

    return 0;
}