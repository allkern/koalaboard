#include "cpu.h"
#include "log.h"

#include <stdlib.h>
#include <string.h>

static const r3000_instruction_t g_r3000_secondary_table[] = {
    r3000_i_sll    , r3000_i_invalid, r3000_i_srl    , r3000_i_sra    ,
    r3000_i_sllv   , r3000_i_invalid, r3000_i_srlv   , r3000_i_srav   ,
    r3000_i_jr     , r3000_i_jalr   , r3000_i_invalid, r3000_i_invalid,
    r3000_i_syscall, r3000_i_break  , r3000_i_invalid, r3000_i_invalid,
    r3000_i_mfhi   , r3000_i_mthi   , r3000_i_mflo   , r3000_i_mtlo   ,
    r3000_i_invalid, r3000_i_invalid, r3000_i_invalid, r3000_i_invalid,
    r3000_i_mult   , r3000_i_multu  , r3000_i_div    , r3000_i_divu   ,
    r3000_i_invalid, r3000_i_invalid, r3000_i_invalid, r3000_i_invalid,
    r3000_i_add    , r3000_i_addu   , r3000_i_sub    , r3000_i_subu   ,
    r3000_i_and    , r3000_i_or     , r3000_i_xor    , r3000_i_nor    ,
    r3000_i_invalid, r3000_i_invalid, r3000_i_slt    , r3000_i_sltu   ,
    r3000_i_invalid, r3000_i_invalid, r3000_i_invalid, r3000_i_invalid,
    r3000_i_invalid, r3000_i_invalid, r3000_i_invalid, r3000_i_invalid,
    r3000_i_invalid, r3000_i_invalid, r3000_i_invalid, r3000_i_invalid,
    r3000_i_invalid, r3000_i_invalid, r3000_i_invalid, r3000_i_invalid,
    r3000_i_invalid, r3000_i_invalid, r3000_i_invalid, r3000_i_invalid
};

static const r3000_instruction_t g_r3000_primary_table[] = {
    r3000_i_special, r3000_i_bxx    , r3000_i_j      , r3000_i_jal    ,
    r3000_i_beq    , r3000_i_bne    , r3000_i_blez   , r3000_i_bgtz   ,
    r3000_i_addi   , r3000_i_addiu  , r3000_i_slti   , r3000_i_sltiu  ,
    r3000_i_andi   , r3000_i_ori    , r3000_i_xori   , r3000_i_lui    ,
    r3000_i_cop0   , r3000_i_cop1   , r3000_i_cop2   , r3000_i_cop3   ,
    r3000_i_invalid, r3000_i_invalid, r3000_i_invalid, r3000_i_invalid,
    r3000_i_invalid, r3000_i_invalid, r3000_i_invalid, r3000_i_invalid,
    r3000_i_invalid, r3000_i_invalid, r3000_i_invalid, r3000_i_invalid,
    r3000_i_lb     , r3000_i_lh     , r3000_i_lwl    , r3000_i_lw     ,
    r3000_i_lbu    , r3000_i_lhu    , r3000_i_lwr    , r3000_i_invalid,
    r3000_i_sb     , r3000_i_sh     , r3000_i_swl    , r3000_i_sw     ,
    r3000_i_invalid, r3000_i_invalid, r3000_i_swr    , r3000_i_invalid,
    r3000_i_lwc0   , r3000_i_lwc1   , r3000_i_lwc2   , r3000_i_lwc3   ,
    r3000_i_invalid, r3000_i_invalid, r3000_i_invalid, r3000_i_invalid,
    r3000_i_swc0   , r3000_i_swc1   , r3000_i_swc2   , r3000_i_swc3   ,
    r3000_i_invalid, r3000_i_invalid, r3000_i_invalid, r3000_i_invalid
};

static const r3000_instruction_t g_r3000_cop0_table[] = {
    r3000_i_mfc0   , r3000_i_tlbr   , r3000_i_tlbwi  , r3000_i_invalid,
    r3000_i_mtc0   , r3000_i_invalid, r3000_i_tlbwr  , r3000_i_invalid,
    r3000_i_tlbp   , r3000_i_invalid, r3000_i_invalid, r3000_i_invalid,
    r3000_i_invalid, r3000_i_invalid, r3000_i_invalid, r3000_i_invalid,
    r3000_i_rfe    , r3000_i_invalid, r3000_i_invalid, r3000_i_invalid,
    r3000_i_invalid, r3000_i_invalid, r3000_i_invalid, r3000_i_invalid,
    r3000_i_invalid, r3000_i_invalid, r3000_i_invalid, r3000_i_invalid,
    r3000_i_invalid, r3000_i_invalid, r3000_i_invalid, r3000_i_invalid
};

static const r3000_instruction_t g_r3000_cop1_table[] = {
    r3000_i_mfc1   , r3000_i_invalid, r3000_i_cfc1   , r3000_i_invalid,
    r3000_i_mtc1   , r3000_i_invalid, r3000_i_ctc1   , r3000_i_invalid,
    r3000_i_invalid, r3000_i_invalid, r3000_i_invalid, r3000_i_invalid,
    r3000_i_invalid, r3000_i_invalid, r3000_i_invalid, r3000_i_invalid,
    r3000_i_invalid, r3000_i_invalid, r3000_i_invalid, r3000_i_invalid,
    r3000_i_invalid, r3000_i_invalid, r3000_i_invalid, r3000_i_invalid,
    r3000_i_invalid, r3000_i_invalid, r3000_i_invalid, r3000_i_invalid,
    r3000_i_invalid, r3000_i_invalid, r3000_i_invalid, r3000_i_invalid
};

