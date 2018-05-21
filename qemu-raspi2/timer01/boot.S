// To keep this in the first portion of the binary.
.section ".text.boot"
 
// Make _start global.
.globl _start
 
_start:
reset:
    // in QEMU all of 4 ARM CPUs are started simultaniously
    // by default. I don't know if this is the real hw behaviour,
    // but here I jump to halt if CPU ID (stored in MPIDR
    // register, first 2 bits) is not 0
    mrc p15, #0, r1, c0, c0, #5
    and r1, r1, #3
    cmp r1, #0
    bne hang 

    // set vector address.
    ldr r0, =vector
    mcR P15, 0, r0, c12, c0, 0

    // save CPSR.
    mrs r0, cpsr

    // set sp in irq mode.
    bic r1, r0, #0x1F
    orr r1, r1, #0x12
    msr cpsr_c,r1
    mov sp, #0x4000

    // set sp in svc mode.
    msr cpsr_c, r0
    mov sp, #0x8000

    // Clear bss.
    ldr r4, =__bss_start
    ldr r9, =__bss_end
    mov r5, #0
    mov r6, #0
    mov r7, #0
    mov r8, #0
    b       2f
 
1:
    // store multiple at r4.
    stmia r4!, {r5-r8}
 
    // If we are still below bss_end, loop.
2:
    cmp r4, r9
    blo 1b
 
    // Call kernel_main
    ldr r3, =kernel_main
    blx r3

hang:
    wfi
    b hang

.globl io_halt
io_halt:
    wfi
    bx lr

.globl enable_irq
enable_irq:
    cpsie i
    bx lr

.globl disable_irq
disable_irq:
    cpsid i
    bx lr

irq:
    push {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}
    bl c_irq_handler
    pop  {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}
    subs pc,lr,#4

.balign 32
vector:
    ldr pc, reset_handler
    ldr pc, undefined_handler
    ldr pc, swi_handler
    ldr pc, prefetch_handler
    ldr pc, data_handler
    ldr pc, unused_handler
    ldr pc, irq_handler
    ldr pc, fiq_handler
reset_handler:      .word reset
undefined_handler:  .word hang
swi_handler:        .word hang
prefetch_handler:   .word hang
data_handler:       .word hang
unused_handler:     .word hang
irq_handler:        .word irq
fiq_handler:        .word hang


