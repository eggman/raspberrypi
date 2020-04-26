#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

static inline void io_halt(void)
{
    asm volatile ("wfi");
}

static inline void enable_irq(void)
{
    asm volatile ("msr daifclr, #2");
}

#define AUX_MU_IO   ((volatile uint32_t *)(0x3F215040))
#define AUX_MU_LSR  ((volatile uint32_t *)(0x3F215054))

void uart_putchar(uint8_t c)
{
    /* wait mini uart for tx idle. */
    while ( !(*AUX_MU_LSR & (1 << 5)) ) { }
    *AUX_MU_IO = c;
}

void uart_puts(const char* str)
{
    for (size_t i = 0; str[i] != '\0'; i ++)
        uart_putchar((uint8_t)str[i]);
}

void uart_puthex(uint64_t v)
{
    const char *hexdigits = "0123456789ABCDEF";
    for (int i = 28; i >= 0; i -= 4)
        uart_putchar(hexdigits[(v >> i) & 0xf]);
}

#define USB_BASE          0x3F980000
//CORE
#define USB_CORE_GAHBCFG       ((volatile uint32_t *)(0x8   + USB_BASE))
#define USB_CORE_GINTSTS       ((volatile uint32_t *)(0x14  + USB_BASE))
#define USB_CORE_GINTMSK       ((volatile uint32_t *)(0x18  + USB_BASE))
#define USB_CORE_GUID          ((volatile uint32_t *)(0x3C  + USB_BASE))
#define USB_CORE_GSNPSID       ((volatile uint32_t *)(0x40  + USB_BASE))
//HOST
#define USB_HOST_HCFG          ((volatile uint32_t *)(0x400 + USB_BASE))
#define USB_HOST_HPRT          ((volatile uint32_t *)(0x440 + USB_BASE))
#define USB_HOST_HCCHAR0       ((volatile uint32_t *)(0x500 + USB_BASE))
//INTC
#define IRQ_BASIC              ((volatile uint32_t *)(0x3F00B200))
#define IRQ_PEND1              ((volatile uint32_t *)(0x3F00B204))
#define IRQ_ENABLE1            ((volatile uint32_t *)(0x3F00B210))
#define GPU_INTERRUPTS_ROUTING ((volatile uint32_t *)(0x4000000C))
#define CORE0_INTERRUPT_SOURCE ((volatile uint32_t *)(0x40000060))

void c_irq_handler(void)
{
    /* check inteerupt source */
    if (    *CORE0_INTERRUPT_SOURCE & (1 << 8)
         && *IRQ_BASIC & (1 << 8)
         && *IRQ_PEND1 & (1 << 9)) {

        uint32_t reg = *USB_CORE_GINTSTS; 
        *USB_CORE_GINTSTS = reg;  //clear
        uart_puthex(reg);
        uart_puts(" usb interrupt\n");
        if (reg & 1 << 28) {
            uart_puts("Connector ID Status Change\n");
        }
    }
    return;
}

void usbhost_id(void)
{
    uart_puts("USB_CORE_GUID    ");
    uart_puthex(*USB_CORE_GUID);
    uart_putchar('\n');

    uart_puts("USB_CORE_GSNPSID ");
    uart_puthex(*USB_CORE_GSNPSID);
    uart_putchar('\n');
}

void usbhost_start(void)
{
    // Host Power on
    // HPRT.PrtPwr = 1'b1
    // HPRT.PrtRst = 1'b1
    // wait 60msec
    // HPRT.PrtRst = 1'b0
    uart_puts("HPRT ");
    uart_puthex(*USB_HOST_HPRT);
    uart_putchar('\n');
    *USB_HOST_HPRT |= 0 << 8;
    *USB_HOST_HPRT |= 1 << 12;
    *USB_HOST_HPRT |= 1 << 8;
    uint32_t count = 0;
    count = 0;
    do {
        ;
    } while (count++ >= 0x100000) ;
    *USB_HOST_HPRT |= 0 << 8;
    *USB_HOST_HPRT |= 1 << 2;

    //enable irq
    *USB_CORE_GAHBCFG   |= 1;
    *USB_CORE_GINTMSK    =  1 << 28 | 0 << 24 | 0 << 3;

    // port enable and retry detect
    *USB_HOST_HPRT = 1 << 3| 1 << 1; 

    // setup channel
    *USB_HOST_HCCHAR0   |= 1<<31;
}

void intc_setup(void)
{
    // IRQ routeing to CORE0.
    *GPU_INTERRUPTS_ROUTING = 0x00;

    // USB interrupt routing.
    *IRQ_ENABLE1 = 1 << 9;

    enable_irq();
}

void kernel_main(void)
{
    uart_puts("usb01\n");

    intc_setup();

    usbhost_id();
    usbhost_start();

    while (1) {
        io_halt();
    }
}