static const r3000_instruction_t g_r3000_bxx_table[] = {
    r3000_i_bltz   , r3000_i_bgez   , r3000_i_bltz   , r3000_i_bgez   ,
    r3000_i_bltz   , r3000_i_bgez   , r3000_i_bltz   , r3000_i_bgez   ,
    r3000_i_bltz   , r3000_i_bgez   , r3000_i_bltz   , r3000_i_bgez   ,
    r3000_i_bltz   , r3000_i_bgez   , r3000_i_bltz   , r3000_i_bgez   ,
    r3000_i_bltzal , r3000_i_bgezal , r3000_i_bltz   , r3000_i_bgez   ,
    r3000_i_bltz   , r3000_i_bgez   , r3000_i_bltz   , r3000_i_bgez   ,
    r3000_i_bltz   , r3000_i_bgez   , r3000_i_bltz   , r3000_i_bgez   ,
    r3000_i_bltz   , r3000_i_bgez   , r3000_i_bltz   , r3000_i_bgez
};

static const uint32_t g_r3000_cop0_write_mask_table[] = {
    0x00000000, // cop0r0   - N/A
    0x00000000, // cop0r1   - N/A
    0x00000000, // cop0r2   - N/A
    0xffffffff, // BPC      - Breakpoint on execute (R/W)
    0x00000000, // cop0r4   - N/A
    0xffffffff, // BDA      - Breakpoint on data access (R/W)
    0x00000000, // JUMPDEST - Randomly memorized jump address (R)
    0xffc0f03f, // DCIC     - Breakpoint control (R/W)
    0x00000000, // BadVaddr - Bad Virtual Address (R)
    0xffffffff, // BDAM     - Data Access breakpoint mask (R/W)
    0x00000000, // cop0r10  - N/A
    0xffffffff, // BPCM     - Execute breakpoint mask (R/W)
    0xffffffff, // SR       - System status register (R/W)
    0x00000300, // CAUSE    - Describes the most recently recognised exception (R)
    0x00000000, // EPC      - Return Address from Trap (R)
    0x00000000  // PRID     - Processor ID (R)
};

#define OP ((cpu->opcode >> 26) & 0x3f)
#define S ((cpu->opcode >> 21) & 0x1f)
#define T ((cpu->opcode >> 16) & 0x1f)
#define D ((cpu->opcode >> 11) & 0x1f)
#define IMM5 ((cpu->opcode >> 6) & 0x1f)
#define CMT ((cpu->opcode >> 6) & 0xfffff)
#define SOP (cpu->opcode & 0x3f)
#define IMM26 (cpu->opcode & 0x3ffffff)
#define IMM16 (cpu->opcode & 0xffff)
#define IMM16S ((int32_t)((int16_t)IMM16))

#define R_R0 (cpu->r[0])
#define R_A0 (cpu->r[4])
#define R_RA (cpu->r[31])

#define DO_PENDING_LOAD \
    cpu->r[cpu->load_d] = cpu->load_v; \
    R_R0 = 0; \
    cpu->load_v = 0xffffffff; \
    cpu->load_d = 0;

// #define CPU_TRACE

#ifdef CPU_TRACE
static const char* g_mips_cc_register_names[] = {
    "r0", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"
};

#define DEBUG_ALL \
    log_fatal("r0=%08x at=%08x v0=%08x v1=%08x", cpu->r[0] , cpu->r[1] , cpu->r[2] , cpu->r[3] ); \
    log_fatal("a0=%08x a1=%08x a2=%08x a3=%08x", cpu->r[4] , cpu->r[5] , cpu->r[6] , cpu->r[7] ); \
    log_fatal("t0=%08x t1=%08x t2=%08x t3=%08x", cpu->r[8] , cpu->r[9] , cpu->r[10], cpu->r[11]); \
    log_fatal("t4=%08x t5=%08x t6=%08x t7=%08x", cpu->r[12], cpu->r[13], cpu->r[14], cpu->r[15]); \
    log_fatal("s0=%08x s1=%08x s2=%08x s3=%08x", cpu->r[16], cpu->r[17], cpu->r[18], cpu->r[19]); \
    log_fatal("s4=%08x s5=%08x s6=%08x s7=%08x", cpu->r[20], cpu->r[21], cpu->r[22], cpu->r[23]); \
    log_fatal("t8=%08x t9=%08x k0=%08x k1=%08x", cpu->r[24], cpu->r[25], cpu->r[26], cpu->r[27]); \
    log_fatal("gp=%08x sp=%08x fp=%08x ra=%08x", cpu->r[28], cpu->r[29], cpu->r[30], cpu->r[31]); \
    log_fatal("pc=%08x hi=%08x lo=%08x l:%s=%08x", cpu->pc, cpu->hi, cpu->lo, g_mips_cc_register_names[cpu->load_d], cpu->load_v); \
    exit(1)

const char* g_mips_cop0_register_names[] = {
    "cop0_r0",
    "cop0_r1",
    "cop0_r2",
    "cop0_bpc",
    "cop0_r4",
    "cop0_bda",
    "cop0_jumpdest",
    "cop0_dcic",
    "cop0_badvaddr",
    "cop0_bdam",
    "cop0_r10",
    "cop0_bpcm",
    "cop0_sr",
    "cop0_cause",
    "cop0_epc",
    "cop0_prid"
};

#define TRACE_M(m) \
    printf("%08x: %-7s $%s, %+i($%s)\n", cpu->pc-4, m, g_mips_cc_register_names[T], IMM16S, g_mips_cc_register_names[S])

#define TRACE_I16S(m) \
    printf("%08x: %-7s $%s, 0x%04x\n", cpu->pc-4, m, g_mips_cc_register_names[T], IMM16)

#define TRACE_I16D(m) \
    printf("%08x: %-7s $%s, $%s, 0x%04x\n", cpu->pc-4, m, g_mips_cc_register_names[T], g_mips_cc_register_names[S], IMM16)

#define TRACE_I5D(m) \
    printf("%08x: %-7s $%s, $%s, %u\n", cpu->pc-4, m, g_mips_cc_register_names[D], g_mips_cc_register_names[T], IMM5)

#define TRACE_I26(m) \
    printf("%08x: %-7s 0x%07x\n", cpu->pc-4, m, ((cpu->pc & 0xf0000000) | (IMM26 << 2)))

#define TRACE_RT(m) \
    printf("%08x: %-7s $%s, $%s, $%s\n", cpu->pc-4, m, g_mips_cc_register_names[D], g_mips_cc_register_names[S], g_mips_cc_register_names[T])

