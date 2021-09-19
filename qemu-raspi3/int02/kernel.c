/*
 * int02 : mini uart rx interrupt
 *
 * BCM2835 ARM Peripherals
 * https://www.raspberrypi.org/documentation/hardware/raspberrypi/bcm2835/BCM2835-ARM-Peripherals.pdf
 *
 * BCM2835 datasheet errata
 * https://elinux.org/BCM2835_datasheet_errata
 *
 * BCM2836 QA7 ARM Quad A7 core
 * https://www.raspberrypi.org/documentation/hardware/raspberrypi/bcm2836/QA7_rev3.4.pdf
 */

#include <stddef.h>
#include <stdint.h>

extern void enable_irq(void);
extern void disable_irq(void);

static inline void io_halt(void)
{
    asm volatile ("wfi");
}

#define AUX_ENABLES ((volatile uint32_t *)(0x3F215004))
#define AUX_MU_IO   ((volatile uint32_t *)(0x3F215040))
#define AUX_MU_IER  ((volatile uint32_t *)(0x3F215044))
#define AUX_MU_IIR  ((volatile uint32_t *)(0x3F215048))
#define AUX_MU_LSR  ((volatile uint32_t *)(0x3F215054))

void uart_putc(unsigned char c)
{
    /* Wait for UART to become ready to transmit. */
    while ( !(*AUX_MU_LSR & (1 << 5)) ) { }
    *AUX_MU_IO = c;
}

void uart_puts(const char* str)
{
    for (size_t i = 0; str[i] != '\0'; i ++)
        uart_putc((unsigned char)str[i]);
}

#define IRQ_BASIC   ((volatile uint32_t *)(0x3F00B200))
#define IRQ_PEND1   ((volatile uint32_t *)(0x3F00B204))
#define IRQ_ENABLE1 ((volatile uint32_t *)(0x3F00B210))
#define GPU_INTERRUPTS_ROUTING ((volatile uint32_t *)(0x4000000C))
#define CORE0_INTERRUPT_SOURCE ((volatile uint32_t *)(0x40000060))

void c_irq_handler(void)
{
    char c;
    /* check inteerupt source */
    if (*CORE0_INTERRUPT_SOURCE & (1 << 8)) {
        if (*IRQ_BASIC & (1 << 8)) {
            if (*IRQ_PEND1 & (1 << 29)) {
                c = *AUX_MU_IO;  /* read for clear tx interrupt. */
                uart_putc(c);
                uart_puts(" c_irq_handler\n");
            }
        }
    }
    return;
}

void kernel_main(void)
{
    uart_puts("qemu exit: Ctrl-A x / qemu monitor: Ctrl-A c\n");
    uart_puts("int02\n");

    /* enable UART RX interrupt. */
    *AUX_ENABLES = 1;
    //*AUX_MU_IIR = 4;
    *AUX_MU_IER = 1;

    /* UART interrupt routing. */
    *IRQ_ENABLE1 = 1 << 29;

    /* IRQ routeing to CORE0. */
    *GPU_INTERRUPTS_ROUTING = 0x00;

    enable_irq();

    while (1) {
        io_halt();
    }
}
