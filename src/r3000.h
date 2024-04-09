/**
 * @file cpu.h
 * @brief MIPS R3000 emulator (with cache and MMU)
 */

#ifndef R3000A_H
#define R3000A_H

#include <stdint.h>
#include <stdio.h>

#define R3000A_CPS 10000000 // 100000000 Clocks/s
#define R3000A_FREQ 10.000000f // 100.000000 MHz

struct r3000_state;

typedef struct r3000_state r3000_state;

/*
    cop0r0      - Index
    cop0r1      - Random
    cop0r2      - EntryLo
    cop0r3      - BPC - Breakpoint on execute (R/W)
    cop0r4      - Context
    cop0r5      - BDA - Breakpoint on data access (R/W)
    cop0r6      - JUMPDEST - Randomly memorized jump address (R)
    cop0r7      - DCIC - Breakpoint control (R/W)
    cop0r8      - BadVaddr - Bad Virtual Address (R)
    cop0r9      - BDAM - Data Access breakpoint mask (R/W)
    cop0r10     - EntryHi
    cop0r11     - BPCM - Execute breakpoint mask (R/W)
    cop0r12     - SR - System status register (R/W)
    cop0r13     - CAUSE - Describes the most recently recognised exception (R)
    cop0r14     - EPC - Return Address from Trap (R)
    cop0r15     - PRID - Processor ID (R)
*/

#define COP0_INDEX    0
#define COP0_RANDOM   1
#define COP0_ENTRYLO  2
#define COP0_BPC      3
#define COP0_CONTEXT  4
#define COP0_BDA      5
#define COP0_JUMPDEST 6
#define COP0_DCIC     7
#define COP0_BADVADDR 8
#define COP0_BDAM     9
#define COP0_ENTRYHI  10
#define COP0_BPCM     11
#define COP0_SR       12
#define COP0_CAUSE    13
#define COP0_EPC      14
#define COP0_PRID     15

/*
  Name       Alias    Common Usage
  R0         zero     Constant (always 0)
  R1         at       Assembler temporary (destroyed by some assembler pseudoinstructions!)
  R2-R3      v0-v1    Subroutine return values, may be changed by subroutines
  R4-R7      a0-a3    Subroutine arguments, may be changed by subroutines
  R8-R15     t0-t7    Temporaries, may be changed by subroutines
  R16-R23    s0-s7    Static variables, must be saved by subs
  R24-R25    t8-t9    Temporaries, may be changed by subroutines
  R26-R27    k0-k1    Reserved for kernel (destroyed by some IRQ handlers!)
  R28        gp       Global pointer (rarely used)
  R29        sp       Stack pointer
  R30        fp(s8)   Frame Pointer, or 9th Static variable, must be saved
  R31        ra       Return address (used so by JAL,BLTZAL,BGEZAL opcodes)
  -          pc       Program counter
  -          hi,lo    Multiply/divide results, may be changed by subroutines
*/

typedef uint32_t (*r3000_bus_read32)(uint32_t addr, void* udata);
typedef uint32_t (*r3000_bus_read16)(uint32_t addr, void* udata);
typedef uint32_t (*r3000_bus_read8)(uint32_t addr, void* udata);
typedef void (*r3000_bus_write32)(uint32_t addr, uint32_t data, void* udata);
typedef void (*r3000_bus_write16)(uint32_t addr, uint32_t data, void* udata);
typedef void (*r3000_bus_write8)(uint32_t addr, uint32_t data, void* udata);
typedef int (*r3000_bus_query_access_cycles)(void* udata);

typedef struct {
    r3000_bus_read32 read32;
    r3000_bus_read16 read16;
    r3000_bus_read8 read8;
    r3000_bus_write32 write32;
    r3000_bus_write16 write16;
    r3000_bus_write8 write8;
    r3000_bus_query_access_cycles query_access_cycles;
    void* udata;
} r3000_bus;

struct __attribute__((__packed__)) r3000_state {
    uint32_t r[32];
    uint32_t opcode;
    uint32_t pc, next_pc, saved_pc;
    uint32_t hi, lo;
    uint32_t load_d, load_v;
    uint32_t last_cycles;
    uint32_t total_cycles;
    int branch, delay_slot, branch_taken, mmu_exception;

    r3000_bus_read32 bus_read32;
    r3000_bus_read16 bus_read16;
    r3000_bus_read8 bus_read8;
    r3000_bus_write32 bus_write32;
    r3000_bus_write16 bus_write16;
    r3000_bus_write8 bus_write8;
    r3000_bus_query_access_cycles bus_query_access_cycles;
    void* bus_udata;

    struct {
        uint32_t lo;
        uint32_t hi;
    } tlb[64];

    uint32_t cop0_r[16];
};

