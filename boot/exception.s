#include "def.h"

.org 0x00000180

.equ SYS_KMEM_BASE, 0x80000000
.equ SYS_KMEM_END, 0x80010000
.equ SYS_IRQ_HANDLER_PTR, 0x80000000
.equ SYS_EXC_HANDLER_PTR, 0x80000004
.equ SYS_XCONTEXT_BASE, 0x80001000

exception_handler:
    # xsave
    la      $k0, SYS_XCONTEXT_BASE
    sw      $t0, 0($k0)
    sw      $t1, 4($k0)
    sw      $t2, 8($k0)
    sw      $t3, 12($k0)
    sw      $t4, 16($k0)
    sw      $t5, 20($k0)
    sw      $t6, 24($k0)
    sw      $t7, 28($k0)
    sw      $t8, 32($k0)
    sw      $t9, 36($k0)
    sw      $at, 40($k0)

    # Get Cause register
    mfc0    $t0, $13

    # Get excode
    srl     $t0, 2
    andi    $t0, 0x1f

    # 0: INT
    beqz    $t0, irq_handler
    nop

    # 8: SYS
    li      $t1, 8
    beq     $t0, $t1, syscall_handler
    nop

    # Unhandled exception (crash kernel)
    la      $t1, unhandled_exception_msg
    jal     kputs
    nop
    move    $k0, $a0
    j       halt
    nop

syscall_handler:
    la      $t0, SYS_IRQ_HANDLER_PTR
    sw      $a0, 0($t0)
    la      $k0, SYS_XCONTEXT_BASE
    lw      $t0, 0($k0)
    lw      $t1, 4($k0)
    lw      $t2, 8($k0)
    lw      $t3, 12($k0)
    lw      $t4, 16($k0)
    lw      $t5, 20($k0)
    lw      $t6, 24($k0)
    lw      $t7, 28($k0)
    lw      $t8, 32($k0)
    lw      $t9, 36($k0)
    lw      $at, 40($k0)

    # Jump to EPC (+4 to skip syscall instr)
    mfc0    $k0, $14
    addi    $k0, 4
    jr      $k0
    rfe

irq_handler:
    lui     $t0, 0x8000
    lw      $t0, 0($t0)
    jalr    $t0
    nop
    la      $k0, SYS_XCONTEXT_BASE
    lw      $t0, 0($k0)
    lw      $t1, 4($k0)
    lw      $t2, 8($k0)
    lw      $t3, 12($k0)
    lw      $t4, 16($k0)
    lw      $t5, 20($k0)
    lw      $t6, 24($k0)
    lw      $t7, 28($k0)
    lw      $t8, 32($k0)
    lw      $t9, 36($k0)
    lw      $at, 40($k0)

    # Jump to EPC (+4 to skip segfault instr)
    mfc0    $k0, $14
    jr      $k0
    rfe

kputs:
    la      $k0, 0x9f900000

0:
    lb      $t2, 0($t1)
    beqz    $t2, 1f
    sb      $t2, 0($k0)
    addi    $t1, 1
    b       0b

1:
    jr      $ra
    nop

halt:
    la      $t1, system_halted_msg
    jal     kputs
    nop

0:
    b       0b
    nop

unhandled_exception_msg:
    .asciiz "kernel: Unhandled exception! Check $k0 for exception cause.\n"

system_halted_msg:
    .asciiz "kernel: System halted\n"
