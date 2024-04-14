#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "elf_ldr.h"
#include "r3000.h"
#include "bus.h"
#include "ram.h"
#include "vmc.h"
#include "uart.h"
#include "rtc.h"
#include "gpu.h"
#include "dma.h"
#include "ic.h"
#include "nvs.h"
#include "rom.h"
#include "nvram.h"

#ifndef NOGRAPHICS
#include "screen.h"
#endif

#include "argparse.h"

/*
    Koalaboard physical memory map:
    00000000-0000ffff - NVRAM (64 KiB)
    00010000-1f7fffff - Empty
    1f800000-1f801818 - MMIO registers
    1fc00000-1fc0ffff - Firmware ROM (64 KiB)
    80000000-81000000 - RAM (16 MiB)
*/

#define RAM_PHYS_BASE   0x00000000
#define NVRAM_PHYS_BASE 0x10000000
#define VMC_PHYS_BASE   0x1f800000
#define UART_PHYS_BASE  0x1f900000
#define RTC_PHYS_BASE   0x1f900010
#define NVS_PHYS_BASE   0x1fa00000
#define GPU_PHYS_BASE   0x1f801810
#define DMA_PHYS_BASE   0x1f801080
#define IC_PHYS_BASE    0x1f801070
#define ROM_PHYS_BASE   0x1fc00000
#define RAM_SIZE        0x1000000
#define NVRAM_SIZE      0x10000
#define VMC_SIZE        0x10
#define UART_SIZE       0x8
#define RTC_SIZE        0xc
#define NVS_SIZE        0x40
#define GPU_SIZE        0x8
#define DMA_SIZE        0x100
#define IC_SIZE         0x8
#define ROM_SIZE        0x10000

void uart_tx_event(uint8_t data) {
    putchar((char)data);

    fflush(stdout);
}

int default_query_access_cycles(void* udata) {
    return 0;
}

static const char* g_desc_text =
    "\nPlease report any bugs to <https://github.com/allkern/koalaboard/issues>\n";

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

#undef main