/*
  0     IEc Current Interrupt Enable  (0=Disable, 1=Enable) ;rfe pops IUp here
  1     KUc Current Kernel/User Mode  (0=Kernel, 1=User)    ;rfe pops KUp here
  2     IEp Previous Interrupt Disable                      ;rfe pops IUo here
  3     KUp Previous Kernel/User Mode                       ;rfe pops KUo here
  4     IEo Old Interrupt Disable                       ;left unchanged by rfe
  5     KUo Old Kernel/User Mode                        ;left unchanged by rfe
  6-7   -   Not used (zero)
  8-15  Im  8 bit interrupt mask fields. When set the corresponding
            interrupts are allowed to cause an exception.
  16    Isc Isolate Cache (0=No, 1=Isolate)
              When isolated, all load and store operations are targetted
              to the Data cache, and never the main memory.
              (Used by PSX Kernel, in combination with Port FFFE0130h)
  17    Swc Swapped cache mode (0=Normal, 1=Swapped)
              Instruction cache will act as Data cache and vice versa.
              Use only with Isc to access & invalidate Instr. cache entries.
              (Not used by PSX Kernel)
  18    PZ  When set cache parity bits are written as 0.
  19    CM  Shows the result of the last load operation with the D-cache
            isolated. It gets set if the cache really contained data
            for the addressed memory location.
  20    PE  Cache parity error (Does not cause exception)
  21    TS  TLB shutdown. Gets set if a programm address simultaneously
            matches 2 TLB entries.
            (initial value on reset allows to detect extended CPU version?)
  22    BEV Boot exception vectors in RAM/ROM (0=RAM/KSEG0, 1=ROM/KSEG1)
  23-24 -   Not used (zero)
  25    RE  Reverse endianness   (0=Normal endianness, 1=Reverse endianness)
              Reverses the byte order in which data is stored in
              memory. (lo-hi -> hi-lo)
              (Affects only user mode, not kernel mode) (?)
              (The bit doesn't exist in PSX ?)
  26-27 -   Not used (zero)
  28    CU0 COP0 Enable (0=Enable only in Kernel Mode, 1=Kernel and User Mode)
  29    CU1 COP1 Enable (0=Disable, 1=Enable)
  30    CU2 COP2 Enable (0=Disable, 1=Enable)
  31    CU3 COP3 Enable (0=Disable, 1=Enable)
*/

#define SR_IEC 0x00000001
#define SR_KUC 0x00000002
#define SR_IEP 0x00000004
#define SR_KUP 0x00000008
#define SR_IEO 0x00000010
#define SR_KUO 0x00000020
#define SR_IM  0x0000ff00
#define SR_IM0 0x00000100
#define SR_IM1 0x00000200
#define SR_IM2 0x00000400
#define SR_IM3 0x00000800
#define SR_IM4 0x00001000
#define SR_IM5 0x00002000
#define SR_IM6 0x00004000
#define SR_IM7 0x00008000
#define SR_ISC 0x00010000
#define SR_SWC 0x00020000
#define SR_PZ  0x00040000
#define SR_CM  0x00080000
#define SR_PE  0x00100000
#define SR_TS  0x00200000
#define SR_BEV 0x00400000
#define SR_RE  0x02000000
#define SR_CU0 0x10000000
#define SR_CU1 0x20000000
#define SR_CU2 0x40000000
#define SR_CU3 0x80000000

r3000_state* r3000_create();
void r3000_init(r3000_state*, r3000_bus*);
void r3000_destroy(r3000_state*);
void r3000_cycle(r3000_state*);
void r3000_exception(r3000_state*, uint32_t);
void r3000_set_irq_pending(r3000_state*);
uint32_t r3000_read32(r3000_state*, uint32_t);
uint32_t r3000_read16(r3000_state*, uint32_t);
uint32_t r3000_read8(r3000_state*, uint32_t);
void r3000_write32(r3000_state*, uint32_t, uint32_t);
void r3000_write16(r3000_state*, uint32_t, uint32_t);
void r3000_write8(r3000_state*, uint32_t, uint32_t);
int r3000_check_irq(r3000_state*);
void r3000_set_pc(r3000_state*, uint32_t);

/*
    00h INT     Interrupt
    01h MOD     TLB modification
    02h TLBL    TLB load
    03h TLBS    TLB store
    04h AdEL    Address error, Data load or Instruction fetch
    05h AdES    Address error, Data store
                The address errors occur when attempting to read
                outside of KUseg in user mode and when the address
                is misaligned. (See also: BadVaddr register)
    06h IBE     Bus error on Instruction fetch
    07h DBE     Bus error on Data load/store
    08h Syscall Generated unconditionally by syscall instruction
    09h BP      Breakpoint - break instruction
    0Ah RI      Reserved instruction
    0Bh CpU     Coprocessor unusable
    0Ch Ov      Arithmetic overflow
*/

