// To keep this in the first portion of the binary.
.section ".text.boot"
 
// Make _start global.
.globl _start
 
_start:
    // in QEMU all of 4 ARM CPUs are started simultaniously
    // by default. I don't know if this is the real hw behaviour,
    // but here I jump to halt if CPU ID (stored in MPIDR
    // register, first 2 bits) is not 0
    mrs   x1, mpidr_el1
    and   x1, x1, #3
    cmp   x1, #0
    bne   hang 

    // address for stack pointer
    ldr   x1, =_start

    // drop to EL2
    mov   x2, #0x5b1    // RW=1, HCE=1, SMD=1, RES=1, NS=1
    msr   scr_el3, x2
    mov   x2, #0x3c9    // D=1, A=1, I=1, F=1 M=EL2h
    msr   spsr_el3, x2
    adr   x2, start_el2
    msr   elr_el3, x2
    eret

start_el2:
    // set sp in EL1
    msr   sp_el1, x1
    // enable AArch64 in EL1
    mov   x0, #(1 << 31)      // AArch64
    orr   x0, x0, #(1 << 1)   // SWIO hardwired on Pi3
    msr   hcr_el2, x0
    mrs   x0, hcr_el2
    // change execution level to EL1
    mov   x2, #0x3c4         // D=1, A=1, I=1, F=1 M=EL1t
    msr   spsr_el2, x2
    adr   x2, start_el1
    msr   elr_el2, x2
    eret

start_el1:
    // set sp
    mov   sp, x1
    // Clear bss.
    ldr   x1, =__bss_start
    ldr   w2, =__bss_size
1:  cbz   w2, 2f
    str   xzr, [x1], #8
    sub   w2, w2, #1
    cbnz  w2, 1b

2:  bl kernel_main

hang:
    wfi
    b     hang

