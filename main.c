#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "elf.h"
#include "bus.h"
#include "ram.h"
#include "vmc.h"
#include "uart.h"

char* get_pt_name(uint32_t pt) {
    char* buf = malloc(14);

    memset(buf, ' ', 14);
    
    if (pt == PT_NULL) {
        strcpy(buf, "NULL");
        buf[strlen("NULL")] = ' ';
    } else if (pt == PT_LOAD) {
        strcpy(buf, "LOAD");
        buf[strlen("LOAD")] = ' ';
    } else if (pt == PT_DYNAMIC) {
        strcpy(buf, "DYNAMIC");
        buf[strlen("DYNAMIC")] = ' ';
    } else if (pt == PT_INTERP) {
        strcpy(buf, "INTERP");
        buf[strlen("INTERP")] = ' ';
    } else if (pt == PT_NOTE) {
        strcpy(buf, "NOTE");
        buf[strlen("NOTE")] = ' ';
    } else if (pt == PT_SHLIB) {
        strcpy(buf, "SHLIB");
        buf[strlen("SHLIB")] = ' ';
    } else if (pt == PT_PHDR) {
        strcpy(buf, "PHDR");
        buf[strlen("PHDR")] = ' ';
    } else if (pt == PT_TLS) {
        strcpy(buf, "TLS");
        buf[strlen("TLS")] = ' ';
    } else if (pt == PT_NUM) {
        strcpy(buf, "NUM");
        buf[strlen("NUM")] = ' ';
    } else if (pt == PT_LOOS) {
        strcpy(buf, "LOOS");
        buf[strlen("LOOS")] = ' ';
    } else if (pt == PT_GNU_EH_FRAME) {
        strcpy(buf, "GNU_EH_FRAME");
        buf[strlen("GNU_EH_FRAME")] = ' ';
    } else if (pt == PT_GNU_STACK) {
        strcpy(buf, "GNU_STACK");
        buf[strlen("GNU_STACK")] = ' ';
    } else if (pt == PT_GNU_RELRO) {
        strcpy(buf, "GNU_RELRO");
        buf[strlen("GNU_RELRO")] = ' ';
    } else if (pt == PT_LOSUNW) {
        strcpy(buf, "LOSUNW");
        buf[strlen("LOSUNW")] = ' ';
    } else if (pt == PT_SUNWBSS) {
        strcpy(buf, "SUNWBSS");
        buf[strlen("SUNWBSS")] = ' ';
    } else if (pt == PT_SUNWSTACK) {
        strcpy(buf, "SUNWSTACK");
        buf[strlen("SUNWSTACK")] = ' ';
    } else if (pt == PT_HISUNW) {
        strcpy(buf, "HISUNW");
        buf[strlen("HISUNW")] = ' ';
    } else if (pt == PT_HIOS) {
        strcpy(buf, "HIOS");
        buf[strlen("HIOS")] = ' ';
    } else if (pt == PT_LOPROC) {
        strcpy(buf, "LOPROC");
        buf[strlen("LOPROC")] = ' ';
    } else if (pt == PT_HIPROC) {
        strcpy(buf, "HIPROC");
        buf[strlen("HIPROC")] = ' ';
    } else {
        strcpy(buf, "<unknown>");
        buf[strlen("<unknown>")] = ' ';
    }

    return buf;
}

#define RAM_PHYS_BASE  0x80000000
#define RAM_SIZE       0x1000000
#define VMC_PHYS_BASE  0x1f800000
#define VMC_SIZE       0x100
#define UART_PHYS_BASE 0x1f900000
#define UART_SIZE      0x8

void uart_tx_event(uint8_t data) {
    putchar((char)data);
}