int main(int argc, const char* argv[]) {
    bus_t* bus = bus_create();
    bus_init(bus);
    
    const char* nvram_path = "nvram.bin";
    const char* nvs_path = "disk.img";
    const char* rom_path = "rom.bin";
    const char* elf_path = NULL;

    static const char *const usages[] = {
        "koalaboard [options] path-to-elf-executable",
        NULL,
    };

    struct argparse_option options[] = {
        OPT_BOOLEAN ('h', "help"      , NULL       , "Display this information", argparse_help_cb, 0, 0),
        OPT_GROUP("Basic options"),
        OPT_STRING  ('n', "nvram"     , &nvram_path, "Specify an NVRAM dump file", NULL, 0, 0),
        OPT_STRING  ('d', "disk"      , &nvs_path  , "Specify a disk image file", NULL, 0, 0),
        OPT_STRING  ('f', "firmware"  , &rom_path  , "Specify a firmware image file", NULL, 0, 0),
        OPT_STRING  ('x', "executable", &elf_path  , "Specify an ELF executable"),
        OPT_END()
    };

    struct argparse argparse;

    argparse_init(&argparse, options, usages, 0);
    argparse_describe(&argparse, NULL, g_desc_text);

    argc = argparse_parse(&argparse, argc, argv);

    if (argc) {
        if (argc > 1) {
            printf("Unrecognized parameter \'%s\'\n", argv[1]);

            exit(1);
        }

        elf_path = argv[0];
    }

    if (!elf_path) {
        printf("No input files!\n");

        return 0;
    }

    r3000_bus cpu_bus = {
        .query_access_cycles = bus_query_access_cycles,
        .read32 = bus_read32,
        .read16 = bus_read16,
        .read8 = bus_read8,
        .write32 = bus_write32,
        .write16 = bus_write16,
        .write8 = bus_write8,
        .udata = bus
    };

    r3000_state* cpu = r3000_create();
    r3000_init(cpu, &cpu_bus);

    bus_device_t* ram_bdev = bus_register_device(bus,
        RAM_PHYS_BASE,
        RAM_PHYS_BASE + RAM_SIZE
    );

    ram_t* ram = ram_create();
    ram_init(ram, RAM_SIZE);
    ram_init_bus_device(ram, ram_bdev);

    bus_device_t* nvram_bdev = bus_register_device(bus,
        NVRAM_PHYS_BASE,
        NVRAM_PHYS_BASE + NVRAM_SIZE
    );

    nvram_t* nvram = nvram_create();
    nvram_init(nvram, NVRAM_SIZE, nvram_path);
    nvram_init_bus_device(nvram, nvram_bdev);

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

    bus_device_t* rtc_bdev = bus_register_device(
        bus,
        RTC_PHYS_BASE,
        RTC_PHYS_BASE + RTC_SIZE
    );

    rtc_t* rtc = rtc_create();
    rtc_init(rtc);
    rtc_init_bus_device(rtc, rtc_bdev);

    bus_device_t* nvs_bdev = bus_register_device(
        bus,
        NVS_PHYS_BASE,
        NVS_PHYS_BASE + NVS_SIZE
    );

    nvs_t* nvs = nvs_create();
    nvs_init(nvs);
    nvs_open(nvs, 0, nvs_path);
    nvs_init_bus_device(nvs, nvs_bdev);

    bus_device_t* ic_bdev = bus_register_device(
        bus,
        IC_PHYS_BASE,
        IC_PHYS_BASE + IC_SIZE
    );

    psx_ic_t* ic = psx_ic_create();
    psx_ic_init(ic, cpu);

    ic_bdev->query_access_cycles = default_query_access_cycles;
    ic_bdev->read32  = bus_ic_read32;
    ic_bdev->read16  = bus_ic_read16;
    ic_bdev->read8   = bus_ic_read8;
    ic_bdev->write32 = bus_ic_write32;
    ic_bdev->write16 = bus_ic_write16;
    ic_bdev->write8  = bus_ic_write8;
    ic_bdev->udata   = ic;

    bus_device_t* rom_bdev = bus_register_device(
        bus,
        ROM_PHYS_BASE,
        ROM_PHYS_BASE + ROM_SIZE
    );

    rom_t* rom = rom_create();
    rom_init(rom, rom_path);
    rom_init_bus_device(rom, rom_bdev);

    bus_device_t* gpu_bdev = bus_register_device(
        bus,
        GPU_PHYS_BASE,
        GPU_PHYS_BASE + GPU_SIZE
    );

    screen_t* screen = screen_create();

    psx_gpu_t* gpu = psx_gpu_create();
    psx_gpu_init(gpu, ic);
    psx_gpu_set_cpu_freq(gpu, R3000A_FREQ);
    psx_gpu_set_event_callback(gpu, GPU_EVENT_DMODE, gpu_dmode_event_cb);
    psx_gpu_set_event_callback(gpu, GPU_EVENT_VBLANK, gpu_vblank_event_cb);
    psx_gpu_set_udata(gpu, 0, screen);
    
    gpu_bdev->query_access_cycles = default_query_access_cycles;
    gpu_bdev->read32  = bus_gpu_read32;
    gpu_bdev->read16  = bus_gpu_read16;
    gpu_bdev->read8   = bus_gpu_read8;
    gpu_bdev->write32 = bus_gpu_write32;
    gpu_bdev->write16 = bus_gpu_write16;
    gpu_bdev->write8  = bus_gpu_write8;
    gpu_bdev->udata   = gpu;

    bus_device_t* dma_bdev = bus_register_device(
        bus,
        DMA_PHYS_BASE,
        DMA_PHYS_BASE + DMA_SIZE
    );

    psx_dma_t* dma = psx_dma_create();
    psx_dma_init(dma, bus, ic);

    dma_bdev->query_access_cycles = default_query_access_cycles;
    dma_bdev->read32  = bus_dma_read32;
    dma_bdev->read16  = bus_dma_read16;
    dma_bdev->read8   = bus_dma_read8;
    dma_bdev->write32 = bus_dma_write32;
    dma_bdev->write16 = bus_dma_write16;
    dma_bdev->write8  = bus_dma_write8;
    dma_bdev->udata   = dma;

    screen_init(screen, gpu, uart);
    screen_set_scale(screen, 2);
    screen_reload(screen);

    elf_file_t* elf = elf_create();

    if (elf_load(elf, elf_path))
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
            // int segments = (phdr->p_memsz >> 12) + ((phdr->p_memsz & 0xfff) ? 1 : 0) + 1;

            // printf("%u segments required\n", segments);

            // uint32_t vaddr = phdr->p_vaddr;
            // uint32_t paddr = phdr->p_paddr;

            elf_load_segment(elf, i, &ram->buf[phdr->p_paddr]);

            // for (int s = 0; s < segments; s++) {
            //     cpu->tlb[seg].hi = vaddr & 0xfffff000;
            //     cpu->tlb[seg].lo = ((paddr + RAM_PHYS_BASE) & 0xfffff000) | TLBE_G | TLBE_V | TLBE_D;

            //     printf("Loading TLB index %u v 0x%08x -> p 0x%08x segment @ 0x%08x\n",
            //         seg,
            //         cpu->tlb[seg].hi,
            //         cpu->tlb[seg].lo,
            //         paddr
            //     );

            //     paddr += 0x1000;
            //     vaddr += 0x1000;
            //     ++seg;
            // }

            // offset += phdr->p_align;
        }
    }

    // Allocate 4 KiB of stack space
    // printf("Allocating stack space...\n");
    // cpu->tlb[63].hi = 0x00fff000;
    // cpu->tlb[63].lo = 0x00fff000 | TLBE_G | TLBE_V | TLBE_D;

    printf("Initializing stack... ");
    cpu->r[29] = 0x01000000;
    cpu->r[30] = cpu->r[29];

    printf("0x%08x\n", cpu->r[29]);

    printf("Initializing pagetable... ");

    int total_pages = RAM_SIZE / 0x1000;

    printf("%d pages, %d KiB\n", total_pages, (total_pages * 4) / 1024);

    uint32_t* ptr = nvram->buf;

    for (int i = 0; i < total_pages; i++)
        ptr[i] = (RAM_PHYS_BASE + (i << 12)) | TLBE_G | TLBE_V | TLBE_D;

    printf("Initializing kernel variables...\n");
    
    // To-do: Set to first free page after OS executable
    // SYS_HEAP_BASE_PAGE: 0x90008008
    // *(uint32_t*)&nvram->buf[0x8008] = 0x00000000;

    printf("Setting PC to entry address %08x\n", elf->ehdr->e_entry);

    // Set BEV (exception vector)
    cpu->cop0_r[COP0_SR] = 0x400000;

    // Set PTEBase to 90000000
    cpu->cop0_r[COP0_CONTEXT] = 0x90000000;

    r3000_set_pc(cpu, elf->ehdr->e_entry);

    printf("MMIO devices:\n");

    for (int i = 0; i < bus->device_count; i++) {
        printf("Device at physical range: %08x-%08x\n",
            bus->devices[i]->io_start,
            bus->devices[i]->io_end
        );
    }

    while (screen_is_open(screen) && !vmc->exit_requested) {
        r3000_cycle(cpu);
        psx_gpu_update(gpu, 1);

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

        if (cpu->opcode == 0xdeadc0de)
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
    nvs_destroy(nvs);
    nvram_destroy(nvram);

    return 0;
}
