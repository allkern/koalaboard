.org 0xbfc00000

reset:
    lui     $v0, 0x3040
    mtc0    $v0, $12

.org 0xbfc00100

exc_handler:
    mfc0    $k0, $13
    andi    $k0, 0x7c
    addiu   $k1, $0, 0x40
    beq     $k0, $k1, syscall_handler
    nop
    beq     $k0, $0, irq_handler
    nop

.L0:
    b       L0
    nop
    nop

syscall_handler:
    addiu   $k0, $0, 0x7f
    beq     $t8, $k0, sys_set_vblank_handler
    nop

.L1:
    b       L1
    nop
    nop

sys_set_vblank_handler:
    lui     $k0, 0xa000
    sw      $t9, +0($k0)
    rfe
    nop
