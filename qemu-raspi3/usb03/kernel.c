/*
 * usb03.c
 */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

static void *memset(void *s, int c, size_t n)
{
    const uint8_t uc = c % 0xff;
    uint8_t  *p = (uint8_t *)s;

    while (n-- > 0)
        *p++ = uc;

    return (s);
}

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

void vprintf(const char* fmt, va_list args)
{
    bool zeropadding = false;
    int64_t num = 0;
    char tmpstr[19] = {0};
    char *p;
    int len = 0;
    
    if (fmt == NULL) { return; }

    while (*fmt) {
        if ( *fmt == '%' ) {
            fmt++;

            if ( '0' == *fmt ) {
                zeropadding = true;
                fmt++;
            }
         
            while ( '0' <= *fmt && *fmt <= '9' ) {
                len = len*10 + (*fmt - '0');
                fmt++;
            }

            if ( *fmt == 'd') {
                int i = 18;
                tmpstr[i]=0;

                num = va_arg(args, int);
                tmpstr[i]=0;
                do {
                    tmpstr[--i] = '0'+ ( num % 10 );
                    num/=10;
                } while ( num != 0 && i > 0);
 
                p = &tmpstr[i];
                goto putstring;
            }
            else if ( *fmt == 'x') {
                int i = 16;
                tmpstr[i]=0;
                num = va_arg(args, int);

                do {
                    char n = num & 0xf;
                    tmpstr[--i] = n + ( n > 9 ? 'a' - 10 : '0');
                    num>>=4;
                } while ( num != 0 && i > 0 );

                if ( zeropadding && len > 0 && len <= 16) {
                    while( i > 16 - len && i > 0) {
                        tmpstr[--i] = '0';
                    }
                }

                p = &tmpstr[i];
putstring:
                while(*p) {
                    uart_putchar(*p++);
                }
            }
        } else {
            uart_putchar(*fmt);
        }
        fmt++;
    }
    return;
}

void uart_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}


#define IRQ_BASIC              ((volatile uint32_t *)(0x3F00B200))
#define IRQ_PEND1              ((volatile uint32_t *)(0x3F00B204))
#define IRQ_ENABLE1            ((volatile uint32_t *)(0x3F00B210))
#define GPU_INTERRUPTS_ROUTING ((volatile uint32_t *)(0x4000000C))
#define CORE0_INTERRUPT_SOURCE ((volatile uint32_t *)(0x40000060))

void intc_setup(void)
{
    // IRQ routeing to CORE0.
    *GPU_INTERRUPTS_ROUTING = 0x00;

    // USB interrupt routing.
    *IRQ_ENABLE1 = 1 << 9;

    enable_irq();
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

#define USB_HOST_HCCHAR2       ((volatile uint32_t *)(0x540 + USB_BASE))
#define USB_HOST_HCINTMSK2     ((volatile uint32_t *)(0x54C + USB_BASE))
#define USB_HOST_HCTSIZ2       ((volatile uint32_t *)(0x550 + USB_BASE))
#define USB_HOST_HCDMA2        ((volatile uint32_t *)(0x554 + USB_BASE))
#define USB_HOST_HCCHAR3       ((volatile uint32_t *)(0x560 + USB_BASE))
#define USB_HOST_HCINTMSK3     ((volatile uint32_t *)(0x56C + USB_BASE))
#define USB_HOST_HCTSIZ3       ((volatile uint32_t *)(0x570 + USB_BASE))
#define USB_HOST_HCDMA3        ((volatile uint32_t *)(0x574 + USB_BASE))

uint8_t usb_buffer0[256];
uint8_t usb_buffer1[256];
uint8_t usb_buffer3[256];

struct UsbDeviceRequest {
    uint8_t  Type;
    uint8_t  Request;
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
        uart_puts(" usb interrupt ");
        uart_puthex(reg);
        uart_puts("\n");
        if (reg & 1 << 28) { uart_puts(" Connector ID Status Change\n"); }
        if (reg & 1 << 27) { uart_puts(" LPM Transaction Received Interrupt\n"); }
        if (reg & 1 << 26) { uart_puts(" Periodic TxFIFO Empty\n"); }
        if (reg & 1 << 25) { uart_puts(" Host Channels Interrupt\n"); 
                           }
        if (reg & 1 << 24) { *USB_HOST_HPRT |= 1 << 1;  //clear host port interrupt
                             uart_puts(" Host Port Interrupt\n"); }
        if (reg & 1 <<  6) { uart_puts(" Global IN Non-periodic NAK Effective\n"); }
        if (reg & 1 <<  5) { uart_puts(" Non-periodic TxFIFO Empty\n"); }
        if (reg & 1 <<  4) { uart_puts(" Host and Device RxFIFO Non-Empty (RxFLvl) \n"); }
        if (reg & 1 <<  3) { uart_puts(" Host and Device Start of Frame\n"); }
        if (reg & 1 <<  2) { uart_puts(" OTG Interrupt\n"); }
        if (reg & 1 <<  1) { uart_puts(" Mode Mismatch Interrupt\n"); }
    }
    return;
}