#define TRACE_C0M(m) \
    printf("%08x: %-7s $%s, $%s\n", cpu->pc-4, m, g_mips_cc_register_names[T], g_mips_cop0_register_names[D])

#define TRACE_C2M(m) \
    printf("%08x: %-7s $%s, $cop2_r%u\n", cpu->pc-4, m, g_mips_cc_register_names[T], D)

#define TRACE_C2MC(m) \
    printf("%08x: %-7s $%s, $cop2_r%u\n", cpu->pc-4, m, g_mips_cc_register_names[T], D + 32)

#define TRACE_B(m) \
    printf("%08x: %-7s $%s, $%s, %-i\n", cpu->pc-4, m, g_mips_cc_register_names[S], g_mips_cc_register_names[T], IMM16S << 2)

#define TRACE_RS(m) \
    printf("%08x: %-7s $%s\n", cpu->pc-4, m, g_mips_cc_register_names[S])

#define TRACE_MTF(m) \
    printf("%08x: %-7s $%s\n", cpu->pc-4, m, g_mips_cc_register_names[D])

#define TRACE_RD(m) \
    printf("%08x: %-7s $%s, $%s\n", cpu->pc-4, m, g_mips_cc_register_names[D], g_mips_cc_register_names[S])

#define TRACE_MD(m) \
    printf("%08x: %-7s $%s, $%s\n", cpu->pc-4, m, g_mips_cc_register_names[S], g_mips_cc_register_names[T]);

#define TRACE_I20(m) \
    printf("%08x: %-7s 0x%05x\n", cpu->pc-4, m, CMT);

#define TRACE_N(m) \
    printf("%08x: %-7s\n", cpu->pc-4, m);
#else
#define TRACE_M(m)
#define TRACE_I16S(m)
#define TRACE_I16D(m)
#define TRACE_I5D(m)
#define TRACE_I26(m)
#define TRACE_RT(m)
#define TRACE_C0M(m)
#define TRACE_C2M(m)
#define TRACE_C2MC(m)
#define TRACE_B(m)
#define TRACE_RS(m)
#define TRACE_MTF(m)
#define TRACE_RD(m)
#define TRACE_MD(m)
#define TRACE_I20(m)
#define TRACE_N(m)
#endif

#define SE8(v) ((int32_t)((int8_t)v))
#define SE16(v) ((int32_t)((int16_t)v))

#define BRANCH(offset) \
    cpu->next_pc = cpu->next_pc + (offset); \
    cpu->next_pc = cpu->next_pc - 4; \
    cpu->branch = 1; \
    cpu->branch_taken = 1;

r3000_t* r3000_create() {
    return (r3000_t*)malloc(sizeof(r3000_t));
}

void r3000_destroy(r3000_t* cpu) {
    free(cpu);
}

uint32_t mmu_tlb_lookup(r3000_t* cpu, uint32_t key) {
    uint32_t kvpn = key & TLBE_VPN;
    uint32_t kasid = key & TLBE_ASID;

    for (int i = 0; i < 64; i++) {
        uint32_t entry = cpu->tlb[i].hi;

        int global = (cpu->tlb[i].lo & TLBE_G) != 0;
        uint32_t evpn = entry & TLBE_VPN;
        uint32_t easid = entry & TLBE_ASID;

        if ((evpn == kvpn) && (global || (easid == kasid)))
            return i;
    }

    return MMU_ENOENT;
}

// To-do: Setup Context register
#define R3000_KUSEG 0
#define R3000_KSEG0 1
#define R3000_KSEG1 2
#define R3000_KSEG2 3

int mmu_get_segment(uint32_t virt) {
    if (virt <= 0x7fffffff) {
        return R3000_KUSEG;
    } else if ((virt >= 0x80000000) && (virt <= 0x9fffffff)) {
        return R3000_KSEG0;
    } else if ((virt >= 0xa0000000) && (virt <= 0xbfffffff)) {
        return R3000_KSEG1;
    }

    return R3000_KSEG2;
}

uint32_t mmu_map_address(r3000_t* cpu, uint32_t virt, uint32_t* phys, int write) {
    int seg = mmu_get_segment(virt);

    if (seg == R3000_KSEG0) {
        *phys = virt & 0x7fffffff;

        return 0;
    } else if (seg == R3000_KSEG1) {
        *phys = virt & 0x1fffffff;

        return 0;
    }

    uint32_t offset = virt & 0xfff;
    uint32_t entry = (virt & 0xfffff000) | (cpu->cop0_r[COP0_ENTRYHI] & TLBE_ASID);

    uint32_t index = mmu_tlb_lookup(cpu, entry);

    if (index == MMU_ENOENT) {
        cpu->cop0_r[COP0_BADVADDR] = cpu->pc;

        r3000_exception(cpu, write ? CAUSE_TLBS : CAUSE_TLBL);

        return MMU_ENOENT;
    }

    uint32_t match = cpu->tlb[index].lo;

    if (write && !(match & TLBE_D)) {
        cpu->cop0_r[COP0_BADVADDR] = cpu->pc;

        r3000_exception(cpu, CAUSE_MOD);

        return MMU_SIGSEGV;
    }

    *phys = (match & 0xfffff000) | offset;

    return 0;
}

uint32_t r3000_read32(r3000_t* cpu, uint32_t addr) {
    uint32_t phys;

    if (mmu_map_address(cpu, addr, &phys, 0))
        return 0xffffffff;

    if (addr & 3) {
        r3000_exception(cpu, CAUSE_ADEL);

        return 0xffffffff;
    }

    uint32_t data = cpu->bus_read32(phys, cpu->bus_udata);

    cpu->last_cycles += cpu->bus_query_access_cycles(cpu->bus_udata);

    return data;
}

