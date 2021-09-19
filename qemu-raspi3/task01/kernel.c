/*
 * task01 : simple start another task.
 */
#include <stddef.h>
#include <stdint.h>

extern void enable_irq(void);
extern void disable_irq(void);

extern void switch_task(uint64_t *next_task_top_of_stack);
void task_b(void);

uint64_t *task_b_top_of_stack;
int32_t  switch_flg = 0;

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

void uart_puthex(uint64_t n)
{
    const char *hexdigits = "0123456789ABCDEF";
    for (int i = 60; i >= 0; i -= 4)
        uart_putc(hexdigits[(n >> i) & 0xf]);
}

void uart_dump(void *ptr)
{
    uint64_t a,b,d;
    uint8_t c;
    for ( a =(uint64_t) ptr; a < (uint64_t) ptr + 256 + 16; a += 16) {
        uart_puthex(a); uart_puts(": ");
        for(b = 0; b < 16; b++) {
            c = *((uint8_t *)(a + b));
            d = (uint64_t) c; d >>= 4; d &= 0xF; d += d > 9 ? 0x37: 0x30; uart_putc(d);
            d = (uint64_t) c; d &= 0xF; d += d > 9 ? 0x37 : 0x30; uart_putc(d);
            uart_putc(' ');
            if(b % 4 == 3)
                uart_putc(' ');
        }
        for(b = 0; b < 16; b++) {
            c = *((uint8_t *)(a + b));
            uart_putc(c < 32 || c >= 127 ? '.' : c);
        }
        uart_putc('\n');
    }
}

#define IRQ_PEND2   ((volatile uint32_t *)(0x3F00B208))
#define IRQ_ENABLE2 ((volatile uint32_t *)(0x3F00B214))
#define GPU_INTERRUPTS_ROUTING ((volatile uint32_t *)(0x4000000C))
#define CORE0_INTERRUPT_SOURCE ((volatile uint32_t *)(0x40000060))

uint64_t get_sp(void)
{
    uint64_t sp;
    asm volatile ("mov %0, sp" : "=r" (sp));
    return sp;
}

void c_irq_handler(void)
{
    char c;

    disable_irq();
    // check inteerupt source
    if (*CORE0_INTERRUPT_SOURCE & (1 << 8)) {
        if (*IRQ_PEND2 & (1 << 25)) {
            if (*UART0_MIS & (1 << 4)) { 
                c = (unsigned char) *UART0_DR; // read for clear tx interrupt.
                enable_irq();
                uart_putc(c);
                uart_puts(" c_irq_handler\n");
                uart_puthex(get_sp()); uart_putc('\n');
                switch_flg = 1; // set task_b start flag
                return;
            }
        }
    }
    enable_irq();
    return;
}

uint64_t *setup_task_b_stack(void)
{
    uint64_t *p =  (uint64_t *) 0x40000;

    uart_puthex((uint64_t) p);
    uart_putc('\n');

    /* prepare data in stack */
    p--;
    *p = 0x0101010101010101ULL; /* x1 */
    p--;
    *p = 0x00000000000000000LL; /* x0 */
    p--;
    *p = 0x0303030303030303ULL; /* x3 */
    p--;
    *p = 0x0202020202020202ULL; /* x2 */
    p--;
    *p = 0x0505050505050505ULL; /* x5 */
    p--;
    *p = 0x0404040404040404ULL; /* x4 */
    p--;
    *p = 0x0707070707070707ULL; /* x7 */
    p--;
    *p = 0x0606060606060606ULL; /* x6 */
    p--;
    *p = 0x0909090909090909ULL; /* x9 */
    p--;
    *p = 0x0808080808080808ULL; /* x8 */
    p--;
    *p = 0x1111111111111111ULL; /* x11 */
    p--;
    *p = 0x1010101010101010ULL; /* x10 */
    p--;
    *p = 0x1313131313131313ULL; /* x13 */
    p--;
    *p = 0x1212121212121212ULL; /* x12 */
    p--;
    *p = 0x1515151515151515ULL; /* x15 */
    p--;
    *p = 0x1414141414141414ULL; /* x14 */
    p--;
    *p = 0x1717171717171717ULL; /* x17 */
    p--;
    *p = 0x1616161616161616ULL; /* x16 */
    p--;
    *p = 0x1919191919191919ULL; /* x19 */
    p--;
    *p = 0x1818181818181818ULL; /* x18 */
    p--;
    *p = 0x2121212121212121ULL; /* x21 */
    p--;
    *p = 0x2020202020202020ULL; /* x20 */
    p--;
    *p = 0x2323232323232323ULL; /* x23 */
    p--;
    *p = 0x2222222222222222ULL; /* x22 */
    p--;
    *p = 0x2525252525252525ULL; /* x25 */
    p--;
    *p = 0x2424242424242424ULL; /* x24 */
    p--;
    *p = 0x2727272727272727ULL; /* x27 */
    p--;
    *p = 0x2626262626262626ULL; /* x26 */
    p--;
    *p = 0x2929292929292929ULL; /* x29 */
    p--;
    *p = 0x2828282828282828ULL; /* x28 */
    p--;
    *p = ( uint64_t ) 0x00;     /* xzr - has no effect, used so there are an even number of registers. */
    p--;
    *p = ( uint64_t ) task_b;   /* x30 - procedure call link register. */ 

    uart_dump(p);
    uart_puthex((uint64_t) p);
    uart_putc('\n');
    return p;
}


void task_b(void)
{
    uart_puts("task B ");
    uart_puthex(get_sp()); uart_putc('\n');
    switch_flg = 0;
    for (;;) {
        io_halt();
    }
}


void kernel_main(void)
{
    uint64_t *task_b_top_of_stack;

    uart_puts("qemu exit: Ctrl-A x / qemu monitor: Ctrl-A c\n");
    uart_puts("task01\n");

    // enable UART RX interrupt.
    *UART0_IMSC = 1 << 4;

    // UART interrupt routing.
    *IRQ_ENABLE2 = 1 << 25;

    // IRQ routeing to CORE0.
    *GPU_INTERRUPTS_ROUTING = 0x00;

    task_b_top_of_stack = setup_task_b_stack();

    enable_irq();

    uart_puts("task A ");
    uart_puthex(get_sp()); uart_putc('\n');
    while (1) {
        if(switch_flg) {
            switch_task(task_b_top_of_stack);
        } else {
            io_halt();
        }
    }
}