#define CAUSE_INT       (0x00 << 2)
#define CAUSE_MOD       (0x01 << 2)
#define CAUSE_TLBL      (0x02 << 2)
#define CAUSE_TLBS      (0x03 << 2)
#define CAUSE_ADEL      (0x04 << 2)
#define CAUSE_ADES      (0x05 << 2)
#define CAUSE_IBE       (0x06 << 2)
#define CAUSE_DBE       (0x07 << 2)
#define CAUSE_SYSCALL   (0x08 << 2)
#define CAUSE_BP        (0x09 << 2)
#define CAUSE_RI        (0x0a << 2)
#define CAUSE_CPU       (0x0b << 2)
#define CAUSE_OV        (0x0c << 2)

#define TLBE_VPN    0xfffff000
#define TLBE_ASID   0x00000fc0
#define TLBE_PFN    0xfffff000
#define TLBE_N      0x00000800
#define TLBE_D      0x00000400
#define TLBE_V      0x00000200
#define TLBE_G      0x00000100
#define MMU_ENOENT  0xffffffff
#define MMU_SIGSEGV 0xffffffff

void r3000_i_invalid(r3000_state*);

// Primary
void r3000_i_special(r3000_state*);
void r3000_i_bxx(r3000_state*);
void r3000_i_j(r3000_state*);
void r3000_i_jal(r3000_state*);
void r3000_i_beq(r3000_state*);
void r3000_i_bne(r3000_state*);
void r3000_i_blez(r3000_state*);
void r3000_i_bgtz(r3000_state*);
void r3000_i_addi(r3000_state*);
void r3000_i_addiu(r3000_state*);
void r3000_i_slti(r3000_state*);
void r3000_i_sltiu(r3000_state*);
void r3000_i_andi(r3000_state*);
void r3000_i_ori(r3000_state*);
void r3000_i_xori(r3000_state*);
void r3000_i_lui(r3000_state*);
void r3000_i_cop0(r3000_state*);
void r3000_i_cop1(r3000_state*);
void r3000_i_cop2(r3000_state*);
void r3000_i_cop3(r3000_state*);
void r3000_i_lb(r3000_state*);
void r3000_i_lh(r3000_state*);
void r3000_i_lwl(r3000_state*);
void r3000_i_lw(r3000_state*);
void r3000_i_lbu(r3000_state*);
void r3000_i_lhu(r3000_state*);
void r3000_i_lwr(r3000_state*);
void r3000_i_sb(r3000_state*);
void r3000_i_sh(r3000_state*);
void r3000_i_swl(r3000_state*);
void r3000_i_sw(r3000_state*);
void r3000_i_swr(r3000_state*);
void r3000_i_lwc0(r3000_state*);
void r3000_i_lwc1(r3000_state*);
void r3000_i_lwc2(r3000_state*);
void r3000_i_lwc3(r3000_state*);
void r3000_i_swc0(r3000_state*);
void r3000_i_swc1(r3000_state*);
void r3000_i_swc2(r3000_state*);
void r3000_i_swc3(r3000_state*);

// Secondary
void r3000_i_sll(r3000_state*);
void r3000_i_srl(r3000_state*);
void r3000_i_sra(r3000_state*);
void r3000_i_sllv(r3000_state*);
void r3000_i_srlv(r3000_state*);
void r3000_i_srav(r3000_state*);
void r3000_i_jr(r3000_state*);
void r3000_i_jalr(r3000_state*);
void r3000_i_syscall(r3000_state*);
void r3000_i_break(r3000_state*);
void r3000_i_mfhi(r3000_state*);
void r3000_i_mthi(r3000_state*);
void r3000_i_mflo(r3000_state*);
void r3000_i_mtlo(r3000_state*);
void r3000_i_mult(r3000_state*);
void r3000_i_multu(r3000_state*);
void r3000_i_div(r3000_state*);
void r3000_i_divu(r3000_state*);
void r3000_i_add(r3000_state*);
void r3000_i_addu(r3000_state*);
void r3000_i_sub(r3000_state*);
void r3000_i_subu(r3000_state*);
void r3000_i_and(r3000_state*);
void r3000_i_or(r3000_state*);
void r3000_i_xor(r3000_state*);
void r3000_i_nor(r3000_state*);
void r3000_i_slt(r3000_state*);
void r3000_i_sltu(r3000_state*);

// BXX
void r3000_i_bltz(r3000_state*);
void r3000_i_bgez(r3000_state*);
void r3000_i_bltzal(r3000_state*);
void r3000_i_bgezal(r3000_state*);

// COP0
void r3000_i_mfc0(r3000_state*);
void r3000_i_mtc0(r3000_state*);
void r3000_i_rfe(r3000_state*);
void r3000_i_tlbp(r3000_state*);
void r3000_i_tlbr(r3000_state*);
void r3000_i_tlbwi(r3000_state*);
void r3000_i_tlbwr(r3000_state*);

// COP1
void r3000_i_mfc1(r3000_state*);
void r3000_i_cfc1(r3000_state*);
void r3000_i_mtc1(r3000_state*);
void r3000_i_ctc1(r3000_state*);
void r3000_i_fpu(r3000_state*);

typedef void (*r3000_instruction)(r3000_state*);

#endif