uint32_t r3000_read16(r3000_t* cpu, uint32_t addr) {
    uint32_t phys;

    if (mmu_map_address(cpu, addr, &phys, 0))
        return 0xffffffff;

    if (addr & 1) {
        r3000_exception(cpu, CAUSE_ADEL);

        return 0xffffffff;
    }

    uint32_t data = cpu->bus_read16(phys, cpu->bus_udata);

    cpu->last_cycles += cpu->bus_query_access_cycles(cpu->bus_udata);

    return data;
}

uint32_t r3000_read8(r3000_t* cpu, uint32_t addr) {
    uint32_t phys;

    if (mmu_map_address(cpu, addr, &phys, 0))
        return 0xffffffff;

    uint32_t data = cpu->bus_read8(phys, cpu->bus_udata);

    cpu->last_cycles += cpu->bus_query_access_cycles(cpu->bus_udata);

    return data;
}

void r3000_write32(r3000_t* cpu, uint32_t addr, uint32_t data) {
    uint32_t phys;

    if (mmu_map_address(cpu, addr, &phys, 1))
        return;

    if (addr & 3) {
        r3000_exception(cpu, CAUSE_ADES);

        return;
    }

    cpu->bus_write32(phys, data, cpu->bus_udata);

    cpu->last_cycles += cpu->bus_query_access_cycles(cpu->bus_udata);
}

void r3000_write16(r3000_t* cpu, uint32_t addr, uint32_t data) {
    uint32_t phys;

    if (mmu_map_address(cpu, addr, &phys, 1))
        return;

    if (addr & 1) {
        r3000_exception(cpu, CAUSE_ADES);

        return;
    }

    cpu->bus_write16(phys, data, cpu->bus_udata);

    cpu->last_cycles += cpu->bus_query_access_cycles(cpu->bus_udata);
}

void r3000_write8(r3000_t* cpu, uint32_t addr, uint32_t data) {
    uint32_t phys;

    if (mmu_map_address(cpu, addr, &phys, 1))
        return;

    cpu->bus_write8(phys, data, cpu->bus_udata);

    cpu->last_cycles += cpu->bus_query_access_cycles(cpu->bus_udata);
}

void r3000_init(r3000_t* cpu, r3000_bus_t* bus) {
    memset(cpu, 0, sizeof(r3000_t));

    cpu->bus_read32 = bus->read32;
    cpu->bus_read16 = bus->read16;
    cpu->bus_read8 = bus->read8;
    cpu->bus_write32 = bus->write32;
    cpu->bus_write16 = bus->write16;
    cpu->bus_write8 = bus->write8;
    cpu->bus_query_access_cycles = bus->query_access_cycles;
    cpu->bus_udata = bus->udata;

    cpu->pc = 0xbfc00000;
    cpu->next_pc = cpu->pc + 4;

    cpu->cop0_r[COP0_SR] = 0x10900000;
    cpu->cop0_r[COP0_PRID] = 0x00000002;
}

void r3000_cycle(r3000_t* cpu) {
    cpu->last_cycles = 0;

    cpu->saved_pc = cpu->pc;
    cpu->delay_slot = cpu->branch;
    cpu->branch = 0;
    cpu->branch_taken = 0;

    if (cpu->saved_pc & 3)
        r3000_exception(cpu, CAUSE_ADEL);

    cpu->opcode = r3000_read32(cpu, cpu->pc);

    cpu->pc = cpu->next_pc;
    cpu->next_pc += 4;

    if (r3000_check_irq(cpu)) {
        r3000_exception(cpu, CAUSE_INT);

        return;
    }

    g_r3000_primary_table[OP](cpu);

    cpu->last_cycles += 1;
    cpu->total_cycles += cpu->last_cycles;

    cpu->r[0] = 0;
}

int r3000_check_irq(r3000_t* cpu) {
    return (cpu->cop0_r[COP0_SR] & SR_IEC) && (cpu->cop0_r[COP0_SR] & cpu->cop0_r[COP0_CAUSE] & 0x00000700);
}

void r3000_set_pc(r3000_t* cpu, uint32_t addr) {
    cpu->pc = addr;
    cpu->next_pc = cpu->pc + 4;
}

void r3000_exception(r3000_t* cpu, uint32_t cause) {
    cpu->cop0_r[COP0_CAUSE] &= 0x0000ff00;

    // Set excode and clear 3 LSBs
    cpu->cop0_r[COP0_CAUSE] &= 0xffffff80;
    cpu->cop0_r[COP0_CAUSE] |= cause;

    cpu->cop0_r[COP0_EPC] = cpu->saved_pc;

    if (cpu->delay_slot) {
        cpu->cop0_r[COP0_EPC] -= 4;
        cpu->cop0_r[COP0_CAUSE] |= 0x80000000;
    }

    if ((cause == CAUSE_INT) && (cpu->cop0_r[COP0_EPC] & 0xfe000000) == 0x4a000000) {
        cpu->cop0_r[COP0_EPC] += 4;
    }

    // Do exception stack push
    uint32_t mode = cpu->cop0_r[COP0_SR] & 0x3f;

    cpu->cop0_r[COP0_SR] &= 0xffffffc0;
    cpu->cop0_r[COP0_SR] |= (mode << 2) & 0x3f;

    // Set PC to the vector selected on BEV
    cpu->pc = (cpu->cop0_r[COP0_SR] & SR_BEV) ? 0xbfc00180 : 0x80000080;
    cpu->next_pc = cpu->pc + 4;
}

void r3000_set_irq_pending(r3000_t* cpu) {
    cpu->cop0_r[COP0_CAUSE] |= SR_IM2;
}

void r3000_i_invalid(r3000_t* cpu) {
    log_fatal("%08x: Illegal instruction %08x", cpu->pc - 8, cpu->opcode);

    r3000_exception(cpu, CAUSE_RI);
}

// Primary
void r3000_i_special(r3000_t* cpu) {
    g_r3000_secondary_table[SOP](cpu);
}

void r3000_i_bxx(r3000_t* cpu) {
    cpu->branch = 1;
    cpu->branch_taken = 0;

    g_r3000_bxx_table[T](cpu);
}