void usbhost_id(void)
{
    uart_printf("USB_CORE_GUID    %x\n", *USB_CORE_GUID);
    uart_printf("USB_CORE_GSNPSID %x\n", *USB_CORE_GSNPSID);
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
}

static void usbhost_poll_intr(void)
{
	uint8_t dev   = 2;

    memset(usb_buffer3, 0, 1024);

    // HCCAR3.EPType = 2'b11 (Interupt), HCCAR3.EPDir = 1'b01(IN), HCCAR3.MPS = 11'h40
    *USB_HOST_HCCHAR3   |= 3 << 18 | 1 << 15 | 1 << 11 | 0x40;  // IN
    *USB_HOST_HCCHAR3   |= dev << 22 ; 

    // recieve interrupt transfer
     // set dma buffer
    *USB_HOST_HCDMA3   = (uint64_t) usb_buffer3;
    *USB_HOST_HCDMA3  |= 0xC0000000;
     // HCTSIZ1.Pid = 2'd2 (DATA1) , HCTSIZ1.PktCnt = 10'h1 , HCTSIZ1.XferSize = 18'd64
    *USB_HOST_HCTSIZ3  = 2 << 29 | 1 << 19 | 64;
     // HCCAR1.ChEna = 1'b1
    *USB_HOST_HCCHAR3 |= 1<<31;

    uart_printf("HUB INT ");
    for (int i = 0; i < 2; i++) {
        uart_printf("%02x ",usb_buffer3[i]);
    }
    uart_printf("\n");
}

static void prepare_control_msg(void)
{
    memset(usb_buffer0, 0, 1024);
    memset(usb_buffer1, 0, 1024);

    // HCCAR1.EPDir = 1'b0 (OUT) / 1'b01(IN), HCCAR1.MPS = 11'h40
    *USB_HOST_HCCHAR0   |= 0x40;            // OUT
    *USB_HOST_HCCHAR1   |= 1 << 15 | 0x40;  // IN

    //clear dev
    *USB_HOST_HCCHAR0   &= ~(0x7f << 22);
    *USB_HOST_HCCHAR1   &= ~(0x7f << 22);
}

static void txrx_control_msg(uint8_t txlen)
{
    // send setup & control packet
     // set dma buffer
    *USB_HOST_HCDMA0   = (uint64_t) usb_buffer0;
    *USB_HOST_HCDMA0  |= 0xC0000000;
     // HCTSIZ0.Pid = 2'd3 (SETUP) , HCTSIZ0.PktCnt = 10'h1 , HCTSIZ0.XferSize = 18'd8
    *USB_HOST_HCTSIZ0 = 3 << 29 | 1 << 19 | txlen;
     // HCCAR1.ChEna = 1'b1
    *USB_HOST_HCCHAR0 |= 1<<31;

    // recieve control packet
     // set dma buffer
    *USB_HOST_HCDMA1   = (uint64_t) usb_buffer1;
    *USB_HOST_HCDMA1  |= 0xC0000000;
     // HCTSIZ1.Pid = 2'd2 (DATA1) , HCTSIZ1.PktCnt = 10'h1 , HCTSIZ1.XferSize = 18'd64
    *USB_HOST_HCTSIZ1  = 2 << 29 | 1 << 19 | 64;
     // HCCAR1.ChEna = 1'b1
    *USB_HOST_HCCHAR1 |= 1<<31;
}


void usbhost_get_descriptor(uint8_t dev, uint8_t descriptor_type, bool is_hub)
{
	prepare_control_msg();

    *USB_HOST_HCCHAR0   |= dev << 22;
    *USB_HOST_HCCHAR1   |= dev << 22; 

    // build packet
    memcpy(usb_buffer0, &(struct UsbDeviceRequest) {
                                .Type = is_hub ? 0xA0 : 0x80,     // DEVICE_TO_HOST | STANDARD | DEVICE
                                .Request = 0x06,  // GET_DESCRIPTOR
                                .Value = descriptor_type << 8 | 0x00,  // descriptor.type = 0x01, decriptor.index = 0x00
                                .Index = 0,
                                .Length = 64,
                      }, 8);

    txrx_control_msg(8);
}

void usbhost_get_portstatus(uint8_t dev, uint8_t port,  bool is_hub)
{
	prepare_control_msg();

    *USB_HOST_HCCHAR0   |= dev << 22;
    *USB_HOST_HCCHAR1   |= dev << 22; 

    // build packet
    memcpy(usb_buffer0, &(struct UsbDeviceRequest) {
                                .Type = is_hub ? 0xA3 : 0x80,     // DEVICE_TO_HOST | CLASS or STANDARD | DEVICE
                                .Request = 0x00,  // GET_STATUS
                                .Index = port,
                                .Length = 4,
                      }, 8);

    txrx_control_msg(8);

    uart_printf("PORT STAtUS %d ", port);
    for (int i = 0; i < 4; i++) {
        uart_printf("%02x ",usb_buffer1[i]);
    }
    uart_puts("\n");
}

