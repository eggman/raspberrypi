/*
 * timer01 : simple generic timer
 */
#include <stddef.h>
#include <stdint.h>

extern void enable_irq(void);
extern void disable_irq(void);

// Memory-Mapped I/O output
static inline void mmio_write(intptr_t reg, uint32_t data)
{
    *(volatile uint32_t*) reg = data;
}
 
// Memory-Mapped I/O input
static inline uint32_t mmio_read(intptr_t reg)
{
    return *(volatile uint32_t*) reg;
}

static inline void io_halt(void)
{
    asm volatile ("wfi");
}

#define UART0_DR   0x3F201000
#define UART0_FR   0x3F201018
#define UART0_IMSC 0x3F201038

void uart_putc(unsigned char c)
{
    // Wait for UART to become ready to transmit.
    while (mmio_read(UART0_FR) & (1 << 5)) { }
    mmio_write(UART0_DR, c);
}
 
void uart_puts(const char* str)
{
    for (size_t i = 0; str[i] != '\0'; i ++)
        uart_putc((unsigned char)str[i]);
}

void uart_hex_puts(uint64_t num)
{
    int n = 0;
    uint64_t base = 16;
    uint64_t d = 1;
    char buf[32], *bf;

    bf = buf;
    *bf++ = '0';
    *bf++ = 'x';

    while ( num / d >= base)
        d *= base;

    while (d != 0) {
        int dgt = num / d;
        num %= d;
        d /= base;
        if (n || dgt > 0 || d == 0) {
            *bf++ = dgt + (dgt < 10 ? '0' : ('A') - 10);
            ++n;
        }
    }
    *bf++ = '\n';
    *bf = 0;
    
    uart_puts(buf);
}

#define CORE0_TIMER_IRQCNTL 0x40000040
#define CORE0_IRQ_SOURCE    0x40000060

void routing_core0cntv_to_core0irq(void)
{
    mmio_write(CORE0_TIMER_IRQCNTL, 0x08);
}

uint32_t read_core0timer_pending(void)
{
    uint32_t tmp;
    tmp = mmio_read(CORE0_IRQ_SOURCE);
    return tmp;
}

static uint32_t cntfrq = 0;

void enable_cntv(void)
{
    uint32_t cntv_ctl;
    cntv_ctl = 1;
	asm volatile ("msr cntv_ctl_el0, %0" :: "r" (cntv_ctl));
}

void disable_cntv(void)
{
    uint32_t cntv_ctl;
    cntv_ctl = 0;
	asm volatile ("msr cntv_ctl_el0, %0" :: "r" (cntv_ctl));
}

uint64_t read_cntvct(void)
{
	uint64_t val;
	asm volatile("mrs %0, cntvct_el0" : "=r" (val));
	return (val);
}

uint32_t read_cntv_tval(void)
{
    uint32_t val;
	asm volatile ("mrs %0, cntv_tval_el0" : "=r" (val));
    return val;
}

void write_cntv_tval(uint32_t val)
{
	asm volatile ("msr cntv_tval_el0, %0" :: "r" (val));
    return;
}

uint32_t read_cntfrq(void)
{
    uint32_t val;
	asm volatile ("mrs %0, cntfrq_el0" : "=r" (val));
    return val;
}

void c_irq_handler(void)
{
    uint32_t cntvct;
    uint32_t val;

    disable_irq();
    if (read_core0timer_pending() & 0x08 ) {
        uart_puts("handler CNTV_TVAL: ");
        val = read_cntv_tval();
        uart_hex_puts(val);

        write_cntv_tval(cntfrq);    // clear cntv interrupt and set next 1sec timer.

        uart_puts("handler CNTVCT   : ");
        cntvct = read_cntvct();
        uart_hex_puts(cntvct);
    }
    enable_irq();
    return;
}

void kernel_main(void)
{
    uint32_t val;

    uart_puts("qemu exit: Ctrl-A x / qemu monitor: Ctrl-A c\n");
    uart_puts("timer01\n");

    uart_puts("CNTFRQ  : ");
    cntfrq = read_cntfrq();
    uart_hex_puts(cntfrq);

    write_cntv_tval(cntfrq);    // clear cntv interrupt and set next 1 sec timer.
    uart_puts("CNTV_TVAL: ");
    val = read_cntv_tval();
    uart_hex_puts(val);
 
    routing_core0cntv_to_core0irq();
    enable_cntv(); 
    enable_irq();

    while (1) {
        io_halt();
    }
}