// BXX
void r3000_i_bltz(r3000_t* cpu) {
    TRACE_B("bltz");

    int32_t s = (int32_t)cpu->r[S];

    DO_PENDING_LOAD;

    if ((int32_t)s < (int32_t)0) {
        BRANCH(IMM16S << 2);
    }
}

void r3000_i_bgez(r3000_t* cpu) {
    TRACE_B("bgez");

    int32_t s = (int32_t)cpu->r[S];

    DO_PENDING_LOAD;

    if ((int32_t)s >= (int32_t)0) {
        BRANCH(IMM16S << 2);
    }
}

void r3000_i_bltzal(r3000_t* cpu) {
    TRACE_B("bltzal");

    int32_t s = (int32_t)cpu->r[S];

    DO_PENDING_LOAD;

    R_RA = cpu->next_pc;

    if ((int32_t)s < (int32_t)0) {
        BRANCH(IMM16S << 2);
    }
}

void r3000_i_bgezal(r3000_t* cpu) {
    TRACE_B("bgezal");

    int32_t s = (int32_t)cpu->r[S];

    DO_PENDING_LOAD;

    R_RA = cpu->next_pc;

    if ((int32_t)s >= (int32_t)0) {
        BRANCH(IMM16S << 2);
    }
}

void r3000_i_j(r3000_t* cpu) {
    cpu->branch = 1;

    TRACE_I26("j");

    DO_PENDING_LOAD;

    cpu->next_pc = (cpu->next_pc & 0xf0000000) | (IMM26 << 2);
}

void r3000_i_jal(r3000_t* cpu) {
    cpu->branch = 1;

    TRACE_I26("jal");

    DO_PENDING_LOAD;

    R_RA = cpu->next_pc;

    cpu->next_pc = (cpu->next_pc & 0xf0000000) | (IMM26 << 2);
}

void r3000_i_beq(r3000_t* cpu) {
    cpu->branch = 1;
    cpu->branch_taken = 0;

    TRACE_B("beq");

    uint32_t s = cpu->r[S];
    uint32_t t = cpu->r[T];

    DO_PENDING_LOAD;

    if (s == t) {
        BRANCH(IMM16S << 2);
    }
}

void r3000_i_bne(r3000_t* cpu) {
    cpu->branch = 1;
    cpu->branch_taken = 0;

    TRACE_B("bne");

    uint32_t s = cpu->r[S];
    uint32_t t = cpu->r[T];

    DO_PENDING_LOAD;

    if (s != t) {
        BRANCH(IMM16S << 2);
    }
}

void r3000_i_blez(r3000_t* cpu) {
    cpu->branch = 1;
    cpu->branch_taken = 0;

    TRACE_B("blez");

    int32_t s = (int32_t)cpu->r[S];

    DO_PENDING_LOAD;

    if ((int32_t)s <= (int32_t)0) {
        BRANCH(IMM16S << 2);
    }
}

void r3000_i_bgtz(r3000_t* cpu) {
    cpu->branch = 1;
    cpu->branch_taken = 0;

    TRACE_B("bgtz");

    int32_t s = (int32_t)cpu->r[S];

    DO_PENDING_LOAD;

    if ((int32_t)s > (int32_t)0) {
        BRANCH(IMM16S << 2);
    }
}

void r3000_i_addi(r3000_t* cpu) {
    TRACE_I16D("addi");

    uint32_t s = cpu->r[S];

    DO_PENDING_LOAD;

    uint32_t i = IMM16S;
    uint32_t r = s + i;
    uint32_t o = (s ^ r) & (i ^ r);

    if (o & 0x80000000) {
        r3000_exception(cpu, CAUSE_OV);
    } else {
        cpu->r[T] = r;
    }
}

void r3000_i_addiu(r3000_t* cpu) {
    TRACE_I16D("addiu");

    uint32_t s = cpu->r[S];

    DO_PENDING_LOAD;

    cpu->r[T] = s + IMM16S;
}

void r3000_i_slti(r3000_t* cpu) {
    TRACE_I16D("slti");

    int32_t s = (int32_t)cpu->r[S];

    DO_PENDING_LOAD;

    cpu->r[T] = s < IMM16S;
}

void r3000_i_sltiu(r3000_t* cpu) {
    TRACE_I16D("sltiu");

    uint32_t s = cpu->r[S];

    DO_PENDING_LOAD;

    cpu->r[T] = s < IMM16S;
}

void r3000_i_andi(r3000_t* cpu) {
    TRACE_I16D("andi");

    uint32_t s = cpu->r[S];

    DO_PENDING_LOAD;

    cpu->r[T] = s & IMM16;
}

void r3000_i_ori(r3000_t* cpu) {
    TRACE_I16D("ori");

    uint32_t s = cpu->r[S];

    DO_PENDING_LOAD;

    cpu->r[T] = s | IMM16;
}

void r3000_i_xori(r3000_t* cpu) {
    TRACE_I16D("xori");

    uint32_t s = cpu->r[S];

    DO_PENDING_LOAD;

    cpu->r[T] = s ^ IMM16;
}

void r3000_i_lui(r3000_t* cpu) {
    TRACE_I16S("lui");

    DO_PENDING_LOAD;

    cpu->r[T] = IMM16 << 16;
}

void r3000_i_cop0(r3000_t* cpu) {
    g_r3000_cop0_table[S](cpu);
}

void r3000_i_cop1(r3000_t* cpu) {
    DO_PENDING_LOAD;

    r3000_exception(cpu, CAUSE_CPU);
}

void r3000_i_cop2(r3000_t* cpu) {
    DO_PENDING_LOAD;

    r3000_exception(cpu, CAUSE_CPU);
}

void r3000_i_cop3(r3000_t* cpu) {
    DO_PENDING_LOAD;

    r3000_exception(cpu, CAUSE_CPU);
}

void r3000_i_lb(r3000_t* cpu) {
    TRACE_M("lb");

    uint32_t s = cpu->r[S];

    DO_PENDING_LOAD;

    cpu->load_d = T;
    cpu->load_v = SE8(r3000_read8(cpu, s + IMM16S));
}

