#include <stddef.h>
#include <stdint.h>

extern void io_halt(void);
extern void enable_irq(void);
extern void disable_irq(void);

#define DR   (*(volatile unsigned int *)0x3F201000)
#define FR   (*(volatile unsigned int *)0x3F201018)
#define IMSC (*(volatile unsigned int *)0x3F201038)

void uart_putc(char ch)
{
	while (FR & (1U << 5));
	DR = (unsigned int)ch;
}

void uart_puts(char *str)
{
	while (*str != '\0')
		uart_putc(*str++);
}

#define IRQ_PEND2    (*(volatile unsigned int *) 0x3F00B208)
#define IRQ_ENABLE2  (*(volatile unsigned int *) 0x3F00B214)
#define IRQ_DISABLE2 (*(volatile unsigned int *) 0x3F00B220)
#define GPU_INTERRUPTS_ROUTING (*(volatile unsigned int *) 0x4000000C)

void kernel_main(void)
{
    // enable UART TX interrupt.
    IMSC = 0x010;

    // enable UART interrupt.
    IRQ_ENABLE2 = 1 << 25;

    // IRQ routeing to CORE3.
    GPU_INTERRUPTS_ROUTING = 0x03;

    // enable IRQ
    enable_irq();

	uart_puts("kernel_main\n");
    while (1)
        io_halt();
}

void c_irq_handler(void)
{
    disable_irq();
    if (IRQ_PEND2 & (1 << 25)) {
        IRQ_DISABLE2 = 1 << 25;

	    uart_putc((char) DR); // read for clear tx interrupt.
	    uart_puts(" c_irq_handler\n");

        IRQ_ENABLE2 = 1 << 25;
    }
    enable_irq();
    return;
}
