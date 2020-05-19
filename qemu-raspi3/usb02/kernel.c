#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

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

//INTC
#define IRQ_BASIC              ((volatile uint32_t *)(0x3F00B200))
#define IRQ_PEND1              ((volatile uint32_t *)(0x3F00B204))
#define IRQ_ENABLE1            ((volatile uint32_t *)(0x3F00B210))
#define GPU_INTERRUPTS_ROUTING ((volatile uint32_t *)(0x4000000C))
#define CORE0_INTERRUPT_SOURCE ((volatile uint32_t *)(0x40000060))

#define USB_BASE          0x3F980000
//CORE
#define USB_CORE_GAHBCFG       ((volatile uint32_t *)(0x8   + USB_BASE))
#define USB_CORE_GINTSTS       ((volatile uint32_t *)(0x14  + USB_BASE))
#define USB_CORE_GINTMSK       ((volatile uint32_t *)(0x18  + USB_BASE))
#define USB_CORE_GUID          ((volatile uint32_t *)(0x3C  + USB_BASE))
#define USB_CORE_GSNPSID       ((volatile uint32_t *)(0x40  + USB_BASE))
//HOST
#define USB_HOST_HCFG          ((volatile uint32_t *)(0x400 + USB_BASE))
#define USB_HOST_HAINTMSK      ((volatile uint32_t *)(0x418 + USB_BASE))
#define USB_HOST_HPRT          ((volatile uint32_t *)(0x440 + USB_BASE))
//CHANNEL
#define USB_HOST_HCCHAR0       ((volatile uint32_t *)(0x500 + USB_BASE))
#define USB_HOST_HCINTMSK0     ((volatile uint32_t *)(0x50C + USB_BASE))
#define USB_HOST_HCTSIZ0       ((volatile uint32_t *)(0x510 + USB_BASE))
#define USB_HOST_HCDMA0        ((volatile uint32_t *)(0x514 + USB_BASE))
#define USB_HOST_HCCHAR1       ((volatile uint32_t *)(0x520 + USB_BASE))
#define USB_HOST_HCINTMSK1     ((volatile uint32_t *)(0x52C + USB_BASE))
#define USB_HOST_HCTSIZ1       ((volatile uint32_t *)(0x530 + USB_BASE))
#define USB_HOST_HCDMA1        ((volatile uint32_t *)(0x534 + USB_BASE))

uint8_t usb_buffer0[1024*16];
uint8_t usb_buffer1[1024*16];