void r3000_i_lh(r3000_t* cpu) {
    TRACE_M("lh");

    uint32_t s = cpu->r[S];

    DO_PENDING_LOAD;

    uint32_t addr = s + IMM16S;

    cpu->load_d = T;
    cpu->load_v = SE16(r3000_read16(cpu, addr));
}

void r3000_i_lwl(r3000_t* cpu) {
    TRACE_M("lwl");

    uint32_t addr = cpu->r[S] + IMM16S;

    uint32_t aligned = r3000_read32(cpu, addr & ~0x3);

    cpu->load_v = cpu->r[T];

    switch (addr & 0x3) {
        case 0: cpu->load_v = (cpu->load_v & 0x00ffffff) | (aligned << 24); break;
        case 1: cpu->load_v = (cpu->load_v & 0x0000ffff) | (aligned << 16); break;
        case 2: cpu->load_v = (cpu->load_v & 0x000000ff) | (aligned << 8 ); break;
        case 3: cpu->load_v =                               aligned       ; break;
    }

    cpu->load_d = T;
}

void r3000_i_lw(r3000_t* cpu) {
    TRACE_M("lw");

    uint32_t s = cpu->r[S];
    uint32_t addr = s + IMM16S;

    DO_PENDING_LOAD;

    cpu->load_d = T;
    cpu->load_v = r3000_read32(cpu, addr);
}

void r3000_i_lbu(r3000_t* cpu) {
    TRACE_M("lbu");

    uint32_t s = cpu->r[S];

    DO_PENDING_LOAD;

    cpu->load_d = T;
    cpu->load_v = r3000_read8(cpu, s + IMM16S);
}

void r3000_i_lhu(r3000_t* cpu) {
    TRACE_M("lhu");

    uint32_t s = cpu->r[S];
    uint32_t addr = s + IMM16S;

    DO_PENDING_LOAD;

    cpu->load_d = T;
    cpu->load_v = r3000_read16(cpu, addr);
}

void r3000_i_lwr(r3000_t* cpu) {
    TRACE_M("lwr");

    uint32_t addr = cpu->r[S] + IMM16S;

    uint32_t aligned = r3000_read32(cpu, addr & ~0x3);

    cpu->load_v = cpu->r[T];

    switch (addr & 0x3) {
        case 0: cpu->load_v =                               aligned       ; break;
        case 1: cpu->load_v = (cpu->load_v & 0xff000000) | (aligned >> 8 ); break;
        case 2: cpu->load_v = (cpu->load_v & 0xffff0000) | (aligned >> 16); break;
        case 3: cpu->load_v = (cpu->load_v & 0xffffff00) | (aligned >> 24); break;
    }

    cpu->load_d = T;
}

void r3000_i_sb(r3000_t* cpu) {
    TRACE_M("sb");

    uint32_t s = cpu->r[S];
    uint32_t t = cpu->r[T];

    DO_PENDING_LOAD;

    // Cache isolated
    if (cpu->cop0_r[COP0_SR] & SR_ISC) {
        log_debug("Ignoring write while cache is isolated");

        return;
    }

    r3000_write8(cpu, s + IMM16S, t);
}

void r3000_i_sh(r3000_t* cpu) {
    TRACE_M("sh");

    uint32_t s = cpu->r[S];
    uint32_t t = cpu->r[T];
    uint32_t addr = s + IMM16S;

    DO_PENDING_LOAD;

    // Cache isolated
    if (cpu->cop0_r[COP0_SR] & SR_ISC) {
        log_debug("Ignoring write while cache is isolated");

        return;
    }

    r3000_write16(cpu, addr, t);
}

void r3000_i_swl(r3000_t* cpu) {
    TRACE_M("swl");

    uint32_t s = cpu->r[S];

    DO_PENDING_LOAD;

    uint32_t addr = s + IMM16S;
    uint32_t aligned = addr & ~0x3;
    uint32_t v = r3000_read32(cpu, aligned);

    switch (addr & 0x3) {
        case 0: v = (v & 0xffffff00) | (cpu->r[T] >> 24); break;
        case 1: v = (v & 0xffff0000) | (cpu->r[T] >> 16); break;
        case 2: v = (v & 0xff000000) | (cpu->r[T] >> 8 ); break;
        case 3: v =                     cpu->r[T]       ; break;
    }

    r3000_write32(cpu, aligned, v);
}

void r3000_i_sw(r3000_t* cpu) {
    TRACE_M("sw");

    uint32_t s = cpu->r[S];
    uint32_t t = cpu->r[T];
    uint32_t addr = s + IMM16S;

    DO_PENDING_LOAD;

    // Cache isolated
    if (cpu->cop0_r[COP0_SR] & SR_ISC) {
        log_debug("Ignoring write while cache is isolated");

        return;
    }

    r3000_write32(cpu, addr, t);
}

void r3000_i_swr(r3000_t* cpu) {
    TRACE_M("swr");

    uint32_t s = cpu->r[S];

    DO_PENDING_LOAD;

    uint32_t addr = s + IMM16S;
    uint32_t aligned = addr & ~0x3;
    uint32_t v = r3000_read32(cpu, aligned);

    switch (addr & 0x3) {
        case 0: v =                     cpu->r[T]       ; break;
        case 1: v = (v & 0x000000ff) | (cpu->r[T] << 8 ); break;
        case 2: v = (v & 0x0000ffff) | (cpu->r[T] << 16); break;
        case 3: v = (v & 0x00ffffff) | (cpu->r[T] << 24); break;
    }

    r3000_write32(cpu, aligned, v);
}

void r3000_i_lwc0(r3000_t* cpu) {
    r3000_exception(cpu, CAUSE_CPU);
}

void r3000_i_lwc1(r3000_t* cpu) {
    r3000_exception(cpu, CAUSE_CPU);
}

void r3000_i_lwc2(r3000_t* cpu) {
    r3000_exception(cpu, CAUSE_CPU);
}

void r3000_i_lwc3(r3000_t* cpu) {
    r3000_exception(cpu, CAUSE_CPU);
}

void r3000_i_swc0(r3000_t* cpu) {
    r3000_exception(cpu, CAUSE_CPU);
}