void usbhost_set_address(uint8_t address)
{
	prepare_control_msg();

    // build packet
    memcpy(usb_buffer0, &(struct UsbDeviceRequest) {
			                    .Type = 0,
			                    .Request = 5,		// Set address request
                      	        .Value = address,	// Address to set
                      }, 8);

    txrx_control_msg(8);
}

void usbhost_set_config(uint8_t dev, uint8_t config)
{
	prepare_control_msg();

    *USB_HOST_HCCHAR0   |= dev << 22;
    *USB_HOST_HCCHAR1   |= dev << 22; 

    // build packet
    memcpy(usb_buffer0, &(struct UsbDeviceRequest) {
			                    .Type = 0,
			                    .Request = 9,		// Set config
                      	        .Value = config,	// Address to set
                                .Length = 0,
                      }, 8);

    txrx_control_msg(8);
}

void usbhost_set_portfeature(uint8_t dev, uint8_t port,  uint16_t feature)
{
	prepare_control_msg();

    *USB_HOST_HCCHAR0   |= dev << 22;
    *USB_HOST_HCCHAR1   |= dev << 22;

    // build packet
    memcpy(usb_buffer0, &(struct UsbDeviceRequest) {
                                .Type = 0x23,     // ??
                                .Request = 0x03,  // SET_FEATURE
                                .Value = (uint16_t)feature,                             // Feature we are changing
                                .Index = port,
                                .Length = 0,
                      }, 8);

    txrx_control_msg(8);
}

void usbhost_clear_portfeature(uint8_t dev, uint8_t port, uint16_t feature)
{
	prepare_control_msg();

    *USB_HOST_HCCHAR0   |= dev << 22;
    *USB_HOST_HCCHAR1   |= dev << 22;

    // build packet
    memcpy(usb_buffer0, &(struct UsbDeviceRequest) {
                                .Type = 0x23,     // ??
                                .Request = 0x01,  // CLEAR_FEATURE
                                .Value = feature,
                                .Index = port,
                                .Length = 0,
                      }, 8);

    txrx_control_msg(8);
} 
void kernel_main(void)
{
    uart_puts("usb03\n");
    uart_printf("usb_buffer0: %02x\n", usb_buffer0);
    uart_printf("usb_buffer1: %02x\n", usb_buffer1);
    uart_printf("usb_buffer3: %02x\n", usb_buffer3);

    intc_setup();

    usbhost_id();
    usbhost_start();

    uart_puts("GET DEVICE_DESCRIPTOR\n");
    usbhost_get_descriptor(0, 0x01, false);
    for (int i = 0; i < 18; i++) {
        uart_printf("%02x ",usb_buffer1[i]);
    }
    uart_puts("\n");

    uart_puts("SET ADDRESS 2\n");
    usbhost_set_address(2);

    uart_puts("GET CONFIG_DESCRIPTOR\n");
    usbhost_get_descriptor(2, 0x02, false);
    for (int i = 0; i < 18; i++) {
        uart_printf("%02x ",usb_buffer1[i]);
    }
    uart_puts("\n");

    uart_puts("SET CONFIG 1\n");
    usbhost_set_config(2, 1);

    uart_puts("GET HUB_DESCRIPTOR\n");
    usbhost_get_descriptor(2, 41, true);
    for (int i = 0; i < 18; i++) {
        uart_printf("%02x ",usb_buffer1[i]);
    }
    uart_puts("\n");
	uart_printf("Hub device %d has %d ports\n", 0, usb_buffer1[2]);

    usbhost_get_portstatus(2, 0, false);
    usbhost_get_portstatus(2, 1, true);
    usbhost_clear_portfeature(2, 1,  0x0010);
    usbhost_get_portstatus(2, 1, true);

    uart_puts("SET PORT 1 POWER ON\n");
    usbhost_set_portfeature(2, 1, 0x0008);
    usbhost_get_portstatus(2, 1, true);

    uint32_t count = 0;
    count = 0;
    do { ; } while (count++ >= 0x100000);

    uart_puts("CLEAR PORT FEATURE\n");
    usbhost_clear_portfeature(2, 1, 0x0010);
//    usbhost_get_portstatus(2, 1, true);
//    usbhost_poll_intr();

    uart_puts("SET PORT 1 RESET\n");
    usbhost_set_portfeature(2, 1, 0x0004);
//    usbhost_poll_intr();

    uart_puts("CLEAR PORT FEATURE\n");
    usbhost_clear_portfeature(2, 1,  0x0014);
    usbhost_clear_portfeature(2, 1,  0x0011);
//    usbhost_get_portstatus(2, 1, true);
//    usbhost_poll_intr();

    uart_puts("GET DEVICE_DESCRIPTOR\n");
    usbhost_get_descriptor(0, 0x01, false);
    for (int i = 0; i < 18; i++) {
        uart_printf("%02x ",usb_buffer1[i]);
    }
    uart_puts("\n");

    usbhost_set_address(3);
    uart_puts("GET CONFIG_DESCRIPTOR\n");
    usbhost_get_descriptor(3, 0x02, false);
    for (int i = 0; i < 18; i++) {
        uart_printf("%02x ",usb_buffer1[i]);
    }
    uart_puts("\n");

    while (1) {
        io_halt();
    }
}