struct UsbDeviceRequest {
    uint8_t Type;
    uint8_t Request;
    uint16_t Value;
    uint16_t Index;
    uint16_t Length;
} __attribute__((__packed__));

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
        if (reg & 1 << 28) { uart_puts("Connector ID Status Change\n"); }
        if (reg & 1 << 27) { uart_puts("LPM Transaction Received Interrupt\n"); }
        if (reg & 1 << 26) { uart_puts("Periodic TxFIFO Empty\n"); }
        if (reg & 1 << 25) { uart_puts("Host Channels Interrupt\n"); 
                           }
        if (reg & 1 << 24) { *USB_HOST_HPRT |= 1 << 1;  //clear host port interrupt
                             uart_puts("Host Port Interrupt\n"); }
        if (reg & 1 <<  6) { uart_puts("Global IN Non-periodic NAK Effective\n"); }
        if (reg & 1 <<  5) { uart_puts("Non-periodic TxFIFO Empty\n"); }
        if (reg & 1 <<  4) { uart_puts("Host and Device RxFIFO Non-Empty (RxFLvl) \n"); }
        if (reg & 1 <<  3) { uart_puts("Host and Device Start of Frame\n"); }
        if (reg & 1 <<  2) { uart_puts("OTG Interrupt\n"); }
        if (reg & 1 <<  1) { uart_puts("Mode Mismatch Interrupt\n"); }
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
    uint32_t count = 0;

    // USB Host power on
    // HPRT.PrtPwr = 1'b1 -> HPRT.PrtRst = 1'b1 -> wait 60msec -> HPRT.PrtRst = 1'b0
    *USB_HOST_HPRT |= 1 << 12;
    *USB_HOST_HPRT |= 1 << 8;
    count = 0;
    do {
        ;
    } while (count++ >= 0x100000) ;
    *USB_HOST_HPRT &= ~(1 << 8);

    // enable irq
    // GAHBCFG.GlblIntrMsk = 1'b1
    // GINTMSK.ConIDStsChngMsk = 1'b1, GINTMSK.PrtIntMsk = 1'b1, GINTMSK.SofMsk = 1'b1
    *USB_CORE_GAHBCFG   |= 1;
    *USB_CORE_GINTMSK    =  1 << 28 | 1 << 24 | 0 << 3;

    // port enable and retry detect
    // HPRT.PrtPwr = 1'b1, HPRT.PrtEnChng = 1'b1, HPRT.PrtConnDet = 1'b1
    *USB_HOST_HPRT = 1 << 12 | 1 << 3 | 1 << 1; 

    // enable channel irq
    // HAINTMASK.HAINTMsk = 16'h3
    // HCINTMSK0.XferComplMsk = 1'b1
    *USB_HOST_HAINTMSK   |= 0x3;
    *USB_HOST_HCINTMSK0  |= 1;
    *USB_HOST_HCINTMSK1  |= 1;

    // HCCAR1.EPDir = 1'b0 (OUT) / 1'b01(IN), HCCAR1.MPS = 11'h40
    *USB_HOST_HCCHAR0   |= 0x40;            // OUT
    *USB_HOST_HCCHAR1   |= 1 << 15 | 0x40;  // IN

    // build packet
    memcpy(usb_buffer0, &(struct UsbDeviceRequest) {
                                .Type = 0x80,     // DEVICE_TO_HOST | STDANDAD | DEVICE
                                .Request = 0x06,  // GET_DESCRIPTOR
                                .Value = 0x0100,  // descriptor.type = 0x01, decriptor.index = 0x00
                                .Index = 0,
                                .Length = 64,
                      }, 8);

    // send setup & control packet
     // set dma buffer
    *USB_HOST_HCDMA0   = usb_buffer0;
    *USB_HOST_HCDMA0  |= 0xC0000000;
     // HCTSIZ0.Pid = 2'd3 (SETUP) , HCTSIZ0.PktCnt = 10'h1 , HCTSIZ0.XferSize = 18'd8
    *USB_HOST_HCTSIZ0 = 3 << 29 | 1 << 19 | 8;
     // HCCAR1.ChEna = 1'b1
    *USB_HOST_HCCHAR0 |= 1<<31;

    // recieve control packet
     // set dma buffer
    *USB_HOST_HCDMA1   = usb_buffer1;
    *USB_HOST_HCDMA1  |= 0xC0000000;
     // HCTSIZ1.Pid = 2'd2 (DATA1) , HCTSIZ1.PktCnt = 10'h1 , HCTSIZ1.XferSize = 18'd64
    *USB_HOST_HCTSIZ1  = 2 << 29 | 1 << 19 | 64;
     // HCCAR1.ChEna = 1'b1
    *USB_HOST_HCCHAR1 |= 1<<31;
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
    uart_puts("usb02\n");
    uart_puts("usb_buffer0:");
    uart_puthex(usb_buffer0);
    uart_puts("\n");
    uart_puts("usb_buffer1:");
    uart_puthex(usb_buffer1);
    uart_puts("\n");

    intc_setup();

    usbhost_id();
    usbhost_start();

    uart_puts("GET_DESCRIPTOR\n");
    for (int i = 0; i < 18; i++) {
        uart_puthex(usb_buffer1[i]);
        uart_puts("\n");
    }

    while (1) {
        io_halt();
    }
}