void r3000_i_swc1(r3000_t* cpu) {
    r3000_exception(cpu, CAUSE_CPU);
}

void r3000_i_swc2(r3000_t* cpu) {
    r3000_exception(cpu, CAUSE_CPU);
}

void r3000_i_swc3(r3000_t* cpu) {
    r3000_exception(cpu, CAUSE_CPU);
}

// Secondary
void r3000_i_sll(r3000_t* cpu) {
    TRACE_I5D("sll");

    uint32_t t = cpu->r[T];

    DO_PENDING_LOAD;

    cpu->r[D] = t << IMM5;
}

void r3000_i_srl(r3000_t* cpu) {
    TRACE_I5D("srl");

    uint32_t t = cpu->r[T];

    DO_PENDING_LOAD;

    cpu->r[D] = t >> IMM5;
}

void r3000_i_sra(r3000_t* cpu) {
    TRACE_I5D("sra");

    int32_t t = (int32_t)cpu->r[T];

    DO_PENDING_LOAD;

    cpu->r[D] = t >> IMM5;
}

void r3000_i_sllv(r3000_t* cpu) {
    TRACE_RT("sllv");

    uint32_t s = cpu->r[S];
    uint32_t t = cpu->r[T];

    DO_PENDING_LOAD;

    cpu->r[D] = t << (s & 0x1f);
}

void r3000_i_srlv(r3000_t* cpu) {
    TRACE_RT("srlv");

    uint32_t s = cpu->r[S];
    uint32_t t = cpu->r[T];

    DO_PENDING_LOAD;

    cpu->r[D] = t >> (s & 0x1f);
}

void r3000_i_srav(r3000_t* cpu) {
    TRACE_RT("srav");

    uint32_t s = cpu->r[S];
    int32_t t = (int32_t)cpu->r[T];

    DO_PENDING_LOAD;

    cpu->r[D] = t >> (s & 0x1f);
}

void r3000_i_jr(r3000_t* cpu) {
    cpu->branch = 1;

    TRACE_RS("jr");

    uint32_t s = cpu->r[S];

    DO_PENDING_LOAD;

    cpu->next_pc = s;
}

void r3000_i_jalr(r3000_t* cpu) {
    cpu->branch = 1;

    TRACE_RD("jalr");

    uint32_t s = cpu->r[S];

    DO_PENDING_LOAD;

    cpu->r[D] = cpu->next_pc;

    cpu->next_pc = s;
}

void r3000_i_syscall(r3000_t* cpu) {
    TRACE_I20("syscall");
    
    DO_PENDING_LOAD;

    r3000_exception(cpu, CAUSE_SYSCALL);
}

void r3000_i_break(r3000_t* cpu) {
    TRACE_I20("break");

    DO_PENDING_LOAD;

    r3000_exception(cpu, CAUSE_BP);
}

void r3000_i_mfhi(r3000_t* cpu) {
    TRACE_MTF("mfhi");

    DO_PENDING_LOAD;

    cpu->r[D] = cpu->hi;
}

void r3000_i_mthi(r3000_t* cpu) {
    TRACE_MTF("mthi");

    DO_PENDING_LOAD;

    cpu->hi = cpu->r[S];
}

void r3000_i_mflo(r3000_t* cpu) {
    TRACE_MTF("mflo");

    DO_PENDING_LOAD;

    cpu->r[D] = cpu->lo;
}

void r3000_i_mtlo(r3000_t* cpu) {
    TRACE_MTF("mtlo");

    DO_PENDING_LOAD;

    cpu->lo = cpu->r[S];
}

void r3000_i_mult(r3000_t* cpu) {
    TRACE_MD("mult");

    int64_t s = (int64_t)((int32_t)cpu->r[S]);
    int64_t t = (int64_t)((int32_t)cpu->r[T]);

    DO_PENDING_LOAD;

    uint64_t r = s * t;

    cpu->hi = r >> 32;
    cpu->lo = r & 0xffffffff;
}

void r3000_i_multu(r3000_t* cpu) {
    TRACE_MD("multu");

    uint64_t s = (uint64_t)cpu->r[S];
    uint64_t t = (uint64_t)cpu->r[T];

    DO_PENDING_LOAD;

    uint64_t r = s * t;

    cpu->hi = r >> 32;
    cpu->lo = r & 0xffffffff;
}

void r3000_i_div(r3000_t* cpu) {
    TRACE_MD("div");

    int32_t s = (int32_t)cpu->r[S];
    int32_t t = (int32_t)cpu->r[T];

    DO_PENDING_LOAD;

    if (!t) {
        cpu->hi = s;
        cpu->lo = (s >= 0) ? 0xffffffff : 1;
    } else if ((((uint32_t)s) == 0x80000000) && (t == -1)) {
        cpu->hi = 0;
        cpu->lo = 0x80000000;
    } else {
        cpu->hi = (uint32_t)(s % t);
        cpu->lo = (uint32_t)(s / t);
    }
}

void r3000_i_divu(r3000_t* cpu) {
    TRACE_MD("divu");

    uint32_t s = cpu->r[S];
    uint32_t t = cpu->r[T];

    DO_PENDING_LOAD;

    if (!t) {
        cpu->hi = s;
        cpu->lo = 0xffffffff;
    } else {
        cpu->hi = s % t;
        cpu->lo = s / t;
    }
}

void r3000_i_add(r3000_t* cpu) {
    TRACE_RT("add");

    int32_t s = cpu->r[S];
    int32_t t = cpu->r[T];

    DO_PENDING_LOAD;

    int32_t r = s + t;
    uint32_t o = (s ^ r) & (t ^ r);

    if (o & 0x80000000) {
        r3000_exception(cpu, CAUSE_OV);
    } else {
        cpu->r[D] = (uint32_t)r;
    }
}

void r3000_i_addu(r3000_t* cpu) {
    TRACE_RT("addu");

    uint32_t s = cpu->r[S];
    uint32_t t = cpu->r[T];

    DO_PENDING_LOAD;

    cpu->r[D] = s + t;
}

