/*
 * int01
 *
 * BCM2835 ARM Peripherals
 * https://www.raspberrypi.org/documentation/hardware/raspberrypi/bcm2835/BCM2835-ARM-Peripherals.pdf
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

#define UART0_DR   ((volatile uint32_t *)(0x3F201000))
#define UART0_FR   ((volatile uint32_t *)(0x3F201018))
#define UART0_IMSC ((volatile uint32_t *)(0x3F201038))
#define UART0_MIS  ((volatile uint32_t *)(0x3F201040))

void uart_putc(unsigned char c)
{
    // Wait for UART to become ready to transmit.
    while (*UART0_FR & (1 << 5)) { }
    *UART0_DR = c;
}

void uart_puts(const char* str)
{
    for (size_t i = 0; str[i] != '\0'; i ++)
        uart_putc((unsigned char)str[i]);
}

#define IRQ_BASIC   ((volatile uint32_t *)(0x3F00B200))
#define IRQ_PEND2   ((volatile uint32_t *)(0x3F00B208))
#define IRQ_ENABLE2 ((volatile uint32_t *)(0x3F00B214))
#define GPU_INTERRUPTS_ROUTING ((volatile uint32_t *)(0x4000000C))
#define CORE0_INTERRUPT_SOURCE ((volatile uint32_t *)(0x40000060))

void c_irq_handler(void)
{
    char c;
    // check inteerupt source
    if (*CORE0_INTERRUPT_SOURCE & (1 << 8)) {
        if (*IRQ_BASIC & (1 << 9)) {
            if (*IRQ_PEND2 & (1 << 25)) {
                if (*UART0_MIS & (1 << 4)) {
                    c = (unsigned char) *UART0_DR; // read for clear tx interrupt.
                    uart_putc(c);
                    uart_puts(" c_irq_handler\n");
                    return;
                }
            }
        }
    }
    return;
}

void kernel_main(void)
{
    uart_puts("int01\n");

    // enable UART RX interrupt.
    *UART0_IMSC = 1 << 4;

    // UART interrupt routing.
    *IRQ_ENABLE2 = 1 << 25;

    // IRQ routeing to CORE0.
    *GPU_INTERRUPTS_ROUTING = 0x00;

    enable_irq();

    while (1) {
        io_halt();
    }
}