int main(int argc, const char* argv[]) {
    bus_t* bus = bus_create();
    bus_init(bus);

    bus_device_t* ram_bdev = bus_register_device(
        bus,
        RAM_PHYS_BASE,
        RAM_PHYS_BASE + RAM_SIZE
    );

    ram_t* ram = ram_create();
    ram_init(ram, 0x1000000);
    ram_init_bus_device(ram, ram_bdev);

    bus_device_t* vmc_bdev = bus_register_device(
        bus,
        VMC_PHYS_BASE,
        VMC_PHYS_BASE + VMC_SIZE
    );

    vmc_t* vmc = vmc_create();
    vmc_init(vmc);
    vmc_init_bus_device(vmc, vmc_bdev);

    bus_device_t* uart_bdev = bus_register_device(
        bus,
        UART_PHYS_BASE,
        UART_PHYS_BASE + UART_SIZE
    );

    uart_t* uart = uart_create();
    uart_init(uart, uart_tx_event);
    uart_init_bus_device(uart, uart_bdev);

    r3000_bus_t cpu_bus = {
        .query_access_cycles = bus_query_access_cycles,
        .read32 = bus_read32,
        .read16 = bus_read16,
        .read8 = bus_read8,
        .write32 = bus_write32,
        .write16 = bus_write16,
        .write8 = bus_write8,
        .udata = bus
    };

    r3000_t* cpu = r3000_create();
    r3000_init(cpu, &cpu_bus);

    elf_file_t* elf = elf_create();

    if (elf_load(elf, argv[1]))
        return 1;

    puts("Program Headers:");
    puts("  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align");

    int seg = 0;
    uint32_t offset = 0;

    for (int i = 0; i < elf->ehdr->e_phnum; i++) {
        Elf32_Phdr* phdr = elf->phdr[i];

        char* pt_name = get_pt_name(phdr->p_type);

        printf("  %s 0x%06x 0x%08x 0x%08x 0x%05x 0x%05x %c%c%c 0x%x\n",
            pt_name,
            phdr->p_offset,
            phdr->p_vaddr,
            phdr->p_paddr,
            phdr->p_filesz,
            phdr->p_memsz,
            phdr->p_flags & PF_R ? 'R' : ' ',
            phdr->p_flags & PF_W ? 'W' : ' ',
            phdr->p_flags & PF_X ? 'X' : ' ',
            phdr->p_align
        );

        free(pt_name);

        if (elf->phdr[i]->p_type == PT_LOAD) {
            cpu->tlb[seg].hi = phdr->p_vaddr & 0xfffff000;
            cpu->tlb[seg].lo = ((offset + RAM_PHYS_BASE) & 0xfffff000) | TLBE_G | TLBE_V | TLBE_D;

            printf("Loading TLB index %u v 0x%08x -> p 0x%08x segment @ 0x%08x\n",
                seg,
                cpu->tlb[seg].hi,
                cpu->tlb[seg].lo,
                offset
            );

            elf_load_segment(elf, i, &ram->buf[offset]);

            offset += elf->phdr[i]->p_align; // phdr->p_memsz;

            ++seg;
        }
    }

    // Allocate 4 KiB of stack space
    printf("Allocating stack space...\n");

    cpu->tlb[63].hi = 0x00fff000;
    cpu->tlb[63].lo = 0x80fff000 | TLBE_G | TLBE_V | TLBE_D;
    cpu->r[29] = 0x01000000;
    cpu->r[30] = cpu->r[29];

    printf("Setting PC to entry address %08x\n", elf->ehdr->e_entry);

    r3000_set_pc(cpu, elf->ehdr->e_entry);

    while (!vmc->exit_requested) {
        r3000_cycle(cpu);
        // printf("r0=%08x at=%08x v0=%08x v1=%08x\n", cpu->r[0] , cpu->r[1] , cpu->r[2] , cpu->r[3] );
        // printf("a0=%08x a1=%08x a2=%08x a3=%08x\n", cpu->r[4] , cpu->r[5] , cpu->r[6] , cpu->r[7] );
        // printf("t0=%08x t1=%08x t2=%08x t3=%08x\n", cpu->r[8] , cpu->r[9] , cpu->r[10], cpu->r[11]);
        // printf("t4=%08x t5=%08x t6=%08x t7=%08x\n", cpu->r[12], cpu->r[13], cpu->r[14], cpu->r[15]);
        // printf("s0=%08x s1=%08x s2=%08x s3=%08x\n", cpu->r[16], cpu->r[17], cpu->r[18], cpu->r[19]);
        // printf("s4=%08x s5=%08x s6=%08x s7=%08x\n", cpu->r[20], cpu->r[21], cpu->r[22], cpu->r[23]);
        // printf("t8=%08x t9=%08x k0=%08x k1=%08x\n", cpu->r[24], cpu->r[25], cpu->r[26], cpu->r[27]);
        // printf("gp=%08x sp=%08x fp=%08x ra=%08x\n", cpu->r[28], cpu->r[29], cpu->r[30], cpu->r[31]);
        // printf("pc=%08x hi=%08x lo=%08x\n", cpu->pc, cpu->hi, cpu->lo);

        // fflush(stdout);
        // getchar();

        if (cpu->opcode == 0xffffffff)
            break;
    }

    printf("\nexit_code=%08x\n", vmc->exit_code);

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
    elf_destroy(elf);

    return 0;
}