void r3000_i_sub(r3000_t* cpu) {
    TRACE_RT("sub");

    int32_t s = (int32_t)cpu->r[S];
    int32_t t = (int32_t)cpu->r[T];
    int32_t r;

    DO_PENDING_LOAD;

    int o = __builtin_ssub_overflow(s, t, &r);

    if (o) {
        r3000_exception(cpu, CAUSE_OV);
    } else {
        cpu->r[D] = r;
    }
}

void r3000_i_subu(r3000_t* cpu) {
    TRACE_RT("subu");

    uint32_t s = cpu->r[S];
    uint32_t t = cpu->r[T];

    DO_PENDING_LOAD;

    cpu->r[D] = s - t;
}

void r3000_i_and(r3000_t* cpu) {
    TRACE_RT("and");

    uint32_t s = cpu->r[S];
    uint32_t t = cpu->r[T];

    DO_PENDING_LOAD;

    cpu->r[D] = s & t;
}

void r3000_i_or(r3000_t* cpu) {
    TRACE_RT("or");

    uint32_t s = cpu->r[S];
    uint32_t t = cpu->r[T];

    DO_PENDING_LOAD;

    cpu->r[D] = s | t;
}

void r3000_i_xor(r3000_t* cpu) {
    TRACE_RT("xor");

    uint32_t s = cpu->r[S];
    uint32_t t = cpu->r[T];

    DO_PENDING_LOAD;

    cpu->r[D] = (s ^ t);
}

void r3000_i_nor(r3000_t* cpu) {
    TRACE_RT("nor");

    uint32_t s = cpu->r[S];
    uint32_t t = cpu->r[T];

    DO_PENDING_LOAD;

    cpu->r[D] = ~(s | t);
}

void r3000_i_slt(r3000_t* cpu) {
    TRACE_RT("slt");

    int32_t s = (int32_t)cpu->r[S];
    int32_t t = (int32_t)cpu->r[T];

    DO_PENDING_LOAD;

    cpu->r[D] = s < t;
}

void r3000_i_sltu(r3000_t* cpu) {
    TRACE_RT("sltu");

    uint32_t s = cpu->r[S];
    uint32_t t = cpu->r[T];

    DO_PENDING_LOAD;

    cpu->r[D] = s < t;
}

// COP0
void r3000_i_mfc0(r3000_t* cpu) {
    TRACE_C0M("mfc0");

    DO_PENDING_LOAD;

    cpu->load_v = cpu->cop0_r[D];
    cpu->load_d = T;
}

void r3000_i_mtc0(r3000_t* cpu) {
    TRACE_C0M("mtc0");

    uint32_t t = cpu->r[T];

    DO_PENDING_LOAD;

    cpu->cop0_r[D] = t & g_r3000_cop0_write_mask_table[D];
}

void r3000_i_rfe(r3000_t* cpu) {
    TRACE_N("rfe");

    DO_PENDING_LOAD;

    uint32_t mode = cpu->cop0_r[COP0_SR] & 0x3f;

    cpu->cop0_r[COP0_SR] &= 0xfffffff0;
    cpu->cop0_r[COP0_SR] |= mode >> 2;
}

void r3000_i_tlbp(r3000_t* cpu) {
    TRACE_N("tlbp");

    DO_PENDING_LOAD;

    uint32_t index = mmu_tlb_lookup(cpu, cpu->cop0_r[COP0_ENTRYHI]);

    if (index == MMU_ENOENT) {
        cpu->cop0_r[COP0_INDEX] |= 0x80000000;
    } else {
        cpu->cop0_r[COP0_INDEX] = index << 8;
    }
}

void r3000_i_tlbr(r3000_t* cpu) {
    TRACE_N("tlbr");

    DO_PENDING_LOAD;

    uint32_t index = cpu->cop0_r[COP0_INDEX] & 0x3f;

    cpu->cop0_r[COP0_ENTRYLO] = cpu->tlb[index].lo;
    cpu->cop0_r[COP0_ENTRYHI] = cpu->tlb[index].hi;
}

void r3000_i_tlbwi(r3000_t* cpu) {
    TRACE_N("tlbwi");

    DO_PENDING_LOAD;

    uint32_t index = (cpu->cop0_r[COP0_INDEX] >> 8) & 0x3f;

    cpu->tlb[index].lo = cpu->cop0_r[COP0_ENTRYLO];
    cpu->tlb[index].hi = cpu->cop0_r[COP0_ENTRYHI];
}

void r3000_i_tlbwr(r3000_t* cpu) {
    TRACE_N("tlbwr");

    DO_PENDING_LOAD;

    uint32_t index = (cpu->cop0_r[COP0_RANDOM] >> 8) & 0x3f;

    cpu->tlb[index].lo = cpu->cop0_r[COP0_ENTRYLO];
    cpu->tlb[index].hi = cpu->cop0_r[COP0_ENTRYHI];
}

// To-do: COP1 (FPU)
void r3000_i_mfc1(r3000_t* cpu) {}
void r3000_i_cfc1(r3000_t* cpu) {}
void r3000_i_mtc1(r3000_t* cpu) {}
void r3000_i_ctc1(r3000_t* cpu) {}
void r3000_i_fpu(r3000_t* cpu) {}

#undef R_R0
#undef R_A0
#undef R_RA

#undef OP
#undef S
#undef T
#undef D
#undef IMM5
#undef CMT
#undef SOP
#undef IMM26
#undef IMM16
#undef IMM16S

#undef TRACE_M
#undef TRACE_I16S
#undef TRACE_I16D
#undef TRACE_I5D
#undef TRACE_I26
#undef TRACE_RT
#undef TRACE_C0M
#undef TRACE_C2M
#undef TRACE_C2MC
#undef TRACE_B
#undef TRACE_RS
#undef TRACE_MTF
#undef TRACE_RD
#undef TRACE_MD
#undef TRACE_I20
#undef TRACE_N

#undef DO_PENDING_LOAD

#undef DEBUG_ALL

#undef SE8
#undef SE16